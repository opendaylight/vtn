/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tpool.c - Thread pool interfaces.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pfc/conf.h>
#include <pfc/log.h>
#include <pfc/util.h>
#include <pfc/atomic.h>
#include <pfc/listmodel.h>
#include <pfc/event.h>
#include <pfc/ctype.h>
#include <pfc/debug.h>
#include "thread_impl.h"

/*
 * Determine whether the given character is suitable for thread pool name.
 */
#define	PFC_TPOOL_VALID_NAME_CHAR(c)	(pfc_isalnum_u(c) || (c) == '_')

/*
 * Global thread pool lock.
 *
 * Remarks:
 *	The following describes hierarchy of thread pool locks.
 *	Lock must be acquired in the below order if you need to acquire
 *	more than 2 locks at a time:
 *
 *	1. Global thread pool lock (tpool_lock)
 *	2. Thread pool's lock
 *	3. Physical thread's lock
 *	4. Thread job's lock
 */
static pfc_rwlock_t	tpool_lock = PFC_RWLOCK_INITIALIZER;

/*
 * Name of configuration map which defines thread pool attributes.
 */
static const char	tpool_conf_mapname[] = "thread_pool";

/*
 * Name of parameter names in the configuration file.
 */
static const char	tpool_conf_stack_size[] = "stack_size";
static const char	tpool_conf_max_threads[] = "max_threads";
static const char	tpool_conf_max_free[] = "max_free";
static const char	tpool_conf_min_free[] = "min_free";
static const char	tpool_conf_reap_threads[] = "reap_threads";

/*
 * Configurable attribute set for the pool.
 */
typedef struct {
	uint32_t	ta_stack_size;		/* size of thread's stack */
	uint32_t	ta_max_threads;		/* max number of threads */
	uint32_t	ta_max_free;		/* max number of free threads */
	uint32_t	ta_min_free;		/* min number of free threads */
	uint32_t	ta_reap_threads;	/* threads to be reaped */
} tpool_attr_t;

/*
 * Default thread pool attributes for the default pool.
 */
#define	PFC_TPOOL_DEFAULT_DEF_STACK_SIZE	0x100000U
#define	PFC_TPOOL_DEFAULT_DEF_MAX		2000U
#define	PFC_TPOOL_DEFAULT_DEF_FREE_MAX		50U
#define	PFC_TPOOL_DEFAULT_DEF_FREE_MIN		4U
#define	PFC_TPOOL_DEFAULT_DEF_REAP		4U

/*
 * Default thread pool attributes for dynamically created pools.
 */
#define	PFC_TPOOL_DEFAULT_STACK_SIZE		0x100000U
#define	PFC_TPOOL_DEFAULT_MAX			50U
#define	PFC_TPOOL_DEFAULT_FREE_MAX		10U
#define	PFC_TPOOL_DEFAULT_FREE_MIN		0U
#define	PFC_TPOOL_DEFAULT_REAP			4U

/*
 * Default thread pool attributes for the default pool.
 */
static const tpool_attr_t	tpool_attr_default = {
	.ta_stack_size		= PFC_TPOOL_DEFAULT_DEF_STACK_SIZE,
	.ta_max_threads		= PFC_TPOOL_DEFAULT_DEF_MAX,
	.ta_max_free		= PFC_TPOOL_DEFAULT_DEF_FREE_MAX,
	.ta_min_free		= PFC_TPOOL_DEFAULT_DEF_FREE_MIN,
	.ta_reap_threads	= PFC_TPOOL_DEFAULT_DEF_REAP,
};

/*
 * Default thread pool attributes for dynamically created pools.
 */
static const tpool_attr_t	tpool_attr_dynamic = {
	.ta_stack_size		= PFC_TPOOL_DEFAULT_STACK_SIZE,
	.ta_max_threads		= PFC_TPOOL_DEFAULT_MAX,
	.ta_max_free		= PFC_TPOOL_DEFAULT_FREE_MAX,
	.ta_min_free		= PFC_TPOOL_DEFAULT_FREE_MIN,
	.ta_reap_threads	= PFC_TPOOL_DEFAULT_REAP,
};

#define	TPOOL_CONF_VALUE(map, key, tatrp)				\
	pfc_conf_get_uint32((map), tpool_conf_##key, (tatrp)->ta_##key)

/*
 * Name of default thread pool.
 */
static const char	tpool_default_name[] = "default";

/*
 * Default thread pool.
 */
static tpool_t		*tpool_default;

/*
 * POSIX thread specific data key.
 */
pthread_key_t	pfc_thread_key PFC_ATTR_HIDDEN;

/*
 * Thread ID of pool manager thread.
 */
static pthread_t	tpool_mgr;

/*
 * Manager threads' work queue.
 */
static pfc_listm_t	tpool_mgr_queue;

/*
 * Stack size for pool manager thread.
 */
#define	TPOOL_MGR_STKSIZE	PFC_CONST_U(0x8000)

/*
 * Determine whether the thread pool should be expanded or not.
 * This macro must be used with holding thread pool lock.
 */
#define	TPOOL_NEED_EXPANDED(tpool)			\
	(!TPOOL_EXPAND_IS_DISABLED(tpool) &&		\
	 (tpool)->tp_nfree < (tpool)->tp_free_min &&	\
	 (tpool)->tp_nthreads < (tpool)->tp_max)

/*
 * Determine whether the thread pool should be shrunk or not.
 * This macro must be used with holding thread pool lock.
 */
#define	TPOOL_NEED_SHRUNK(tpool)			\
	((tpool)->tp_nfree > (tpool)->tp_free_max)

/*
 * Internal prototypes.
 */
static void		pfc_tpool_apply_conf(tpool_t *PFC_RESTRICT tp,
					     const char *PFC_RESTRICT name,
					     const tpool_attr_t *PFC_RESTRICT
					     tatrp);
static int		pfc_tpool_create_impl(const char *PFC_RESTRICT name,
					      const tpool_attr_t *PFC_RESTRICT
					      tatrp,
					      tpool_t **PFC_RESTRICT tpoolp);
static int		pfc_tpool_verify_name(const char *name);
static int		pfc_tpool_thread_get(tpool_t *tpool,
					     tpool_thread_t **ttpp);
static int		pfc_tpool_thread_create(tpool_t *tpool,
						tpool_thread_t **ttpp);
static tpool_job_t	*pfc_tpool_thread_get_job(tpool_thread_t *ttp);
static void		*pfc_tpool_thread_main(void *arg);
static void		pfc_tpool_thread_cleanup(void *arg);
static void		*pfc_tpool_mgr_main(void *arg);
static void		pfc_tpool_mgr_adjust(tpool_t *tpool);
static void		pfc_tpool_adjust_async(tpool_t *tpool);
static void		pfc_tpool_shrink(tpool_t *tpool, pfc_bool_t force);
static uint32_t		pfc_tpool_delete_aged(tpool_t *tpool,
					      uint32_t nthreads);
static void		pfc_tpool_delete_free(tpool_t *tpool,
					      uint32_t nthreads);
static pfc_cptr_t	pfc_tpool_getkey(pfc_rbnode_t *node);

/*
 * The Red-Black Tree which keeps pairs of thread pool name and pool instance.
 */
static pfc_rbtree_t	thread_pools =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, pfc_tpool_getkey);

/*
 * static inline void
 * pfc_tpool_rdlock(void)
 *	Acquire thread pool global lock in reader mode.
 */
static inline void
pfc_tpool_rdlock(void)
{
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&tpool_lock), 0);
}

/*
 * static inline void
 * pfc_tpool_wrlock(void)
 *	Acquire thread pool global lock in writer mode.
 */
static inline void
pfc_tpool_wrlock(void)
{
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&tpool_lock), 0);
}

/*
 * static inline void
 * pfc_tpool_unlock(void)
 *	Release thread pool global lock.
 */
static inline void
pfc_tpool_unlock(void)
{
	PFC_ASSERT_INT(pfc_rwlock_unlock(&tpool_lock), 0);
}

/*
 * static inline int
 * pfc_tpool_timedwrlock_abs(pfc_timespec_t *abstime)
 *	Try to acquire thread pool global lock in writer mode.
 *	ETIMEDOUT is returned if the current thread can't acquire the lock
 *	until the specified absolute time.
 */
static inline int
pfc_tpool_timedwrlock_abs(pfc_timespec_t *abstime)
{
	int	err = pfc_rwlock_timedwrlock_abs(&tpool_lock, abstime);

	PFC_ASSERT(err == 0 || err == ETIMEDOUT);

	return err;
}

/*
 * static inline tpool_t *
 * pfc_tpool_lookup(const char *name)
 *	Search thread pool by name.
 *
 * Remarks:
 *	The caller must hold global thread pool lock.
 */
static inline tpool_t *
pfc_tpool_lookup(const char *name)
{
	pfc_rbnode_t	*node;
	tpool_t		*tp;

	node = pfc_rbtree_get(&thread_pools, name);
	tp = (PFC_EXPECT_TRUE(node != NULL)) ? TPOOL_NODE2PTR(node) : NULL;

	return tp;
}

/*
 * static inline tpool_t *
 * pfc_tpool_get(const char *name)
 *	Return thread pool instance associated with the given name.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function holds thread pool lock on return.
 */
static inline tpool_t *
pfc_tpool_get(const char *name)
{
	tpool_t		*tpool;

	if (name == NULL) {
		tpool = tpool_default;
		TPOOL_LOCK(tpool);
	}
	else {
		pfc_tpool_rdlock();
		tpool = pfc_tpool_lookup(name);
		if (PFC_EXPECT_TRUE(tpool != NULL)) {
			TPOOL_LOCK(tpool);
		}
		pfc_tpool_unlock();
	}

	return tpool;
}

/*
 * static inline void
 * pfc_tpool_nthreads_dec(tpool_t *tpool)
 *	Decrement tpool->tp_nthreads.
 */
static inline void
pfc_tpool_nthreads_dec(tpool_t *tpool)
{
	TPOOL_LOCK(tpool);
	PFC_ASSERT(tpool->tp_nthreads > 0);
	tpool->tp_nthreads--;

	if (TPOOL_IS_SHUTDOWN(tpool)) {
		if (tpool->tp_nthreads == 0) {
			TPOOL_SIGNAL(tpool);
		}
		TPOOL_UNLOCK(tpool);
	}
	else if (TPOOL_NEED_EXPANDED(tpool) || TPOOL_NEED_SHRUNK(tpool)) {
		/*
		 * Adjust number of free threads asynchronously.
		 * Lock will be released by pfc_tpool_adjust_async().
		 */
		pfc_tpool_adjust_async(tpool);
	}
	else {
		TPOOL_UNLOCK(tpool);
	}
}

/*
 * static inline void
 * pfc_tpool_hold(tpool_t *tpool)
 *	Hold thread pool to prevent the pool instance from freeing.
 */
static inline void
pfc_tpool_hold(tpool_t *tpool)
{
	PFC_ASSERT(tpool->tp_refcnt != 0);
	pfc_atomic_inc_uint32(&tpool->tp_refcnt);
}

/*
 * static inline void
 * pfc_tpool_release(tpool_t *tpool)
 *	Release thread pool held by pfc_tpool_hold().
 *
 * Remarks:
 *	The caller must NOT hold thread pool lock on calling.
 */
static inline void
pfc_tpool_release(tpool_t *tpool)
{
	PFC_ASSERT(tpool->tp_refcnt != 0);

	if (pfc_atomic_dec_uint32_old(&tpool->tp_refcnt) == 1) {
		PFC_ASSERT_INT(pfc_cond_destroy(&tpool->tp_cond), 0);
		PFC_ASSERT_INT(pfc_mutex_destroy(&tpool->tp_mutex), 0);
		PFC_ASSERT_INT(pthread_attr_destroy(&tpool->tp_attr), 0);
		free((void *)tpool->tp_name);
		free(tpool);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_tpool_init(void)
 *	Initialize thread pool subsystem.
 */
void PFC_ATTR_HIDDEN
pfc_tpool_libinit(void)
{
	int	err;

	/* Create pthread key so that we can detect unexpected thread death. */
	err = pthread_key_create(&pfc_thread_key, pfc_tpool_thread_cleanup);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fprintf(stderr, "tpool_init: Failed to create key: %s\n",
			strerror(err));
		abort();
		/* NOTREACHED */
	}

	/*
	 * Create default thread pool.
	 * Note that we must not specify thread pool attributes here because
	 * PFC system configuration file is not yet loaded.
	 */
	err = pfc_tpool_create_impl(tpool_default_name, NULL, &tpool_default);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fprintf(stderr,
			"tpool_init: Failed to create default pool: %s\n",
			strerror(err));
		abort();
		/* NOTREACHED */
	}

	/* Set temporary attributes. */
	tpool_default->tp_max = PFC_TPOOL_DEFAULT_DEF_MAX;
	tpool_default->tp_free_max = PFC_TPOOL_DEFAULT_DEF_FREE_MAX;
	tpool_default->tp_free_min = PFC_TPOOL_DEFAULT_DEF_FREE_MIN;
	tpool_default->tp_reap = PFC_TPOOL_DEFAULT_DEF_REAP;

	/* Initialize logical thread management layer. */
	pfc_thread_libinit();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_tpool_bootstrap(void)
 *	Bootstrap code of thread pool management layer.
 *	This function must be called once before the first thread is created
 *	from the thread pool.
 */
void PFC_ATTR_HIDDEN
pfc_tpool_bootstrap(void)
{
	pthread_attr_t	attr;
	tpool_t	*tpool = tpool_default;
	int	err;

	/* Apply configuration for the default pool. */
	pfc_tpool_apply_conf(tpool, tpool_default_name, &tpool_attr_default);

	/*
	 * Create work queue for pool manager thread.
	 * This takes tpool_t pointer.
	 */
	err = pfc_llist_create(&tpool_mgr_queue);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to create tpool manager queue: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	/* Create thread pool manager thread. */
	PFC_ASSERT_INT(pthread_attr_init(&attr), 0);
	PFC_ASSERT_INT(pthread_attr_setstacksize(&attr, TPOOL_MGR_STKSIZE), 0);
	PFC_ASSERT_INT(pthread_attr_setdetachstate
		       (&attr, PTHREAD_CREATE_DETACHED), 0);

	err = pthread_create(&tpool_mgr, &attr, pfc_tpool_mgr_main, NULL);
	PFC_ASSERT_INT(pthread_attr_destroy(&attr), 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to create tpool manager thread: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	TPOOL_LOCK(tpool);

	/* Expand thread pool to min_free. */
	while (tpool->tp_nfree < tpool->tp_free_min) {
		int	err;
		tpool_thread_t	*ttp;

		/*
		 * pfc_tpool_thread_create() always drops thread pool's
		 * lock.
		 */
		err = pfc_tpool_thread_create(tpool, &ttp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_warn("Failed to expand default thread pool: %s",
				     strerror(err));
			break;
		}

		TPOOL_LOCK(tpool);
		pfc_list_push(&tpool->tp_free, &ttp->tt_list);
		tpool->tp_nfree++;
	}

	if (tpool->tp_nthreads != 0) {
		pfc_log_verbose("Expanded threads in the default pool to %u.",
				tpool->tp_nthreads);
	}

	TPOOL_UNLOCK(tpool);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_tpool_fini(void)
 *	Shutdown thread pool management layer.
 */
void PFC_ATTR_HIDDEN
pfc_tpool_fini(void)
{
	pfc_rbnode_t	*node;

	/* Stop the pool manager thread. */
	pfc_llist_shutdown(tpool_mgr_queue, PFC_TRUE);

	/*
	 * Set shutdown flag to all pools to inhibit other threads from
	 * creating new thread.
	 */
	pfc_tpool_wrlock();

	node = NULL;
	while ((node = pfc_rbtree_next(&thread_pools, node)) != NULL) {
		tpool_t	*tpool = TPOOL_NODE2PTR(node);

		pfc_log_verbose("%s: thread pool shutdown.",
				TPOOL_NAME(tpool));
		TPOOL_LOCK(tpool);
		tpool->tp_state |= TPOOL_STATE_SHUTDOWN;
		TPOOL_UNLOCK(tpool);
	}

	pfc_tpool_unlock();
}

/*
 * int
 * pfc_tpool_create(const char *name)
 *	Create a new thread pool.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_tpool_create(const char *name)
{
	int	err;
	tpool_t	*tpool;

	err = pfc_tpool_create_impl(name, &tpool_attr_dynamic, &tpool);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/*
		 * Expand thread pool if min_free is not zero.
		 * Lock is always released by pfc_tpool_adjust_async().
		 */
		TPOOL_LOCK(tpool);
		if (tpool->tp_free_min != 0) {
			pfc_tpool_adjust_async(tpool);
		}
		else {
			TPOOL_UNLOCK(tpool);
		}
	}

	return err;
}

/*
 * int
 * pfc_tpool_destroy(const char *PFC_RESTRICT name,
 *		     const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Destroy the thread pool specified by the name.
 *	The current thread is blocked until all threads in the pool quit.
 *	`timeout' specifies an upper limit of the time for which the current
 *	thread is blocked. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_tpool_destroy(const char *PFC_RESTRICT name,
		  const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	tspec, *abstime;
	uint16_t	state;
	pfc_list_t	*list;
	tpool_t	*tpool;
	int	err;

	if (name == NULL || strcmp(name, tpool_default_name) == 0) {
		/* Default pool can't be destroyed. */
		return EPERM;
	}

	/* Convert timeout period into system absolute time. */
	if (timeout != NULL) {
		abstime = &tspec;
		err = pfc_clock_abstime(abstime, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		abstime = NULL;
	}

	/* Acquire global pool lock in writer mode. */
	err = pfc_tpool_timedwrlock_abs(abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Obtain thread pool instance. */
	tpool = pfc_tpool_lookup(name);
	if (PFC_EXPECT_FALSE(tpool == NULL)) {
		pfc_tpool_unlock();

		return ENOENT;
	}

	err = TPOOL_TIMEDLOCK_ABS(tpool, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_tpool_unlock();

		return ENOENT;
	}

	state = tpool->tp_state;
	if (PFC_EXPECT_FALSE(state & TPOOL_STATE_SHUTDOWN)) {
		/* Another thread is trying to destroy this pool. */
		TPOOL_UNLOCK(tpool);
		pfc_tpool_unlock();

		return EINVAL;
	}

	/* Set shutdown flag to this pool. */
	tpool->tp_state = (state | TPOOL_STATE_SHUTDOWN);

	/* Release global pool lock here. */
	pfc_tpool_unlock();

	/* Send shutdown signal to all free threads in this pool. */
	while ((list = pfc_list_pop(&tpool->tp_free)) != NULL) {
		tpool_thread_t	*ttp;

		ttp = PFC_CAST_CONTAINER(list, tpool_thread_t, tt_list);

		err = TPOOL_THREAD_TIMEDLOCK_ABS(ttp, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* Restore thread pool state on error. */
			tpool->tp_state = state;
			TPOOL_UNLOCK(tpool);

			return err;
		}
		ttp->tt_state |= TPOOL_THR_STATE_SHUTDOWN;
		TPOOL_THREAD_SIGNAL(ttp);
		TPOOL_THREAD_UNLOCK(ttp);
	}

	tpool->tp_nfree = 0;

	/* Wait for all threads to quit. */
	while (tpool->tp_nthreads != 0) {
		err = TPOOL_TIMEDWAIT_ABS(tpool, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* Restore thread pool state on error. */
			tpool->tp_state = state;
			TPOOL_UNLOCK(tpool);

			return err;
		}
		PFC_MEMORY_RELOAD();
	}

	TPOOL_UNLOCK(tpool);

	/*
	 * Make this pool invisible.
	 * We ignore timeout here because it is expected not to take too
	 * much time.
	 */
	pfc_tpool_wrlock();
	pfc_rbtree_remove_node(&thread_pools, &tpool->tp_node);
	pfc_tpool_unlock();

	/* Free up all resources. */
	pfc_tpool_release(tpool);

	return 0;
}

/*
 * static void
 * pfc_tpool_apply_conf(tpool_t *PFC_RESTRICT tp, const char *PFC_RESTRICT name,
 *			const tpool_attr_t *PFC_RESTRICT tatrp)
 *	Apply configuration for the specified thread pool.
 */
static void
pfc_tpool_apply_conf(tpool_t *PFC_RESTRICT tp, const char *PFC_RESTRICT name,
		     const tpool_attr_t *PFC_RESTRICT tatrp)
{
	pfc_cfblk_t	map;
	uint32_t	size, max, max_free, min_free, reap;
	int	err;

	PFC_ASSERT(tp->tp_nthreads == 0);

	map = pfc_sysconf_get_map(tpool_conf_mapname, name);

	/* Apply parameters related to number of threads. */
	max = TPOOL_CONF_VALUE(map, max_threads, tatrp);
	max_free = TPOOL_CONF_VALUE(map, max_free, tatrp);
	min_free = TPOOL_CONF_VALUE(map, min_free, tatrp);
	reap = TPOOL_CONF_VALUE(map, reap_threads ,tatrp);

	if (PFC_EXPECT_FALSE(max_free > max)) {
		uint32_t	v = (tatrp->ta_max_free > max)
			? max : tatrp->ta_max_free;

		pfc_log_warn("%s: max_free(%u) exceeds max_threads(%u). "
			     "Use %u instead.",
			     name, max_free, max, v);
		max_free = v;
	}
	if (PFC_EXPECT_FALSE(min_free > max)) {
		uint32_t	v = (tatrp->ta_min_free > max)
			? max : tatrp->ta_min_free;

		pfc_log_warn("%s: min_free(%u) exceeds max_threads(%u). "
			     "Use %u instead.",
			     name, min_free, max, v);
		min_free = v;
	}
	if (PFC_EXPECT_FALSE(min_free > max_free)) {
		uint32_t	v = (tatrp->ta_min_free > max_free)
			? max_free : tatrp->ta_min_free;

		pfc_log_warn("%s: min_free(%u) exceeds max_free(%u). "
			     "Use %u instead.",
			     name, min_free, max_free, v);
		min_free = v;
	}

	pfc_log_verbose("%s: max_threads=%u, max_free=%u, min_free=%u, reap=%u",
			name, max, max_free, min_free, reap);

	tp->tp_max = max;
	tp->tp_free_max = max_free;
	tp->tp_free_min = min_free;
	tp->tp_reap = reap;

	/* Apply stack size. */
	size = TPOOL_CONF_VALUE(map, stack_size, tatrp);
	size = PFC_POW2_ROUNDUP(size, pfc_get_pagesize());

	err = pthread_attr_setstacksize(&tp->tp_attr, size);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_warn("%s: Failed to set stack size: %x", name, size);
	}
}

/*
 * static int
 * pfc_tpool_create_impl(const char *PFC_RESTRICT name,
 *			 const tpool_attr_t *PFC_RESTRICT tatrp,
 *			 tpool_t **PFC_RESTRICT tpoolp)
 *	Create a new thread pool.
 *
 * Calling/Exit State:
 *	Upon successful completion, thread pool instance is set to *tpoolp
 *	and zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function just creates a new thread pool instance.
 *	Any physical thread is not created here.
 */
static int
pfc_tpool_create_impl(const char *PFC_RESTRICT name,
		      const tpool_attr_t *PFC_RESTRICT tatrp,
		      tpool_t **PFC_RESTRICT tpoolp)
{
	int		err;
	pthread_attr_t	*attrp = NULL;
	char		*namebuf;
	tpool_t		*tp;

	PFC_ASSERT(tpoolp != NULL);

	if (PFC_EXPECT_FALSE(name == NULL)) {
		return EINVAL;
	}

	/* Ensure that pool name is valid. */
	err = pfc_tpool_verify_name(name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	tp = (tpool_t *)malloc(sizeof(*tp));
	if (PFC_EXPECT_FALSE(tp == NULL)) {
		return ENOMEM;
	}

	namebuf = strdup(name);
	if (PFC_EXPECT_FALSE(namebuf == NULL)) {
		free(tp);
		return ENOMEM;
	}

	tp->tp_name = (const char *)namebuf;
	tp->tp_nthreads = tp->tp_nfree = 0;
	tp->tp_refcnt = 1;
	tp->tp_state = 0;
	tp->tp_lastreap = 0;

	/* Initialize the list which contains free physical threads. */
	pfc_list_init(&tp->tp_free);

	err = PFC_MUTEX_INIT(&tp->tp_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	err = pfc_cond_init(&tp->tp_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Initialize pthread attributes. */
	attrp = &tp->tp_attr;
	err = pthread_attr_init(attrp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		attrp = NULL;
		goto error;
	}

	/* Apply configurations for this thread pool. */
	if (tatrp != NULL) {
		pfc_tpool_apply_conf(tp, name, tatrp);
	}

	/* Ensure that system scope thread is used. */
	err = pthread_attr_setscope(attrp, PTHREAD_SCOPE_SYSTEM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* All threads in a pool should be detached. */
	err = pthread_attr_setdetachstate(attrp, PTHREAD_CREATE_DETACHED);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Make this pool visible. */
	pfc_tpool_wrlock();
	err = pfc_rbtree_put(&thread_pools, &tp->tp_node);
	pfc_tpool_unlock();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	*tpoolp = tp;

	return 0;

error:
	if (attrp != NULL) {
		(void)pthread_attr_destroy(attrp);
	}
	free(namebuf);
	free(tp);

	return err;
}

/*
 * static int
 * pfc_tpool_verify_name(const char *name)
 *	Verifies that the given name is suitable for thread pool name.
 *
 * Remarks:
 *	This function never checks whether the given name is already used.
 */
static int
pfc_tpool_verify_name(const char *name)
{
	const char	*p, *limit;

	if (PFC_EXPECT_FALSE(!pfc_isalpha_u(*name))) {
		return EINVAL;
	}

	limit = name + PFC_TPOOL_NAME_MAX;
	for (p = name + 1; *p != '\0'; p++) {
		if (PFC_EXPECT_FALSE(p >= limit)) {
			return ENAMETOOLONG;
		}

		if (PFC_EXPECT_FALSE(!PFC_TPOOL_VALID_NAME_CHAR(*p))) {
			return EINVAL;
		}
	}

	return 0;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_tpool_dispatch(const char *PFC_RESTRICT name,
 *		      tpool_job_t *PFC_RESTRICT job)
 *	Run the given job on the thread pool.
 *	Default thread pool is chosen if NULL is specified as pool name.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
pfc_tpool_dispatch(const char *PFC_RESTRICT name, tpool_job_t *PFC_RESTRICT job)
{
	tpool_t		*tpool;
	tpool_thread_t	*ttp = NULL;	/* Suppress warning by old gcc. */

	tpool = pfc_tpool_get(name);
	if (PFC_EXPECT_FALSE(tpool == NULL)) {
		return ENOENT;
	}

	while (1) {
		int	err;

		/* Pick up one free physical thread. */
		err = pfc_tpool_thread_get(tpool, &ttp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		/* Dispatch the given job to the physical thread. */
		TPOOL_THREAD_LOCK(ttp);
		if (!TPOOL_THREAD_IS_SHUTDOWN(ttp)) {
			break;
		}

		/*
		 * This physical thread is about to quit.
		 * We must choose another one.
		 */
		TPOOL_THREAD_UNLOCK(ttp);
	}

	ttp->tt_job = job;
	TPOOL_THREAD_SIGNAL(ttp);
	TPOOL_THREAD_UNLOCK(ttp);

	return 0;
}

/*
 * static int
 * pfc_tpool_thread_get(tpool_t *tpool, tpool_thread_t **ttpp)
 *	Get one physical thread to dispatch a new job.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to tpool_thread_t associated
 *	with a new thread is set to *ttpp, and zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding thread pool lock.
 *	It is always released on return.
 */
static int
pfc_tpool_thread_get(tpool_t *tpool, tpool_thread_t **ttpp)
{
	pfc_list_t	*elem;

	if (TPOOL_IS_SHUTDOWN(tpool)) {
		TPOOL_UNLOCK(tpool);

		return ESHUTDOWN;
	}

	/* Search for a free physical thread in the free list. */
	elem = pfc_list_pop(&tpool->tp_free);
	if (elem != NULL) {
		pfc_list_init(elem);
		PFC_ASSERT(tpool->tp_nfree > 0);
		tpool->tp_nfree--;

		if (TPOOL_NEED_EXPANDED(tpool)) {
			/*
			 * Expand thread pool asynchronously.
			 * Lock will be released by pfc_tpool_adjust_async().
			 */
			pfc_tpool_adjust_async(tpool);
		}
		else {
			TPOOL_UNLOCK(tpool);
		}
		*ttpp = PFC_CAST_CONTAINER(elem, tpool_thread_t, tt_list);

		return 0;
	}

	/* Try to expand thread pool. */
	return pfc_tpool_thread_create(tpool, ttpp);
}

/*
 * static int
 * pfc_tpool_thread_create(tpool_t *tpool, tpool_thread_t **ttpp)
 *	Create a physical thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to tpool_thread_t associated
 *	with a new thread is set to *ttpp, and zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must call this function with holding thread pool lock,
 *	and it is released on return.
 */
static int
pfc_tpool_thread_create(tpool_t *tpool, tpool_thread_t **ttpp)
{
	pthread_attr_t	*attrp = &tpool->tp_attr;
	tpool_thread_t	*ttp;
	int	err;

	PFC_ASSERT(ttpp != NULL);

	/* Ensure that we can expand thread pool. */
	if (PFC_EXPECT_FALSE(tpool->tp_nthreads >= tpool->tp_max)) {
		/* Too many physical threads. */
		TPOOL_UNLOCK(tpool);

		return EAGAIN;
	}

	tpool->tp_nthreads++;
	TPOOL_UNLOCK(tpool);

	/* Allocate and initialize physical thread instance. */
	ttp = (tpool_thread_t *)malloc(sizeof(*ttp));
	if (PFC_EXPECT_FALSE(ttp == NULL)) {
		err = ENOMEM;
		goto error;
	}

	PFC_ASSERT_INT(PFC_MUTEX_INIT(&ttp->tt_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_init(&ttp->tt_cond), 0);
	ttp->tt_pool = tpool;
	ttp->tt_job = NULL;
	ttp->tt_state = 0;
	pfc_list_init(&ttp->tt_list);

	/* Create physical thread. */
	err = pthread_create(&ttp->tt_thread, attrp, pfc_tpool_thread_main,
			     ttp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_free;
	}

	/* Wait until the thread finishes initialization. */
	TPOOL_THREAD_LOCK(ttp);
	while ((ttp->tt_state & TPOOL_THR_STATE_INIT_DONE) == 0) {
		TPOOL_THREAD_WAIT(ttp);
	}
	if (ttp->tt_state & TPOOL_THR_STATE_INIT_FAIL) {
		/*
		 * Thread initialization failed.
		 * We return ENOMEM because this should be caused by failure
		 * of pthread_setspecific().
		 */
		TPOOL_THREAD_UNLOCK(ttp);
		err = ENOMEM;
		goto error_free;
	}

	TPOOL_THREAD_UNLOCK(ttp);
	*ttpp = ttp;

	return 0;

error_free:
	free(ttp);

error:
	pfc_tpool_nthreads_dec(tpool);

	return err;
}

/*
 * static tpool_job_t *
 * pfc_tpool_thread_get_job(tpool_thread_t *ttp)
 *	Wait for a new job to be run on the given physical thread.
 *	NULL is returned if the thread must quit.
 */
static tpool_job_t *
pfc_tpool_thread_get_job(tpool_thread_t *ttp)
{
	tpool_job_t	*job;

	TPOOL_THREAD_LOCK(ttp);
	while (1) {
		uint16_t	state = ttp->tt_state;

		if ((job = ttp->tt_job) != NULL) {
			ttp->tt_state = (state & ~TPOOL_THR_STATE_FREE_AGED);
			break;
		}
		if (TPOOL_THREAD_STATE_IS_SHUTDOWN(state)) {
			job = NULL;
			break;
		}

		TPOOL_THREAD_WAIT(ttp);
	}
	TPOOL_THREAD_UNLOCK(ttp);

	return job;
}

/*
 * static void *
 * pfc_tpool_thread_main(void *arg)
 *	Main routine for a physical thread.
 */
static void *
pfc_tpool_thread_main(void *arg)
{
	tpool_thread_t	*ttp = (tpool_thread_t *)arg;
	tpool_t	*tpool = ttp->tt_pool;
	int	err;

	/* Register physical thread instance as thread-specific data. */
	err = pthread_setspecific(pfc_thread_key, ttp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_tpool_thread_signal(ttp, TPOOL_THR_STATE_INIT_FAIL);

		return NULL;
	}

	pfc_tpool_thread_signal(ttp, TPOOL_THR_STATE_INIT);

	while (1) {
		tpool_job_t	*job;

		/* Wait for a new job. */
		job = pfc_tpool_thread_get_job(ttp);
		if (job == NULL) {
			break;
		}

		/* Dispatch the job. */
		pfc_thread_dispatch(job);

		TPOOL_THREAD_LOCK(ttp);
		ttp->tt_job = NULL;
		TPOOL_THREAD_UNLOCK(ttp);

		TPOOL_LOCK(tpool);
		if (tpool->tp_nfree >= tpool->tp_free_max) {
			TPOOL_UNLOCK(tpool);
			break;
		}

		if (TPOOL_IS_SHUTDOWN(tpool)) {
			/* Thread pool has been destroyed. */
			TPOOL_UNLOCK(tpool);
			break;
		}

		/* Push this thread into the free list. */
		pfc_list_push(&tpool->tp_free, &ttp->tt_list);
		tpool->tp_nfree++;
		TPOOL_UNLOCK(tpool);
	}

	return NULL;
}

/*
 * static void
 * pfc_tpool_thread_cleanup(void *arg)
 *	Clean up function for a physical thread.
 *	This function is called when a physical thread quits.
 */
static void
pfc_tpool_thread_cleanup(void *arg)
{
	tpool_thread_t	*ttp = (tpool_thread_t *)arg;
	tpool_job_t	*job;
	tpool_t	*tpool = ttp->tt_pool;

	TPOOL_THREAD_LOCK(ttp);
	job = ttp->tt_job;
	TPOOL_THREAD_UNLOCK(ttp);

	if (PFC_EXPECT_FALSE(job != NULL)) {
		/*
		 * This physical thread is terminated unexpectedly.
		 * The current job must be canceled.
		 */
		pfc_thread_cleanup(job, PFC_THREAD_CANCELED);
	}

	/* Decrement number of threads in the pool. */
	pfc_tpool_nthreads_dec(tpool);

	/* Destroy physical thread instance. */
	free(ttp);
}

/*
 * static void *
 * pfc_tpool_mgr_main(void *arg)
 *	Main routine for the pool manager thread.
 */
static void *
pfc_tpool_mgr_main(void *arg)
{
	pfc_listm_t	queue = tpool_mgr_queue;

	pfc_log_verbose("Start thread pool manager thread.");

	while (1) {
		tpool_t	*tpool;
		int	err;

		/* Pop the first element in the queue. This should not fail. */
		err = pfc_llist_pop_wait(queue, (pfc_cptr_t *)&tpool);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ESHUTDOWN);
			break;
		}

		pfc_tpool_mgr_adjust(tpool);
	}

	return NULL;
}

/*
 * static void
 * pfc_tpool_mgr_adjust(tpool_t *tpool)
 *	Adjust number of threads in the pool on pool manager thread.
 */
static void
pfc_tpool_mgr_adjust(tpool_t *tpool)
{
	TPOOL_LOCK(tpool);

	if (TPOOL_IS_SHUTDOWN(tpool)) {
		goto out_unlock;
	}

	if (tpool->tp_nfree > tpool->tp_free_max) {
		uint32_t	delete = tpool->tp_nfree - tpool->tp_free_max;

		/* Too many free threads. */
		pfc_log_verbose("%s: Shrink %u threads in the pool.",
				TPOOL_NAME(tpool), delete);
		pfc_tpool_delete_free(tpool, delete);
		goto out_unlock;
	}

	if (tpool->tp_nfree < tpool->tp_free_min) {
		uint32_t	avail = tpool->tp_max - tpool->tp_nthreads;
		uint32_t	new = tpool->tp_free_min - tpool->tp_nfree;

		/* Create free threads. */
		if (new > avail) {
			new = avail;
		}

		if (new > 0) {
			pfc_log_verbose("%s: Expand %u threads in the pool.",
					TPOOL_NAME(tpool), new);
		}
		while (new > 0) {
			tpool_thread_t	*ttp;
			int	err;

			/*
			 * pfc_tpool_thread_cleanup() always releases thread
			 * pool lock.
			 */
			err = pfc_tpool_thread_create(tpool, &ttp);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto out;
			}

			new--;

			TPOOL_LOCK(tpool);
			if (TPOOL_IS_SHUTDOWN(tpool)) {
				TPOOL_THREAD_LOCK(ttp);
				ttp->tt_state |= TPOOL_THR_STATE_SHUTDOWN;
				TPOOL_THREAD_SIGNAL(ttp);
				TPOOL_THREAD_UNLOCK(ttp);
				goto out_unlock;
			}
			pfc_list_push(&tpool->tp_free, &ttp->tt_list);
			tpool->tp_nfree++;

			if (tpool->tp_nfree >= tpool->tp_free_min) {
				goto out_unlock;
			}
		}
	}

out_unlock:
	TPOOL_UNLOCK(tpool);
out:
	pfc_tpool_release(tpool);
}

/*
 * static void
 * pfc_tpool_adjust_async(tpool_t *tpool)
 *	Adjust number of free threads in background.
 *
 * Remarks:
 *	The caller must hold thread pool lock. It is always released on return.
 */
static void
pfc_tpool_adjust_async(tpool_t *tpool)
{
	int	err;

	pfc_tpool_hold(tpool);
	TPOOL_UNLOCK(tpool);

	err = pfc_listm_push_tail(tpool_mgr_queue, tpool);
	if (PFC_EXPECT_FALSE(err != 0 && err != ESHUTDOWN)) {
		pfc_log_warn("%s: Failed to push tpool manager queue: %s",
			     TPOOL_NAME(tpool), strerror(err));
		pfc_tpool_release(tpool);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_tpool_reap(void)
 *	Reap free threads in all thread pools.
 *	This function is called periodically on the resource reaper thread.
 */
void PFC_ATTR_HIDDEN
pfc_tpool_reap(void)
{
	pfc_rbnode_t	*node;

	pfc_tpool_wrlock();

	node = NULL;
	while ((node = pfc_rbtree_next(&thread_pools, node)) != NULL) {
		tpool_t		*tpool = TPOOL_NODE2PTR(node);
		uint32_t	nthreads;

		TPOOL_LOCK(tpool);

		/*
		 * Enable pool expansion which was disabled by low memory
		 * handler.
		 */
		tpool->tp_state &= ~TPOOL_STATE_DISABLE_EXPAND;

		/*
		 * Reap this pool if it has not been expanded since the last
		 * task of reaper thread.
		 */
		nthreads = tpool->tp_nthreads;
		if (nthreads <= tpool->tp_lastreap) {
			pfc_tpool_shrink(tpool, PFC_FALSE);
		}
		tpool->tp_lastreap = nthreads;
		TPOOL_UNLOCK(tpool);
	}

	pfc_tpool_unlock();
}

/*
 * static void
 * pfc_tpool_shrink(tpool_t *tpool, pfc_bool_t force)
 *	Shrink the thread pool.
 *	If `force' is PFC_TRUE, free threads are reduced to the minimum.
 *	If PFC_FALSE, this function deletes free threads at most tp_reap
 *	threads.
 *
 * Remarks:
 *	The caller must call this function with holding the thread pool lock.
 */
static void
pfc_tpool_shrink(tpool_t *tpool, pfc_bool_t force)
{
	uint32_t	delete;

	if (TPOOL_IS_SHUTDOWN(tpool) ||
	    tpool->tp_nfree <= tpool->tp_free_min) {
		return;
	}

	delete = tpool->tp_nfree - tpool->tp_free_min;
	if (force) {
		/* Shrink free threads by force. */
		pfc_log_verbose("%s: Reap %u threads in the pool by force.",
				TPOOL_NAME(tpool), delete);
		pfc_tpool_delete_free(tpool, delete);

		return;
	}

	if (delete > tpool->tp_reap) {
		/* Only tp_reap threads can be reaped at once. */
		delete = tpool->tp_reap;
	}

	/* Reap old free threads which seem unnecessary. */
	if (delete > 0) {
		delete = pfc_tpool_delete_aged(tpool, delete);
	}
	if (delete > 0) {
		pfc_log_verbose("%s: Reaped %u threads in the pool.",
				TPOOL_NAME(tpool), delete);
	}
}

/*
 * static uint32_t
 * pfc_tpool_delete_aged(tpool_t *tpool, uint32_t nthreads)
 *	Delete old free threads.
 *
 *	This function deletes old free threads using the aged flag.
 *	If the aged flag is not set in the state of the free thread, it is
 *	set and the thread stays on the free list. Once a job is scheduled
 *	to the free thread, the aged flag is cleared.
 *	If the aged flag is set, it means that the thread has never run since
 *	the last call of this function. So it is deleted as old free thread.
 *
 *	`nthreads' is the maximum number of free threads to be deleted.
 *
 * Calling/Exit State:
 *	The number of actually removed free threads is returned.
 *
 * Remarks:
 *	The caller must call this function with holding the thread pool lock.
 */
static uint32_t
pfc_tpool_delete_aged(tpool_t *tpool, uint32_t nthreads)
{
	uint32_t	deleted = 0;
	pfc_list_t	*list, *next;

	/* Walk through free thread list, and test aged flag. */
	PFC_LIST_FOREACH_SAFE(&tpool->tp_free, list, next) {
		tpool_thread_t	*ttp =
			PFC_CAST_CONTAINER(list, tpool_thread_t, tt_list);
		uint16_t	state;

		TPOOL_THREAD_LOCK(ttp);
		state = ttp->tt_state;
		if (state & TPOOL_THR_STATE_FREE_AGED) {
			if (nthreads != 0) {
				/*
				 * This thread has never scheduled since the
				 * last reaper thread task. Delete this thread.
				 */
				deleted++;
				nthreads--;
				pfc_list_remove(list);
				ttp->tt_state =
					state | TPOOL_THR_STATE_SHUTDOWN;
				TPOOL_THREAD_SIGNAL(ttp);
			}
		}
		else {
			/* Set aged flag to this thread. */
			ttp->tt_state = state | TPOOL_THR_STATE_FREE_AGED;
		}
		TPOOL_THREAD_UNLOCK(ttp);
	}

	PFC_ASSERT(tpool->tp_nfree >= deleted);
	tpool->tp_nfree -= deleted;

	return deleted;
}

/*
 * static void
 * pfc_tpool_delete_free(tpool_t *tpool, uint32_t nthreads)
 *	Delete specified number of free threads in the thread pool.
 *
 * Remarks:
 *	The caller must call this function with holding the thread pool lock.
 *	In addition, the caller must guarantee that the specified number of
 *	free threads exist in the pool.
 */
static void
pfc_tpool_delete_free(tpool_t *tpool, uint32_t nthreads)
{
	tpool->tp_nfree -= nthreads;

	while (nthreads > 0) {
		pfc_list_t	*list;
		tpool_thread_t	*ttp;

		list = pfc_list_pop(&tpool->tp_free);
		PFC_ASSERT(list != NULL);
		ttp = PFC_CAST_CONTAINER(list, tpool_thread_t, tt_list);

		TPOOL_THREAD_LOCK(ttp);
		ttp->tt_state |= TPOOL_THR_STATE_SHUTDOWN;
		TPOOL_THREAD_SIGNAL(ttp);
		TPOOL_THREAD_UNLOCK(ttp);

		nthreads--;
	}
}

/*
 * static pfc_cptr_t
 * wdt_getkey(pfc_rbnode_t *node)
 *	Return the key of thread pool instance specified by `node'.
 *	`node' must be a pointer to tp_name in tpool_t.
 */
static pfc_cptr_t
pfc_tpool_getkey(pfc_rbnode_t *node)
{
	tpool_t	*tpool = TPOOL_NODE2PTR(node);

	return (pfc_cptr_t)TPOOL_NAME(tpool);
}
