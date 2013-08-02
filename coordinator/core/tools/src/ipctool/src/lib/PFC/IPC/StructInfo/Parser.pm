#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## IPC struct information file parser.
##

package PFC::IPC::StructInfo::Parser;

use strict;
use vars qw(%TEMPLATES);

use FileHandle;
use File::stat;
use POSIX;
use PFC::IPC;
use PFC::IPC::Util;
use PFC::IPC::StructInfo;
use PFC::IPC::StringTable;

{
	# Template to unpack big-endian file.
	my $big = {IFHEAD	=> "CCCCNNNNNNN",
		   IFSTRUCT	=> "NNNNNNA" . IPC_STRUCT_SIG_SIZE,
		   IFFIELD	=> "NNNN"};

	# Template to unpack little-endian file.
	my $little = {IFHEAD	=> "CCCCVVVVVVV",
		      IFSTRUCT	=> "VVVVVVA" . IPC_STRUCT_SIG_SIZE,
		      IFFIELD	=> "VVVV"};

	%TEMPLATES = (IPC_ORDER_BIG()		=> $big,
		      IPC_ORDER_LITTLE()	=> $little);
}

=head1 NAME

PFC::IPC::StructInfo::Parser - IPC struct information file parser.

=head1 SYNOPSIS

  my $parser = PFC::IPC::StructInfo::Parser->new("foo.ipcs");

  my $structs = $parser->getStructs();
  my $fields = $parser->getFields();
  my $string = $parser->getString(10);

=head1 ABSTRACT

B<PFC::IPC::StructInfo::Parser> is IPC structinformation file parser.

This package strongly depends on the format of IPC struct information file.
If you want to change the format, this package may need to be modified.

=head1 DESCRIPTION

This section describes about public interface provided by
B<PFC::IPC::StructInfo::Parser>.

=over 4

=item B<new>($path)

Constructor.

=over 4

=item I<$path>

Path to IPC struct information file.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($path) = @_;

	my $me = bless({PATH => $path}, $class);
	$me->parse($path);

	return $me;
}

=item B<getPath>()

Return path to IPC struct information file.

=cut

sub getPath
{
	my $me = shift;

	return $me->{PATH};
}

=item B<getHeader>()

Return a HASH reference which contains the header of IPC struct information
file. The key is identical to field name of ipc_ifhead_t.

=cut

sub getHeader
{
	my $me = shift;

	return $me->{HEAD};
}

=item B<getStructs>()

Return the list of HASH reference which contains struct information record.
The key of each HASH table is identical to field name of ipc_ifstruct_t.

=cut

sub getStructs
{
	my $me = shift;

	my $structs = $me->{STRUCT};

	return (wantarray) ? @$structs : $structs;
}

=item B<getFields>()

Return the list of HASH reference which contains struct field record.
The key of each HASH table is identical to field name of ipc_iffield_t.

=cut

sub getFields
{
	my $me = shift;

	my $fields = $me->{FIELD};

	return (wantarray) ? @$fields : $fields;
}

=item B<getFieldAt>($index)

Return HASH reference which contains struct field record at the given
index.

Undefined value is returned if not found.

=cut

sub getFieldAt
{
	my $me = shift;
	my ($index) = @_;

	my $fields = $me->{FIELD};

	return $fields->[$index];
}

=item B<getString>($index)

Return a string associated with the given index in the string table section.

Undefined value is returned if not found.

=cut

sub getString
{
	my $me = shift;
	my ($index) = @_;

	my $stable = $me->{STRTABLE};

	return $stable->getAt($index);
}

=item B<getStringKeys>()

Return the list of string table indices, in ascending order.

=cut

sub getStringKeys()
{
	my $me = shift;

	my $stable = $me->{STRTABLE};
	my $keys = $stable->getKeys();

	return (wantarray) ? @$keys : $keys;
}

=item B<getModifiedTime>()

Return the last modification time of the IPC struct information file.

Undefined value is returned if the file is not yet parsed.

=cut

sub getModifiedTime
{
	my $me = shift;

	return $me->{MTIME};
}

=back

=cut

##
## Below are private methods.
##

sub parse
{
	my $me = shift;
	my ($path) = @_;

	my $fh = FileHandle->new($path) or die "open($path) failed: $!\n";
	my $st = stat($fh) or die "stat($path) failed: $!\n";
	$me->{MTIME} = $st->mtime();

	# Read IPC struct information file header.
	my $bin = $me->readFile($fh, IPC_IFHEAD_SIZE);

	# Unpack the first 3 octets.
	my $tmpl;

	{
		my ($magic, $version, $order) = unpack("CCC", $bin);
		unless ($magic == IPC_STRINFO_MAGIC) {
			my $bad = sprintf("0x%02x", $magic);
			die "Bad header magic: $bad\n";
		}
		die "Unknown format version: $version\n"
			unless ($version == IPC_STRINFO_VERSION);

		$tmpl = $TEMPLATES{$order};
		die "Unknown byte order: $order\n" unless ($tmpl);
	}

	# Unpack the header.
	my ($magic, $version, $order, $resv1, $resv2, $nstructs, $ns,
	    $fldoff, $fldsize, $stroff, $strsize) =
		    unpack($tmpl->{IFHEAD}, $bin);
	my (%head) = (ifh_magic		=> $magic,
		      ifh_version	=> $version,
		      ifh_order		=> $order,
		      ifh_resv1		=> $resv1,
		      ifh_resv2		=> $resv2,
		      ifh_nstructs	=> $nstructs,
		      ifh_namespace	=> $ns,
		      ifh_fldoff	=> $fldoff,
		      ifh_fldsize	=> $fldsize,
		      ifh_stroff	=> $stroff,
		      ifh_strsize	=> $strsize);
	$me->{HEAD} = \%head;

	# Parse struct section.
	$me->parseStruct($fh, $tmpl, $nstructs);

	# Parse field section.
	die "Unexpected field section size: $fldsize\n"
		unless (($fldsize % IPC_IFFIELD_SIZE) == 0);
	$fh->seek($fldoff, SEEK_SET) or
		die "Failed to seek field section: $!\n";
	$me->parseField($fh, $tmpl, $fldsize / IPC_IFFIELD_SIZE);

	# Parse string table section.
	$fh->seek($stroff, SEEK_SET) or
		die "Failed to seek string table section: $!\n";
	$bin = $me->readFile($fh, $strsize);

	$me->{STRTABLE} = PFC::IPC::StringTable->new($bin);
}

sub parseStruct
{
	my $me = shift;
	my ($fh, $tmpl, $nstructs) = @_;

	my @list;
	for (my $i = 0; $i < $nstructs; $i++) {
		# Read one struct section record.
		my $bin = $me->readFile($fh, IPC_IFSTRUCT_SIZE);

		# Unpack ipc_ifstruct_t.
		my ($name, $nfields, $size, $align, $field, $resv, $sig) =
			unpack($tmpl->{IFSTRUCT}, $bin);

		my (%struct) = (ifs_name	=> $name,
				ifs_nfields	=> $nfields,
				ifs_size	=> $size,
				ifs_align	=> $align,
				ifs_field	=> $field,
				ifs_resv	=> $resv,
				ifs_sig		=> $sig);
		push(@list, \%struct);
	}

	$me->{STRUCT} = \@list;
}

sub parseField
{
	my $me = shift;
	my ($fh, $tmpl, $nfields) = @_;

	my @list;
	for (my $i = 0; $i < $nfields; $i++) {
		# Read one field section record.
		my $bin = $me->readFile($fh, IPC_IFFIELD_SIZE);

		# Unpack ipc_iffield_t.
		my ($name, $array, $type, $resv) =
			unpack($tmpl->{IFFIELD}, $bin);

		my (%field) = (iff_name		=> $name,
			       iff_array	=> $array,
			       iff_type		=> $type,
			       iff_resv		=> $resv);
		push(@list, \%field);
	}

	$me->{FIELD} = \@list;
}

sub readFile
{
	my $me = shift;
	my ($fh, $len) = @_;

	my $bin;
	my $nbytes = $fh->sysread($bin, $len);
	die "read() failed: $!\n" unless (defined($nbytes));
	die "Unexpected EOF: $nbytes: required=$len\n" unless ($nbytes == $len);

	return $bin;
}

=head1 SEE ALSO

B<ipctc>(1),
B<PFC::IPC::StructInfo>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
