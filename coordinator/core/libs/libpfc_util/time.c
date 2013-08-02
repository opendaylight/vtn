/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * time.c - Timestamp utilities.
 */

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <pfc/util.h>
#include <pfc/clock.h>

/*
 * Convert nanoseconds into microseconds.
 */
#define	NSEC2USEC_DIVISOR	(PFC_CLOCK_NANOSEC / PFC_CLOCK_MICROSEC)
#define	NSEC2USEC(nsec)							\
	(((nsec) + (NSEC2USEC_DIVISOR >> 1)) / NSEC2USEC_DIVISOR)

/*
 * Internal prototypes.
 */
static int	pfc_time_gettm(struct tm *tp);
static int	pfc_time_do_asctime(const struct tm *PFC_RESTRICT tp,
				    char *PFC_RESTRICT buf, size_t bufsize);
static int	pfc_time_do_clock_asctime(const pfc_timespec_t *PFC_RESTRICT
					  clock, char *PFC_RESTRICT buf,
					  size_t bufsize, pfc_bool_t usec);

/*
 * strftime(3) format of system time.
 * It's similar to ISO-8601, but we use whitespace for date and time separator.
 * I don't like 'T' in ISO-8601 date...
 */
static const char	time_format[] = "%Y-%m-%d %H:%M:%S";

/*
 * int
 * pfc_time_ctime(char *buf, size_t bufsize)
 *	Convert the current system time to a string.
 *
 * Calling/Exit State:
 *	Upon successful completion, string representation of the current
 *	system time is stored to `buf', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_time_ctime(char *buf, size_t bufsize)
{
	struct tm	tmbuf;
	int	err;

	err = pfc_time_gettm(&tmbuf);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pfc_time_do_asctime(&tmbuf, buf, bufsize);
	}

	return err;
}

/*
 * int
 * pfc_time_asctime(const struct tm *PFC_RESTRICT tp, char *PFC_RESTRICT buf,
 *		    size_t bufsize)
 *	Convert the system time represented by struct tm to a string.
 *	Returned string contains a date and time information, including
 *	nanosec clock.
 *
 * Calling/Exit State:
 *	Upon successful completion, string representation of the specified
 *	system time is stored to `buf', and zero is returned.
 *	ENOSPC is returned if the specified buffer is too small to store
 *	date string.
 */
int
pfc_time_asctime(const struct tm *PFC_RESTRICT tp, char *PFC_RESTRICT buf,
		 size_t bufsize)
{
	return pfc_time_do_asctime(tp, buf, bufsize);
}

/*
 * int
 * pfc_time_clock_ctime(char *PFC_RESTRICT buf, size_t bufsize)
 *	Convert the current system realtime clock to a string.
 *	Returned string contains a date and time information, including
 *	nanosec clock.
 *
 * Calling/Exit State:
 *	Upon successful completion, string representation of the current
 *	system time is stored to `buf', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_time_clock_ctime(char *PFC_RESTRICT buf, size_t bufsize)
{
	pfc_timespec_t	tspec;
	int	err;

	err = pfc_clock_get_realtime(&tspec);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pfc_time_do_clock_asctime(&tspec, buf, bufsize,
						PFC_FALSE);
	}

	return err;
}

/*
 * int
 * pfc_time_clock_asctime(const pfc_timespec_t *PFC_RESTRICT clock,
 *			  char *PFC_RESTRICT buf, size_t bufsize)
 *	Convert the specified system realtime clock to a string.
 *
 * Calling/Exit State:
 *	Upon successful completion, string representation of the current
 *	system time is stored to `buf', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_time_clock_asctime(const pfc_timespec_t *PFC_RESTRICT clock,
		       char *PFC_RESTRICT buf, size_t bufsize)
{
	return pfc_time_do_clock_asctime(clock, buf, bufsize, PFC_FALSE);
}

/*
 * int
 * pfc_time_mclock_ctime(char *PFC_RESTRICT buf, size_t bufsize)
 *	Convert the current system realtime clock to a string.
 *
 *	This function is same as pfc_time_clock_ctime(), but nanosecond clock
 *	is rounded up to microsecond.
 *
 * Calling/Exit State:
 *	Upon successful completion, string representation of the current
 *	system time is stored to `buf', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_time_mclock_ctime(char *PFC_RESTRICT buf, size_t bufsize)
{
	pfc_timespec_t	tspec;
	int	err;

	err = pfc_clock_get_realtime(&tspec);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pfc_time_do_clock_asctime(&tspec, buf, bufsize,
						PFC_TRUE);
	}

	return err;
}

/*
 * int
 * pfc_time_mclock_asctime(const pfc_timespec_t *PFC_RESTRICT clock,
 *			   char *PFC_RESTRICT buf, size_t bufsize)
 *	Convert the specified system realtime clock to a string.
 *
 *	This function is same as pfc_time_clock_asctime(), but nanosecond clock
 *	is rounded up to microsecond.
 *
 * Calling/Exit State:
 *	Upon successful completion, string representation of the current
 *	system time is stored to `buf', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_time_mclock_asctime(const pfc_timespec_t *PFC_RESTRICT clock,
			char *PFC_RESTRICT buf, size_t bufsize)
{
	return pfc_time_do_clock_asctime(clock, buf, bufsize, PFC_TRUE);
}

/*
 * static int
 * pfc_time_gettime(struct tm *tp)
 *	Get current system time as struct tm.
 *
 * Calling/Exit State:
 *	Upon successful completion, current system time is stored to `tm',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_time_gettime(struct tm *tp)
{
	return pfc_time_gettm(tp);
}

/*
 * static int
 * pfc_time_gettm(struct tm *tp)
 *	Get current system time as struct tm.
 */
static int
pfc_time_gettm(struct tm *tp)
{
	pfc_timespec_t	tspec;
	int	err;

	err = pfc_clock_get_realtime(&tspec);
	if (PFC_EXPECT_TRUE(err == 0)) {
		struct tm	*t = localtime_r(&tspec.tv_sec, tp);

		if (PFC_EXPECT_FALSE(t == NULL)) {
			err = EINVAL;
		}
	}

	return err;
}

/*
 * static int
 * pfc_time_do_asctime(const struct tm *PFC_RESTRICT tp, char *PFC_RESTRICT buf,
 *		       size_t bufsize)
 *	Convert the system time represented by struct tm to a string.
 */
static int
pfc_time_do_asctime(const struct tm *PFC_RESTRICT tp, char *PFC_RESTRICT buf,
		    size_t bufsize)
{
	size_t	sz;

	sz = strftime(buf, bufsize, time_format, tp);
	if (PFC_EXPECT_FALSE(sz == 0)) {
		return ENOSPC;
	}

	return 0;
}

/*
 * static int
 * pfc_time_do_clock_asctime(const pfc_timespec_t *PFC_RESTRICT clock,
 *			     char *PFC_RESTRICT buf, size_t bufsize,
 *			     pfc_bool_t usec)
 *	Convert the system realtime clock specified by pfc_timespec_t
 *	to a string.
 *	If `usec' is PFC_TRUE, tv_nsec value is rounded up to microseconds.
 */
static int
pfc_time_do_clock_asctime(const pfc_timespec_t *PFC_RESTRICT clock,
			  char *PFC_RESTRICT buf, size_t bufsize,
			  pfc_bool_t usec)
{
	struct tm	*tp, tmbuf;
	int	err = 0;

	tp = localtime_r(&clock->tv_sec, &tmbuf);
	if (PFC_EXPECT_TRUE(tp != NULL)) {
		size_t	sz;
		int	ret;

		sz = strftime(buf, bufsize, time_format, &tmbuf);
		if (PFC_EXPECT_FALSE(sz == 0 || sz >= bufsize - 1)) {
			return ENOSPC;
		}

		buf += sz;
		bufsize -= sz;
		if (usec) {
			ret = snprintf(buf, bufsize, ".%06lu",
				       NSEC2USEC(clock->tv_nsec));
		}
		else {
			ret = snprintf(buf, bufsize, ".%09lu", clock->tv_nsec);
		}
		if (PFC_EXPECT_FALSE((size_t)ret >= bufsize)) {
			err = ENOSPC;
		}
	}
	else {
		err = EINVAL;
	}

	return err;
}
