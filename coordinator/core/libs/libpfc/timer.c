/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * timer.c - PFC timer system.
 */

/*
 * Use "timer" as log identifier.
 */
static const char	log_ident[] = "timer";
#undef	PFC_LOG_IDENT
#define	PFC_LOG_IDENT	log_ident

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pfc/base.h>
#include <pfc/synch.h>
#include <pfc/list.h>
#include <pfc/rbtree_ex.h>
#include <pfc/taskq.h>
#include <pfc/atomic.h>
#include <pfc/thread.h>
#include <pfc/timer.h>
#include <pfc/log.h>
#include <pfc/refptr.h>
#include "taskq_impl.h"

/*
 * Cast 32-bit integer value into Red-Black Tree key.
 */
#define	TIMER_IDKEY(id)		((pfc_cptr_t)(uintptr_t)(id))

/*
 * Clock ticks
 */
typedef	uint64_t	pfc_tick_t;

/*
 * Timer instance.
 */
typedef struct timersys {
	pfc_rbtree_t	t_idtree;		/* timeout ID tree */
	pfc_rbtree_t	t_ticktree;		/* timeout tick tree */
	pfc_mutex_t	t_mutex;		/* mutex for this timer */
	pfc_cond_t	t_cond;			/* condvar for this timer */
	pfc_tick_t	t_nextrun;		/* next run time of thread */
	pfc_tick_t	t_resolution;		/* interval of thread */
	pfc_refptr_t	*t_poolname;		/* name of thread pool */
	pfc_rbnode_t	t_node;			/* tree node */
	pfc_timer_t	t_id;			/* timer ID */
	pfc_thread_t	t_thread;		/* thread ID */
	pfc_taskq_t	t_taskq;		/* taskq for dispatch */
	uint32_t	t_flags;		/* flags */
	uint32_t	t_count;		/* number of timeout */
	pfc_timeout_t	t_toid_next;		/* next timeout ID */
} timersys_t;

#define	TIMER_NODE2PTR(node)	PFC_CAST_CONTAINER((node), timersys_t, t_node)

/*
 * Timeout entry.
 */
typedef struct timeout {
	pfc_list_t	to_list;		/* timeout list */
	pfc_timeout_t	to_id;			/* timeout ID */
	pfc_timespec_t	to_time;		/* specified duration */
	pfc_tick_t	to_tick;		/* run time */
	pfc_taskfunc_t	to_func;		/* function to call */
	pfc_ptr_t	to_arg;			/* argument to function */
	pfc_taskdtor_t	to_dtor;		/* destructor of argument */
	pfc_rbnode_t	to_node;		/* tree node */
} timeout_t;

#define	TIMEOUT_LIST2PTR(list)	PFC_CAST_CONTAINER((list), timeout_t, to_list)
#define	TIMEOUT_NODE2PTR(list)	PFC_CAST_CONTAINER((node), timeout_t, to_node)

/*
 * Head of the timeout list per expiry time.
 */
typedef struct {
	pfc_tick_t	tl_tick;		/* time to run in tick */
	pfc_list_t	tl_list;		/* list of timeout_t */
	pfc_rbnode_t	tl_node;		/* tree node */
} tick_list_t;

#define	TICK_NODE2PTR(node)					\
	PFC_CAST_CONTAINER((node), tick_list_t, tl_node)

#define	TICK_KEY(tick)		PFC_RBTREE_KEY64(tick)

/*
 * Timer flags.
 */
#define	TIMER_SHUTDOWN		0x0001

/*
 * Lock/unlock timer instance.
 */
#define	TIMER_LOCK(tip)		pfc_mutex_lock(&(tip)->t_mutex);
#define	TIMER_UNLOCK(tip)	pfc_mutex_unlock(&(tip)->t_mutex);

/*
 * Macros for timer condition variable operation.
 */
#define	TIMER_SIGNAL(tip)						\
	PFC_ASSERT_INT(pfc_cond_signal(&(tip)->t_cond), 0)
#define	TIMER_WAIT(tip)							\
	PFC_ASSERT_INT(pfc_cond_wait(&(tip)->t_cond, &(tip)->t_mutex), 0)
#ifdef	PFC_VERBOSE_DEBUG
#define	TIMER_TIMEDWAIT_ABS(tip, abstime)				\
	do {								\
		int	__err;						\
									\
		__err = pfc_cond_timedwait_abs(&(tip)->t_cond,		\
					       &(tip)->t_mutex,		\
					       (abstime));		\
		PFC_ASSERT(__err == 0 || __err == ETIMEDOUT);		\
	} while (0)
#else	/* !PFC_VERBOSE_DEBUG */
#define	TIMER_TIMEDWAIT_ABS(tip, abstime)	\
	((void)pfc_cond_timedwait_abs(&(tip)->t_cond, &(tip)->t_mutex,	\
				      (abstime)))
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * Timer granularity.
 */
#define	TIMER_GRANULARITY	100000000U	/* 100ms */
#define	TIMER_NSECS_PER_TICK	TIMER_GRANULARITY
#define	TIMER_TICKS_PER_SEC	(PFC_CLOCK_NANOSEC / TIMER_NSECS_PER_TICK)

/*
 * Timer default resolution.
 */
#define	TIMER_DEFAULT_RESOLUTION	1U	/* 1 tick */

/*
 * Maximum amount of time, in timer tick, that timer execution thread will
 * block. (1 day)
 */
#define	TIMER_MAXSLEEP		((pfc_tick_t)(86400 * TIMER_TICKS_PER_SEC))

/*
 * Determine whether the timer resolution represented by timespec is valid
 * or not.
 */
#define	TIMER_RESOLUTION_IS_VALID(res)					\
	((pfc_ulong_t)(res)->tv_sec < PFC_TIMER_MAXRES &&		\
	 (pfc_ulong_t)(res)->tv_nsec < PFC_CLOCK_NANOSEC)

/*
 * Timer ID for the next allocation.
 */
static pfc_timer_t	timer_id_next;

/*
 * Internal prototypes.
 */
static void	*timer_main(void *);
static int	timer_register(timersys_t *tip, const pfc_timespec_t *timeout,
			       pfc_tick_t tick, pfc_taskfunc_t func,
			       void *PFC_RESTRICT arg, pfc_taskdtor_t dtor,
			       pfc_timeout_t *toidp);
static pfc_bool_t	timer_execute(timersys_t *tip, pfc_tick_t curtick);
static void	timer_dtor(pfc_taskdtor_t dtor, pfc_ptr_t arg);
static int	timer_alloc(timersys_t **);
static void	timer_free(timersys_t *);
static int	timer_timeout_alloc(timersys_t *tip, timeout_t **topp);

static pfc_cptr_t	timer_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	timer_timeout_getkey(pfc_rbnode_t *node);
static void		timer_timeout_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);
static pfc_cptr_t	timer_tick_getkey(pfc_rbnode_t *node);
static void		timer_tick_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

/*
 * Red-Black tree which keeps pairs of timer ID and timer instance.
 */
static pfc_rbtree_ex_t	timer_tree =
	PFC_RBTREE_EX_INITIALIZER(pfc_rbtree_uint32_compare, timer_getkey);

/*
 * void PFC_ATTR_HIDDEN
 * pfc_timer_init(void)
 *	Initialize timer subsystem.
 */
void PFC_ATTR_HIDDEN
pfc_timer_init(void)
{
	/* Nothing to do. */
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_timer_fini(void)
 *	Shutdown timer subsystem.
 */
void PFC_ATTR_HIDDEN
pfc_timer_fini(void)
{
	size_t  sz;

	sz = pfc_rbtree_ex_get_size(&timer_tree);
	if (sz != 0) {
		pfc_log_warn("%" PFC_PFMT_SIZE_T " timer still remain.", sz);
	}
}

/*
 * static inline pfc_timer_t
 * pfc_timer_new_id(void)
 *	Allocate new timer ID.
 *
 * Remarks:
 *	We assume that this function never fails.
 *	We believe that it is impossible to create UINT32_MAX timer
 *	at a time!
 */
static inline pfc_timer_t
pfc_timer_new_id(void)
{
	pfc_timer_t	tid;

	tid = pfc_atomic_inc_uint32_old(&timer_id_next) + 1;
	if (PFC_EXPECT_FALSE(tid == PFC_TIMER_INVALID_ID)) {
		tid = pfc_atomic_inc_uint32_old(&timer_id_next) + 1;
	}

	return tid;
}

/*
 * static inline int
 * pfc_timer_lookup(pfc_timer_t tid, timersys_t **tipp)
 *	Get instance associated with the ID in timer_tree.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to timersys_t is set to `*tipp',
 *	and then zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding global timer ID lock in
 *	reader or writer mode.
 */
static inline int
pfc_timer_lookup(pfc_timer_t tid, timersys_t **tipp)
{
	pfc_rbnode_t	*node;
	timersys_t	*tip;

	node = pfc_rbtree_ex_get_l(&timer_tree, TIMER_IDKEY(tid));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		return ENOENT;
	}

	tip = TIMER_NODE2PTR(node);
	if (PFC_EXPECT_FALSE(tip->t_flags & TIMER_SHUTDOWN)) {
		return ESHUTDOWN;
	}

	*tipp = tip;
	return 0;
}

/*
 * static inline pfc_tick_t
 * pfc_timer_time2tick(const pfc_timespec_t *tsp)
 *	Convert timespec to clock ticks.
 *	The return value is rounded up.
 */
static inline pfc_tick_t
pfc_timer_time2tick(const pfc_timespec_t *tsp)
{
	pfc_tick_t	tick;

	tick = (pfc_tick_t)tsp->tv_sec * TIMER_TICKS_PER_SEC +
	       (tsp->tv_nsec + TIMER_NSECS_PER_TICK - 1) / TIMER_NSECS_PER_TICK;
	return tick;
}

/*
 * static inline void
 * pfc_timer_tick2time(pfc_tick_t tick, pfc_timespec_t *tsp)
 *	Convert clock ticks to timespec.
 */
static inline void
pfc_timer_tick2time(pfc_tick_t tick, pfc_timespec_t *tsp)
{
	tsp->tv_sec = tick / TIMER_TICKS_PER_SEC;
	tsp->tv_nsec = (tick % TIMER_TICKS_PER_SEC) * TIMER_NSECS_PER_TICK;
}

/*
 * static inline pfc_tick_t
 * pfc_timer_curtick()
 *	Retrieve the current PFC system clock by clock ticks.
 *	The return value is rounded down.
 */
static inline pfc_tick_t
pfc_timer_curtick()
{
	pfc_tick_t	tick;
	pfc_timespec_t	ts;

	PFC_VERIFY_INT(pfc_clock_gettime(&ts), 0);
	tick = (pfc_tick_t)ts.tv_sec * TIMER_TICKS_PER_SEC
		+ ts.tv_nsec / TIMER_NSECS_PER_TICK;
	return tick;
}

/*
 * static inline void
 * timer_timeout_free(timeout_t *top)
 *	Free the specified timeout instance.
 */
static inline void
timer_timeout_free(timeout_t *top)
{
	free(top);
}

/*
 * static void
 * timer_list_free(tick_list_t *tlp)
 *	Free the specified timeout list per expiry time.
 *
 * Remarks:
 *	The caller must guarantee that no timeout is linked to the list.
 */
static void
timer_list_free(tick_list_t *tlp)
{
	PFC_ASSERT(pfc_list_is_empty(&tlp->tl_list));
	free(tlp);
}

/*
 * int
 * pfc_timer_create(pfc_timer_t *PFC_RESTRICT tidp,
 *		    const char *PFC_RESTRICT poolname, pfc_taskq_t tqid,
 *		    const pfc_timespec_t *resolution)
 *	Create a new timer.
 *	This function creates a timer execution thread and associate it
 *	with a new timer.
 *
 * Calling/Exit State:
 *      Upon successful completion, timer instance is set to *tidp
 *      and zero is returned.
 *      Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_timer_create(pfc_timer_t *PFC_RESTRICT tidp,
		 const char *PFC_RESTRICT poolname, pfc_taskq_t tqid,
		 const pfc_timespec_t *resolution)
{
	timersys_t	*tip = NULL;
	pfc_timer_t	tid;
	int		err;
	pfc_tick_t	restick;

	if (resolution == NULL) {
		restick = TIMER_DEFAULT_RESOLUTION;
	} else {
		if (PFC_EXPECT_FALSE(!TIMER_RESOLUTION_IS_VALID(resolution))) {
			return EINVAL;
		}
		restick = pfc_timer_time2tick(resolution);
		if (restick == 0) {
			restick = TIMER_DEFAULT_RESOLUTION;
		}
	}

	/* Allocate timer instance */
	err = timer_alloc(&tip);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	if (poolname == NULL) {
		tip->t_poolname = NULL;
	} else {
		tip->t_poolname = pfc_refptr_string_create(poolname);
		if (PFC_EXPECT_FALSE(tip->t_poolname == NULL)) {
			timer_free(tip);
			return ENOMEM;
		}
	}
	tip->t_resolution = restick;
	tip->t_taskq = tqid;

	pfc_rbtree_ex_wrlock(&timer_tree);
	while (1) {
		/* Allocate a new timer ID. */
		tid = pfc_timer_new_id();
		tip->t_id = tid;

		/* Register timer ID and timer instance. */
		err = pfc_rbtree_ex_put_l(&timer_tree, &tip->t_node);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/* Create a timer scheduling thread. */
			err = pfc_thread_createat(&tip->t_thread, poolname,
						  timer_main, (void *)tip, 0);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto error;
			}
			break;
		}

		/*
		 * Assigned ID is not available.
		 * We must assign another ID.
		 */
		PFC_ASSERT(err == EEXIST);
	}
	pfc_rbtree_ex_unlock(&timer_tree);

	*tidp = tid;
	return 0;

error:
	pfc_rbtree_ex_remove_node_l(&timer_tree, &tip->t_node);
	pfc_rbtree_ex_unlock(&timer_tree);
	timer_free(tip);

	return err;
}

/*
 * int
 * pfc_timer_destroy(pfc_timer_t tid)
 *	Destroy the timer.
 *	This function waits for the timer executor thread to be terminated.
 *
 * Calling/Exit State:
 *      Upon successful completion, zero is returned.
 *      Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_timer_destroy(pfc_timer_t tid)
{
	timersys_t	*tip;
	pfc_rbnode_t	*node;
	int		err;

	/* Make the specified timer invisible. */
	pfc_rbtree_ex_wrlock(&timer_tree);
	node = pfc_rbtree_ex_remove_l(&timer_tree, TIMER_IDKEY(tid));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		pfc_rbtree_ex_unlock(&timer_tree);

		return ENOENT;
	}

	/* Notify to thread */
	tip = TIMER_NODE2PTR(node);
	TIMER_LOCK(tip);
	tip->t_flags |= TIMER_SHUTDOWN;
	PFC_ASSERT_INT(pfc_cond_signal(&tip->t_cond), 0);
	TIMER_UNLOCK(tip);

	pfc_rbtree_ex_unlock(&timer_tree);

	err = pfc_thread_join(tip->t_thread, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	timer_free(tip);
	return 0;
}

/*
 * int
 * pfc_timer_post(pfc_timer_t tid, const pfc_timespec_t *timeout,
 *		  pfc_taskfunc_t func, void *PFC_RESTRICT arg,
 *		  pfc_timeout_t *toidp)
 *	Post a timeout request. The given function 'func' is executed
 *	after the specified length of time 'timeout'.
 *
 * Calling/Exit State:
 *      Upon successful completion, timeout ID is set to *toidp
 *      and zero is returned.
 *      Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_timer_post(pfc_timer_t tid, const pfc_timespec_t *timeout,
	       pfc_taskfunc_t func, void *PFC_RESTRICT arg,
	       pfc_timeout_t *toidp)
{
	return pfc_timer_post_dtor(tid, timeout, func, arg, NULL, toidp);
}

/*
 * int
 * pfc_timer_post_dtor(pfc_timer_t tid, const pfc_timespec_t *timeout,
 *		       pfc_taskfunc_t func, void *PFC_RESTRICT arg,
 *		       pfc_taskdtor_t dtor, pfc_timeout_t *toidp)
 *	Post a timeout request. The given function 'func' is executed
 *	after the specified length of time 'timeout'.
 *
 *	'dtor' is a destructor of 'arg'. If a timeout is expired or removed,
 *	an associated 'dtor' is called with specifying arg.
 *
 * Calling/Exit State:
 *      Upon successful completion, timeout ID is set to *toidp
 *      and zero is returned.
 *      Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_timer_post_dtor(pfc_timer_t tid, const pfc_timespec_t *timeout,
		    pfc_taskfunc_t func, void *PFC_RESTRICT arg,
		    pfc_taskdtor_t dtor, pfc_timeout_t *toidp)
{
	timersys_t	*tip;
	int		err;
	pfc_timespec_t	abstime;
	pfc_tick_t	abstick, sec;

	/* Convert time interval to absolute ticks */
	err = pfc_clock_abstime(&abstime, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	abstick = pfc_timer_time2tick(&abstime);

	/* Ensure that the timer tick never causes integer overflow. */
	sec = abstick / TIMER_TICKS_PER_SEC;
	if (PFC_EXPECT_FALSE(sec != (pfc_tick_t)abstime.tv_sec &&
			     sec != (pfc_tick_t)abstime.tv_sec + 1)) {
		return ERANGE;
	}

	pfc_rbtree_ex_rdlock(&timer_tree);
	err = pfc_timer_lookup(tid, &tip);
	if (PFC_EXPECT_TRUE(err == 0)) {
		TIMER_LOCK(tip);
		err = timer_register(tip, timeout, abstick, func, arg, dtor,
				     toidp);
		TIMER_UNLOCK(tip);
	}
	pfc_rbtree_ex_unlock(&timer_tree);

	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	return 0;
}

/*
 * static int
 * timer_register(timersys_t *tip, const pfc_timespec_t *timeout,
 *		  pfc_tick_t tick, pfc_taskfunc_t func, void *PFC_RESTRICT arg,
 *		  pfc_taskdtor_t dtor, pfc_timeout_t *toidp)
 *	Allocate timeout instance and register the instance to
 *	ID tree and tick tree.
 *	If the executor thread is in long term sleep, wake up thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, an ID for a new timeout instance is set
 *	to `*toidp', and then zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the timer lock.
 */
static int
timer_register(timersys_t *tip, const pfc_timespec_t *timeout, pfc_tick_t tick,
	       pfc_taskfunc_t func, void *PFC_RESTRICT arg,
	       pfc_taskdtor_t dtor, pfc_timeout_t *toidp)
{
	timeout_t	*top = NULL;
	int		err;
	pfc_rbnode_t	*node;
	tick_list_t	*tlp;

	/* Allocate timeout instance and register to ID tree. */
	err = timer_timeout_alloc(tip, &top);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	top->to_time = *timeout;
	top->to_tick = tick;
	top->to_func = func;
	top->to_arg = arg;
	top->to_dtor = dtor;

	/* Link timeout instance to the list associated with expiry tick. */
	node = pfc_rbtree_get(&tip->t_ticktree, TICK_KEY(top->to_tick));
	if (node == NULL) {
		/* Allocate a new list head. */
		tlp = (tick_list_t *)malloc(sizeof(*tlp));
		if (PFC_EXPECT_FALSE(tlp == NULL)) {
			err = ENOMEM;
			goto error;
		}
		tlp->tl_tick = top->to_tick;
		pfc_list_init(&tlp->tl_list);
		err = pfc_rbtree_put(&tip->t_ticktree, &tlp->tl_node);
		if (PFC_EXPECT_FALSE(err != 0)) {
			free(tlp);
			goto error;
		}
	} else {
		tlp = TICK_NODE2PTR(node);
	}
	pfc_list_push_tail(&tlp->tl_list, &top->to_list);

	if (tip->t_count == 0 ||
	    top->to_tick < (tip->t_nextrun - tip->t_resolution)) {
		/* Let the timer thread recalculate next time to run. */
		TIMER_SIGNAL(tip);
	}
	tip->t_count++;

	if (toidp != NULL) {
		*toidp = top->to_id;
	}
	return 0;

error:
	pfc_rbtree_remove_node(&tip->t_idtree, &top->to_node);
	timer_timeout_free(top);
	return err;
}

/*
 * int
 * pfc_timer_cancel(pfc_timer_t tid, pfc_timeout_t toid)
 *	Cancel the specified timeout.
 *
 * Calling/Exit State:
 *      Upon successful completion, zero is returned.
 *      Otherwise, error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	ESRCH is returned if the given toid is expired.
 */
int
pfc_timer_cancel(pfc_timer_t tid, pfc_timeout_t toid)
{
	timersys_t	*tip;
	int		err;
	pfc_taskdtor_t	dtor = NULL;
	pfc_ptr_t	argp = NULL;

	pfc_rbtree_ex_rdlock(&timer_tree);

	/* Obtain timer instance associated with the given ID. */
	err = pfc_timer_lookup(tid, &tip);

	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_rbnode_t	*node;

		TIMER_LOCK(tip);

		/* Try to make specified timeout instance invisible. */
		node = pfc_rbtree_remove(&tip->t_idtree, TIMER_IDKEY(toid));
		if (PFC_EXPECT_TRUE(node != NULL)) {
			timeout_t	*top = TIMEOUT_NODE2PTR(node);
			pfc_rbnode_t	*tnode;
			tick_list_t	*tlp;

			/* Remove from tick tree. */
			tnode = pfc_rbtree_get(&tip->t_ticktree,
					       TICK_KEY(top->to_tick));
			PFC_ASSERT(tnode != NULL);
			tlp = TICK_NODE2PTR(tnode);
			pfc_list_remove(&top->to_list);
			if (pfc_list_is_empty(&tlp->tl_list)) {
				pfc_rbtree_remove_node(&tip->t_ticktree,
						       tnode);
				timer_list_free(tlp);
			}

			dtor = top->to_dtor;
			argp = top->to_arg;
			tip->t_count--;

			/* Destroy timeout instance. */
			timer_timeout_free(top);
		} else {
			err = ESRCH;
		}

		TIMER_UNLOCK(tip);
	}

	pfc_rbtree_ex_unlock(&timer_tree);

	/* Call destructor. */
	timer_dtor(dtor, argp);

	return err;
}

/*
 * static void *
 * timer_main(void *arg)
 *	Main routine for a timer executor thread.
 */
static void *
timer_main(void *arg)
{
	timersys_t	*tip = (timersys_t *)arg;
	pfc_tick_t	curtick;
	pfc_timespec_t	waittime;

	TIMER_LOCK(tip);
	while (1) {
		pfc_tick_t	nextrun, maxsleep;

		if (tip->t_flags & TIMER_SHUTDOWN) {
			break;
		}

		curtick = pfc_timer_curtick();

		/* Execute timeout */
		if (timer_execute(tip, curtick) == PFC_FALSE) {
			continue;
		}

		if (tip->t_count == 0) {
			/* Waiting for posting */
			tip->t_nextrun = 0;
			TIMER_WAIT(tip);
			continue;
		}

		nextrun = tip->t_nextrun;
		maxsleep = curtick + TIMER_MAXSLEEP;
		if (PFC_EXPECT_FALSE(nextrun > maxsleep)) {
			/*
			 * Too long term sleep may cause unexpected problem
			 * on kernel or system library. So we should limit
			 * sleep interval within TIMER_MAXSLEEP.
			 */
			nextrun = maxsleep;
		}

		/* Waiting for expiring */
		pfc_timer_tick2time(nextrun, &waittime);
		TIMER_TIMEDWAIT_ABS(tip, &waittime);
	}
	TIMER_UNLOCK(tip);

	return NULL;
}

/*
 * static pfc_bool_t
 * timer_execute(timersys_t *tip, pfc_tick_t curtick)
 *	Execute expired timeout entries.
 *
 *	This function executes all timeout entries which have expiry time
 *	less than or equal `curtick'.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned.
 *	If execution is aborted, PFC_FALSE is returned.
 *	The caller must retry execution immediately.
 *
 * Remarks:
 *	This function must be called with holding the timer lock.
 */
static pfc_bool_t
timer_execute(timersys_t *tip, pfc_tick_t curtick)
{
	pfc_rbnode_t	*node;

	/* Retrieve the least expiry tick. */
	while ((node = pfc_rbtree_next(&tip->t_ticktree, NULL)) != NULL) {
		tick_list_t	*tlp = TICK_NODE2PTR(node);
		pfc_list_t	*elem;

		if (tlp->tl_tick > curtick) {
			pfc_tick_t	nextrun;

			/* Update next time to run. */
			nextrun = curtick + tip->t_resolution;
			tip->t_nextrun = (nextrun < tlp->tl_tick)
				? tlp->tl_tick : nextrun;
			PFC_ASSERT(tip->t_count > 0);

			return PFC_TRUE;
		}

		while ((elem = pfc_list_pop(&tlp->tl_list)) != NULL) {
			timeout_t	*top = TIMEOUT_LIST2PTR(elem);
			pfc_task_t	tid;
			int		err;

			/* Dispatch function */
			err = pfc_taskq_dispatch_dtor(tip->t_taskq,
						      top->to_func, top->to_arg,
						      top->to_dtor, 0, &tid);
			if (PFC_EXPECT_FALSE(err != 0)) {
				pfc_taskdtor_t	dtor;
				pfc_ptr_t	argp;

				/*
				 * Failed to dispatch to the taskq
				 * and lost this timeout entry.
				 * dtor must be invoked here.
				 */
				pfc_log_error("dispatch failed: err=%d, "
					      "tqid=%d, toid=%d",
					      err, tip->t_taskq, top->to_id);

				/* Remove timeout entry from ID tree. */
				dtor = top->to_dtor;
				argp = top->to_arg;
				pfc_rbtree_remove_node(&tip->t_idtree,
						       &top->to_node);
				timer_timeout_free(top);
				tip->t_count--;

				/* Call destructor without mutex. */
				TIMER_UNLOCK(tip);
				timer_dtor(dtor, argp);
				TIMER_LOCK(tip);

				/*
				 * Timeout execution must be started from
				 * scratch because the timer lock was released.
				 * We can return without freeing empty tick
				 * list head because it will be freed by the
				 * next call.
				 */
				return PFC_FALSE;
			}

			/* Remove from ID tree */
			pfc_rbtree_remove_node(&tip->t_idtree, &top->to_node);
			timer_timeout_free(top);
			tip->t_count--;
		}
		pfc_rbtree_remove_node(&tip->t_ticktree, node);
		timer_list_free(tlp);
	}

	PFC_ASSERT(tip->t_count == 0);

	return PFC_TRUE;
}

/*
 * static void
 * timer_dtor(pfc_taskdtor_t dtor, pfc_ptr_t arg)
 *	Call the destructor.
 */
static void
timer_dtor(pfc_taskdtor_t dtor, pfc_ptr_t arg)
{
	if (dtor != NULL && arg != NULL) {
		dtor(arg);
	}
}

/*
 * static int
 * timer_alloc(timersys_t **tipp)
 *	Allocate and initialize new timer instance.
 */
static int
timer_alloc(timersys_t **tipp)
{
	timersys_t *tip;
	int err;

	/* Allocate a new timer instance. */
	tip = (timersys_t *)malloc(sizeof(timersys_t));
	if (PFC_EXPECT_FALSE(tip == NULL)) {
		return ENOMEM;
	}

	err = PFC_MUTEX_INIT(&tip->t_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_alloc;
	}
	err = pfc_cond_init(&tip->t_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_mutex;
	}

	pfc_rbtree_init(&tip->t_idtree, pfc_rbtree_uint32_compare,
			timer_timeout_getkey);
	pfc_rbtree_init(&tip->t_ticktree, pfc_rbtree_uint64_compare,
			timer_tick_getkey);

	tip->t_nextrun = 0;
	tip->t_flags = 0;
	tip->t_count = 0;
	tip->t_toid_next = 1;

	*tipp = tip;
	return 0;

error_mutex:
	PFC_ASSERT_INT(pfc_mutex_destroy(&tip->t_mutex), 0);
error_alloc:
	free(tip);
	return err;
}

/*
 * static void
 * timer_free(timersys_t *tip)
 *	Free the specified timer instance.
 *	t_thread must be terminated by the caller.
 */
static void
timer_free(timersys_t *tip)
{
	/* Clear all timeout tick list heads. */
	pfc_rbtree_clear(&tip->t_ticktree, timer_tick_dtor, NULL);

	/* Call destructor for all timeouts, and dispose timeouts. */
	pfc_rbtree_clear(&tip->t_idtree, timer_timeout_dtor, NULL);

	PFC_ASSERT_INT(pfc_cond_destroy(&tip->t_cond), 0);
	PFC_ASSERT_INT(pfc_mutex_destroy(&tip->t_mutex), 0);
	if (tip->t_poolname != NULL) {
		pfc_refptr_put(tip->t_poolname);
	}
	free(tip);
}

/*
 * static int
 * timer_timeout_alloc(timersys_t *tip, timeout_t **topp)
 *	Allocate new timeout instance.
 *
 * Remarks:
 *	This function must be called with holding the timer lock.
 */
static int
timer_timeout_alloc(timersys_t *tip, timeout_t **topp)
{
	timeout_t	*top;
	int		err;

	/* Allocate a new timeout instance. */
	top = (timeout_t *)malloc(sizeof(*top));
	if (PFC_EXPECT_FALSE(top == NULL)) {
		return ENOMEM;
	}

	while (1) {
		/* Assign new timeout ID */
		top->to_id = tip->t_toid_next++;
		if (tip->t_toid_next == PFC_TIMER_INVALID_TIMEOUTID) {
			tip->t_toid_next++;
		}

		/* Register timeout ID and timeout instance. */
		err = pfc_rbtree_put(&tip->t_idtree, &top->to_node);
		if (PFC_EXPECT_TRUE(err == 0)) {
			break;
		}

		/*
		 * Assigned ID is not available.
		 * We must assign another ID.
		 */
		PFC_ASSERT(err == EEXIST);
	}

	if (PFC_EXPECT_FALSE(err != 0)) {
		free(top);
		return err;
	}

	*topp = top;
	return 0;
}

/*
 * static pfc_cptr_t
 * timer_getkey(pfc_rbnode_t *node)
 *	Return the key of timer instance specified by `node'.
 *	`node' must be a pointer to t_node in timersys_t.
 */
static pfc_cptr_t
timer_getkey(pfc_rbnode_t *node)
{
	timersys_t	*tip = TIMER_NODE2PTR(node);

	return (pfc_cptr_t)(uintptr_t)tip->t_id;
}

/*
 * static pfc_cptr_t
 * timer_timeout_getkey(pfc_rbnode_t *node)
 *	Return the key of timeout instance specified by `node'.
 *	`node' must be a pointer to to_node in timeout_t.
 */
static pfc_cptr_t
timer_timeout_getkey(pfc_rbnode_t *node)
{
	timeout_t	*top = TIMEOUT_NODE2PTR(node);

	return TIMER_IDKEY(top->to_id);
}

/*
 * static void
 * timer_timeout_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of timeout instance.
 *	`node' must be a pointer to to_node in timeout_t.
 *
 *	This function is called when t_idtree is cleared by the call of
 *	pfc_rbtree_clear();
 */
static void
timer_timeout_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	timeout_t	*top = TIMEOUT_NODE2PTR(node);

	/* Call destructor of this timeout instance. */
	timer_dtor(top->to_dtor, top->to_arg);

	/* Destroy timeout instance. */
	timer_timeout_free(top);
}

/*
 * static pfc_cptr_t
 * timer_tick_getkey(pfc_rbnode_t *node)
 *	Return the key of timeout list head specified by `node'.
 *	`node' must be a pointer to tl_node in tick_list_t.
 */
static pfc_cptr_t
timer_tick_getkey(pfc_rbnode_t *node)
{
	tick_list_t	*tlp = TICK_NODE2PTR(node);

	return TICK_KEY(tlp->tl_tick);
}

/*
 * static void
 * timer_tick_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of timeout list head.
 *	`node' must be a pointer to tl_node in tick_list_t.
 *
 *	This function is called when t_ticktree is cleared by the call of
 *	pfc_rbtree_clear();
 */
static void
timer_tick_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	tick_list_t	*tlp = TICK_NODE2PTR(node);

#ifdef	PFC_VERBOSE_DEBUG
	/* Make the list empty. */
	pfc_list_init(&tlp->tl_list);
#endif	/* PFC_VERBOSE_DEBUG */

	/* Destroy timeout list instance. */
	timer_list_free(tlp);
}
