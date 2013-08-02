/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.c - UNC daemon internal event management.
 */

#include "launcher_impl.h"

/*
 * Priority of system start event handler.
 */
#define	LNC_EVPRI_START			UINT32_MAX	/* lowest priority */

/*
 * Priority of system shutdown event handler.
 */
#define	LNC_EVPRI_STOP			PFC_CONST_U(5)

/*
 * Internal prototypes.
 */
static void	event_start_handler(pfc_event_t event, pfc_ptr_t arg);
static void	event_shutdown_handler(pfc_event_t event, pfc_ptr_t arg);
static void	event_stop_task(void *arg);

/*
 * const char *
 * lncapi_event_getsource(void)
 *	Return a static string which represents event source to listen
 *	launcher module events.
 */
const char *
lncapi_event_getsource(void)
{
	return LNC_EVENT_SOURCE;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_event_init(lnc_ctx_t *ctx)
 *	Initialize launcher event.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_event_init(lnc_ctx_t *ctx)
{
	const char	*evsrc;
	pfc_evmask_t	mask;
	pfc_evhandler_t	*hdlrp;
	int		err;

	/* Register launcher module event source. */
	err = pfc_event_register(LNC_EVENT_SOURCE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to register event source: %s",
			      strerror(err));

		return err;
	}

	evsrc = pfc_event_global_source();
	if (PFC_EXPECT_FALSE(pfc_module_is_started())) {
		/* SYS_START event should be already delivered. */
		LNC_SETFLAG(ctx, LNC_CF_STARTED);
	}
	else {
		/* Register system start event handler. */
		pfc_event_mask_empty(&mask);
		PFC_ASSERT_INT(pfc_event_mask_add(&mask, PFC_EVTYPE_SYS_START),
			       0);
		err = pfc_event_add_handler(&LNC_EVHDLR_START(ctx), evsrc,
					    event_start_handler, ctx, &mask,
					    LNC_EVPRI_START);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to register start event handler"
				      ": %s", strerror(err));
			goto error;
		}
	}

	/* Register system shutdown event handler. */
	pfc_event_mask_empty(&mask);
	PFC_ASSERT_INT(pfc_event_mask_add(&mask, PFC_EVTYPE_SYS_STOP), 0);
	err = pfc_event_add_handler(&LNC_EVHDLR_STOP(ctx), evsrc,
				    event_shutdown_handler, ctx, &mask,
				    LNC_EVPRI_STOP);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to register shutdown event handler: %s",
			      strerror(err));
		goto error_handler;
	}

	return 0;

error_handler:
	for (hdlrp = ctx->lc_event_handler;
	     hdlrp < PFC_ARRAY_LIMIT(ctx->lc_event_handler); hdlrp++) {
		pfc_evhandler_t	hdlr = *hdlrp;

		if (hdlr == EVHANDLER_ID_INVALID) {
			continue;
		}

		*hdlrp = EVHANDLER_ID_INVALID;
		PFC_ASSERT_INT(pfc_event_remove_handler(hdlr, NULL), 0);
	}

error:
	PFC_ASSERT_INT(pfc_event_unregister(LNC_EVENT_SOURCE, NULL), 0);

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_event_fini(lnc_ctx_t *UNC_RESTRICT ctx,
 *		  const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Finalize launcher event.
 *
 *	`abstime' must be a pointer to pfc_timespec_t which represents deadline
 *	of event finalization.	
 */
int PFC_ATTR_HIDDEN
lnc_event_fini(lnc_ctx_t *UNC_RESTRICT ctx,
	       const pfc_timespec_t *UNC_RESTRICT abstime)
{
	pfc_evhandler_t	handlers[LNC_NR_EVENT_HANDLERS];
	pfc_timespec_t	ts, *timeout;
	uint32_t	i;
	int		err;

	LNC_LOCK(ctx);
	for (i = 0; i < PFC_ARRAY_CAPACITY(handlers); i++) {
		handlers[i] = ctx->lc_event_handler[i];
		ctx->lc_event_handler[i] = EVHANDLER_ID_INVALID;
	}
	LNC_UNLOCK(ctx);

	timeout = (abstime == NULL) ? NULL : &ts;

	/* Remove event handlers. */
	for (i = 0; i < PFC_ARRAY_CAPACITY(handlers); i++) {
		pfc_evhandler_t	hdlr = handlers[i];

		if (hdlr == EVHANDLER_ID_INVALID) {
			continue;
		}

		if (timeout != NULL) {
			lnc_clock_remains(timeout, abstime);
		}

		err = pfc_event_remove_handler(hdlr, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to remove event handler[%u]: %s",
				      i, strerror(err));

			return err;
		}
	}

	if (timeout != NULL) {
		lnc_clock_remains(timeout, abstime);
	}

	/* Unregister event source. */
	err = pfc_event_unregister(LNC_EVENT_SOURCE, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to unregister event source: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_event_post(pfc_evtype_t type)
 *	Post an launcher event which has no additional data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
lnc_event_post(pfc_evtype_t type)
{
	pfc_event_t	event;
	int		err;

	err = PFC_EVENT_CREATE(&event, type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create an event: type=%u: %s", type,
			      strerror(err));

		return err;
	}

	/* Enable event delivery logging. */
	pfc_event_enable_log(event, PFC_TRUE);

	err = pfc_event_post(LNC_EVENT_SOURCE, event);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to post an event: type=%u: %s", type,
			      strerror(err));
		/* FALLTHROUGH */
	}

	return err;
}

/*
 * static void
 * event_start_handler(pfc_event_t event, pfc_ptr_t arg)
 *	Invoked when the system service has been started.
 */
static void
event_start_handler(pfc_event_t event, pfc_ptr_t arg)
{
	lnc_ctx_t	*ctx = (lnc_ctx_t *)arg;
	pfc_evhandler_t	hdlr;
	pfc_timespec_t	timeout = {1, 0};

	PFC_ASSERT(ctx == &launcher_ctx);

	LNC_LOCK(ctx);
	LNC_SETFLAG_L(ctx, LNC_CF_STARTED);
	hdlr = LNC_EVHDLR_START(ctx);
	LNC_EVHDLR_START(ctx) = EVHANDLER_ID_INVALID;
	LNC_UNLOCK(ctx);

	if (PFC_EXPECT_TRUE(hdlr != EVHANDLER_ID_INVALID)) {
		/*
		 * Remove this event handler.
		 * This call should never block the calling thread.
		 */
		PFC_ASSERT_INT(pfc_event_remove_handler(hdlr, &timeout), 0);
	}
}

/*
 * static void
 * event_shutdown_handler(pfc_event_t event, pfc_ptr_t arg)
 *	Invoked when the system shutdown sequence has been started.
 */
static void
event_shutdown_handler(pfc_event_t event, pfc_ptr_t arg)
{
	lnc_ctx_t	*ctx = (lnc_ctx_t *)arg;
	pfc_task_t	task;
	int		err;

	PFC_ASSERT(ctx == &launcher_ctx);

	/* Finalize the launcher module. */
	pfc_log_info("Finalize the launcher due to shutdown.");
	lnc_finalize(ctx);

	/* Stop low priority daemons on a task queue. */
	LNC_LOCK(ctx);

	err = pfc_taskq_dispatch(ctx->lc_taskq, event_stop_task, ctx, 0,
				 &task);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_warn("Failed to dispatch a task to stop low priority "
			     "daemons: %s", strerror(err));
	}
	else {
		ctx->lc_flags |= LNC_CF_STASK_POSTED;
	}
	
	LNC_UNLOCK(ctx);
}

/*
 * static void
 * event_stop_task(void *arg)
 *	Stop daemons which are assigned "stop_order" value less than
 *	LNC_ORDER_UNCD.
 *
 *	This function is called on a task queue thread.
 */
static void
event_stop_task(void *arg)
{
	lnc_ctx_t	*ctx = (lnc_ctx_t *)arg;

	PFC_ASSERT(ctx == &launcher_ctx);

	lnc_daemon_stop_low();
	LNC_SETFLAG(ctx, LNC_CF_STASK_DONE);
}
