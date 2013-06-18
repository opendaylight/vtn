#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## IPC struct information file generator.
##

package PFC::IPC::StructInfo::Generator;

use strict;

use POSIX;
use PFC::IPC;
use PFC::IPC::Util;
use PFC::IPC::StructInfo;
use PFC::IPC::StringTable;

=head1 NAME

PFC::IPC::StructInfo::Generator - IPC struct information file generator.

=head1 SYNOPSIS

  my $gen = PFC::IPC::StructInfo::Generator->new();

  # Add PFC::IPC::Struct instance.
  $gen->add($struct);

  $gen->output($fh);

=head1 ABSTRACT

B<PFC::IPC::StructInfo::Generator> generates the contents of IPC struct
information file.

This package strongly depends on the format of IPC struct information file.
If you want to change the format, this package may need to be modified.

=head1 DESCRIPTION

This section describes about public interface provided by
B<PFC::IPC::StructInfo::Generator>.

=over 4

=item B<new>($ns)

Constructor.

=over 4

=item I<$ns>

Namespace of IPC struct information file.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($ns) = @_;

	my $stable = PFC::IPC::StringTable->new();
	my $nsidx = $stable->add($ns);
	my $me = bless({NAMESPACE => $nsidx, STRUCT => "", NSTRUCTS => 0,
			FIELD => "", NFIELDS => 0, STRTABLE => $stable},
		       $class);

	return $me;
}

=item B<add>($struct)

Add a struct to this file.

=over 4

=item I<$struct>

B<PFC::IPC::Struct> instance to be added.

=back

=cut

sub add
{
	my $me = shift;
	my ($struct) = @_;

	# Add struct name to the string table.
	my $stname = $struct->getStructName();
	my $stable = $me->{STRTABLE};
	my $nameidx = $stable->add($stname);

	# Construct ipc_ifstruct_t record.
	my $nfields = $struct->getFieldCount();
	my $size = $struct->getSize();
	my $align = $struct->getAlignment();
	my $sig = $struct->getSignature();
	my $findex = $me->{NFIELDS};

	my $bin = pack("LLLLLLA" . IPC_STRUCT_SIG_SIZE,
		       $nameidx, $nfields, $size, $align, $findex, 0, $sig);
	$me->{STRUCT} .= $bin;
	$me->{NSTRUCTS}++;
	$me->{NFIELDS} += $nfields;

	# Add field records for this struct.
	foreach my $field (@{$struct->getFields()}) {
		$me->addField($field, $stable);
	}
}

=item B<output>($fh)

Output the contents of IPC struct information file to the given file handle.

=over 4

=item I<$fh>

B<FileHandle> instance to be added.

=back

=cut

sub output
{
	my $me = shift;
	my ($fh) = @_;

	# Determine layout of IPC struct information file.
	my $nstructs = $me->{NSTRUCTS};
	my $struct = $me->{STRUCT};
	my $stlen = length($struct);

	die "Internal Error: Invalid struct section size: $stlen\n"
		unless ($stlen == IPC_IFSTRUCT_SIZE * $nstructs);

	my $field = $me->{FIELD};
	my $nfields = $me->{NFIELDS};
	my $fldoff = IPC_IFHEAD_SIZE + $stlen;
	my $fldsize = $nfields * IPC_IFFIELD_SIZE;
	die "Internal Error: Invalid field section size: $fldsize\n"
		unless ($fldsize == length($field));

	my $stable = $me->{STRTABLE};
	my $strings = $stable->getBinary();
	my $stroff = $fldoff + $fldsize;
	my $strsize = length($strings);

	my $total = IPC_IFHEAD_SIZE + $stlen + $fldsize + $strsize;
	die "Too large IPC struct information file: $total\n"
		if ($total >= MAX_INFO_OFFSET);

	# Construct IPC struct information file header.
	my $order = detect_byte_order();
	my $ns = $me->{NAMESPACE};

	my $head = pack("CCCCLLLLLLL",
			IPC_STRINFO_MAGIC,		# ifh_magic
			IPC_STRINFO_VERSION,		# ifh_version
			$order,				# ifh_order
			0,				# ifh_resv1
			0,				# ifh_resv2
			$nstructs,			# ifh_nstructs
			$ns,				# ifh_namespace
			$fldoff,			# ifh_fldoff
			$fldsize,			# ifh_fldsize
			$stroff,			# ifh_stroff
			$strsize);			# ifh_strsize

	# Ensure that the file is empty.
	$fh->flush();
	$fh->truncate(0) or die "Failed to truncate info file: $!\n";
	$fh->seek(0, SEEK_SET) or die "Failed to rewind file handle: $!\n";

	# Write the contents.
	$fh->syswrite($head) or die "Failed to write info file header: $!\n";

	if ($nstructs) {
		$fh->syswrite($struct) or
			die "Failed to write struct records: $!\n";
		$fh->syswrite($field) or
			die "Failed to write field records: $!\n";
	}

	if ($strsize) {
		$fh->syswrite($strings) or
			die "Failed to write string table: $!\n";
	}
}

=back

=cut

##
## Below are private methods.
##

sub addField
{
	my $me = shift;
	my ($field, $stable) = @_;

	# Add field name to the string table.
	my $nameidx = $stable->add($field->getName());

	# Construct ipc_iffield_t record.
	my $nelems = $field->getArraySize();
	my $type = $field->getType();
	my $typeid = $type->getType();
	if ($typeid == IPCTYPE_STRUCT) {
		my $stname = $type->getStructName();
		my $stidx = $stable->get($stname);
		die "Internal Error: struct not found: \"$stname\""
			unless (defined($stidx));
		$typeid = $stidx | IPC_IFFIELD_STRUCT;
	}

	my $bin = pack("LLLL", $nameidx, $nelems, $typeid, 0);
	$me->{FIELD} .= $bin;
}

=head1 SEE ALSO

B<FileHandle>, B<ipctc>(1), B<PFC::IPC::StructInfo>(3),
B<PFC::IPC::Struct>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
