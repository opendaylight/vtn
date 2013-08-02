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

my $pat = qr,\x22,;
my ($java_home) = map { s,$pat,\\\x22,g; $_; } (@ARGV);
$fh->print($Config{startperl}, "\n");

$fh->print(<<EOF);

#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Create JNI header files using javah.
##

use vars qw(\$JAVAH);
\$JAVAH = "$java_home" . "/bin/javah";
EOF

$fh->print(<<'EOF');

use strict;
use vars qw($PROGNAME $OUTDIR $VERBOSE $LOCKFILE);

use Fcntl qw(:flock);
use FileHandle;
use File::stat;
use File::Path;
use Getopt::Long;
use POSIX;

$PROGNAME = 'mkjavah';
$OUTDIR = 'include';
$LOCKFILE = '.mkjavah.lock';

sub usage($);
sub fatal(@);
sub dir_create($$);
sub check_classpath(\@);
sub check_header(\@);
sub exec_javah(\@\@);

MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

	my $terminate = sub { exit(3); };
	$SIG{INT} = $terminate;
	$SIG{TERM} = $terminate;

	my $help = sub { usage(0); };

	my (@classpath);
	usage(1) unless (GetOptions
			 ('help'		=> $help,
			  'o|outdir=s'		=> \$OUTDIR,
			  'c|classpath=s'	=> \@classpath,
			  'l|lockfile=s'	=> \$LOCKFILE,
			  'v|verbose+'		=> \$VERBOSE));

	my $lk = FileHandle->new($LOCKFILE, O_CREAT | O_TRUNC | O_RDWR, 0644);
	fatal("$LOCKFILE: Unable to open lockfile: $!") unless ($lk);
	flock($lk, LOCK_EX) or fatal("$LOCKFILE: flock() failed: $!");

	my $cl_mtime = check_classpath(@classpath);
	my $h_mtime = check_header(@ARGV);
	if (!defined($h_mtime) or $h_mtime < $cl_mtime) {
		# Execute javah.
		exec_javah(@classpath, @ARGV);
	}
	elsif ($VERBOSE) {
		print "--- JNI headers are up to date.\n",
	}
}

sub usage($)
{
	my ($status) = @_;

	my $out = ($status) ? \*STDERR : \*STDOUT;
	$out->print(<<OUT);
Usage: $PROGNAME [options] full-qualified-classname [...]

OUT
	if ($status) {
		$out->print(<<OUT);
Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);
        Create JNI header files.

Options:
   -o|--outdir <directory>
        Specify directory path to store JNI header files.
        Default is "$OUTDIR".

   -c|--classpath <file>
        Specify JAR file to be added to classpath.
        At least one JAR file must be specified by this option.
        Multiple arguments can be specified.

   -l|--lockfile <file>
        Specify lock file path.
        Default is "$LOCKFILE" under the current directory.

   -v|--verbose
        Verbose mode.
OUT
	}

	exit $status;
}

sub fatal(@)
{
	die "*** ERROR: ", join('', @_), "\n";
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

sub check_classpath(\@)
{
	my ($classpath) = @_;

	fatal("At least one JAR file must be specified.") unless (@$classpath);

	my $mtime = 0;
	foreach my $c (@$classpath) {
		my $st = stat($c);
		fatal("stat($c) failed: $!") unless ($st);
		fatal("Not a regular file: $c") unless (S_ISREG($st->mode));

		my $mt = $st->mtime;
		$mtime = $mt if ($mtime < $mt);
	}

	# Return the latest modified time.
	print "JAR mtime: $mtime\n" if ($VERBOSE);

	return $mtime;
}

sub check_header(\@)
{
	my ($classes) = (@_);

	fatal("At least one class name must be specified.") unless (@$classes);

	my $pat = qr,[\.\$],;
	my $mtime;
	foreach my $c (@$classes) {
		my $fname = $c;
		$fname =~ s,$pat,_,g;
		my $path = $OUTDIR . '/' . $fname . '.h';
		print "--- $c => $path\n" if ($VERBOSE > 1);

		my $st = stat($path);
		unless ($st and S_ISREG($st->mode)) {
			rmtree($path);
			return undef;
		}

		my $mt = $st->mtime;
		$mtime = $mt if (!defined($mtime) or $mtime > $mt);
	}

	# Return the oldest modified time.
	print "Header mtime: $mtime\n" if ($VERBOSE);

	return $mtime;
}

sub exec_javah(\@\@)
{
	my ($classpath, $classes) = @_;

	unless (-d $OUTDIR) {
		print "=== Creating $OUTDIR\n";
		dir_create($OUTDIR, 0755);
	}

	print "=== Creating JNI headers.\n";
	my $clpath = join(':', @$classpath);

	my (@args) = ('-classpath', $clpath, '-d', $OUTDIR, '-force');

	if ($VERBOSE) {
		print "--- Execute javah: $JAVAH ",
		join(' ', @args, @$classes), "\n";
		
		push(@args, '-verbose') if ($VERBOSE > 1);
	}

	my $pid = fork();
	fatal("fork() failed: $!") unless (defined($pid));
	if ($pid == 0) {
		exec($JAVAH, @args, @$classes);
		die "exec($JAVAH): $!\n";
	}

	while (1) {
		if (waitpid($pid, 0) == -1) {
			next if ($! == EINTR);
			fatal("waitpid($pid) failed: $!");
		}

		use POSIX qw(:sys_wait_h);
		my $status = $?;
		if (WIFEXITED($status)) {
			my $ex = WEXITSTATUS($status);
			last if ($ex == 0);
			fatal("javah died with status $ex.");
		}
		elsif (WIFSIGNALED($status)) {
			my $sig = WTERMSIG($status);
			fatal("javah was killed by signal $sig.");
		}
	}
}
EOF
