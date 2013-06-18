/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_CLOCK_H
#define	_PFC_CLOCK_H

/*
 * Definitions for system clock utilities.
 * Linux specific.
 */

#include <pfc/base.h>
#include <errno.h>
#ifdef	PFC_HAVE_TIMESPEC
#include <time.h>
#endif	/* PFC_HAVE_TIMESPEC */

PFC_C_BEGIN_DECL

/*
 * Common clock resolutions.
 */
#define	PFC_CLOCK_SECOND	PFC_CONST_U(1)
#define	PFC_CLOCK_MILLISEC	PFC_CONST_U(1000)
#define	PFC_CLOCK_MICROSEC	PFC_CONST_U(1000000)
#define	PFC_CLOCK_NANOSEC	PFC_CONST_U(1000000000)

/* PFC timespec data type. */

#ifdef	PFC_HAVE_TIMESPEC
typedef struct timespec		pfc_timespec_t;
#else	/* !PFC_HAVE_TIMESPEC */
#error	pfc_timespec_t must be defined here.
#endif	/* PFC_HAVE_TIMESPEC */

/*
 * Determine whether the given timespec is valid or not.
 * NULL must not be specified.
 */
#define	PFC_CLOCK_IS_VALID(tsp)						\
	((tsp)->tv_sec >= 0 && (pfc_ulong_t)(tsp)->tv_nsec < PFC_CLOCK_NANOSEC)

/*
 * Prototypes.
 */
extern int	pfc_clock_gettime(pfc_timespec_t *tsp);
extern int	pfc_clock_abstime(pfc_timespec_t *PFC_RESTRICT abstime,
				  const pfc_timespec_t *PFC_RESTRICT interval);
extern int	pfc_clock_compare(const pfc_timespec_t *tsp1,
				  const pfc_timespec_t *tsp2);
extern int	pfc_clock_isexpired(pfc_timespec_t *PFC_RESTRICT remains,
				    const pfc_timespec_t *PFC_RESTRICT abstime);
extern void	pfc_timespec_add(pfc_timespec_t *PFC_RESTRICT augend,
				 const pfc_timespec_t *PFC_RESTRICT addend);
extern void	pfc_timespec_sub(pfc_timespec_t *PFC_RESTRICT minuend,
				 const pfc_timespec_t *PFC_RESTRICT sub);
extern void	pfc_clock_msec2time(pfc_timespec_t *tsp, uint64_t msec);
extern uint64_t	pfc_clock_time2msec(const pfc_timespec_t *tsp);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_clock_sec2time(pfc_timespec_t *tsp, time_t sec)
 *	Convert amount of time in seconds into pfc_timespec_t.
 *
 * Calling/Exit State:
 *	Time in seconds specified by `sec' is converted to pfc_timespec_t,
 *	and it is stored to the buffer pointed by `tsp'.
 *
 * Remarks:
 *	- Specifying NULL to `tsp' causes undefined behavior.
 *
 *	- The caller must ensure that `sec' is greater than or equal to zero.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_clock_sec2time(pfc_timespec_t *tsp, time_t sec)
{
	tsp->tv_sec = sec;
	tsp->tv_nsec = 0;
}

/*
 * static inline pfc_ulong_t PFC_FATTR_ALWAYS_INLINE
 * pfc_clock_time2sec(const pfc_timespec_t *tsp)
 *	Convert pfc_timespec_t specified by `tsp' into seconds.
 *
 * Calling/Exit State:
 *	This function always returns converted value in seconds.
 *
 * Remarks:
 *	- Specifying NULL to `tsp' causes undefined behavior.
 *
 *	- tv_nsec is rounded off to second order.
 *
 *	- The caller must ensure pfc_timespec_t pointed by `tsp' keeps valid
 *	  value. For instance, this function will return incorrect value
 *	  if `tsp->tv_nsec' is greater than or equal to PFC_CLOCK_NANOSEC.
 */
static inline pfc_ulong_t PFC_FATTR_ALWAYS_INLINE
pfc_clock_time2sec(const pfc_timespec_t *tsp)
{
	pfc_ulong_t	sec = (pfc_ulong_t)tsp->tv_sec;

	if ((pfc_ulong_t)tsp->tv_nsec >= (PFC_CLOCK_NANOSEC >> 1)) {
		sec++;
	}

	return sec;
}

#ifdef	PFC_USE_CLOCK_MONOTONIC

/*
 * PFC system clock is monotonic clock. APIs to handle system realtime clock
 * should be provided for system APIs that don't support monotonic clock.
 */
extern int	pfc_clock_get_realtime(pfc_timespec_t *tsp);
extern int	pfc_clock_real_abstime(pfc_timespec_t *PFC_RESTRICT abstime,
				       const pfc_timespec_t
				       *PFC_RESTRICT interval);
extern int	pfc_clock_mono2real(pfc_timespec_t *PFC_RESTRICT result,
				    const pfc_timespec_t *PFC_RESTRICT mono);

#else	/* !PFC_USE_CLOCK_MONOTONIC */

/*
 * PFC system clock is realtime clock.
 */

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_clock_get_realtime(pfc_timespec_t *tsp)
{
	return pfc_clock_gettime(tsp);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_clock_real_abstime(pfc_timespec_t *PFC_RESTRICT abstime,
		       const pfc_timespec_t *PFC_RESTRICT interval)
{
	return pfc_clock_abstime(abstime, interval);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_clock_mono2real(pfc_timespec_t *PFC_RESTRICT result,
		    const pfc_timespec_t *PFC_RESTRICT mono)
{
	*result = *mono;
}

#endif	/* PFC_USE_CLOCK_MONOTONIC */

PFC_C_END_DECL

#endif	/* !_PFC_CLOCK_H */
