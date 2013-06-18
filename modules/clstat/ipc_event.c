/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ipc_event.c - IPC event handling.
 */

#include <unc/liblauncher.h>
#include <unc/lnc_ipc.h>
#include <pfc/ipc_pfcd.h>
#include "clstat_impl.h"

/*
 * Priority value for IPC event handlers.
 */
#define	CLST_IPCEVENT_PRI	PFC_CONST_U(0)		/* highest priority */

/*
 * Derive the number of milliseconds (UINT32) from the given configuration
 * block handle, and convert it into pfc_timespec_t value.
 */
#define	CLSTAT_GETCONF_TIMESPEC(options, name, u32, tsp)	\
	do {							\
		(u32) = CLSTAT_CONF_GET(options, uint32, name);	\
		pfc_log_verbose("%s=%u", #name, (u32));		\
		pfc_clock_msec2time((tsp), (u32));		\
	} while (0)

clst_ipcctx_t	ipc_ctx PFC_ATTR_HIDDEN;

/*
 * How long, in seconds, we should wait for clstat_fini() to be called
 * after PFCD_IPCEVENT_SYS_STOP event.
 */
#define	CLST_STOP_FINI_TIMEOUT	PFC_CONST_U(30)		/* 30 seconds */

/*
 * Internal prototypes.
 */
static void	clst_ipc_chstate(pfc_ipcevent_t *event, pfc_ptr_t arg);
static void	clst_ipc_clevent(pfc_ipcevent_t *event, pfc_ptr_t arg);
static void	clst_ipc_sys_stop(pfc_ipcevent_t *event, pfc_ptr_t arg);
static void	clst_ipc_clevent_ack(clst_ipcctx_t *ctx, pfc_bool_t result);
static void	*clst_ipc_clevent_thread(void *arg);
static void	*clst_ipc_fini_timer(void *arg);

/*
 * int PFC_ATTR_HIDDEN
 * clst_ipc_init(void)
 *	Initialize IPC event handlers to catch cluster state change events
 *	raised by the UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
clst_ipc_init(void)
{
	clst_ipcctx_t	*ctx = &ipc_ctx;
	pfc_ipcevattr_t	attr, *attrp = NULL;
	pfc_ipcevmask_t	mask;
	pfc_cfblk_t	options;
	pfc_timespec_t	timeout, abstime;
	uint32_t	init_timeout, ack_timeout;
	int		err;

	ctx->cic_hdlr_clevent = PFC_IPCEVHDLR_INVALID;
	ctx->cic_hdlr_chstate = PFC_IPCEVHDLR_INVALID;
	ctx->cic_hdlr_stop = PFC_IPCEVHDLR_INVALID;
	ctx->cic_flags = 0;
	ctx->cic_thread = PFC_THREAD_INVALID;
	ctx->cic_stopthread = PFC_THREAD_INVALID;
	ctx->cic_limit.tv_sec = 0;
	ctx->cic_limit.tv_nsec = 0;
	ctx->cic_type = CLSTAT_EVTYPE_INVALID;
	ctx->cic_sysact = PFC_FALSE;
	ctx->cic_trans_start.tv_sec = 0;
	ctx->cic_trans_start.tv_nsec = 0;
	ctx->cic_trans_end.tv_sec = 0;
	ctx->cic_trans_end.tv_nsec = 0;
	clstat_ipclist_init(&ctx->cic_sessions);

	if (CLST_ON_UNCD()) {
		/* Nothing to do. */
		return 0;
	}

	pfc_log_info("Set up IPC event handler.");

	/* Fetch module configurations. */
	options = pfc_module_conf_getblock("options");
	CLSTAT_GETCONF_TIMESPEC(options, init_timeout, init_timeout, &timeout);
	CLSTAT_GETCONF_TIMESPEC(options, ack_timeout, ack_timeout,
				&ctx->cic_ack_timeout);

	/* Create IPC event handler attributes. */
	err = pfc_ipcevent_attr_init(&attr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create IPC event attributes: %s",
			      strerror(err));

		return err;
	}

	attrp = &attr;
	PFC_ASSERT_INT(pfc_ipcevent_attr_setpriority(attrp, CLST_IPCEVENT_PRI),
		       0);
	PFC_ASSERT_INT(pfc_ipcevent_attr_setarg(attrp, ctx), 0);

	/*
	 * Register IPC event handler to catch IPC channel state change events.
	 */
	pfc_ipcevent_mask_empty(&mask);
	PFC_ASSERT_INT(pfc_ipcevent_mask_add(&mask, PFC_IPCCHSTATE_UP), 0);
	PFC_ASSERT_INT(pfc_ipcevent_mask_add(&mask, PFC_IPCCHSTATE_DOWN), 0);
	PFC_ASSERT_INT(pfc_ipcevent_mask_add(&mask, PFC_IPCCHSTATE_NOTIFY), 0);
	err = pfc_ipcevent_attr_addtarget(attrp, NULL, &mask);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to add channel state change event to "
			      "target: %s", strerror(err));
		goto error;
	}

	err = pfc_module_ipcevent_add_handler(&ctx->cic_hdlr_chstate,
					      LIBLNC_UNCD_NAME,
					      clst_ipc_chstate, attrp,
					      "clstat:CHSTATE");
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to add channel state handler: %s",
			      strerror(err));
		goto error;
	}

	/* Register IPC event handler to receive LNC_IPCEVENT_CLACT event. */
	PFC_ASSERT_INT(pfc_ipcevent_attr_resettarget(attrp), 0);
	pfc_ipcevent_mask_empty(&mask);
	PFC_ASSERT_INT(pfc_ipcevent_mask_add(&mask, LNC_IPCEVENT_CLEVENT), 0);
	err = pfc_ipcevent_attr_addtarget(attrp, LNC_IPC_SERVICE, &mask);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to add CLEVENT event to target: %s",
			      strerror(err));
		goto error_chstate;
	}

	err = pfc_module_ipcevent_add_handler(&ctx->cic_hdlr_clevent,
					      LIBLNC_UNCD_NAME,
					      clst_ipc_clevent, attrp,
					      "clstat:CLEVENT");
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to add CLEVENT handler: %s",
			      strerror(err));
		goto error_chstate;
	}

	/* Register IPC event handler to detect parent uncd shutdown. */
	PFC_ASSERT_INT(pfc_ipcevent_attr_resettarget(attrp), 0);
	pfc_ipcevent_mask_empty(&mask);
	PFC_ASSERT_INT(pfc_ipcevent_mask_add(&mask, PFCD_IPCEVENT_SYS_STOP),
		       0);
	err = pfc_ipcevent_attr_addtarget(attrp, PFCD_IPC_SERVICE, &mask);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to add SYS_STOP event to target: %s",
			      strerror(err));
		goto error_clevent;
	}

	err = pfc_module_ipcevent_add_handler(&ctx->cic_hdlr_stop,
					      LIBLNC_UNCD_NAME,
					      clst_ipc_sys_stop, attrp,
					      "clstat:SYS_STOP");
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to add SYS_STOP handler: %s",
			      strerror(err));
		goto error_clevent;
	}

	pfc_ipcevent_attr_destroy(attrp);
	attrp = NULL;

	/* Raise an channel state notification event. */
	err = pfc_ipcevent_notifystate(ctx->cic_hdlr_chstate);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to raise an IPC channel state "
			      "notification event: %s", strerror(err));
		goto error_sys_stop;
	}

	/* Wait for the listener session to be established. */
	PFC_ASSERT_INT(pfc_clock_abstime(&abstime, &timeout), 0);

	CLST_IPC_LOCK(ctx);
	for (;;) {
		uint32_t	flags = ctx->cic_flags;

		if (PFC_EXPECT_TRUE(flags & CLIPCF_UP)) {
			err = 0;
			break;
		}
		if (PFC_EXPECT_FALSE(flags & CLIPCF_CANCELED)) {
			err = ECANCELED;
			break;
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			pfc_log_error("Event listener session was not "
				      "established within %u milliseconds.",
				      init_timeout);
			break;
		}

		err = CLST_IPC_TIMEDWAIT_ABS(ctx, &abstime);
	}
	CLST_IPC_UNLOCK(ctx);

	return err;

error_sys_stop:
	PFC_ASSERT_INT(pfc_module_ipcevent_remove_handler
		       (ctx->cic_hdlr_stop), 0);
	ctx->cic_hdlr_stop = PFC_IPCEVHDLR_INVALID;

error_clevent:
	PFC_ASSERT_INT(pfc_module_ipcevent_remove_handler
		       (ctx->cic_hdlr_clevent), 0);
	ctx->cic_hdlr_clevent = PFC_IPCEVHDLR_INVALID;

error_chstate:
	PFC_ASSERT_INT(pfc_module_ipcevent_remove_handler
		       (ctx->cic_hdlr_chstate), 0);
	ctx->cic_hdlr_chstate = PFC_IPCEVHDLR_INVALID;

error:
	if (attrp != NULL) {
		pfc_ipcevent_attr_destroy(attrp);
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * clst_ipc_fini(const pfc_timespec_t *abstime)
 *	Finalize IPC event handlers.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
clst_ipc_fini(const pfc_timespec_t *abstime)
{
	clst_ipcctx_t	*ctx = &ipc_ctx;
	clst_ipclist_t	*ilp = &ctx->cic_sessions;
	pfc_thread_t	t, st;
	pfc_timespec_t	remains;
	int		err;

	/*
	 * Remarks:
	 *	IPC event handlers must be already removed by the module
	 *	management layer.
	 */

	/* Cancel all IPC service requests. */
	err = clstat_ipclist_fini(ilp, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to disable IPC client session list: %s",
			      strerror(err));

		return err;
	}

	/* Join CLEVENT and SYS_STOP timer thread. */
	CLST_IPC_LOCK(ctx);
	CLST_IPC_SETFLAG(ctx, STOP);
	t = ctx->cic_thread;
	st = ctx->cic_stopthread;
	ctx->cic_thread = PFC_THREAD_INVALID;
	ctx->cic_stopthread = PFC_THREAD_INVALID;
	CLST_IPC_UNLOCK(ctx);

	if (t != PFC_THREAD_INVALID) {
		clst_clock_remains(&remains, abstime);
		err = pfc_thread_timedjoin(t, NULL, &remains);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to join CLEVENT thread: %s",
				      strerror(err));
			/* FALLTHROUGH */
		}
	}

	if (st != PFC_THREAD_INVALID) {
		clst_clock_remains(&remains, abstime);
		err = pfc_thread_timedjoin(st, NULL, &remains);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to join SYS_STOP timer thread: "
				      "%s", strerror(err));
			/* FALLTHROUGH */
		}
	}

	return err;
}

/*
 * static void
 * clst_ipc_chstate(pfc_ipcevent_t *event, pfc_ptr_t arg)
 *	Handle IPC channel state events.
 */
static void
clst_ipc_chstate(pfc_ipcevent_t *event, pfc_ptr_t arg)
{
	clst_ipcctx_t	*ctx = (clst_ipcctx_t *)arg;
	pfc_ipcevtype_t	evtype;

	PFC_ASSERT(ctx == &ipc_ctx);
	PFC_ASSERT(strcmp(pfc_ipcevent_getchannelname(event), LIBLNC_UNCD_NAME)
		   == 0);

	evtype = pfc_ipcevent_gettype(event);

	CLST_IPC_LOCK(ctx);

	if (PFC_EXPECT_FALSE(ctx->cic_flags & CLIPCF_CANCELED)) {
		/*
		 * There is nothing to do because the UNC daemon enters
		 * shutdown sequence, or the event listener session is
		 * in fatal state.
		 */
		pfc_log_notice("Ignore channel state event: type=%u", evtype);
		goto out;
	}

	if (evtype == PFC_IPCCHSTATE_NOTIFY) {
		pfc_ipcsess_t	*sess;
		uint32_t	state;
		int		err;

		/* Determine IPC channel state. */
		PFC_ASSERT(evtype == PFC_IPCCHSTATE_NOTIFY);
		sess = pfc_ipcevent_getsess(event);
		err = pfc_ipcclnt_getres_uint32(sess, 0, &state);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should never happen. */
			pfc_log_fatal("Failed to get channel state from "
				      "CHSTATE event: %s", strerror(err));
			CLST_IPC_SETFLAG(ctx, FATAL);
			
			goto out;
		}

		if (PFC_EXPECT_FALSE(state == PFC_IPCCHNOTIFY_DOWN)) {
			pfc_log_notice("Event listener session is not yet "
				       "established.");
			goto out;
		}

		PFC_ASSERT(state == PFC_IPCCHNOTIFY_UP);
	}
	else if (PFC_EXPECT_FALSE(evtype == PFC_IPCCHSTATE_DOWN)) {
		/*
		 * Parent process has died.
		 * This is fatal error.
		 */
		pfc_log_fatal("IPC event listener session has been destroyed: "
			      "parent=%u", uncd_pid);
		CLST_IPC_SETFLAG(ctx, DOWN);
		goto out;
	}
	else {
		PFC_ASSERT(evtype == PFC_IPCCHSTATE_UP);
	}

	if ((ctx->cic_flags & CLIPCF_UP) == 0) {
		pfc_log_info("IPC event listener session has been established:"
			     " parent=%u", uncd_pid);
		CLST_IPC_SETFLAG(ctx, UP);
	}

out:
	CLST_IPC_UNLOCK(ctx);
}

/*
 * static void
 * clst_ipc_clevent(pfc_ipcevent_t *event, pfc_ptr_t arg)
 *	Handle LNC_IPCEVENT_CLEVENT event.
 *
 *	This handler broadcasts cluster state change events to registered
 *	handlers.
 */
static void
clst_ipc_clevent(pfc_ipcevent_t *event, pfc_ptr_t arg)
{
	clst_ipcctx_t	*ctx = (clst_ipcctx_t *)arg;
	pfc_ipcsess_t	*sess;
	uint8_t		type, sysact;
	uint64_t	sec, nsec;
	int		err;

	PFC_ASSERT(ctx == &ipc_ctx);

	/* Determine event type. */
	sess = pfc_ipcevent_getsess(event);
	err = pfc_ipcclnt_getres_uint8(sess, 0, &type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("CLEVENT: Failed to get event type: %s",
			      strerror(err));
		goto error;
	}

	/* Determine SYSACT flag. */
	err = pfc_ipcclnt_getres_uint8(sess, 1, &sysact);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("CLEVENT: Failed to get SYSACT flag: %s",
			      strerror(err));
		goto error;
	}

	PFC_ASSERT(sysact == PFC_TRUE || sysact == PFC_FALSE);

	/* Determine deadline of state transition. */
	err = pfc_ipcclnt_getres_uint64(sess, 2, &sec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("CLEVENT: Failed to get deadline(sec): %s",
			      strerror(err));
		goto error;
	}

	err = pfc_ipcclnt_getres_uint64(sess, 3, &nsec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("CLEVENT: Failed to get deadline(nsec): %s",
			      strerror(err));
		goto error;
	}

	CLST_IPC_LOCK(ctx);

	if (PFC_EXPECT_FALSE(CLST_IPC_IS_FINI(ctx))) {
		pfc_log_error("CLEVENT: Module is now being unloaded.");
		goto error_unlock;
	}

	if (PFC_EXPECT_FALSE(ctx->cic_thread != PFC_THREAD_INVALID)) {
		pfc_log_error("CLEVENT: Cluster state is in transition.");
		goto error_unlock;
	}

	ctx->cic_type = type;
	ctx->cic_sysact = (pfc_bool_t)sysact;
	ctx->cic_limit.tv_sec = (pfc_long_t)sec;
	ctx->cic_limit.tv_nsec = (pfc_long_t)nsec;
	clst_ipc_mark_trans_start(ctx);

	/* Raise a cluster state change event in background. */
	err = pfc_thread_create(&ctx->cic_thread, clst_ipc_clevent_thread,
				ctx, 0);
	if (PFC_EXPECT_TRUE(err == 0)) {
		CLST_IPC_UNLOCK(ctx);

		return;
	}

	clst_ipc_mark_trans_end(ctx);
	pfc_log_error("CLEVENT: Failed to create a thread: %s", strerror(err));

error_unlock:
	CLST_IPC_UNLOCK(ctx);

error:
	clst_ipc_clevent_ack(ctx, PFC_FALSE);
}

/*
 * static void
 * clst_ipc_sys_stop(pfc_ipcevent_t *event, pfc_ptr_t arg)
 *	Handle PFCD_IPCEVENT_SYS_STOP event.
 *
 *	This handler is invoked when the parent UNC daemon enters the shutdown
 *	sequence.
 */
static void
clst_ipc_sys_stop(pfc_ipcevent_t *event, pfc_ptr_t arg)
{
	clst_ipcctx_t	*ctx = (clst_ipcctx_t *)arg;
	pfc_ipcsess_t	*sess = pfc_ipcevent_getsess(event);
	uint8_t		status;
	int		err;

	err = pfc_ipcclnt_getres_uint8(sess, 0, &status);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		pfc_log_warn("Failed to get status in SYS_STOP IPC event: %s",
			     strerror(err));
	}
	else {
		pfc_log_info("SYS_STOP IPC event has been received: "
			     "status=%s", (status) ? "normal" : "fatal");
	}

	/* Stop IPC event listener session monitoring. */
	clst_ipc_stop_listener(ctx);

	CLST_IPC_LOCK(ctx);

	if (!CLST_IPC_IS_FINI(ctx)) {
		/*
		 * Create a timer thread which waits for the call of
		 * clstat_fini().
		 */
		PFC_ASSERT(ctx->cic_stopthread == PFC_THREAD_INVALID);
		err = pfc_thread_create(&ctx->cic_stopthread,
					clst_ipc_fini_timer, ctx, 0);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_fatal("Failed to create SYS_STOP timer thread"
				      ": %s", strerror(err));
			/* FALLTHROUGH */
		}
	}

	CLST_IPC_UNLOCK(ctx);
}

/*
 * static void
 * clst_ipc_clevent_ack(clst_ipcctx_t *ctx, pfc_bool_t result)
 *	Send acknowledgement of CLEVENT to the UNC daemon.
 *
 * Remarks:
 *	Any error causes fatal error.
 */
static void
clst_ipc_clevent_ack(clst_ipcctx_t *ctx, pfc_bool_t result)
{
	clst_ipclist_t	*ilp = &ctx->cic_sessions;
	clst_ipcsess_t	*csess;
	pfc_ipcsess_t	*sess;
	pfc_ipcresp_t	resp;
	int		err;

	err = clstat_sess_create(ilp, &csess, LIBLNC_UNCD_NAME,
				 LNC_IPC_SERVICE, LNC_IPC_SVID_CLEVACK);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ECANCELED) {
			pfc_log_fatal("Failed to create IPC client session "
				      "for CLEVACK service: %s",
				      strerror(err));
		}

		return;
	}

	sess = csess->cis_sess;
	err = pfc_ipcclnt_sess_settimeout(sess, &ctx->cic_ack_timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to set timeout for CLEVACK: %s",
			      strerror(err));
		goto out;
	}

	err = pfc_ipcclnt_output_uint8(sess, result);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to set CLEVACK result: %s",
			      strerror(err));
		goto out;
	}

	err = pfc_ipcclnt_sess_invoke(sess, &resp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to invoke CLEVACK service: %s",
			      strerror(err));
		goto out;
	}

	if (PFC_EXPECT_FALSE(resp != 0)) {
		pfc_log_fatal("Unexpected response of CLEVACK service: %d",
			      resp);
		/* FALLTHROUGH */
	}

	pfc_log_info("CLEVACK: result=%u", result);

out:
	clstat_sess_destroy(ilp, csess);
}

/*
 * static void *
 * clst_ipc_clevent_thread(void *arg)
 *	Start routine of a thread which posts cluster state change event.
 */
static void *
clst_ipc_clevent_thread(void *arg)
{
	clst_ipcctx_t	*ctx = (clst_ipcctx_t *)arg;
	pfc_bool_t	result;
	int		err;

	PFC_ASSERT(ctx == &ipc_ctx);

	/* Raise a cluster state change event. */
	err = clstat_raise(ctx->cic_type, ctx->cic_sysact, &ctx->cic_limit);
	result = (PFC_EXPECT_TRUE(err == 0)) ? PFC_TRUE : PFC_FALSE;

	/* Send acknowledgement. */
	clst_ipc_clevent_ack(ctx, result);

	CLST_IPC_LOCK(ctx);

	/*
	 * If clst_ipc_fini() is already called, this thread should be joined
	 * by clst_ipc_fini() to ensure no thread refers clstat module text.
	 */
	if (!CLST_IPC_IS_FINI(ctx)) {
		pfc_thread_t	t = ctx->cic_thread;

		/* Detach myself. */
		PFC_ASSERT(t == pfc_thread_self());
		PFC_ASSERT_INT(pfc_thread_detach(t), 0);
		ctx->cic_thread = PFC_THREAD_INVALID;
	}

	CLST_IPC_UNLOCK(ctx);

	return NULL;
}

/*
 * static void *
 * clst_ipc_fini_timer(void *arg)
 *	Start routine of a thread which waits for the call of clstat_fini().
 */
static void *
clst_ipc_fini_timer(void *arg)
{
	clst_ipcctx_t	*ctx = (clst_ipcctx_t *)arg;
	pfc_timespec_t	abstime;
	int		err;

	PFC_ASSERT(ctx == &ipc_ctx);

	pfc_log_verbose("SYS_STOP timer thread has been started.");

	PFC_ASSERT_INT(pfc_clock_gettime(&abstime), 0);
	abstime.tv_sec += CLST_STOP_FINI_TIMEOUT;

	err = 0;
	CLST_IPC_LOCK(ctx);

	while (!CLST_IPC_IS_FINI(ctx)) {
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			pfc_log_fatal("fini() was not called after SYS_STOP "
				      "IPC event within %u seconds.",
				      CLST_STOP_FINI_TIMEOUT);
			break;
		}

		err = CLST_IPC_TIMEDWAIT_ABS(ctx, &abstime);
	}

	CLST_IPC_UNLOCK(ctx);

	pfc_log_verbose("SYS_STOP timer thread has been terminated.");

	return NULL;
}
