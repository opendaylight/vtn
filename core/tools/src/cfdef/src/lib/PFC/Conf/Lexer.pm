#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Lexical analyzer of the configuration definition file.
##

package PFC::Conf::Lexer;

use strict;
use vars qw(%TOKEN_CHAR);
use FileHandle;
use POSIX;

use PFC::Conf;
use PFC::Conf::Constants;
use PFC::Conf::DepFile;
use PFC::Conf::Process;
use PFC::Conf::Token;
use PFC::Conf::Token::Integer;
use PFC::Conf::Token::String;

%TOKEN_CHAR = (';'		=> TOKEN_SEMI,
	       '{'		=> TOKEN_LBRACE,
	       '}'		=> TOKEN_RBRACE,
	       '['		=> TOKEN_SQLEFT,
	       ']'		=> TOKEN_SQRIGHT,
	       '='		=> TOKEN_EQUAL,
	       ','		=> TOKEN_COMMA,
	       ':'		=> TOKEN_COLON);

=head1 NAME

PFC::Conf::Lexer - Lexer for the cfdef file

=head1 SYNOPSIS

  my $lexer = PFC::Conf::Lexer->new("config.cfdef");

=head1 ABSTRACT

B<PFC::Conf::Lexer> is a lexical analyzer for the configuration definition
file. It splits the contents of the cfdef file into tokens.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Lexer>.

=head2 METHODS

=over 4

=item I<new>($file, %args)

Constructor.

=over 4

=item $file

Path to the cfdef file.

=item %args

HASH which represents optional arguments.
The following keys are recognized.

=over 4

=item CPP

Path to C preprocessor.
If this key is specified, this class analyzes definitions processed by
C preprocessor.

=item CPPFLAGS

ARRAY reference which contains options for C preprocessor.

=item DEPFILE

Path to dependency file to store header file dependency.

=item OUTFILE

Path to output file to write C language source code.

=back

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($file, %args) = @_;

	my $me = bless({PATH => $file, UNGET => [], UNDO => []}, $class);
	$me->openHandle($file, \%args);

	return $me;
}

=item I<nextToken>([$needed])

Return B<PFC::Conf::Value> or its subclass which represents a token
in the cfdef file.

Undefined value is returned if it detects EOF unless $needed is true.

=over 4

=item $needed

If true is specified, I<nextToken> raises exception if it detects EOF.

=back

=cut

sub nextToken
{
	my $me = shift;
	my ($needed) = @_;

	my $undo = pop(@{$me->{UNDO}});
	return $undo if (defined($undo));

	my ($skip, $c);
	while (defined($c = $me->getChar())) {
		# Skip newline.
		if ($c eq "\n") {
			undef $skip;
			next;
		}

		# Skip whitespace.
		next if ($c =~ /^\s$/o);

		# Skip comment.
		if ($c eq '#' or $c eq '%') {
			$skip = 1;
			next;
		}
		last unless ($skip);
	}

	unless (defined($c)) {
		$me->syntaxError("Unexpected EOF.") if ($needed);
		return undef;
	}

	return $me->tokenize($c);
}

=item I<undo>($token)

Undo I<nextToken>.
The given value will be returned by the subsequent I<nextToken> call.

=over 4

=item $token

An instance of B<PFC::Conf::Value> or its subclass.

=back

=cut

sub undo
{
	my $me = shift;
	my ($token) = @_;

	push(@{$me->{UNDO}}, $token);
}

=item I<error>($message, ...)

Raise an error with the given message.

=cut

sub error
{
	my $me = shift;

	my $line = $me->lineNumber();
	my $path = $me->getPath();
	my $err = join("", @_);

	die "$path: $line: $err\n";
}

=item I<syntaxError>($message, ...)

Raise a syntax error exception with the given message.

=cut

sub syntaxError
{
	my $me = shift;

	$me->error('Syntax Error: ', @_);
}

=item I<getPath>()

Return path to the cfdef file.

=cut

sub getPath
{
	my $me = shift;

	return $me->{PATH};
}

=item I<lineNumber>()

Return file line number for the cfdef file currently processed by lexer.

=cut

sub lineNumber
{
	my $me = shift;

	my $fh = $me->{HANDLE};

	return $fh->input_line_number();
}

=back

=cut

##
## Below are private methods.
##

sub openHandle
{
	my $me = shift;
	my ($file, $args) = @_;

	my $cpp = $args->{CPP};
	my $fh = FileHandle->new($file) or die "open($file) failed: $!\n";
	unless (defined($cpp)) {
		$fh->input_line_number(1);
		$me->{HANDLE} = $fh;

		return;
	}

	my $dep;
	my ($cppflags, $outfile, $depfile) =
		($args->{CPPFLAGS}, $args->{OUTFILE}, $args->{DEPFILE});

	if ($outfile and $depfile) {
		$dep = PFC::Conf::DepFile->new($file, $outfile);
	}

	# Create a filter process, which filters out comments.
	my $fproc = PFC::Conf::Process->new(NAME => 'filter', OUT => undef);
	my $filter = sub {
		$me->filter($fh, $file);
		exit 0;
	};
	$fproc->start($filter);

	# Execute cpp.
	my $cproc = PFC::Conf::Process->new(NAME => 'cpp', OUT => undef,
					    IN => $fproc->getOutHandle());
	my (@a) = ($cpp);
	push(@a, @$cppflags) if (defined($cppflags));
	my $func = sub { exec(@a, '-'); };
	$cproc->start($func);

	$me->{HANDLE} = $cproc->getOutHandle();
	$me->{PROC} = [$fproc, $cproc];

	if ($dep) {
		$me->{DEP} = $dep;
		$me->{DEPFILE} = $depfile;
	}
}

sub getChar
{
	my $me = shift;

	# At first, try to pop a character from the stack.
	my $unget = $me->{UNGET};
	return $me->updateLine(pop(@$unget)) if (@$unget);

	my $fh = $me->{HANDLE};

	# Read a character from the source file if C preprocessor is not used.
	my $proc = $me->{PROC};
	return $me->updateLine($fh->getc()) unless ($proc);

	my ($line, $ln);
	my $path = $me->getPath();
	my $dep = $me->{DEP};

	# If we use C preprocessor, we need to grab output line to detect
	# cpp's control message.
	while (defined($line = $fh->getline())) {
		if ($line =~ /^#\s+(\d+)\s+"(.+)".*$/o) {
			my $p = $2;

			if ($p eq $path) {
				$ln = $1;
			}
			elsif ($dep) {
				# Add this file to dependency list.
				$dep->add($p);
			}
		}
		elsif ($line !~ /^#/) {
			last;
		}
	}

	if (defined($ln)) {
		# Use line number reported by C preprocessor.
		$fh->input_line_number($ln);
	}
	else {
		# Revert line number. It will be bumped up when a newline
		# character is popped from the stack.
		$ln = $fh->input_line_number();
		$fh->input_line_number($ln - 1);
	}

	unless (defined($line)) {
		# Check exit status of sub processes.
		foreach my $p (@$proc) {
			$p->wait(FATAL => 1);
		}

		# Write dependency file.
		$dep->output($me->{DEPFILE}) if ($dep);

		return undef;
	}

	(@$unget) = (reverse(split(//, $line)));

	return $me->updateLine(pop(@$unget));
}

sub ungetChar
{
	my $me = shift;
	my ($c) = (@_);

	if ($c eq "\n") {
		my $fh = $me->{HANDLE};
		my $line = $fh->input_line_number();
		$fh->input_line_number($line - 1);
	}

	push(@{$me->{UNGET}}, $c);
}

sub updateLine
{
	my $me = shift;
	my ($c) = @_;

	if ($c eq "\n") {
		my $fh = $me->{HANDLE};
		my $ln = $fh->input_line_number();
		$fh->input_line_number($ln + 1);
	}

	return $c;
}

sub tokenize
{
	my $me = shift;
	my ($c) = @_;

	my $token;
	my $type = $TOKEN_CHAR{$c};
	if (defined($type)) {
		return PFC::Conf::Token->new($type, $c);
	}
	elsif ($c eq '-' or $c =~ /^\d$/o) {
		my ($itype, $value) = $me->parseInt($c);
		eval {
			$token = PFC::Conf::Token::Integer->
				new($itype, $value);
		};
		if ($@) {
			my $err = "$@";
			chomp($err);
			$me->error($err);
		}
		return $token;
	}
	elsif ($c eq '"') {
		eval {
			$token = $me->parseString($c);
		};
		if ($@) {
			my $err = "$@";
			chomp($err);
			$me->error($err);
		}
		return $token;
	}
	else {
		$token = $me->parseSymbol($c);
	}

	return $token;
}

# Parse integer token.
sub parseInt
{
	my $me = shift;
	my ($c) = @_;

	# This method parse integer as 64-bit value.
	my $arg = $c;
	my $type = INT_UINT64;
	my $base;
	if ($c eq '-') {
		# Negative value.
		$type = INT_INT64;
		$c = $me->getChar();
		$arg .= $c;
	}

	if ($c eq '0') {
		$c = $me->getChar();
		return ($type, 0) if (!defined($c));

		# Determine radix.
		$arg .= $c;
		if ($c eq 'x' or $c eq 'X') {
			$base = 16;
			$c = $me->getChar();
			$arg .= $c;
		}
		else {
			$base = 8;
		}
	}
	else {
		$base = 10;
	}

	my $parsed;
	while (defined($c)) {
		my $v;
		if ($c =~ /^\d$/o) {
			$v = $c + 0;
		}
		elsif ($c =~ /^[a-f]$/io) {
			$v = ord(lc($c)) - ord('a') + 10;
		}
		else {
			$me->ungetChar($c);
			my $len = length($arg);
			$arg = substr($arg, 0, $len - 1) if ($len > 0);
			last;
		}

		$me->error("Invalid character in integer: $c")
			if ($v >= $base);
		$parsed = 1;
		$c = $me->getChar();
		$arg .= $c;
	}

	return (INT_UINT64, 0) if ($arg eq '-0');

	unless ($parsed) {
		$me->error("No integer value is specified after ",
			   "\"$arg\".") unless ($arg eq '0');
	}

	return ($type, $arg);
}

# Parse quoted string.
sub parseString
{
	my $me = shift;
	my ($first) = @_;

	my $token = $me->parseStringImpl($first);
	while (my $tk = $me->nextToken()) {
		unless ($tk->getType() == TOKEN_STRING) {
			$me->undo($tk);
			last;
		}

		# Concatenate string.
		$token->concat($tk);
	}

	return $token;
}

sub parseStringImpl
{
	my $me = shift;
	my ($first) = @_;

	my $token = $first;
	my $backslash = \%PFC::Conf::Token::String::BACKSLASH;
	while (1) {
		my $c = $me->getChar();

		die "Unterminated string.\n" unless (defined($c));

		$token .= $c;
		my $escape;
		if ($c eq '\\') {
			$c = $me->getChar();
			die "Invalid backslash escape in string.\n"
				unless (defined($c));
			die "Unsupported backslash escape: \"\\$c\"\n"
				unless ($backslash->{$c});
			$token .= $c;
			$escape = 1;
		}
		last if (!$escape and $c eq '"');
	}

	return PFC::Conf::Token::String->new($token);
}

sub parseSymbol
{
	my $me = shift;
	my ($first) = @_;

	# Symbol must start with alphabet, or underscore.
	$me->error("Invalid character: $first")
		unless ($first =~ /^[a-zA-Z_]$/o);

	my $token = $first;
	my $c = $me->getChar();

	if (defined($c)) {
		while (1) {
			if ($c !~ /^[\w]$/o) {
				$me->ungetChar($c);
				last;
			}
			$token .= $c;

			$c = $me->getChar();
			last unless (defined($c));
		}
	}

	$me->error("Too long symbol: $token")
		if (length($token) > SYMBOL_MAXLEN);

	return PFC::Conf::Token->new(TOKEN_SYMBOL, $token);
}

# Body of filter process, which eliminates comments in cfdef file.
sub filter
{
	my $me = shift;
	my ($fh, $file) = @_;

	# Use "#line" directive to let cpp know cfdef file path.
	$file =~ s,",\\",g;
	print "#line 1 \"$file\"\n";

	my $comm;
	while (1) {
		my $c = $fh->getc();
		last unless (defined($c));

		if ($comm) {
			if ($c eq "\n") {
				undef $comm;
				print $c;
			}
			next;
		}
		if ($c eq '%') {
			$comm = 1;
		}
		else {
			print $c;
		}
	}
}

=head1 SEE ALSO

B<PFC::Conf::Constants>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
