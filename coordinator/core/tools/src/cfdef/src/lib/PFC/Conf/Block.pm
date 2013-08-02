#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Parameter block.
##

package PFC::Conf::Block;

use strict;

use PFC::Conf::Constants;

=head1 NAME

PFC::Conf::Block - Parameter block.

=head1 SYNOPSIS

  my $block = PFC::Conf::Block->new("options");
  $block->add($parameter);

=head1 ABSTRACT

An instance of B<PFC::Conf::Block> represents a parameter block defined by
B<defblock> directive in the cfdef file.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Block>.

=head2 METHODS

=over 4

=item I<new>($name, $line)

Constructor.

=over 4

=item $name

Name of parameter block.

=item $line

Line number of the cfdef file in which this block is defined.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($name, $line) = @_;

	my $me = {NAME => $name, LINE => $line, PARAMS => [], NAMES => {}};

	return bless($me, $class);
}

=item I<getName>()

Return name of parameter block.

=cut

sub getName
{
	my $me = shift;

	return $me->{NAME};
}

=item I<getLineNumber>()

Return line number of the cfdef file in which this block is defined.

=cut

sub getLineNumber
{
	my $me = shift;

	return $me->{LINE};
}

=item I<add>($param)

Add the specified parameter to this block.

=over 4

=item $param

A B<PFC::Conf::Parameter> instance which represents a parameter.

=back

=cut

sub add
{
	my $me = shift;
	my ($param) = @_;

	my $name = $param->getName();
	my $nmap = $me->{NAMES};
	die "Duplicated parameter name: $name\n" if ($nmap->{$name});

	$nmap->{$name} = $param;
	push(@{$me->{PARAMS}}, $param);
}

=item I<size>()

Return number of parameters.

=cut

sub size
{
	my $me = shift;

	return scalar(@{$me->{PARAMS}});
}

=item I<getType>()

Return string representation of this block.

=cut

sub getType
{
	return 'block';
}

=item I<getFlags>()

Return ARRAY reference which contains flags for I<pfc_cfdef_block_t> struct.

=cut

sub getFlags
{
	my $me = shift;

	my (@flags);

	foreach my $params (@{$me->{PARAMS}}) {
		if ($params->option(OPT_MANDATORY)) {
			push(@flags, 'PFC_CFBF_MANDATORY');
			last;
		}
	}

	return \@flags;
}

=item I<getParameterSymbolName>()

Return symbol name of I<pfc_cfdef_param_t> array which defines parameters
in this block.

=cut

sub getParameterSymbolName
{
	my $me = shift;

	my $name = $me->getName();
	my $type = $me->getType();

	return sprintf("_cfdef_params_%s_%s", $type, $name);
}

=item I<getForward>()

Return C language code which must be defined before block definitions.

=cut

sub getForward
{
	my $me = shift;

	my (@defs);
	foreach my $param (@{$me->{PARAMS}}) {
		push(@defs, $param->getDefinition());
	}

	my $type = $me->getType();
	my $params = join(",\n", @defs);
	my $symname = $me->getParameterSymbolName();

	my $defs = <<OUT;
static const pfc_cfdef_param_t	$symname\[\] = {
$params
};

OUT

	return $defs;
}

=item I<getDefinition>()

Return C language code which defines this parameter block.

This method creates C language code to initialize I<pfc_cfdef_block_t> struct.

=cut

sub getDefinition
{
	my $me = shift;

	my $name = $me->getName();
	my $symname = $me->getParameterSymbolName();
	my $nparams = scalar(@{$me->{PARAMS}});
	my $f = $me->getFlags();
	my $flags = (@$f) ? join(' | ', @$f) : 0;

	my $defs = <<OUT;
	{
		.cfdb_name	= "$name",
		.cfdb_params	= $symname,
		.cfdb_nparams	= $nparams,
		.cfdb_flags	= ($flags),
	}
OUT

	chomp($defs);

	return $defs;
}

=back

=head1 SEE ALSO

B<PFC::Conf::Parameter>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
