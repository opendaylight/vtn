/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * taskq.c - asynchronous task scheduling.
 */

/*
 * Use "taskq" as log identifier.
 */
static const char	log_ident[] = "taskq";
#undef	PFC_LOG_IDENT
#define	PFC_LOG_IDENT	log_ident

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pfc/base.h>
#include <pfc/list.h>
#include <pfc/listmodel.h>
#include <pfc/rbtree.h>
#include <pfc/rbtree_ex.h>
#include <pfc/synch.h>
#include <pfc/thread.h>
#include <pfc/taskq.h>
#include <pfc/atomic.h>
#include <pfc/conf.h>
#include <pfc/log.h>
#include <pfc/refptr.h>
#include "taskq_impl.h"

/*
 * Task on a task queue.
 */
typedef struct task {
	pfc_list_t	t_list;			/* link for queued task */
	pfc_task_t	t_id;			/* task ID */
	uint16_t	t_state;		/* task state */
	uint16_t	t_flags;		/* task flags */
	pfc_taskfunc_t	t_func;			/* function */
	void		*t_arg;			/* argument */
	pfc_taskdtor_t	t_dtor;			/* destructor of argument */
	pfc_rbnode_t	t_node;			/* tree node */
} task_t;

#define	TASK_LIST2PTR(list)	PFC_CAST_CONTAINER((list), task_t, t_list)
#define	TASK_NODE2PTR(node)	PFC_CAST_CONTAINER((node), task_t, t_node)

/*
 * Task flags.
 */
#define	TASK_FLAG_DETACHED	0x0001
#define	TASK_FLAG_HASWAITER	0x0002
#define	TASK_FLAG_SYNC		0x0004

/*
 * Task status.
 */
#define	TASK_STATE_QUEUED	1
#define	TASK_STATE_INPROGRESS	2
#define	TASK_STATE_DONE		3

/*
 * Task queue instance.
 */
typedef struct taskq {
	pfc_taskq_t	tq_id;			/* taskq ID */
	uint32_t	tq_flags;		/* flags */
	pfc_rbtree_t	tq_tasktree;		/* task tree */
	pfc_rbnode_t	tq_node;		/* tree node */
	pfc_mutex_t	tq_mutex;		/* mutex for this taskq */
	pfc_refptr_t	*tq_poolname;		/* name of thread pool */
	uint32_t	tq_concurrency;		/* concurrency level */
	uint32_t	tq_maxfree;		/* max number of free threads */
	uint32_t	tq_nthreads;		/* number of threads */
	uint32_t	tq_nreserve;		/* number of reserve threads */
	uint32_t	tq_nfree;		/* number of free threads */
	uint32_t	tq_njoin;		/* number of join threads */
	pfc_cond_t	tq_dispcond;		/* cv for dispatching */
	pfc_cond_t	tq_joincond;		/* cv for joining */
	pfc_list_t	tq_tasklist;		/* link for queued task */
	uint32_t	tq_ntasks;		/* number of queued tasks */
	uint32_t	tq_taskid_next;		/* next task ID */
	size_t		tq_nnodes;		/* number of task nodes */
	pfc_list_t	tq_running;		/* link for running task */
	pfc_refptr_t	*tq_ownername;		/* name of owner (for debug) */
} taskq_t;

#define	TASKQ_NODE2PTR(node)	PFC_CAST_CONTAINER((node), taskq_t, tq_node)

/*
 * Task queue flags.
 */
#define	TASKQ_SHUTDOWN		0x0001

/*
 * Lock/unlock for taskq instance.
 */
#define	TASKQ_LOCK(tqp)		pfc_mutex_lock(&(tqp)->tq_mutex)
#define	TASKQ_UNLOCK(tqp)	pfc_mutex_unlock(&(tqp)->tq_mutex)

/*
 * Taskq ID for the next allocation.
 */
static pfc_taskq_t	taskq_id_next;

/*
 * Reserve thread.
 */
static pfc_thread_t	taskq_reserve_thread;

/*
 * Request to the reserve thread.
 */
static pfc_listm_t	taskq_reserve_req;

/*
 * Default maximum number of concurrency.
 */
#define	TASKQ_DEFAULT_MAX_CONCURRENCY	64U

/*
 * Default staying time for dispatcher thread.
 */
#define	TASKQ_DEFAULT_STAYTIME		500U	/* 500 milliseconds */

/*
 * Default maximum number of tasks in a task queue.
 * The value of zero means no limit.
 */
#define	TASKQ_DEFAULT_MAX_TASKS		0U

/*
 * Maximum number of concurrency.
 */
static uint32_t		taskq_max_concurrency;

/*
 * Staying time for dispatcher thread.
 */
static pfc_timespec_t	taskq_stay_time;

/*
 * Maximum number of tasks in a task queue.
 */
static uint32_t		taskq_max_tasks;

/*
 * Keys for configuration file.
 */
static const char	taskq_conf[] = "taskq";
static const char	taskq_conf_max_concurrency[] = "max_concurrency";
static const char	taskq_conf_stay_time[] = "stay_time";
static const char	taskq_conf_max_tasks[] = "max_tasks";

/*
 * Internal prototypes.
 */
static int	taskq_dispatch_impl(pfc_taskq_t tqid, task_t *PFC_RESTRICT tp,
				    pfc_task_t *PFC_RESTRICT tidp);
static int	taskq_register(taskq_t *tqp, task_t *tp, pfc_task_t *tidp);
static void	taskq_create_dispatcher(taskq_t *tqp);
static int	taskq_dispatch_sync(pfc_taskq_t tqid, pfc_task_t *tidp);
static int	taskq_wait_sync(pfc_taskq_t tqid, pfc_task_t tid,
				const pfc_timespec_t *PFC_RESTRICT timeout);
static void	*taskq_main(void *);
static void	*taskq_reserve_main(void *);
static void	taskq_unexpected_exit(void *);
static task_t	*taskq_prepare_calling(taskq_t *tqp);
static void	taskq_call(task_t *tp);
static void	taskq_dtor(pfc_taskdtor_t dtor, void *arg);
static void	taskq_finish_calling(taskq_t *tqp, task_t *tp);
static int	taskq_alloc(taskq_t **);
static void	taskq_free(taskq_t *);
static task_t	*taskq_task_alloc(pfc_taskfunc_t func, void *PFC_RESTRICT arg,
				  pfc_taskdtor_t dtor, uint32_t flags);

static pfc_cptr_t	taskq_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	taskq_task_getkey(pfc_rbnode_t *node);
static void		taskq_task_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

/*
 * Red-Black tree which keeps pairs of taskq ID and queue instance.
 */
static pfc_rbtree_ex_t	taskq_tree =
	PFC_RBTREE_EX_INITIALIZER(pfc_rbtree_uint32_compare, taskq_getkey);

/*
 * static inline pfc_taskq_t
 * pfc_taskq_new_id(void)
 *	Allocate new task queue ID.
 *
 * Remarks:
 *	We assume that this function never fails.
 *	We believe that it is impossible to create UINT32_MAX taskq
 *	at a time!
 */
static inline pfc_taskq_t
pfc_taskq_new_id(void)
{
	pfc_taskq_t	tqid;

	tqid = pfc_atomic_inc_uint32_old(&taskq_id_next) + 1;
	if (PFC_EXPECT_FALSE(tqid == PFC_TASKQ_INVALID_ID)) {
		tqid = pfc_atomic_inc_uint32_old(&taskq_id_next) + 1;
	}

	return tqid;
}

/*
 * static inline int
 * pfc_taskq_lookup(pfc_taskq_t tqid, taskq_t **tqpp)
 *	Get instance associated with the ID in taskq_tree.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to taskq_t is set to `*tqpp',
 *	and then zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding global taskq ID lock in
 *	reader or writer mode.
 */
static inline int
pfc_taskq_lookup(pfc_taskq_t tqid, taskq_t **tqpp)
{
	taskq_t		*tqp;
	pfc_rbnode_t	*node;
	pfc_cptr_t	tqkey;

	tqkey = (pfc_cptr_t)(uintptr_t)tqid;
	node = pfc_rbtree_ex_get_l(&taskq_tree, tqkey);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		return ENOENT;
	}

	tqp = TASKQ_NODE2PTR(node);
	if (PFC_EXPECT_FALSE(tqp->tq_flags & TASKQ_SHUTDOWN)) {
		return ESHUTDOWN;
	}

	*tqpp = tqp;
	return 0;
}

/*
 * static inline void
 * pfc_taskq_task_free(task_t *tp)
 *	Free the specified task instance.
 */
static inline void
pfc_taskq_task_free(task_t *tp)
{
	free(tp);
}

/*
 * static inline int
 * pfc_taskq_task_lookup(taskq_t *tqp, pfc_task_t tid, task_t **tpp)
 *	Get task instance associated with the ID in tq_tasktree.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to task_t is set to `*tpp',
 *	and then zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding taskq lock.
 */
static inline int
pfc_taskq_task_lookup(taskq_t *tqp, pfc_task_t tid, task_t **tpp)
{
	task_t		*tp;
	pfc_rbnode_t	*node;
	pfc_cptr_t	idkey;

	idkey = (pfc_cptr_t)(uintptr_t)tid;
	node = pfc_rbtree_get(&tqp->tq_tasktree, idkey);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		return ESRCH;
	}

	tp = TASK_NODE2PTR(node);
	if (PFC_EXPECT_FALSE(tp->t_flags & TASK_FLAG_HASWAITER)) {
		/* Another thread is waiting for this task. */
		return EINVAL;
	}

	*tpp = tp;

	return 0;
}

/*
 * static inline void
 * pfc_taskq_task_remove(taskq_t *PFC_RESTRICT tqp, task_t *PFC_RESTRICT tp)
 *	Remove task instance specified by `tp' from the task tree in `tqp',
 *	and then destroy `tp'.
 *
 * Remarks:
 *	This function must be called with holding taskq lock.
 */
static inline void
pfc_taskq_task_remove(taskq_t *PFC_RESTRICT tqp, task_t *PFC_RESTRICT tp)
{
	pfc_rbtree_remove_node(&tqp->tq_tasktree, &tp->t_node);
	tqp->tq_nnodes--;
	pfc_taskq_task_free(tp);
}

/*
 * static inline void
 * pfc_taskq_task_unlink(taskq_t *PFC_RESTRICT tqp, task_t *PFC_RESTRICT tp)
 *	Unlink task instance specified by `tp' from the task tree in `tqp'.
 *
 * Remarks:
 *	This function must be called with holding taskq lock.
 */
static inline void
pfc_taskq_task_unlink(taskq_t *PFC_RESTRICT tqp, task_t *PFC_RESTRICT tp)
{
	pfc_rbtree_remove_node(&tqp->tq_tasktree, &tp->t_node);
	tqp->tq_nnodes--;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_taskq_init(void)
 *	Initialize task queue subsystem.
 */
void PFC_ATTR_HIDDEN
pfc_taskq_init(void)
{
	int		err;
	pfc_cfblk_t	cf;
	uint32_t	staytime;

	/* Initialize tunable parameters */
	cf = pfc_sysconf_get_block(taskq_conf);
	taskq_max_concurrency = pfc_conf_get_uint32(cf,
					taskq_conf_max_concurrency,
					TASKQ_DEFAULT_MAX_CONCURRENCY);
	staytime = pfc_conf_get_uint32(cf, taskq_conf_stay_time,
				       TASKQ_DEFAULT_STAYTIME);
	taskq_stay_time.tv_sec = staytime / PFC_CLOCK_MILLISEC;
	taskq_stay_time.tv_nsec = (staytime % PFC_CLOCK_MILLISEC) *
				  (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);
	taskq_max_tasks = pfc_conf_get_uint32(cf, taskq_conf_max_tasks,
					      TASKQ_DEFAULT_MAX_TASKS);

	/* Create reserve thread */
	err = pfc_llist_create(&taskq_reserve_req);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("failed to create llist for reserve: %s",
			      strerror(err));
		/* NOTREACHED */
	}
	err = pfc_thread_create(&taskq_reserve_thread, taskq_reserve_main,
				NULL, PFC_THREAD_DETACHED);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("failed to create reserve thread: %s",
			      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_taskq_fini(void)
 *	Shutdown task queue subsystem.
 */
void PFC_ATTR_HIDDEN
pfc_taskq_fini(void)
{
	size_t	sz;

	/* Stop reserve thread */
	pfc_llist_shutdown(taskq_reserve_req, PFC_TRUE);

	/* Ensure that no task queue remains. */
	sz = pfc_rbtree_ex_get_size(&taskq_tree);
	if (sz != 0) {
		pfc_rbnode_t	*n = NULL;
		taskq_t		*tqp;
		while ((n = pfc_rbtree_ex_next_l(&taskq_tree, n)) != NULL) {
			tqp = TASKQ_NODE2PTR(n);
			TASKQ_LOCK(tqp);
			pfc_log_warn("taskq[%d] still remain : %s", tqp->tq_id,
				    pfc_refptr_string_value(tqp->tq_ownername));
			TASKQ_UNLOCK(tqp);
		}
	}
}

/*
 * int
 * pfc_taskq_create_named(pfc_taskq_t *PFC_RESTRICT tqidp,
 *			const char *PFC_RESTRICT poolname, uint32_t concurrency,
 *			const char *PFC_RESTRICT owner)
 *	Create a new task queue.
 *
 * Calling/Exit State:
 *	Upon successful completion, task queue instance is set to *tqidp
 *	and zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_taskq_create_named(pfc_taskq_t *PFC_RESTRICT tqidp,
		       const char *PFC_RESTRICT poolname, uint32_t concurrency,
		       const char *PFC_RESTRICT owner)
{
	pfc_taskq_t	tqid;
	taskq_t		*tqp = NULL;
	int		err;
	pfc_refptr_t	*rname, *oname;

	if (concurrency < 1 || concurrency > taskq_max_concurrency) {
		return EINVAL;
	}
	if (poolname != NULL) {
		rname = pfc_refptr_string_create(poolname);
		if (PFC_EXPECT_FALSE(rname == NULL)) {
			return ENOMEM;
		}
	} else {
		rname = NULL;
	}
	if (owner != NULL) {
		oname = pfc_refptr_string_create(owner);
	} else {
		oname = pfc_refptr_string_create("-");
	}
	PFC_ASSERT(oname != NULL);

	err = taskq_alloc(&tqp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (rname != NULL) {
			pfc_refptr_put(rname);
		}
		pfc_refptr_put(oname);
		return err;
	}
	tqp->tq_concurrency = concurrency;
	tqp->tq_poolname = rname;
	tqp->tq_ownername = oname;

	while (1) {
		/* Allocate a new taskq ID. */
		tqid = pfc_taskq_new_id();
		tqp->tq_id = tqid;

		/* Register task queue into the taskq_tree. */
		err = pfc_rbtree_ex_put(&taskq_tree, &tqp->tq_node);
		if (PFC_EXPECT_TRUE(err == 0)) {
			break;
		}
		PFC_ASSERT(err == EEXIST);

		/*
		 * Assigned ID is not available.
		 * We must assign another ID.
		 */
	}

	*tqidp = tqid;
	return 0;
}

/*
 * void
 * pfc_taskq_destroy(pfc_taskq_t tqid)
 *	Destroy the task queue.
 *	This function wait for dispatcher and joiner threads to be terminated.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_taskq_destroy(pfc_taskq_t tqid)
{
	taskq_t		*tqp;
	pfc_rbnode_t	*node;
	pfc_cptr_t	tqkey = (pfc_cptr_t)(uintptr_t)tqid;

	/* Make the specified queue invisible. */
	pfc_rbtree_ex_wrlock(&taskq_tree);
	node = pfc_rbtree_ex_remove_l(&taskq_tree, tqkey);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		pfc_rbtree_ex_unlock(&taskq_tree);

		return ENOENT;
	}

	tqp = TASKQ_NODE2PTR(node);
	TASKQ_LOCK(tqp);
	tqp->tq_flags |= TASKQ_SHUTDOWN;

	/* Wake up all threads */
	PFC_ASSERT_INT(pfc_cond_broadcast(&tqp->tq_dispcond), 0);
	PFC_ASSERT_INT(pfc_cond_broadcast(&tqp->tq_joincond), 0);

	pfc_rbtree_ex_unlock(&taskq_tree);

	/* Wait for dispatcher and joiner threads to be exited. */
	while (tqp->tq_nthreads != 0 || tqp->tq_njoin != 0 ||
	       tqp->tq_nreserve != 0) {
		PFC_ASSERT_INT(pfc_cond_wait(&tqp->tq_joincond, &tqp->tq_mutex),
			       0);
	}
	TASKQ_UNLOCK(tqp);

	taskq_free(tqp);

	return 0;
}

/*
 * int
 * pfc_taskq_set_free(pfc_taskq_t tqid, uint32_t maxfree)
 *	Set maxfree attribute of the given taskq.
 *	Default value of maxfree is zero. By default,
 *	dispatcher threads will soon terminate, if taskq has no task.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_taskq_set_free(pfc_taskq_t tqid, uint32_t maxfree)
{
	taskq_t		*tqp;
	int		err;

	if (maxfree > taskq_max_concurrency) {
		return EINVAL;
	}

	pfc_rbtree_ex_rdlock(&taskq_tree);
	err = pfc_taskq_lookup(tqid, &tqp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		TASKQ_LOCK(tqp);
		tqp->tq_maxfree = maxfree;
		TASKQ_UNLOCK(tqp);
	}
	pfc_rbtree_ex_unlock(&taskq_tree);

	return err;
}

/*
 * int
 * pfc_taskq_dispatch(pfc_taskq_t tqid, pfc_taskfunc_t func,
 *		      void *PFC_RESTRICT arg, uint32_t flags,
 *		      pfc_task_t *PFC_RESTRICT tidp)
 *	Dispatch the task to the task queue.
 *
 * Calling/Exit State:
 *	Upon successful completion, task instance is set to *tidp
 *	and zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_taskq_dispatch(pfc_taskq_t tqid, pfc_taskfunc_t func,
		   void *PFC_RESTRICT arg, uint32_t flags,
		   pfc_task_t *PFC_RESTRICT tidp)
{
	task_t	*tp;

	tp = taskq_task_alloc(func, arg, NULL, flags);
	if (PFC_EXPECT_FALSE(tp == NULL)) {
		return ENOMEM;
	}

	return taskq_dispatch_impl(tqid, tp, tidp);
}

/*
 * int
 * pfc_taskq_dispatch_dtor(pfc_taskq_t tqid, pfc_taskfunc_t func,
 * 			   void *PFC_RESTRICT arg, pfc_taskdtor_t dtor,
 * 			   uint32_t flags, pfc_task_t *PFC_RESTRICT tidp)
 *	Dispatch the task to the task queue.
 *
 *	'dtor' is a destructor of 'arg'. If the task is destroyed after
 *	dispatched/joined/canceled, 'dtor' is called with specifying arg.
 *
 * Calling/Exit State:
 *	Upon successful completion, task instance is set to *tidp
 *	and zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_taskq_dispatch_dtor(pfc_taskq_t tqid, pfc_taskfunc_t func,
			void *PFC_RESTRICT arg, pfc_taskdtor_t dtor,
			uint32_t flags, pfc_task_t *PFC_RESTRICT tidp)
{
	task_t	*tp;

	tp = taskq_task_alloc(func, arg, dtor, flags);
	if (PFC_EXPECT_FALSE(tp == NULL)) {
		return ENOMEM;
	}

	return taskq_dispatch_impl(tqid, tp, tidp);
}

/*
 * static int
 * taskq_dispatch_impl(pfc_taskq_t tqid, task_t *PFC_RESTRICT tp,
 *		       pfc_task_t *PFC_RESTRICT tidp)
 *	Internal implementation for dispatching the task to the task queue.
 */
static int
taskq_dispatch_impl(pfc_taskq_t tqid, task_t *PFC_RESTRICT tp,
		    pfc_task_t *PFC_RESTRICT tidp)
{
	taskq_t	*tqp;
	int	err;

	pfc_rbtree_ex_rdlock(&taskq_tree);
	err = pfc_taskq_lookup(tqid, &tqp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		TASKQ_LOCK(tqp);
		err = taskq_register(tqp, tp, tidp);
		TASKQ_UNLOCK(tqp);
	}
	pfc_rbtree_ex_unlock(&taskq_tree);

	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_taskq_task_free(tp);
	}

	return err;
}

/*
 * static int
 * taskq_register(taskq_t *tqp, task_t *tp, pfc_task_t *tidp)
 *	Register task instance.
 *	Task ID is assigned for the task instance and the task is enqueued.
 *	This function notify to the dispatcher threads waiting task.
 *	If the taskq has no idle threads, create a new thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, assigned task ID is set to *tidp
 *	and zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding taskq lock.
 */
static int
taskq_register(taskq_t *tqp, task_t *tp, pfc_task_t *tidp)
{
	int	err;

	if (taskq_max_tasks != 0 && tqp->tq_ntasks >= taskq_max_tasks) {
		return EAGAIN;
	}

	/* Assign task ID */
	while (1) {
		tp->t_id = tqp->tq_taskid_next++;
		if (tqp->tq_taskid_next == PFC_TASKQ_INVALID_TASKID) {
			tqp->tq_taskid_next++;
		}

		err = pfc_rbtree_put(&tqp->tq_tasktree, &tp->t_node);
		if (PFC_EXPECT_TRUE(err == 0)) {
			tqp->tq_nnodes++;
			break;
		}

		/*
		 * Assigned ID is not available.
		 * We must assign another ID.
		 */
		PFC_ASSERT(err == EEXIST);
	}

	/* Enqueue task */
	pfc_list_push_tail(&tqp->tq_tasklist, &tp->t_list);
	tqp->tq_ntasks++;

	/* Notify to dispatcher thread */
	if (tqp->tq_nfree != 0) {
		err = pfc_cond_signal(&tqp->tq_dispcond);
		PFC_ASSERT(err == 0);
	} else {
		taskq_create_dispatcher(tqp);
	}

	*tidp = tp->t_id;
	return 0;
}

/*
 * static void
 * taskq_create_dispatcher(taskq_t *tqp)
 *	Create a dispatcher thread.
 */
static void
taskq_create_dispatcher(taskq_t *tqp)
{
	pfc_thread_t	thread;
	int		err;
	pfc_cptr_t	value;
	const char	*poolname;

	if (tqp->tq_nthreads >= tqp->tq_concurrency) {
		return;
	}

	if (tqp->tq_poolname == NULL) {
		poolname = NULL;
	} else {
		poolname = pfc_refptr_string_value(tqp->tq_poolname);
	}
	err = pfc_thread_createat(&thread, poolname, taskq_main,
				  (void *)tqp, PFC_THREAD_DETACHED);
	if (PFC_EXPECT_TRUE(err == 0)) {
		tqp->tq_nthreads++;
	} else {
		PFC_ASSERT(err == EAGAIN);
		if (tqp->tq_nthreads == 0) {
			/*
			 * Dispatch to the reserve thread.
			 */
			value = (pfc_cptr_t)(uintptr_t)tqp->tq_id;
			err = pfc_listm_push_tail(taskq_reserve_req, value);
			if (PFC_EXPECT_FALSE(err != 0)) {
				pfc_log_fatal("failed to dispatch to "
					      "the reserve thread: %s",
					      strerror(err));
			}
			PFC_ASSERT(err == 0);
		}
	}

	return;
}

/*
 * int
 * pfc_taskq_cancel(pfc_taskq_t tqid, pfc_task_t tid)
 *	Cancel the specified task on the task queue.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	If the specified task is running, this function returns EBUSY.
 *	This function does not stop the task.
 */
int
pfc_taskq_cancel(pfc_taskq_t tqid, pfc_task_t tid)
{
	taskq_t *tqp;
	task_t *tp;
	int err;
	pfc_taskdtor_t dtor = NULL;
	void *argp = NULL;

	pfc_rbtree_ex_rdlock(&taskq_tree);

	/* Obtain taskq instance associated with the given ID. */
	err = pfc_taskq_lookup(tqid, &tqp);

	if (PFC_EXPECT_TRUE(err == 0)) {
		TASKQ_LOCK(tqp);
		err = pfc_taskq_task_lookup(tqp, tid, &tp);
		if (PFC_EXPECT_TRUE(err == 0)) {
			switch (tp->t_state) {
			case TASK_STATE_INPROGRESS:
				err = EBUSY;
				break;
			case TASK_STATE_DONE:
				PFC_ASSERT(!(tp->t_flags & TASK_FLAG_DETACHED));
				dtor = tp->t_dtor;
				argp = tp->t_arg;
				pfc_taskq_task_remove(tqp, tp);
				err = ESRCH;
				break;
			case TASK_STATE_QUEUED:
				dtor = tp->t_dtor;
				argp = tp->t_arg;
				pfc_list_remove(&tp->t_list);
				tqp->tq_ntasks--;
				pfc_taskq_task_remove(tqp, tp);
				break;
			default:
				PFC_ASSERT(0);
				break;
			}
		}
		TASKQ_UNLOCK(tqp);
	}

	pfc_rbtree_ex_unlock(&taskq_tree);

	/* Call task destructor. */
	taskq_dtor(dtor, argp);

	return err;
}

/*
 * int
 * pfc_taskq_join(pfc_taskq_t tqid, pfc_task_t tid)
 *	Block current thread until the specified task quits.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_taskq_join(pfc_taskq_t tqid, pfc_task_t tid)
{
	return pfc_taskq_timedjoin(tqid, tid, NULL);
}

/*
 * int
 * pfc_taskq_timedjoin(pfc_taskq_t tqid, pfc_task_t tid,
 *		       const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Block current thread until the specified task quits within
 *	the specified timeout. NULL timeout means no timeout is specified.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise, error number which indicates the cause of error is returned.
 */
int
pfc_taskq_timedjoin(pfc_taskq_t tqid, pfc_task_t tid,
		    const pfc_timespec_t *PFC_RESTRICT timeout)
{
	taskq_t *tqp;
	task_t *tp;
	int err;
	pfc_taskdtor_t dtor = NULL;
	void *argp = NULL;

	pfc_rbtree_ex_rdlock(&taskq_tree);

	/* Obtain taskq instance associated with the give ID. */
	err = pfc_taskq_lookup(tqid, &tqp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_rbtree_ex_unlock(&taskq_tree);

		return err;
	}

	TASKQ_LOCK(tqp);
	err = pfc_taskq_task_lookup(tqp, tid, &tp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		TASKQ_UNLOCK(tqp);
		pfc_rbtree_ex_unlock(&taskq_tree);

		return err;
	}

	pfc_rbtree_ex_unlock(&taskq_tree);

	if (tp->t_flags & TASK_FLAG_DETACHED) {
		/* Detached task cannot be joined. */
		TASKQ_UNLOCK(tqp);
		return EINVAL;
	}

	while (tp->t_state != TASK_STATE_DONE) {
		tp->t_flags |= TASK_FLAG_HASWAITER;
		tqp->tq_njoin++;
		if (timeout == NULL) {
			err = pfc_cond_wait(&tqp->tq_joincond, &tqp->tq_mutex);
		} else {
			err = pfc_cond_timedwait(&tqp->tq_joincond,
						 &tqp->tq_mutex, timeout);
		}
		tp->t_flags &= ~TASK_FLAG_HASWAITER;
		tqp->tq_njoin--;
		if (PFC_EXPECT_FALSE(err != 0)) {
			TASKQ_UNLOCK(tqp);
			return err;
		}
		if (tqp->tq_flags & TASKQ_SHUTDOWN) {
			if (tqp->tq_njoin == 0) {
				/* Wakeup destroyer. */
				err = pfc_cond_broadcast(&tqp->tq_joincond);
				PFC_ASSERT(err == 0);
			}
			/*
			 * In this case, we must not free task,
			 * even if tp->t_state is TASK_STATE_DONE.
			 * Destroyer of this taskq will free it.
			 */
			TASKQ_UNLOCK(tqp);
			return ESHUTDOWN;
		}
	}
	dtor = tp->t_dtor;
	argp = tp->t_arg;
	pfc_taskq_task_remove(tqp, tp);
	TASKQ_UNLOCK(tqp);

	/* Call task destructor. */
	taskq_dtor(dtor, argp);

	return 0;
}

/*
 * static int
 * taskq_dispatch_sync(pfc_taskq_t tqid, pfc_task_t *tidp)
 *	Dispatch the sync task to the task queue.
 */
static int
taskq_dispatch_sync(pfc_taskq_t tqid, pfc_task_t *tidp)
{
	task_t	*tp;

	tp = taskq_task_alloc(NULL, NULL, NULL, PFC_TASKQ_JOINABLE);
	if (PFC_EXPECT_FALSE(tp == NULL)) {
		return ENOMEM;
	}
	tp->t_flags |= TASK_FLAG_SYNC;
	return taskq_dispatch_impl(tqid, tp, tidp);
}

/*
 * static int
 * taskq_wait_sync(pfc_taskq_t tqid, pfc_task_t tid,
 *		   const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Block current thread until the sync task quits.
 */
static int
taskq_wait_sync(pfc_taskq_t tqid, pfc_task_t tid,
		const pfc_timespec_t *PFC_RESTRICT timeout)
{
	int	err;

	err = pfc_taskq_timedjoin(tqid, tid, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ESRCH) {
			/*
			 * This sync task has finished before calling
			 * join operation.
			 */
			err = 0;
		}
		return err;
	}
	return 0;
}

/*
 * int
 * pfc_taskq_flush(pfc_taskq_t tqid, const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Flush the specified task queue.
 *
 *	If this function returns zero, it is guaranteed that all tasks
 *	dispatched to the specified taskq before the call of pfc_taskq_flush()
 *	have been executed.
 *
 *	If `timeout' is NULL, pfc_taskq_flush() waits until all tasks are
 *	flushed. If not NULL, pfc_taskq_flush() only waits the specified
 *	timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_taskq_flush(pfc_taskq_t tqid, const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_task_t	tid;
	int		err;

	err = taskq_dispatch_sync(tqid, &tid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	return taskq_wait_sync(tqid, tid, timeout);
}

/*
 * int
 * pfc_taskq_clear(pfc_taskq_t tqid,
 *		   const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Clear the specified task queue.
 *
 *	If this function returns zero, it is guaranteed that all tasks
 *	dispatched to the specified taskq before the call of
 *	pfc_taskq_clear() have been canceled or executed.
 *	If a task is running or queuing with joiner, pfc_taskq_clear()
 *	waits for its execution. Otherwise, a task is canceled.
 *
 *	If `timeout' is NULL, pfc_taskq_clear() waits until all running
 *	tasks are executed. If not NULL, pfc_taskq_clear() only waits
 *	the specified timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_taskq_clear(pfc_taskq_t tqid,
		     const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_task_t	tid;
	task_t		*tp;
	int		err;
	taskq_t		*tqp;
	pfc_list_t	removed = PFC_LIST_INITIALIZER(removed);

	/* Dispatch a sync task. */
	err = taskq_dispatch_sync(tqid, &tid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Clear all queued tasks */
	pfc_rbtree_ex_rdlock(&taskq_tree);
	err = pfc_taskq_lookup(tqid, &tqp);
	if (PFC_EXPECT_FALSE(err == 0)) {
		pfc_list_t	*elem, *next;

		TASKQ_LOCK(tqp);

		PFC_LIST_FOREACH_SAFE(&tqp->tq_tasklist, elem, next) {
			task_t	*tmp = TASK_LIST2PTR(elem);

			if (tmp->t_id == tid) {
				/* Found the sync task. */
				break;
			}
			if (tmp->t_flags &
			    (TASK_FLAG_SYNC|TASK_FLAG_HASWAITER)) {
				/* Skip the other sync task. */
				/* Skip the task having joiner. */
				continue;
			}

			PFC_ASSERT(tmp->t_state == TASK_STATE_QUEUED);
			pfc_list_remove(&tmp->t_list);
			tqp->tq_ntasks--;
			pfc_taskq_task_unlink(tqp, tmp);
			pfc_list_push_tail(&removed, &tmp->t_list);
		}

		TASKQ_UNLOCK(tqp);
	}
	pfc_rbtree_ex_unlock(&taskq_tree);

	while ((tp = TASK_LIST2PTR(pfc_list_pop(&removed))) != NULL) {
		taskq_dtor(tp->t_dtor, tp->t_arg);
		pfc_taskq_task_free(tp);
	}

	/* Wait for all running tasks. */
	return taskq_wait_sync(tqid, tid, timeout);
}

/*
 * static void *
 * taskq_main(void *arg)
 *	Main routine for a dispatcher thread.
 */
static void *
taskq_main(void *arg)
{
	taskq_t		*tqp = (taskq_t *)arg;
	task_t		*tp;
	int		err;
	uint32_t	nactive;

	pthread_cleanup_push(taskq_unexpected_exit, arg);

	TASKQ_LOCK(tqp);

	while (1) {
		if (tqp->tq_flags & TASKQ_SHUTDOWN) {
			break;
		}
		nactive = tqp->tq_nthreads + tqp->tq_nreserve - tqp->tq_nfree;
		if (tqp->tq_ntasks == 0 || nactive > tqp->tq_concurrency) {
			tqp->tq_nfree++;
			if (tqp->tq_nfree <= tqp->tq_maxfree) {
				err = pfc_cond_wait(&tqp->tq_dispcond,
						    &tqp->tq_mutex);
			} else {
				/*
				 * An idle dispatcher thread stays here
				 * for a short time, before exiting
				 * this routine.
				 */
				err = pfc_cond_timedwait(&tqp->tq_dispcond,
							 &tqp->tq_mutex,
							 &taskq_stay_time);
			}
			tqp->tq_nfree--;
			if (err == ETIMEDOUT && tqp->tq_ntasks == 0) {
				break;
			}
			PFC_ASSERT(err == 0 || err == ETIMEDOUT);
			continue;
		}

		tp = taskq_prepare_calling(tqp);
		if (tp == NULL) {
			continue;
		}

		if (tqp->tq_ntasks != 0) {
			if (tqp->tq_nfree != 0) {
				err = pfc_cond_signal(&tqp->tq_dispcond);
				PFC_ASSERT(err == 0);
			} else {
				taskq_create_dispatcher(tqp);
			}
		}
		TASKQ_UNLOCK(tqp);

		/* Call task function */
		taskq_call(tp);

		TASKQ_LOCK(tqp);
		taskq_finish_calling(tqp, tp);
	}

	tqp->tq_nthreads--;

	if ((tqp->tq_flags & TASKQ_SHUTDOWN) && (tqp->tq_nthreads == 0)) {
		/* Wakeup destroyer. */
		PFC_ASSERT_INT(pfc_cond_broadcast(&tqp->tq_joincond), 0);
	}

	TASKQ_UNLOCK(tqp);

	pthread_cleanup_pop(0);

	return NULL;
}

/*
 * static void *
 * taskq_reserve_main(void *arg)
 *	Main routine for the reserve thread.
 */
static void *
taskq_reserve_main(void *arg)
{
	int		err;
	pfc_cptr_t	value;
	pfc_taskq_t	tqid;
	taskq_t		*tqp = NULL;
	task_t		*tp;

	while (1) {
		/* Get request to me. */
		err = pfc_llist_pop_wait(taskq_reserve_req, &value);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_VERIFY(err == ESHUTDOWN);
			break;
		}
		tqid = (pfc_taskq_t)(uintptr_t)value;

		/* Get required taskq */
		pfc_rbtree_ex_rdlock(&taskq_tree);
		err = pfc_taskq_lookup(tqid, &tqp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_rbtree_ex_unlock(&taskq_tree);
			continue;
		}

		TASKQ_LOCK(tqp);
		if (tqp->tq_ntasks == 0 || tqp->tq_nthreads != 0) {
			TASKQ_UNLOCK(tqp);
			pfc_rbtree_ex_unlock(&taskq_tree);
			continue;
		}
		pfc_rbtree_ex_unlock(&taskq_tree);

		/*
		 * The reserve thread starts working on the taskq.
		 */
		tp = taskq_prepare_calling(tqp);
		if (tp == NULL) {
			TASKQ_UNLOCK(tqp);
			continue;
		}

		pthread_cleanup_push(taskq_unexpected_exit, tqp);
		tqp->tq_nreserve++;
		TASKQ_UNLOCK(tqp);

		taskq_call(tp);

		TASKQ_LOCK(tqp);
		taskq_finish_calling(tqp, tp);

		/* Wakeup an idle dispatcher thread if exists. */
		if (tqp->tq_ntasks != 0 && tqp->tq_nfree != 0) {
			PFC_ASSERT_INT(pfc_cond_signal(&tqp->tq_dispcond), 0);
		}
		/* Wakeup destroyer if waiting. */
		if (tqp->tq_flags & TASKQ_SHUTDOWN) {
			PFC_ASSERT_INT(pfc_cond_broadcast(&tqp->tq_joincond),0);
		}

		tqp->tq_nreserve--;
		pthread_cleanup_pop(0);

		TASKQ_UNLOCK(tqp);
	}
	return NULL;
}

/*
 * static void
 * taskq_unexpected_exit(void *arg)
 *	Pthread cleanup handler for dispatcher threads.
 */
static void
taskq_unexpected_exit(void *arg)
{
	taskq_t		*tqp = (taskq_t *)arg;
	pfc_log_fatal("dispatcher thread for %d terminated abnormally.",
		      tqp->tq_id);
}

/*
 * static task_t *
 * taskq_prepare_calling(taskq_t *tqp)
 *	Prepare a task calling.
 *
 *	Return a task on the head of task queue.
 *	If sync task is found, process it and get next task on the queue.
 *	NULL is returned if no more task exists.
 */
static task_t *
taskq_prepare_calling(taskq_t *tqp)
{
	task_t	*tp;

	while (tqp->tq_ntasks != 0) {
		tp = TASK_LIST2PTR(pfc_list_pop(&tqp->tq_tasklist));
		tqp->tq_ntasks--;
		if (!(tp->t_flags & TASK_FLAG_SYNC)) {
			tp->t_state = TASK_STATE_INPROGRESS;
			pfc_list_push_tail(&tqp->tq_running, &tp->t_list);
			return tp;
		}

		/* Found a sync task */
		if (!pfc_list_is_empty(&tqp->tq_running)) {
			tp->t_state = TASK_STATE_INPROGRESS;
			pfc_list_push_tail(&tqp->tq_running, &tp->t_list);
			continue;
		}
		/*
		 * Notify to the caller of flush operation.
		 * Currently there is no running task,
		 * so we can say all tasks dispatched before sync task
		 * are finished.
		 */
		tp->t_state = TASK_STATE_DONE;
		if (!(tp->t_flags & TASK_FLAG_HASWAITER)) {
			/*
			 * In this condition, no one wait for
			 * this sync task because of timeout.
			 * So, we must remove this sync task instance.
			 */
			pfc_taskq_task_remove(tqp, tp);
		} else {
			PFC_ASSERT_INT(pfc_cond_broadcast(&tqp->tq_joincond),0);
		}
	}

	return NULL;
}

/*
 * static void
 * taskq_call(task_t *tp)
 *	Call the task function and destructor.
 */
static void
taskq_call(task_t *tp)
{
	tp->t_func(tp->t_arg);

	if (tp->t_flags & TASK_FLAG_DETACHED) {
		taskq_dtor(tp->t_dtor, tp->t_arg);
	}
}

/*
 * static void
 * taskq_dtor(pfc_taskdtor_t dtor, void *arg)
 *	Call the task destructor.
 */
static void
taskq_dtor(pfc_taskdtor_t dtor, void *arg)
{
	if (dtor != NULL && arg != NULL) {
		dtor(arg);
	}
}

/*
 * static void
 * taskq_finish_calling(taskq_t *tqp, task_t *tp)
 *	Finish the task calling.
 *	Wakeup joiner thread and remove the detached task instance.
 *
 *	Remove the task from running list. If the head of running list
 *	is sync task, it means that all tasks dispatched before the sync task
 *	are finished. In that case, notify the caller of flush operation.
 */
static void
taskq_finish_calling(taskq_t *tqp, task_t *tp)
{
	pfc_bool_t	syncdone = PFC_FALSE;
	pfc_list_t	*elem, *next;

	tp->t_state = TASK_STATE_DONE;
	pfc_list_remove(&tp->t_list);

	/* Check whether a sync task is queued on the head of tq_running */
	PFC_LIST_FOREACH_SAFE(&tqp->tq_running, elem, next) {
		task_t	*tmp = TASK_LIST2PTR(elem);

		if (tmp->t_flags & TASK_FLAG_SYNC) {
			tmp->t_state = TASK_STATE_DONE;
			pfc_list_remove(&tmp->t_list);
			if (!(tmp->t_flags & TASK_FLAG_HASWAITER)) {
				/*
				 * In this condition, no one wait for
				 * this sync task because of timeout.
				 * So, we must remove this sync task instance.
				 */
				pfc_taskq_task_remove(tqp, tmp);
			}
			syncdone = PFC_TRUE;
		} else {
			break;
		}
	}

	/* Wakeup joiner if waiting. */
	if (syncdone || (tp->t_flags & TASK_FLAG_HASWAITER)) {
		PFC_ASSERT_INT(pfc_cond_broadcast(&tqp->tq_joincond), 0);
	}
	/* Remove the task instance if detached. */
	if (tp->t_flags & TASK_FLAG_DETACHED) {
		/*
		 * Note that the t_dtor must be called without any locks.
		 * So, t_dtor is called by taskq_call().
		 */
		pfc_taskq_task_remove(tqp, tp);
	}
}

/*
 * static int
 * taskq_alloc(taskq_t **tqpp)
 *	Allocate and initialize new task queue instance.
 */
static int
taskq_alloc(taskq_t **tqpp)
{
	taskq_t *tqp;
	int err;

	/* Allocate a new task queue instance. */
	tqp = (taskq_t *)malloc(sizeof(taskq_t));
	if (PFC_EXPECT_FALSE(tqp == NULL)) {
		return ENOMEM;
	}

	/* Initialize mutex */
	err = PFC_MUTEX_INIT(&tqp->tq_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_alloc;
	}

	/* Initialize cond */
	err = pfc_cond_init(&tqp->tq_dispcond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_mutex;
	}
	err = pfc_cond_init(&tqp->tq_joincond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_cond_destroy(&tqp->tq_dispcond);
		goto error_mutex;
	}

	pfc_rbtree_init(&tqp->tq_tasktree, pfc_rbtree_uint32_compare,
			taskq_task_getkey);

	tqp->tq_flags = 0;
	tqp->tq_nthreads = 0;
	tqp->tq_nreserve = 0;
	tqp->tq_nfree = 0;
	tqp->tq_njoin = 0;
	tqp->tq_maxfree = 0;

	pfc_list_init(&tqp->tq_tasklist);
	pfc_list_init(&tqp->tq_running);
	tqp->tq_ntasks = 0;
	tqp->tq_nnodes = 0;
	tqp->tq_taskid_next = 1;

	*tqpp = tqp;
	return 0;

error_mutex:
	pfc_mutex_destroy(&tqp->tq_mutex);
error_alloc:
	free(tqp);
	return err;
}

/*
 * static void
 * taskq_free(taskq_t *tqp)
 *	Destroy the task queue instance.
 */
static void
taskq_free(taskq_t *tqp)
{
	/* Call destructor for all tasks, and dispose all task instances. */
	pfc_rbtree_clear(&tqp->tq_tasktree, taskq_task_dtor, NULL);

	pfc_mutex_destroy(&tqp->tq_mutex);
	pfc_cond_destroy(&tqp->tq_dispcond);
	pfc_cond_destroy(&tqp->tq_joincond);
	if (tqp->tq_poolname != NULL) {
		pfc_refptr_put(tqp->tq_poolname);
	}
	pfc_refptr_put(tqp->tq_ownername);
	free(tqp);
}

/*
 * static task_t *
 * taskq_task_alloc(pfc_taskfunc_t func, void *PFC_RESTRICT arg,
 *		    pfc_taskdtor_t dtor, uint32_t flags)
 *	Allocate a task instance.
 */
static task_t *
taskq_task_alloc(pfc_taskfunc_t func, void *PFC_RESTRICT arg,
		 pfc_taskdtor_t dtor, uint32_t flags)
{
	task_t *tp = (task_t *)malloc(sizeof(*tp));
	if (PFC_EXPECT_FALSE(tp == NULL)) {
		return NULL;
	}

	tp->t_func = func;
	tp->t_arg  = arg;
	tp->t_dtor = dtor;
	tp->t_flags = 0;
	if (!(flags & PFC_TASKQ_JOINABLE)) {
		tp->t_flags |= TASK_FLAG_DETACHED;
	}
	tp->t_state = TASK_STATE_QUEUED;

	return tp;
}

/*
 * static pfc_cptr_t
 * taskq_getkey(pfc_rbnode_t *node)
 *	Return the key of taskq node specified by `node'.
 *	`node' must be a pointer to tq_node in taskq_t.
 */
static pfc_cptr_t
taskq_getkey(pfc_rbnode_t *node)
{
	taskq_t	*tqp = TASKQ_NODE2PTR(node);

	return (pfc_cptr_t)(uintptr_t)tqp->tq_id;
}

/*
 * static pfc_cptr_t
 * taskq_task_getkey(pfc_rbnode_t *node)
 *	Return the key of task node specified by `node'.
 *	`node' must be a pointer to t_node in task_t.
 */
static pfc_cptr_t
taskq_task_getkey(pfc_rbnode_t *node)
{
	task_t	*tp = TASK_NODE2PTR(node);

	return (pfc_cptr_t)(uintptr_t)tp->t_id;
}

/*
 * static void
 * taskq_task_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of task_t.
 *	`node' must be a pointer to t_node in task_t.
 */
static void
taskq_task_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	task_t	*tp = TASK_NODE2PTR(node);

	/* Call destructor of this task. */
	taskq_dtor(tp->t_dtor, tp->t_arg);

	/* Destroy task instance. */
	pfc_taskq_task_free(tp);
}
