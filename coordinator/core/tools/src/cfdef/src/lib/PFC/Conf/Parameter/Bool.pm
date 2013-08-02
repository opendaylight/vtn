#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Boolean parameter.
##

package PFC::Conf::Parameter::Bool;

use strict;
use base qw(PFC::Conf::Parameter);

use PFC::Conf::Constants;

=head1 NAME

PFC::Conf::Parameter::Bool - Boolean parameter class.

=head1 SYNOPSIS

  my $param = PFC::Conf::Parameter::Bool->new("bool_value");

=head1 ABSTRACT

An instance of B<PFC::Conf::Parameter::Bool> represents a boolean parameter
defined in the cfdef file.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Parameter:Bool>.

=head2 METHODS

=over 4

=item I<validateRange>($min, $max)

Validate I<min> and I<max> option.

This method throws an exception if I<min> or I<max> option is defined.

=over 4

=item $min

Value of I<min> option.
Undefined value is specified if not defined.

=item $max

Value of I<max> option.
Undefined value is specified if not defined.

=back

=cut

sub validateRange
{
	my $me = shift;
	my ($min, $max) = @_;

	my $check = sub {
		my ($opt, $value) = @_;

		if (defined($value)) {
			my $err = sprintf("BOOL doesn't support \"%s\" option.",
					  $opt);
			die "$err\n";
		}
	};

	&$check(OPT_MIN, $min);
	&$check(OPT_MAX, $max);
}

=back

=head1 SEE ALSO

B<PFC::Conf::Lexer>(3),
B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
