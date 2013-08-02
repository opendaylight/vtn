/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ipc.c - IPC service handler.
 */

#include <stdio.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unc/lnc_ipc.h>
#include <unc/usess_ipc.h>
#include <unc/tc/external/tc_services.h>
#include <pfc/conf.h>
#include "launcher_impl.h"

/*
 * Default and maximum timeout for CLEVENT service.
 */
#define	CLEVENT_TIMEOUT		PFC_CONST_U(30)		/* 30 seconds */
#define	CLEVENT_TIMEOUT_MAX	PFC_CONST_U(86400)	/* 1 day */

/*
 * Additional time reserved for sending response code of CLEVENT service.
 * This value is added to CLEVENT service timeout.
 */
#define	CLEVENT_IOTIME		PFC_CONST_U(5)		/* 5 seconds */

/*
 * Convert internal state flags (LNC_CF_XXX) into launcher state flags
 * (LNC_IPC_LNCSTAT_XXX).
 */
#define	LNCSTAT_STATE_MASK						\
	(LNC_CF_INIT | LNC_CF_FINI | LNC_CF_SYSACT | LNC_CF_STARTED)
#define	LNCSTAT_STATE(ctx)	((ctx)->lc_flags & LNCSTAT_STATE_MASK)

#if	LNC_CF_INIT != LNC_IPC_LNCSTAT_INIT
#error	LNC_CF_INIT must equal LNC_IPC_LNCSTAT_INIT.
#endif	/* LNC_CF_INIT != LNC_IPC_LNCSTAT_INIT */

#if	LNC_CF_FINI != LNC_IPC_LNCSTAT_FINI
#error	LNC_CF_FINI must equal LNC_IPC_LNCSTAT_FINI.
#endif	/* LNC_CF_FINI != LNC_IPC_LNCSTAT_FINI */

#if	LNC_CF_SYSACT != LNC_IPC_LNCSTAT_SYSACT
#error	LNC_CF_SYSACT must equal LNC_IPC_LNCSTAT_SYSACT.
#endif	/* LNC_CF_SYSACT != LNC_IPC_LNCSTAT_SYSACT */

#if	LNC_CF_STARTED != LNC_IPC_LNCSTAT_STARTED
#error	LNC_CF_STARTED must equal LNC_IPC_LNCSTAT_STARTED.
#endif	/* LNC_CF_STARTED != LNC_IPC_LNCSTAT_STARTED */

/*
 * Launcher state flags required by CLEVENT service.
 */
#define	CLEVENT_AVAIL_FLAGS	(LNC_CF_INIT | LNC_CF_STARTED)

/*
 * Prototype of IPC service handler.
 */
typedef pfc_ipcresp_t	(*lnc_ipcserv_t)(pfc_ipcsrv_t *srv);

/*
 * IPC client session.
 * This is used by CLEVENT service.
 */
typedef struct {
	pfc_ipcconn_t	lcs_conn;	/* IPC client connection */
	pfc_ipcsess_t	*lcs_sess;	/* IPC client session */
} lnc_clsess_t;

/*
 * Internal prototypes.
 */
static pfc_ipcresp_t	ipc_notify(pfc_ipcsrv_t *srv);
static pfc_ipcresp_t	ipc_clstate(pfc_ipcsrv_t *srv);
static pfc_ipcresp_t	ipc_clevack(pfc_ipcsrv_t *srv);
static pfc_ipcresp_t	ipc_status(pfc_ipcsrv_t *srv);
static pfc_ipcresp_t	ipc_vstatus(pfc_ipcsrv_t *srv);
static pfc_ipcresp_t	ipc_clevent(pfc_ipcsrv_t *srv);
static pfc_ipcresp_t	ipc_lncstat(pfc_ipcsrv_t *srv);

static pid_t		ipc_getpid(pfc_ipcsrv_t *UNC_RESTRICT srv,
				   const char *UNC_RESTRICT svname);
static pfc_ipcresp_t	ipc_status_all(pfc_ipcsrv_t *UNC_RESTRICT srv,
				       lnc_ctx_t *UNC_RESTRICT ctx);
static pfc_ipcresp_t	ipc_status_byname(pfc_ipcsrv_t *UNC_RESTRICT srv,
					  lnc_ctx_t *UNC_RESTRICT ctx,
					  uint32_t count);
static pfc_ipcresp_t	ipc_status_getdaemons(pfc_ipcsrv_t *UNC_RESTRICT srv,
					      lnc_ctx_t *UNC_RESTRICT ctx,
					      uint32_t count,
					      lnc_daemon_t ***UNC_RESTRICT
					      arrayp);
static void		ipc_status_fill(lnc_daemon_t *UNC_RESTRICT ldp,
					lnc_proc_t *UNC_RESTRICT lprp,
					const char *UNC_RESTRICT dname,
					lncipc_dmstat_t *UNC_RESTRICT statp);
static pfc_ipcresp_t	ipc_status_error(pfc_ipcsrv_t *UNC_RESTRICT srv,
					 const char *UNC_RESTRICT fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);

static pfc_ipcresp_t	ipc_vstatus_all(pfc_ipcsrv_t *UNC_RESTRICT srv,
					lnc_ctx_t *UNC_RESTRICT ctx);
static pfc_ipcresp_t	ipc_vstatus_byname(pfc_ipcsrv_t *UNC_RESTRICT srv,
					   lnc_ctx_t *UNC_RESTRICT ctx,
					   uint32_t count);

static int	ipc_vstatus_output(pfc_ipcsrv_t *UNC_RESTRICT srv,
				   lnc_ctx_t *UNC_RESTRICT ctx,
				   lnc_daemon_t *UNC_RESTRICT ldp);
static int	ipc_vstatus_output_errlog(pfc_ipcsrv_t *UNC_RESTRICT srv,
					  lnc_proc_t *UNC_RESTRICT lprp,
					  const char *UNC_RESTRICT dname);

/*
 * static inline pfc_ipcresp_t PFC_FATTR_ALWAYS_INLINE
 * ipc_service_enter_l(lnc_ctx_t *ctx)
 *	Commom prologue for IPC service handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returend.
 *	Otherwise PFC_IPCRESP_FATAL is returned.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
static inline pfc_ipcresp_t PFC_FATTR_ALWAYS_INLINE
ipc_service_enter_l(lnc_ctx_t *ctx)
{
	if (PFC_EXPECT_FALSE(LNC_IS_FINI(ctx))) {
		pfc_log_error("Reject IPC service request after shutdown.");

		return PFC_IPCRESP_FATAL;
	}

	ctx->lc_nipcs++;

	return 0;
}

/*
 * static inline pfc_ipcresp_t PFC_FATTR_ALWAYS_INLINE
 * ipc_service_enter(lnc_ctx_t *ctx)
 *	Commom prologue for IPC service handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returend.
 *	Otherwise PFC_IPCRESP_FATAL is returned.
 */
static inline pfc_ipcresp_t PFC_FATTR_ALWAYS_INLINE
ipc_service_enter(lnc_ctx_t *ctx)
{
	pfc_ipcresp_t	resp;

	LNC_LOCK(ctx);
	resp = ipc_service_enter_l(ctx);
	LNC_UNLOCK(ctx);

	return resp;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_service_exit_l(lnc_ctx_t *ctx)
 *	Common epilogue of IPC service handler.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_service_exit_l(lnc_ctx_t *ctx)
{
	PFC_ASSERT(ctx->lc_nipcs != 0);

	ctx->lc_nipcs--;
	if (PFC_EXPECT_FALSE(LNC_IS_FINI(ctx) && ctx->lc_nipcs == 0)) {
		LNC_BROADCAST(ctx);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_service_exit(lnc_ctx_t *ctx)
 *	Common epilogue of IPC service handler.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_service_exit(lnc_ctx_t *ctx)
{
	LNC_LOCK(ctx);
	ipc_service_exit_l(ctx);
	LNC_UNLOCK(ctx);
}

/*
 * List of IPC service handlers.
 * Array index must be the same as IPC service ID.
 */
static lnc_ipcserv_t	ipc_services[] = {
	ipc_notify,		/* LNC_IPC_SVID_NOTIFY */
	ipc_clstate,		/* LNC_IPC_SVID_CLSTATE */
	ipc_clevack,		/* LNC_IPC_SVID_CLEVACK */
	ipc_status,		/* LNC_IPC_SVID_STATUS */
	ipc_vstatus,		/* LNC_IPC_SVID_VSTATUS */
	ipc_clevent,		/* LNC_IPC_SVID_CLEVENT */
	ipc_lncstat,		/* LNC_IPC_SVID_LNCSTAT */
};

/*
 * int
 * lnc_ipc_fini(lnc_ctx_t *UNC_RESTRICT ctx,
 *		const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Finalize IPC services provided by the launcher module.
 *
 *	`abstime' must be a pointer to pfc_timespec_t which represents deadline
 *	of stopping IPC services.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
lnc_ipc_fini(lnc_ctx_t *UNC_RESTRICT ctx,
	     const pfc_timespec_t *UNC_RESTRICT abstime)
{
	int		err = 0;

	LNC_LOCK(ctx);

	for (;;) {
		if (PFC_EXPECT_TRUE(ctx->lc_nipcs == 0)) {
			err = 0;
			break;
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			pfc_log_error("IPC services did not complete.");
			break;
		}

		err = LNC_TIMEDWAIT_ABS(ctx, abstime);
	}

	LNC_UNLOCK(ctx);

	return err;
}

/*
 * pfc_ipcresp_t PFC_ATTR_HIDDEN
 * lnc_ipc_service(pfc_ipcsrv_t *srv, pfc_ipcid_t service)
 *	Entry point of IPC service handlers provided by the launcher module.
 *
 * Calling/Exit State:
 *	Depends on IPC service ID specified by `service'.
 */
pfc_ipcresp_t PFC_ATTR_HIDDEN
lnc_ipc_service(pfc_ipcsrv_t *srv, pfc_ipcid_t service)
{
	lnc_ipcserv_t	serv = ipc_services[service];

	PFC_ASSERT(service < PFC_ARRAY_CAPACITY(ipc_services));

	return (*serv)(srv);
}

/*
 * static pfc_ipcresp_t
 * ipc_notify(pfc_ipcsrv_t *srv)
 *	Handle LNC_IPC_SVID_NOTIFY service.
 *
 *	This service receives service-ready notification from a daemon launched
 *	by the launcher daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise PFC_IPCRESP_FATAL is returned.
 */
static pfc_ipcresp_t
ipc_notify(pfc_ipcsrv_t *srv)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_ipcresp_t	resp;
	const char	*channel = NULL;
	pid_t		pid;
	int		err;

	resp = ipc_service_enter(ctx);
	if (PFC_EXPECT_FALSE(resp != 0)) {
		return resp;
	}

	/* Get process ID of the client. */
	pid = ipc_getpid(srv, "NOTIFY");
	if (PFC_EXPECT_FALSE(pid == (pid_t)-1)) {
		resp = PFC_IPCRESP_FATAL;
		goto out;
	}

	/* Get IPC channel name. */
	err = pfc_ipcsrv_getarg_string(srv, 0, &channel);
	if (PFC_EXPECT_FALSE(err != 0 && err != EINVAL)) {
		pfc_log_error("NOTIFY: Failed to receive IPC channel name: %s",
			      strerror(err));
		resp = PFC_IPCRESP_FATAL;
		goto out;
	}

	/* Notify that this daemon becomes ready. */
	err = lnc_daemon_notify(pid, channel);
	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_log_verbose("NOTIFY: pid=%u, channel=%s", pid,
				(channel == NULL) ? "<null>" : channel);
	}
	else {
		resp = PFC_IPCRESP_FATAL;
		pfc_log_error("NOTIFY: Notification has been failed: pid=%u, "
			      "channel=%s", pid,
			      (channel == NULL) ? "<null>" : channel);
	}

out:
	ipc_service_exit(ctx);

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_clstate(pfc_ipcsrv_t *srv)
 *	Handle LNC_IPC_SVID_CLSTATE service.
 *
 *	This service returns clst_state_t value which represents current
 *	cluster node state.
 *
 * Calling/Exit State:
 *	clst_state_t value which represents current node state is returned.
 */
static pfc_ipcresp_t
ipc_clstate(pfc_ipcsrv_t *srv)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_ipcresp_t	resp;
	pfc_bool_t	sysact;
	int		err;

	LNC_LOCK(ctx);
	resp = ipc_service_enter_l(ctx);
	if (PFC_EXPECT_FALSE(resp != 0)) {
		LNC_UNLOCK(ctx);

		return resp;
	}

	resp = (pfc_ipcresp_t)ctx->lc_clstate;
	sysact = LNC_GETSYSACT(ctx);
	LNC_UNLOCK(ctx);

	PFC_ASSERT(resp != PFC_IPCRESP_FATAL);

	err = pfc_ipcsrv_output_uint8(srv, sysact);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_clstate: Failed to output sysact: %s",
			      strerror(err));
		resp = PFC_IPCRESP_FATAL;
		/* FALLTHROUGH */
	}

	ipc_service_exit(ctx);

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_clevack(pfc_ipcsrv_t *srv)
 *	Handle LNC_IPC_SVID_CLEVACK service.
 *
 *	This service receives acknowledgement of LNC_IPCEVENT_CLEVENT event.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise PFC_IPCRESP_FATAL is returned.
 */
static pfc_ipcresp_t
ipc_clevack(pfc_ipcsrv_t *srv)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_ipcresp_t	resp;
	pid_t		pid;
	uint8_t		result;
	int		err;

	resp = ipc_service_enter(ctx);
	if (PFC_EXPECT_FALSE(resp != 0)) {
		return resp;
	}

	/* Determine process ID of the client. */
	pid = ipc_getpid(srv, "CLEVACK");
	if (PFC_EXPECT_FALSE(pid == (pid_t)-1)) {
		resp = PFC_IPCRESP_FATAL;
		goto out;
	}

	/* Get result of cluster state transition. */
	err = pfc_ipcsrv_getarg_uint8(srv, 0, &result);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("CLEVACK: Failed to get result of transition: "
			      "%s", strerror(err));
		resp = PFC_IPCRESP_FATAL;
		goto out;
	}

	/* Notify that acknowledgement has been received. */
	err = lnc_daemon_clevent_ack(pid, result);
	if (PFC_EXPECT_FALSE(err != 0)) {
		resp = PFC_IPCRESP_FATAL;
		/* FALLTHROUGH */
	}

out:
	ipc_service_exit(ctx);

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_status(pfc_ipcsrv_t *srv)
 *	Handle LNC_IPC_SVID_STATUS service.
 *
 *	This service gets daemon status associated with daemon names sent by
 *	the client.
 *
 * Calling/Exit State:
 *	Upon successful completion, LNC_IPC_STATUS_OK is returned.
 *
 *	On error, an error message is set to IPC additional data, and
 *	LNC_IPC_STATUS_ERROR is returned.
 *
 *	PFC_IPCRESP_FATAL is returned if even an error message can not be set.
 */
static pfc_ipcresp_t
ipc_status(pfc_ipcsrv_t *srv)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	uint32_t	count = pfc_ipcsrv_getargcount(srv);
	pfc_ipcresp_t	resp;
	pfc_timespec_t	timeout = {0, 0};
	int		err;

	LNC_LOCK(ctx);

	resp = ipc_service_enter_l(ctx);
	if (PFC_EXPECT_FALSE(resp != 0)) {
		goto out;
	}

	/* Check launcher module state with specifying zero timeout. */
	err = lnc_wait(ctx, &timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ECANCELED) {
			resp = ipc_status_error(srv, "The launcher module is "
						"already finalized.");
		}
		else {
			resp = ipc_status_error(srv, "The launcher module is "
						"now launching daemons.");
		}
	}
	else if (count == 0) {
		/* Send all daemon information. */
		resp = ipc_status_all(srv, ctx);
	}
	else {
		/* Send daemon information required by the client. */
		resp = ipc_status_byname(srv, ctx, count);
	}

	ipc_service_exit_l(ctx);

out:
	LNC_UNLOCK(ctx);

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_vstatus(pfc_ipcsrv_t *srv)
 *	Handle LNC_IPC_SVID_VSTATUS service.
 *
 *	This service gets verbose daemon status associated with daemon names
 *	sent by the client.
 *
 * Calling/Exit State:
 *	Upon successful completion, LNC_IPC_STATUS_OK is returned.
 *
 *	On error, an error message is set to IPC additional data, and
 *	LNC_IPC_STATUS_ERROR is returned.
 *
 *	PFC_IPCRESP_FATAL is returned if even an error message can not be set.
 */
static pfc_ipcresp_t
ipc_vstatus(pfc_ipcsrv_t *srv)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	uint32_t	count = pfc_ipcsrv_getargcount(srv);
	pfc_ipcresp_t	resp;
	pfc_timespec_t	timeout = {0, 0};
	int		err;

	LNC_LOCK(ctx);

	resp = ipc_service_enter_l(ctx);
	if (PFC_EXPECT_FALSE(resp != 0)) {
		goto out;
	}

	/* Check launcher module state with specifying zero timeout. */
	err = lnc_wait(ctx, &timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ECANCELED) {
			resp = ipc_status_error(srv, "The launcher module is "
						"already finalized.");
		}
		else {
			resp = ipc_status_error(srv, "The launcher module is "
						"now launching daemons.");
		}
	}
	else if (count == 0) {
		/* Send all daemon information. */
		resp = ipc_vstatus_all(srv, ctx);
	}
	else {
		/* Send daemon information required by the client. */
		resp = ipc_vstatus_byname(srv, ctx, count);
	}

	ipc_service_exit_l(ctx);

out:
	LNC_UNLOCK(ctx);

	return resp;
}


/*
 * static pfc_ipcresp_t
 * ipc_clevent(pfc_ipcsrv_t *srv)
 *	Handle LNC_IPC_SVID_CLEVENT service.
 *
 *	This service raise a requested type of cluster state change event.
 *
 * Calling/Exit State:
 *	An response code for CLEVENT service (LNC_IPC_CLEVENT_XXX) is returned.
 */
static pfc_ipcresp_t
ipc_clevent(pfc_ipcsrv_t *srv)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	lnc_clstate_t	state, newstate;
	pfc_timespec_t	limit;
	pfc_ipcresp_t	resp;
	pfc_bool_t	sysact;
	uint8_t		type;
	uint32_t	timeout, exstatus;
	int		err;

	resp = ipc_service_enter(ctx);
	if (PFC_EXPECT_FALSE(resp != 0)) {
		return resp;
	}

	/* Get event type sent by the client. */
	err = pfc_ipcsrv_getarg_uint8(srv, 0, &type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("CLEVENT: Failed to receive cluster event type: "
			      "%s", strerror(err));
		resp = LNC_IPC_CLEVENT_INVALID;
		goto error;
	}

	if (PFC_EXPECT_FALSE(type >= CLSTAT_NEVTYPES)) {
		pfc_log_error("CLEVENT: Unsupported event type: %u", type);
		resp = LNC_IPC_CLEVENT_INVALID;
		goto error;
	}

	/* Get timeout sent by the client. */
	err = pfc_ipcsrv_getarg_uint32(srv, 1, &timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		if (PFC_EXPECT_FALSE(timeout > CLEVENT_TIMEOUT_MAX)) {
			pfc_log_error("CLEVENT: Too large timeout: %u",
				      timeout);
			resp = LNC_IPC_CLEVENT_INVALID;
			goto error;
		}
	}
	else {
		if (PFC_EXPECT_FALSE(err != EINVAL)) {
			pfc_log_error("CLEVENT: Failed to receive timeout: %s",
				      strerror(err));
			resp = LNC_IPC_CLEVENT_INVALID;
			goto error;
		}

		/* Use default timeout. */
		timeout = CLEVENT_TIMEOUT;
	}

	/* Set server session timeout. */
	limit.tv_sec = timeout + CLEVENT_IOTIME;
	limit.tv_nsec = 0;
	err = pfc_ipcsrv_settimeout(srv, &limit);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("CLEVENT: Failed to set server session timeout: "
			      "%s", strerror(err));
		resp = LNC_IPC_CLEVENT_FAILED;
		goto error;
	}

	/* Determine deadline of CLEVENT service. */
	PFC_ASSERT_INT(pfc_clock_gettime(&limit), 0);
	limit.tv_sec += timeout;

	/* Wait for the system to be ready for cluster state transition. */
	LNC_LOCK(ctx);
	err = lnc_wait_flags(ctx, CLEVENT_AVAIL_FLAGS, &limit);

	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ETIMEDOUT) {
			pfc_log_error("CLEVENT: Timed out.");
		}
		else {
			PFC_ASSERT(err == ECANCELED);
			pfc_log_error("CLEVENT: UNC daemon is going to exit.");
		}

		resp = LNC_IPC_CLEVENT_FAILED;
		goto error_unlock;
	}

	/* Check current cluster node state. */
	state = ctx->lc_clstate;
	exstatus = LNC_CLSTAT_GETEXSTATUS(state);
	if (PFC_EXPECT_FALSE(exstatus != LNC_CLSTAT_EX_STABLE)) {
		if (exstatus == LNC_CLSTAT_EX_TRANS) {
			pfc_log_error("CLEVENT: State is in transition: "
				      "0x%x: type=%u", state, type);
			resp = LNC_IPC_CLEVENT_BUSY;
		}
		else {
			PFC_ASSERT(exstatus == LNC_CLSTAT_EX_ERROR);
			pfc_log_error("CLEVENT: Cluster is in error state: "
				      "0x%x: type=%u", state, type);
			resp = LNC_IPC_CLEVENT_ERRSTATE;
		}
		goto error_unlock;
	}

	newstate = LNC_CLSTAT_MKSTATE(type, STABLE);
	PFC_ASSERT(newstate == LNC_CLSTATE_ACT);
	if (PFC_EXPECT_FALSE(state == newstate)) {
		pfc_log_error("CLEVENT: State is not changed: 0x%x: type=%u",
			      state, type);
		resp = LNC_IPC_CLEVENT_UNCHANGED;
		goto error_unlock;
	}

	/* Start cluster state transition. */
	sysact = LNC_GETSYSACT(ctx);
	ctx->lc_clstate = LNC_CLSTAT_MKSTATE(type, TRANS);
	PFC_ASSERT(ctx->lc_clstate == LNC_CLSTATE_ACT_TRANS);
	PFC_ASSERT_INT(pfc_clock_get_realtime(&ctx->lc_clevent_start), 0);
	ctx->lc_clevent_end.tv_sec = 0;
	ctx->lc_clevent_end.tv_nsec = 0;
	pfc_log_info("CLEVENT: state 0x%x -> 0x%x", state, ctx->lc_clstate);
	LNC_UNLOCK(ctx);

	/* Send cluster state change event to daemons. */
	err = lnc_cluster_event(ctx, type, sysact, &limit);
	if (PFC_EXPECT_FALSE(err != 0)) {
		lnc_log_fatal("CLEVENT: State transition has failed: type=%u: "
			      "%s", type, strerror(err));
		resp = LNC_IPC_CLEVENT_FAILED;

		/* Keep error state. */
		LNC_LOCK(ctx);
		ctx->lc_clstate = LNC_CLSTAT_MKSTATE(type, ERROR);
		PFC_ASSERT(ctx->lc_clstate == LNC_CLSTATE_ACT_ERROR);
		PFC_ASSERT_INT(pfc_clock_get_realtime(&ctx->lc_clevent_end),
			       0);
		goto error_unlock;
	}

	/* Cluster node state has become stable. */
	LNC_LOCK(ctx);

	pfc_log_info("CLEVENT: state 0x%x -> 0x%x", ctx->lc_clstate, newstate);
	ctx->lc_clstate = newstate;
	PFC_ASSERT_INT(pfc_clock_get_realtime(&ctx->lc_clevent_end), 0);

	/* Check launcher status again. */
	if (PFC_EXPECT_FALSE(LNC_IS_FINI(ctx))) {
		pfc_log_error("CLEVENT: UNC daemon is going to exit.");
		resp = LNC_IPC_CLEVENT_FAILED;
		goto error_unlock;
	}

	ipc_service_exit_l(ctx);
	LNC_UNLOCK(ctx);

	return LNC_IPC_CLEVENT_OK;

error_unlock:
	ipc_service_exit_l(ctx);
	LNC_UNLOCK(ctx);

	return resp;

error:
	ipc_service_exit(ctx);

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_lncstat(pfc_ipcsrv_t *srv)
 *	Handle LNC_IPC_SVID_LNCSTAT service.
 *
 *	This service returns a bitwise OR-ed flag value which indicates
 *	status information about the launcher module.
 *
 * Calling/Exit State:
 *	An response code for CLEVENT service (LNC_IPC_CLEVENT_XXX) is returned.
 */
static pfc_ipcresp_t
ipc_lncstat(pfc_ipcsrv_t *srv)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_ipcresp_t	resp;

	LNC_LOCK(ctx);
	resp = LNCSTAT_STATE(ctx);
	LNC_UNLOCK(ctx);

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_status_all(pfc_ipcsrv_t *UNC_RESTRICT srv, lnc_ctx_t *UNC_RESTRICT ctx)
 *	Get status of all daemons.
 *	This is an internal function of ipc_status();
 *
 * Calling/Exit State:
 *	Upon successful completion, LNC_IPC_STATUS_OK is returned.
 *	Otherwise PFC_IPCRESP_FATAL is returned.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
static pfc_ipcresp_t
ipc_status_all(pfc_ipcsrv_t *UNC_RESTRICT srv, lnc_ctx_t *UNC_RESTRICT ctx)
{
	pfc_rbtree_t	*tree = &daemon_tree;
	pfc_rbnode_t	*node = NULL;
	pfc_ipcresp_t	resp = LNC_IPC_STATUS_OK;

	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		lnc_daemon_t	*ldp = LNC_DAEMON_NODE2PTR(node);
		lnc_proc_t	*lprp = ldp->ld_command;
		lncipc_dmstat_t	stat;
		int		err;

		LNC_PROC_LOCK(lprp);
		ipc_status_fill(ldp, lprp, LNC_DAEMON_NAME(ldp), &stat);
		LNC_PROC_UNLOCK(lprp);

		err = PFC_IPCSRV_OUTPUT_STRUCT(srv, lncipc_dmstat, &stat);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("ipc_status: %s: "
				      "Failed to output lncipc_dmstat: %s",
				      LNC_DAEMON_NAME(ldp), strerror(err));
			resp = PFC_IPCRESP_FATAL;
			break;
		}
	}

	return resp;
}

/*
 * static pid_t
 * ipc_getpid(pfc_ipcsrv_t *UNC_RESTRICT srv, const char *UNC_RESTRICT svname)
 *	Determine process ID of the IPC client associated with the given
 *	server session.
 *
 *	This function is used to determine process ID of the process launched
 *	by the launcher module. So the host address of the client must be
 *	LOCAL address.
 *
 *	`svname' is a symbolic name of the IPC service.
 *	It is used to record error message.
 *
 * Calling/Exit State:
 *	Upon successful completion, valid process ID is returned.
 *	Otherwise (pid_t)-1 is returned.
 */
static pid_t
ipc_getpid(pfc_ipcsrv_t *UNC_RESTRICT srv, const char *UNC_RESTRICT svname)
{
	pfc_ipccladdr_t	claddr;
	int		err, htype;

	/* Determine client address. */
	err = pfc_ipcsrv_getcladdr(srv, &claddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s: Failed to get client address: %s",
			      svname, strerror(err));

		return (pid_t)-1;
	}

	/* Ensure that the host address of the client is LOCAL. */
	htype = pfc_hostaddr_gettype(&claddr.cla_hostaddr);
	if (PFC_EXPECT_FALSE(htype != AF_UNIX)) {
		pfc_log_error("%s: Unexpected host address type: %u",
			      svname, htype);

		return (pid_t)-1;
	}

	return claddr.cla_pid;
}

/*
 * static pfc_ipcresp_t
 * ipc_status_byname(pfc_ipcsrv_t *UNC_RESTRICT srv,
 *		     lnc_ctx_t *UNC_RESTRICT ctx, uint32_t count)
 *	Get status of daemons specified by daemon names sent by the client.
 *	This is an internal function of ipc_status();
 *
 * Calling/Exit State:
 *	Upon successful completion, LNC_IPC_STATUS_OK is returned.
 *
 *	On error, an error message is set to IPC additional data, and
 *	LNC_IPC_STATUS_ERROR is returned.
 *
 *	PFC_IPCRESP_FATAL is returned if even an error message can not be set.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
static pfc_ipcresp_t
ipc_status_byname(pfc_ipcsrv_t *UNC_RESTRICT srv, lnc_ctx_t *UNC_RESTRICT ctx,
		  uint32_t count)
{
	lnc_daemon_t	**dmarray, **ldpp;
	pfc_ipcresp_t	resp;
	uint32_t	i;

	PFC_ASSERT(count != 0);

	/* Get daemon instances associated with the given names. */
	resp = ipc_status_getdaemons(srv, ctx, count, &dmarray);
	if (PFC_EXPECT_FALSE(resp != LNC_IPC_STATUS_OK)) {
		return resp;
	}

	/* Create output message. */
	for (ldpp = dmarray, i = 0; i < count; ldpp++, i++) {
		lnc_daemon_t	*ldp = *ldpp;
		lnc_proc_t	*lprp = ldp->ld_command;
		lncipc_dmstat_t	stat;
		int		err;

		LNC_PROC_LOCK(lprp);
		ipc_status_fill(ldp, lprp, LNC_DAEMON_NAME(ldp), &stat);
		LNC_PROC_UNLOCK(lprp);

		err = PFC_IPCSRV_OUTPUT_STRUCT(srv, lncipc_dmstat, &stat);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("ipc_status: %s: "
				      "Failed to output lncipc_dmstat: %s",
				      LNC_DAEMON_NAME(ldp), strerror(err));
			resp = PFC_IPCRESP_FATAL;
			goto out;
		}
	}

out:
	free(dmarray);

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_status_getdaemons(pfc_ipcsrv_t *UNC_RESTRICT srv,
 *			 lnc_ctx_t *UNC_RESTRICT ctx, uint32_t count,
 *			 lnc_daemon_t ***UNC_RESTRICT arrayp)
 *	Get daemon instances associated with daemon names sent by the client.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to lnc_daemon_t array is set
 *	to the buffer pointed by `arrayp', and LNC_IPC_STATUS_OK is returned.
 *	The caller must release the buffer set to `*arrayp' by calling free(3).
 *
 *	On error, an error message is set to IPC additional data, and
 *	LNC_IPC_STATUS_ERROR is returned.
 *
 *	PFC_IPCRESP_FATAL is returned if even an error message can not be set.
 */
static pfc_ipcresp_t
ipc_status_getdaemons(pfc_ipcsrv_t *UNC_RESTRICT srv,
		      lnc_ctx_t *UNC_RESTRICT ctx, uint32_t count,
		      lnc_daemon_t ***UNC_RESTRICT arrayp)
{
	lnc_daemon_t	**dmarray, **ldpp;
	pfc_rbtree_t	*tree = &daemon_tree;
	pfc_ipcresp_t	resp;
	uint32_t	i;
	int		err;

	/* Create a temporary daemon instance array. */
	dmarray = (lnc_daemon_t **)malloc(sizeof(*dmarray) * count);
	if (PFC_EXPECT_FALSE(dmarray == NULL)) {
		return ipc_status_error(srv, "Failed to allocate a temporary "
					"daemon array.");
	}

	for (ldpp = dmarray, i = 0; i < count; ldpp++, i++) {
		const char	*name;
		pfc_rbnode_t	*node;

		/* Fetch daemon name sent by the client. */
		err = pfc_ipcsrv_getarg_string(srv, i, &name);
		if (PFC_EXPECT_FALSE(err != 0)) {
			resp = ipc_status_error(srv, "Failed to fetch daemon "
						"name at index %u: %s",
						i, strerror(err));
			goto error;
		}

		/* Determine daemon instance associated with the given name. */
		node = pfc_rbtree_get(tree, name);
		if (PFC_EXPECT_FALSE(node == NULL)) {
			resp = ipc_status_error(srv, "Unknown daemon name: %s",
						name);
			goto error;
		}

		*ldpp = LNC_DAEMON_NODE2PTR(node);
	}

	*arrayp = dmarray;

	return LNC_IPC_STATUS_OK;

error:
	/* Suppress compiler warning. */
	*arrayp = NULL;

	free(dmarray);

	return resp;
}

/*
 * static void
 * ipc_status_fill(lnc_daemon_t *UNC_RESTRICT ldp,
 *		   lnc_proc_t *UNC_RESTRICT lprp,
 *		   const char *UNC_RESTRICT dname,
 *		   lncipc_dmstat_t *UNC_RESTRICT statp)
 *	Fill the given lncipc_dmstat_t buffer with the daemon status
 *	information about `ldp'.
 *
 * Calling/Exit State:
 *	This function must be called with holding the launcher lock and the
 *	daemon process lock in order.
 */
static void
ipc_status_fill(lnc_daemon_t *UNC_RESTRICT ldp, lnc_proc_t *UNC_RESTRICT lprp,
		const char *UNC_RESTRICT dname,
		lncipc_dmstat_t *UNC_RESTRICT statp)
{
	pfc_refptr_t	*rchannel;
	size_t		len;

	len = pfc_strlcpy_assert((char *)statp->ids_name, dname,
				 sizeof(statp->ids_name));
	for (len++; len < sizeof(statp->ids_name); len++) {
		statp->ids_name[len] = '\0';
	}

	/* Set an empty string if no IPC channel is provided. */
	rchannel = ldp->ld_channel;
	len =lnc_daemon_setstring(rchannel, (char *)statp->ids_channel,
				  sizeof(statp->ids_channel));
	for (len++ ; len < sizeof(statp->ids_channel); len++) {
		statp->ids_channel[len] = '\0';
	}

	statp->ids_type = ldp->ld_type;
	statp->ids_pid = lprp->lp_pid;
}

/*
 * static pfc_ipcresp_t
 * ipc_status_error(pfc_ipcsrv_t *UNC_RESTRICT srv,
 *		    const char *UNC_RESTRICT fmt, ...)
 *	Set an error message for LNC_IPC_SVID_STATUS service.
 *
 * Calling/Exit State:
 *	PFC_IPCRESP_FATAL is returned if it failed to set an error message.
 *	Otherwise LNC_IPC_STATUS_ERROR is returned.
 */
static pfc_ipcresp_t
ipc_status_error(pfc_ipcsrv_t *UNC_RESTRICT srv, const char *UNC_RESTRICT fmt,
		 ...)
{
	pfc_ipcresp_t	resp = LNC_IPC_STATUS_ERROR;
	char		emsg[128];
	va_list		ap;
	int		err;

	va_start(ap, fmt);
	vsnprintf(emsg, sizeof(emsg), fmt, ap);
	va_end(ap);

	err = pfc_ipcsrv_output_string(srv, emsg);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_status: %s", emsg);
		pfc_log_error("ipc_status: Failed to set an error message: %s",
			      strerror(err));
		resp = PFC_IPCRESP_FATAL;
		/* FALLTHROUGH */
	}

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_vstatus_all(pfc_ipcsrv_t *UNC_RESTRICT srv, lnc_ctx_t *UNC_RESTRICT ctx)
 *	Get verbose status of all daemons.
 *	This is an internal function of ipc_vstatus();
 *
 * Calling/Exit State:
 *	Upon successful completion, LNC_IPC_STATUS_OK is returned.
 *	Otherwise PFC_IPCRESP_FATAL is returned.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
static pfc_ipcresp_t
ipc_vstatus_all(pfc_ipcsrv_t *UNC_RESTRICT srv, lnc_ctx_t *UNC_RESTRICT ctx)
{
	pfc_rbtree_t	*tree = &daemon_tree;
	pfc_rbnode_t	*node = NULL;
	pfc_ipcresp_t	resp = LNC_IPC_STATUS_OK;

	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		lnc_daemon_t	*ldp = LNC_DAEMON_NODE2PTR(node);
		int		err;

		err = ipc_vstatus_output(srv, ctx, ldp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			resp = PFC_IPCRESP_FATAL;
			break;
		}
	}

	return resp;
}

/*
 * static pfc_ipcresp_t
 * ipc_vstatus_byname(pfc_ipcsrv_t *UNC_RESTRICT srv,
 *		      lnc_ctx_t *UNC_RESTRICT ctx, uint32_t count)
 *	Get verbose status of daemons specified by daemon names sent by the
 *	client.
 *	This is an internal function of ipc_vstatus();
 *
 * Calling/Exit State:
 *	Upon successful completion, LNC_IPC_STATUS_OK is returned.
 *
 *	On error, an error message is set to IPC additional data, and
 *	LNC_IPC_STATUS_ERROR is returned.
 *
 *	PFC_IPCRESP_FATAL is returned if even an error message can not be set.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
static pfc_ipcresp_t
ipc_vstatus_byname(pfc_ipcsrv_t *UNC_RESTRICT srv, lnc_ctx_t *UNC_RESTRICT ctx,
		  uint32_t count)
{
	lnc_daemon_t	**dmarray, **ldpp;
	pfc_ipcresp_t	resp;
	uint32_t	i;

	PFC_ASSERT(count != 0);

	/* Get daemon instances associated with the given names. */
	resp = ipc_status_getdaemons(srv, ctx, count, &dmarray);
	if (PFC_EXPECT_FALSE(resp != LNC_IPC_STATUS_OK)) {
		return resp;
	}

	/* Create output message. */
	for (ldpp = dmarray, i = 0; i < count; ldpp++, i++) {
		lnc_daemon_t	*ldp = *ldpp;
		int		err;

		err = ipc_vstatus_output(srv, ctx, ldp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			resp = PFC_IPCRESP_FATAL;
			break;
		}
	}

	free(dmarray);

	return resp;
}

/*
 * static int
 * ipc_vstatus_output(pfc_ipcsrv_t *UNC_RESTRICT srv,
 *		      lnc_ctx_t *UNC_RESTRICT ctx,
 *		      lnc_daemon_t *UNC_RESTRICT ldp)
 *	Construct verbose daemon status of the given daemon, and set it to
 *	output stream of the IPC server session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_vstatus_output(pfc_ipcsrv_t *UNC_RESTRICT srv, lnc_ctx_t *UNC_RESTRICT ctx,
		   lnc_daemon_t *UNC_RESTRICT ldp)
{
	pfc_conf_t	conf;
	pfc_cfblk_t	blk;
	pfc_refptr_t	*rpath;
	pfc_timespec_t	rstart;
	pfc_listm_t	argv;
	lnc_proc_t	*lprp = ldp->ld_command;
	lncipc_dmstat_t	stat;
	const char	*dname = LNC_DAEMON_NAME(ldp);
	const char	*desc;
	int		err, i, argc;

	LNC_PROC_LOCK(lprp);

	/* Output lncipc_dmstat_t. */
	ipc_status_fill(ldp, lprp, dname, &stat);
	err = PFC_IPCSRV_OUTPUT_STRUCT(srv, lncipc_dmstat, &stat);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to output lncipc_dmstat: %s",
			      dname, strerror(err));
		goto error;
	}

	if (lprp->lp_pid == 0) {
		/* Daemon is not running. */
		rstart.tv_sec = 0;
		rstart.tv_nsec = 0;
		argc = 0;
		argv = PFC_LISTM_INVALID;
	}
	else {
		err = pfc_clock_mono2real(&rstart, &ldp->ld_start_time);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("ipc_vstatus: %s: "
				      "Failed to convert start_time into "
				      "realtime clock: %s", dname,
				      strerror(err));
			goto error;
		}

		err = pfc_proc_getcmdline(lprp->lp_pid, &argv);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("ipc_vstatus: %s "
				      "Failed to get command line arguments: "
				      "pid=%u: %s", dname, lprp->lp_pid,
				      strerror(err));
			goto error;
		}

		argc = pfc_listm_get_size(argv);
	}

	/*
	 * Open daemon configuration file in order to get description about
	 * the daemon.
	 */
	rpath = pfc_refptr_sprintf("%s/%s.daemon", ctx->lc_confdir, dname);
	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to create configuration file path.",
			      dname);
		err = ENOMEM;
		goto error_argv;
	}

	err = pfc_ipcsrv_output_string(srv, pfc_refptr_string_value(rpath));
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to output configuration file path: %s",
			      dname, strerror(err));
		goto error_rpath;
	}

	err = pfc_conf_refopen(&conf, rpath, liblnc_getcfdef());
	pfc_refptr_put(rpath);
	rpath = NULL;

	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to open configuration file: %s",
			      dname, strerror(err));
		goto error_argv;
	}

	blk = pfc_conf_get_block(conf, "daemon");
	desc = pfc_conf_get_string(blk, "description", "<unavailable>");
	err = pfc_ipcsrv_output_string(srv, desc);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to output description: %s", dname,
			      strerror(err));
		pfc_conf_close(conf);
		goto error_argv;
	}

	pfc_conf_close(conf);

	/* Output contents of the stderr logfile. */
	err = ipc_vstatus_output_errlog(srv, lprp, dname);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_argv;
	}

	/* Output process start time. */
	err = pfc_ipcsrv_output_uint64(srv, rstart.tv_sec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to output process start time(sec): %s",
			      dname, strerror(err));
		goto error_argv;
	}

	err = pfc_ipcsrv_output_uint64(srv, rstart.tv_nsec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to output process start time(nsec): %s",
			      dname, strerror(err));
		goto error_argv;
	}

	/* Output the number of command line arguments. */
	err = pfc_ipcsrv_output_uint32(srv, argc);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: %s: Failed to output argc: %s",
			      dname, strerror(err));
		goto error_argv;
	}

	/* Output command line arguments. */
	for (i = 0; i < argc; i++) {
		pfc_refptr_t	*rstr;
		const char	*arg;

		PFC_ASSERT_INT(pfc_listm_getat(argv, i, (pfc_cptr_t *)&rstr),
			       0);
		arg = pfc_refptr_string_value(rstr);
		pfc_log_verbose("argv[%u] = %s", i, arg);
		err = pfc_ipcsrv_output_string(srv, arg);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("ipc_vstatus: %s: "
				      "Failed to output argv[%d]: %s",
				      dname, i, strerror(err));
			goto error_argv;
		}
	}

	LNC_PROC_UNLOCK(lprp);

	pfc_listm_destroy(argv);

	return 0;

error_rpath:
	if (rpath != NULL) {
		pfc_refptr_put(rpath);
	}

error_argv:
	if (argv != PFC_LISTM_INVALID) {
		pfc_listm_destroy(argv);
	}

error:
	LNC_PROC_UNLOCK(lprp);

	return err;
}

/*
 * static int
 * ipc_vstatus_output_errlog(pfc_ipcsrv_t *UNC_RESTRICT srv,
 *			     lnc_proc_t *UNC_RESTRICT lprp,
 *			     const char *UNC_RESTRICT dname)
 *	Output contents of the stderr logfile for the daemon specified by
 *	`lprp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_vstatus_output_errlog(pfc_ipcsrv_t *UNC_RESTRICT srv,
			  lnc_proc_t *UNC_RESTRICT lprp,
			  const char *UNC_RESTRICT dname)
{
	lnc_daemon_t	*ldp;
	lnc_stderr_t	*lsep = lprp->lp_stderr;
	pfc_refptr_t	*rpath;
	struct stat	sbuf;
	const char	*path;
	char		*ptr;
	int		fd = -1, err;

	if (lsep == NULL) {
		/* stderr logging is disabled. */
		rpath = NULL;
		goto unavailable;
	}

	ldp = lprp->lp_daemon;
	PFC_ASSERT(ldp->ld_logdir != NULL);
	rpath = pfc_refptr_sprintf("%s/%s", LNC_DAEMON_LOGDIR(ldp),
				   lsep->lse_fname);
	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to create stderr logfile path.", dname);

		return ENOMEM;
	}

	path = pfc_refptr_string_value(rpath);
	fd = pfc_open_cloexec(path, O_RDONLY);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		pfc_log_warn("ipc_vstatus: %s: open(%s) failed: %s", dname,
			     path, strerror(errno));
		goto unavailable;
	}

	if (PFC_EXPECT_FALSE(stat(path, &sbuf) != 0)) {
		pfc_log_warn("ipc_vstatus: %s: stat(%s) failed: %s", dname,
			     path, strerror(errno));
		goto unavailable;
	}

	if (sbuf.st_size == 0) {
		goto unavailable;
	}

	/* Map whole stderr logfile. */
	ptr = (char *)mmap(NULL, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (PFC_EXPECT_FALSE(ptr == (char *)MAP_FAILED)) {
		pfc_log_warn("ipc_vstatus: %s: mmap(%s) failed: %s", dname,
			     path, strerror(errno));
		goto unavailable;
	}

	(void)close(fd);
	fd = -1;
	err = pfc_ipcsrv_output_string(srv, ptr);
	PFC_ASSERT_INT(munmap(ptr, sbuf.st_size), 0);

	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: %s: "
			      "Failed to output stderr log: %s",
			      dname, strerror(err));
		/* FALLTHROUGH */
	}

	pfc_refptr_put(rpath);

	return err;

unavailable:
	if (fd != -1) {
		(void)close(fd);
	}
	if (rpath != NULL) {
		pfc_refptr_put(rpath);
	}

	/* Set NULL to the output stream. */
	err = pfc_ipcsrv_output_string(srv, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("ipc_vstatus: Failed to output NULL as stderr "
			      "output: %s", strerror(err));
		/* FALLTHROUGH */
	}

	return err;
}
