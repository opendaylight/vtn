#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Structure defined by the IPC struct template file.
##

package PFC::IPC::Struct;

use strict;
use base qw(PFC::IPC::Type);

use Digest::SHA;
use PFC::IPC;
use PFC::IPC::Util;

use constant	TAB_WIDTH	=> 8;
use constant	MIN_FTYPELEN	=> 24;
use constant	MAX_FTYPELEN	=> 40;

use constant	CXX_OP_SCOPE	=> '::';

=head1 NAME

PFC::IPC::Struct - Structure defined by the IPC struct template file.

=head1 SYNOPSIS

  my $st = PFC::IPC::Struct->new("my_namespace", "my_struct");

=head1 ABSTRACT

B<PFC::IPC::Struct> represents a struct defined by the IPC struct template
file.

=head1 DESCRIPTION

This section describes about public interface provided by B<PFC::IPC::Struct>.

=over 4

=item B<new>($ns, $name, $file, $lnum)

Constructor.

=over 4

=item I<$ns>

Namespace of this structure.

=item I<$name>

Name of structure.

=item I<$file>

Name of template file which contains the definition of this struct.

=item I<$lnum>

Line number of the template file where this struct is defined.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($ns, $name, $file, $lnum) = (@_);

	check_name($name, "Struct name");

	my $me = $class->SUPER::new(IPCTYPE_STRUCT);

	my $stname = ($ns) ? $ns . '_' . $name : $name;

	$me->{NAME} = $name;
	$me->{STRUCT_NAME} = $stname;
	$me->{CDEF} = $stname . TYPE_SUFFIX;
	$me->{FIELDS} = [];
	$me->{FIELD_MAP} = {};
	$me->{FTYPELEN_MAX} = 0;
	$me->{SIZE} = 0;
	$me->{ALIGN} = 0;
	$me->{SIZE} = 0;
	$me->{TEMPLATE} = $file;
	$me->{LINE_NUMBER} = $lnum;
	$me->{PAD} = 0;
	$me->{DIGEST} = Digest::SHA->new(256);	# Use SHA-256

	return $me;
}

=item B<getName>()

Return the name of this struct, without prepending namespace.

=cut

sub getName
{
	my $me = shift;

	return $me->{NAME};
}

=item B<getStructName>()

Return actual name of this struct.

=cut

sub getStructName
{
	my $me = shift;

	return $me->{STRUCT_NAME};
}

=item B<add>($field)

Add struct field.

=over 4

=item I<$field>

B<PFC::IPC::Field> instance which represents a struct field.

=back

=cut

sub add
{
	my $me = shift;
	my ($field) = @_;

	my $digest = $me->{DIGEST};
	die "Internal Error: Already finalized.\n" unless ($digest);

	my $name = $field->getName();
	my $fmap = $me->{FIELD_MAP};
	die "Field name \"$name\" is duplicated.\n" if ($fmap->{$name});

	$me->update($field);

	$fmap->{$name} = $field;
	my $list = $me->{FIELDS};
	push(@$list, $field);

	die "Too many struct fields.\n" if (scalar(@$list) > MAX_STRUCT_SIZE);

	# Update layout signature.
	my $type = $field->getType();
	my $typeid = $type->getType();
	$digest->add(sprintf("0x%02x", $typeid));

	my $nelems = $field->getArraySize();
	$digest->add(sprintf("array:%u", $nelems));

	if ($typeid == IPCTYPE_STRUCT) {
		# Update digest with struct layout signature.
		my $sig = $type->getSignature();
		$digest->add($sig);
	}
}

=item B<getFieldCount>()

Return the number of fields.

=cut

sub getFieldCount
{
	my $me = shift;

	return scalar(@{$me->{FIELDS}});
}

=item B<getFields>()

Return the list of B<PFC::IPC::Fields> instances.

=cut

sub getFields
{
	my $me = shift;

	my $list = $me->{FIELDS};

	return (wantarray) ? (@$list) : $list;
}

=item B<finalize>()

Finalize struct definition.

=cut

sub finalize
{
	my $me = shift;

	my $digest = $me->{DIGEST};
	die "Internal Error: Already finalized.\n" unless ($digest);

	my $align = $me->getAlignment();
	die "Internal Error: Alignment is not a power of 2: $align\n"
		unless (is_p2($align));

	my $size = $me->getSize();
	$size = p2roundup($size, $align);
	die "Too large struct size: $size\n" if ($size > MAX_STRUCT_SIZE);

	$me->{SIZE} = $size;

	my $max = p2roundup($me->{FTYPELEN_MAX} + 1, TAB_WIDTH);
	if ($max < MIN_FTYPELEN) {
		$max = MIN_FTYPELEN;
	}
	elsif ($max > MAX_FTYPELEN) {
		$max = MAX_FTYPELEN;
	}
	$me->{FTYPELEN_MAX} = $max;

	# Finalize layout signature.
	$me->{SIGNATURE} = $digest->hexdigest();
	delete($me->{DIGEST});
}

=item B<getSignature>()

Return layout signature of this struct.

=cut

sub getSignature
{
	my $me = shift;

	my $sig = $me->{SIGNATURE};
	die "Internal Error: Not yet finalized.\n" unless ($sig);

	return $sig;
}

=item B<getTemplateFile>()

Return the name of the template file where this struct is defined.

=cut

sub getTemplateFile
{
	my $me = shift;

	return $me->{TEMPLATE};
}

=item B<getLineNumber>()

Return the line number of the template file where this struct is defined.

=cut

sub getLineNumber
{
	my $me = shift;

	return $me->{LINE_NUMBER};
}

=item B<dumpHeader>($fh)

Dump C language header file to the given file handle.

=cut

sub dumpHeader
{
	my $me = shift;
	my ($fh) = @_;

	my $name = $me->{NAME};
	my $stname = $me->{STRUCT_NAME};
	my $tname = $me->getDefinition();

	my $size = $me->getSize();
	my $align = $me->getAlignment();
	my $sig = $me->getSignature();
	my $signame = SIGNATURE_PREFIX . $stname;

	my $aligned;
	if ($align > 1) {
		$aligned = " PFC_ATTR_ALIGNED($align)";
	}
	else {
		$aligned = '';
	}

	$fh->print(<<OUT);

/*
 * ipc_struct $name
 *
 *   Size   : $size
 *   Align  : $align
 */
typedef struct $stname {
OUT

	$me->dumpContents($fh);

	$fh->print(<<OUT);
}$aligned $tname;

/* Verify struct size. */
PFC_TYPE_SIZE_ASSERT($tname, $size);

/* Layout signature. */
#define	$signame		\\
	"$sig"

OUT
}

=item B<dumpCxxServerPrototype>($fh)

Dump prototype of B<pfc::core::ServerSession> methods for this struct to
the given file handle.

=cut

sub dumpCxxServerPrototype
{
	my $me = shift;
	my ($fh) = @_;

	my $name = $me->{NAME};
	my $stname = $me->{STRUCT_NAME};
	my $tname = CXX_OP_SCOPE . $me->getDefinition();

	$fh->print(<<OUT);

    //
    // Accessor for ipc_struct $name.
    //
    int addOutput(const $tname &data);
    int getArgument(uint32_t index, $tname &data);
OUT
}

=item B<dumpCxxServerAccessor>($fh)

Dump B<pfc::core::ServerSession> inline methods for this struct to the given
file handle.

=cut

sub dumpCxxServerAccessor
{
	my $me = shift;
	my ($fh) = @_;

	my $name = $me->{NAME};
	my $stname = $me->{STRUCT_NAME};
	my $tname = CXX_OP_SCOPE . $me->getDefinition();

	$fh->print(<<OUT);

//
// Accessor for ipc_struct $name.
//
inline int
ServerSession::addOutput(const $tname &data)
{
    return PFC_IPCSRV_OUTPUT_STRUCT(_srv, $stname, &data);
}

inline int
ServerSession::getArgument(uint32_t index, $tname &data)
{
    return PFC_IPCSRV_GETARG_STRUCT(_srv, index, $stname, &data);
}
OUT
}

=item B<dumpCxxClientPrototype>($fh)

Dump prototype of B<pfc::core::ipc::ClientSession> methods for this struct to
the given file handle.

=cut

sub dumpCxxClientPrototype
{
	my $me = shift;
	my ($fh) = @_;

	my $name = $me->{NAME};
	my $stname = $me->{STRUCT_NAME};
	my $tname = CXX_OP_SCOPE . $me->getDefinition();

	$fh->print(<<OUT);

    //
    // Accessor for ipc_struct $name.
    //
    int addOutput(const $tname &data);
    int getResponse(uint32_t index, $tname &data);
OUT
}

=item B<dumpCxxClientAccessor>($fh)

Dump B<pfc::core::ipc::ClientSession> inline methods for this struct to the
given file handle.

=cut

sub dumpCxxClientAccessor
{
	my $me = shift;
	my ($fh) = @_;

	my $name = $me->{NAME};
	my $stname = $me->{STRUCT_NAME};
	my $tname = CXX_OP_SCOPE . $me->getDefinition();

	$fh->print(<<OUT);

//
// Accessor for ipc_struct $name.
//
inline int
ClientSession::addOutput(const $tname &data)
{
    return PFC_IPCCLNT_OUTPUT_STRUCT(_sess, $stname, &data);
}

inline int
ClientSession::getResponse(uint32_t index, $tname &data)
{
    return PFC_IPCCLNT_GETRES_STRUCT(_sess, index, $stname, &data);
}
OUT
}

# Below methods are inherited from PFC::IPC::Type.

=item B<getType>()

Return IPC framework PDU type for this type.
This method of this class always returns B<IPCTYPE_STRUCT>.

=item B<getAlignment>()

Return address alignment required by this struct.

=item B<getSize>()

Return size of this type of data.

=back

=cut

##
## Below are private methods.
##

sub update
{
	my $me = shift;
	my ($field) = @_;

	my $size = $me->getSize();
	my $align = $me->getAlignment();
	my $tsize = $field->getSize();
	my $talign = $field->getAlignment();

	$size = p2roundup($size, $talign) + $tsize;
	die "Too large struct size: $size\n" if ($size > MAX_STRUCT_SIZE);

	$me->{SIZE} = $size;
	$me->{ALIGN} = $talign if ($talign > $align);

	my $len = $field->getTypeLength();
	$me->{FTYPELEN_MAX} = $len if ($len > $me->{FTYPELEN_MAX});
}

sub dumpContents
{
	my $me = shift;
	my ($fh, $lp64) = @_;

	my $off = 0;
	$me->{PAD} = 1;
	foreach my $field (@{$me->{FIELDS}}) {
		$off = $me->dumpField($fh, $field, $off, $lp64);
	}

	my $size = $me->getSize($lp64);
	my $pad = $size - $off;
	$me->dumpPad($fh, $pad) if ($pad);
}

sub dumpField
{
	my $me = shift;
	my ($fh, $field, $off, $lp64) = @_;

	my $cdef = $field->getDefinition();
	my $nelems = $field->getArraySize();
	my $fname = $field->getName();

	my $size = $field->getSize($lp64);
	my $align = $field->getAlignment($lp64);

	my $roff = p2roundup($off, $align);
	my $pad =  $roff - $off;
	$me->dumpPad($fh, $pad) if ($pad);

	$fh->printf("\t%s%s%s", $cdef, $me->getTabs($cdef), $fname);
	$fh->printf("[%u]", $nelems) if ($nelems);
	$fh->print(";\n");

	return $roff + $size;
}

sub dumpPad
{
	my $me = shift;
	my ($fh, $size) = @_;

	my $index = $me->{PAD};
	my $cdef = 'uint8_t';
	$fh->printf("\t%s%s_pad%u[%u];\n", $cdef, $me->getTabs($cdef),
		    $index, $size);
	$me->{PAD} = $index + 1;
}

sub getTabs
{
	my $me = shift;
	my ($cdef) = @_;

	my $max = $me->{FTYPELEN_MAX};
	my $ntabs;
	my $len = length($cdef);
	if ($len >= $max) {
		$ntabs = 1;
	}
	else {
		my $diff = $max - length($cdef);
		$ntabs = p2roundup($diff, TAB_WIDTH) / TAB_WIDTH;
	}

	return "\t" x $ntabs;
}

=head1 SEE ALSO

B<ipctc>(1), B<PFC::IPC::Field>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
