#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Define constants for IPC build tool.
##

package PFC::IPC;

use strict;
use vars qw($VERSION @EXPORT);
use base qw(Exporter);

$VERSION = 0.01;

=head1 NAME

PFC::IPC - Define constants for IPC build tool.

=head1 SYNOPSIS

  use PFC::IPC;

=head1 ABSTRACT

The main purpose of this class is to provide constants for IPC build tool.

=head1 DESCRIPTION

This section describes about public interface provided by B<PFC::IPC>.

=head2 IPC PDU TYPE

=over 4

=item B<IPCTYPE_INT8>

IPC data PDU type for signed 8-bit integer.
This value is identical to B<PFC_IPCTYPE_INT8> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_INT8		=> 0;

=item B<IPCTYPE_UINT8>

IPC data PDU type for unsigned 8-bit integer.
This value is identical to B<PFC_IPCTYPE_UINT8> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_UINT8		=> 1;

=item B<IPCTYPE_INT16>

IPC data PDU type for signed 16-bit integer.
This value is identical to B<PFC_IPCTYPE_INT16> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_INT16		=> 2;

=item B<IPCTYPE_UINT16>

IPC data PDU type for unsigned 16-bit integer.
This value is identical to B<PFC_IPCTYPE_UINT16> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_UINT16		=> 3;

=item B<IPCTYPE_INT32>

IPC data PDU type for signed 32-bit integer.
This value is identical to B<PFC_IPCTYPE_INT32> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_INT32		=> 4;

=item B<IPCTYPE_UINT32>

IPC data PDU type for unsigned 32-bit integer.
This value is identical to B<PFC_IPCTYPE_UINT32> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_UINT32		=> 5;

=item B<IPCTYPE_INT64>

IPC data PDU type for signed 64-bit integer.
This value is identical to B<PFC_IPCTYPE_INT64> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_INT64		=> 6;

=item B<IPCTYPE_UINT64>

IPC data PDU type for unsigned 64-bit integer.
This value is identical to B<PFC_IPCTYPE_UINT64> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_UINT64		=> 7;

=item B<IPCTYPE_FLOAT>

IPC data PDU type for single precision floating point.
This value is identical to B<PFC_IPCTYPE_FLOAT> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_FLOAT		=> 8;

=item B<IPCTYPE_DOUBLE>

IPC data PDU type for double precision floating point.
This value is identical to B<PFC_IPCTYPE_DOUBLE> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_DOUBLE		=> 9;

=item B<IPCTYPE_IPV4>

IPC data PDU type for IPv4 address.
This value is identical to B<PFC_IPCTYPE_IPV4> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_IPV4		=> 10;

=item B<IPCTYPE_IPV6>

IPC data PDU type for IPv6 address.
This value is identical to B<PFC_IPCTYPE_IPV6> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_IPV6		=> 11;

=item B<IPCTYPE_STRUCT>

IPC data PDU type for user-defined structure.
This value is identical to B<PFC_IPCTYPE_STRUCT> defined in B<pfc/ipc.h>.

=cut

use constant	IPCTYPE_STRUCT		=> 0xff;

=back

=head2 FILE NAME

=over 4

=item B<INFO_DEFAULT>

Default file name to place IPC struct information.

=cut

use constant	INFO_DEFAULT		=> 'ipc_struct.bin';

=back

=head2 BYTE ORDER MARK

=over 4

=item B<IPC_ORDER_LITTLE>

Byte order mark which represents little endian.

=cut

use constant	IPC_ORDER_LITTLE	=> 1;

=item B<IPC_ORDER_BIG>

Byte order mark which represents big endian.

=cut

use constant	IPC_ORDER_BIG		=> 2;

=back

=head2 OTHERS

=over 4

=item B<MAX_STRUCT_SIZE>

The limit of structure size.

=cut

use constant	MAX_STRUCT_SIZE		=> 0x100000;	# 1 megabytes

=item B<MAX_SYMBOL_LENGTH>

The limit of symbol name, such as struct and field name.

=cut

use constant	MAX_SYMBOL_LENGTH	=> 60;

=item B<MAX_INFO_OFFSET>

The limit of file offset of IPC struct information file.

=cut

use constant	MAX_INFO_OFFSET		=> 0x80000000;	# 2 gigabytes

=item B<TYPE_SUFFIX>

Suffix of C language type for struct data.

=cut

use constant	TYPE_SUFFIX		=> '_t';

=item B<SIGNATURE_PREFIX>

Prefix of C preprocessor constant name which defines struct signature.

=cut

use constant	SIGNATURE_PREFIX	=> '__PFC_IPCTMPL_SIG_';

=item B<CXX_IMPORT_PREFIX>

Prefix of C preprocessor constant name used to reject unexpected inclusion
of CXX accessor methods.

=cut

use constant	CXX_IMPORT_PREFIX	=> '_PFCXX_IPC_IMPORT_STRUCT_';

=back

=head1 AUTHOR

NEC Corporation

=cut

@EXPORT = qw(IPCTYPE_INT8 IPCTYPE_UINT8 IPCTYPE_INT16 IPCTYPE_UINT16
	     IPCTYPE_INT32 IPCTYPE_UINT32 IPCTYPE_INT64 IPCTYPE_UINT64
	     IPCTYPE_FLOAT IPCTYPE_DOUBLE IPCTYPE_IPV4 IPCTYPE_IPV6
	     IPCTYPE_STRING IPCTYPE_STRUCT
	     HEADER_DEFAULT INFO_DEFAULT CXX_SERVER_DEFAULT CXX_CLIENT_DEFAULT
	     IPC_ORDER_LITTLE IPC_ORDER_BIG
	     MAX_STRUCT_SIZE MAX_SYMBOL_LENGTH MAX_INFO_OFFSET TYPE_SUFFIX
	     SIGNATURE_PREFIX CXX_IMPORT_PREFIX);

1;
