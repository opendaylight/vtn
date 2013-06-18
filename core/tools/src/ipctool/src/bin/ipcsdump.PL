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
## IPC struct information dumper.
##

use strict;
use vars qw($PROGNAME %Config @INC $PARSER @UNLINK_ON_ERROR);
use Config;
use Cwd qw(abs_path);
use FileHandle;
use File::Basename;
use Getopt::Long;
use POSIX;

$PROGNAME = 'ipcsdump';

use constant	COL_LABEL		=> 20;
use constant	COL_STRIDX		=> 6;

=head1 NAME

ipcsdump - Dump contents of IPC struct information file.

=head1 SYNOPSIS

  ipcsdump [options] infofile

=head1 ABSTRACT

B<ipcsdump> is a debugging utility to dump the contents of IPC struct
information file. It is always dumped to the standard output.

=head1 DESCRIPTION

B<ipcsdump> takes one IPC struct information file path as argument.

Options for B<ipcsdump> are described below:

=over 4

=item B<-h>|B<--dump-header> <file>

Directs B<ipcsdump> to construct C language header file instead of dumping
the contents of IPC struct information file. C language header is dumped
into the given file.

=item B<--help>

Show supported command line options.

=item B<--doc>

Show this document.

=back

=head1 AUTHOR

NEC Corporation

=cut

END {
	unlink(@UNLINK_ON_ERROR) if ($? != 0 and @UNLINK_ON_ERROR);
}

sub usage($);
sub fatal(@);
sub get_parser($);
sub dump_contents($$);
sub dump_struct($$);
sub dump_field($$);
sub dump_c_header($$);
sub get_string($$);

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

MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

	my $quit = sub {
		exit 1;
	};
	$SIG{INT} = $quit;
	$SIG{TERM} = $quit;

	my $help = sub { usage(0); };
	my $perldoc = sub {
		my $perldoc = $Config{installbin} . '/perldoc';
		exec($perldoc, $0);
		fatal("exec($perldoc, $0) failed: $!");
	};

	my ($head, $verbose);
	usage(1) unless (GetOptions
			 ('help'		=> $help,
			  'doc'			=> $perldoc,
			  'h|dump-header=s'	=> \$head,
			  'v|verbose+'		=> \$verbose));
	usage(1) unless (@ARGV == 1);

	my $file = $ARGV[0];

	# Parse information file.
	eval {
		my $parser = get_parser($file);

		if ($head) {
			dump_c_header($parser, $head);
		}
		else {
			dump_contents($parser, $verbose);
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
Usage: $PROGNAME [options] template
OUT

	if ($status) {
		$out->print(<<OUT);

Try "$PROGNAME --help".
OUT
	}
	else {
		$out->print(<<OUT);

       Dump the contents of IPC struct information file.

Options:
    -h|--dump-header <file>
        Construct C language header file from the given information file.

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

sub get_parser($)
{
	my ($file, $args) = @_;

	my $parser;
	if ($PARSER) {
		$parser = $PARSER->new($file, %$args);
	}
	else {
		require PFC::IPC::StructInfo::Parser;
		import PFC::IPC::StructInfo::Parser;

		$parser = PFC::IPC::StructInfo::Parser->new($file);
		$PARSER = $parser;
	}

	return $parser;
}

sub dump_contents($$)
{
	my ($parser, $verbose) = @_;

	require PFC::IPC;
	import PFC::IPC;

	my $head = $parser->getHeader();
	my $order = $head->{ifh_order};
	if ($order == IPC_ORDER_LITTLE()) {
		$order = "LITTLE";
	}
	elsif ($order == IPC_ORDER_BIG()) {
		$order = "BIG";
	}
	else {
		$order = "UNKNOWN ($order)";
	}

	my $ns = $head->{ifh_namespace};
	my $namespace = get_string($parser, $ns);
	$namespace = '<no namespace>' unless ($namespace);
	my $nstructs = $head->{ifh_nstructs};

	my $lwidth = COL_LABEL;
	print "=== Header\n";
	printf("  %*s: 0x%02x\n", -$lwidth, "Magic", $head->{ifh_magic});
	printf("  %*s: %u\n", -$lwidth, "Version", $head->{ifh_version});
	printf("  %*s: %s\n", -$lwidth, "Order", $order);
	printf("  %*s: %s\n", -$lwidth, "Number of Structs", $nstructs);
	printf("  %*s: %u (%s)\n", -$lwidth, "Namespace", $ns, $namespace);
	printf("  %*s: %u\n", -$lwidth, "Field Section Offset",
	       $head->{ifh_fldoff});
	printf("  %*s: %u\n", -$lwidth, "Field Section Size",
	       $head->{ifh_fldsize});
	printf("  %*s: %u\n", -$lwidth, "String Table Offset",
	       $head->{ifh_stroff});
	printf("  %*s: %u\n", -$lwidth, "String Table Size",
	       $head->{ifh_strsize});

	print "\n=== Struct Section\n";
	my $index = 0;
	foreach my $struct (@{$parser->getStructs()}) {
		print "  -- [$index]\n";
		dump_struct($parser, $struct);
	}
	continue {
		$index++;
	}
	print "  No struct is defined.\n" if ($index == 0);

	print "\n=== Field Section\n";
	$index = 0;
	foreach my $field (@{$parser->getFields()}) {
		print "  -- [$index]\n";
		dump_field($parser, $field);
	}
	continue {
		$index++;
	}
	print "  No struct field is defined.\n" if ($index == 0);

	return unless ($verbose);

	print "\n=== String Table Section\n";
	my $strkeys = $parser->getStringKeys();
	print "\n    Number of Strings: ", scalar(@$strkeys), "\n\n";
	foreach my $idx (@$strkeys) {
		my $sidx = "[$idx]";
		my $str = $parser->getString($idx);
		printf("    %*s  \"%s\"\n", -COL_STRIDX(), $sidx, $str);
	}
}

sub dump_struct($$)
{
	my ($parser, $struct) = @_;

	my $nameidx = $struct->{ifs_name};
	my $name = get_string($parser, $nameidx);

	my $lwidth = COL_LABEL - 2;
	printf("    %*s: %u (%s)\n", -$lwidth, "Name", $nameidx, $name);
	printf("    %*s: %u\n", -$lwidth, "Number of Fields",
	       $struct->{ifs_nfields});
	printf("    %*s: %u\n", -$lwidth, "Size", $struct->{ifs_size});
	printf("    %*s: %u\n", -$lwidth, "Alignment", $struct->{ifs_align});
	printf("    %*s: %u\n", -$lwidth, "Field Index", $struct->{ifs_field});
	printf("    %*s: %s\n", -$lwidth, "Signature", $struct->{ifs_sig});
}

sub dump_field($$)
{
	my ($parser, $field) = @_;

	require PFC::IPC::StructInfo;
	import PFC::IPC::StructInfo;
	require PFC::IPC::Type;
	import PFC::IPC::Type;

	my $nameidx = $field->{iff_name};
	my $name = get_string($parser, $nameidx);

	my $type = $field->{iff_type};
	my $tdesc;
	if ($type & IPC_IFFIELD_STRUCT()) {
		my $idx = $type & (~IPC_IFFIELD_STRUCT());
		my $stname = get_string($parser, $idx);
		$tdesc = "STRUCT: index=$idx, $stname";
	}
	else {
		$tdesc = PFC::IPC::Type::getSymbol($type);
		$tdesc = "???" unless ($tdesc);
	}

	my $lwidth = COL_LABEL - 2;
	printf("    %*s: %u (%s)\n", -$lwidth, "Name", $nameidx, $name);
	printf("    %*s: %u\n", -$lwidth, "Array Size", $field->{iff_array});
	printf("    %*s: 0x%x (%s)\n", -$lwidth, "Type", $type, $tdesc);
}

sub dump_c_header($$)
{
	my ($parser, $file) = @_;

	require PFC::IPC::Parser;
	import PFC::IPC::Parser;

	my $tparser = PFC::IPC::Parser->newFromInfo($parser);
	my $mtime = $parser->getModifiedTime();
	my (@tm) = localtime($mtime);

	my $fdesc = ($file eq '-') ? "the standard output" : $file;
	print "--- Generating C header file into $fdesc.\n";
	$tparser->dumpHeader($file, \@tm, \@UNLINK_ON_ERROR);
}

sub get_string($$)
{
	my ($parser, $index) = @_;

	my $str = $parser->getString($index);
	$str = "???" unless (defined($str));

	return $str;
}
EOF
