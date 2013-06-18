#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Abstract class for IPC data type.
##

package PFC::IPC::Type;

use strict;
use vars qw(%TYPEATTR %TYPEMAP %TYPE2STR);

use PFC::IPC;

=head1 NAME

PFC::IPC::Type - Abstract class for IPC data type.

=head1 SYNOPSIS

  $type = PFC::IPC::Type->new(IPCTYPE_INT32);

  # Get built-in type.
  $tpye = PFC::IPC::TYPE::get("UINT8");

=head1 ABSTRACT

B<PFC::IPC::Type> is an abstract class for IPC data type.
Subclasses of this class represent characteristics of the IPC data type.

=head1 DESCRIPTION

This section describes about public interface provided by B<PFC::IPC::Type>.

=head2 INSTANCE METHOD

=over 4

=item B<new>($type)

Constructor.

=over 4

=item I<$type>

PDU type identifier defined in B<PFC::IPC>.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($type) = @_;

	my $attr = $TYPEATTR{$type};
	my $me = {TYPE => $type, CDEF => $attr->[0], SIZE => $attr->[1],
		  ALIGN => $attr->[2]};

	return bless($me, $class);
}

=item B<getType>()

Return IPC framework PDU type for this type.

=cut

sub getType
{
	my $me = shift;

	return $me->{TYPE};
}

=item B<getAlignment>()

Return address alignment required by this type.

=cut

sub getAlignment
{
	my $me = shift;

	return $me->{ALIGN};
}

=item B<getSize>()

Return size of this type of data.

=cut

sub getSize
{
	my $me = shift;

	return $me->{SIZE};
}

=item B<getDefinition>()

Return type definition clause for C language.

=cut

sub getDefinition
{
	my $me = shift;

	return $me->{CDEF};
}

=back

=head2 STATIC METHOD

=over 4

=item B<get>($type)

Return B<PFC::IPC::Type> instance which represents built-in type associated
with the type keyword in the IPC structure template file.

Undefined value is returned if unknown type is specified.

=cut

sub get
{
	my ($type) = @_;

	return $TYPEMAP{$type};
}

=item B<getSymbol>($id)

Return a symbol which represents the given IPC PDU type in the IPC struct
template file.

Undefined value is returned if unknown type is specified.

=cut

sub getSymbol
{
	my ($id) = @_;

	return $TYPE2STR{$id};
}

=back

=cut

##
## Define attributes for built-in type.
##
%TYPEATTR = (IPCTYPE_INT8()	=> ['int8_t', 1, 1],	# C-type, size, align
	     IPCTYPE_UINT8()	=> ['uint8_t', 1, 1],
	     IPCTYPE_INT16()	=> ['int16_t', 2, 2],
	     IPCTYPE_UINT16()	=> ['uint16_t', 2, 2],
	     IPCTYPE_INT32()	=> ['int32_t', 4, 4],
	     IPCTYPE_UINT32()	=> ['uint32_t', 4, 4],
	     IPCTYPE_INT64()	=> ['int64_t', 8, 8],
	     IPCTYPE_UINT64()	=> ['uint64_t', 8, 8],
	     IPCTYPE_FLOAT()	=> ['float', 4, 4],
	     IPCTYPE_DOUBLE()	=> ['double', 8, 8],
	     IPCTYPE_IPV4()	=> ['struct in_addr', 4, 4],
	     IPCTYPE_IPV6()	=> ['struct in6_addr', 16, 8]);

%TYPEMAP = (INT8	=> PFC::IPC::Type->new(IPCTYPE_INT8),
	    UINT8	=> PFC::IPC::Type->new(IPCTYPE_UINT8),
	    INT16	=> PFC::IPC::Type->new(IPCTYPE_INT16),
	    UINT16	=> PFC::IPC::Type->new(IPCTYPE_UINT16),
	    INT32	=> PFC::IPC::Type->new(IPCTYPE_INT32),
	    UINT32	=> PFC::IPC::Type->new(IPCTYPE_UINT32),
	    INT64	=> PFC::IPC::Type->new(IPCTYPE_INT64),
	    UINT64	=> PFC::IPC::Type->new(IPCTYPE_UINT64),
	    FLOAT	=> PFC::IPC::Type->new(IPCTYPE_FLOAT),
	    DOUBLE	=> PFC::IPC::Type->new(IPCTYPE_DOUBLE),
	    IPV4	=> PFC::IPC::Type->new(IPCTYPE_IPV4),
	    IPV6	=> PFC::IPC::Type->new(IPCTYPE_IPV6));

{
	while (my ($sym, $type) = each(%TYPEMAP)) {
		my $id = $type->getType();
		$TYPE2STR{$id} = $sym;
	}
}

=head1 SEE ALSO

B<ipctc>(1), B<PFC::IPC>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
