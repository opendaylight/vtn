# -*- Mode: perl; -*-

use strict;
use vars qw(%Config);
use Config;
use FileHandle;
use File::Basename;
use POSIX;

my $dir = dirname($0);
chdir($dir);
my $file = basename($0);
$file =~ s,\.PL$,,;

my $fh = FileHandle->new($file, O_CREAT|O_WRONLY|O_TRUNC, 0755) or
	die "open($file) failed: $!\n";

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
## Inter-Process Communication framework struct template compiler.
##

use strict;
use vars qw($PROGNAME %Config @INC @UNLINK_ON_ERROR $DEBUG);
use Config;
use Cwd qw(abs_path);
use DirHandle;
use File::Basename;
use File::Path;
use File::stat;
use FileHandle;
use Getopt::Long;
use POSIX;

$PROGNAME = 'ipctc';

=head1 NAME

ipctc - Inter-Process Communication framework struct template compiler.

=head1 SYNOPSIS

  ipctc [options] template [...]

=head1 ABSTRACT

PFC IPC framework supports data structure transfer between processes.
Data structure must be defined by IPC struct template.
B<ipctc> compiles IPC struct template file, and generates header file for
C and C++ language, and struct information file.

=head1 DESCRIPTION

B<ipctc> takes zero or more template file path as argument.

=over 2

=item -

If no template file is specified, empty definitions will be generated.

=item -

If two or more template files are specified, they are merged into one
C language header and one IPC struct information file.

=back

Options for B<ipctc> are described below:

=over 4

=item B<-h>|B<--header-file> I<file>

Place C and C++ language header in the file specified by I<file>.

If omitted, C and C++ language header file is not created.

=item B<-i>|B<--info-file> I<file>

Place IPC struct information in the file specified by I<file>.

If omitted, it is placed in the file named B<ipc_struct.bin> under the
current directory.

=item B<-S>|B<--cxx-server-header> I<file>

Place prototypes for B<pfc::core::ipc::ServerSession> methods for user-defined
data structures in the file specified by I<file>.

If omitted, prototypes are not dumped any file.

=item B<-L>|B<--cxx-server-inline> I<file>

Place inline methods of B<pfc::core::ipc::ServerSession> for user-defined
data structures in the file specified by I<file>.

If omitted, inline methods are not dumped to any file.

=item B<-C>|B<--cxx-client-header> I<file>

Place prototypes for B<pfc::core::ipc::ClientSession> methods for user-defined
data structures in the file specified by I<file>.

If omitted, prototypes are not dumped to any file.

=item B<-l>|B<--cxx-client-inline> I<file>

Place inline methods of B<pfc::core::ipc::ClientSession> for user-defined
data structures in the file specified by I<file>.

If omitted, inline methods are not dumped to any file.

=item B<-n>|B<--namespace> I<string>

Use I<string> as additional namespace for generated structures.

If this option is specified, "I<string>" and "_" are prepended to each
struct name and its type name.

=item B<-I> I<dir>

Append I<dir> to the template file search directory path.

A directory where the source template file exists is always prepended to
the search path.

=item B<-a>|B<--all> I<regexp>

Collect files that match the given regular expression against the file
name under the current directory, and compile them.

If this option is specified, template files specified by command line
arguments are ignored.

=item B<-T>|B<--template-dir> I<dir>

Append template file search path for B<-a> option.
If this option is specified in conjunction with B<-a> option,
template files that match the regular expression specified
by B<-a> option against the file name under I<dir> are compiled.

I<dir> is also appended to template file search path for "include" directive.

=item B<-s>|B<--stamp-file> I<file>

Specify file path used to timestamp check.

If this option is specified, compiled template file paths are recorded
in the file specified by I<file>. It will be used to decide whether
IPC struct information needs to be rebuilt or not.

If no template file is compiled, B<ipctc> exits with status 75.

=item B<--help>

Show supported command line options.

=item B<--doc>

Show this document.

=back

=head1 TEMPLATE FILE FORMAT

IPC structure template file consists of one or more structure definitions.
B<ipc_struct> directive declares the beginning of structure definition.
Structure name must follows B<ipc_struct>. Similar to C language struct
definition, struct fields are defined within enclosed braces, and a semicolon
is required at the end of field definition, and at the end of struct
definition.

B<include> directive can be used to import IPC struct definitions in
other template file.

  include "other_tmplate.ipct";

  ipc_struct <name> {
       <data-type>   <field-name>;
       ...
  };

<data-type> is a keyword which determines data type of the field,
and <field-name> is the field name.

<name> is used to determine struct name, and its type.
If the name of template file is B<foo.ipct>, and the name of structure in the
file is B<bar>, the structure is defined as below:

  typedef struct foo_bar {
      ...
  } foo_bar_t;

=head2 SUPPORTED DATA TYPE

<data-type> must be one of the follows:

=over 4

=item B<INT8>

Signed 8-bit integer. (int8_t)

=item B<UINT8>

Unsigned 8-bit integer. (uint8_t)

=item B<INT16>

Signed 16-bit integer. (int16_t)
The field of this type is always aligned to 2 bytes.

=item B<UINT16>

Unsigned 16-bit integer. (uint16_t)
The field of this type is always aligned to 2 bytes.

=item B<INT32>

Signed 32-bit integer. (int32_t)
The field of this type is always aligned to 4 bytes.

=item B<UINT32>

Unsigned 32-bit integer. (uint32_t)
The field of this type is always aligned to 4 bytes.

=item B<INT64>

Signed 64-bit integer. (int64_t)
The field of this type is always aligned to 8 bytes.

=item B<UINT64>

Unsigned 64-bit integer. (uint64_t)
The field of this type is always aligned to 8 bytes.

=item B<FLOAT>

Single precision floating point. (float)
The field of this type is always aligned to 4 bytes.

=item B<DOUBLE>

Double precision floating point. (double)
The field of this type is always aligned to 8 bytes.

=item B<IPV4>

IPv4 address. (struct in_addr)
The field of this type is always aligned to 4 bytes.

=item B<IPV6>

IPv6 address. (struct in6_addr)
The field of this type is always aligned to 8 bytes.

=back

In addition, other name of struct, which is defined in the same template
file or imported by B<include> directive in advance, can be specified
as data type.

=head2 ARRAY FIELD

Array field can be defined in the same way of C language.

For example, the below definition in the template file generates a uint8_t
array field which has 10 elements.

  UINT8    foo[8];

Note that zero-length array can't be defined.

=head2 STRUCTURE SIZE

The size of structure must be equal or less than 1 megabytes.

=head1 SEE ALSO

B<PFC::IPC::Parser>(3)

=head1 AUTHOR

NEC Corporation

=cut

END {
	unlink(@UNLINK_ON_ERROR) if ($? != 0 and @UNLINK_ON_ERROR);
}

sub usage($);
sub fatal(@);
sub message(@);
sub dir_create($$);
sub get_parser(\%);
sub make_parent_dir($);
sub collect_templates($$);
sub check_timestamp($$\@\@);
sub check_stat($$$);
sub print_time($$);
sub save_templates($$\@);

# Derive library path from script path.
{
	my $dir = dirname($0);
	my $libdir = abs_path($dir . '/../lib');
	my $arch = $Config{archname};
	my $archlibdir = $libdir . '/' . $arch;

	fatal("Failed to determine library path: $libdir")
		unless ($libdir and -d $libdir);
	unshift(@INC, $archlibdir, $libdir);
}

# Stamp file which keeps the list of template files.
{
	package StampFile;

	use overload	'""'	=> \&getName;

	use Fcntl qw(:flock);
	use FileHandle;
	use File::stat;
	use POSIX;

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($name) = @_;

		my $fh = FileHandle->new($name, O_RDWR | O_CREAT, 0644);
		&main::fatal("open($name) failed: $!") unless ($fh);

		flock($fh, LOCK_EX) or
			&main::fatal("$name: flock() failed: $!");
		my $st = stat($fh);
		&main::fatal("$name: stat() failed: $!") unless ($st);

		my $me = {HANDLE => $fh, NAME => $name, STAT => $st};

		return bless($me, $class);
	}

	sub getName
	{
		my $me = shift;

		return $me->{NAME};
	}

	sub getStat
	{
		my $me = shift;

		return $me->{STAT};
	}

	sub readStamp
	{
		my $me = shift;

		my $fh = $me->{HANDLE};
		$fh->seek(0, SEEK_SET) or $me->fatal("seek(0) failed: $!");

		my $st = $me->getStat();
		return undef if ($st->size() == 0);

		# The first line keeps all command line arguments.
		my $cmdline = $fh->getline();
		chomp($cmdline);

		my (@templates);
		while (my $line = $fh->getline()) {
			chomp($line);
			push(@templates, $line);
		}

		return ($cmdline, \@templates);
	}

	sub save
	{
		my $me = shift;
		my ($cmdline, $templates) = @_;

		my $fh = $me->{HANDLE};

		$fh->truncate(0) or $me->fatal("truncate(0) failed: $!");
		$fh->seek(0, SEEK_SET) or $me->fatal("seek(0) failed: $!");

		$fh->print($cmdline, "\n");
		foreach my $tmpl (@$templates) {
			$fh->print($tmpl, "\n");
		}
	}

	sub fatal
	{
		my $me = shift;

		my $name = $me->getName();
		&main::fatal($name, ': ', @_);
	}
}

MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

	my $quit = sub {
		exit 1;
	};
	$SIG{INT} = $quit;
	$SIG{TERM} = $quit;

	my (@orgargv) = (@ARGV);
	my $help = sub { usage(0); };
	my $perldoc = sub {
		my $perldoc = $Config{installbin} . '/perldoc';
		exec($perldoc, $0);
		fatal("exec($perldoc, $0) failed: $!");
	};

	require PFC::IPC;
	import PFC::IPC;

	my $infofile = INFO_DEFAULT();
	my ($header, $server_hh, $svinline_hh, $client_hh, $clinline_hh);
	my ($debug, $namespace, @incpath, $all, $stampfile, @tmpldir);
	usage(1) unless (GetOptions
			 ('help'			=> $help,
			  'doc'				=> $perldoc,
			  'd|debug+'			=> \$DEBUG,
			  'h|header-file=s'		=> \$header,
			  'i|info-file=s'		=> \$infofile,
			  'S|cxx-server-header=s'	=> \$server_hh,
			  'L|cxx-server-inline=s'	=> \$svinline_hh,
			  'C|cxx-client-header=s'	=> \$client_hh,
			  'l|cxx-client-inline=s'	=> \$clinline_hh,
			  'n|namespace=s'		=> \$namespace,
			  'I=s'				=> \@incpath,
			  'a|all=s'			=> \$all,
			  'T|template-dir=s'		=> \@tmpldir,
			  's|stamp-file=s'		=> \$stampfile));

	my $templates;
	if ($all) {
		my $cwd = getcwd();
		push(@incpath, $cwd);
		$templates = collect_templates($all, $cwd);
		foreach my $dir (@tmpldir) {
			my $parent = abs_path($dir);
			my $files = collect_templates($all, $parent);
			push(@$templates, @$files);
			push(@incpath, $parent);
		}
	}
	else {
		fatal("No template file is specified.") unless (@ARGV);
		$templates = \@ARGV;
	}

	my $stamp;
	my (@filespec) = ({PATH => $header, DESC => 'C header'},
			  {PATH => $infofile, DESC => 'info file'},
			  {PATH => $server_hh, DESC => 'C++ server header'},
			  {PATH => $svinline_hh,
			   DESC => 'C++ server inline header'},
			  {PATH => $client_hh, DESC => 'C++ client header'},
			  {PATH => $clinline_hh,
			   DESC => 'C++ client inline header'});

	if ($stampfile) {
		my $cmdline = join(' ', @orgargv);

		$stamp = StampFile->new($stampfile);

		# Determine whether we can skip compilation.
		check_timestamp($cmdline, $stamp, @filespec, @$templates)
			or exit 75;

		# Save template file names.
		$stamp->save($cmdline, $templates);

		# Unlink old target files.
		foreach my $spec (@filespec) {
			my $path = $spec->{PATH};
			unlink($path) if ($path);
		}
	}
	undef @orgargv;

	my (%args);
	$args{NAMESPACE} = $namespace if (defined($namespace));
	$args{INCLUDE_PATH} = \@incpath if (@incpath);

	my $parser = get_parser(%args);
	foreach my $spec (@filespec) {
		my $path = $spec->{PATH};
		make_parent_dir($path);
	}

	eval {
		# Parse the given template files.
		foreach my $file (@$templates) {
			my $ps = $parser->new(%args);
			$ps->parse($file);
			$parser->merge($ps);
		}

		my (@tm) = localtime(time());
		# Generate header file.
		if ($header) {
			message("Generating C header file: $header");
			$parser->dumpHeader($header, \@tm, \@UNLINK_ON_ERROR);
		}

		# Generate IPC struct information file.
		message("Generating IPC struct information: $infofile");
		$parser->dumpInfo($infofile, \@UNLINK_ON_ERROR);

		if ($server_hh) {
			# Generate C++ prototypes for IPC server.
			message("Generating C++ prototypes for IPC server: ",
				$server_hh);
			$parser->dumpServerHeader($server_hh, \@tm,
						  \@UNLINK_ON_ERROR);
		}

		if ($svinline_hh) {
			# Generate C++ inline methods for IPC server.
			message("Generating C++ inline methods for " .
				"IPC server: ", $svinline_hh);
			$parser->dumpServerInlineHeader($svinline_hh, \@tm,
							\@UNLINK_ON_ERROR);
		}

		if ($client_hh) {
			# Generate C++ prototypes for IPC client.
			message("Generating C++ prototypes for IPC client: ",
				$client_hh);
			$parser->dumpClientHeader($client_hh, \@tm,
						  \@UNLINK_ON_ERROR);
		}

		if ($clinline_hh) {
			# Generate C++ inline methods for IPC client.
			message("Generating C++ inline methods for " .
				"IPC client: ", $clinline_hh);
			$parser->dumpClientInlineHeader($clinline_hh, \@tm,
							\@UNLINK_ON_ERROR);
		}
	};
	if ($@) {
		my $err = "$@";
		chomp($err);
		fatal($err);
	}
}

sub usage($)
{
	my ($status) = @_;

	my $out = ($status) ? \*STDERR : \*STDOUT;

	$out->print(<<OUT);
Usage: $PROGNAME [options] template [...]
OUT

	if ($status) {
		$out->print(<<OUT);

Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);

       PFC Inter-Process Communication struct template compiler.

Options:
    -h|--header-file <file>
        Place C and C++ language header in file <file>.

    -i|--info-file <file>
        Place IPC struct information in file <file>.

    -S|--cxx-server-header <file>
        Place C++ accessors for IPC server in file <file>.

    -L|--cxx-server-inline <file>
        Place C++ inline methods for IPC server in file <file>.

    -C|--cxx-client-header <file>
        Place C++ accessors for IPC client in file <file>.

    -l|--cxx-client-inline <file>
        Place C++ inline methods for IPC client in file <file>.

    -n|--namespace <string>
        Specify additional namespace for generated structures.

    -I <dir>
        Append template file search path for "include" directive.

    -a|--all <regexp>
        Compile all files that match the given regular expression against
	file name under the current directory.

    -T <dir>
        Append template file search path for -a option.

    -s|--stamp-file <file>
        Specify file path used to timestamp check.

    --help
        Show this help message.

    --doc
        Show manual.
OUT
	}

	exit $status;
}

sub fatal(@)
{
	die "*** FATAL: " . join('', @_) . "\n";
}

sub message(@)
{
	print "--- ", @_, "\n";
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

sub get_parser(\%)
{
	my ($args) = @_;

	my $parser;
	eval {
		require PFC::Conf;
		import  PFC::Conf;

		# ipctc uses parser for PFC configuration file.
		# IPC struct template file never uses 64-bit integer,
		# so we can configure the PFC::Conf library as ILP32.
		PFC::Conf::LP64(0);

		require PFC::IPC::Parser;
		import PFC::IPC::Parser;

		$parser = PFC::IPC::Parser->new(%$args);
	};
	if ($@) {
		my $err = "$@";
		chomp($err);
		fatal($err);
	}

	return $parser;
}

sub make_parent_dir($)
{
	my ($path) = @_;

	return unless ($path);

	my $dir = dirname($path);
	unless (-d $dir) {
		dir_create($dir, 0755);
	}
}

sub collect_templates($$)
{
	my ($regexp, $parent) = @_;

	message("Collecting: $parent, \"$regexp\"") if ($DEBUG);

	my $reg;
	eval {
		$reg = qr,$regexp,;
	};
	if ($@) {
		my $err = "$@";
		chomp($err);
		fatal("Failed to compile regular expression: $regexp: $err");
	}

	my $dirp = DirHandle->new($parent);
	fatal("Failed to open directory: $parent: $!") unless ($dirp);

	my (@paths);
	while (my $dp = $dirp->read()) {
		next if ($dp eq '.' or $dp eq '..');
		next unless ($dp =~ $reg);
		my $path = $parent . '/' . $dp;
		stat($path) or fatal("stat($dp) failed: $!");
		next unless (-f _ and -r _);
		push(@paths, $path);
	}

	(@paths) = sort(@paths);
	if ($DEBUG) {
		foreach my $tmpl (@paths) {
			message("Template: $tmpl");
		}
	}

	return \@paths;
}

sub check_timestamp($$\@\@)
{
	my ($cmdline, $stamp, $filespec, $templates) = @_;

	# Read template files recorded in the stamp file.
	my ($prevcmd, $prev_tmpl) = $stamp->readStamp();
	unless (defined($prevcmd)) {
		message("Need rebuild: stamp file is missing: $stamp")
			if ($DEBUG);
		return 1;
	}

	# Check to see whether command line argument is changed.
	unless ($cmdline eq $prevcmd) {
		message("Need rebuild: Command line argument has been ",
			"changed.") if ($DEBUG);
		if ($DEBUG > 1) {
			message("Prev: $prevcmd");
			message("Curr: $cmdline");
		}
		return 1;
	}

	# Compare the last modification times.
	# If at least one file is older than stamp file, rebuild is needed.
	my $st = $stamp->getStat();
	my $stamp_mtime = $st->mtime();
	print_time($stamp_mtime, 'Last compiled time') if ($DEBUG);

	foreach my $spec (@$filespec) {
		my $path = $spec->{PATH};
		next unless ($path);
		check_stat($path, $spec->{DESC}, $stamp_mtime) or return 1;
	}

	my $ntmpls = scalar(@$templates);
	my $prev_ntmpls = scalar(@$prev_tmpl);
	unless ($ntmpls == $prev_ntmpls) {
		message("Need rebuild: Number of templates has been changed: ",
			"$prev_ntmpls -> $ntmpls") if ($DEBUG);
		return 1;
	}

	# Compare the last modification time of template with stamp file.
	# If at least one template file is newer than stamp file, rebuild
	# is needed.
	for (my $i = 0; $i < $ntmpls; $i++) {
		my $tmpl = $templates->[$i];
		my $prev = $prev_tmpl->[$i];

		unless ($prev eq $tmpl) {
			message("Need rebuild: Template has been added or ",
				"removed: $prev -> $tmpl") if ($DEBUG);
			return 1;
		}

		$st = stat($tmpl);
		unless ($st) {
			message("Need rebuild: Template is missing: $tmpl")
				if ($DEBUG);
			return 1;
		}

		my $mtime = $st->mtime();
		print_time($mtime, "$tmpl") if ($DEBUG > 1);
		unless ($mtime < $stamp_mtime) {
			message("Need rebuild: Template was modified: $tmpl")
				if ($DEBUG);
			return 1;
		}
	}

	message("Target files are up to date.") if ($DEBUG);

	return undef;
}

sub check_stat($$$)
{
	my ($file, $label, $stamp_mtime) = @_;

	my $st = stat($file);
	unless ($st) {
		message("Need rebuild: $label is missing: $file") if ($DEBUG);
		return undef;
	}

	return $st unless (defined($stamp_mtime));

	my $mtime = $st->mtime();
	print_time($mtime, "$file") if ($DEBUG > 1);
	return $st unless ($mtime < $stamp_mtime);

	message("Need rebuild: $label is older than stamp file: $file")
		if ($DEBUG);

	return undef;
}

sub print_time($$)
{
	my ($t, $label) = @_;

	my (@tm) = localtime($t);
	my $tstr = POSIX::strftime("%Y-%m-%d %H:%M:%S %Z", @tm);
	message("$label: $tstr");
}
EOF
