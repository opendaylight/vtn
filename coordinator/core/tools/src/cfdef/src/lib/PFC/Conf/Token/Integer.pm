#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Integer value.
##

package PFC::Conf::Token::Integer;

use strict;
use base qw(PFC::Conf::Token);

use PFC::Conf::Integer;
use PFC::Conf::Constants;

=head1 NAME

PFC::Conf::Token::Integer - Value of integer token in the cfdef file

=head1 SYNOPSIS

  use PFC::Conf::Constants;

  my $value = PFC::Conf::Token::Integer->new(INT_INT32, $value);

=head1 ABSTRACT

An instance of B<PFC::Conf::Token::Integer> represents an integer token
in the cfdef file.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Token::String>.

=head2 METHODS

=over 4

=item I<new>($itype, $value)

Constructor.

=over 4

=item $itype

Integer type defined in B<PFC::Conf::Constants>.

=item $value

A token value.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($itype, $value) = @_;

	my $obj = PFC::Conf::Integer->new($value, $itype);
	my $me = $class->SUPER::new(TOKEN_INT, $obj);

	return bless($me, $class);
}

=back

=head1 SEE ALSO

B<PFC::Conf::Token>(3),
B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
