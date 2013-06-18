/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ctype.c - ctype.h functions specific to C locale.
 */

#include <pfc/ctype.h>

/*
 * Set character specification bit with US-ASCII bit.
 */
#define	CT_ASCII(name)		(_PFC_CT_##name | _PFC_CT_ASCII)

/*
 * C locale character specification.
 */
static const uint8_t	ctype_spec[257] = {
	0,			/* EOF */		/* -1 */
	_PFC_CT_ASCII,		/* NUL */		/* 0 */
	_PFC_CT_ASCII,		/* SOH */
	_PFC_CT_ASCII,		/* STX */
	_PFC_CT_ASCII,		/* ETX */
	_PFC_CT_ASCII,		/* EOT */
	_PFC_CT_ASCII,		/* ENQ */
	_PFC_CT_ASCII,		/* ACK */
	_PFC_CT_ASCII,		/* '\a' */
	_PFC_CT_ASCII,		/* '\b' */
	CT_ASCII(SPACE),	/* '\t' */
	CT_ASCII(SPACE),	/* '\n' */		/* 10 */
	CT_ASCII(SPACE),	/* '\v' */
	CT_ASCII(SPACE),	/* '\f' */
	CT_ASCII(SPACE),	/* '\r' */
	_PFC_CT_ASCII,		/* SO */
	_PFC_CT_ASCII,		/* SI */
	_PFC_CT_ASCII,		/* DLE */
	_PFC_CT_ASCII,		/* DC1 */
	_PFC_CT_ASCII,		/* DC2 */
	_PFC_CT_ASCII,		/* DC3 */
	_PFC_CT_ASCII,		/* DC4 */		/* 20 */
	_PFC_CT_ASCII,		/* NAK */
	_PFC_CT_ASCII,		/* SYN */
	_PFC_CT_ASCII,		/* ETB */
	_PFC_CT_ASCII,		/* CAN */
	_PFC_CT_ASCII,		/* EM */
	_PFC_CT_ASCII,		/* SUB */
	_PFC_CT_ASCII,		/* ESC */
	_PFC_CT_ASCII,		/* FS */
	_PFC_CT_ASCII,		/* GS */
	_PFC_CT_ASCII,		/* RS */		/* 30 */
	_PFC_CT_ASCII,		/* US */
	CT_ASCII(SPACE),	/* SPACE */
	_PFC_CT_ASCII,		/* '!' */
	_PFC_CT_ASCII,		/* '"' */
	_PFC_CT_ASCII,		/* '#' */
	_PFC_CT_ASCII,		/* '$' */
	_PFC_CT_ASCII,		/* '%' */
	_PFC_CT_ASCII,		/* '&' */
	_PFC_CT_ASCII,		/* '\'' */
	_PFC_CT_ASCII,		/* '(' */		/* 40 */
	_PFC_CT_ASCII,		/* ')' */
	_PFC_CT_ASCII,		/* '*' */
	_PFC_CT_ASCII,		/* '+' */
	_PFC_CT_ASCII,		/* ',	' */
	_PFC_CT_ASCII,		/* '-' */
	_PFC_CT_ASCII,		/* '.' */
	_PFC_CT_ASCII,		/* '/' */
	CT_ASCII(DIGIT),	/* '0' */
	CT_ASCII(DIGIT),	/* '1' */
	CT_ASCII(DIGIT),	/* '2' */		/* 50 */
	CT_ASCII(DIGIT),	/* '3' */
	CT_ASCII(DIGIT),	/* '4' */
	CT_ASCII(DIGIT),	/* '5' */
	CT_ASCII(DIGIT),	/* '6' */
	CT_ASCII(DIGIT),	/* '7' */
	CT_ASCII(DIGIT),	/* '8' */
	CT_ASCII(DIGIT),	/* '9' */
	_PFC_CT_ASCII,		/* ':' */
	_PFC_CT_ASCII,		/* ';' */
	_PFC_CT_ASCII,		/* '<' */		/* 60 */
	_PFC_CT_ASCII,		/* '=' */
	_PFC_CT_ASCII,		/* '>' */
	_PFC_CT_ASCII,		/* '?' */
	_PFC_CT_ASCII,		/* '@' */
	CT_ASCII(UPPER),	/* 'A' */
	CT_ASCII(UPPER),	/* 'B' */
	CT_ASCII(UPPER),	/* 'C' */
	CT_ASCII(UPPER),	/* 'D' */
	CT_ASCII(UPPER),	/* 'E' */
	CT_ASCII(UPPER),	/* 'F' */		/* 70 */
	CT_ASCII(UPPER),	/* 'G' */
	CT_ASCII(UPPER),	/* 'H' */
	CT_ASCII(UPPER),	/* 'I' */
	CT_ASCII(UPPER),	/* 'J' */
	CT_ASCII(UPPER),	/* 'K' */
	CT_ASCII(UPPER),	/* 'L' */
	CT_ASCII(UPPER),	/* 'M' */
	CT_ASCII(UPPER),	/* 'N' */
	CT_ASCII(UPPER),	/* 'O' */
	CT_ASCII(UPPER),	/* 'P' */		/* 80 */
	CT_ASCII(UPPER),	/* 'Q' */
	CT_ASCII(UPPER),	/* 'R' */
	CT_ASCII(UPPER),	/* 'S' */
	CT_ASCII(UPPER),	/* 'T' */
	CT_ASCII(UPPER),	/* 'U' */
	CT_ASCII(UPPER),	/* 'V' */
	CT_ASCII(UPPER),	/* 'W' */
	CT_ASCII(UPPER),	/* 'X' */
	CT_ASCII(UPPER),	/* 'Y' */
	CT_ASCII(UPPER),	/* 'Z' */		/* 90 */
	_PFC_CT_ASCII,		/* '[' */
	_PFC_CT_ASCII,		/* '\\' */
	_PFC_CT_ASCII,		/* ']' */
	_PFC_CT_ASCII,		/* '^' */
	_PFC_CT_ASCII,		/* '_' */
	_PFC_CT_ASCII,		/* '`' */
	CT_ASCII(LOWER),	/* 'a' */
	CT_ASCII(LOWER),	/* 'b' */
	CT_ASCII(LOWER),	/* 'c' */
	CT_ASCII(LOWER),	/* 'd' */		/* 100 */
	CT_ASCII(LOWER),	/* 'e' */
	CT_ASCII(LOWER),	/* 'f' */
	CT_ASCII(LOWER),	/* 'g' */
	CT_ASCII(LOWER),	/* 'h' */
	CT_ASCII(LOWER),	/* 'i' */
	CT_ASCII(LOWER),	/* 'j' */
	CT_ASCII(LOWER),	/* 'k' */
	CT_ASCII(LOWER),	/* 'l' */
	CT_ASCII(LOWER),	/* 'm' */
	CT_ASCII(LOWER),	/* 'n' */		/* 110 */
	CT_ASCII(LOWER),	/* 'o' */
	CT_ASCII(LOWER),	/* 'p' */
	CT_ASCII(LOWER),	/* 'q' */
	CT_ASCII(LOWER),	/* 'r' */
	CT_ASCII(LOWER),	/* 's' */
	CT_ASCII(LOWER),	/* 't' */
	CT_ASCII(LOWER),	/* 'u' */
	CT_ASCII(LOWER),	/* 'v' */
	CT_ASCII(LOWER),	/* 'w' */
	CT_ASCII(LOWER),	/* 'x' */		/* 120 */
	CT_ASCII(LOWER),	/* 'y' */
	CT_ASCII(LOWER),	/* 'z' */
	_PFC_CT_ASCII,		/* '{' */
	_PFC_CT_ASCII,		/* '|' */
	_PFC_CT_ASCII,		/* '}' */
	_PFC_CT_ASCII,		/* '~' */
	_PFC_CT_ASCII,		/* DEL */		/* 127 */

	/* 128 - 255 must be zero. */
};

const uint8_t	*const __pfc_ctype_spec = &ctype_spec[1];
