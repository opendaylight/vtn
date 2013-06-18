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
## List files under the given directory.
##

use strict;
use vars qw($PROGNAME);
use DirHandle;
use FileHandle;
use Getopt::Long;

$PROGNAME = 'listfile';

sub usage($);
sub fatal(@);
sub parse_args();
sub list_file($$\@);

# File test class for "--contains".
{
	package Contains;

	use DirHandle;

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($regexp) = @_;

		my $pattern;
		eval {
			$pattern = qr,$regexp,;
		};
		if ($@) {
			my $err = "$@";
			chomp($err);
			die "Invalid regular expression: $err\n";
		}

		return bless({PATTERN => $pattern}, $class);
	}

	sub evaluate
	{
		my $me = shift;
		my ($dir, $name) = @_;

		my $path = "$dir/$name";
		return undef unless (-d $path);

		my $dirp = DirHandle->new($path);
		return undef unless ($dirp);

		my $pattern = $me->{PATTERN};
		while (my $dp = $dirp->read()) {
			return 1 if ($dp =~ m,$pattern,);
		}

		return undef;
	}
}

# File test class for "--match".
{
	package Match;

	use base qw(Contains);

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;

		my $me = $class->SUPER::new(@_);

		return bless($me, $class);
	}

	sub evaluate
	{
		my $me = shift;
		my ($dir, $name) = @_;

		my $pattern = $me->{PATTERN};

		return $name =~ $pattern;
	}
}

# File test class for "--eliminate".
{
	package Eliminate;

	use base qw(Contains);

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;

		my $me = $class->SUPER::new(@_);

		return bless($me, $class);
	}

	sub evaluate
	{
		my $me = shift;
		my ($dir, $name) = @_;

		my $pattern = $me->{PATTERN};

		return $name !~ $pattern;
	}
}

MAIN:
{
	my ($listfunc, $conditions, $conderr, $make) = parse_args();

	eval {
		fatal($conderr) if ($conderr);
		fatal("At least one directory must be specified.")
			unless (@ARGV);

		foreach my $dir (@ARGV) {
			list_file($dir, $listfunc, @$conditions);
		}
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
Usage: $PROGNAME [options] directory [...]
OUT

	if ($status) {
		$out->print(<<OUT);

Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);

       List files under the given directories.

Options:
   -c|--contains <regexp>
        List directories which contain the regular file of which the
        file name matches the given regular expression.

   -p|--match <regexp>
        List files only if the file name matches the given regular expression.

   -e|--eliminate <regexp>
        Do not list the file if its file name matches the given regular
        expression.

   --abs
        List absolute path rather than file name.

   --rel
        List relative path rather than file name.

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

sub parse_args()
{
	Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

	my $help = sub { usage(0); };

	my (%classes) = (c => 'Contains', p => 'Match', e => 'Eliminate');
	my (@conditions, $conderr);
	my $condfunc = sub {
		my ($key, $value) = @_;

		my $class = $classes{$key};
		eval {
			push(@conditions, $class->new($value));
		};
		if ($@ and !$conderr) {
			my $err = "$@";
			chomp($err);
			$conderr = "-$key: $err";
		}
	};

	my $listfunc = sub {
		print $_[1], "\n";
	};
	my $absfunc = sub {
		$listfunc = sub {
			my ($dir, $name) = @_;
			use Cwd qw(abs_path);

			my $path = abs_path("$dir/$name");
			print $path, "\n";
		};
	};
	my $relfunc = sub {
		$listfunc = sub {
			my ($dir, $name) = @_;

			my $path = ($dir eq '.') ? $name : "$dir/$name";
			print $path, "\n";
		};
	};
	my $make;
	usage(1) unless (GetOptions('help'		=> $help,
				    'abs'		=> $absfunc,
				    'rel'		=> $relfunc,
				    'c|contains=s'	=> $condfunc,
				    'p|match=s'		=> $condfunc,
				    'e|eliminate=s'	=> $condfunc,
				    'm|make'		=> \$make));

	return ($listfunc, \@conditions, $conderr, $make);
}

sub list_file($$\@)
{
	my ($dir, $listfunc, $conditions) = @_;

	my $dirp = DirHandle->new($dir);
	fatal("$dirp: Failed to open directory: $!") unless ($dirp);

	DIRLOOP:
	while (my $dp = $dirp->read()) {
		next if ($dp eq '.' or $dp eq '..');

		foreach my $cond (@$conditions) {
			next DIRLOOP unless ($cond->evaluate($dir, $dp));
		}

		&$listfunc($dir, $dp);
	}
}
EOF
