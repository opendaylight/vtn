#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## IPC struct template file parser.
##

package PFC::IPC::Parser;

use strict;
use vars qw(%TOPLEVEL %TYPE_CLASS);

use Cwd qw(abs_path);
use FileHandle;
use File::Basename;
use POSIX;

# Piggyback lexer for PFC configuration file.
use PFC::Conf::Constants;
use PFC::Conf::Lexer;

use PFC::IPC;
use PFC::IPC::Include;
use PFC::IPC::Type;
use PFC::IPC::Field;
use PFC::IPC::Struct;
use PFC::IPC::StructInfo;
use PFC::IPC::StructInfo::Generator;

# Top level parser methods.
%TOPLEVEL = (ipc_struct		=> 'parseStruct',
	     include		=> 'parseInclude');

# Parser state for parseStruct().
use constant	S_FIELD_TYPE		=> 1;
use constant	S_FIELD_NAME		=> 2;
use constant	S_FIELD_NAME_END	=> 3;
use constant	S_FIELD_ARRAY		=> 4;
use constant	S_FIELD_ARRAY_SIZE	=> 5;
use constant	S_FIELD_ARRAY_END	=> 6;

use constant	OLDEST_YEAR		=> 2012;

=head1 NAME

PFC::IPC::Parser - IPC struct template file parser.

=head1 SYNOPSIS

  my $parser = PFC::IPC::Parser->new(NAMESPACE => 'ns');

=head1 ABSTRACT

B<PFC::IPC::Parser> is a syntax analyzer for the IPC struct template file.
It allows to you to analyze the IPC struct template file, and generates
C language header file and struct information file.

Note that the name of IPC struct template file must have suffix ".ipct".

=head1 DESCRIPTION

This section describes about public interface provided by B<PFC::IPC::Parser>.

=over 4

=item B<new>(%args)

Constructor.

Arguments must be specified by HASH.
The following keys are recognized.

=over 4

=item B<NAMESPACE>

Additional namespace of struct data.

=item B<INCLUDE_PATH>

Additional template file search path for B<include> directive.

=back

=cut

sub new
{
	my $this = shift;
	my (%args) = @_;

	my $class = ref($this);
	if ($class) {
		# This is top level parser.
		# Create struct information file generator.
		my $ns = $args{NAMESPACE};
		my $gen = PFC::IPC::StructInfo::Generator->new($ns);
		$args{GENERATOR} = $gen;
	}
	else {
		$class = $this;
	}

	$args{STRUCT_MAP} = {};
	$args{STRUCT_LIST} = [];

	return bless(\%args, $class);
}

=item B<clone>()

Create a sub parser to parse included template file.

=cut

sub clone
{
	my $me = shift;

	my (%copied);
	foreach my $key (qw(STRUCT_MAP STRUCT_LIST NAMESPACE PARSED_FILES
			    INCLUDE_PATH)) {
		$copied{$key} = $me->{$key};
	}

	return bless(\%copied, ref($me));
}

=item B<newFromInfo>($info)

Constructor using B<PFC::IPC::StructInfo::Parser> instance.
I<$info> must be instance of B<PFC::IPC::StructInfo::Parser>.

This constructor is used to generate C language header file from
IPC struct information file.

=cut

sub newFromInfo
{
	my $this = shift;
	my $class = ref($this) || $this;
	my ($info) = @_;

	my $head = $info->getHeader();
	my $ns = $head->{ifh_namespace};
	my (%args) = (INFO => $info, STRUCT_MAP => {}, STRUCT_LIST => []);
	if ($ns) {
		my $namespace = $info->getString($ns);
		die "Invalid namespace index: $ns\n" unless ($namespace);
		$args{NAMESPACE} = $namespace;
	}

	my $me = bless(\%args, $class);

	# Restore struct information from IPC struct information file.
	my $stmap = $me->{STRUCT_MAP};
	foreach my $stinfo (@{$info->getStructs()}) {
		$me->restoreStruct($info, $stinfo);
	}

	return $me;
}

=item B<parse>($file)

Parse the given template file.
An exception will be thrown on parse error.

If this instance has parsed IPC struct information, definitions in the
given file are merged.

=cut

sub parse
{
	my $me = shift;

	$me->parseImpl(@_);
	my $inc = $me->{INCLUDE_STACK};
	$inc->popStack() if ($inc);
}

=item B<merge>($parser)

Merge parsed data in the given B<PFC::IPC::Parser> instance.

=cut

sub merge
{
	my $me = shift;
	my ($parser) = @_;

	my $stlist = $parser->{STRUCT_LIST};
	my $stmap = $parser->{STRUCT_MAP};
	my $parsed = $parser->{PARSED_FILES} || {};

	my $mylist = $me->{STRUCT_LIST};
	my $mymap = $me->{STRUCT_MAP};
	my $myparsed = $me->{PARSED_FILES} || {};
	my $gen = $me->{GENERATOR};

	foreach my $struct (@$stlist) {
		my $name = $struct->getName();
		unless ($mymap->{$name}) {
			$mymap->{$name} = $struct;
			push(@$mylist, $struct);
			$gen->add($struct) if ($gen);
		}
	}

	while (my ($apath, $v) = each(%$parsed)) {
		$myparsed->{$apath} = $v;
	}
}

=item B<dumpHeader>($path, $tm, $filelist)

Output C language header file to the given file path.

=over 4

=item I<$path>

File path to store C language header definitions.
If "-" is specified, it is dumped to the standard output.

=item I<$tm>

An ARRAY reference which contains current calendar date returned by
localtime().

=item I<$filelist>

An ARRAY reference to keep generated file.

=back

=cut

sub dumpHeader
{
	my $me = shift;
	my ($path, $tm, $filelist) = @_;

	my ($fh);
	if ($path eq '-') {
		$fh = \*STDOUT;
		undef $path;
	}
	else {
		$fh = FileHandle->new($path, O_WRONLY | O_CREAT | O_TRUNC,
				      0644);
		die "open($path) failed: $!\n" unless ($fh);
		push(@$filelist, $path);
	}

	my $desc = 'User-defined data structures to be transferred via ' .
		'IPC framework.';
	$me->dumpHeaderCommon($fh, $tm, $path, $desc, 'dumpDefinitions');
}

=item B<dumpInfo>($path, $filelist)

Output IPC struct information file to the given file path.

=item I<$path>

File path to store IPC struct information.

=item I<$filelist>

An ARRAY reference to keep generated file.

=cut

sub dumpInfo
{
	my $me = shift;
	my ($path, $filelist) = @_;

	my $fh = FileHandle->new($path, O_WRONLY | O_CREAT | O_TRUNC, 0644) or
		die "open($path) failed: $!\n";
	push(@$filelist, $path);

	my $gen = $me->{GENERATOR};
	unless ($gen) {
		$gen = PFC::IPC::StructInfo::Generator->new($me->{NAMESPACE});
		foreach my $struct (@{$me->{STRUCT_LIST}}) {
			$gen->add($struct);
		}
	}

	$gen->output($fh);
}

=item B<dumpServerHeader>($path, $tm, $filelist)

Output C++ prototypes for IPC servers to the given file path.

=over 4

=item I<$path>

File path to store C++ prototypes for IPC server.

=item I<$tm>

An ARRAY reference which contains current calendar date returned by
localtime().

=item I<$filelist>

An ARRAY reference to keep generated file.

=back

=cut

sub dumpServerHeader
{
	my $me = shift;
	my ($path, $tm, $filelist) = @_;

	my $fh = FileHandle->new($path, O_WRONLY | O_CREAT | O_TRUNC, 0644) or
		die "open($path) failed: $!\n";
	push(@$filelist, $path);

	my $desc = 'pfc::core::ipc::ServerSession methods for ' .
		'user-defined data structure.';
	$me->dumpHeaderCommon($fh, $tm, $path, $desc, 'dumpAccessors',
			      'SERVER', 'dumpCxxServerPrototype');
}

=item B<dumpServerInlineHeader>($path, $tm, $filelist)

Output C++ inline methods for IPC servers to the given file path.

=over 4

=item I<$path>

File path to store C++ inline methods for IPC server.

=item I<$tm>

An ARRAY reference which contains current calendar date returned by
localtime().

=item I<$filelist>

An ARRAY reference to keep generated file.

=back

=cut

sub dumpServerInlineHeader
{
	my $me = shift;
	my ($path, $tm, $filelist) = @_;

	my $fh = FileHandle->new($path, O_WRONLY | O_CREAT | O_TRUNC, 0644) or
		die "open($path) failed: $!\n";
	push(@$filelist, $path);

	my $desc = 'pfc::core::ipc::ServerSession inline methods for ' .
		'user-defined data structure.';
	$me->dumpHeaderCommon($fh, $tm, $path, $desc, 'dumpAccessors',
			      'SERVER', 'dumpCxxServerAccessor');
}

=item B<dumpClientHeader>($path, $tm, $filelist)

Output C++ prototypes for IPC clients to the given file path.

=over 4

=item I<$path>

File path to store C++ prototypes for IPC client.

=item I<$tm>

An ARRAY reference which contains current calendar date returned by
localtime().

=item I<$filelist>

An ARRAY reference to keep generated file.

=back

=cut

sub dumpClientHeader
{
	my $me = shift;
	my ($path, $tm, $filelist) = @_;

	my $fh = FileHandle->new($path, O_WRONLY | O_CREAT | O_TRUNC, 0644) or
		die "open($path) failed: $!\n";
	push(@$filelist, $path);

	my $desc = 'pfc::core::ipc::ClientSession methods for ' .
		'user-defined data structure.';
	$me->dumpHeaderCommon($fh, $tm, $path, $desc, 'dumpAccessors',
			      'CLIENT', 'dumpCxxClientPrototype');
}

=item B<dumpClientInlineHeader>($path, $tm, $filelist)

Output C++ inline methods for IPC clients to the given file path.

=over 4

=item I<$path>

File path to store C++ inline methods for IPC client.

=item I<$tm>

An ARRAY reference which contains current calendar date returned by
localtime().

=item I<$filelist>

An ARRAY reference to keep generated file.

=back

=cut

sub dumpClientInlineHeader
{
	my $me = shift;
	my ($path, $tm, $filelist) = @_;

	my $fh = FileHandle->new($path, O_WRONLY | O_CREAT | O_TRUNC, 0644) or
		die "open($path) failed: $!\n";
	push(@$filelist, $path);

	my $desc = 'pfc::core::ipc::ClientSession inline methods for ' .
		'user-defined data structure.';
	$me->dumpHeaderCommon($fh, $tm, $path, $desc, 'dumpAccessors',
			      'CLIENT', 'dumpCxxClientAccessor');
}

=back

=cut

##
## Below are private methods.
##

sub error
{
	my $me = shift;
	my $lexer = shift;

	my $inc = $me->{INCLUDE_STACK};
	$inc->printStack() if ($inc);

	$lexer->error(@_);
}

sub syntaxError
{
	my $me = shift;
	my $lexer = shift;

	my $inc = $me->{INCLUDE_STACK};
	$inc->printStack() if ($inc);

	$lexer->syntaxError(@_);
}

sub semicolonError
{
	my $me = shift;
	my ($lexer, $token) = @_;

	my $v = $token->getValue();
	$me->syntaxError($lexer, "Semicolon is missing near \"$v\".");
}

sub tokenError
{
	my $me = shift;
	my ($lexer, $token) = @_;

	my $v = $token->getValue();
	$me->syntaxError($lexer, "Unexpected token: '$v'");
}

sub parseImpl
{
	my $me = shift;
	my ($file) = @_;

	my $apath = $me->{ABS_PATH};
	unless ($apath) {
		$apath = abs_path($file);
		die "Failed to determine absolute path: $file"
			unless ($apath);
	}

	my $parsed = $me->{PARSED_FILES};
	if ($parsed) {
		return if ($parsed->{$apath});
	}
	else {
		$parsed = {};
		$me->{PARSED_FILES} = $parsed;
	}

	$parsed->{$apath} = 1;

	my $parent = dirname($file);
	$parent =~ s,/*$,,go;
	$parent = undef if ($parent eq '.');

	$me->{FILE} = $file;
	$me->{ABS_PATH} = $apath;
	$me->{PARENT} = $parent;

	my $lexer = PFC::Conf::Lexer->new($file);

	while (1) {
		my $token = $lexer->nextToken();
		last unless (defined($token));

		$me->tokenError($lexer, $token)
			unless ($token->getType() == TOKEN_SYMBOL);

		my $value = $token->getValue();
		my $method = $TOPLEVEL{$value};
		$me->tokenError($lexer, $token) unless ($method);

		$me->$method($lexer);
	}

	delete($me->{ABS_PATH});
}

# Parse "ipc_struct" directive.
sub parseStruct
{
	my $me = shift;
	my ($lexer) = @_;

	my $token = $lexer->nextToken(1);
	my $type = $token->getType();
	$me->syntaxError($lexer, "Struct name is missing.")
		if ($type == TOKEN_LBRACE);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_SYMBOL);
	my $name = $token->getValue();
	my $ns = $me->{NAMESPACE};

	my $stmap = $me->{STRUCT_MAP};
	my $struct = $stmap->{$name};
	if ($struct) {
		my $tmpl = $struct->getTemplateFile();
		my $ln = $struct->getLineNumber();
		$me->error($lexer, "Struct \"$name\" is already defined in ",
			   "$tmpl at line $ln.");
	}

	eval {
		$struct = PFC::IPC::Struct->new($ns, $name, $me->{FILE},
						$lexer->lineNumber());
	};
	if ($@) {
		my $err = "$@";
		chomp($err);
		$me->error($lexer, $err);
	}

	$token = $lexer->nextToken(1);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_LBRACE);
	my $state = S_FIELD_TYPE;
	my ($ftype, $fname, $asize);

	while (1) {
		$token = $lexer->nextToken(1);
		if ($state == S_FIELD_TYPE) {
			$type = $token->getType();
			if ($type == TOKEN_RBRACE) {
				last;
			}
			elsif ($type != TOKEN_SYMBOL) {
				$me->tokenError($lexer, $token);
			}

			my $tname = $token->getValue();

			# Try built-in type.
			$ftype = PFC::IPC::Type::get($tname);
			unless ($ftype) {
				# Try struct type.
				$ftype = $stmap->{$tname};
				$me->syntaxError($lexer,"Unknown type: $tname")
					unless ($ftype);

			}
			$state = S_FIELD_NAME;
		}
		elsif ($state == S_FIELD_NAME) {
			$me->tokenError($lexer, $token)
				unless ($token->getType() == TOKEN_SYMBOL);
			$fname = $token->getValue();
			$state = S_FIELD_NAME_END;
		}
		elsif ($state == S_FIELD_NAME_END) {
			$type = $token->getType();
			if ($type == TOKEN_SQLEFT) {
				# Array type.
				$state = S_FIELD_ARRAY;
			}
			elsif ($type == TOKEN_SEMI) {
				$me->addField($lexer, $struct, $ftype, $fname);
				$state = S_FIELD_TYPE;
			}
			else {
				$me->tokenError($lexer, $token);
			}
		}
		elsif ($state == S_FIELD_ARRAY) {
			$type = $token->getType();
			$me->tokenError($lexer, $token)
				unless ($type == TOKEN_INT);

			$asize = $token->getValue();
			$state = S_FIELD_ARRAY_SIZE;
		}
		elsif ($state == S_FIELD_ARRAY_SIZE) {
			$type = $token->getType();
			$me->tokenError($lexer, $token)
				unless ($type == TOKEN_SQRIGHT);
			$state = S_FIELD_ARRAY_END;
		}
		elsif ($state == S_FIELD_ARRAY_END) {
			$type = $token->getType();
			$me->semicolonError($lexer, $token)
				unless ($type == TOKEN_SEMI);
			$me->addField($lexer, $struct, $ftype, $fname, $asize);
			$state = S_FIELD_TYPE;
		}
	}

	$token = $lexer->nextToken(1);
	$me->semicolonError($lexer, $token)
		unless ($token->getType() == TOKEN_SEMI);

	$me->error($lexer, "No field is defined in struct \"$name\".")
		if ($struct->getFieldCount() == 0);

	eval {
		$struct->finalize();
	};
	if ($@) {
		my $err = "$@";
		chomp($err);
		$me->error($lexer, $err);
	}
	$stmap->{$name} = $struct;
	push(@{$me->{STRUCT_LIST}}, $struct);
}

# Parse "include" directive.
sub parseInclude
{
	my $me = shift;
	my ($lexer) = @_;

	# Fetch template file to be included.
	my $token = $lexer->nextToken(1);
	$me->tokenError($lexer, $token)
		unless ($token->getType() == TOKEN_STRING);
	my $file = $token->body();

	$token = $lexer->nextToken(1);
	$me->semicolonError($lexer, $token)
		unless ($token->getType() == TOKEN_SEMI);

	# Determine template file path to be included.
	my $path = $me->searchFile($file);
	$me->error($lexer, "Could not find file to be included: $file")
		unless ($path);

	my $apath = abs_path($path);
	$me->error($lexer, "Could not determine absolute path: $file")
		unless ($apath);

	# Push current position to the inclusion stack.
	my $inc = $me->{INCLUDE_STACK};
	my $from = $me->{ABS_PATH};
	my $lnum = $lexer->lineNumber();
	if ($inc) {
		$inc->pushStack($from, $lnum);
	}
	else {
		$inc = PFC::IPC::Include->new($from, $lnum);
	}

	unless ($inc->canPush($apath)) {
		$me->{INCLUDE_STACK} = $inc;
		$me->error($lexer, "File inclusion loop was detected.");
	}

	# Create sub parser to include another template file.
	my $ps = $me->clone();

	$ps->{ABS_PATH} = $apath;
	$ps->{INCLUDE_STACK} = $inc;

	# Import definitions in the given file.
	$ps->parse($path);
}

sub searchFile
{
	my $me = shift;
	my ($fname) = @_;

	my $parent = $me->{PARENT};
	my $path = ($parent) ? $parent . '/' . $fname : $fname;
	return $path if (-r $path);

	foreach my $dir (@{$me->{INCLUDE_PATH}}) {
		$path = $dir . '/' . $fname;
		return $path if (-r $path);
	}

	return undef;
}

sub addField
{
	my $me = shift;
	my ($lexer, $struct, $ftype, $fname, $asize) = @_;

	eval {
		my $f = PFC::IPC::Field->new($ftype, $fname, $asize);
		$struct->add($f);
	};
	if ($@) {
		my $err = "$@";
		chomp($err);
		$me->error($lexer, $err);
	}

}

sub dumpHeaderCommon
{
	my $me = shift;
	my ($fh, $tm, $path, $desc, $method, @args) = @_;

	my $ident = '_PFC_IPCTC_COMPILED_';
	if ($path) {
		my $fname = basename($path);
		$fname =~ s,[-.],_,go;
		$ident .= uc($fname);
	}
	else {
		$ident .= 'IPC_STRUCT_H';
	}

	my $yrange = $tm->[5] + 1900;
	$yrange = OLDEST_YEAR . '-' . $yrange if ($yrange > OLDEST_YEAR);
	my $date = POSIX::strftime("%Y-%m-%d %H:%M:%S %Z", @$tm);

	my (%meta) = (Date => $date);
	my $tagwidth = 4;
	my $ns = $me->{NAMESPACE};
	if ($ns) {
		$meta{Namespace} = $ns;
		$tagwidth = 9;
	}

	$fh->print(<<OUT);
/*
 * Copyright (c) $yrange NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	$ident
#define	$ident

/*
 * $desc
 *
 * This file was generated by the IPC struct template compiler.
 * DO NOT EDIT!!
 *
OUT

	foreach my $tag (sort(keys(%meta))) {
		my $v = $meta{$tag};
		$fh->printf(" *   %*s : %s\n", -$tagwidth, $tag, $v);
	}

	$fh->print(<<OUT);
 */

OUT

	$me->$method($fh, @args);

	$fh->print(<<OUT);

#endif	/* $ident */
OUT
}

sub dumpDefinitions
{
	my $me = shift;
	my ($fh) = @_;

	my $sigpfx = SIGNATURE_PREFIX;

	$fh->print(<<OUT);
#include <pfc/base.h>
#include <netinet/in.h>

PFC_C_BEGIN_DECL
OUT

	foreach my $struct (@{$me->{STRUCT_LIST}}) {
		$struct->dumpHeader($fh);
	}

	$fh->print(<<OUT);
PFC_C_END_DECL
OUT
}

sub dumpAccessors
{
	my $me = shift;
	my ($fh, $imprefix, $method) = @_;

	my $import_def = CXX_IMPORT_PREFIX . $imprefix;

	$fh->print(<<OUT);

#ifndef	$import_def
#error	Never include this file directly!
#endif	/* !$import_def */

OUT

	foreach my $struct (@{$me->{STRUCT_LIST}}) {
		$struct->$method($fh);
	}
}

sub restoreStruct
{
	my $me = shift;
	my ($info, $stinfo) = @_;

	my $nameidx = $stinfo->{ifs_name};
	my $name = $info->getString($nameidx);
	die "Invalid struct name index: $nameidx\n" unless ($name);
	$name = $me->restoreStructName($name);
	my $struct = PFC::IPC::Struct->new($me->{NAMESPACE}, $name);

	my $fidx = $stinfo->{ifs_field};
	my $nfields = $stinfo->{ifs_nfields};

	for (my $i = $fidx; $i < $fidx + $nfields; $i++) {
		my $finfo = $info->getFieldAt($i);
		die "$name.$fidx: Invalid field index: $i\n" unless ($finfo);

		eval {
			my $field = $me->restoreField($info, $finfo);
			$struct->add($field);
		};
		if ($@) {
			my $err = "$@";
			chomp($err);
			die "$name.$fidx: $err\n";
		}
	}

	$struct->finalize();
	my $sig = $struct->getSignature();
	my $isig = $stinfo->{ifs_sig};
	die "Unexpected signature: $sig: required=$isig\n"
		unless ($sig eq $isig);

	$me->{STRUCT_MAP}->{$name} = $struct;
	push(@{$me->{STRUCT_LIST}}, $struct);
}

sub restoreStructName
{
	my $me = shift;
	my ($name) = @_;

	my $ns = $me->{NAMESPACE};
	if ($ns) {
		die "Unexpected struct type name: $name\n"
			unless ($name =~ s,^${ns}_(.+)$,$1,);
	}

	return $name;
}

sub restoreField
{
	my $me = shift;
	my ($info, $finfo) = @_;

	my $tp;
	my $type = $finfo->{iff_type};
	if ($type & IPC_IFFIELD_STRUCT()) {
		my $idx = $type & (~IPC_IFFIELD_STRUCT());
		my $stname = $info->getString($idx);
		die "Invalid struct name index: $idx\n" unless ($stname);
		$stname = $me->restoreStructName($stname);
		$tp = $me->{STRUCT_MAP}->{$stname};
		die "Unknown struct name: $stname\n" unless ($tp);
	}
	else {
		my $sym = PFC::IPC::Type::getSymbol($type);
		die "Unknown type identifier: $sym\n" unless ($sym);
		$tp = PFC::IPC::Type::get($sym);
		die "Unknown type identifier: $type ($sym)\n" unless ($tp);
	}

	my $nidx = $finfo->{iff_name};
	my $fname = $info->getString($nidx) or
		die "Invalid field name index: $nidx\n";
	my $nelems = $finfo->{iff_array} || undef;

	return PFC::IPC::Field->new($tp, $fname, $nelems);
}

=head1 SEE ALSO

B<PFC::Conf::Lexer>(3),
B<PFC::IPC>(3),
B<PFC::IPC::Type>(3),
B<PFC::IPC::Struct>(3),
B<PFC::IPC::Field>(3),
B<PFC::IPC::StructInfo::Parser>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
