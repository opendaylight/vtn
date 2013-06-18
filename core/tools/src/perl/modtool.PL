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
## Tool for PFC module build.
##

use strict;
use vars qw($PROGNAME $SRCROOT @MODULE_SRCDIR %MODULE_SRCMAP $MODULE_BASEDIR);
use Cwd qw(abs_path);
use File::Basename;
use FileHandle;
use POSIX qw(:DEFAULT :errno_h);
use Getopt::Long;

$PROGNAME = 'modtool';

use constant	CONFIG_PL	=> '/build/config.pl';

use constant	MOD_NAME_LENGTH_MAX	=> 31;
use constant	MOD_VERSION_MAX		=> 255;

sub usage($);
sub srcdir_append($);
sub srcdir_lookup($);
sub load_config_pl();
sub check(@);
sub depends(@);
sub depend_sources(@);
sub parse_depends($$);
sub load_modules(@);
sub load_modules_byname(@);

MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

	my $help = sub { usage(0); };
	my ($mode, $make, $multi, $basedir);
	my (%modemap) = (c => \&check, d => \&depends, s => \&depend_sources,
			 l => \&load_modules, L => \&load_modules_byname);
	my $modefunc = sub {
		my ($key, $value) = @_;
		$multi = 1 if ($mode);
		$mode = $modemap{$key};
	};

	load_config_pl();

	usage(1) unless (GetOptions('help'		=> $help,
				    'c|check'		=> $modefunc,
				    'd|depends'		=> $modefunc,
				    's|depend-sources'	=> $modefunc,
				    'l|load-modules'	=> $modefunc,
				    'L|module'		=> $modefunc,
				    'm|make'		=> \$make,
				    'D|basedir=s'	=> \$basedir));

	eval {
		die "-c, -d, -D, -l, and -L are mutually exclusive.\n"
			if ($multi);
		die "One option must be specified.\n" unless ($mode);
		$basedir = '..' unless ($basedir);
		$MODULE_BASEDIR = srcdir_append($basedir);
		&$mode(@ARGV);
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

		$out->print($prefix, "ERROR: ", $err, "\n");
		exit 1;
	}
}

sub usage($)
{
	my ($status) = @_;

	my $out = ($status) ? \*STDERR : \*STDOUT;
	$out->print(<<OUT);
Usage: $PROGNAME [options] -c <modname>
       $PROGNAME [options] -d <depend-file>
       $PROGNAME [options] -D <depend-file>
       $PROGNAME [options] -l <file>
       $PROGNAME [options] -L <modname> [...]
OUT
	if ($status) {
		$out->print(<<OUT);

Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);

       Tool for PFC module build.

Options:
   -c|--check
        Check whether the given module name is valid.

   -d|--depends
        Parse module dependency file, and list the module names.

   -s|--depend-sources
        Parse module dependency file, and list the module source directories.

   -l|--load-modules
        Generate "load_modules" for pfcd.conf from the given file.

   -L|--module
        Generate "load_modules" for pfcd.conf from the given module name.

   -m|--make
        Assume that $PROGNAME is invoked by make.

   -D|--basedir
        Specify base directory of module sources.
OUT
	}

	exit $status;
}

sub srcdir_append($)
{
	my ($dir) = @_;

	my $path = abs_path($dir);
	die "Unable to determine absolute path: $dir\n" unless ($path);

	unless ($MODULE_SRCMAP{$path}) {
		push(@MODULE_SRCDIR, $path);
		$MODULE_SRCMAP{$path} = 1;
	}

	return $path;
}

sub srcdir_lookup($)
{
	my ($name) = @_;

	foreach my $dir (@MODULE_SRCDIR) {
		my $path = $dir . '/' . $name;
		return $path if (-d $path);
	}

	return undef;
}

sub load_config_pl()
{
	my $dir = dirname($0);

	my $parent = $dir . '/../..';
	my $cf = $parent . CONFIG_PL;
	unless (-r $cf) {
		$parent = '..';
		while (1) {
			$cf = $parent . CONFIG_PL;
			last if (-r $cf);

			my $d = abs_path($parent . '/..');
			die "config.pl was not found.\n" if (!$d or $d eq '/');
			$parent = $d;
		}
	}

	require $cf;

	(%MODULE_SRCMAP) = map {$_ => 1} @MODULE_SRCDIR;
}

sub check(@)
{
	my ($name) = @_;

	die "Module name must be specified.\n" unless ($name);
	die "$name: Too long module name.\n"
		if (length($name) > MOD_NAME_LENGTH_MAX);
	die "$name: Module name must start with alphabet.\n"
		unless ($name =~ m,^[a-zA-Z],o);
	die "$name: Invalid character in module name.\n"
		unless ($name =~ m,^[a-zA-Z]\w*$,o);

	my $srcdir = $MODULE_BASEDIR . '/' . $name;
	die "$name: Source directory does not exist: $srcdir\n"
		unless (-d $srcdir);

	foreach my $dir (@MODULE_SRCDIR) {
		next if ($dir eq $MODULE_BASEDIR);
		my $path = $dir . '/' . $name;
		die "$name: Duplicated module name: $path\n" if (-d $path);
	}

	print "$name\n";
}

sub depends(@)
{
	my ($file) = @_;

	my (@modules);
	my $func = sub {
		push(@modules, $_[0]);
	};

	parse_depends($file, $func);
	print join("\n", @modules), "\n";
}

sub depend_sources(@)
{
	my ($file) = @_;

	my (@dirs);
	my $func = sub {
		push(@dirs, $_[1]);
	};

	parse_depends($file, $func);
	print join("\n", @dirs), "\n";
}

sub parse_depends($$)
{
	my ($file, $func) = @_;

	die "Module dependency file must be specified.\n" unless ($file);

	my $fh = FileHandle->new($file) or die "open($file) failed: $!\n";
	my $errfunc = sub {
		my $ln = $fh->input_line_number();
		my $msg = "$file:$ln " . join('', @_);
		die "$msg\n";
	};

	my (@modules, %map);
	while (my $line = $fh->getline()) {
		next if ($line =~ /^\s*\x23/o);
		next if ($line =~ /^\s*$/o);
		chomp($line);
		$line =~ s,\x23.*$,,go;

		&$errfunc("Invalid format.")
			unless ($line =~ /^\s*([a-zA-Z]\w*)(\@(\d+))?\s*$/o);

		my ($name, $ver) = ($1, $3);
		&$errfunc("Too long module name: $name")
			if (length($name) > MOD_NAME_LENGTH_MAX);
		&$errfunc("Duplicated module dependency: $name")
			if (exists($map{$name}));
		&$errfunc("Invalid version number: $ver")
			if ($ver > MOD_VERSION_MAX);

		# Ensure that the module directory exists.
		my $path = srcdir_lookup($name);
		&$errfunc("Unknown module name: $name") unless (-d $path);

		$map{$name} = 1;
		&$func($name, $path);
	}
}

sub load_modules(@)
{
	my ($file) = @_;

	die "File which lists module names must be specified.\n"
		unless ($file);

	my $fh = FileHandle->new($file);
	my (@modules);
	if ($fh) {
		my $errfunc = sub {
			my $ln = $fh->input_line_number();
			my $msg = "$file:$ln " . join('', @_);
			die "$msg\n";
		};
		my %map;

		while (my $line = $fh->getline()) {
			next if ($line =~ /^\s*\x23/o);
			next if ($line =~ /^\s*$/o);
			chomp($line);
			$line =~ s,\x23.*$,,go;

			&$errfunc("Invalid format.")
				unless ($line =~ /^\s*([a-zA-Z]\w*)\s*$/o);

			my ($name) = ($1);
			next if (exists($map{$name}));
			&$errfunc("Too long module name: $name")
				if (length($name) > MOD_NAME_LENGTH_MAX);
			$map{$name} = 1;
			push(@modules, $name);

			# Ensure that the module directory exists.
			my $path = srcdir_lookup($name);
			&$errfunc("Unknown module name: $name")
				unless (-d $path);
		}
	}
	elsif ($! != ENOENT) {
		die "open($file) failed: $!\n";
	}

	if (@modules) {
		my $mod = join(', ', map { '"' . $_ . '"' } @modules);
		print <<OUT;
	load_modules	= [$mod];
OUT
	}
	else {
		# Print sample configuration.
		print <<OUT;
#	load_modules	= ["module1", "module2"];
OUT
	}
}

sub load_modules_byname(@)
{
	my (@args) = @_;

	my (@modules, %map);
	foreach my $name (@args) {
		next if (exists($map{$name}));

		die "Too long module name: $name\n"
			if (length($name) > MOD_NAME_LENGTH_MAX);
		$map{$name} = 1;
		push(@modules, $name);

		# Ensure that the module directory exists.
		my $path = srcdir_lookup($name);
		die "Unknown module name: $name\n" unless (-d $path);
	}

	if (@modules) {
		my $mod = join(', ', map { '"' . $_ . '"' } @modules);
		print <<OUT;
	load_modules	= [$mod];
OUT
	}
	else {
		# Print sample configuration.
		print <<OUT;
#	load_modules	= ["module1", "module2"];
OUT
	}
}
EOF
