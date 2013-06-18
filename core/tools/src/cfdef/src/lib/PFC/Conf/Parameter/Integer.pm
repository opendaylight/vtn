#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Integer parameter.
##

package PFC::Conf::Parameter::Integer;

use strict;
use vars qw(%INTEGER_TYPE);
use base qw(PFC::Conf::Parameter);

use PFC::Conf::Constants;
use PFC::Conf::Integer;

# Convert cfdef parameter type into integer type.
%INTEGER_TYPE = (TYPE_BYTE()	=> INT_BYTE(),
		 TYPE_INT32()	=> INT_INT32(),
		 TYPE_UINT32()	=> INT_UINT32(),
		 TYPE_INT64()	=> INT_INT64(),
		 TYPE_UINT64()	=> INT_UINT64(),
		 TYPE_LONG()	=> INT_LONG(),
		 TYPE_ULONG()	=> INT_ULONG());

=head1 NAME

PFC::Conf::Parameter::Integer - Integer parameter class.

=head1 SYNOPSIS

  my $param = PFC::Conf::Parameter::Integer->new("int32_value", "INT32");

=head1 ABSTRACT

B<PFC::Conf::Parameter::Integer> is an abstract class for integer type
parameter.

=head1 DESCRIPTION

This section describes about public interface for
B<PFC::Conf::Parameter:Integer>.

=head2 METHODS

=over 4

=item I<new>($name, $type)

Constructor.

=over 4

=item $name

Name of parameter.

=item $type

String representation of parameter type in the cfdef file.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($name, $type) = @_;

	my $itype = $INTEGER_TYPE{$type};
	die "Unknown integer type: $type\n" unless (defined($itype));
	my $me = $class->SUPER::new($name, $type);

	$me->{INTTYPE} = $itype;
	$me->{INTOBJ} = PFC::Conf::Integer->new(0, $itype);

	return bless($me, $class);
}

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

	my $itype = $me->{INTTYPE};
	my $check = sub {
		my ($opt, $value) = @_;

		return unless (defined($value));

		eval {
			my $ival = PFC::Conf::Integer->new("$value", $itype);
		};
		if ($@) {
			my $err = "$@";
			chomp($err);
			die "$opt: $err\n";
		}
	};

	&$check(OPT_MIN, $min);
	&$check(OPT_MAX, $max);

	$me->SUPER::validateRange($min, $max);
}

=item I<getDefaultMinValue>()

Return the default value to be set to I<cp_min> member in I<pfc_cfdef_param_t>.

=cut

sub getDefaultMinValue
{
	my $me = shift;

	my $i = $me->{INTOBJ};

	return $i->getMinHexValue();
}

=item I<getDefaultMaxValue>()

Return the default value to be set to I<cp_max> member in I<pfc_cfdef_param_t>.

=cut

sub getDefaultMaxValue
{
	my $me = shift;

	my $i = $me->{INTOBJ};

	return $i->getMaxHexValue();
}

=back

=cut

=head1 SEE ALSO

B<PFC::Conf::Lexer>(3),
B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
