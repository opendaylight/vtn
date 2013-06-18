#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Parameter class.
##

package PFC::Conf::Parameter;

use strict;

use PFC::Conf::Constants;
use PFC::Conf::Integer;

=head1 NAME

PFC::Conf::Parameter - Absolute class for parameter definition.

=head1 SYNOPSIS

  my $param = PFC::Conf::Parameter->new("int_value", "INT32");

=head1 ABSTRACT

B<PFC::Conf::Parameter> is an abstract class for parameter in the cfdef file.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Parameter>.

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

	my $me = {NAME => $name, TYPE => $type, OPTIONS => {}};

	return bless($me, $class);
}

=item I<getName>()

Return parameter name.

=cut

sub getName
{
	my $me = shift;

	return $me->{NAME};
}

=item I<getType>()

Return parameter type.

=cut

sub getType
{
	my $me = shift;

	return $me->{TYPE};
}

=item I<validate>

Validate parameter.
An exception will be thrown if the parameter is invalid.

=cut

sub validate
{
	my $me = shift;

	my $min = $me->option(OPT_MIN);
	my $max = $me->option(OPT_MAX);

	$me->validateRange($min, $max);
}

=item I<validateRange>($min, $max)

Validate I<min> and I<max> option.
This method is called by I<validate>.

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

	if (defined($min) and defined($max)) {
		die "\"min\" must be less than or equal \"max\".\n"
			if ($min > $max);
	}
}

=item I<option>($name, $value)

Set or get option.

This method returns current value of the specified option.

=over 4

=item $name

Option name.

=item $value

Option value to be set.
If the value is not specified, the option value is not changed.

=back

=cut

sub option
{
	my $me = shift;
	my $name = shift;

	my $map = $me->{OPTIONS};
	my $value = $map->{$name};
	if (scalar(@_) > 0) {
		$map->{$name} = $_[0];
	}

	return $value;
}

=item I<arraySize>([$size])

Set or get array size of this parameter.

This method returns current array size of this parameter.
If undefined value is returned, it means that this parameter is not array
parameter.

=over 4

=item $size

Size of array.

If undefined value is specified, the parameter can take variable length array
value. An exception will be thrown if the size is invalid.

If the size is not specified, this method doesn't change the current setting.

=back

=cut

sub arraySize
{
	my $me = shift;

	my $value = $me->{ARRAY};
	if (scalar(@_) > 0) {
		my $size = $_[0];

		# Array size must be INT32 value.
		if (defined($size)) {
			my $i = PFC::Conf::Integer->new("$size", INT_INT32);
			die "must be positive value.\n" if ($i <= 0);
			if ($i > MAX_ARRAY_LENGTH) {
				my $err = sprintf("must be less than %u",
						  MAX_ARRAY_LENGTH);
				die "$err\n";
			}
			$me->{ARRAY} = $i;
		}
		else {
			$me->{ARRAY} = -1;
		}
	}

	return $value;
}

=item I<getDefaultMinValue>()

Return the default value to be set to I<cp_min> member in I<pfc_cfdef_param_t>.

=cut

sub getDefaultMinValue
{
	return 0;
}

=item I<getDefaultMaxValue>()

Return the default value to be set to I<cp_max> member in I<pfc_cfdef_param_t>.

=cut

sub getDefaultMaxValue
{
	return 0;
}

=item I<getDefinition>()

Return C language code which defines this parameter.

This method creates C language code to initialize I<pfc_cfdef_param_t>
struct.

=cut

sub getDefinition
{
	my $me = shift;

	my (@f);
	my $name = $me->getName();
	my $type = 'PFC_CFTYPE_' . $me->getType();

	my $min = $me->option(OPT_MIN);
	if (defined($min)) {
		$min = $min->getHexValue();
	}
	else {
		$min = $me->getDefaultMinValue();
	}

	my $max = $me->option(OPT_MAX);
	if (defined($max)) {
		$max = $max->getHexValue();
	}
	else {
		$max = $me->getDefaultMaxValue();
	}

	push(@f, 'PFC_CFPF_MANDATORY') if ($me->option(OPT_MANDATORY));
	my $flags;
	if (@f) {
		$flags = join(' | ', @f);
	}
	else {
		$flags = 0;
	}

	my $size = $me->arraySize();
	if ($size) {
		$size = 'PFC_CFPARAM_NELEMS_VARLEN' if ($size < 0);
	}
	else {
		$size = 'PFC_CFPARAM_NELEMS_SCALAR';
	}

	my $defs = <<OUT;
	{
		.cfdp_name	= "$name",
		.cfdp_min	= (uint64_t)${min}ULL,
		.cfdp_max	= (uint64_t)${max}ULL,
		.cfdp_type	= $type,
		.cfdp_nelems	= $size,
		.cfdp_flags	= ($flags),
	}
OUT

	chomp($defs);

	return $defs;
}

=back

=head1 SEE ALSO

B<PFC::Conf::Lexer>(3),
B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
