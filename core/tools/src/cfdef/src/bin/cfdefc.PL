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
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Convert PFC configuration definitions file into C language code.
##

use strict;
use vars qw($PROGNAME %Config @INC $PARSER @UNLINK_ON_ERROR);
use Config;
use Cwd qw(abs_path);
use File::Basename;
use FileHandle;
use Getopt::Long;
use POSIX qw(:DEFAULT :signal_h);

$PROGNAME = 'cfdefc';

=head1 NAME

cfdefc - PFC configuration definition file compiler

=head1 SYNOPSIS

  cfdefc [options] cfdef_file

=head1 ABSTRACT

PFC configuration file system requires configuration definition file
(cfdef file). B<cfdefc> converts cfdef file into C language code which contains
pfc_cfdef_t, which determines the syntax of the configuration file.

=head1 DESCRIPTION

B<cfdefc> takes one cfdef file path as argument, and compiles it.

The options for B<cfdefc> are:

=over 4

=item B<-o>|B<--output> I<file>

Specify filename to dump C language code.

If omitted, B<cfdefc> tries to detect output filename from input filename.
In this case, input filename must have '.cfdef' as suffix. If 'foo.cfdef' is
specified as input filename, output filename will be 'foo.c'.

=item B<-c>|B<--cpp> I<cpp>

Specify path to C preprocessor.

If this option is specified, the cfdef file is filtered by C preprocessor
before parsing.

=item B<-C>|B<--cppflag> I<flag>

Specify option for C preprocessor.

If B<-L> option is specified, "B<-DPFC_LP64=1>" is automatically added to
C preprocessor flags. So you can use "#ifdef PFC_LP64" in the cfdef file
to define LP64 specific parameter.

This option is ignored unless B<-c> option is specified.

=item B<-D>|B<--depfile> I<file>

Specify file to store header file dependencies.

If this option is specified, B<cfdefc> generates a rule suitable for
B<make> program describing the header file dependencies of the cfdef file.

This option is ignored unless B<-c> option is specified.

=item B<-L>|-B<--LP64>

Create definition for LP64 system.

=item B<-n>|B<--name> I<name>

Force to use the given I<name> as a symbol name for I<pfc_cfdef_t> struct.

If this option is specified, the I<name> directive in the cfdef file is
ignored.

=item B<-v>|B<--visibility> I<visibility>

Force to change symbol visibility for pfc_cfdef_t struct.

I<visibility> must be one of the followings:

=over 4

=item default

Use default visibility.
This is the default.

=item hidden

Use hidden visibility.

=back

=item B<--help>

Show supported command line options.

=item B<--doc>

Show this document.

=back

=head1 CFDEF FILE FORMAT

PFC configuration file consists of I<block> and I<map>.
The following is the format of block in the configuration file.

  <block-name> {
      <parameters>;
      ...
  }

Each block has unique name and one or more parameters.
Block name must be unique in the configuration file.
Parameters are pairs of name and value combined with '=' character,
and it ends with ';'.

The following example defines a block named "options",
and parameters "int_value" and "string_value". Value of "int_value" is 1,
and value of "string_value" is "default".

  options {
      int_value    = 1;
      string_value = "default";
  }

The following is the format of map in the configuration file.

  <map-name> <name> {
      <parameters>;
      ...
  }

<map-name> defines the name of map, and <name> defines the key of map block.
Name of map must be unique in the configuration file, and name parameter must
be unique in the same map. That is, map acts like hash table which associates
parameter block with unique name.

The following example defines a map named "maps".
"maps" has the two keys, "foo" and "bar".
Each parameter block in the map has two parameters, "int_value" and
"string_value". Values of "int_value" and "string_value" associated with the
map key "foo" is 1 and "value1", values associated with the map key "bar" is
2 and "value2".

  maps "foo" {
      int_value    = 1;
      string_value = "value1";
  }

  maps "bar" {
      int_value    = 2;
      string_value = "value2";
  }

Note that characters between '%' and the end of line are considered
as comment.

=head2 HOW TO DEFINE BLOCK

To define a block, you must put I<defblock> directive in in the cfdef file.

  defblock <block-name> {
      <param-def>;
      ...
  }

<block-name> is the name of block. <param-def> defines parameters in this
block. The format of <param-def> is described later.

The name of block has the following constraints:

=over 2

=item -

It must starts with an alphabet.

=item -

It must consists of alphabet, digits, and underscore character ('_').

=item -

The length of block name must be less than 64.

=back

If the cfdef file contains the following definition,

  defblock options {
      ...
  }

the configuration file can have one block named "options".

  options {
      ...
  }

=head2 HOW TO DEFINE MAP

Use I<defmap> directive instead of I<defblock>.

  defmap <map-name> {
      <param-def>;
      ...
  }

<map-name> is the name of map. The name of map has the same constraints as
the name of block. Note that the same name can't be used for the block and
the map.

If the cfdef file contains the following definition,

  defmap maps {
      ...
  }

the configuration file can have maps named "maps".
You can put more than two maps as long as they have unique name.

  maps "foo" {
      ...
  }

  maps "bar" {
      ...
  }

=head2 HOW TO DEFINE PARAMETERS

The following is the format of parameter definition.

  <param-name> = <param-type> <param-options>;

<param-name> is the name of parameter, and <param-type> is a symbol which
determines data type of parameter. <param-options> are optional definitions
for the parameter. More than two options can be defined by specifying options
separated by comma.

=head3 INTEGER PARAMETER

If you want to define parameter which takes integer value, you must choose
one parameter type from the followings.

=over 4

=item B<BYTE>

Parameter takes a byte value. A byte value is considered as unsigned 8-bit
value.

=item B<INT32>

Parameter takes a signed 32-bit value.

=item B<UINT32>

Parameter takes an unsigned 32-bit value.

=item B<INT64>

Parameter takes a signed 64-bit value.

=item B<UINT64>

Parameter takes an unsigned 64-bit value.

=item B<LONG>

Parameter takes a signed long integer value.
On LP64 system, this type is identical to INT64.
Otherwise it is identical to INT32.

=item B<ULONG>

Parameter takes an unsigned long integer value.
On LP64 system, this type is identical to UINT64.
Otherwise it is identical to UINT32.

=back

An integer value can be specified in the following format.

=over 2

=item -

If the value starts with "0x", it is considered as hexadecimal format.

=item -

If the value starts with "0", it is considered as octal format.

=item -

Otherwise, it is considered as decimal format.

=back

For example, the following line in the cfdef file defines a parameter
named "int32_value", and its type is signed 32-bit integer.

  int32_value = INT32;

Value for this parameter can be specified in the configuration file like this:

  int32_value = 0x10;

=head3 BOOLEAN PARAMETER

If you want to define parameter which takes boolean value, you must specify
B<BOOL> as parameter type.

A boolean value is specified by I<true> or I<false>. I<true> means a true
value, and I<false> a false.

For example, the following line in the cfdef file defines a boolean pamrater
named "bool_value".

  bool_value = BOOL;

The following line in the configuration file defines value for "bool_value"
parameter as true.

  bool_value = true;

=head3 STRING PARAMETER

If you want to define parameter which takes string value, you must specify
B<STRING> as parameter type.

A string value is specified by a quoted string, quoted by double quotation
mark. Supported encodings are US-ASCII and UTF-8. Just like C language style
string, backslash escape can be used in a string, but only the followings are
supported.

=over 4

=item \r

Carriage return

=item \n

Line feed (new line)

=item \t

Horizontal tab

=item \'

Single quotation mark

=item \"

Double quotation mark

=item \\

Backslash character

=back

For example, the following line in the cfdef file defines a string parameter
named "string_value".

  string_value = STRING;

The following line in the configuration file defines value for "string_value"
parameter as "value".

  string_value = "value";

=head3 ARRAY PARAMETER

A parameter can have array type, which takes a sequence of values.
If '[]' is specified just after parameter type, it is considered as an array
parameter. If a number is specified in square bracket, it is considered as
the size of an array. If not, it is considered as variable-length array.

For example, the following line in the cfdef file defines a parameter
"int32_array" which has variable-length int32 value.

  int32_array = INT32[];

The following line in the cfdef file determines the size of array elements
as 4. If the number of elements specified in the configuration file is not 4,
the parser will cause a fatal error.

  int32_array = INT32[4];

In the configuration file, value of array can be specified by square bracket
and comma-separated value like this.

  int32_array = [1, 2, 3, 4];

=cut

=head3 PARAMETER OPTIONS

Each parameter definition has one or more options.
If you want to define options for a parameter, you must put ':' between
parameter type and option list.

The following is the supported options:

=over 4

=item I<mandatory>

This option declares the parameter is mandatory.
The configuration file parser causes fatal error if at least one mandatory
option is not defined in the configuration file.

The following example declares "int32_value" as mandatory.

  int32_value = INT32: mandatory;

=item I<min>

This option declares the minimum value of the parameter.
The configuration file parser causes fatal error if the parameter value is
less than this value.

=over 2

=item -

This option can't be specified to boolean parameter.

=item -

If this option is specified to string parameter, the value of this option
is considered as the minimum length of the parameter.

=item -

If this option is specified to array parameter, this constraint is applied
to all array elements.

=back

=item I<max>

This option declares the maximum value of the parameter.
The configuration file parser causes fatal error if the parameter value is
greater than this value.

=over 2

=item -

This option can't be specified to boolean parameter.

=item -

If this option is specified to string parameter, the value of this option
is considered as the maximum length of the parameter.

=item -

If this option is specified to array parameter, this constraint is applied
to all array elements.

=back

The following example declares "int32_value" which can take a value between
-10 and 10.

  int32_value = INT32: min=-10, max=10;

=back

=head2 C LANGUAGE SYMBOL

B<cfdefc> constructs I<pfc_cfdef_t> struct in the C language source file.
The definition of I<pfc_cfdef_t> can be controlled by the cfdef file directive.

=over 4

=item B<cf_name>

The name of I<pfc_cfdef_t> struct can be specified by B<cf_name> directive.

The following example specifies the name of I<pfc_cfdef_t> struct as
"pfc_cfdef".

  cf_name = pfc_cfdef;

If a B<cf_name> directive is not specified, the name of I<pfc_cfdef_t>
struct is determined by the cfdef filename, replacing all '.' in filename
with '_'.

If the cfdef filename is "foo.cfdef", the default I<pfc_cfdef_t> struct name
is "foo_cfdef".

=item B<cf_visibility>

Symbol visibility of I<pfc_cfdef_t> can be specified by B<cf_visibility>
directive.

The following example changes visibility of I<pfc_cfdef_t> struct to hidden.

  cf_visibility = hidden;

Currently, supported visibilities are I<default> and I<hidden>.
If omitted, the default visibility is used.

=back

=head2 REMARKS

=over 4

=item -

B<cfdefc> limits the length of string parameter to 1023.
If a value larger than 1023 is specified for B<STRING> parameter's
I<min> or I<max> option, B<cfdefc> causes a fatal error.

=item -

B<cfdefc> limits the number of array elements to 256.
If a value larger than 256 is specified for the number of array elements.
B<cfdefc> causes a fatal error.

=back

=head1 AUTHOR

NEC Corporation

=cut

END {
	unlink(@UNLINK_ON_ERROR) if ($? != 0 and @UNLINK_ON_ERROR);
}

use constant	CFDEF_SUFFIX	=> 'cfdef';

sub usage($);
sub fatal(@);
sub get_parser($$\%);
sub get_output($$);

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
	$SIG{CHLD} = 'DEFAULT';
	POSIX::sigprocmask(SIG_SETMASK, POSIX::SigSet->new());

	my $help = sub { usage(0); };
	my $perldoc = sub {
		my $perldoc = $Config{installbin} . '/perldoc';
		exec($perldoc, $0);
		fatal("exec($perldoc, $0) failed: $!");
	};

	my ($outfile, $cpp, @cppflags, $lp64, $symname, $vis, $depfile);
	usage(1) unless (GetOptions
			 ('help'		=> $help,
			  'doc'			=> $perldoc,
			  'o|output=s'		=> \$outfile,
			  'c|cpp=s'		=> \$cpp,
			  'C|cppflag=s'		=> \@cppflags,
			  'D|depfile=s'		=> \$depfile,
			  'L|lp64'		=> \$lp64,
			  'n|name=s'		=> \$symname,
			  'v|visibility=s'	=> \$vis));
	usage(1) unless (@ARGV == 1);

	if ($cpp) {
		if ($lp64) {
			push(@cppflags, '-DPFC_LP64=1');
		}
		else {
			push(@cppflags, '-UPFC_LP64');
		}
	}

	my $file = $ARGV[0];
	my %args;
	$args{CPP} = $cpp if ($cpp);
	$args{CPPFLAGS} = \@cppflags if (@cppflags);
	if ($depfile) {
		$args{DEPFILE} = $depfile;
		push(@UNLINK_ON_ERROR, $depfile);
	}

	my $out;
	($out, $outfile) = get_output($outfile, $file);
	$args{OUTFILE} = $outfile if ($outfile);

	my $parser = get_parser($file, $lp64, %args);

	eval {
		$parser->parse();

		$out->print(<<OUT);
/*
 * Generated by $PROGNAME. DO NOT EDIT!!
 */
OUT
		$parser->dumpSource($out, SYMNAME => $symname,
				    VISIBILITY => $vis);
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

       PFC configuration definition file compiler

Options:
    -o|--output directory
        Specify filename to dump C language code.

    -c|--cpp cpp
        Specify path to C preprocessor.

    -C|--cppflag flag
        Specify flags for C preprocessor.

    -D|--depfile file
        Write a make rule describing the header file dependencies.

    -L|--LP64
        Create definition for LP64 system.

    -n|--name name
        Force to use the given name as symbol name for pfc_cfdef_t struct.

    -v|--visibility visibility
        Force to change symbol visibility for pfc_cfdef_t struct.

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

sub get_parser($$\%)
{
	my ($file, $lp64, $args) = @_;

	my $parser;
	eval {
		if ($PARSER) {
			$parser = $PARSER->new($file, %$args);
		}
		else {
			require PFC::Conf;
			import  PFC::Conf;

			# Configure the target system type.
			PFC::Conf::LP64($lp64);

			require PFC::Conf::Parser;
			import PFC::Conf::Parser;

			$parser = PFC::Conf::Parser->new($file, %$args);
			$PARSER = $parser;
		}
	};
	if ($@) {
		my $err = "$@";
		chomp($err);
		fatal($err);
	}

	return $parser;
}

sub get_output($$)
{
	my ($fname, $cfdef) = @_;

	if ($fname) {
		return (\*STDOUT, undef) if ($fname eq '-');
	}
	else {
		# Determine output file from the cfdef filename.
		my $suffix = CFDEF_SUFFIX;
		my ($name, $dir, $sfx) = fileparse($cfdef, $suffix);
		fatal("The cfdef file path must have \".$suffix\" as suffix.")
			unless ($sfx eq $suffix);
		if ($dir eq '.' or $dir eq './') {
			$fname = $name . 'c';
		}
		else {
			$fname = $dir . $name . 'c';
		}
	}

	my $fh = FileHandle->new($fname, O_CREAT|O_WRONLY|O_TRUNC, 0644) or
		fatal("open($fname) failed: $!");
	push(@UNLINK_ON_ERROR, $fname);

	return ($fh, $fname);
}
EOF
