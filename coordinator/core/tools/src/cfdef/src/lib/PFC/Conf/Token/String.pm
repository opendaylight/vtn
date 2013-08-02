#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Quoted string value.
##

package PFC::Conf::Token::String;

use strict;
use vars qw(%BACKSLASH);
use base qw(PFC::Conf::Token);

use Encode::Guess;

use PFC::Conf::Constants;

%BACKSLASH = ('r' => 1, 'n' => 1, 't' => 1, '\'' => 1, '"' => 1, '\\' => 1);

=head1 NAME

PFC::Conf::Token::String - Value of quoted string token in the cfdef file

=head1 SYNOPSIS

  my $value = PFC::Conf::Token::String->new($value);

=head1 ABSTRACT

An instance of B<PFC::Conf::Token::String> represents a quoted string token
in the cfdef file.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Token::String>.

=head2 METHODS

=over 4

=item I<new>($value)

Constructor.

=over 4

=item $value

A token value.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($value) = @_;

	# Ensure that the given string is valid.
	die "Not a quoted string: $value\n" unless ($value =~ m,^"(.*)"$,s);
	my $body = $1;

	my $s = eval "return $value";
	if ($@) {
		my $err = "$@";
		chomp($err);
		die "Invalid quoted string: $value\n";
	}

	# Ensure that character encoding is valid.
	my $decoder = guess_encoding($value);
	die "Invalid encoding in quoted string.\n" unless (ref($decoder));

	my $me = $class->SUPER::new(TOKEN_STRING, $value);
	$me->{BODY} = $body;

	return bless($me, $class);
}

=item I<checkRange>($min, $max)

Ensure that this value conforms the I<min> and I<max> option.
An exception will be thrown if not.

=over 4

=item $min

Value of I<min> option.
Undefined value must be specified if no I<min> option is specified.

=item $max

Value of I<max> option.
Undefined value must be specified if no I<max> option is specified.

=back

=cut

sub checkRange
{
	my $me = shift;
	my ($min, $max) = @_;

	my $body = $me->{BODY};
	my $len = length($body);

	if (defined($min)) {
		die "String length is less than $min.\n" if ($len < $min);
	}
	if (defined($max)) {
		die "String length is greater than $max.\n" if ($len > $max);
	}
}

=item I<body>()

Return a string eliminated the first and last quotation mark.

=cut

sub body
{
	my $me = shift;

	return $me->{BODY};
}

=item I<concat>($str)

Concatenate this string and the given string.

=over 4

=item $str

String to be concatenated.
It must be a B<PFC::Conf::Token::String> instance.

=back

=cut

sub concat
{
	my $me = shift;
	my ($str) = @_;

	die "concat: PFC::Conf::Token::String must be specified: \"$str\"\n"
		unless (ref($str) eq 'PFC::Conf::Token::String');

	my $body = $me->body() . $str->body();

	$me->{BODY} = $body;
	$me->{VALUE} = '"' . $body . '"';
}

=back

=head1 SEE ALSO

B<PFC::Conf::Token>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
