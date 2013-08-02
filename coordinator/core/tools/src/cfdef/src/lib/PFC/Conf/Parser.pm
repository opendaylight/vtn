#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## The cfdef file parser.
##

package PFC::Conf::Parser;

use strict;
use vars qw(%TOPLEVEL %PARAM_CLASS %OPTIONS %VISIBILITY);

use PFC::Conf::Constants;
use PFC::Conf::Lexer;
use PFC::Conf::Integer;
use PFC::Conf::Block;
use PFC::Conf::Map;
use PFC::Conf::Parameter::Bool;
use PFC::Conf::Parameter::String;
use PFC::Conf::Parameter::Integer;

# Top level parser methods.
%TOPLEVEL = (cf_name		=> 'parseName',
	     cf_visibility	=> 'parseVisibility',
	     defblock		=> 'parseDefBlock',
	     defmap		=> 'parseDefMap');

# Class names associated with the cfdef parameter types.
%PARAM_CLASS = (TYPE_BOOL()	=> 'Bool',
		TYPE_STRING()	=> 'String',
		TYPE_BYTE()	=> 'Integer',
		TYPE_INT32()	=> 'Integer',
		TYPE_UINT32()	=> 'Integer',
		TYPE_INT64()	=> 'Integer',
		TYPE_UINT64()	=> 'Integer',
		TYPE_LONG()	=> 'Integer',
		TYPE_ULONG()	=> 'Integer');

# Supported options and parser methods.
%OPTIONS = (mandatory	=> 'parseBoolOption',
	    min		=> 'parseIntOption',
	    max		=> 'parseIntOption');

# Supported symbol visibility.
%VISIBILITY = (default	=> VIS_DEFAULT(),
	       hidden	=> VIS_HIDDEN());

# Parser state for parseBlock().
use constant	S_PARAM_NAME		=> 1;
use constant	S_PARAM_NAME_END	=> 2;
use constant	S_PARAM_TYPE		=> 3;
use constant	S_PARAM_TYPE_END	=> 4;
use constant	S_PARAM_ARRAY		=> 5;
use constant	S_PARAM_ARRAY_END	=> 6;

# Parser state for parseOptions().
use constant	S_OPT_NAME		=> 1;
use constant	S_OPT_END		=> 2;

=head1 NAME

PFC::Conf::Parser - The cfdef file parser.

=head1 SYNOPSIS

  my $parser = PFC::Conf::Parser->new("foo.cfdef");

=head1 ABSTRACT

B<PFC::Conf::Parser> is a syntax analyzer for the cfdef file.
It allows to you to analyze the cfdef file, and dump C language code which
contains the I<pfc_cfdef_t> struct, which represents definitions of the
configuration file syntax.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Parser>.

=head2 METHODS

=over 4

=item I<new>($cfdef, %args)

Constructor.

=over 4

=item $cfdef

Path to the cfdef file.

=item %args

HASH which represents optional arguments.
The following keys are recognized.

=over 4

=item CPP

Path to C preprocessor.

=item CPPFLAGS

ARRAY reference which contains options for C preprocessor.

=item DEPFILE

Path to dependency file to store header file dependency.

=item OUTFILE

Path to output file to write C language source code.

=back

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($file, %args) = @_;

	# Construct default symbol name for pfc_cfdef_t.
	my $symname = $file;
	$symname =~ s,\.,_,g;

	my $me = {SYMNAME => $symname, FILE => $file, ARGS => \%args,
		  VISIBILITY => VIS_DEFAULT(), BLOCK_NAMES => {},
		  BLOCKS => []};

	return bless($me, $class);
}

=item I<parse>()

Parse template file.
An exception will be thrown on parse error.

=cut

sub parse
{
	my $me = shift;

	return if ($me->{_PARSED});
	my $lexer = PFC::Conf::Lexer->new($me->{FILE}, %{$me->{ARGS}});

	while (1) {
		my $token = $lexer->nextToken();
		last unless (defined($token));

		$me->tokenError($lexer, $token)
			unless ($token->getType() == TOKEN_SYMBOL);

		my $value = $token->getValue();
		my $method = $TOPLEVEL{$value};
		$me->tokenError($lexer, $token) unless ($method);

		$me->$method($lexer);
	}

	$lexer->error("No configuration is defined.")
		if (scalar(@{$me->{BLOCKS}}) == 0);

	$me->{_PARSED} = 1;
}

=item dumpSource($fh, %args)

Dump C language code which contains the configuration definitions.

=over 4

=item $fh

File handle of the output file.

=item %args

HASH that contains options.
The following keys are recognized.

=over 4

=item SYMNAME

Symbol name of the I<pfc_cfdef_t> struct.
If not defined, the default value is used.

=item VISIBILITY

Use the specified symbol visibility for I<pfc_cfdef_t> struct.

Value must be one of the followings:

=over 4

=item default

Use default visibility.
This is the default.

=item hidden

Use hidden visibility.

=back

=back

=back

=cut

sub dumpSource
{
	my $me = shift;
	my ($fh, %args) = @_;

	my $symname = $args{SYMNAME};
	unless ($symname) {
		$symname = $me->{SYMNAME};
		die "Couldn't determine symbol name.\n" unless ($symname);
	}

	my $vis = $args{VISIBILITY};
	my $visibility;
	if ($vis) {
		$visibility = $VISIBILITY{$vis};
		die "Unknown visibility: $vis\n" unless (defined($visibility));
	}
	else {
		$visibility = $me->{VISIBILITY};
	}

	my $blocks = $me->{BLOCKS};

	$fh->print(<<OUT);

#include <pfc/conf_parser.h>

/*
 * Definitions for parameters in parameter blocks.
 */
OUT

	foreach my $block (@$blocks) {
		$fh->print($block->getForward());
	}

	$fh->print(<<OUT);
/*
 * Definitions for parameter blocks.
 */
OUT
	my (@defs);
	foreach my $block (@$blocks) {
		push(@defs, $block->getDefinition());
	}

	my $bdefs = join(",\n", @defs);
	my $bsym = '_cfdef_blocks';
	my $nblocks = scalar(@$blocks);

	$vis = ($visibility == VIS_HIDDEN) ? 'PFC_ATTR_HIDDEN' : '';

	$fh->print(<<OUT);
static const pfc_cfdef_block_t	$bsym\[\] = {
$bdefs
};

/*
 * Definition for the configuration file.
 */
const pfc_cfdef_t	$symname $vis = {
	.cfd_block	= $bsym,
	.cfd_nblocks	= $nblocks,
};
OUT
}

=back

=cut

##
## Below are private methods.
##

sub tokenError
{
	my $me = shift;
	my ($lexer, $token) = @_;

	my $v = $token->getValue();
	$lexer->syntaxError("Unexpected token: '$v'");
}

# Parse "cfdef_name" directive.
sub parseName
{
	my $me = shift;
	my ($lexer) = @_;

	my $name = $me->parseSymbolSubstitution($lexer);
	$me->{SYMNAME} = $name->getValue();
}

# Parse "cfdef_visibility" directive.
sub parseVisibility
{
	my $me = shift;
	my ($lexer) = @_;

	my $vis = $me->parseSymbolSubstitution($lexer);
	my $v = $vis->getValue();
	my $visibility = $VISIBILITY{$v};
	$lexer->syntaxError("Unknown visibility: $v")
		unless (defined($visibility));
	$me->{VISIBILITY} = $visibility;
}

# Parse simple substitution, "directive = symbol;".
sub parseSymbolSubstitution
{
	my $me = shift;
	my ($lexer) = @_;

	my $token = $lexer->nextToken(1);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_EQUAL);

	my $symbol = $lexer->nextToken(1);
	$me->tokenError($lexer, $symbol)
		unless ($symbol->getType() == TOKEN_SYMBOL);

	$token = $lexer->nextToken(1);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_SEMI);

	return $symbol;
}

# Parse "defblock" directive.
sub parseDefBlock
{
	my $me = shift;
	my ($lexer) = @_;

	$me->parseBlock($lexer, 'PFC::Conf::Block');
}

# Parse "defmap" directive.
sub parseDefMap
{
	my $me = shift;
	my ($lexer) = @_;

	$me->parseBlock($lexer, 'PFC::Conf::Map');
}

sub parseBlock
{
	my $me = shift;
	my ($lexer, $class) = @_;

	my $token = $lexer->nextToken(1);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_SYMBOL);
	my $name = $token->getValue();
	my $block = $class->new($name, $lexer->lineNumber());

	my $namemap = $me->{BLOCK_NAMES};
	if ($namemap->{$name}) {
		my $block = $namemap->{$name};
		my $ln = $block->getLineNumber();
		$lexer->error("Block name \"$name\" is already defined ",
			      "at line $ln.");
	}

	$token = $lexer->nextToken(1);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_LBRACE);
	my $state = S_PARAM_NAME;
	my ($pname, $param);

	while (1) {
		$token = $lexer->nextToken(1);
		if ($state == S_PARAM_NAME) {
			my $type = $token->getType();
			if ($type == TOKEN_RBRACE) {
				last;
			}
			elsif ($type == TOKEN_SYMBOL) {
				$pname = $token->getValue();
				$state = S_PARAM_NAME_END;
			}
			else {
				$me->tokenError($lexer, $token);
			}
		}
		elsif ($state == S_PARAM_NAME_END) {
			$me->tokenError($lexer, $token)
				unless ($token->getType() == TOKEN_EQUAL);
			$state = S_PARAM_TYPE;
		}
		elsif ($state == S_PARAM_TYPE) {
			$me->tokenError($lexer, $token)
				unless ($token->getType() == TOKEN_SYMBOL);
			my $type = $token->getValue();
			my $clname = $PARAM_CLASS{$type};
			$lexer->syntaxError("Unknown type: $type")
				unless ($clname);
			$clname = 'PFC::Conf::Parameter::' . $clname;
			eval {
				$param = $clname->new($pname, $type);
			};
			if ($@) {
				my $err = "$@";
				chomp($err);
				$lexer->error("Unexpected error: $err");
			}
			eval {
				$block->add($param);
			};
			if ($@) {
				my $err = "$@";
				chomp($err);
				$lexer->error($err);
			}
			$state = S_PARAM_TYPE_END;
		}
		elsif ($state == S_PARAM_TYPE_END) {
			my $type = $token->getType();
			if ($type == TOKEN_SQLEFT) {
				# Array type.
				$state = S_PARAM_ARRAY;
			}
			elsif ($type == TOKEN_COLON) {
				# Options.
				$me->parseOptions($lexer, $param);
				$state = S_PARAM_NAME;
			}
			elsif ($type == TOKEN_SEMI) {
				$state = S_PARAM_NAME;
			}
			else {
				$me->tokenError($lexer, $token);
			}
		}
		elsif ($state == S_PARAM_ARRAY) {
			my $type = $token->getType();
			my $size;
			if ($type == TOKEN_INT) {
				$size = $token->getValue();
				$token = $lexer->nextToken(1);
				$type = $token->getType();
			}
			if ($type == TOKEN_SQRIGHT) {
				eval {
					$param->arraySize($size);
				};
				if ($@) {
					my $err = "$@";
					chomp($err);
					$lexer->error("array size: $err");
				}
				$state = S_PARAM_ARRAY_END;
			}
			else {
				$me->tokenError($lexer, $token);
			}
		}
		elsif ($state == S_PARAM_ARRAY_END) {
			my $type = $token->getType();
			if ($type == TOKEN_COLON) {
				# Options.
				$me->parseOptions($lexer, $param);
				$state = S_PARAM_NAME;
			}
			elsif ($type == TOKEN_SEMI) {
				$state = S_PARAM_NAME;
			}
			else {
				$me->tokenError($lexer, $token);
			}
		}

		if ($state == S_PARAM_NAME and $param) {
			eval {
				$param->validate();
			};
			if ($@) {
				my $err = "$@";
				chomp($err);
				$lexer->error($err);
			}
		}
	}

	$lexer->error("No parameter is defined.")
		if ($block->size() == 0);

	$namemap->{$name} = $block;
	push(@{$me->{BLOCKS}}, $block);
}

sub parseOptions
{
	my $me = shift;
	my ($lexer, $param) = @_;

	my $state = S_OPT_NAME;

	while (1) {
		my $token = $lexer->nextToken(1);
		if ($state == S_OPT_NAME) {
			$me->tokenError($lexer, $token)
				unless ($token->getType() == TOKEN_SYMBOL);
			my $value = $token->getValue();
			my $method = $OPTIONS{$value};
			$lexer->syntaxError("Unknown parameter option: $value")
				unless ($method);
			$me->$method($lexer, $param, $value);
			$state = S_OPT_END;
		}
		elsif ($state == S_OPT_END) {
			my $type = $token->getType();
			if ($type == TOKEN_SEMI) {
				last;
			}
			elsif ($type == TOKEN_COMMA) {
				$state = S_OPT_NAME;
			}
			else {
				$me->tokenError($lexer, $token);
			}
		}
	}
}

sub parseBoolOption
{
	my $me = shift;
	my ($lexer, $param, $opt) = @_;

	$param->option($opt, 1);
}

sub parseIntOption
{
	my $me = shift;
	my ($lexer, $param, $opt) = @_;

	my $token = $lexer->nextToken(1);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_EQUAL);

	$token = $lexer->nextToken(1);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_INT);

	$param->option($opt, $token->getValue());
}

=head1 SEE ALSO

B<PFC::Conf::Lexer>(3),
B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
