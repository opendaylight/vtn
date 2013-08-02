/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * strtoint.c - String to integer converter.
 */

#include <pfc/strtoint.h>
#include <pfc/ctype.h>

/*
 * Valid range of signed integer.
 */
#define	U32_INT32_MIN		((uint32_t)INT32_MIN)
#define	U32_INT32_MAX		((uint32_t)INT32_MAX)

#define	U64_INT64_MIN		((uint64_t)INT64_MIN)
#define	U64_INT64_MAX		((uint64_t)INT64_MAX)

/*
 * Declare parser function.
 */
#define	STRTOINT_PARSER_DECL(funcname, itype, imax)			\
	static int							\
	funcname(const char *PFC_RESTRICT string,			\
		 itype *PFC_RESTRICT valuep)				\
	{								\
		itype		radix, value, max;			\
		char		c;					\
									\
		/* Determine radix by the string prefix. */		\
		if (*string == '0') {					\
			string++;					\
			if (*string == 'x' || *string == 'X') {		\
				radix = 16;				\
				string++;				\
			}						\
			else {						\
				radix = 8;				\
			}						\
		}							\
		else {							\
			radix = 10;					\
		}							\
									\
		c = *string;						\
		if (c == '\0') {					\
			if (PFC_EXPECT_FALSE(radix != 8)) {		\
				/* An empty string is specified. */	\
				return EINVAL;				\
			}						\
			*valuep = 0;					\
									\
			return 0;					\
		}							\
									\
		value = 0;						\
		max = imax / radix;					\
		do {							\
			itype	v, nextval;				\
									\
			if (pfc_isdigit_u(c)) {				\
				v = c - '0';				\
			}						\
			else if (c >= 'a' && c <= 'f') {		\
				v = c - 'a' + 10;			\
			}						\
			else if (c >= 'A' && c <= 'F') {		\
				v = c - 'A' + 10;			\
			}						\
			else {						\
				return EINVAL;				\
			}						\
									\
			if (PFC_EXPECT_FALSE(v >= radix)) {		\
				return EINVAL;				\
			}						\
									\
			nextval = value * radix + v;			\
			if (PFC_EXPECT_FALSE(value > max ||		\
					     nextval < value)) {	\
				/* Arithmetic overflow. */		\
				return ERANGE;				\
			}						\
			value = nextval;				\
			string++;					\
			c = *string;					\
		} while (c != '\0');					\
									\
		*valuep = value;					\
									\
		return 0;						\
	}

/*
 * static int
 * strtou32(const char *PFC_RESTRICT string, uint32_t *PFC_RESTRICT valuep)
 *	Parse the specified string as unsigned 32-bit integer.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted value is stored to the buffer
 *	pointed by `valuep', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must eliminate sign prefix from the string.
 */
STRTOINT_PARSER_DECL(strtou32, uint32_t, UINT32_MAX);

/*
 * static int
 * strtou64(const char *PFC_RESTRICT string, uint64_t *PFC_RESTRICT valuep)
 *	Parse the specified string as unsigned64-bit integer.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted value is stored to the buffer
 *	pointed by `valuep', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must eliminate sign prefix from the string.
 */
STRTOINT_PARSER_DECL(strtou64, uint64_t, UINT64_MAX);

/*
 * int
 * pfc_strtoi32(const char *PFC_RESTRICT string, int32_t *PFC_RESTRICT valuep)
 *	Convert the specified string to signed 32-bit value.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted value is stored to `*valuep',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 *
 * Remarks:
 *	Unlike strtol(3), leading whitespace is not allowed.
 */
int
pfc_strtoi32(const char *PFC_RESTRICT string, int32_t *PFC_RESTRICT valuep)
{
	char		c;
	int		err;
	uint32_t	u32;
	pfc_bool_t	negative;

	/* Parse a sign character. */
	c = *string;
	if (c == '-') {
		negative = PFC_TRUE;
		string++;
	}
	else {
		negative = PFC_FALSE;
		if (c == '+') {
			string++;
		}
	}

	/* Parse the string as unsigned 32-bit integer. */
	err = strtou32(string, &u32);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if (negative) {
		if (PFC_EXPECT_FALSE(u32 > U32_INT32_MIN)) {
			return ERANGE;
		}

		*valuep = -(int32_t)u32;
	}
	else {
		if (PFC_EXPECT_FALSE(u32 > U32_INT32_MAX)) {
			return ERANGE;
		}

		*valuep = (int32_t)u32;
	}

	return 0;
}

/*
 * int
 * pfc_strtou32(const char *PFC_RESTRICT string, uint32_t *PFC_RESTRICT valuep)
 *	Convert the specified string to unsigned 32-bit value.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted value is stored to `*valuep',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 *
 * Remarks:
 *	Unlike strtoul(3), leading minus sign and whitespace are not allowed.
 */
int
pfc_strtou32(const char *PFC_RESTRICT string, uint32_t *PFC_RESTRICT valuep)
{
	/* Skip plus sign. */
	if (*string == '+') {
		string++;
	}

	return strtou32(string, valuep);
}

/*
 * int
 * pfc_strtoi64(const char *PFC_RESTRICT string, int64_t *PFC_RESTRICT valuep)
 *	Convert the specified string to signed 64-bit value.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted value is stored to `*valuep',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 *
 * Remarks:
 *	Unlike strtoll(3), leading whitespace is not allowed.
 */
int
pfc_strtoi64(const char *PFC_RESTRICT string, int64_t *PFC_RESTRICT valuep)
{
	char		c;
	int		err;
	uint64_t	u64;
	pfc_bool_t	negative;

	/* Parse a sign character. */
	c = *string;
	if (c == '-') {
		negative = PFC_TRUE;
		string++;
	}
	else {
		negative = PFC_FALSE;
		if (c == '+') {
			string++;
		}
	}

	/* Parse the string as unsigned 64-bit integer. */
	err = strtou64(string, &u64);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if (negative) {
		if (PFC_EXPECT_FALSE(u64 > U64_INT64_MIN)) {
			return ERANGE;
		}

		*valuep = -(int64_t)u64;
	}
	else {
		if (PFC_EXPECT_FALSE(u64 > U64_INT64_MAX)) {
			return ERANGE;
		}

		*valuep = (int64_t)u64;
	}

	return 0;
}

/*
 * int
 * pfc_strtou64(const char *PFC_RESTRICT string, uint64_t *PFC_RESTRICT valuep)
 *	Convert the specified string to unsigned 64-bit value.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted value is stored to `*valuep',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 *
 * Remarks:
 *	Unlike strtoull(3), leading minus sign and whitespace are not allowed.
 */
int
pfc_strtou64(const char *PFC_RESTRICT string, uint64_t *PFC_RESTRICT valuep)
{
	/* Skip plus sign. */
	if (*string == '+') {
		string++;
	}

	return strtou64(string, valuep);
}
