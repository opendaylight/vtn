#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Integer value
##

package PFC::Conf::Integer;

use strict;
use overload
	'""'		=> \&stringify,
	'<=>'		=> \&compare,
	fallback	=> 1;

use PFC::Conf;

=head1 NAME

PFC::Conf::Integer - Integer value

=head1 SYNOPSIS

  my $value = PFC::Conf::Integer->new($value, $type);

=head1 ABSTRACT

B<PFC::Conf::Integer> is a class which represents integer value.
This class has uses a native library, so that it can represent 64-bit integer
on a perl which is compiled in 32-bit mode.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Integer>.

=head2 METHODS

=over 4

=item I<new>($value, $type)

Constructor.

=over 4

=item $value

A string representation of integer value.

=item $type

Type of integer value.
One of the following type must be specified.
These values are defined in B<PFC::Conf::Constants>.

=over 4

=item B<INT_BYTE>

A byte value (unsigned 8-bit value)

=item B<INT_INT32>

A signed 32-bit value.

=item B<INT_UINT32>

An unsigned 32-bit value.

=item B<INT_INT64>

A signed 64-bit value.

=item B<INT_UINT64>

An unsigned 64-bit value.

=item B<INT_LONG>

A signed long integer value.

=item B<INT_ULONG>

An unsigned long integer value.

=back

An exception will be thrown if the specified value can't represents by the
specified data type. Valid range of B<INT_LONG> and B<INT_ULONG> depends on
the system ILP model. If the system is ILP32 system, a long integer value
is considered as 32-bit integer value. If LP64 system, a long integer value
is considered as 64-bit integer value.

=back

=item I<stringify>()

Return a string representation of this integer value.

=item I<compare>($obj, $reverse)

Emulate "<=>" operator.

=over 4

=item $obj

Value to be compared. It may be integer or B<PFC::Conf::Integer> instance.

=item $reverse

TRUE will be passed if argument order is reversed.

=back

=item I<getHexValue>()

Return a string which represents this value in hexadecimal format.

=item I<getMinHexValue>()

Return the minimum value of this integer type in hexadecimal format.

=item I<getMaxHexValue>()

Return the maximum value of this integer type in hexadecimal format.

=back

=head1 SEE ALSO

B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
