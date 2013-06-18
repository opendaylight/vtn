#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Struct field.
##

package PFC::IPC::Field;

use strict;
use vars qw($AUTOLOAD);

use PFC::IPC;
use PFC::IPC::Util;

=head1 NAME

PFC::IPC::Field - Struct field.

=head1 SYNOPSIS

  # Scalar field.
  $type = PFC::IPC::Field->new($type, "field_name");

  # Array field which has 4 elements.
  $type = PFC::IPC::Field->new($type, "field_name", 4);

=head1 ABSTRACT

B<PFC::IPC::Field> instance represents a field of a data structure.

=head1 DESCRIPTION

This section describes about public interface provided by B<PFC::IPC::Field>.

=over 4

=item B<new>($type, $name, [$nelems])

Constructor.

=over 4

=item I<$type>

Instance of B<PFC::IPC::Type> class which represents data type.

=item I<$name>

Name of struct field.

=item I<$nelems>

Number of array elements.
Undefined value means that this field is not array.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($type, $name, $nelems) = @_;

	check_name($name, "Field name");

	my $len = length($type->getDefinition());
	my $me = bless({TYPE => $type, NAME => $name, TYPELEN => $len}, $class);
	my $size = $type->getSize();
	my $num;
	if (defined($nelems)) {
		die "Number of array elements must be greater than zero: " .
			"$name, $nelems\n" unless ($nelems);
		die "Too many array elements: $nelems\n"
			if ($nelems > MAX_STRUCT_SIZE);
		$num = "$nelems";
		my $sz = $size * $num;
		die "Too large field size: $size * $num\n"
			if ($sz > MAX_STRUCT_SIZE);
		$size = $sz;
	}
	else {
		$num = 0;
	}

	$me->{NELEMS} = $num;
	$me->{SIZE} = $size;

	return $me;
}

=item B<getName>()

Return the name of this field.

=cut

sub getName
{
	my $me = shift;

	return $me->{NAME};
}

=item B<getType>()

Return B<PFC::IPC::Type> instance which represents the type of this field.

=cut

sub getType
{
	my $me = shift;

	return $me->{TYPE};
}

=item B<getTypeLength>()

Return string length of C language type.

=cut

sub getTypeLength
{
	my $me = shift;

	return $me->{TYPELEN};
}

=item B<getArraySize>()

Return the number of array elements.
Zero is returned if this field is not an array.

=cut

sub getArraySize
{
	my $me = shift;

	return $me->{NELEMS};
}

=item B<getSize>()

Return size of this field.

=cut

sub getSize
{
	my $me = shift;

	return $me->{SIZE};
}

=item B<getAlignment>()

Return address alignment required by this field.

=item B<getDefinition>()

Return type definition clause for C language.

=cut

# Proxy to PFC::IPC::Type.
sub AUTOLOAD
{
	my $me = shift;

	my $method = $AUTOLOAD;
	$method =~ s,^.*::([^:]+)$,$1,;
	return if ($method eq 'DESTROY');

	my $type = $me->{TYPE};

	return $type->$method(@_);
}

=back

=head1 SEE ALSO

B<PFC::IPC::Type>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
