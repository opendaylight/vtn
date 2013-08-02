/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ptimer.c - Simple periodic timer.
 *
 * This file implements simple periodic timer for libpfc internal use.
 * Unlike pfc_timer_t, periodic timer does not use task queue. Timeout posted
 * to the periodic timer runs on a single thread.
 */

/*
 * Use "ptimer" as log identifier.
 */
static const char	log_ident[] = "ptimer";
#undef	PFC_LOG_IDENT
#define	PFC_LOG_IDENT	log_ident

#include <string.h>
#include <pthread.h>
#include <pfc/log.h>
#include <pfc/synch.h>
#include <pfc/clock.h>
#include <pfc/rbtree.h>
#include <pfc/list.h>
#include "ptimer.h"

/*
 * Periodic timeout instance.
 */
struct __ptimeout;
typedef struct __ptimeout	ptimeout_t;

struct __ptimeout {
	pfc_ptimeout_t		pto_id;		/* timeout ID */
	pfc_ptmfunc_t		pto_func;	/* timeout function */
	pfc_ptr_t		pto_arg;	/* arbitrary data */
	pfc_list_t		pto_sibling;	/* another timeouts to run */
	pfc_list_t		pto_oneshot;	/* one shot timer list */

	/*
	 * Temporary link list used for expiration.
	 * Only timer thread uses pto_explist.
	 */
	pfc_list_t		pto_explist;

	pfc_timespec_t		pto_nextrun;	/* absolute time to run */
	pfc_timespec_t		pto_interval;	/* timer interval */
	uint32_t		pto_flags;	/* flags */
	pfc_rbnode_t		pto_tmnode;	/* tree node for pt_tmtree */
	pfc_rbnode_t		pto_idnode;	/* tree node for pt_idtree */
};

#define	PTIMEOUT_TM_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ptimeout_t, pto_tmnode)
#define	PTIMEOUT_ID_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ptimeout_t, pto_idnode)

#define	PTIMEOUT_IDKEY(id)	((pfc_cptr_t)(uintptr_t)(id))

#define	PTIMEOUT_EXPLIST2PTR(list)				\
	PFC_CAST_CONTAINER((list), ptimeout_t, pto_explist)
#define	PTIMEOUT_ONESHOT2PTR(list)				\
	PFC_CAST_CONTAINER((list), ptimeout_t, pto_oneshot)
#define	PTIMEOUT_SIBLING_NEXT(top)					\
	PFC_CAST_CONTAINER((top)->pto_sibling.pl_next, ptimeout_t, pto_sibling)

/*
 * Flags for pto_flags.
 */
#define	PTMOF_ENABLED		PFC_CONST_U(0x1)	/* enabled */
#define	PTMOF_TMTREE		PFC_CONST_U(0x2)	/* on pt_tmtree */
#define	PTMOF_SIBLING		PFC_CONST_U(0x4)	/* on sibling list */
#define	PTMOF_ONESHOT		PFC_CONST_U(0x8)	/* on oneshot list */

#define	PTMOF_REGISTERED	(PTMOF_TMTREE | PTMOF_SIBLING)

/*
 * Periodic timer instance.
 */
struct __pfc_ptimer {
	/*
	 * Timer thread ID.
	 * Periodic timer does not use PFC thread because it never terminated
	 * as long as PFC daemon runs.
	 */
	pthread_t		pt_thread;

	/*
	 * Timer mutex and condition variable.
	 */
	pfc_mutex_t		pt_mutex;
	pfc_cond_t		pt_cond;

	/*
	 * Red-Black tree which keeps timeouts.
	 * pt_tmtree uses next time to run as key, and pt_idtree uses timeout
	 * ID as key.
	 */
	pfc_rbtree_t		pt_tmtree;
	pfc_rbtree_t		pt_idtree;

	pfc_list_t		pt_oneshot;	/* one shot timer list */
	pfc_ptimeout_t		pt_nextid;	/* timeout ID for next */
	volatile uint32_t	pt_flags;	/* flags */
};

#define	PTIMER_LOCK(timer)	pfc_mutex_lock(&(timer)->pt_mutex)
#define	PTIMER_UNLOCK(timer)	pfc_mutex_unlock(&(timer)->pt_mutex)

#define	PTIMER_WAIT(timer)					\
	pfc_cond_wait(&(timer)->pt_cond, &(timer)->pt_mutex)
#define	PTIMER_TIMEDWAIT_ABS(timer, abs)				\
	pfc_cond_timedwait_abs(&(timer)->pt_cond, &(timer)->pt_mutex, (abs))
#define	PTIMER_SIGNAL(timer)	pfc_cond_signal(&(timer)->pt_cond)

/*
 * Initial value of pt_nextid.
 */
#define	PTIMER_NEXTID_INITIAL	PFC_CONST_U(1)

/*
 * Flags for pt_flags.
 */
#define	PTF_DESTROYED		PFC_CONST_U(0x1)	/* destroyed */

#define	PTIMER_IS_DESTROYED(timer)				\
	PFC_EXPECT_FALSE((timer)->pt_flags & PTF_DESTROYED)

/*
 * Stack size for timer thread.
 */
#define	PTIMER_STKSIZE		PFC_CONST_U(0x10000)	/* 64K */

/*
 * Maximum amount of time, in seconds, that timer thread will block.
 */
#define	PTIMER_MAXSLEEP		PFC_CONST_U(86400)	/* 1 day */

/*
 * Internal prototypes.
 */
static void	*ptimer_thread(void *arg);
static int	ptimer_run(pfc_ptimer_t timer, pfc_timespec_t **abstimepp);
static void	ptimer_expire_makelist(pfc_list_t *PFC_RESTRICT expired,
				       ptimeout_t *PFC_RESTRICT top);
static int	ptimer_expire(pfc_ptimer_t timer,
			      ptimeout_t *PFC_RESTRICT top);
static int	ptimer_oneshot(pfc_ptimer_t timer);
static void	ptimer_register(pfc_ptimer_t timer,
				ptimeout_t *PFC_RESTRICT top);
static void	ptimer_unregister(pfc_ptimer_t timer,
				  ptimeout_t *PFC_RESTRICT top);
static int	ptimer_get_nextrun(pfc_timespec_t *PFC_RESTRICT nextrun,
				   const pfc_timespec_t *PFC_RESTRICT ival);
static int	ptimer_gettime(pfc_timespec_t *tsp);

static void	ptimeout_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

static ptimeout_t	*ptimeout_lookup(pfc_ptimer_t timer, pfc_ptimeout_t id);
static pfc_cptr_t	ptimeout_tm_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ptimeout_id_getkey(pfc_rbnode_t *node);

/*
 * int
 * pfc_ptimer_create(pfc_ptimer_t *timerp)
 *	Create periodic timer instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, a periodic timer identifier is set to the
 *	buffer pointed by `timerp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ptimer_create(pfc_ptimer_t *timerp)
{
	pfc_ptimer_t	timer;
	pthread_attr_t	attr;
	int		err;

	timer = (pfc_ptimer_t)malloc(sizeof(*timer));
	if (PFC_EXPECT_FALSE(timer == NULL)) {
		return ENOMEM;
	}

	err = PFC_MUTEX_INIT(&timer->pt_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	err = pfc_cond_init(&timer->pt_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	pfc_rbtree_init(&timer->pt_tmtree, (pfc_rbcomp_t)pfc_clock_compare,
			ptimeout_tm_getkey);
	pfc_rbtree_init(&timer->pt_idtree, pfc_rbtree_uint32_compare,
			ptimeout_id_getkey);
	pfc_list_init(&timer->pt_oneshot);
	timer->pt_nextid = PTIMER_NEXTID_INITIAL;
	timer->pt_flags = 0;

	/* Create timer thread. */
	PFC_ASSERT_INT(pthread_attr_init(&attr), 0);
	PFC_ASSERT_INT(pthread_attr_setstacksize(&attr, PTIMER_STKSIZE), 0);
	PFC_ASSERT_INT(pthread_attr_setdetachstate
		       (&attr, PTHREAD_CREATE_JOINABLE), 0);

	err = pthread_create(&timer->pt_thread, &attr, ptimer_thread, timer);
	PFC_ASSERT_INT(pthread_attr_destroy(&attr), 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	*timerp = timer;

	return 0;

error:
	free(timer);

	return err;
}

/*
 * int
 * pfc_ptimer_destroy(pfc_ptimer_t timer)
 *	Destroy the given periodic timer.
 *	Note that this function waits for completion of ongoing timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ptimer_destroy(pfc_ptimer_t timer)
{
	int	err;

	PFC_ASSERT(timer != NULL);

	/* Notify the timer thread of shutdown request. */
	PTIMER_LOCK(timer);
	timer->pt_flags |= PTF_DESTROYED;
	PTIMER_SIGNAL(timer);
	PTIMER_UNLOCK(timer);

	err = pthread_join(timer->pt_thread, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Free up timeouts. */
	pfc_rbtree_clear(&timer->pt_idtree, ptimeout_dtor, NULL);

	free(timer);

	return 0;
}

/*
 * int
 * pfc_ptimer_timeout(pfc_ptimer_t timer, pfc_ptmfunc_t func, pfc_ptr_t arg,
 *		      const pfc_ptmattr_t *PFC_RESTRICT attrp,
 *		      pfc_ptimeout_t *PFC_RESTRICT idp)
 *	Register a new periodic timeout to the periodic timer specified by
 *	`timer'.
 *
 *	`func' is a pointer to function to run periodically. `arg' is an
 *	arbitrary data passed to `func'.
 *
 *	`attrp' determines timeout attributes:
 *	  - ptma_interval is interval between periodic timeout in milliseconds.
 *	    `func' will be called periodically every `interval' milliseconds.
 *	    Specifying zero to ptma_interval results in undefined behavior.
 *	  - ptma_enabled determines whether the timeout should be enabled or
 *	    not. If PFC_TRUE is set, the timeout is enabled immediately.
 *	    Otherwise the timeout is registered with disabled state.
 *	    Disabled timeout can be enabled by the call of
 *	    pfc_ptimer_setattr().
 *
 * Calling/Exit State:
 *	Upon successful completion, timeout identifier is set to the buffer
 *	pointed by `idp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ptimer_timeout(pfc_ptimer_t timer, pfc_ptmfunc_t func, pfc_ptr_t arg,
		   const pfc_ptmattr_t *PFC_RESTRICT attrp,
		   pfc_ptimeout_t *PFC_RESTRICT idp)
{
	ptimeout_t	*top;
	pfc_ptimeout_t	start, id;
	pfc_timespec_t	*itsp;
	int		err;

	PFC_ASSERT(timer != NULL);
	PFC_ASSERT(func != NULL);
	PFC_ASSERT(attrp != NULL && attrp->ptma_interval != 0);
	PFC_ASSERT(idp != NULL);

	/* Allocate a new timeout instance. */
	top = (ptimeout_t *)malloc(sizeof(*top));
	if (PFC_EXPECT_FALSE(top == NULL)) {
		return ENOMEM;
	}

	itsp = &top->pto_interval;
	top->pto_func = func;
	top->pto_arg = arg;
	top->pto_flags = (attrp->ptma_enabled) ? PTMOF_ENABLED : 0;
	pfc_clock_msec2time(itsp, attrp->ptma_interval);

	PTIMER_LOCK(timer);
	start = id = timer->pt_nextid;
	PFC_ASSERT(id != PFC_PTIMEOUT_INVALID);

	/* Assign a new timeout ID. */
	err = 0;
	for (;;) {
		top->pto_id = id;
		err = pfc_rbtree_put(&timer->pt_idtree, &top->pto_idnode);

		/* Prepare timeout ID for next allocation. */
		id++;
		if (PFC_EXPECT_FALSE(id == PFC_PTIMEOUT_INVALID)) {
			id = PTIMER_NEXTID_INITIAL;
		}

		if (PFC_EXPECT_TRUE(err == 0)) {
			timer->pt_nextid = id;
			break;
		}
		PFC_ASSERT(err == EEXIST);

		/* Try next ID. */
		if (PFC_EXPECT_FALSE(id == start)) {
			/* Timeout ID is not available. */
			err = ENFILE;
			goto error;
		}
	}

	/* Register timeout unless it is disabled. */
	if (top->pto_flags & PTMOF_ENABLED) {
		/* Initialize absolute time to run. */
		err = ptimer_get_nextrun(&top->pto_nextrun, itsp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should never happen. */
			goto error_idtree;
		}

		ptimer_register(timer, top);
	}

	*idp = top->pto_id;
	PTIMER_UNLOCK(timer);

	return 0;

error_idtree:
	pfc_rbtree_remove_node(&timer->pt_idtree, &top->pto_idnode);

error:
	PTIMER_UNLOCK(timer);
	free(top);

	return err;
}

/*
 * int
 * pfc_ptimer_setattr(pfc_ptimer_t timer, pfc_ptimeout_t id,
 *		      const pfc_ptmattr_t *PFC_RESTRICT attrp)
 *	Set attributes of the periodic timeout specified by periodic timer
 *	`timer' and timeout ID `id'.
 *
 *	`attrp' is a pointer to pfc_ptmattr_t which keeps new attributes.
 *	Unlike pfc_ptimer_timeout(), pfc_ptimer_setattr() allows zero in
 *	ptma_interval. If ptma_interval is zero, this function does not
 *	change the timer interval.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ptimer_setattr(pfc_ptimer_t timer, pfc_ptimeout_t id,
		   const pfc_ptmattr_t *PFC_RESTRICT attrp)
{
	ptimeout_t	*top;
	uint32_t	flags, interval = attrp->ptma_interval;
	pfc_bool_t	need_reg, need_unreg;
	pfc_timespec_t	nextrun, ival, *ivalp = NULL;
	int		err = 0;

	PFC_ASSERT(timer != NULL);
	PFC_ASSERT(attrp != NULL);

	PTIMER_LOCK(timer);

	/*
	 * Determine timeout instance associated with the given timer and
	 * timeout ID.
	 */
	top = ptimeout_lookup(timer, id);
	if (PFC_EXPECT_FALSE(top == NULL)) {
		err = ENOENT;
		goto out;
	}

	need_reg = need_unreg = PFC_FALSE;
	if (interval != 0) {
		/* Convert interval in milliseconds into pfc_timespec_t. */
		ivalp = &ival;
		pfc_clock_msec2time(ivalp, interval);
		need_reg = need_unreg = PFC_TRUE;
	}

	flags = top->pto_flags;
	if (attrp->ptma_enabled) {
		if ((flags & PTMOF_ENABLED) == 0) {
			/*
			 * pto_nextrun of disabled timeout keeps undefined
			 * value. So it needs to be updated.
			 */
			if (ivalp == NULL) {
				ivalp = &top->pto_interval;
			}
			need_reg = PFC_TRUE;
			top->pto_flags |= PTMOF_ENABLED;
		}
	}
	else {
		/* Never pass disabled timeout to ptimer_register(). */
		need_reg = PFC_FALSE;

		if (flags & PTMOF_ENABLED) {
			need_unreg = PFC_TRUE;
			top->pto_flags &= ~PTMOF_ENABLED;
		}
	}

	if (ivalp != NULL) {
		/*
		 * Calculate next time to run.
		 *
		 * Remarks:
		 *	Never update top->pto_nextrun here, or
		 *	ptimer_unregister() will not work.
		 */
		err = ptimer_get_nextrun(&nextrun, ivalp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should never happen. */
			goto out;
		}
	}

	if (need_unreg) {
		/* Unregister timeout from the timer tree. */
		ptimer_unregister(timer, top);
	}

	if (interval != 0) {
		/* Update timeout interval. */
		top->pto_interval = ival;
	}

	if (need_reg) {
		/* Update next time to run. */
		PFC_ASSERT(ivalp != NULL);
		top->pto_nextrun = nextrun;

		/* Register timeout to the timer tree. */
		ptimer_register(timer, top);
	}

out:
	PTIMER_UNLOCK(timer);

	return err;
}

/*
 * int
 * pfc_ptimer_oneshot(pfc_ptimer_t timer, pfc_ptimeout_t id)
 *	Execute timeout function specified by the timer `timer' and the timeout
 *	ID `id'.
 *
 *	This function invokes timeout function immediately without changing
 *	periodic timer state, unless it is disabled.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Oneshot timer request is not queued. So the number of invocation
 *	may be different from the number of request.
 */
int
pfc_ptimer_oneshot(pfc_ptimer_t timer, pfc_ptimeout_t id)
{
	ptimeout_t	*top;
	uint32_t	flags;
	int		err = 0;

	PFC_ASSERT(timer != NULL);

	PTIMER_LOCK(timer);
	top = ptimeout_lookup(timer, id);
	if (PFC_EXPECT_FALSE(top == NULL)) {
		err = ENOENT;
		goto out;
	}

	flags = top->pto_flags;
	if ((flags & PTMOF_ONESHOT) == 0 && (flags & PTMOF_ENABLED)) {
		/* Schedule oneshot timer. */
		top->pto_flags |= PTMOF_ONESHOT;
		pfc_list_push_tail(&timer->pt_oneshot, &top->pto_oneshot);
		PTIMER_SIGNAL(timer);
	}

out:
	PTIMER_UNLOCK(timer);

	return err;
}

/*
 * static void *
 * ptimer_thread(void *arg)
 *	Start routine of periodic timer thread.
 */
static void *
ptimer_thread(void *arg)
{
	pfc_ptimer_t	timer = (pfc_ptimer_t)arg;

	PTIMER_LOCK(timer);

	while (!PTIMER_IS_DESTROYED(timer)) {
		pfc_timespec_t	ts, *abstime = &ts;
		int		err;

		err = ptimer_run(timer, &abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			if (PFC_EXPECT_FALSE(err != ESHUTDOWN)) {
				/* This should never happen. */
				pfc_log_error("Fatal error on timer thread: %s",
					      strerror(err));
			}
			break;
		}

		/*
		 * Although ptimer_run() updates `abstime' atomically,
		 * another oneshot timer can be scheduled while the timer
		 * thread releases the lock. So we must ensure that the
		 * oneshot timer list is not empty.
		 */
		if (pfc_list_is_empty(&timer->pt_oneshot)) {
			PTIMER_TIMEDWAIT_ABS(timer, abstime);
		}
	}

	PTIMER_UNLOCK(timer);

	return NULL;
}

/*
 * static int
 * ptimer_run(pfc_ptimer_t timer, pfc_timespec_t **abstimepp)
 *	Perform periodic timer work.
 *	This function is called on the timer thread.
 *
 *	`abstimepp' must be an address of pointer to pfc_timespec_t, which
 *	points valid buffer. On successful return, ptimer_run() sets
 *	next time to run the timer to `abstimepp'.
 *	If no timeout is registered to the timer tree, NULL is set to
 *	`*abstimepp'. Otherwise, next time to run the timer is set to the
 *	buffer pointed by `*abstimepp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, next time to run the timer is set to
 *	`abstimepp', and zero is returned.
 *
 *	ESHUTDOWN is returned if the given timer is destroyed.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the timer lock.
 *	Note that it may be released for a while.
 */
static int
ptimer_run(pfc_ptimer_t timer, pfc_timespec_t **abstimepp)
{
	pfc_rbtree_t	*tmtree = &timer->pt_tmtree;
	pfc_rbnode_t	*node;
	pfc_timespec_t	now, *abstimep;
	pfc_list_t	expired = PFC_LIST_INITIALIZER(expired), *elem;
	ptimeout_t	*top;
	int		err;

	/* Obtain current time. */
	err = ptimer_gettime(&now);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This is fatal error. */
		return err;
	}

	if (pfc_rbtree_isempty(tmtree)) {
		/* No timeout is registered. */
		goto empty;
	}

	/* Retrieve expired timeouts. */
	while ((node = pfc_rbtree_next(tmtree, NULL)) != NULL) {
		top = PTIMEOUT_TM_NODE2PTR(node);
		if (pfc_clock_compare(&now, &top->pto_nextrun) < 0) {
			break;
		}

		/* Remove this timeout from the timer tree. */
		pfc_rbtree_remove_node(tmtree, node);

		/* Link all expired timeouts to `expired' list. */
		ptimer_expire_makelist(&expired, top);
	}

	/* Invoke timeout function, and update state. */
	PFC_LIST_FOREACH(&expired, elem) {
		top = PTIMEOUT_EXPLIST2PTR(elem);
		err = ptimer_expire(timer, top);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	/* Invoke scheduled oneshot timeouts. */
	err = ptimer_oneshot(timer);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Update next time to run the timer thread. */
	node = pfc_rbtree_next(tmtree, NULL);
	if (node != NULL) {
		top = PTIMEOUT_TM_NODE2PTR(node);
		abstimep = *abstimepp;
		*abstimep = top->pto_nextrun;

		pfc_log_verbose("now = %lu.%lu, nextrun = %lu.%lu",
				now.tv_sec, now.tv_nsec,
				abstimep->tv_sec, abstimep->tv_nsec);
		return 0;
	}

empty:
	pfc_log_verbose("now = %lu.%lu, nextrun = <null>",
			now.tv_sec, now.tv_nsec);
	*abstimepp = NULL;

	return 0;
}

/*
 * static void
 * ptimer_expire_makelist(pfc_list_t *PFC_RESTRICT expired,
 *			  ptimeout_t *PFC_RESTRICT top)
 *	Link the timeout specified by `top' and all timeouts linked to its
 *	sibling list to the list specified by `expired'.
 *
 *	This is internal function of ptimer_run().
 *
 * Remarks:
 *	This function must be called with holding the timer lock.
 */
static void
ptimer_expire_makelist(pfc_list_t *PFC_RESTRICT expired,
		       ptimeout_t *PFC_RESTRICT top)
{
	ptimeout_t	*t = top;

	/*
	 * Create timeout list using pto_explist.
	 * We can not use pto_sibling because it may be updated while
	 * the timer thread releases the timer lock.
	 */
	for (;;) {
		ptimeout_t	*next = PTIMEOUT_SIBLING_NEXT(t);

		/*
		 * Clear registered flag because this timeout is no longer
		 * registered to the timer tree.
		 */
		t->pto_flags &= ~PTMOF_REGISTERED;

		pfc_list_push_tail(expired, &t->pto_explist);
		if (next == top) {
			break;
		}

		PFC_ASSERT(pfc_clock_compare(&t->pto_nextrun,
					     &next->pto_nextrun) == 0);
		t = next;
	}
}

/*
 * static int
 * ptimer_expire(pfc_ptimer_t timer, ptimeout_t *PFC_RESTRICT top)
 *	Expire the timeout specified by `top'.
 *	This function is called on the timer thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ESHUTDOWN is returned if the given timer is destroyed.
 *
 * Remarks:
 *	- This function must be called with holding the timer lock.
 *	  Note that it may be released for a while.
 *
 *	- The caller must remove the given timeout from the timer tree
 *	  in advance.
 */
static int
ptimer_expire(pfc_ptimer_t timer, ptimeout_t *PFC_RESTRICT top)
{
	uint32_t	flags;

	if (PFC_EXPECT_FALSE((top->pto_flags & PTMOF_ENABLED) == 0)) {
		/* This timeout was disabled by another thread. */
		return 0;
	}

	/* Remove this timeout from oneshot list. */
	if (top->pto_flags & PTMOF_ONESHOT) {
		pfc_list_remove(&top->pto_oneshot);
		top->pto_flags &= ~PTMOF_ONESHOT;
	}

	/* Invoke timeout function without holding the timer lock. */
	PTIMER_UNLOCK(timer);
	top->pto_func(top->pto_arg);
	PTIMER_LOCK(timer);
	if (PTIMER_IS_DESTROYED(timer)) {
		/* This timer is already destroyed. */
		return ESHUTDOWN;
	}

	/*
	 * Update next time to run, unless this timeout is registered or
	 * disabled by another thread.
	 */
	flags = (top->pto_flags & (PTMOF_REGISTERED | PTMOF_ENABLED));
	if (PFC_EXPECT_TRUE(flags == PTMOF_ENABLED)) {
		pfc_timespec_add(&top->pto_nextrun, &top->pto_interval);
		ptimer_register(timer, top);
	}

	return 0;
}

/*
 * static int
 * ptimer_oneshot(pfc_ptimer_t timer)
 *	Run oneshot timeout scheduled on the given periodic timer.
 *	This function is called on the timer thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ESHUTDOWN is returned if the given timer is destroyed.
 *
 * Remarks:
 *	This function must be called with holding the timer lock.
 *	Note that it may be released for a while.
 */
static int
ptimer_oneshot(pfc_ptimer_t timer)
{
	pfc_list_t	head = PFC_LIST_INITIALIZER(head), *elem;

	/*
	 * Create oneshot timeout list using pto_explist.
	 * We can not use pto_oneshot because it may be updated while
	 * the timer thread releases the timer lock.
	 */
	while ((elem = pfc_list_pop(&timer->pt_oneshot)) != NULL) {
		ptimeout_t	*top = PTIMEOUT_ONESHOT2PTR(elem);

		PFC_ASSERT(top->pto_flags & PTMOF_ONESHOT);
		pfc_list_push_tail(&head, &top->pto_explist);
		top->pto_flags &= ~PTMOF_ONESHOT;
	}

	PFC_LIST_FOREACH(&head, elem) {
		ptimeout_t	*top = PTIMEOUT_EXPLIST2PTR(elem);

		if (PFC_EXPECT_TRUE(top->pto_flags & PTMOF_ENABLED)) {
			/*
			 * Invoke timeout functions without holding the timer
			 * lock.
			 */
			PTIMER_UNLOCK(timer);
			top->pto_func(top->pto_arg);
			PTIMER_LOCK(timer);

			if (PTIMER_IS_DESTROYED(timer)) {
				/* This timer is already destroyed. */
				return ESHUTDOWN;
			}
		}
	}

	return 0;
}

/*
 * static void
 * ptimer_register(pfc_ptimer_t timer, ptimeout_t *PFC_RESTRICT top)
 *	Register timeout instance to the timer tree.
 *	This function wakes up the timer thread as appropriate.
 *
 * Remarks:
 *	- The caller must acquire the timer lock.
 *
 *	- The caller must ensure that the given timeout instance is enabled.
 *
 *	- The caller must update top->pto_nextrun in advance.
 */
static void
ptimer_register(pfc_ptimer_t timer, ptimeout_t *PFC_RESTRICT top)
{
	pfc_rbtree_t	*tmtree = &timer->pt_tmtree;
	pfc_rbnode_t	*node;
	ptimeout_t	*head;

	PFC_ASSERT(top->pto_flags & PTMOF_ENABLED);
	PFC_ASSERT((top->pto_flags & PTMOF_REGISTERED) == 0);

	/* Search for a timeout which will run at the same time. */
	node = pfc_rbtree_get(tmtree, (pfc_cptr_t)&top->pto_nextrun);
	if (PFC_EXPECT_FALSE(node != NULL)) {
		/* Link this timeout to another timeout on the timer tree. */
		head = PTIMEOUT_TM_NODE2PTR(node);
		PFC_ASSERT(head->pto_flags & PTMOF_TMTREE);
		top->pto_flags &= ~PTMOF_TMTREE;
		top->pto_flags |= PTMOF_SIBLING;
		pfc_list_push_tail(&head->pto_sibling, &top->pto_sibling);

		pfc_log_verbose("register (sibling): id=%u, nextrun=%lu.%lu",
				top->pto_id,
				top->pto_nextrun.tv_sec,
				top->pto_nextrun.tv_nsec);

		return;
	}

	/* Register this timeout to the timer tree. */
	top->pto_flags |= PTMOF_TMTREE;
	top->pto_flags &= ~PTMOF_SIBLING;
	pfc_list_init(&top->pto_sibling);
	PFC_ASSERT_INT(pfc_rbtree_put(tmtree, &top->pto_tmnode), 0);

	pfc_log_verbose("register (tree): id=%u, nextrun=%lu.%lu",
			top->pto_id,
			top->pto_nextrun.tv_sec,
			top->pto_nextrun.tv_nsec);

	/* Wake up the timer thread. */
	PTIMER_SIGNAL(timer);
}

/*
 * static void
 * ptimer_unregister(pfc_ptimer_t timer, ptimeout_t *PFC_RESTRICT top)
 *	Unregister timeout instance from the timer tree.
 *	This function wakes up the timer thread as appropriate.
 *
 * Remarks:
 *	- The caller must acquire the timer lock.
 */
static void
ptimer_unregister(pfc_ptimer_t timer, ptimeout_t *PFC_RESTRICT top)
{
	pfc_rbtree_t	*tmtree;

	if (PFC_EXPECT_FALSE(top->pto_flags & PTMOF_SIBLING)) {
		/* Remove this timeout from the sibling list. */
		PFC_ASSERT(!pfc_list_is_empty(&top->pto_sibling));
		PFC_ASSERT((top->pto_flags & PTMOF_TMTREE) == 0);
		pfc_list_remove(&top->pto_sibling);
		top->pto_flags &= ~PTMOF_SIBLING;

		pfc_log_verbose("unregister (sibling): id=%u",
				top->pto_id);

		return;
	}

	if ((top->pto_flags & PTMOF_TMTREE) == 0) {
		/* This timeout is not registered. */
		return;
	}

	/* This timeout is registered in the timer tree. */
	PFC_ASSERT((top->pto_flags & PTMOF_SIBLING) == 0);
	tmtree = &timer->pt_tmtree;
	pfc_rbtree_remove_node(tmtree, &top->pto_tmnode);
	top->pto_flags &= ~PTMOF_TMTREE;

	pfc_log_verbose("unregister (tree): id=%u", top->pto_id);

	if (pfc_list_is_empty(&top->pto_sibling)) {
		/* Wake up the timer thread. */
		PTIMER_SIGNAL(timer);
	}
	else {
		ptimeout_t	*sibp = PTIMEOUT_SIBLING_NEXT(top);

		/* This timeout is sibling head, so it needs to be replaced. */
		PFC_ASSERT(sibp->pto_flags & PTMOF_SIBLING);
		pfc_list_remove(&top->pto_sibling);
		sibp->pto_flags |= PTMOF_TMTREE;
		sibp->pto_flags &= ~PTMOF_SIBLING;
		PFC_ASSERT_INT(pfc_rbtree_put(tmtree, &sibp->pto_tmnode), 0);
		PFC_ASSERT(pfc_clock_compare(&sibp->pto_nextrun,
					     &top->pto_nextrun) == 0);

		pfc_log_verbose("promote: id=%u, nextrun=%lu.%lu",
				sibp->pto_id,
				sibp->pto_nextrun.tv_sec,
				sibp->pto_nextrun.tv_nsec);
	}
}

/*
 * static int
 * ptimer_get_nextrun(pfc_timespec_t *PFC_RESTRICT nextrun,
 *		      const pfc_timespec_t *PFC_RESTRICT ival)
 *	Calculate absolute system time to run timeout.
 *	`interval' must be a pointer to pfc_timespec which keeps timeout
 *	interval.
 *
 * Calling/Exit State:
 *	Upon successful completion, calculated system time is set to the buffer
 *	pointed by `nextrun', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ptimer_get_nextrun(pfc_timespec_t *PFC_RESTRICT nextrun,
		   const pfc_timespec_t *PFC_RESTRICT ival)
{
	int	err;

	err = ptimer_gettime(nextrun);
	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_timespec_add(nextrun, ival);
	}

	return err;
}

/*
 * static int
 * ptimer_gettime(pfc_timespec_t *tsp)
 *	Obtain current absolute system time.
 *	Note that this function obtains system time in monotonic clock.
 *
 * Calling/Exit State:
 *	Upon successful completion, system time is set to the buffer pointed
 *	by `tsp', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function calls pfc_log_fatal() on error.
 */
static int
ptimer_gettime(pfc_timespec_t *tsp)
{
	int	err;

	err = pfc_clock_gettime(tsp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		pfc_log_fatal("Failed to get system time: %s", strerror(err));
	}

	return err;
}

/*
 * static void
 * ptimeout_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
 *	Destructor of periodic timeout instance.
 */
static void
ptimeout_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
{
	ptimeout_t	*top = PTIMEOUT_ID_NODE2PTR(node);

	free(top);
}

/*
 * static ptimeout_t *
 * ptimeout_lookup(pfc_ptimer_t timer, pfc_ptimeout_t id)
 *	Return the timeout instance associated with the given periodic timer
 *	and timeout ID.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to timeout instance is returned if found.
 *	NULL is returned if not found.
 */
static ptimeout_t *
ptimeout_lookup(pfc_ptimer_t timer, pfc_ptimeout_t id)
{
	pfc_rbnode_t	*node;

	node = pfc_rbtree_get(&timer->pt_idtree, PTIMEOUT_IDKEY(id));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		return NULL;
	}

	return PTIMEOUT_ID_NODE2PTR(node);
}

/*
 * static pfc_cptr_t
 * ptimeout_tm_getkey(pfc_rbnode_t *node)
 *	Return the key of periodic timeout instance specified by `node'.
 *	`node' must be a pointer to pto_tmnode in ptimeout_t.
 */
static pfc_cptr_t
ptimeout_tm_getkey(pfc_rbnode_t *node)
{
	ptimeout_t	*top = PTIMEOUT_TM_NODE2PTR(node);

	return (pfc_cptr_t)&top->pto_nextrun;
}

/*
 * static pfc_cptr_t
 * ptimeout_id_getkey(pfc_rbnode_t *node)
 *	Return the key of periodic timeout instance specified by `node'.
 *	`node' must be a pointer to pto_idnode in ptimeout_t.
 */
static pfc_cptr_t
ptimeout_id_getkey(pfc_rbnode_t *node)
{
	ptimeout_t	*top = PTIMEOUT_ID_NODE2PTR(node);

	return PTIMEOUT_IDKEY(top->pto_id);
}
