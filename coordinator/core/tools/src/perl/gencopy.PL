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
## Create C source code which contains copyright string.
##

use strict;
use vars qw($PROGNAME $TMPFILE $OLDEST);
use File::stat;
use FileHandle;
use Getopt::Long;
use POSIX;

$PROGNAME = 'gencopy';
$OLDEST = 2010;

sub usage($);
sub get_year_range(\@);
sub generate($$);
sub diff($$);

END {
	unlink($TMPFILE) if ($TMPFILE);
}

MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case bundling require_order));
	my $help = sub { usage(0); };
	my ($stamp, $year);

	usage(1) unless (GetOptions('help'		=> $help,
				    's|stamp'		=> \$stamp,
				    'Y|year'		=> \$year,
				    'O|oldest=s'	=> \$OLDEST));
	if ($year) {
		my $range;
		eval {
			my (@tm) = localtime(time);
			$range = get_year_range(@tm);
		};
		if ($@) {
			my $err = "$@";
			chomp($err);

			die "*** ERROR: $err\n";
		}
		print $range;

		exit 0;
	}

	usage(1) unless (@ARGV == 1);
	my $path = $ARGV[0];
	eval {
		generate($path, $stamp);
	};
	if ($@) {
		my $err = "$@";
		chomp($err);

		die "*** ERROR: $err\n";
	}
}

sub usage($)
{
	my ($status) = @_;

	my $out = ($status) ? \*STDERR : \*STDOUT;
	$out->print(<<OUT);
Usage: $PROGNAME [options] <filename>

OUT
	if ($status) {
		$out->print(<<OUT);
Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);
        Generate C source code which contains copyright notice.

Options:
   -s|--stamp
        Embed build timestamp.
        This option always causes rebuild.

   -Y|--year
        Print year to be embedded in copyright notice.

   -O|--oldest <year>
        Specify the oldest year to be embedded in copyright notice.

OUT
	}

	exit $status;
}

sub get_year_range(\@)
{
	my ($tm) = @_;

	my $year = $tm->[5] + 1900;
	die "Adjust system clock on your host: year = $year\n"
		if ($year < $OLDEST);

	my $range;
	if ($year == $OLDEST) {
		$range = $year;
	}
	else {
		$range = $OLDEST . '-' . $year;
	}

	return $range;
}

sub generate($$)
{
	my ($path, $stamp) = @_;

	my (@tm) = localtime(time);
	my $range = get_year_range(@tm);

	$stamp = POSIX::strftime("%Y-%m-%d %H:%M:%S %Z", @tm) if ($stamp);

	# Create temporary file.
	my $tmpfile = sprintf("%s.__tmp.%d", $path, $$);
	my $fh = FileHandle->new($tmpfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	die "open($path) failed: $!\n" unless ($fh);
	$TMPFILE = $tmpfile;

	$fh->print(<<OUT);
const char	copyright[] =
	"Copyright (c) $range NEC Corporation\\n"
	"All rights reserved.";
OUT
	if ($stamp) {
		$fh->print(<<OUT);

const char	build_stamp[] = "$stamp";
OUT
	}

	undef $fh;

	if (-r $path and diff($tmpfile, $path)) {
		# Up to date.
		exit 0;
	}

	print "-- Creating $path\n";
	rename($tmpfile, $path) or die "rename() failed: $!\n";
	undef $TMPFILE;
}

sub diff($$)
{
	my ($path_a, $path_b) = @_;

	my $fh_a = FileHandle->new($path_a);
	die "open($path_a) failed: $!\n" unless ($fh_a);
	my $fh_b = FileHandle->new($path_b);
	die "open($path_b) failed: $!\n" unless ($fh_b);

	while (1) {
		my $la = $fh_a->getline();
		my $lb = $fh_b->getline();

		if (defined($la)) {
			return 0 unless (defined($lb));
			return 0 unless ($la eq $lb);
		}
		elsif (defined($lb)) {
			return 0;
		}
		else {
			last;
		}
	}

	return 1;
}
EOF
