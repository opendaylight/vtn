#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## String parameter.
##

package PFC::Conf::Parameter::String;

use strict;
use base qw(PFC::Conf::Parameter);

use PFC::Conf::Constants;

=head1 NAME

PFC::Conf::Parameter::String - String parameter class.

=head1 SYNOPSIS

  my $param = PFC::Conf::Parameter::String->new("string_value", "STRING");

=head1 ABSTRACT

An instance of B<PFC::Conf::Parameter::String> represents a string parameter
defined in the cfdef file.

=head1 DESCRIPTION

This section describes about public interface for
B<PFC::Conf::Parameter:String>.

=head2 METHODS

=over 4

=item I<validateRange>($min, $max)

Validate I<min> and I<max> option.

This method throws an exception if invalid value range is specified
by I<min> and I<max> option.

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
			if ($value < 0) {
				my $err = sprintf("\"%s\" for %s must be " .
						  "positive.",
						  $opt, TYPE_STRING);
				die "$err\n";
			}
			if ($value > MAX_STRING_LENGTH) {
				my $err = sprintf("String length must be " .
						  "less than %u.",
						  MAX_STRING_LENGTH);
				die "$err\n";
			}
		}
	};

	die "Empty string is meaningless.\n"
		if (defined($max) and $max == 0);

	&$check(OPT_MIN, $min);
	&$check(OPT_MAX, $max);

	$me->SUPER::validateRange($min, $max);
}

=item I<getDefaultMaxValue>()

Return the default value to be set to I<cp_max> member in I<pfc_cfdef_param_t>.

=cut

sub getDefaultMaxValue
{
	return MAX_STRING_LENGTH;
}

=back

=head1 SEE ALSO

B<PFC::Conf::Lexer>(3),
B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
