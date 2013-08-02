/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.c - Miscellaneous utilities.
 */

#include <string.h>
#include "pfc_control.h"

#define	MINUTE_SECONDS		PFC_CONST_U(60)
#define	HOUR_SECONDS		(MINUTE_SECONDS * PFC_CONST_U(60))
#define	DAY_SECONDS		(HOUR_SECONDS * PFC_CONST_U(24))

#define	SHIFT_KILOBYTE		PFC_CONST_U(10)
#define	SHIFT_MEGABYTE		(SHIFT_KILOBYTE + PFC_CONST_U(10))
#define	SHIFT_GIGABYTE		(SHIFT_MEGABYTE + PFC_CONST_U(10))

#define	KILOBYTES		(PFC_CONST_ULL(1) << SHIFT_KILOBYTE)
#define	MEGABYTES		(PFC_CONST_ULL(1) << SHIFT_MEGABYTE)
#define	GIGABYTES		(PFC_CONST_ULL(1) << SHIFT_GIGABYTE)

/*
 * Strings which represents boolean value.
 */
static const char	*bool_true[] = {
	"on",
	"yes",
	"true",
	"enable",
};

static const char	*bool_false[] = {
	"off",
	"no",
	"false",
	"disable",
};

/*
 * Internal prototypes.
 */
static int	prompt_read(void);

/*
 * int
 * prompt(void)
 *	Print prompt to the standard output, and require user to input
 *	'y' or 'n'.
 *
 * Calling/Exit State:
 *	1 is returned if 'y' was read.
 *	0 is returned if 'n' was read.
 *	-1 is returned on failure.
 */
int
prompt(void)
{
	for (;;) {
		int	c;

		fputs(str_prompt_prefix, stdout);
		fputs("Is this OK? (y/N): ", stdout);
		fflush(stdout);

		c = prompt_read();
		if (PFC_EXPECT_FALSE(c == EOF)) {
			return -1;
		}

		if (c == 'y' || c == 'Y') {
			return 1;
		}

		/* Default is 'N'. */
		if (c == 'n' || c == 'N' || c == 0) {
			return 0;
		}
		fputs("\n*** Type 'y' or 'n'.\n\n", stdout);
	}
}

/*
 * int
 * as_bool(const char *PFC_RESTRICT str, pfc_bool_t *boolp)
 *	Convert the string specified by `str' to pfc_bool_t value.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted boolean value is set to the
 *	buffer pointed by `boolp', and zero is returned.
 *
 *	Otherwise EINVAL is returned.
 */
int
as_bool(const char *PFC_RESTRICT str, pfc_bool_t *boolp)
{
	const char	**pp;

	PFC_ASSERT(str != NULL && boolp != NULL);

	for (pp = bool_true; pp < PFC_ARRAY_LIMIT(bool_true); pp++) {
		if (strcasecmp(str, *pp) == 0) {
			*boolp = PFC_TRUE;

			return 0;
		}
	}

	for (pp = bool_false; pp < PFC_ARRAY_LIMIT(bool_false); pp++) {
		if (strcasecmp(str, *pp) == 0) {
			*boolp = PFC_FALSE;

			return 0;
		}
	}

	return EINVAL;
}

/*
 * void
 * pretty_format_seconds(char *buf, size_t bufsize, uint32_t sec)
 *	Format amount of time in seconds.
 */
void
pretty_format_seconds(char *buf, size_t bufsize, uint32_t sec)
{
	uint32_t	mod, min, hour, day;
	const char	*sep;
	int		len;

	if (sec < MINUTE_SECONDS) {
		snprintf(buf, bufsize, "%u (second%s)", sec,
			 (sec <= 1) ? str_empty : str_s);

		return;
	}

	day = sec / DAY_SECONDS;
	mod = sec - (day * DAY_SECONDS);
	hour = mod / HOUR_SECONDS;
	mod -= (hour * HOUR_SECONDS);
	min = mod / MINUTE_SECONDS;
	mod -= (min * MINUTE_SECONDS);

	len = snprintf(buf, bufsize, "%u", sec);
	PFC_ASSERT(len > 0);
	if (PFC_EXPECT_FALSE((size_t)len + 1 >= bufsize)) {
		return;
	}
	buf += len;
	bufsize -= len;
	sep = " (";

	if (day != 0) {
		len = snprintf(buf, bufsize, "%s%u day%s",
			       sep, day, (day == 1) ? str_empty : str_s);
		PFC_ASSERT(len > 0);
		if (PFC_EXPECT_FALSE((size_t)len + 1 >= bufsize)) {
			return;
		}

		buf += len;
		bufsize -= len;
		sep = ", ";
	}

	if (hour != 0 || min != 0 || mod != 0) {
		len = snprintf(buf, bufsize, "%s%02u:%02u:%02u",
			       sep, hour, min, mod);
		PFC_ASSERT(len > 0);
		if (PFC_EXPECT_FALSE((size_t)len + 1 >= bufsize)) {
			return;
		}

		buf += len;
		bufsize -= len;
	}

	PFC_ASSERT(bufsize > 1);
	*buf = ')';
	*(buf + 1) = '\0';
}

/*
 * void
 * pretty_format_bytes(char *buf, size_t bufsize, uint64_t bytes)
 *	Format amount of bytes.
 */
void
pretty_format_bytes(char *buf, size_t bufsize, uint64_t bytes)
{
	uint64_t	quotient;
	uint32_t	nshift;
	const char	*unit;

	if (bytes < KILOBYTES) {
		snprintf(buf, bufsize, "%" PFC_PFMT_u64 " (byte%s)",
			 bytes, ENGLISH_PLUAL(bytes));

		return;
	}

	if (bytes < MEGABYTES) {
		nshift = SHIFT_KILOBYTE;
		unit = "KiB";
	}
	else if (bytes < GIGABYTES) {
		nshift = SHIFT_MEGABYTE;
		unit = "MiB";
	}
	else {
		nshift = SHIFT_GIGABYTE;
		unit = "GiB";
	}

	quotient = bytes >> nshift;
	if (bytes == (quotient << nshift)) {
		snprintf(buf, bufsize, "%" PFC_PFMT_u64
			 " (%" PFC_PFMT_u64 " %s)",
			 bytes, quotient, unit);
	}
	else {
		double	divisor = (double)(PFC_CONST_ULL(1) << nshift);
		double	value = (double)bytes / divisor;

		snprintf(buf, bufsize, "%" PFC_PFMT_u64 " (%.3f %s)",
			 bytes, value, unit);
	}
}

/*
 * static int
 * prompt_read(void)
 *	Read answer of prompt from the standard input.
 *
 * Calling/Exit State:
 *	First character in input line is returned.
 */
static int
prompt_read(void)
{
	int	first = 1, ret = 0;

	for (;;) {
		int	c = getchar();

		if (c == EOF) {
			return c;
		}
		if (c == ' ' || c == '\t') {
			continue;
		}
		if (c == '\n') {
			break;
		}

		if (first) {
			ret = c;
			first = 0;
		}
		else {
			/* Disallow trailing character. */
			ret = 1;
		}
	}

	return ret;
}
