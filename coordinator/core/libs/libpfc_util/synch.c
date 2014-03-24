/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * synch.c - Utilities for thread synchronization.
 */

#include <pfc/synch.h>
#include "synch_impl.h"

#ifndef	PFC_HAVE_POSIX_THREAD
#error	POSIX thread is required.
#endif	/* !PFC_HAVE_POSIX_THREAD */

#ifndef	PFC_HAVE_TIMESPEC
#error	pfc_timespec_t is assumed to be identical to struct timespec.
#endif	/* !PFC_HAVE_TIMESPEC */

#ifdef	PFC_VERBOSE_DEBUG
#define	USE_ERRORCHECK_MUTEX	1	/* Use error-check mutex. */
#else	/* !PFC_VERBOSE_DEBUG */
#define	USE_ERRORCHECK_MUTEX	0
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * int
 * pfc_mutex_init(pfc_mutex_t *mutexp, pfc_mutextype_t type)
 *	Initialize PFC mutex.
 */
int
pfc_mutex_init(pfc_mutex_t *mutexp, pfc_mutextype_t type)
{
	pthread_mutexattr_t	attr;
	int	err;

	/* Initialize mutex attr. */
	err = pthread_mutexattr_init(&attr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if (type == PFC_MUTEX_TYPE_RECURSIVE) {
		/* Recursive mutex */
		err = pthread_mutexattr_settype(&attr,
						PTHREAD_MUTEX_RECURSIVE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}
	}
	else if (USE_ERRORCHECK_MUTEX) {
		/* Error check mutex for debugging. */
		err = pthread_mutexattr_settype(&attr,
						PTHREAD_MUTEX_ERRORCHECK);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}
	}

	err = pthread_mutex_init((pthread_mutex_t *)mutexp, &attr);

out:
	pthread_mutexattr_destroy(&attr);

	return err;
}

/*
 * int
 * __pfc_mutex_lock(pfc_mutex_t *mutexp, const char *file, uint32_t line,
 *		    const char *func)
 *	Acquire the mutex lock, and check errors using assertion.
 */
int
__pfc_mutex_lock(pfc_mutex_t *mutexp, const char *file, uint32_t line,
		 const char *func)
{
	int	err = pthread_mutex_lock((pthread_mutex_t *)mutexp);

	PFC_SYNC_ASSERT(err == 0, file, line, func,
			"pfc_mutex_lock: err = %d", err);

	return err;
}

/*
 * int
 * __pfc_mutex_trylock(pfc_mutex_t *mutexp, const char *file, uint32_t line,
 *		       const char *func)
 *	Try to acquire the mutex lock, and check errors using assertion.
 */
int
__pfc_mutex_trylock(pfc_mutex_t *mutexp, const char *file, uint32_t line,
		    const char *func)
{
	int	err = pthread_mutex_trylock((pthread_mutex_t *)mutexp);

	PFC_SYNC_ASSERT(err == 0 || err == EBUSY, file, line, func,
			"pfc_mutex_trylock: err = %d", err);

	return err;
}

/*
 * int
 * __pfc_mutex_unlock(pfc_mutex_t *mutexp, const char *file, uint32_t line,
 *		      const char *func)
 *	Release the mutex lock, and check errors using assertion.
 */
int
__pfc_mutex_unlock(pfc_mutex_t *mutexp, const char *file, uint32_t line,
		   const char *func)
{
	int	err = pthread_mutex_unlock((pthread_mutex_t *)mutexp);

	PFC_SYNC_ASSERT(err == 0, file, line, func,
			"pfc_mutex_unlock: err = %d", err);

	return err;
}

/*
 * int
 * __pfc_mutex_destroy(pfc_mutex_t *mutexp, const char *file, uint32_t line,
 *		       const char *func)
 *	Destroy the mutex lock, and check errors using assertion.
 */
int
__pfc_mutex_destroy(pfc_mutex_t *mutexp, const char *file, uint32_t line,
		    const char *func)
{
	int	err = pthread_mutex_destroy((pthread_mutex_t *)mutexp);

	PFC_SYNC_ASSERT(err == 0, file, line, func,
			"pfc_mutex_destroy: err = %d", err);

	return err;
}

/*
 * int
 * pfc_rwlock_init(pfc_rwlock_t *rwlockp)
 *	Initialize PFC read/write lock.
 */
int
pfc_rwlock_init(pfc_rwlock_t *rwlockp)
{
	return pthread_rwlock_init((pthread_rwlock_t *)rwlockp, NULL);
}

/*
 * int
 * pfc_cond_timedwait(pfc_cond_t *PFC_RESTRICT condp,
 *		      pfc_mutex_t *PFC_RESTRICT mutexp,
 *		      const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Block on the condition variable, within the specified timeout period.
 *	Unlike POSIX thread interface, timeout is specified by time period,
 *	not absolute time.
 */
int
pfc_cond_timedwait(pfc_cond_t *PFC_RESTRICT condp,
		   pfc_mutex_t *PFC_RESTRICT mutexp,
		   const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	abstime;
	int	err;

	if (PFC_EXPECT_FALSE(timeout == NULL)) {
		return pfc_cond_wait(condp, mutexp);
	}

	/* Convert timeout value to absolute system time. */
	err = pfc_clock_abstime(&abstime, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	return pthread_cond_timedwait((pthread_cond_t *)condp,
				      (pthread_mutex_t *)mutexp, &abstime);
}

/*
 * int
 * pfc_cond_timedwait_abs(pfc_cond_t *PFC_RESTRICT condp,
 *			  pfc_mutex_t *PFC_RESTRICT mutexp,
 *			  const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Block on the condition variable, until the given absolute system time.
 */
int
pfc_cond_timedwait_abs(pfc_cond_t *PFC_RESTRICT condp,
		       pfc_mutex_t *PFC_RESTRICT mutexp,
		       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	if (PFC_EXPECT_FALSE(abstime == NULL)) {
		return pfc_cond_wait(condp, mutexp);
	}

	return pthread_cond_timedwait((pthread_cond_t *)condp,
				      (pthread_mutex_t *)mutexp,
				      (const struct timespec *)abstime);
}

/*
 * int
 * pfc_sem_init(pfc_sem_t *semp, unsigned int value)
 *	Initialize PFC semaphore.
 */
int
pfc_sem_init(pfc_sem_t *semp, unsigned int value)
{
	return sem_init((sem_t *)semp, 0, value);
}

#ifndef	PFC_SYNC_ASSERT
/*
 * void PFC_ATTR_HIDDEN
 * PFC_SYNC_ASSERT(int ex, const char *file, uint32_t line, const char *func,
 *		   const char *fmt, ...)
 *	Internal assertion for non-GNU C compiler.
 *
 *	If `ex' is false, print error message specified by `fmt' in printf(3)
 *	format to the standard error output, and raise SIGABRT.
 */
void PFC_ATTR_HIDDEN
PFC_SYNC_ASSERT(int ex, const char *file, uint32_t line, const char *func,
		const char *fmt, ...)
{
	if (PFC_EXPECT_FALSE(!ex)) {
		va_list	ap;

		va_start(ap, fmt);
		__pfc_assfail_vprintf(file, line, func, fmt, ap);
		va_end(ap);
		/* NOTREACHED */
	}
}
#endif	/* !PFC_SYNC_ASSERT */
