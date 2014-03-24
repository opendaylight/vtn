/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_CTYPE_H
#define	_PFC_CTYPE_H

/*
 * ctype.h functions specific to C locale.
 *
 * Current locale never affects functions defined in this file.
 * They always check the specified character according to C locale.
 */

#include <pfc/base.h>
#include <pfc/debug.h>
#ifdef	PFC_VERBOSE_DEBUG
#include <stdio.h>			/* for EOF */
#endif	/* PFC_VERBOSE_DEBUG */

PFC_C_BEGIN_DECL

/*
 * Character specification.
 */

/* US-ASCII */
#define	_PFC_CT_ASCII		PFC_CONST_U(0x01)	/* US-ASCII */
#define	_PFC_CT_LOWER		PFC_CONST_U(0x02)	/* lower alphabet */
#define	_PFC_CT_UPPER		PFC_CONST_U(0x04)	/* upper alphabet */
#define	_PFC_CT_DIGIT		PFC_CONST_U(0x08)	/* digit */
#define	_PFC_CT_SPACE		PFC_CONST_U(0x10)	/* space */

#define	_PFC_CT_ALPHA		(_PFC_CT_LOWER | _PFC_CT_UPPER)
#define	_PFC_CT_ALNUM		(_PFC_CT_ALPHA | _PFC_CT_DIGIT)

/*
 * Assertion which detects invalid character.
 */
#define	__PFC_CTYPE_CHAR_ASSERT(c)					\
	PFC_ASSERT((int)(c) == EOF || (uint32_t)(c) <= UINT8_MAX)

extern const uint8_t	*const __pfc_ctype_spec;

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isdigit(int c)
 *	Check for a decimal digit character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is a decimal digit.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isdigit(int c)
{
	__PFC_CTYPE_CHAR_ASSERT(c);

	return *(__pfc_ctype_spec + c) & _PFC_CT_DIGIT;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isspace(int c)
 *	Check for a white-space character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is a white-space.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isspace(int c)
{
	__PFC_CTYPE_CHAR_ASSERT(c);

	return *(__pfc_ctype_spec + c) & _PFC_CT_SPACE;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isalpha(int c)
 *	Check for an alphabet.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is an alphabet
 *	or digit.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isalpha(int c)
{
	__PFC_CTYPE_CHAR_ASSERT(c);

	return *(__pfc_ctype_spec + c) & _PFC_CT_ALPHA;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isalnum(int c)
 *	Check for an alpha-numeric character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is an alphabet
 *	or digit.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isalnum(int c)
{
	__PFC_CTYPE_CHAR_ASSERT(c);

	return *(__pfc_ctype_spec + c) & _PFC_CT_ALNUM;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_islower(int c)
 *	Check for a lower-case alphabet character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is a lower-case
 *	alphabet.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_islower(int c)
{
	__PFC_CTYPE_CHAR_ASSERT(c);

	return *(__pfc_ctype_spec + c) & _PFC_CT_LOWER;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isupper(int c)
 *	Check for an upper-case alphabet character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is an upper-case
 *	alphabet.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isupper(int c)
{
	__PFC_CTYPE_CHAR_ASSERT(c);

	return *(__pfc_ctype_spec + c) & _PFC_CT_UPPER;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isascii(int c)
 *	Check for an US-ASCII character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is an US-ASCII
 *	character.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isascii(int c)
{
	__PFC_CTYPE_CHAR_ASSERT(c);

	return *(__pfc_ctype_spec + c) & _PFC_CT_ASCII;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_tolower(int c)
 *	Convert the character specified by `c' to lower case, if possible.
 *
 * Calling/Exit State:
 *	On successful return, the converted character is returned.
 *	`c' is returned if the conversion was not possible.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_tolower(int c)
{
	return (pfc_isupper(c)) ? (c - 'A') + 'a' : c;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_toupper(int c)
 *	Convert the character specified by `c' to upper case, if possible.
 *
 * Calling/Exit State:
 *	On successful return, the converted character is returned.
 *	`c' is returned if the conversion was not possible.
 *
 * Remarks:
 *	`c' must be unsigned char value or EOF.
 *	Otherwise this function results in undefined behavior.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_toupper(int c)
{
	return (pfc_islower(c)) ? (c - 'a') + 'A' : c;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isdigit_u(uint8_t c)
 *	Check for a decimal digit character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is a decimal digit.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	Unlike pfc_isdigit(), pfc_isdigit_u() can not take EOF as argument.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isdigit_u(uint8_t c)
{
	return pfc_isdigit(c);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isspace_u(uint8_t c)
 *	Check for a white-space character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is a white-space.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	Unlike pfc_isspace(), pfc_isspace_u() can not take EOF as argument.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isspace_u(uint8_t c)
{
	return pfc_isspace(c);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isalpha_u(uint8_t c)
 *	Check for an alphabet.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is an alphabet
 *	or digit.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	Unlike pfc_isalpha(), pfc_isalpha_u() can not take EOF as argument.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isalpha_u(uint8_t c)
{
	return pfc_isalpha(c);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isalnum_u(uint8_t c)
 *	Check for an alpha-numeric character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is an alphabet
 *	or digit.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	Unlike pfc_isalnum(), pfc_isalnum_u() can not take EOF as argument.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isalnum_u(uint8_t c)
{
	return pfc_isalnum(c);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_islower_u(uint8_t c)
 *	Check for a lower-case alphabet character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is a lower-case
 *	alphabet.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	Unlike pfc_islower(), pfc_islower_u() can not take EOF as argument.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_islower_u(uint8_t c)
{
	return pfc_islower(c);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isupper_u(uint8_t c)
 *	Check for an upper-case alphabet character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is an upper-case
 *	alphabet.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	Unlike pfc_isupper(), pfc_isupper_u() can not take EOF as argument.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isupper_u(uint8_t c)
{
	return pfc_isupper(c);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_isascii_u(uint8_t c)
 *	Check for an US-ASCII character.
 *
 * Calling/Exit State:
 *	A non-zero value is returned if the given character is an US-ASCII
 *	character.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	Unlike pfc_isascii(), pfc_isascii_u() can not take EOF as argument.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_isascii_u(uint8_t c)
{
	return pfc_isascii(c);
}

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_tolower_u(uint8_t c)
 *	Convert the character specified by `c' to lower case, if possible.
 *
 * Calling/Exit State:
 *	On successful return, the converted character is returned.
 *	`c' is returned if the conversion was not possible.
 *
 * Remarks:
 *	Unlike pfc_tolower(), pfc_tolower_u() can not take EOF as argument.
 */
static inline uint8_t PFC_FATTR_ALWAYS_INLINE
pfc_tolower_u(uint8_t c)
{
	return (uint8_t)pfc_tolower(c);
}

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_toupper_u(uint8_t c)
 *	Convert the character specified by `c' to upper case, if possible.
 *
 * Calling/Exit State:
 *	On successful return, the converted character is returned.
 *	`c' is returned if the conversion was not possible.
 *
 * Remarks:
 *	Unlike pfc_toupper(), pfc_toupper_u() can not take EOF as argument.
 */
static inline uint8_t PFC_FATTR_ALWAYS_INLINE
pfc_toupper_u(uint8_t c)
{
	return (uint8_t)pfc_toupper(c);
}

PFC_C_END_DECL

#endif	/* !_PFC_CTYPE_H */
