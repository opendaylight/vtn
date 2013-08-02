#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Define constants related to IPC struct information file.
##

package PFC::IPC::StructInfo;

use strict;
use vars qw(@EXPORT);
use base qw(Exporter);

=head1 NAME

PFC::IPC::StructInfo - Define constants related to IPC struct information file.

=head1 SYNOPSIS

  use PFC::IPC::StructInfo;

=head1 ABSTRACT

The main purpose of this class is to provide constants related to
IPC struct information file.

This package strongly depends on the format of IPC struct information file.
If you want to change the format, this package may need to be modified.

=head1 DESCRIPTION

This section describes about public interface provided by
B<PFC::IPC::StructInfo>.

=over 4

=item B<IPC_STRINFO_MAGIC>

Magic number in IPC struct information file header.

=cut

use constant	IPC_STRINFO_MAGIC	=> 0xac;

=item B<IPC_STRINFO_VERSION>

Current version number of IPC struct information file format.

=cut

use constant	IPC_STRINFO_VERSION	=> 1;

=item B<IPC_STRUCT_SIG_SIZE>

Length of struct signature string.

=cut

use constant	IPC_STRUCT_SIG_SIZE	=> 64;

=item B<IPC_IFHEAD_SIZE>

Size of IPC struct information file header. (ipc_ifhead_t)

=cut

use constant	IPC_IFHEAD_SIZE		=> 32;

=item B<IPC_IFSTRUCT_SIZE>

Size of IPC struct information record. (ipc_ifstruct_t)

=cut

use constant	IPC_IFSTRUCT_SIZE	=> 88;

=item B<IPC_IFFIELD_SIZE>

Size of struct field section record. (ipc_iffield_t)

=cut

use constant	IPC_IFFIELD_SIZE	=> 16;

=item B<IPC_IFFIELD_STRUCT>

Bit for iff_type which represents that iff_type keeps string table offset
to struct name.

=cut

use constant	IPC_IFFIELD_STRUCT	=> 0x80000000;

=back

=head1 SEE ALSO

B<ipctc>(1), B<PFC::IPC::StructInfo::Generator>(3),
B<PFC::IPC::StructInfo::Parser>(3)

=head1 AUTHOR

NEC Corporation

=cut

@EXPORT = qw(IPC_STRINFO_MAGIC IPC_STRINFO_VERSION IPC_STRUCT_SIG_SIZE
	     IPC_IFHEAD_SIZE IPC_IFSTRUCT_SIZE IPC_IFFIELD_SIZE
	     IPC_IFFIELD_STRUCT);

1;
