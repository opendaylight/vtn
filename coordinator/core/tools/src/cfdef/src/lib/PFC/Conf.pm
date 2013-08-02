#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Internal class to load native library.
##

package PFC::Conf;

use strict;
use vars qw($VERSION);
use base qw(DynaLoader);

$VERSION = 0.01;

bootstrap PFC::Conf;

=head1 NAME

PFC::Conf - Native library boot loader for B<PFC::Conf> package.

=head1 SYNOPSIS

  use PFC::Conf;

=head1 ABSTRACT

The main purpose of this class is to provide native library interface.
Currently, only one class method is implemented to this class.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf>.

=head2 METHODS

=over 4

=item I<LP64>($lp64)

Configure the target system type.

If true is specified to argument, the target system is considered as LP64
system. If false, the target is considered as ILP32.

If undefined value is specified to argument, this method returns current
configuration without change.

=back

=head1 AUTHOR

NEC Corporation

=cut

1;
