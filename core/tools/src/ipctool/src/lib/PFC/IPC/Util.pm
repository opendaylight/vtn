#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Utilities.
##

package PFC::IPC::Util;

use strict;
use vars qw(@EXPORT);
use base qw(Exporter);

use PFC::IPC;

=head1 NAME

PFC::IPC::Util - Collection of static utility functions.

=head1 SYNOPSIS

  use PFC::IPC::Util;

=head1 ABSTRACT

B<PFC::IPC::Util> provides collection of static utility functions.

=head1 DESCRIPTION

=over 4

=item B<check_name>($name, $type)

Ensure that the given name can be used in C language.

=over 4

=item I<$name>

Name to be tested.

=item I<$type>

A string which represents the type of the given name.
This is used to generate error message.

=back

=cut

sub check_name($$)
{
	my ($name, $type) = @_;

	die "$type must starts with an alphabet: $name\n"
		unless ($name =~ m,^[a-zA-Z],o);
	die "$type must consists of alphabets, digits, and '_': $name\n"
		if ($name =~ m,\W,o);
	die "$type is too long: $name\n"
		if (length($name) > MAX_SYMBOL_LENGTH);
}

=item B<p2roundup>($value, $align)

Round up I<$value> to I<$align>.
I<$align> must be power of 2.

=cut

sub p2roundup($$)
{
	my ($value, $align) = @_;

	use integer;
	return ($value + $align - 1) & (~($align - 1));
}

=item B<is_p2>($value)

Return true if I<$value> is power of 2.

=cut

sub is_p2($)
{
	my ($value) = @_;

	use integer;
	return (($value & ($value - 1)) == 0)
}

=item B<detect_byte_order>()

Determine byte order of the system.

B<IPC_ORDER_LITTLE> is returned if little endian system.
B<IPC_ORDER_BIG> is returned if big endian system.

=cut

sub detect_byte_order
{
	my $bin = pack("n", 0x1234);
	my $num = unpack("S", $bin);

	return IPC_ORDER_LITTLE if ($num == 0x3412);
	return IPC_ORDER_BIG if ($num == 0x1234);

	die "Failed to detect byte order.\n";
}

=back

=head1 AUTHOR

NEC Corporation

=cut

@EXPORT = qw(check_name p2roundup is_p2 detect_byte_order);

1;
