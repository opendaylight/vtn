/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cluster.c - Cluster state management.
 */

#include <dlfcn.h>
#include <unc/lnc_ipc.h>
#include "launcher_impl.h"

/*
 * Internal prototypes.
 */
static void	cluster_setsysact(lnc_ctx_t *ctx);

/*
 * int PFC_ATTR_HIDDEN
 * lnc_cluster_boot(lnc_ctx_t *ctx, pfc_cfblk_t options)
 *	Initialize cluster configuration.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_cluster_boot(lnc_ctx_t *ctx, pfc_cfblk_t options)
{
	/* Initialize cluster node state. */
	ctx->lc_clstate = LNC_CLSTATE_INITIAL;

	return 0;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_cluster_event(lnc_ctx_t *UNC_RESTRICT ctx, uint8_t type,
 *		     pfc_bool_t sysact,
 *		     const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Raise cluster change event specified by `type'.
 *
 *	`type' specifies cluster event type defined as CLSTAT_EVTYPE_XXX.
 *	This function sends the given cluster event type to daemon processes
 *	which has clevent_order[type] configuration.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_cluster_event(lnc_ctx_t *UNC_RESTRICT ctx, uint8_t type, pfc_bool_t sysact,
		  const pfc_timespec_t *UNC_RESTRICT abstime)
{
	pfc_rbtree_t	*tree;
	pfc_rbnode_t	*node = NULL;
	int		err = 0;

	/*
	 * Determine Red-Black tree which keeps order of cluster event
	 * delivery.
	 */
	tree = lnc_daemon_order_gettree(LNC_ORDTYPE_CLEVENT, (int)type);

	/* Iterate daemons which listen this event. */
	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		lnc_ordnode_t	*onp = LNC_ORDNODE_NODE2PTR(node);

		PFC_ASSERT(onp->lo_clevent != NULL);
		pfc_log_info("Sending cluster event: name=%s, order=%u, "
			     "type=%u", LNC_ORDNODE_NAME(onp), onp->lo_order,
			     type);

		err = onp->lo_clevent(onp, type, sysact, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	if (type == CLSTAT_EVTYPE_ACT) {
		cluster_setsysact(ctx);
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_cluster_notify(lnc_ordnode_t *UNC_RESTRICT onp, uint8_t type,
 *		      pfc_bool_t sysact,
 *		      const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Notify change of cluster state to components in the UNC daemon.
 *
 *	This function sends an IPC event to the daemon process associated with
 *	`onp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_cluster_notify(lnc_ordnode_t *UNC_RESTRICT onp, uint8_t type,
		   pfc_bool_t sysact,
		   const pfc_timespec_t *UNC_RESTRICT abstime)
{
	lnc_daemon_t	*ldp = onp->lo_daemon;
	lnc_proc_t	*lprp;
	pfc_ipccladdr_t	claddr;
	pfc_ipcsrv_t	*event;
	const char	*reason = NULL;
	pid_t		pid;
	uint32_t	flags;
	int		err;

	PFC_ASSERT(ldp != NULL);
	PFC_ASSERT(abstime != NULL);
	lprp = ldp->ld_command;

	/* Create an IPC event that notifies cluster event. */
	err = pfc_module_ipcevent_create(&event, LNC_IPCEVENT_CLEVENT);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s.%s: Failed to create a CLEVENT event: "
			      "%s", LNC_DAEMON_NAME(ldp), lprp->lp_name,
			      strerror(err));

		return err;
	}

	err = pfc_ipcsrv_output_uint8(event, type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s.%s: Failed to set event type to CLEVENT: %s",
			      LNC_DAEMON_NAME(ldp), lprp->lp_name,
			      strerror(err));
		goto error_event;
	}

	err = pfc_ipcsrv_output_uint8(event, (uint8_t)sysact);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s.%s: Failed to set SYSACT to CLEVENT: %s",
			      LNC_DAEMON_NAME(ldp), lprp->lp_name,
			      strerror(err));
		goto error_event;
	}

	err = pfc_ipcsrv_output_uint64(event, (uint64_t)abstime->tv_sec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s.%s: Failed to set deadline(sec) to "
			      "CLEVENT: %s", LNC_DAEMON_NAME(ldp),
			      lprp->lp_name, strerror(err));
		goto error_event;
	}

	err = pfc_ipcsrv_output_uint64(event, (uint64_t)abstime->tv_nsec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s.%s: Failed to set deadline(nsec) to "
			      "CLEVENT: %s", LNC_DAEMON_NAME(ldp),
			      lprp->lp_name, strerror(err));
		goto error_event;
	}

	LNC_PROC_LOCK(lprp);

	flags = lprp->lp_flags;
	PFC_ASSERT((flags & LNC_PF_CLEVENT) == 0);
	if (PFC_EXPECT_FALSE(flags & LNC_PF_FINI)) {
		reason = "Already finalized";
		err = ECANCELED;
		goto error_unlock;
	}

	/* Determine target process. */
	pid = lprp->lp_pid;
	if (PFC_EXPECT_FALSE(pid == 0)) {
		goto error_noproc;
	}

	claddr.cla_pid = pid;
	lprp->lp_flags |= LNC_PF_CLEVENT;
	lprp->lp_flags &= ~LNC_PF_CLEVOK;

	/* Send an IPC event to the target process. */
	PFC_ASSERT_INT(pfc_hostaddr_init_local(&claddr.cla_hostaddr), 0);
	err = pfc_ipcsrv_event_postto(event, &claddr);
	event = NULL;

	if (PFC_EXPECT_FALSE(err != 0)) {
		reason = strerror(err);
		goto error_unlock;
	}

	/*
	 * Ensure that the target process is still alive because it may exit
	 * while the process lock is held.
	 */
	if (PFC_EXPECT_FALSE(kill(pid, 0) != 0)) {
		PFC_ASSERT(errno == ESRCH);
		goto error_noproc;
	}

	/* Wait for ACK from the target process. */
	for (;;) {
		flags = lprp->lp_flags;

		if (PFC_EXPECT_TRUE(flags & LNC_PF_CLEVOK)) {
			PFC_ASSERT((flags & LNC_PF_CLEVENT) == 0);
			err = 0;
			break;
		}
		if (PFC_EXPECT_FALSE((flags & LNC_PF_CLEVENT) == 0)) {
			pfc_log_error("%s.%s: CLEVENT failed.",
				      LNC_DAEMON_NAME(ldp), lprp->lp_name);
			err = EPERM;
			goto error_unlock;
		}
		if (PFC_EXPECT_FALSE(flags & LNC_PF_FINI)) {
			pfc_log_error("%s.%s: CLEVENT wait has been canceled.",
				      LNC_DAEMON_NAME(ldp), lprp->lp_name);
			err = ECANCELED;
			goto error_unlock;
		}

		pid = lprp->lp_pid;
		if (PFC_EXPECT_FALSE(pid == 0)) {
			pfc_log_error("%s.%s: Daemon process terminated "
				      "without responding CLEVENT.",
				      LNC_DAEMON_NAME(ldp), lprp->lp_name);
			err = ESRCH;
			goto error_unlock;
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			pfc_log_error("%s.%s: Daemon process did not respond "
				      "to CLEVENT.", LNC_DAEMON_NAME(ldp),
				      lprp->lp_name);
			goto error_unlock;
		}

		err = LNC_PROC_TIMEDWAIT_ABS(lprp, abstime);
	}

	LNC_PROC_UNLOCK(lprp);

	return err;

error_noproc:
	reason = "Mandatory daemon is dead";
	err = ESRCH;

error_unlock:
	lprp->lp_flags &= ~LNC_PF_CLEVENT;
	LNC_PROC_UNLOCK(lprp);
	if (reason != NULL) {
		pfc_log_error("%s.%s: Failed to post a CLEVENT event: %s",
			      LNC_DAEMON_NAME(ldp), lprp->lp_name, reason);
	}

error_event:
	if (event != NULL) {
		PFC_ASSERT_INT(pfc_ipcsrv_event_destroy(event), 0);
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_cluster_notify_uncd(lnc_ordnode_t *UNC_RESTRICT onp, uint8_t type,
 *			   pfc_bool_t sysact,
 *			   const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Notify change of cluster state to components in the UNC daemon.
 *
 *	This function raises a PFC event which notifies cluster state change.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_cluster_notify_uncd(lnc_ordnode_t *UNC_RESTRICT onp, uint8_t type,
			pfc_bool_t sysact,
			const pfc_timespec_t *UNC_RESTRICT abstime)
{
	PFC_ASSERT(onp->lo_order == LNC_ORDER_UNCD);
	PFC_ASSERT(onp->lo_daemon == NULL);

	return clstat_raise(type, sysact, abstime);
}

/*
 * static void
 * cluster_setsysact(lnc_ctx_t *ctx)
 *	Set SYSACT flag, which represents the cluster system becomes active.
 */
static void
cluster_setsysact(lnc_ctx_t *ctx)
{
	LNC_LOCK(ctx);

	if (!LNC_IS_SYSACT(ctx)) {
		ctx->lc_flags |= LNC_CF_SYSACT;
		pfc_log_info("The cluster system has become active.");
	}

	LNC_UNLOCK(ctx);
}
