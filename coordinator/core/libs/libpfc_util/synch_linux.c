/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * synch_linux.c - Utilities for thread synchronization. (Linux specific)
 *
 * Remarks:
 *	On Linux system, default PFC clock is monotonic clock.
 *	Although pthread_cond_t in glibc supports monotonic clock,
 *	other features, such as pthread_mutex_timedlock(), don't support
 *	monotonic clock. So we need to convert the given monotonic absolute
 *	time into realtime before calling those interfaces.
 */

#include <pfc/synch.h>
#include "synch_impl.h"

/*
 * int
 * pfc_cond_init(pfc_cond_t *condp)
 *	Initialize PFC condition variable.
 */
int
pfc_cond_init(pfc_cond_t *condp)
{
	pthread_condattr_t	cattr;
	int	err;

	/* Initialize condition variable attribute. */
	err = pthread_condattr_init(&cattr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Use monotonic clock. */
	err = pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pthread_cond_init((pthread_cond_t *)condp, &cattr);
	}

	pthread_condattr_destroy(&cattr);

	return err;
}

/*
 * int
 * __pfc_mutex_timedlock(pfc_mutex_t *PFC_RESTRICT mutexp,
 *			 const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Try to acquire mutex within the specified timeout period.
 *	Unlike POSIX thread interface, timeout is specified by time period,
 *	not absolute time.
 */
int
__pfc_mutex_timedlock(pfc_mutex_t *PFC_RESTRICT mutexp,
		      const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	abstime;
	int	err;

	if (PFC_EXPECT_FALSE(timeout == NULL)) {
		return pfc_mutex_lock(mutexp);
	}

	/* Convert timeout value to absolute system time (realtime). */
	err = pfc_clock_real_abstime(&abstime, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	return pthread_mutex_timedlock((pthread_mutex_t *)mutexp, &abstime);
}

/*
 * int
 * __pfc_mutex_timedlock_errchk(pfc_mutex_t *PFC_RESTRICT mutexp,
 *				const pfc_timespec_t *PFC_RESTRICT timeout,
 *				const char *file, uint32_t line,
 *				const char *func)
 *	Try to acquire mutex within the specified timeout period,
 *	and verify error state.
 */
int
__pfc_mutex_timedlock_errchk(pfc_mutex_t *PFC_RESTRICT mutexp,
			     const pfc_timespec_t *PFC_RESTRICT timeout,
			     const char *file, uint32_t line, const char *func)
{
	pfc_timespec_t	abstime;
	int	err;

	if (PFC_EXPECT_FALSE(timeout == NULL)) {
		return pfc_mutex_lock(mutexp);
	}

	/* Convert timeout value to absolute system time (realtime). */
	err = pfc_clock_real_abstime(&abstime, timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pthread_mutex_timedlock((pthread_mutex_t *)mutexp,
					      &abstime);
	}

	PFC_SYNC_ASSERT(err == 0 || err == ETIMEDOUT, file, line, func,
			"pfc_mutex_timedlock: err = %d", err);

	return err;
}

/*
 * int
 * __pfc_mutex_timedlock_abs(pfc_mutex_t *PFC_RESTRICT mutexp,
 *			     const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Try to acquire mutex until the given absolute system time.
 */
int
__pfc_mutex_timedlock_abs(pfc_mutex_t *PFC_RESTRICT mutexp,
			  const pfc_timespec_t *PFC_RESTRICT abstime)
{
	pfc_timespec_t	real;
	pthread_mutex_t	*mtx = (pthread_mutex_t *)mutexp;
	const struct timespec	*tspec = &real;
	int	err;

	if (PFC_EXPECT_FALSE(abstime == NULL)) {
		return pfc_mutex_lock(mutexp);
	}

	/* Convert monotonic clock into realtime clock. */
	err = pfc_clock_mono2real(&real, abstime);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pthread_mutex_timedlock(mtx, tspec);
	}

	return err;
}

/*
 * int
 * __pfc_mutex_timedlock_abs_errchk(pfc_mutex_t *PFC_RESTRICT mutexp,
 *				    const pfc_timespec_t *PFC_RESTRICT abstime,
 *				    const char *file, uint32_t line,
 *				    const char *func)
 *	Try to acquire mutex until the given absolute system time,
 *	and verify error state.
 */
int
__pfc_mutex_timedlock_abs_errchk(pfc_mutex_t *PFC_RESTRICT mutexp,
				 const pfc_timespec_t *PFC_RESTRICT abstime,
				 const char *file, uint32_t line,
				 const char *func)
{
	pfc_timespec_t	real;
	pthread_mutex_t	*mtx = (pthread_mutex_t *)mutexp;
	const struct timespec	*tspec = &real;
	int	err;

	if (PFC_EXPECT_FALSE(abstime == NULL)) {
		return pfc_mutex_lock(mutexp);
	}

	/* Convert monotonic clock into realtime clock. */
	err = pfc_clock_mono2real(&real, abstime);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pthread_mutex_timedlock(mtx, tspec);
	}

	PFC_SYNC_ASSERT(err == 0 || err == ETIMEDOUT, file, line, func,
			"pfc_mutex_timedlock_abs: err = %d", err);

	return err;
}

/*
 * int
 * pfc_rwlock_timedrdlock(pfc_rwlock_t *PFC_RESTRICT rwlockp,
 *			  const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Try to acquire read lock within the specified timeout period.
 *	Unlike POSIX thread interface, timeout is specified by time period,
 *	not absolute time.
 */
int
pfc_rwlock_timedrdlock(pfc_rwlock_t *PFC_RESTRICT rwlockp,
		       const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	abstime;
	int	err;

	if (PFC_EXPECT_FALSE(timeout == NULL)) {
		return pfc_rwlock_rdlock(rwlockp);
	}

	/* Convert timeout value to absolute system time (realtime). */
	err = pfc_clock_real_abstime(&abstime, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	return pthread_rwlock_timedrdlock((pthread_rwlock_t *)rwlockp,
					  &abstime);
}

/*
 * int
 * pfc_rwlock_timedwrlock(pfc_rwlock_t *PFC_RESTRICT rwlockp,
 *			  const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Try to acquire write lock within the specified timeout period.
 *	Unlike POSIX thread interface, timeout is specified by time period,
 *	not absolute time.
 */
int
pfc_rwlock_timedwrlock(pfc_rwlock_t *PFC_RESTRICT rwlockp,
		       const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	abstime;
	int	err;

	if (PFC_EXPECT_FALSE(timeout == NULL)) {
		return pfc_rwlock_wrlock(rwlockp);
	}

	/* Convert timeout value to absolute system time (realtime). */
	err = pfc_clock_real_abstime(&abstime, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	return pthread_rwlock_timedwrlock((pthread_rwlock_t *)rwlockp,
					  &abstime);
}

/*
 * int
 * pfc_rwlock_timedrdlock_abs(pfc_rwlock_t *PFC_RESTRICT rwlockp,
 *			      const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Try to acquire read lock until the given absolute system time.
 */
int
pfc_rwlock_timedrdlock_abs(pfc_rwlock_t *PFC_RESTRICT rwlockp,
			   const pfc_timespec_t *PFC_RESTRICT abstime)
{
	pfc_timespec_t	real;
	pthread_rwlock_t	*rw = (pthread_rwlock_t *)rwlockp;
	const struct timespec	*tspec = &real;
	int	err;

	if (PFC_EXPECT_FALSE(abstime == NULL)) {
		return pfc_rwlock_rdlock(rwlockp);
	}

	/* Convert monotonic clock into realtime clock. */
	err = pfc_clock_mono2real(&real, abstime);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pthread_rwlock_timedrdlock(rw, tspec);
	}

	return err;
}

/*
 * int
 * pfc_rwlock_timedwrlock_abs(pfc_rwlock_t *PFC_RESTRICT rwlockp,
 *			      const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Try to acquire write lock until the given absolute system time.
 */
int
pfc_rwlock_timedwrlock_abs(pfc_rwlock_t *PFC_RESTRICT rwlockp,
			   const pfc_timespec_t *PFC_RESTRICT abstime)
{
	pfc_timespec_t	real;
	pthread_rwlock_t	*rw = (pthread_rwlock_t *)rwlockp;
	const struct timespec	*tspec = &real;
	int	err;

	if (PFC_EXPECT_FALSE(abstime == NULL)) {
		return pfc_rwlock_wrlock(rwlockp);
	}

	/* Convert monotonic clock into realtime clock. */
	err = pfc_clock_mono2real(&real, abstime);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pthread_rwlock_timedwrlock(rw, tspec);
	}

	return err;
}

/*
 * int
 * pfc_sem_timedwait(pfc_sem_t *PFC_RESTRICT semp,
 *		     const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Try to lock the semaphore within the specified timeout period
 *	Unlike POSIX thread interface, timeout is specified by time period,
 *	not absolute time.
 */
int
pfc_sem_timedwait(pfc_sem_t *PFC_RESTRICT semp,
		  const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	abstime;
	int	err;

	if (PFC_EXPECT_FALSE(timeout == NULL)) {
		return pfc_sem_wait(semp);
	}

	/* Convert timeout value to absolute system time (realtime). */
	err = pfc_clock_real_abstime(&abstime, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		errno = err;

		return -1;
	}

	return sem_timedwait((sem_t *)semp, &abstime);
}

/*
 * int
 * pfc_sem_timedwait_abs(pfc_sem_t *PFC_RESTRICT semp,
 *			 const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Try to lock the semaphore until the given absolute system time.
 */
int
pfc_sem_timedwait_abs(pfc_sem_t *PFC_RESTRICT semp,
		      const pfc_timespec_t *PFC_RESTRICT abstime)
{
	pfc_timespec_t	real;
	const struct timespec	*tspec = &real;
	int	err;

	if (PFC_EXPECT_FALSE(abstime == NULL)) {
		return pfc_sem_wait(semp);
	}

	/* Convert monotonic clock into realtime clock. */
	err = pfc_clock_mono2real(&real, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		errno = err;

		return -1;
	}

	return sem_timedwait((sem_t *)semp, tspec);
}
