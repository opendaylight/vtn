/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_THREAD_IMPL_H
#define	_PFC_LIBPFC_THREAD_IMPL_H

/*
 * Internal definitions for thread management.
 */

#include <stdio.h>
#include <pfc/base.h>
#include <pfc/tpool.h>
#include <pfc/thread.h>
#include <pfc/list.h>
#include <pfc/synch.h>
#include <pfc/refptr.h>
#include <pfc/rbtree.h>

PFC_C_BEGIN_DECL

/*
 * Thread pool instance.
 */
typedef struct {
	pfc_list_t		tp_free;	/* free threads in this pool */
	const char		*tp_name;	/* name of thread pool */
	pthread_attr_t		tp_attr;	/* pthread attributes */
	pfc_mutex_t		tp_mutex;	/* mutex for this pool */
	pfc_cond_t		tp_cond;	/* condvar for this pool */
	pfc_rbnode_t		tp_node;	/* Red-Black Tree node */
	uint32_t		tp_nthreads;	/* number of threads */
	uint32_t		tp_nfree;	/* number of free threads */
	uint32_t		tp_max;		/* max number of threads */
	uint32_t		tp_free_max;	/* max number of free threads */
	uint32_t		tp_free_min;	/* min number of free threads */
	uint32_t		tp_reap;	/* threads to be reaped */
	uint32_t		tp_lastreap;	/* nthreads on the last reap */
	uint32_t		tp_refcnt;	/* reference counter */
	volatile uint16_t	tp_state;	/* pool state */
} tpool_t;

#define	TPOOL_NODE2PTR(node)	PFC_CAST_CONTAINER((node), tpool_t, tp_node)

/*
 * Thread pool state bits.
 */
#define	TPOOL_STATE_SHUTDOWN		0x1U	/* shutdown */
#define	TPOOL_STATE_DISABLE_EXPAND	0x2U	/* pool expansion is disabled */

/*
 * Lock/Unlock physical thread.
 */
#define	TPOOL_LOCK(tpool)	pfc_mutex_lock(&(tpool)->tp_mutex)
#define	TPOOL_TIMEDLOCK_ABS(tpool, abstime)			\
	pfc_mutex_timedlock_abs(&(tpool)->tp_mutex, (abstime))
#define	TPOOL_UNLOCK(tpool)	pfc_mutex_unlock(&(tpool)->tp_mutex)

/*
 * Condition variable macros for thread pool.
 */
#define	TPOOL_TIMEDWAIT_ABS(tpool, abstime)				\
	pfc_cond_timedwait_abs(&(tpool)->tp_cond, &(tpool)->tp_mutex,	\
			       (abstime))
#define	TPOOL_SIGNAL(tpool)	pfc_cond_signal(&(tpool)->tp_cond)

/*
 * Determine whether the thread pool shutdown flag is set or not.
 */
#define	TPOOL_IS_SHUTDOWN(tpool)					\
	(PFC_EXPECT_FALSE((tpool)->tp_state & TPOOL_STATE_SHUTDOWN))

/*
 * Determine whether pool expansion is disabled or not.
 */
#define	TPOOL_EXPAND_IS_DISABLED(tpool)				\
	((tpool)->tp_state & TPOOL_STATE_DISABLE_EXPAND)

/*
 * Return name of thread pool.
 */
#define	TPOOL_NAME(tpool)	((tpool)->tp_name)

/*
 * Job on a thread.
 */
struct tpool_job;
typedef struct tpool_job	tpool_job_t;

/*
 * Remarks:
 *	tj_storage can be accessed without holding any lock because it is used
 *	by the current job only.
 */
struct tpool_job {
	pfc_thfunc_t	tj_func;	/* job function pointer */
	void		*tj_arg;	/* argument */
	void		*tj_status;	/* return value of job function */
	pfc_rbtree_t	tj_storage;	/* thread specific data storage */
	pfc_mutex_t	tj_mutex;	/* mutex for this job */
	pfc_cond_t	tj_cond;	/* condition variable for this job */
	tpool_job_t	*tj_next;	/* link for internal hash table */
	pfc_thread_t	tj_id;		/* job ID */
	uint32_t	tj_flags;	/* flags */
};

/*
 * Job flags.
 */
#define	TPOOL_JOB_DETACHED		0x1U	/* detached job */
#define	TPOOL_JOB_DONE			0x2U	/* job has been finished */
#define	TPOOL_JOB_JOINING		0x4U	/* someone waits for this job */

/*
 * Lock/Unlock thread job.
 */
#define	TPOOL_JOB_LOCK(job)		pfc_mutex_lock(&(job)->tj_mutex)
#define	TPOOL_JOB_UNLOCK(job)		pfc_mutex_unlock(&(job)->tj_mutex)

/*
 * Condition variable macros for thread job.
 */
#define	TPOOL_JOB_WAIT(job)					\
	pfc_cond_wait(&(job)->tj_cond, &(job)->tj_mutex)
#define	TPOOL_JOB_TIMEDWAIT(job, timeout)				\
	pfc_cond_timedwait(&(job)->tj_cond, &(job)->tj_mutex, (timeout))
#define	TPOOL_JOB_SIGNAL(job)		pfc_cond_signal(&(job)->tj_cond)
#define	TPOOL_JOB_BROADCAST(job)	pfc_cond_broadcast(&(job)->tj_cond)

/*
 * Determine whether the given job is active.
 */
#define	TPOOL_JOB_IS_ACTIVE(job)			\
	(((job)->tj_flags & TPOOL_JOB_DONE) == 0)

/*
 * Determine whether the given job is not yet joined.
 */
#define	TPOOL_JOB_IS_UNJOINED(job)					\
	(((job)->tj_flags & (TPOOL_JOB_DONE | TPOOL_JOB_JOINING)) ==	\
	 TPOOL_JOB_DONE)

/*
 * Physical thread instance.
 */
typedef struct {
	pfc_list_t		tt_list;	/* link for free list */
	tpool_t			*tt_pool;	/* thread pool */
	tpool_job_t *volatile	tt_job;		/* current job */
	pfc_mutex_t		tt_mutex;	/* mutex for this thread */
	pfc_cond_t		tt_cond;	/* condvar for this thread */
	pthread_t		tt_thread;	/* thread ID */
	volatile uint16_t	tt_state;	/* state bits */
} tpool_thread_t;

/*
 * Lock/Unlock physical thread.
 * If you want to acquire both physical thread lock and thread pool lock,
 * thread pool lock must be acquired at first.
 */
#define	TPOOL_THREAD_LOCK(ttp)		pfc_mutex_lock(&(ttp)->tt_mutex)
#define	TPOOL_THREAD_TIMEDLOCK_ABS(ttp, abstime)		\
	pfc_mutex_timedlock_abs(&(ttp)->tt_mutex, (abstime))
#define	TPOOL_THREAD_UNLOCK(ttp)	pfc_mutex_unlock(&(ttp)->tt_mutex)

/*
 * Condition variable macros for physical thread.
 */
#define	TPOOL_THREAD_WAIT(ttp)					\
	pfc_cond_wait(&(ttp)->tt_cond, &(ttp)->tt_mutex)
#define	TPOOL_THREAD_SIGNAL(ttp)	pfc_cond_signal(&(ttp)->tt_cond)
#define	TPOOL_THREAD_BROADCAST(ttp)	pfc_cond_broadcast(&(ttp)->tt_cond)

/*
 * static inline void
 * pfc_tpool_thread_signal(tpool_thread_t *ttp, uint16_t state)
 *	Set the given state bit to the physical thread.
 *	The caller must not hold TPOOL_THREAD_LOCK().
 */
static inline void
pfc_tpool_thread_signal(tpool_thread_t *ttp, uint16_t state)
{
	TPOOL_THREAD_LOCK(ttp);
	ttp->tt_state |= state;
	TPOOL_THREAD_SIGNAL(ttp);
	TPOOL_THREAD_UNLOCK(ttp);
}

/*
 * Physical thread state bits.
 */
#define	TPOOL_THR_STATE_INIT		0x1U	/* initialized */
#define	TPOOL_THR_STATE_INIT_FAIL	0x2U	/* initialization failed */
#define	TPOOL_THR_STATE_SHUTDOWN	0x4U	/* shutdown */
#define	TPOOL_THR_STATE_FREE_AGED	0x8U	/* aged on the free list */
#define	TPOOL_THR_STATE_INIT_DONE				\
	(TPOOL_THR_STATE_INIT | TPOOL_THR_STATE_INIT_FAIL)

/*
 * Determine whether the given physical thread is now on the free list.
 * Thread pool lock must be acquired by the caller.
 */
#define	TPOOL_THREAD_IS_FREE(ttp)	(!pfc_list_is_empty(&(ttp)->tt_list))

/*
 * Determine whether the physical thread shutdown flag is set or not.
 */
#define	TPOOL_THREAD_STATE_IS_SHUTDOWN(state)				\
	(PFC_EXPECT_FALSE((state) & TPOOL_THR_STATE_SHUTDOWN))
#define	TPOOL_THREAD_IS_SHUTDOWN(ttp)					\
	TPOOL_THREAD_STATE_IS_SHUTDOWN((ttp)->tt_state)

/*
 * Current job on the thread.
 */
extern __thread tpool_job_t	*tpool_job_current;

/*
 * POSIX thread specific data key that keeps physical thread instance.
 */
extern pthread_key_t	pfc_thread_key;

/*
 * Cast TSD key for Red-Black Tree interface.
 */
#define	PFC_TSD_KEY(key)		((pfc_cptr_t)(uintptr_t)(key))

/*
 * Return current logical thread ID.
 */
static inline pfc_thread_t
pfc_thread_current(void)
{
	tpool_job_t	*cur = tpool_job_current;

	return (cur == NULL) ? PFC_THREAD_INVALID : cur->tj_id;
}

/*
 * Prototypes.
 */
extern void	pfc_tpool_libinit(void);
extern void	pfc_tpool_bootstrap(void);
extern void	pfc_tpool_fini(void);
extern int	pfc_tpool_dispatch(const char *PFC_RESTRICT name,
				   tpool_job_t *PFC_RESTRICT job);

extern void	pfc_thread_libinit(void);
extern void	pfc_thread_dispatch(tpool_job_t *job);
extern void	pfc_thread_cleanup(tpool_job_t *PFC_RESTRICT job,
				   void *PFC_RESTRICT status);
extern void	pfc_tpool_reap(void);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_THREAD_IMPL_H */
