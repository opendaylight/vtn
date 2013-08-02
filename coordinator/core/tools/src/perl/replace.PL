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
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Replace strings in a text file.
##

use strict;
use vars qw($PROGNAME @UNLINK_ON_ERROR);
use FileHandle;
use POSIX;
use Getopt::Long;

$PROGNAME = 'replace';

sub usage($);
sub fatal(@);
sub stringify($);
sub parse_attributes(\%$$);
sub parse_patterns(\@\%);
sub replace($$\@);

END {
	if (@UNLINK_ON_ERROR) {
		unlink(@UNLINK_ON_ERROR) unless ($? == 0);
	}
}

# A pair of regular expression and replacement.
MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case require_order));

	my (@patterns, $pat, @attributes, $attr, $output);
	my $help = sub { usage(0); };
	my $pfunc = sub {
		my ($key, $value) = @_;

		if ($pat) {
			$pat->{REPL} = $value;
			push(@patterns, $pat);
			undef $pat;
		}
		else {
			$pat = {PATTERN => $value};
		}
	};
	my $afunc = sub {
		my ($key, $value) = @_;

		if ($attr) {
			$attr->{VALUE} = $value;
			push(@attributes, $attr);
			undef $attr;
		}
		else {
			$attr = {PATTERN => $value};
		}
	};

	usage(1) unless (GetOptions('help'		=> $help,
				    'o|output=s'	=> \$output,
				    'p|pattern=s{2}'	=> $pfunc,
				    'a|attr=s{2}'	=> $afunc));
	usage(1) unless (@ARGV == 1);

	# Parse attributes.
	my (%attrmap);
	foreach my $attr (@attributes) {
		parse_attributes(%attrmap, $attr->{PATTERN}, $attr->{VALUE});
	}

	# Parse patterns.
	parse_patterns(@patterns, %attrmap);

	# Prepare output file if needed.
	my ($out);
	if ($output) {
		$out = FileHandle->new($output, O_CREAT | O_TRUNC | O_WRONLY,
				       0644);
		fatal("Unable to open output file($output): $!") unless ($out);
		push(@UNLINK_ON_ERROR, $output);
	}
	else {
		$out = \*STDOUT;
	}

	# Replace patterns, and store results to the file.
	replace($ARGV[0], $out, @patterns);
}

sub usage($)
{
	my ($status) = @_;

	my $out = ($status) ? \*STDERR : \*STDOUT;
	$out->print(<<OUT);
Usage: $PROGNAME [options] file
OUT
	if ($status) {
		$out->print(<<OUT);

Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);

       Replace strings specified by regular expression in a text file.

Options:
   -o|--output <file>
        Store results to the given file.
        If omitted, results are printed out to the standard output.

   -p|--pattern <regexp> <replacement>
        Replace all strings that matches <regexp> to <replacement>.

   -a|--attr <regexp> <attribute>
        Specify attribute for <regexp> specified by -p and -f.

        string
            <replacement> is converted into a quoted string.

        min=<number>
            Specify the minimum length of <replacement>.

        max=<number>
            Specify the maximum length of <replacement>.
OUT
	}

	exit $status;
}

sub fatal(@)
{
	die "*** ERROR: ", join('', @_), "\n";
}

sub stringify($)
{
	my ($str) = @_;

	$str =~ s,\x22,\\\x22,g;

	return '"' . $str . '"';
}

sub parse_attributes(\%$$)
{
	my ($map, $regexp, $value) = @_;

	my $attr = $map->{$regexp};
	unless ($attr) {
		$attr = {};
		$map->{$regexp} = $attr;
	}

	foreach my $val (split(/,/, $value)) {
		if ($val eq 'string') {
			$attr->{string} = 1;
			next;
		}

		if ($val =~ m,^(max|min)=(\d+)$,o) {
			$attr->{$1} = $2;

			my ($min, $max);
			if ($1 eq 'max') {
				$min = $attr->{min};
				$max = $2;
			}
			else {
				$min = $2;
				$max = $attr->{max};
			}

			if (defined($min) and defined($max) and $max < $min) {
				fatal("$regexp: \"max\" value must not be ",
				      "less than \"min\": min=$min, max=$max");
			}
			next;
		}

		fatal("$regexp: Unexpected attributes: $val");
	}
}

sub parse_patterns(\@\%)
{
	my ($patterns, $attrmap) = @_;

	foreach my $pat (@$patterns) {
		my $pattern = $pat->{PATTERN};
		my $repl = $pat->{REPL};
		my $attr = $attrmap->{$pattern};

		my $len = length($repl);
		my $min = $attr->{min};
		if (defined($min) and $len < $min) {
			fatal("$pattern: The length of replacement($len) ",
			      "must be greater than or equal to $min.");
		}
		my $max = $attr->{max};
		if (defined($max) and $len > $max) {
			fatal("$pattern: The length of replacement($len) ",
			      "must be less than or equal to $max.");
		}

		$pat->{REPL} = stringify($repl) if ($attr->{string});

		eval {
			$pat->{REGEXP} = qr,$pattern,;
		};
		if ($@) {
			my $err = "$@";

			chomp($err);
			fatal("Illegal regular expression: $pattern: $err");
		}
	}
}

sub replace($$\@)
{
	my ($file, $out, $patterns) = @_;

	my $fh = FileHandle->new($file);
	fatal("Unable to open input file($file): $!") unless ($fh);

	while (my $line = $fh->getline()) {
		foreach my $pat (@$patterns) {
			my ($regexp, $repl) = ($pat->{REGEXP}, $pat->{REPL});

			$line =~ s,$regexp,$repl,g;
		}

		$out->print($line);
	}
}
EOF
