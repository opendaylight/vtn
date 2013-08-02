/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_SYNCH_H
#define	_PFC_SYNCH_H

/*
 * Definitions for thread synchronization utilities.
 */

#include <pfc/base.h>
#include <pfc/clock.h>
#include <pfc/debug.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

PFC_C_BEGIN_DECL

/*
 * Mutex type.
 */
typedef pthread_mutex_t		pfc_mutex_t;

/*
 * Read/Write lock.
 */
typedef pthread_rwlock_t	pfc_rwlock_t;

/*
 * Condition variable.
 */
typedef pthread_cond_t		pfc_cond_t;

/*
 * Semaphore.
 */
typedef sem_t			pfc_sem_t;

/*
 * Static mutex initializer.
 * Define error-check mutex as default mutex type if PFC_VERBOSE_DEBUG
 * is defined.
 */
#ifdef	PFC_VERBOSE_DEBUG
#ifdef	PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
#define	PFC_MUTEX_INITIALIZER	PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
#endif	/* PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP */
#endif	/* PFC_VERBOSE_DEBUG */

#ifndef	PFC_MUTEX_INITIALIZER
#define	PFC_MUTEX_INITIALIZER	PTHREAD_MUTEX_INITIALIZER
#endif	/* !PFC_MUTEX_INITIALIZER */

/*
 * Static rwlock initializer.
 */
#define	PFC_RWLOCK_INITIALIZER	PTHREAD_RWLOCK_INITIALIZER

/*
 * PFC mutex type.
 */
typedef enum {
	PFC_MUTEX_TYPE_NORMAL	= 0,		/* normal mutex */
	PFC_MUTEX_TYPE_RECURSIVE,		/* recursive mutex */
} pfc_mutextype_t;

/*
 * PFC mutex APIs.
 */
extern int	pfc_mutex_init(pfc_mutex_t *mutexp, pfc_mutextype_t type);
extern int	__pfc_mutex_timedlock(pfc_mutex_t *PFC_RESTRICT mutexp,
				      const pfc_timespec_t
				      *PFC_RESTRICT timeout);
extern int	__pfc_mutex_timedlock_errchk(pfc_mutex_t *PFC_RESTRICT mutexp,
					     const pfc_timespec_t
					     *PFC_RESTRICT timeout,
					     const char *file, uint32_t line,
					     const char *func);
extern int	__pfc_mutex_timedlock_abs(pfc_mutex_t *PFC_RESTRICT mutexp,
					  const pfc_timespec_t
					  *PFC_RESTRICT abstime);
extern int	__pfc_mutex_timedlock_abs_errchk(pfc_mutex_t *PFC_RESTRICT
						 mutexp,
						 const pfc_timespec_t
						 *PFC_RESTRICT abstime,
						 const char *file,
						 uint32_t line,
						 const char *func);

#ifdef	PFC_VERBOSE_DEBUG
#define	pfc_mutex_timedlock(mutexp, timeout)				\
	__pfc_mutex_timedlock_errchk(mutexp, timeout, __FILE__, __LINE__, \
				     PFC_ASSERT_FUNCNAME)
#define	pfc_mutex_timedlock_abs(mutexp, abstime)			\
	__pfc_mutex_timedlock_abs_errchk(mutexp, abstime, __FILE__,	\
					 __LINE__, PFC_ASSERT_FUNCNAME)
#else	/* !PFC_VERBOSE_DEBUG */
#define	pfc_mutex_timedlock(mutexp, timeout)		\
	__pfc_mutex_timedlock(mutexp, timeout)
#define	pfc_mutex_timedlock_abs(mutexp, timeout)	\
	__pfc_mutex_timedlock_abs(mutexp, timeout)
#endif	/* PFC_VERBOSE_DEBUG */

/* Initialize mutex with default parameter. */
#define	PFC_MUTEX_INIT(mutexp)	pfc_mutex_init(mutexp, PFC_MUTEX_TYPE_NORMAL)

#ifdef	PFC_VERBOSE_DEBUG

/*
 * Error of mutex operations are checked by assertion.
 */
extern int	__pfc_mutex_lock(pfc_mutex_t *mutexp, const char *file,
				 uint32_t line, const char *func);
extern int	__pfc_mutex_trylock(pfc_mutex_t *mutexp, const char *file,
				    uint32_t line, const char *func);
extern int	__pfc_mutex_unlock(pfc_mutex_t *mutexp, const char *file,
				   uint32_t line, const char *func);
extern int	__pfc_mutex_destroy(pfc_mutex_t *mutexp, const char *file,
				    uint32_t line, const char *func);

#define	pfc_mutex_lock(mutexp)						\
	__pfc_mutex_lock(mutexp, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME)
#define	pfc_mutex_trylock(mutexp)					\
	__pfc_mutex_trylock(mutexp, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME)
#define	pfc_mutex_unlock(mutexp)					\
	__pfc_mutex_unlock(mutexp, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME)
#define	pfc_mutex_destroy(mutexp)					\
	__pfc_mutex_destroy(mutexp, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME)

#else	/* !PFC_VERBOSE_DEBUG */

#define	__PFC_MUTEX_API_DECL(func)					\
	static inline int PFC_FATTR_ALWAYS_INLINE			\
	pfc_mutex_##func(pfc_mutex_t *mutexp)				\
	{								\
		return pthread_mutex_##func((pthread_mutex_t *)mutexp);	\
	}

__PFC_MUTEX_API_DECL(lock)
__PFC_MUTEX_API_DECL(trylock)
__PFC_MUTEX_API_DECL(unlock)
__PFC_MUTEX_API_DECL(destroy)

#endif	/* PFC_VERBOSE_DEBUG */

/*
 * PFC rwlock APIs.
 */
extern int	pfc_rwlock_init(pfc_rwlock_t *rwlockp);
extern int	pfc_rwlock_timedrdlock(pfc_rwlock_t *PFC_RESTRICT rwlockp,
				       const pfc_timespec_t
				       *PFC_RESTRICT timeout);
extern int	pfc_rwlock_timedwrlock(pfc_rwlock_t *PFC_RESTRICT rwlockp,
				       const pfc_timespec_t
				       *PFC_RESTRICT timeout);
extern int	pfc_rwlock_timedrdlock_abs(pfc_rwlock_t *PFC_RESTRICT rwlockp,
					   const pfc_timespec_t
					   *PFC_RESTRICT abstime);
extern int	pfc_rwlock_timedwrlock_abs(pfc_rwlock_t *PFC_RESTRICT rwlockp,
					   const pfc_timespec_t
					   *PFC_RESTRICT abstime);

#define	__PFC_RWLOCK_API_DECL(func)					\
	static inline int PFC_FATTR_ALWAYS_INLINE			\
	pfc_rwlock_##func(pfc_rwlock_t *rwlockp)			\
	{								\
		return pthread_rwlock_##func((pthread_rwlock_t *)rwlockp); \
	}

__PFC_RWLOCK_API_DECL(rdlock)
__PFC_RWLOCK_API_DECL(tryrdlock)
__PFC_RWLOCK_API_DECL(wrlock)
__PFC_RWLOCK_API_DECL(trywrlock)
__PFC_RWLOCK_API_DECL(unlock)
__PFC_RWLOCK_API_DECL(destroy)

/*
 * PFC condvar APIs.
 */
extern int	pfc_cond_init(pfc_cond_t *condp);
extern int	pfc_cond_timedwait(pfc_cond_t *PFC_RESTRICT condp,
				   pfc_mutex_t *PFC_RESTRICT mutexp,
				   const pfc_timespec_t *PFC_RESTRICT timeout);
extern int	pfc_cond_timedwait_abs(pfc_cond_t *PFC_RESTRICT condp,
				       pfc_mutex_t *PFC_RESTRICT mutexp,
				       const pfc_timespec_t *PFC_RESTRICT
				       abstime);

#define	__PFC_COND_API_DECL(func)					\
	static inline int PFC_FATTR_ALWAYS_INLINE			\
	pfc_cond_##func(pfc_cond_t *condp)				\
	{								\
		return pthread_cond_##func((pthread_cond_t *)condp);	\
	}

__PFC_COND_API_DECL(signal)
__PFC_COND_API_DECL(broadcast)
__PFC_COND_API_DECL(destroy)

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_cond_wait(pfc_cond_t *PFC_RESTRICT condp, pfc_mutex_t *PFC_RESTRICT mutexp)
{
	return pthread_cond_wait((pthread_cond_t *)condp,
				 (pthread_mutex_t *)mutexp);
}

/*
 * PFC semaphore APIs.
 */
extern int	pfc_sem_init(pfc_sem_t *semp, unsigned int value);
extern int	pfc_sem_timedwait(pfc_sem_t *PFC_RESTRICT semp,
				  const pfc_timespec_t *PFC_RESTRICT timeout);
extern int	pfc_sem_timedwait_abs(pfc_sem_t *PFC_RESTRICT semp,
				      const pfc_timespec_t *PFC_RESTRICT
				      abstime);

#define	__PFC_SEM_API_DECL(func)					\
	static inline int PFC_FATTR_ALWAYS_INLINE			\
	pfc_sem_##func(pfc_sem_t *semp)					\
	{								\
		return sem_##func((sem_t *)semp);			\
	}

__PFC_SEM_API_DECL(post)
__PFC_SEM_API_DECL(wait)
__PFC_SEM_API_DECL(trywait)
__PFC_SEM_API_DECL(destroy)

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_sem_getvalue(pfc_sem_t *PFC_RESTRICT semp, int *PFC_RESTRICT sval)
{
	return sem_getvalue((sem_t *)semp, sval);
}

PFC_C_END_DECL

#endif	/* !_PFC_SYNCH_H */
