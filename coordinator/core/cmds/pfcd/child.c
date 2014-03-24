/*
 * Copyright (c) 2011-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * child.c - Management of child process created via pfc_extcmd_t.
 */

/*
 * Use "extcmd_child" for log identifier.
 */
static const char	log_ident[] = "extcmd_child";
#undef	PFC_LOG_IDENT
#define	PFC_LOG_IDENT	log_ident

#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pfc/thread.h>
#include <pfc/rbtree.h>
#include <pfc/synch.h>
#include <extcmd_impl.h>
#include "pfcd.h"

/*
 * Timeout period, in seconds, to wait for completion of all callbacks.
 */
#define	CHILD_CBWAIT_TIMEOUT		PFC_CONST_U(10)

/*
 * Stack size of process reaper thread.
 */
#define	CHILD_REAPER_STKSIZE		PFC_CONST_U(0x10000)	/* 64K */

/*
 * Cast process ID to Red-Black tree key.
 */
#define	CHILD_KEY(pid)		((pfc_cptr_t)(uintptr_t)(pid))

struct child_extcmd;
typedef struct child_extcmd	child_extcmd_t;

/*
 * Reaped child process information.
 */
typedef struct {
	child_extcmd_t	*cc_cmd;	/* command context */
	pid_t		cc_pid;		/* process ID */
	int		cc_status;	/* status information */
} child_cb_t;

/*
 * Orphan child process information.
 */
typedef struct {
	pid_t		co_pid;		/* process ID */
	int		co_status;	/* status information */
	pfc_rbnode_t	co_node;	/* Red-Black tree node */
} child_orphan_t;

#define	CHILD_ORPHAN_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), child_orphan_t, co_node)

/*
 * Command context to execute external command.
 */
struct child_extcmd {
	extcmd_t	ce_cmd;		/* actual command context */
	pfc_rbnode_t	ce_node;	/* Red-Black tree node */
	uint32_t	ce_ncallbacks;	/* number of active callbacks */
	uint32_t	ce_flags;	/* flags */
};

#define	CHILD_NODE2PTR(node)					\
	PFC_CAST_CONTAINER((node), child_extcmd_t, ce_node)
#define	CHILD_CMD2PTR(cmd)					\
	PFC_CAST_CONTAINER((cmd), child_extcmd_t, ce_cmd)

/*
 * Flags for ce_flags.
 */
#define	CECMDF_KILLED		PFC_CONST_U(0x1)	/* already killed */

/*
 * Definition of global context which manages all child processes.
 */
typedef struct {
	pfc_rbtree_t	ct_tree;	/* child_extcmd_t tree */
	pfc_rbtree_t	ct_orphan;	/* orphan process tree */
	pfc_mutex_t	ct_mutex;	/* mutex */
	pfc_cond_t	ct_cond;	/* condition variable */
	pthread_t	ct_thread;	/* process reaper thread */
	pfc_tsd_key_t	ct_cbkey;	/* TSD key for callback thread */
	uint32_t	ct_ncallbacks;	/* number of callback threads */
	pfc_bool_t	ct_shutdown;	/* shutdown flag */
} child_tree_t;

#define	CHILD_TREE_LOCK(ctp)		pfc_mutex_lock(&(ctp)->ct_mutex)
#define	CHILD_TREE_UNLOCK(ctp)		pfc_mutex_unlock(&(ctp)->ct_mutex)

#define	CHILD_TREE_WAIT(ctp)					\
	pfc_cond_wait(&(ctp)->ct_cond, &(ctp)->ct_mutex)
#define	CHILD_TREE_TIMEDWAIT_ABS(ctp, abstime)				\
	pfc_cond_timedwait_abs(&(ctp)->ct_cond, &(ctp)->ct_mutex, (abstime))
#define	CHILD_TREE_SIGNAL(ctp)		pfc_cond_signal(&(ctp)->ct_cond)

/*
 * Command context flag which child_wait() should wait.
 */
#define	CHILD_WAIT_FLAGS	(ECMDF_EXECUTED | ECMDF_LOST)

/*
 * Argument for child_mark_lost() which notifies that the command lock is held.
 */
#define	CHILD_LOCKED		((pfc_ptr_t)(uintptr_t)1)

/*
 * Internal prototypes.
 */
static pfc_cptr_t	child_keyfunc(pfc_rbnode_t *node);
static pfc_cptr_t	child_orphan_keyfunc(pfc_rbnode_t *node);
static extcmd_t		*child_alloc(void);
static void		child_free(extcmd_t *cmd);
static void		child_register(extcmd_t *cmd);
static int		child_wait(extcmd_t *cmd);
static int		child_on_callback(extcmd_t *cmd);
static void		*child_reaper(void *arg);
static pid_t		child_wait_proc(child_tree_t *PFC_RESTRICT ctp,
					int *PFC_RESTRICT statusp);
static void		child_update(child_tree_t *PFC_RESTRICT ctp,
				     child_extcmd_t *PFC_RESTRICT cep,
				     pid_t pid, int status);
static void		child_mark_lost(pfc_rbnode_t *node, pfc_ptr_t arg);
static void		*child_callback(void *arg);
static void		child_callback_dtor(pfc_ptr_t value);
static void		child_orphan_register(child_tree_t *ctp, pid_t pid,
					      int status);
static void		child_orphan_update(child_tree_t *PFC_RESTRICT ctp,
					    child_extcmd_t *PFC_RESTRICT cep,
					    child_orphan_t *PFC_RESTRICT cop);
static void		child_orphan_dtor(pfc_rbnode_t *node,
					  pfc_ptr_t PFC_ATTR_UNUSED arg);
static void		child_killall_l(child_tree_t *ctp);

/*
 * Global context which manages all child processes.
 */
static child_tree_t	child_tree = {
	.ct_tree	= PFC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare,
						 child_keyfunc),
	.ct_orphan	= PFC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare,
						 child_orphan_keyfunc),
	.ct_mutex	= PFC_MUTEX_INITIALIZER,
	.ct_thread	= PFC_PTHREAD_INVALID_ID,
	.ct_ncallbacks	= 0,
	.ct_shutdown	= PFC_FALSE,
};

/*
 * pfc_extcmd_t operation to implement asynchronous process waiting.
 */
static const extcmd_ops_t	child_extcmd_ops = {
	.ecops_alloc		= child_alloc,
	.ecops_free		= child_free,
	.ecops_register		= child_register,
	.ecops_wait		= child_wait,
	.ecops_on_callback	= child_on_callback,
	.ecops_preexec		= signal_child_init,
};

/*
 * void
 * child_init(void)
 *	Initialize child process management.
 */
void
child_init(void)
{
	child_tree_t	*ctp = &child_tree;
	pthread_attr_t	attr;
	int		err;

	PFC_ASSERT_INT(pfc_cond_init(&ctp->ct_cond), 0);

	/* Create TSD key for callback thread. */
	err = pfc_thread_key_create(&ctp->ct_cbkey, child_callback_dtor);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to create TSD key for callback: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	/*
	 * Create child process reaper thread.
	 * We use POSIX thread instead of PFC thread because this thread
	 * never quits as long as the PFC daemon is active.
	 */
	PFC_ASSERT_INT(pthread_attr_init(&attr), 0);
	PFC_ASSERT_INT(pthread_attr_setstacksize(&attr, CHILD_REAPER_STKSIZE),
		       0);
	PFC_ASSERT_INT(pthread_attr_setdetachstate
		       (&attr, PTHREAD_CREATE_JOINABLE), 0);

	err = pthread_create(&ctp->ct_thread, &attr, child_reaper, ctp);
	PFC_ASSERT_INT(pthread_attr_destroy(&attr), 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to start child process reaper thread: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	/* Register pfc_extcmd_t operation. */
	__pfc_extcmd_setops(&child_extcmd_ops);
}

/*
 * void
 * child_fini(void)
 *	Finalize child process management.
 *	All child processes are killed by sending SIGKILL.
 */
void
child_fini(void)
{
	child_tree_t	*ctp = &child_tree;
	pfc_timespec_t	*abstime, abs, to;
	int		err;

	/* Send SIGKILL to all active processes. */
	CHILD_TREE_LOCK(ctp);
	child_killall_l(ctp);

	/* Terminate the child reaper thread. */
	ctp->ct_shutdown = PFC_TRUE;
	CHILD_TREE_SIGNAL(ctp);
	CHILD_TREE_UNLOCK(ctp);

	if (ctp->ct_thread != PFC_PTHREAD_INVALID_ID) {
		err = pthread_join(ctp->ct_thread, NULL);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to join the child reaper thread: "
				      "%s", strerror(err));
			return;
		}

		ctp->ct_thread = PFC_PTHREAD_INVALID_ID;
	}

	/* Wait for completion of all active callbacks. */
	to.tv_sec = CHILD_CBWAIT_TIMEOUT;
	to.tv_nsec = 0;
	err = pfc_clock_abstime(&abs, &to);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		pfc_log_error("Failed to calculate timeout period: %s",
			      strerror(err));
		abstime = NULL;
	}
	else {
		abstime = &abs;
	}

	CHILD_TREE_LOCK(ctp);

	while (ctp->ct_ncallbacks != 0) {
		err = CHILD_TREE_TIMEDWAIT_ABS(ctp, abstime);
		if (PFC_EXPECT_FALSE(err == ETIMEDOUT)) {
			pfc_log_error("Callback did not complete within %u "
				      "seconds.", CHILD_CBWAIT_TIMEOUT);
			CHILD_TREE_UNLOCK(ctp);

			return;
		}
		PFC_ASSERT(err == 0);
	}

	/* Clean up orphan process information. */
	pfc_rbtree_clear(&ctp->ct_orphan, child_orphan_dtor, NULL);

	CHILD_TREE_UNLOCK(ctp);

	err = pfc_thread_key_delete(ctp->ct_cbkey);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to delete TSD key for callback: %s",
			      strerror(err));
	}
}

/*
 * void
 * child_killall(void)
 *	Broadcast SIGKILL to all children.
 *
 * Remarks:
 *	This function never waits for children to exit.
 */
void
child_killall(void)
{
	child_tree_t	*ctp = &child_tree;

	CHILD_TREE_LOCK(ctp);
	child_killall_l(ctp);
	CHILD_TREE_UNLOCK(ctp);
}

/*
 * static pfc_cptr_t
 * child_keyfunc(pfc_rbnode_t *node)
 *	Return key value of the specified child process node.
 *	`node' must be a pointer to ce_node in child_extcmd_t.
 *
 * Calling/Exit State:
 *	This function returns process ID as key.
 */
static pfc_cptr_t
child_keyfunc(pfc_rbnode_t *node)
{
	child_extcmd_t	*cep = CHILD_NODE2PTR(node);

	return CHILD_KEY(cep->ce_cmd.ec_pid);
}

/*
 * static pfc_cptr_t
 * child_orphan_keyfunc(pfc_rbnode_t *node)
 *	Return key value of the specified orphan child process node.
 *	`node' must be a pointer to co_node in child_orphan_t.
 *
 * Calling/Exit State:
 *	This function returns process ID as key.
 */
static pfc_cptr_t
child_orphan_keyfunc(pfc_rbnode_t *node)
{
	child_orphan_t	*cop = CHILD_ORPHAN_NODE2PTR(node);

	return CHILD_KEY(cop->co_pid);
}

/*
 * static extcmd_t *
 * child_alloc(void)
 *	Allocate buffer for the command context.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to command context is
 *	returned. NULL is returned on failure.
 */
static extcmd_t *
child_alloc(void)
{
	child_extcmd_t	*cep;

	cep = (child_extcmd_t *)malloc(sizeof(*cep));
	if (PFC_EXPECT_TRUE(cep != NULL)) {
		/* Initialize reference counter for callback and flags. */
		cep->ce_ncallbacks = 0;
		cep->ce_flags = 0;

		return &cep->ce_cmd;
	}

	return NULL;
}

/*
 * static void
 * child_free(extcmd_t *cmd)
 *	Free the buffer specified by `cmd'.
 *	`cmd' must be a pointer returned by the call of child_alloc().
 */
static void
child_free(extcmd_t *cmd)
{
	child_extcmd_t	*cep = CHILD_CMD2PTR(cmd);

	free(cep);
}

/*
 * static void
 * child_register(extcmd_t *cmd)
 *	Register the command context specified by `cmd' to the child tree
 *	to monitor child process.
 *
 * Remarks:
 *	This function must NOT be called with holding the command lock.
 */
static void
child_register(extcmd_t *cmd)
{
	child_tree_t	*ctp = &child_tree;
	child_extcmd_t	*cep = CHILD_CMD2PTR(cmd);
	pid_t		pid;
	pfc_rbnode_t	*node;

	/*
	 * We can see ec_pid without holding lock because it is modified
	 * by the child reaper thread. The context is not yet visible to
	 * the child reaper thread.
	 */
	pid = cmd->ec_pid;
	PFC_ASSERT(pid != EXTCMD_PID_INVALID);

	CHILD_TREE_LOCK(ctp);

	/* Check to see whether this process is already reaped. */
	node = pfc_rbtree_remove(&ctp->ct_orphan, CHILD_KEY(pid));
	if (PFC_EXPECT_FALSE(node != NULL)) {
		/*
		 * This process is already reaped.
		 * Update the command context immediately.
		 */
		child_orphan_update(ctp, cep, CHILD_ORPHAN_NODE2PTR(node));
		CHILD_TREE_UNLOCK(ctp);

		return;
	}

	/* Register the context to the child tree. */
	node = pfc_rbtree_update(&ctp->ct_tree, &cep->ce_node);
	CHILD_TREE_SIGNAL(ctp);
	CHILD_TREE_UNLOCK(ctp);

	if (PFC_EXPECT_FALSE(node != NULL)) {
		child_extcmd_t	*c = CHILD_NODE2PTR(node);

		/* This should never happen. */
		PFC_ASSERT(pid == c->ce_cmd.ec_pid);
		pfc_log_error("PID(%u) is duplicated.", pid);
		child_mark_lost(&c->ce_node, NULL);
	}
}

/*
 * static int
 * child_wait(extcmd_t *cmd)
 *	Block the calling thread until the process associated with `cmd' quits.
 *
 * Calling/Exit State:
 *	Upon successful completion, process status returned by the call of
 *	wait(2) is set to cmd->ec_status, and then zero is returned.
 *	ESRCH is returned if no child process is found.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 *	Note that it may be released for a while.
 */
static int
child_wait(extcmd_t *cmd)
{
	uint32_t	flags;

	/* Wait for the child reaper thread to reap the target process. */
	while (((flags = cmd->ec_flags) & CHILD_WAIT_FLAGS) == 0) {
		EXTCMD_WAIT(cmd);
	}

	if (PFC_EXPECT_TRUE(flags & ECMDF_EXECUTED)) {
		return 0;
	}

	PFC_ASSERT(flags & ECMDF_LOST);

	return ESRCH;
}

/*
 * static int
 * child_on_callback(extcmd_t *cmd)
 *	Determine whether that the caller is called on callback context.
 *
 * Calling/Exit State:
 *	Zero is returned if this function is not called on callback context.
 *
 *	An error number is returned if this function is called on the callback
 *	context. ELOOP is returned if `cmd' equals the context currently
 *	callbacked, EAGAIN if it does not equal.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 *	Note that the lock may be released for a while.
 */
static int
child_on_callback(extcmd_t *cmd)
{
	child_tree_t	*ctp = &child_tree;
	child_extcmd_t	*current;

	current = (child_extcmd_t *)pfc_thread_getspecific(ctp->ct_cbkey);
	if (current == NULL) {
		return 0;
	}

	return (&current->ce_cmd == cmd) ? ELOOP : EAGAIN;
}

/*
 * static void *
 * child_reaper(void *arg)
 *	Start routine of the process reaper thread.
 */
static void *
child_reaper(void *arg)
{
	child_tree_t	*ctp = (child_tree_t *)arg;
	pfc_rbtree_t	*tree = &ctp->ct_tree;

	PFC_ASSERT(ctp == &child_tree);

	pfc_log_verbose("Start process reaper thread.");
	CHILD_TREE_LOCK(ctp);

	for (;;) {
		child_extcmd_t	*cep;
		pfc_rbnode_t	*node;
		pid_t		pid;
		int		status;

		/* Wait for a child process to be registered. */
		if (tree->rb_root == NULL) {
			if (ctp->ct_shutdown) {
				break;
			}
			pfc_log_verbose("Waiting.");
			CHILD_TREE_WAIT(ctp);
			pfc_log_verbose("Woken up.");
			continue;
		}

		/* Wait for a child process to quit. */
		pid = child_wait_proc(ctp, &status);
		if (PFC_EXPECT_FALSE(pid == (pid_t)-1)) {
			continue;
		}

		/*
		 * Remove the child node associated with the terminated
		 * process from the child tree.
		 */
		node = pfc_rbtree_remove(tree, CHILD_KEY(pid));
		if (PFC_EXPECT_FALSE(node == NULL)) {
			child_orphan_register(ctp, pid, status);
			continue;
		}

		/* Wake up threads blocked on this context. */
		cep = CHILD_NODE2PTR(node);

		/* Update the command context. */
		child_update(ctp, cep, pid, status);
	}

	CHILD_TREE_UNLOCK(ctp);
	pfc_log_verbose("Process reaper thread has been terminated.");

	return NULL;
}

/*
 * static pid_t
 * child_wait_proc(child_tree_t *PFC_RESTRICT ctp, int *PFC_RESTRICT statusp)
 *	Wait for one child process to quit.
 *
 * Calling/Exit State:
 *	Process ID of terminated process is returned on success.
 *	-1 is returned if no child process is found.
 *
 * Remarks:
 *	This function must be called with holding the global tree lock.
 *	Note that this function releases it for a while.
 */
static pid_t
child_wait_proc(child_tree_t *PFC_RESTRICT ctp, int *PFC_RESTRICT statusp)
{
	pid_t	pid;

	CHILD_TREE_UNLOCK(ctp);

	/*
	 * Remarks:
	 *	This function must be called on the child reaper thread.
	 *	So we don't need to ensure that child node exists in the child
	 *	tree in this loop because only the child reaper thread removes
	 *	child node from the tree.
	 */
	for (;;) {
		pid = wait(statusp);
		if (PFC_EXPECT_FALSE(pid == (pid_t)-1)) {
			int	err = errno;

			if (err == EINTR) {
				continue;
			}

			if (PFC_EXPECT_FALSE(err != ECHILD)) {
				pfc_log_error("wait() failed: %s",
					      strerror(err));
			}

			/*
			 * Set the lost mark to all child process context,
			 * and clear child tree.
			 */
			CHILD_TREE_LOCK(ctp);
			pfc_rbtree_clear(&ctp->ct_tree, child_mark_lost, NULL);
			break;
		}

		if (PFC_EXPECT_TRUE(WIFEXITED(*statusp) ||
				    WIFSIGNALED(*statusp))) {
			CHILD_TREE_LOCK(ctp);
			break;
		}
	}

	return pid;
}

/*
 * static void
 * child_update(child_tree_t *PFC_RESTRICT ctp,
 *		child_extcmd_t *PFC_RESTRICT cep, pid_t pid, int status)
 *	Update status of the command context.
 *
 *	`pid' and `status' must be an ID of reaped child process and status
 *	information respectively.
 *
 * Remarks:
 *	This function must be called with holding the global tree lock.
 */
static void
child_update(child_tree_t *PFC_RESTRICT ctp, child_extcmd_t *PFC_RESTRICT cep,
	     pid_t pid, int status)
{
	extcmd_t	*cmd = &cep->ce_cmd;
	child_cb_t	*ccp;
	pfc_thread_t	t;
	int		err;

	EXTCMD_LOCK(cmd);

	pfc_log_debug("%s: Reaped: pid=%u, status=0x%x",
		      cmd->ec_command, pid, status);
	PFC_ASSERT(cmd->ec_pid == pid);

	if (cmd->ec_callback == NULL) {
		/* No callback is configured on this context. */
		cmd->ec_status = status;
		cmd->ec_flags |= ECMDF_EXECUTED;
		EXTCMD_BROADCAST(cmd);
		goto out;
	}

	/* Allocate a context for callback execution. */
	ccp = (child_cb_t *)malloc(sizeof(*ccp));
	if (PFC_EXPECT_FALSE(ccp == NULL)) {
		pfc_log_error("Failed to allocate callback context.");
		child_mark_lost(&cep->ce_node, CHILD_LOCKED);
		goto out;
	}

	ccp->cc_cmd = cep;
	ccp->cc_pid = pid;
	ccp->cc_status = status;

	/* Create a callback thread. */
	err = pfc_thread_create(&t, child_callback, ccp, PFC_THREAD_DETACHED);
	if (PFC_EXPECT_FALSE(err != 0)) {
		free(ccp);
		pfc_log_error("Failed to create callback thread: %s",
			      strerror(err));
		child_mark_lost(&cep->ce_node, CHILD_LOCKED);
		goto out;
	}

	PFC_ASSERT((cep->ce_ncallbacks != 0) ==
		   ((cmd->ec_flags & ECMDF_CALLBACK) != 0));

	/* Set callback flag. */
	cmd->ec_flags |= ECMDF_CALLBACK;

	/* Reset the command context. */
	cmd->ec_flags &= ~ECMDF_RUNNING;
	cmd->ec_pid = EXTCMD_PID_INVALID;

	cep->ce_ncallbacks++;
	ctp->ct_ncallbacks++;

out:
	EXTCMD_UNLOCK(cmd);
}

/*
 * static void
 * child_mark_lost(pfc_rbnode_t *node, pfc_ptr_t locked)
 *	Set the lost mark to the specified command context, in order to notify
 *	that the command status was lost.
 *
 *	`node' must be a pointer to ce_node in child_extcmd_t.
 *
 *	A non-NULL value must be specified to `locked' if the caller calls
 *	this function with holding the command lock.
 */
static void
child_mark_lost(pfc_rbnode_t *node, pfc_ptr_t locked)
{
	child_extcmd_t	*cep = CHILD_NODE2PTR(node);
	extcmd_t	*cmd = &cep->ce_cmd;

	if (locked == NULL) {
		EXTCMD_LOCK(cmd);
	}
	pfc_log_error("%s: Command has been lost: pid=%u",
		      cmd->ec_command, cmd->ec_pid);
	cmd->ec_status = 0;
	cmd->ec_flags |= ECMDF_LOST;
	EXTCMD_BROADCAST(cmd);

	if (locked == NULL) {
		EXTCMD_UNLOCK(cmd);
	}
}

/*
 * static void *
 * child_callback(void *arg)
 *	Start routine of callback thread, which calls callback function for
 *	the specified command.
 */
static void *
child_callback(void *arg)
{
	pfc_extcmd_proc_t	proc;
	child_cb_t	*ccp = (child_cb_t *)arg;
	child_extcmd_t	*cep = ccp->cc_cmd;
	child_tree_t	*ctp = &child_tree;
	pid_t		pid = ccp->cc_pid;
	int		err, status = ccp->cc_status;
	extcmd_t	*cmd = &cep->ce_cmd;
	pfc_extcmd_t	ecmd;
	pfc_extcmd_cb_t	callback;
	pfc_ptr_t	cbarg;

	free(ccp);

	EXTCMD_LOCK(cmd);

	pfc_log_debug("%s: Callback: func=%p", cmd->ec_command,
		      cmd->ec_callback);

	PFC_ASSERT(cmd->ec_flags & (ECMDF_CALLBACK | ECMDF_RUNNING));
	PFC_ASSERT((cmd->ec_flags & (ECMDF_EXECUTED | ECMDF_LOST)) == 0);
	PFC_ASSERT(cep->ce_ncallbacks != 0);
	PFC_ASSERT(cmd->ec_pid == EXTCMD_PID_INVALID);

	/* Set this command context to thread specific data. */
	err = pfc_thread_setspecific(ctp->ct_cbkey, cep);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to set TSD: %s", strerror(err));
		child_mark_lost(&cep->ce_node, CHILD_LOCKED);
		EXTCMD_UNLOCK(cmd);

		child_callback_dtor((pfc_ptr_t)cep);

		return NULL;
	}

	callback = cmd->ec_callback;
	cbarg = cmd->ec_cbarg;
	cmd->ec_cbarg = NULL;
	ecmd = cmd->ec_id;

	/* Enable to get status of child process. */
	cmd->ec_status = status;
	cmd->ec_flags |= ECMDF_EXECUTED;

	EXTCMD_UNLOCK(cmd);

	/* Call callback function. */
	PFC_ASSERT(callback != NULL);
	proc.pep_pid = pid;
	if (WIFEXITED(status)) {
		proc.pep_status = WEXITSTATUS(status);
		proc.pep_signal = PFC_EXTCMD_ERR_UNSPEC;
	}
	else {
		PFC_ASSERT(WIFSIGNALED(status));
		proc.pep_status = PFC_EXTCMD_ERR_UNSPEC;
		proc.pep_signal = WTERMSIG(status);
	}
	(*callback)(ecmd, &proc, cbarg);

	return NULL;
}

/*
 * static void
 * child_callback_dtor(pfc_ptr_t value)
 *	Destructor of callback thread.
 */
static void
child_callback_dtor(pfc_ptr_t value)
{
	child_extcmd_t	*cep = (child_extcmd_t *)value;
	extcmd_t	*cmd = &cep->ce_cmd;
	child_tree_t	*ctp = &child_tree;
	pfc_bool_t	destroyed = PFC_FALSE;

	EXTCMD_LOCK(cmd);

	PFC_ASSERT(cmd->ec_flags & ECMDF_CALLBACK);
	PFC_ASSERT(cep->ce_ncallbacks != 0);

	if (cmd->ec_pid == EXTCMD_PID_INVALID) {
		/* Reset I/O handles. */
		__pfc_extcmd_ioreset(cmd);
	}

	/* Update callback status. */
	cep->ce_ncallbacks--;
	if (cep->ce_ncallbacks == 0) {
		cmd->ec_flags &= ~ECMDF_CALLBACK;
		EXTCMD_BROADCAST(cmd);

		/*
		 * The context should be destroyed if ECMDF_CB_DESTROY flag
		 * is set. But we must ensure that ec_pid does not keep valid
		 * process ID because callback function may repost a new
		 * command.
		 */
		if ((cmd->ec_flags & ECMDF_CB_DESTROY) &&
		    cmd->ec_pid == EXTCMD_PID_INVALID) {
			destroyed = PFC_TRUE;
		}
	}

	EXTCMD_UNLOCK(cmd);

	if (destroyed) {
		/* Destroy this context. */
		__pfc_extcmd_destroy(cmd);
	}

	CHILD_TREE_LOCK(ctp);
	PFC_ASSERT(ctp->ct_ncallbacks != 0);

	ctp->ct_ncallbacks--;
	if (ctp->ct_shutdown && ctp->ct_ncallbacks == 0) {
		CHILD_TREE_SIGNAL(ctp);
	}

	CHILD_TREE_UNLOCK(ctp);
}

/*
 * static void
 * child_orphan_register(child_tree_t *ctp, pid_t pid, int status)
 *	Register orphan child process information to the orphan process tree.
 *
 *	`pid' and `status' must be an ID of orphan child process and status
 *	information respectively.
 *
 * Remarks:
 *	This function must be called with holding the global tree lock.
 */
static void
child_orphan_register(child_tree_t *ctp, pid_t pid, int status)
{
	child_orphan_t	*cop;
	pfc_rbnode_t	*node;

	pfc_log_warn("Orphan process has been found: pid=%u, status=0x%x",
		     pid, status);

	cop = (child_orphan_t *)malloc(sizeof(*cop));
	if (PFC_EXPECT_FALSE(cop == NULL)) {
		pfc_log_error("Failed to allocate orphan process data.");

		return;
	}

	cop->co_pid = pid;
	cop->co_status = status;
	node = pfc_rbtree_update(&ctp->ct_orphan, &cop->co_node);
	if (PFC_EXPECT_FALSE(node != NULL)) {
		child_orphan_t	*c = CHILD_ORPHAN_NODE2PTR(node);

		/* This should never happen. */
		PFC_ASSERT(pid == c->co_pid);
		pfc_log_error("Orphan process ID(%u) is duplicated.", pid);
		free(c);
	}
}

/*
 * static void
 * child_orphan_update(child_tree_t *PFC_RESTRICT ctp,
 *		       child_extcmd_t *PFC_RESTRICT cep,
 *		       child_orphan_t *PFC_RESTRICT cop)
 *	Update status of the command context associated with the specified
 *	orphan child process.
 *
 *	`cop' must point child_orphan_t which keeps process information
 *	of the orphan process. Note that this function always releases
 *	child_orphan_t buffer pointed by `cop'.
 *
 * Remarks:
 *	This function must be called with holding the global tree lock.
 */
static void
child_orphan_update(child_tree_t *PFC_RESTRICT ctp,
		    child_extcmd_t *PFC_RESTRICT cep,
		    child_orphan_t *PFC_RESTRICT cop)
{
	pid_t	pid = cop->co_pid;
	int	status = cop->co_status;

	pfc_log_notice("Orphan process has been reaped: pid=%u, status=0x%x",
		       pid, status);

	free(cop);
	child_update(ctp, cep, pid, status);
}

/*
 * static void
 * child_orphan_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destroy orphan process information.
 *
 * Remarks:
 *	This function is called via the call of pfc_rbtree_clear().
 */
static void
child_orphan_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	child_orphan_t	*cop = CHILD_ORPHAN_NODE2PTR(node);
	pid_t		pid = cop->co_pid;
	int		status = cop->co_status;

	free(cop);
	pfc_log_warn("Unreaped orphan process: pid=%u, status=0x%x",
		     pid, status);
}

/*
 * static void
 * child_killall_l(child_tree_t *ctp)
 *	Broadcast SIGKILL to all children.
 *
 * Remarks:
 *	This function must be called with holding the global tree lock.
 */
static void
child_killall_l(child_tree_t *ctp)
{
	pfc_rbtree_t	*tree = &ctp->ct_tree;
	pfc_rbnode_t	*node = NULL;

	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		child_extcmd_t	*cep = CHILD_NODE2PTR(node);
		extcmd_t	*cmd = &cep->ce_cmd;
		pid_t		pid;

		EXTCMD_LOCK(cmd);

		/* Inhibit further command execution. */
		cmd->ec_flags |= ECMDF_DESTROYED;

		pid = cmd->ec_pid;
		if (PFC_EXPECT_FALSE(pid != EXTCMD_PID_INVALID &&
				     (cep->ce_flags & CECMDF_KILLED) == 0)) {
			pfc_log_warn("%s: Terminate by sending SIGKILL: "
				     "pid=%u", cmd->ec_command, pid);
			(void)kill(pid, SIGKILL);
			cep->ce_flags |= CECMDF_KILLED;
		}

		EXTCMD_UNLOCK(cmd);
	}
}
