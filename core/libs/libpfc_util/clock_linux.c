/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * clock_linux.c - System clock management. Linux specific.
 */

#include <pfc/clock.h>
#include <pfc/debug.h>
#include "util_impl.h"

/*
 * static inline int
 * pfc_get_monotonic_clock(pfc_timespec_t *tsp)
 *	Obtain monotonic system clock.
 *
 * Calling/Exit State:
 *	Upon successful completion, current monotonic system clock is set to
 *	`*tsp', and then zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int
pfc_get_monotonic_clock(pfc_timespec_t *tsp)
{
	int	err = clock_gettime(CLOCK_MONOTONIC, (struct timespec *)tsp);

	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
	}
	else {
		PFC_ASSERT(tsp->tv_sec >= 0);
		PFC_ASSERT((pfc_ulong_t)tsp->tv_nsec < PFC_CLOCK_NANOSEC);
	}

	return err;
}

/*
 * static inline int
 * pfc_get_realtime_clock(pfc_timespec_t *tsp)
 *	Obtain realtime system clock.
 *
 * Calling/Exit State:
 *	Upon successful completion, current realtime system clock is set to
 *	`*tsp', and then zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int
pfc_get_realtime_clock(pfc_timespec_t *tsp)
{
	int	err = clock_gettime(CLOCK_REALTIME, (struct timespec *)tsp);

	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
	}
	else {
		PFC_ASSERT(tsp->tv_sec >= 0);
		PFC_ASSERT((pfc_ulong_t)tsp->tv_nsec < PFC_CLOCK_NANOSEC);
	}

	return err;
}

/*
 * static inline void
 * pfc_timespec_fixup(pfc_timespec_t *tsp)
 *	Adjust tv_sec and tv_nsec field in the specified timespec.
 */
static inline void
pfc_timespec_fixup(pfc_timespec_t *tsp)
{
	pfc_ulong_t	div;

	div = (pfc_ulong_t)tsp->tv_nsec / PFC_CLOCK_NANOSEC;
	tsp->tv_sec += div;
	tsp->tv_nsec -= (div * PFC_CLOCK_NANOSEC);
}

/*
 * static inline void
 * __pfc_timespec_add(pfc_timespec_t *PFC_RESTRICT augend,
 *		      const pfc_timespec_t *PFC_RESTRICT addend)
 *	Add timespec value `addend' to `augend'.
 */
static inline void
__pfc_timespec_add(pfc_timespec_t *PFC_RESTRICT augend,
		   const pfc_timespec_t *PFC_RESTRICT addend)
{
	augend->tv_sec += addend->tv_sec;
	augend->tv_nsec += addend->tv_nsec;
	pfc_timespec_fixup(augend);
}

/*
 * static inline void
 * __pfc_timespec_sub(pfc_timespec_t *PFC_RESTRICT minuend,
 *		      const pfc_timespec_t *PFC_RESTRICT sub)
 *	Subtract timespec value `sub' from `minuend'.
 */
static inline void
__pfc_timespec_sub(pfc_timespec_t *PFC_RESTRICT minuend,
		   const pfc_timespec_t *PFC_RESTRICT sub)
{
	pfc_ulong_t	nsec;

	minuend->tv_sec -= sub->tv_sec;
	nsec = minuend->tv_nsec - sub->tv_nsec;
	if ((pfc_ulong_t)minuend->tv_nsec < (pfc_ulong_t)sub->tv_nsec) {
		nsec += PFC_CLOCK_NANOSEC;
		minuend->tv_sec--;
	}

	minuend->tv_nsec = nsec;
	pfc_timespec_fixup(minuend);
}

/*
 * static inline int
 * pfc_clock_do_compare(const pfc_timespec_t *tsp1, const pfc_timespec_t *sp2)
 *	Compare the given times.
 *	See comments on pfc_clock_compare().
 */
static inline int
pfc_clock_do_compare(const pfc_timespec_t *tsp1, const pfc_timespec_t *tsp2)
{
	pfc_ulong_t	sec1 = (pfc_ulong_t)tsp1->tv_sec;
	pfc_ulong_t	sec2 = (pfc_ulong_t)tsp2->tv_sec;

	if (sec1 == sec2) {
		pfc_ulong_t	nsec1 = (pfc_ulong_t)tsp1->tv_nsec;
		pfc_ulong_t	nsec2 = (pfc_ulong_t)tsp2->tv_nsec;

		if (nsec1 == nsec2) {
			return 0;
		}

		return (nsec1 < nsec2) ? -1 : 1;
	}

	return (sec1 < sec2) ? -1 : 1;
}

/*
 * static inline int
 * pfc_clock_is_valid(const pfc_timespec_t *interval)
 *	Determine whether the specified timespec is valid or not.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified timespec is suitable for time
 *	interval.
 *	Otherwise EINVAL is returned.
 */
static inline int
pfc_clock_is_valid(const pfc_timespec_t *interval)
{
	if (PFC_EXPECT_FALSE(!PFC_CLOCK_IS_VALID(interval))) {
		return EINVAL;
	}

	return 0;
}

/*
 * int
 * pfc_clock_gettime(pfc_timespec_t *tsp)
 *	Retrieve the current PFC system clock.
 *	On Linux, this function returns system monotonic clock.
 */
int
pfc_clock_gettime(pfc_timespec_t *tsp)
{
	return pfc_get_monotonic_clock(tsp);
}

/*
 * int
 * pfc_clock_get_realtime(pfc_timespec_t *tsp)
 *	Retrieve the current realtime clock.
 */
int
pfc_clock_get_realtime(pfc_timespec_t *tsp)
{
	return pfc_get_realtime_clock(tsp);
}

/*
 * int
 * pfc_clock_abstime(pfc_timespec_t *PFC_RESTRICT abstime,
 *		     const pfc_timespec_t *PFC_RESTRICT interval)
 *	Convert time interval to absolute system time from now.
 *	Note that returned absolute time is monotonic clock, not realtime.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted absolute time is set into
 *	`*abstime', and then zero is returned.
 *
 *	EINVAL is returned if the given `interval' is invalid.
 *	ERANGE is returned if adding the given `interval' to the current time
 *	causes integer overflow.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function guarantees that abstime->tv_sec keeps positive value.
 *
 *	- This function does not work if the current monotonic clock is
 *	  negative.
 */
int
pfc_clock_abstime(pfc_timespec_t *PFC_RESTRICT abstime,
		  const pfc_timespec_t *PFC_RESTRICT interval)
{
	int	err;

	err = pfc_clock_is_valid(interval);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = pfc_get_monotonic_clock(abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	__pfc_timespec_add(abstime, interval);

	if (PFC_EXPECT_FALSE(abstime->tv_sec < 0)) {
		err = ERANGE;
	}

	return err;
}

/*
 * int
 * pfc_clock_real_abstime(pfc_timespec_t *PFC_RESTRICT abstime,
 *		     const pfc_timespec_t *PFC_RESTRICT interval)
 *	Convert time interval to absolute system time from now.
 *	Note that returned absolute time is realtime clock.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted absolute time is set into
 *	`*abstime', and then zero is returned.
 *
 *	EINVAL is returned if the given `interval' is invalid.
 *	ERANGE is returned if adding the given `interval' to the current time
 *	causes integer overflow.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function guarantees that abstime->tv_sec keeps positive value.
 *
 *	- This function does not work if the current realtime clock is
 *	  negative.
 */
int
pfc_clock_real_abstime(pfc_timespec_t *PFC_RESTRICT abstime,
		       const pfc_timespec_t *PFC_RESTRICT interval)
{
	int	err;

	err = pfc_clock_is_valid(interval);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = pfc_get_realtime_clock(abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	__pfc_timespec_add(abstime, interval);

	if (PFC_EXPECT_FALSE(abstime->tv_sec < 0)) {
		err = ERANGE;
	}

	return err;
}

/*
 * int
 * pfc_clock_mono2real(pfc_timespec_t *PFC_RESTRICT result,
 *		       const pfc_timespec_t *PFC_RESTRICT mono)
 *	Convert monotonic clock to realtime clock.
 *
 * Calling/Exit State:
 *	Upon successful completion, converted absolute time is set into
 *	`*abstime', and then zero is returned.
 *
 *	EINVAL is returned if the given `interval' is invalid.
 *	ERANGE is returned if adding the given `interval' to the current time
 *	causes integer overflow.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function guarantees that result->tv_sec keeps positive value.
 *
 *	- This function does not work if the current monotonic or realtime
 *	  clock is negative.
 */
int
pfc_clock_mono2real(pfc_timespec_t *PFC_RESTRICT result,
		    const pfc_timespec_t *PFC_RESTRICT mono)
{
	pfc_timespec_t	curmono;
	int	err;

	err = pfc_clock_is_valid(mono);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = pfc_get_monotonic_clock(&curmono);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = pfc_get_realtime_clock(result);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	__pfc_timespec_sub(&curmono, mono);
	__pfc_timespec_sub(result, &curmono);

	if (PFC_EXPECT_FALSE(result->tv_sec < 0)) {
		err = ERANGE;
	}

	return err;
}

/*
 * int
 * pfc_clock_compare(const pfc_timespec_t *tsp1, const pfc_timespec_t *tsp2)
 *	Compare the given two pfc_timespec_t values.
 *	This function acts just like comparator of qsort(3).
 *
 * Calling/Exit State:
 *	If the given two times are identical, zero is returned.
 *	If the first argument is less than the second, a negative value is
 *	returned.
 *	If the first argument is greater than the second, a positive value is
 *	returned.
 */
int
pfc_clock_compare(const pfc_timespec_t *tsp1, const pfc_timespec_t *tsp2)
{
	return pfc_clock_do_compare(tsp1, tsp2);
}

/*
 * int
 * pfc_clock_isexpired(pfc_timespec_t *PFC_RESTRICT remains,
 *		       const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Determine whether the current system time exceeds the given
 *	absolute time or not.
 *
 * Calling/Exit State:
 *	If the current system time is less than the given absolute time,
 *	a difference between the absolute time and the current time is set to
 *	`*remains', and zero is returned.
 *
 *	If the current time equals or exceeds the given absolute time,
 *	ETIMEDOUT is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_clock_isexpired(pfc_timespec_t *PFC_RESTRICT remains,
		    const pfc_timespec_t *PFC_RESTRICT abstime)
{
	pfc_timespec_t	cur;
	int	err = pfc_get_monotonic_clock(&cur);

	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if (pfc_clock_do_compare(&cur, abstime) >= 0) {
		return ETIMEDOUT;
	}

	*remains = *abstime;
	__pfc_timespec_sub(remains, &cur);

	return 0;
}

/*
 * void
 * pfc_timespec_add(pfc_timespec_t *PFC_RESTRICT augend,
 *		    const pfc_timespec_t *PFC_RESTRICT addend)
 *	Add timespec value `addend' to `augend'.
 */
void
pfc_timespec_add(pfc_timespec_t *PFC_RESTRICT augend,
		 const pfc_timespec_t *PFC_RESTRICT addend)
{
	return __pfc_timespec_add(augend, addend);
}

/*
 * void
 * pfc_timespec_sub(pfc_timespec_t *PFC_RESTRICT minuend,
 *		    const pfc_timespec_t *PFC_RESTRICT sub)
 *	Subtract timespec value `sub' from `minuend'.
 */
void
pfc_timespec_sub(pfc_timespec_t *PFC_RESTRICT minuend,
		 const pfc_timespec_t *PFC_RESTRICT sub)
{
	return __pfc_timespec_sub(minuend, sub);
}

/*
 * void
 * pfc_clock_msec2time(pfc_timespec_t *tsp, uint64_t msec)
 *	Convert amount of time in milliseconds into pfc_timespec_t.
 *
 * Calling/Exit State:
 *	Time in milliseconds specified by `msec' is converted to
 *	pfc_timespec_t, and it is stored to the buffer pointed by `tsp'.
 *
 * Remarks:
 *	- Specifying NULL to `tsp' causes undefined behavior.
 *
 *	- Any error check is not implemented. This function may generate
 *	  incorrect value if too large value is specified to `msec'.
 */
void
pfc_clock_msec2time(pfc_timespec_t *tsp, uint64_t msec)
{
	uint64_t	seconds, mod;

	seconds = msec / PFC_CLOCK_MILLISEC;
	mod = msec - (seconds * PFC_CLOCK_MILLISEC);

	tsp->tv_sec = seconds;
	tsp->tv_nsec = mod * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);
}

/*
 * uint64_t
 * pfc_clock_time2msec(const pfc_timespec_t *tsp)
 *	Convert pfc_timespec_t specified by `tsp' into milliseconds.
 *
 * Calling/Exit State:
 *	This function always returns converted value in milliseconds.
 *
 * Remarks:
 *	- Specifying NULL to `tsp' causes undefined behavior.
 *
 *	- tv_nsec is rounded off to milliseconds order.
 *
 *	- The caller must ensure pfc_timespec_t pointed by `tsp' keeps valid
 *	  value.
 */
uint64_t
pfc_clock_time2msec(const pfc_timespec_t *tsp)
{
	const uint64_t	divisor = (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);
	uint64_t	msec;

	msec = ((uint64_t)tsp->tv_sec * PFC_CLOCK_MILLISEC) +
		((tsp->tv_nsec + (divisor >> 1)) / divisor);

	return msec;
}
