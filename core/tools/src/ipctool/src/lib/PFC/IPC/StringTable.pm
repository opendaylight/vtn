#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## String table in IPC struct information file.
##

package PFC::IPC::StringTable;

use strict;

use POSIX;
use PFC::IPC::StructInfo;

=head1 NAME

PFC::IPC::StringTable - String table in IPC struct information file.

=head1 SYNOPSIS

  # Construct string table.
  my $table = PFC::IPC::StringTable->new();
  my $index1 = $table->add("string_1");
  my $index2 = $table->add("string_2");
  my $binary = $table->getBinary();

  # Read string table.
  my $table = PFC::IPC::StringTable->new($binary);
  my $string = $table->getAt(10);
  my $index = $table->get("string_1");

=head1 ABSTRACT

B<PFC::IPC::StringTable> is a utility class to generate String Table Section
for IPC struct information file. In addition, it has ability to decode
String Table Section.

=head1 DESCRIPTION

This section describes about public interface provided by
B<PFC::IPC::StringTable>.

=over 4

=item B<new>([$binary])

Constructor.

=over 4

=item I<$binary>

Binary image of String Table Section.
This parameter is used to create initial value of string table.
If omitted, a new instance will represent an empty String Table Section.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($binary) = (@_);

	my $me = bless({STRMAP => {}, IDXMAP => {}, BINARY => "", LENGTH => 0},
		       $class);

	if (defined($binary)) {
		$me->parse($binary);
	}
	else {
		# First entry must be an empty string.
		$me->add("");
	}

	return $me;
}

=item B<add>($string)

Add the given string to the string table.

This method returns an index in the string table assigned to the given string.

=cut

sub add
{
	my $me = shift;
	my ($string) = @_;

	my $strmap = $me->{STRMAP};
	my $index = $strmap->{$string};
	unless (defined($index)) {
		# Add a new string.
		$index = $me->{LENGTH};
		my $binary = $me->{BINARY};
		my $bin = pack("A*", $string) . pack("C", 0);
		my $length = $index + length($bin);
		die "Too many strings. Please reduce struct definitions.\n"
			if ($length >= IPC_IFFIELD_STRUCT);

		$me->{LENGTH} = $length;
		$me->{BINARY} = $binary . $bin;
		$strmap->{$string} = $index;
		$me->{IDXMAP}->{$index} = $string;
	}

	return $index;
}

=item B<get>($string)

Return a string table index associated of the given string.
Undefined value is returned if not found.

=cut

sub get
{
	my $me = shift;
	my ($string) = @_;

	return $me->{STRMAP}->{$string};
}

=item B<getAt>($index)

Return a string associated with the given index.
Undefined value is returned if not found.

=cut

sub getAt
{
	my $me = shift;
	my ($index) = @_;

	return $me->{IDXMAP}->{$index};
}

=item B<getBinary>()

Return binary image of the string table.

=cut

sub getBinary()
{
	my $me = shift;

	return $me->{BINARY};
}

=item B<getKeys>()

Return the list of string table indices, in ascending order.

=cut

sub getKeys()
{
	my $me = shift;

	my (@keys) = sort {$a <=> $b} (keys(%{$me->{IDXMAP}}));

	return (wantarray) ? @keys : \@keys;
}

=back

=cut

##
## Below are private methods.
##

sub parse
{
	my $me = shift;
	my ($binary) = @_;

	my $str = "";
	my $len = length($binary);
	my $strmap = $me->{STRMAP};
	my $idxmap = $me->{IDXMAP};
	my $index = 0;
	for (my $i = 0; $i < $len; $i++) {
		my $c = unpack("A", substr($binary, $i, 1));

		if ($c ne "") {
			$str .= $c;
			next;
		}

		$strmap->{$str} = $index;
		$idxmap->{$index} = $str;
		$str = "";
		$index = $i + 1;
	}

	$me->{BINARY} = $binary;
	$me->{LENGTH} = $len;
}

=head1 AUTHOR

NEC Corporation

=cut

1;

