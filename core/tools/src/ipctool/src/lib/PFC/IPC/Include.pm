#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Template file inclusion stack.
##

package PFC::IPC::Include;

use strict;

use FileHandle;

=head1 NAME

PFC::IPC::Include - Inclusion stack of IPC struct template file.

=head1 SYNOPSIS

  my $inc = PFC::IPC::Include->new();

  # Included from foo.ipct at line 10.
  $inc->pushStack('foo.ipct', 10);

  # Pop one entry from inclusion stack.
  $inc->popStack();

=head1 ABSTRACT

B<PFC::IPC::Include> is a template file inclusion stack.

An instance of B<PFC::IPC::Include> is created when a template parser
detects B<include> directive to import another template file.
B<PFC::IPC::Include> is used to detect template file inclusion loop.

=head1 DESCRIPTION

This section describes about public interface provided by B<PFC::IPC::Include>.

=over 4

=item B<new>($file, $lnum)

Constructor.

=over 4

=item I<$file>

Name of template file which B<include> directive exists.

=item I<$lnum>

Line number where B<include> directive exists in I<$file>.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($file, $lnum) = @_;

	my (%ent) = (FILE => $file, LNUM => $lnum);
	my %fmap = ($file => \%ent);

	return bless({STACK => [\%ent], FILEMAP => \%fmap}, $class);
}

=item B<pushStack>($file, $lnum)

Push one entry to the inclusion stack.

=over 4

=item I<$file>

Name of template file which B<include> directive exists.

=item I<$lnum>

Line number where B<include> directive exists in I<$file>.

=back

=cut

sub pushStack
{
	my $me = shift;
	my ($file, $lnum) = @_;

	my $fmap = $me->{FILEMAP};
	die "Internal Error: Duplicated inclusion stack entry: $file\n"
		if ($fmap->{$file});

	my (%ent) = (FILE => $file, LNUM => $lnum);
	$fmap->{$file} = \%ent;
	unshift(@{$me->{STACK}}, \%ent);
}

=item B<popStack>()

Pop an entry from the inclusion stack.

=cut

sub popStack
{
	my $me = shift;

	my $ent = shift(@{$me->{STACK}});
	my $fmap = $me->{FILEMAP};
	delete($fmap->{$ent->{FILE}}) if ($ent);
}

=item B<canPush>($file)

Determine whether the file specified by I<$file> can be pushed into the
inclusion stack.

=cut

sub canPush
{
	my $me = shift;
	my ($file) = @_;

	my $fmap = $me->{FILEMAP};

	return (!exists($fmap->{$file}));
}

=item B<printStack>($fh)

Print include stack to the file handle specified by I<$fh>.

If undefined value is specified to I<$fh>, STDERR is used.

=cut

sub printStack
{
	my $me = shift;
	my ($fh) = @_;

	my $stack = $me->{STACK};
	return unless (@$stack);

	$fh = \*STDERR unless ($fh);
	my $msg = 'In file included';
	my $len = length($msg);
	foreach my $ent (@$stack) {
		$fh->printf("*** %*s from %s:%u\n", -$len, $msg,
			    $ent->{FILE}, $ent->{LNUM});
		$msg = '';
	}
}

=back

=head1 SEE ALSO

B<ipctc>(1),
B<PFC::IPC::Parser>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
