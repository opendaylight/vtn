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
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Resolve file path.
##

use strict;
use vars qw($PROGNAME $SRCROOT $OBJROOT $CREATE $DEPS $CMDS);
use Cwd qw(getcwd abs_path);
use FileHandle;
use File::Basename;
use File::Path;
use Getopt::Long;

$PROGNAME = 'dirpath';

use constant	CONFIG_PL	=> '/build/config.pl';

sub usage($);
sub fatal(@);
sub config_pl();
sub dir_create($$);
sub srcroot($);
sub objroot($);
sub objdir($);

MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

	my $config_pl = config_pl();
	require $config_pl;

	my $help = sub { usage(0); };
	my ($mode, $make, $multi);
	my (%modemap) = (S => \&srcroot, O => \&objroot);
	my $modefunc = sub {
		my ($key, $value) = @_;
		$multi = 1 if ($mode);
		$mode = $modemap{$key};
	};
	usage(1) unless (GetOptions('help'		=> $help,
				    'S|srcroot'		=> $modefunc,
				    'O|objroot'		=> $modefunc,
				    'D=s'		=> \$DEPS,
				    'C=s'		=> \$CMDS,
				    'c'			=> \$CREATE,
				    'm|make'		=> \$make));

	eval {
		fatal("-S and -O are mutually exclusive.") if ($multi);

		my $basedir;
		if (@ARGV == 0) {
			$basedir = getcwd();
			fatal("Failed to determine current directory: $!")
				unless ($basedir);
		}
		elsif (@ARGV == 1) {
			$basedir = $ARGV[0];
		}
		else {
			usage(1);
		}

		$mode = \&objdir unless (defined($mode));
		my $path = &$mode($basedir);
		print $path, "\n";
	};
	if ($@) {
		my $err = "$@";
		chomp($err);

		my ($prefix, $out);
		if ($make) {
			$err =~ s,\.$,,;
			$prefix = '';
			$out = \*STDOUT;
		}
		else {
			$prefix = '*** ';
			$out = \*STDERR;
		}

		$out->print($prefix, $err, "\n");
		exit 1;
	}
}

sub usage($)
{
	my ($status) = @_;

	my $out = ($status) ? \*STDERR : \*STDOUT;
	$out->print(<<OUT);
Usage: $PROGNAME [options] [<base-directory>]

OUT
	if ($status) {
		$out->print(<<OUT);
Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);
        Resolve file path used by build process.
        If no option is specified, object directory path associated with
        the base directory is printed to the standard output.

Options:
   -S|--srcroot
        Print source tree root path.

   -O|--objroot
        Print object tree root path.

   -c
        Create directory.
        This option is ignored if -S or -O option is specified.

   -C <dir>
        Directory name to store build command log.

   -D <dir>
        Directory name to store header dependency file.

   -m|--make
        Assume that $PROGNAME is invoked by make.
OUT
	}

	exit $status;
}

sub fatal(@)
{
	die "ERROR: ", join('', @_), "\n";
}

sub config_pl()
{
	my $dir = dirname($0);

	my $parent = $dir . '/../..';
	my $cf = $parent . CONFIG_PL;
	return $cf if (-r $cf);

	$parent = '..';
	while (1) {
		my $cf = $parent . CONFIG_PL;
		return $cf if (-r $cf);

		my $d = abs_path($parent . '/..');
		fatal("config.pl is not found.") if (!$d or $d eq '/');
		$parent = $d;
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

sub srcroot($)
{
	return $SRCROOT;
}

sub objroot($)
{
	return $OBJROOT;
}

sub objdir($)
{
	my ($basedir) = @_;

	my $base = abs_path($basedir) or
		fatal("Failed to determine base directory: $basedir");
	fatal("Object directory can't be specified as base directory: $basedir")
		if ($base =~ m,^$OBJROOT(/.*)?$,);

	my $objdir;
	if ($base eq $SRCROOT) {
		$objdir = $SRCROOT;
	}
	else {
		$base =~ s,^$SRCROOT/,,;
		fatal("Base directory doesn't reside under source root: ",
		      $basedir) if (substr($base, 0, 1) eq '/');

		$objdir = $OBJROOT . '/' . $base;
	}

	if ($CREATE) {
		unless (-d $objdir) {
			dir_create($objdir, 0755);
		}
		if ($DEPS) {
			my $dir = $objdir . '/' . $DEPS;
			unless (-d $dir) {
				dir_create($dir, 0755);
			}
		}
		if ($CMDS) {
			my $dir = $objdir . '/' . $CMDS;
			unless (-d $dir) {
				dir_create($dir, 0755);
			}
		}
	}

	return $objdir;
}
EOF
