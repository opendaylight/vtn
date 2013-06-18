#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Parser constants.
##

package PFC::Conf::Constants;

use strict;
use vars qw(@EXPORT);
use base qw(Exporter);

=head1 NAME

PFC::Conf::Constants - Parser constants

=head1 ABSTRACT

B<PFC::Conf::Constants> defines constants used by the cfdef file parser.

=head1 DESCRIPTION

This section describes about public constants for B<PFC::Core::Constants>.

=head2 TOKEN TYPE

=over 4

=item B<TOKEN_SEMI>

Token type which represents a semicolon. (;)

=cut

use constant	TOKEN_SEMI	=> 1;

=item B<TOKEN_LBRACE>

Token type which represents a left brace. ({)

=cut

use constant	TOKEN_LBRACE	=> 2;

=item B<TOKEN_RBRACE>

Token type which represents a right brace. (})

=cut

use constant	TOKEN_RBRACE	=> 3;

=item B<TOKEN_SQLEFT>

Token type which represents a left square bracket. ([)

=cut

use constant	TOKEN_SQLEFT	=> 4;

=item B<TOKEN_SQRIGHT>

Token type which represents a right square bracket. (])

=cut

use constant	TOKEN_SQRIGHT	=> 5;

=item B<TOKEN_EQUAL>

Token type which represents an equal. (=)

=cut

use constant	TOKEN_EQUAL	=> 6;

=item B<TOKEN_COMMA>

Token type which represents a comma. (,)

=cut

use constant	TOKEN_COMMA	=> 7;

=item B<TOKEN_COLON>

Token type which represents a colon. (:)

=cut

use constant	TOKEN_COLON	=> 8;

=item B<TOKEN_SYMBOL>

Token type which represents a string which represents a symbol.

Symbol must start with an alphabetical character or an underscore character
('_'), and consist of alphabet, digit, underscore character.
In addition, its length must be less than 64.

=cut

use constant	TOKEN_SYMBOL	=> 9;

=item B<TOKEN_INT>

Token type which represents an integer value.

If a token starts with a minus sign character ('-') or a digit, it will be
considered as INT token. Returned value is a B<PFC::Core::Value::Integer>
instance.

This token type may have 64-bit integer value.
The actual size check must be done by an application.

=cut

use constant	TOKEN_INT	=> 10;

=item B<TOKEN_STRING>

Token type which represents a string value.

If a token starts with a double quotation mark ('"'), it will be considered
as STRING token. Return value is a B<PFC::Core::Value::String> instance.

=cut

use constant	TOKEN_STRING	=> 11;

=back

=head2 INTEGER TYPE

=over 4

=item B<INT_BYTE>

Integer type of BYTE. (unsigned 8-bit)

=cut

use constant	INT_BYTE	=> 0;

=item B<INT_INT32>

Integer type of signed 32-bit value.

=cut

use constant	INT_INT32	=> 1;

=item B<INT_UINT32>

Integer type of unsigned 32-bit value.

=cut

use constant	INT_UINT32	=> 2;

=item B<INT_INT64>

Integer type of signed 64-bit value.

=cut

use constant	INT_INT64	=> 3;

=item B<INT_UINT64>

Integer type of unsigned 64-bit value.

=cut

use constant	INT_UINT64	=> 4;

=item B<INT_LONG>

Integer type of signed long integer value.

=cut

use constant	INT_LONG	=> 5;

=item B<INT_ULONG>

Integer type of unsigned long integer value.

=cut

use constant	INT_ULONG	=> 6;

=back

=head2 PARAMETER TYPE

String constants which represents parameter type.
These values can be converted to I<pfc_cftype_t> symbol by appending
"PFC_CFTYPE_" as prefix.

=over 4

=item B<TYPE_BOOL>

Boolean type name in the cfdef file.

=cut

use constant	TYPE_BOOL	=> 'BOOL';

=item B<TYPE_BYTE>

Byte type name in the cfdef file.

=cut

use constant	TYPE_BYTE	=> 'BYTE';

=item B<TYPE_INT32>

Signed 32-bit integer type name in the cfdef file.

=cut

use constant	TYPE_INT32	=> 'INT32';

=item B<TYPE_UINT32>

Unsigned 32-bit integer type name in the cfdef file.

=cut

use constant	TYPE_UINT32	=> 'UINT32';

=item B<TYPE_INT64>

Signed 64-bit integer type name in the cfdef file.

=cut

use constant	TYPE_INT64	=> 'INT64';

=item B<TYPE_UINT64>

Unsigned 64-bit integer type name in the cfdef file.

=cut

use constant	TYPE_UINT64	=> 'UINT64';

=item B<TYPE_LONG>

Signed long integer type name in the cfdef file.

=cut

use constant	TYPE_LONG	=> 'LONG';

=item B<TYPE_ULONG>

Unsigned long integer type name in the cfdef file.

=cut

use constant	TYPE_ULONG	=> 'ULONG';

=item B<TYPE_STRING>

String type name in the cfdef file.

=cut

use constant	TYPE_STRING	=> 'STRING';

=back

=head2 PARAMETER OPTIONS

=over 4

=item B<OPT_MIN>

Parameter option which defines the minimum value.

=cut

use constant	OPT_MIN		=> 'min';

=item B<OPT_MAX>

Parameter option which defines the maximum value.

=cut

use constant	OPT_MAX		=> 'max';

=item B<OPT_MANDATORY>

Parameter option which declares the parameter is mandatory.

=cut

use constant	OPT_MANDATORY	=> 'mandatory';

=back

=head2 INTERNAL LIMITS

=over 4

=item B<MAX_STRING_LENGTH>

Internal limit of string length.

=cut

use constant	MAX_STRING_LENGTH	=> 1023;

=item B<MAX_ARRAY_LENGTH>

Internal limits of array length.

=cut

use constant	MAX_ARRAY_LENGTH	=> 256;

=back

=head2 SYMBOL VISIBILITY

=over 4

=item B<VIS_DEFAULT>

Default visibility.

=cut

use constant	VIS_DEFAULT		=> 1;

=item B<VIS_HIDDEN>

Hidden visibility.

=cut

use constant	VIS_HIDDEN		=> 2;

=back

=head2 MISCELLANEOUS

=over 4

=item B<SYMBOL_MAXLEN>

Maximum length of symbol. (inclusive)

=cut

use constant	SYMBOL_MAXLEN	=> 63;

=back

=cut

@EXPORT = qw(TOKEN_SEMI TOKEN_LBRACE TOKEN_RBRACE TOKEN_SQLEFT TOKEN_SQRIGHT
	     TOKEN_EQUAL TOKEN_COMMA TOKEN_COLON TOKEN_SYMBOL TOKEN_INT
	     TOKEN_STRING
	     INT_BYTE INT_INT32 INT_UINT32 INT_INT64 INT_UINT64
	     INT_LONG INT_ULONG
	     TYPE_BOOL TYPE_BYTE TYPE_INT32 TYPE_UINT32 TYPE_INT64 TYPE_UINT64
	     TYPE_LONG TYPE_ULONG TYPE_STRING
	     OPT_MIN OPT_MAX OPT_MANDATORY
	     MAX_STRING_LENGTH MAX_ARRAY_LENGTH
	     VIS_DEFAULT VIS_HIDDEN
	     SYMBOL_MAXLEN);


=head1 SEE ALSO

B<PFC::Conf::Lexer>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
