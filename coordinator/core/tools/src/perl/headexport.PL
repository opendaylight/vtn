use strict;
use vars qw(%Config);
use Config;
use FileHandle;
use File::Basename;
use POSIX;

my $fname = $0;
$fname =~ s,\.PL$,,;

my $fh = FileHandle->new($fname, O_CREAT|O_TRUNC|O_WRONLY, 0755) or
	die "open($fname) failed: $!\n";

$fh->print($Config{startperl}, "\n");

$fh->print(<<'EOF');

#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Export header files into the specified directory.
##

use strict;
use vars qw($PROGNAME $OUTPUT $VERBOSE $QUIET);
use Cwd qw(abs_path);
use DirHandle;
use FileHandle;
use File::Path;
use Getopt::Long;
use POSIX;

$PROGNAME = 'headexport';
$OUTPUT = 'out';
$VERBOSE = 0;

sub usage($);
sub fatal(@);
sub verbose($@);
sub dir_create($$);
sub parse($$);
sub output_init($);

END {
	rmtree($OUTPUT) if ($? != 0 and -d $OUTPUT);
}

# Header file class.
{
	package BaseHeader;

	use File::Basename;
	use File::Copy;
	use File::Path;

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($path) = (@_);

		my $me = {PATH => $path, NAME => basename($path)};

		return bless($me, $class);
	}

	sub getName
	{
		my $me = shift;

		return $me->{NAME};
	}

	sub parentDirectory
	{
		my $me = shift;

		return $_[0];
	}

	sub export
	{
		my $me = shift;
		my ($output) = @_;

		my $name = $me->getName();
		my $parent = $me->parentDirectory($output);
		my $src = $me->{PATH};
		my $dst = $parent . '/' . $me->{NAME};

		$! = 0;
		unless (-d $parent) {
			&main::dir_create($parent, 0755);
		}

		&main::fatal("Duplicated header: $name: $dst") if (-e $dst);
		copy($src, $dst) or
			&main::fatal("copy($src, $dst) failed: $!");
		unless ($main::QUIET) {
			print <<OUT;
=== Installing $dst
OUT
		}
	}
}

# Header file class which takes directory prefix for inclusion.
{
	package PrefixedHeader;

	use base qw(BaseHeader);

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($prefix, $path, $subpfx, $name) = (@_);

		my $me = $class->SUPER::new($path);
		$me->{PREFIX} = ($subpfx) ? $prefix . '/' . $subpfx : $prefix;

		return bless($me, $class);
	}

	sub getName
	{
		my $me = shift;

		return $me->{PREFIX} . '/' . $me->{NAME};
	}

	sub parentDirectory
	{
		my $me = shift;
		my ($output) = @_;

		return $output . '/' . $me->{PREFIX};
	}
}

# Header inclusion prefix.
{
	package InclusionPrefix;

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($pfx) = @_;

		my $me = {PREFIX_LIST => $pfx};

		return bless($me, $class);
	}

	sub getHeader
	{
		my $me = shift;
		my ($path, $relpath) = @_;

		my (@list) = split('/', $relpath);
		my $fname = pop(@list);
		return undef unless ($fname);

		my $pfx = $me->{PREFIX_LIST};
		my $npfx = scalar(@$pfx);
		my $i;

		LOOP:
		for ($i = scalar(@list) - $npfx; $i >= 0; $i--) {
			for (my $j = 0; $j < $npfx; $j++) {
				next LOOP if ($list[$i + $j] ne $pfx->[$j]);
			}
			last;
		}

		return undef if ($i < 0);

		my $prefix = join('/', @$pfx);
		my $start = $i + $npfx;
		my $subpfx = ($start > $#list) ? undef : @list[$start, -1];

		return PrefixedHeader->new($prefix, $path, $subpfx, $fname);
	}
}

# Temporary file for cpp input.
{
	package Input;

	use FileHandle;
	use POSIX qw(:DEFAULT :errno_h :fcntl_h);
	use vars qw($AUTOLOAD);

	use constant	FILENAME_PREFIX => '_headexport';

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($retain) = @_;

		# Determine file path.
		my $pid = $$;
		my $name = sprintf("%s.%u", FILENAME_PREFIX, $pid);
		if (-e $name) {
			my $index = 0;
			while (1) {
				$name = sprintf("%s.%u.%u", FILENAME_PREFIX,
						$pid, $index);
				last unless (-e $name);
				$index++;
			}
		}

		my $oflags = O_CREAT | O_EXCL | O_RDWR | O_TRUNC;
		my $fh = FileHandle->new($name, $oflags, 0600);
		&main::fatal("open($name) failed: $!") unless ($fh);

		my $me = {HANDLE => $fh, NAME => $name, RETAIN => $retain};

		return bless($me, $class);
	}

	sub add
	{
		my $me = shift;
		my ($line) = @_;

		my $fh = $me->{HANDLE};
		$fh->printf("#include <%s>\n", $line);
	}

	sub getName
	{
		my $me = shift;

		return $me->{NAME};
	}

	sub DESTROY
	{
		my $me = shift;

		delete($me->{HANDLE});
		unlink($me->{NAME}) unless ($me->{RETAIN});
	}
}

# C preprocessor context.
{
	package PreProcessor;

	use Cwd qw(abs_path);
	use FileHandle;
	use POSIX qw(:DEFAULT :sys_wait_h);

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($sources, $cpp, $cppflags) = @_;

		my $pat_line = qr,^\x23\s+\d+\s+"(.+)",;
		my (@pat_sources);
		foreach my $src (@$sources) {
			push(@pat_sources, qr,\Q$src/\E,);
		}
		my $pat_src = '^(' . join('|', @pat_sources) . ')';
		$pat_src = qr,$pat_src,;

		my $me = {PAT_LINE => $pat_line, PAT_SOURCE => $pat_src,
			  CPP => $cpp, CPPFLAGS => $cppflags};

		return bless($me, $class);
	}

	sub run
	{
		my $me = shift;
		my ($input, $output, $headfunc) = @_;

		my ($rh, $wh) = FileHandle::pipe();
		&main::fatal("pipe() failed: $!") unless ($rh);

		my $ipath = abs_path($input->getName());
		my $cpp = $me->{CPP};
		my (@args) = ($cpp);
		my $cppflags = $me->{CPPFLAGS};
		push(@args, @$cppflags) if ($cppflags);
		push(@args, $ipath);

		my $pid = fork();
		&main::fatal("fork() failed: $!") unless (defined($pid));
		if ($pid == 0) {
			undef $rh;
			$wh->autoflush(1);
			POSIX::dup2($wh->fileno(), STDOUT->fileno()) or
				die "dup2() failed: $!\n";
			exec(@args);
			die "exec($cpp): $!\n";
		}

		undef $wh;

		my $pat_line = $me->{PAT_LINE};
		my $pat_source = $me->{PAT_SOURCE};
		my (%handled) = ($ipath => 1);
		while (my $line = $rh->getline()) {
			next unless ($line =~ $pat_line);
			my $file = $1;
			next if ($handled{$file});
			$handled{$file} = 1;

			unless (-e $file) {
				&main::verbose(1, 'Ignore pseudo inclusion: ',
					       $file);
				next;
			}
			$file = abs_path($file);
			my $relpath = $file;
			unless ($relpath =~ s,$pat_source,,) {
				&main::verbose(1, 'Ignore system header: ',
					       $file);
				next;
			}

			my $head = &$headfunc($file, $relpath);
			unless ($head) {
				&main::verbose(1, 'Ignore private header: ',
					       $file);
				next;
			}

			$head->export($output);
		}

		while (1) {
			if (waitpid($pid, 0) == -1) {
				next if ($! == EINTR);
				&main::fatal("waitpid() failed: $!");
			}

			my $status = $?;
			if (WIFEXITED($status)) {
				my $ex = WEXITSTATUS($status);
				last if ($ex == 0);
				&main::fatal("C preprocessor died with ",
					     "status: ", $ex);
			}
			elsif (WIFSIGNALED($status)) {
				&main::fatal("C preprocessor was killed by ",
					     "signal ", WTERMSIG($status),
					     ".");
			}
		}
	}
}

MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

	my $terminate = sub { exit(3); };
	$SIG{INT} = $terminate;
	$SIG{TERM} = $terminate;

	my $help = sub { usage(0); };
	my (@prefix, @cppflags, $retain);
	my (@srcs);
	my $cpp = 'cpp';
	usage(1) unless (GetOptions
			 ('help'		=> $help,
			  'q|quiet'		=> \$QUIET,
			  's|source=s'		=> \@srcs,
			  'p|prefix=s'		=> \@prefix,
			  'o|output=s'		=> \$OUTPUT,
			  'c|cpp=s'		=> \$cpp,
			  'C|cppflag=s'		=> \@cppflags,
			  'retain'		=> \$retain,
			  'v|verbose+'		=> \$VERBOSE));


	my (@sources);
	foreach my $src (@srcs) {
		fatal('Source directory is not found: ', $src)
			unless (-d $src);
		$src = abs_path($src);
		verbose(1, 'Source directory: ', $src);
		push(@sources, $src);
	}
	push(@sources, getcwd()) unless (@sources);

	my $headfunc;
	if (@prefix) {
		# Directory prefixes are specified.
		my (@patterns);

		my $pat = qr,^/+(.+?)/+$,;
		my $slash = qr,/+,;
		foreach my $p (@prefix) {
			$p =~ s,$pat,$1,g;
			next unless ($p);
			my (@pl) = split($slash, $p);

			push(@patterns, InclusionPrefix->new(\@pl));
		}

		$headfunc = sub {
			my ($path, $relpath) = @_;

			foreach my $p (@patterns) {
				my $head = $p->getHeader($path, $relpath);
				return $head if ($head);
			}

			verbose(0, "Prefix does not match: ", $path);
			return undef;
		};
	}
	else {
		$headfunc = sub { return BaseHeader->new($_[0]); };
	}

	if ($VERBOSE) {
		print "CPP       : $cpp\n";
		if (@cppflags) {
			print "CPPFLAGS  :\n";
			foreach my $f (@cppflags) {
				print "  $f\n";
			}
			print "\n";
		}
	}

	# Create temporary file for cpp input.
	my $input = Input->new($retain);

	# Initialize output directory.
	output_init($OUTPUT);

	# Parse input files.
	if (@ARGV) {
		foreach my $file (@ARGV) {
			unless (-f $file) {
				verbose(1, "Skip: $file");
				next;
			}

			verbose(1, "Input: $file");
			my $fh = FileHandle->new($file);

			fatal("open($file) failed: $!") unless ($fh);
			parse($input, $fh);
		}
	}
	else {
		parse($input, \*STDIN);
	}

	# Run C preprocessor, and export header files.
	my $processor = PreProcessor->new(\@sources, $cpp, \@cppflags);
	$processor->run($input, $OUTPUT, $headfunc);
}

sub usage($)
{
	my ($status) = @_;

	my $out = ($status) ? \*STDERR : \*STDOUT;
	$out->print(<<OUT);
Usage: $PROGNAME [options] [<filename> ...]

OUT
	if ($status) {
		$out->print(<<OUT);
Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);
        Export header files into the specified directory.
        If more than one filenames are specified as argument, header files
        to be exported are read from them. If no argument is specified,
        the standard input is used.

Options:
   -q|--quiet
        Do not print installation log.

   -s|--source <directory>
        Specify source directory path which contains header files to be
        exported. Default is current directory.

   -p|--prefix <prefix>
        Specify directory prefix for inclusion.
        Multiple prefixes can be specified.

   -o|--output <directory>
        Specify directory path for output. Default is "out".
        Note that the specified directory will be cleaned up.

   -c|-cpp <cpp>
        Specify path to C preprocessor command. Default is "cpp".

   -C|-cppflag <flag>
        Specify flags for C preprocessor. Multiple flags can be specified.

   -v|--verbose
        Verbose mode.

   --retain
        Retain cpp input file for debugging.

OUT
	}

	exit $status;
}

sub fatal(@)
{
	die '*** FATAL: ' . join('', @_) . "\n";
}

sub verbose($@)
{
	my $level = shift;

	if ($level < $VERBOSE) {
		my $prefix = '-' x ($level + 1);
		STDOUT->print($prefix, ' ', join('', @_), "\n");
	}
}

sub dir_create($$)
{
	my ($path, $mode) = @_;

	eval { mkpath($path, undef, $mode); };
	if ($@) {
		my $err = "$@";

		chomp($err);
		fatal("Unable to create directory: $path: $err");
	}
}

sub parse($$)
{
	my ($input, $fh) = @_;

	my $pat_comment_line = qr,^\x23,;
	my $pat_comment = qr,\x23.*$,;
	my $pat_strip = qr,^\s*(.+?)\s*$,;

	while (my $line = $fh->getline()) {
		next if ($line =~ $pat_comment_line);
		chomp($line);
		$line =~ s,$pat_comment,,g;
		$line =~ s,$pat_strip,$1,g;
		next unless ($line);

		$input->add($line)
	}
}

sub output_init($)
{
	my ($output) = @_;

	unless (-d $output) {
		# Create output directory.
		verbose(0, "New output directory: $output");
		dir_create($output, 0755);

		return;
	}

	# Clean up output directory.
	verbose(0, "Clean up output directory: $output");
	my $dirp = DirHandle->new($output) or
		fatal("opendir($output) failed: $!");
	while (my $dp = $dirp->read()) {
		next if ($dp eq '.' or $dp eq '..');

		my $path = $output . '/' . $dp;
		rmtree($path);
	}
}

EOF
