#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Token value class.
##

package PFC::Conf::Token;

use strict;
use vars qw(%TOKEN_NAME);
use overload
	'""'		=> \&stringify,
	fallback	=> 1;

use PFC::Conf::Constants;

# String representation of a token.
%TOKEN_NAME = (TOKEN_SEMI()	=> "SEMI",
	       TOKEN_LBRACE()	=> "LBRACE",
	       TOKEN_RBRACE()	=> "RBRACE",
	       TOKEN_SQLEFT()	=> "SQLEFT",
	       TOKEN_SQRIGHT()	=> "SQRIGHT",
	       TOKEN_EQUAL()	=> "EQUAL",
	       TOKEN_COMMA()	=> "COMMA",
	       TOKEN_COLON()	=> "COLON",
	       TOKEN_SYMBOL()	=> "SYMBOL",
	       TOKEN_INT()	=> "INT",
	       TOKEN_STRING()	=> "STRING");

=head1 NAME

PFC::Conf::Token - Value of token in the cfdef file

=head1 SYNOPSIS

  my $value = PFC::Conf::Token->new($type, $value);

=head1 ABSTRACT

B<PFC::Conf::Token> is an abstract class for token value in the cfdef file.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Token>.

=head2 METHODS

=over 4

=item I<new>($type, $value)

Constructor.

=over 4

=item $type

A token type defined in B<PFC::Conf::Constants>.

=item $value

A token value.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($type, $value) = @_;

	my $me = {TYPE => $type, VALUE => $value};

	return bless($me, $class);
}

=item I<getType>()

Return a token type.

=cut

sub getType
{
	my $me = shift;

	return $me->{TYPE};
}

=item I<getValue>()

Return a token value.

=cut

sub getValue
{
	my $me = shift;

	return $me->{VALUE};
}

=item I<stringify>()

Return a string representation of this token value.

=cut

sub stringify
{
	my $me = shift;

	my $value = $me->getValue();

	return "$value";
}

=item I<getTypeAsString>()

Return a string representation of this token type.

=cut

sub getTypeAsString
{
	my $me = shift;

	my $type = $me->getType();
	my $name = $TOKEN_NAME{$type};
	die "Unknown token type: $type\n" unless ($name);

	return $name;
}

=back

=head1 SEE ALSO

B<PFC::Conf::Lexer>(3),
B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
