#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Parameter map.
##

package PFC::Conf::Map;

use strict;
use base qw(PFC::Conf::Block);

=head1 NAME

PFC::Conf::Map - Parameter map.

=head1 SYNOPSIS

  my $map = PFC::Conf::Map->new("map");
  $map->add($parameter);

=head1 ABSTRACT

An instance of B<PFC::Conf::Map> represents a parameter block defined by
B<defmap> directive in the cfdef file.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Map>.

=head2 METHODS

=over 4

=item I<getType>()

Return string representation of this block.

=cut

sub getType
{
	return 'map';
}

=item I<getFlags>()

Return ARRAY reference which contains flags for I<pfc_cfdef_block_t> struct.

=cut

sub getFlags
{
	my $me = shift;

	my $flags = $me->SUPER::getFlags();
	push(@$flags, 'PFC_CFBF_MAP');

	return $flags;
}

=back

=head1 SEE ALSO

B<PFC::Conf::Block>(3),
B<PFC::Conf::Parameter>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
