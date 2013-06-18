/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.c - Internal event management.
 */

#include <pfc/event.h>
#include <pfc/synch.h>
#include <pfc/util.h>
#include "clstat_impl.h"
#include "clstat_launcher.h"

/*
 * Event source name of clstat event.
 */
#define	CLST_EVENT_SOURCE		PFC_MODULE_THIS_NAME

/*
 * Number of threads in the clstat event source.
 */
#define	CLST_EVENT_NTHREADS		PFC_CONST_U(1)	/* single-threaded */

/*
 * Priority of cluster state change event handler.
 */
#define	CLST_EVPRI_CLEVENT		UINT32_MAX	/* lowest priority */

/*
 * Priority of system shutdown event handler.
 */
#define	CLST_EVPRI_STOP			PFC_CONST_U(10)

/*
 * Number of event handlers to be registered.
 */
#define	CLST_NR_EVENT_HANDLERS		PFC_CONST_U(2)

/*
 * Internal event handlers.
 */
static pfc_evhandler_t	clevent_handlers[CLST_NR_EVENT_HANDLERS];

#define	CLST_EVHDLR_SYNC		(clevent_handlers[0])
#define	CLST_EVHDLR_STOP		(clevent_handlers[1])

/*
 * Private data of cluster state change event.
 */
typedef struct {
	pfc_timespec_t	e_deadline;		/* deadline of state change */
	pfc_bool_t	e_sysact;		/* SYSACT flag */
} clst_event_t;

/*
 * Context to synchronize cluster state change event.
 */
typedef struct {
	pfc_mutex_t	es_mutex;		/* mutex */
	pfc_cond_t	es_cond;		/* condition variable */
	uint32_t	es_flags;		/* flags */
} clst_evsync_t;

/*
 * Flags for es_flags.
 */
#define	EVSYNC_BUSY	PFC_CONST_U(0x1)	/* synchronizing */
#define	EVSYNC_FINI	PFC_CONST_U(0x2)	/* fini() was called */

static clst_evsync_t	clevent_sync_ctx;

#define	CLST_EVSYNC_LOCK(esp)		pfc_mutex_lock(&(esp)->es_mutex)
#define	CLST_EVSYNC_UNLOCK(esp)		pfc_mutex_unlock(&(esp)->es_mutex)

#define	CLST_EVSYNC_SIGNAL(esp)		pfc_cond_signal(&(esp)->es_cond)
#define	CLST_EVSYNC_TIMEDWAIT_ABS(esp, abstime)				\
	pfc_cond_timedwait_abs(&(esp)->es_cond, &(esp)->es_mutex, (abstime))

/*
 * Internal prototypes.
 */
static void	clevent_sentinel(pfc_event_t event, pfc_ptr_t arg);
static void	clevent_shutdown(pfc_event_t event, pfc_ptr_t arg);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * clevent_sync_fini(void)
 *	Finalize cluster event synchronization.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
clevent_sync_fini(void)
{
	clst_evsync_t	*esp = &clevent_sync_ctx;

	/* Wake up a thread blocked by clevent_sync_ctx. */
	CLST_EVSYNC_LOCK(esp);
	if ((esp->es_flags & EVSYNC_FINI) == 0) {
		pfc_log_info("Finalize cluster event synchronization.");
		esp->es_flags |= EVSYNC_FINI;
		CLST_EVSYNC_SIGNAL(esp);
	}
	CLST_EVSYNC_UNLOCK(esp);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * clevent_clock2str(char *UNC_RESTRICT buf, size_t bufsize,
 *		     const pfc_timespec_t *UNC_RESTRICT real)
 *	Convert realtime system clock into string.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
clevent_clock2str(char *UNC_RESTRICT buf, size_t bufsize,
		  const pfc_timespec_t *UNC_RESTRICT real)
{
	int	err;

	err = pfc_time_clock_asctime(real, buf, bufsize);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		snprintf(buf, bufsize, "real:%lu.%lu",
			 real->tv_sec, real->tv_nsec);
	}
}

/*
 * const char *
 * clstat_event_getsource(void)
 *	Return a static string which represents event source to listen
 *	clstat module events.
 */
const char *
clstat_event_getsource(void)
{
	return CLST_EVENT_SOURCE;
}

/*
 * pfc_bool_t
 * clstat_event_isactive(pfc_event_t event)
 *	Determine whether the cluster system is active or not.
 *
 *	`event' must be a cluster state change event delivered to the event
 *	source returned by clstat_event_getsource(). Otherwise this function
 *	results in undefined behavior.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the cluster system is active.
 *	PFC_FALSE is returned if the cluster system is not yet active.
 */
pfc_bool_t
clstat_event_isactive(pfc_event_t event)
{
	clst_event_t	*cep;

	if (PFC_EXPECT_FALSE(event == PFC_EVENT_INVALID)) {
		return PFC_FALSE;
	}

	cep = PFC_EVENT_DATA(event, clst_event_t *);

	return cep->e_sysact;
}

/*
 * int
 * clstat_event_getdeadline(pfc_event_t event, pfc_timespec_t *tsp)
 *	Get deadline of cluster state change event handling.
 *
 *	`event' must be a cluster state change event delivered to the event
 *	source returned by clstat_event_getsource(). Otherwise this function
 *	results in undefined behavior.
 *
 * Calling/Exit State:
 *	Upon successful completion, deadline of cluster state change event
 *	handling is set to the buffer pointed by `tsp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
clstat_event_getdeadline(pfc_event_t event, pfc_timespec_t *tsp)
{
	clst_event_t	*cep;

	if (PFC_EXPECT_FALSE(event == PFC_EVENT_INVALID || tsp == NULL)) {
		return EINVAL;
	}

	cep = PFC_EVENT_DATA(event, clst_event_t *);
	*tsp = cep->e_deadline;

	return 0;
}

/*
 * int
 * clstat_raise(uint8_t type, pfc_bool_t sysact, const pfc_timespec_t *abstime)
 *	Raise a cluster state change event.
 *
 *	`type' specifies cluster event type defined as CLSTAT_EVTYPE_XXX.
 *	`sysact' determines whether the cluster system is active or not.
 *	PFC_TRUE must be specified if active.
 *
 *	This function blocks the calling thread until the event is delivered
 *	to all handlers, or the system absolute time exceeds `abstime'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
clstat_raise(uint8_t type, pfc_bool_t sysact, const pfc_timespec_t *abstime)
{
	clst_ipcctx_t	*ctx = &ipc_ctx;
	clst_evsync_t	*esp = &clevent_sync_ctx;
	clst_event_t	*cep;
	pfc_event_t	event;
	uint32_t	flags;
	int		err;

	if (CLST_ON_UNCD()) {
		/* Save transition start time for the UNC daemon. */
		CLST_IPC_LOCK(ctx);
		ctx->cic_type = type;
		ctx->cic_sysact = sysact;
		clst_ipc_mark_trans_start(ctx);
		CLST_IPC_UNLOCK(ctx);
	}

	PFC_ASSERT(abstime != NULL);
	pfc_log_info("Raise a cluster state change event: "
		     "type=%u, sysact=%u, timeout=%lu.%lu",
		     type, sysact, abstime->tv_sec, abstime->tv_nsec);

	/* Create a cluster state change event. */
	cep = (clst_event_t *)malloc(sizeof(*cep));
	if (PFC_EXPECT_FALSE(cep == NULL)) {
		pfc_log_error("Failed to allocate a cluster event private "
			      "data: type=%u", type);
		err = ENOMEM;
		goto out;
	}

	cep->e_sysact = sysact;
	cep->e_deadline = *abstime;

	err = pfc_event_create(&event, type, cep, free);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create a cluster state change event: "
			      "type=%u: %s", type, strerror(err));
		free(cep);
		goto out;
	}

	/* Enable event delivery logging. */
	pfc_event_enable_log(event, PFC_TRUE);

	CLST_EVSYNC_LOCK(esp);

	/* Ensure that we can raise a cluster state change event. */
	flags = esp->es_flags;
	if (PFC_EXPECT_FALSE(flags != 0)) {
		CLST_EVSYNC_UNLOCK(esp);

		if (flags & EVSYNC_FINI) {
			err = ECANCELED;
		}
		else {
			PFC_ASSERT(flags & EVSYNC_BUSY);
			err = EBUSY;
		}

		pfc_log_error("Could not change cluster state: %s",
			      strerror(err));
		pfc_event_release(event);
		goto out;
	}

	esp->es_flags |= EVSYNC_BUSY;
	CLST_EVSYNC_UNLOCK(esp);

	/* Post a cluster state change event. */
	err = pfc_event_post(CLST_EVENT_SOURCE, event);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to post a cluster state change event: "
			      "%s", strerror(err));

		CLST_EVSYNC_LOCK(esp);
		esp->es_flags &= ~EVSYNC_BUSY;
		goto out_unlock;
	}

	/* Wait for completion of event delivery. */
	CLST_EVSYNC_LOCK(esp);

	for (;;) {
		if ((esp->es_flags & EVSYNC_BUSY) == 0) {
			err = 0;
			break;
		}
		if (PFC_EXPECT_FALSE(esp->es_flags & EVSYNC_FINI)) {
			pfc_log_notice("Event synchronization has been "
				       "canceled.");
			err = ECANCELED;
			break;
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			pfc_log_error("Cluster event delivery did not complete"
				      ": %s", strerror(err));
			break;
		}
		err = CLST_EVSYNC_TIMEDWAIT_ABS(esp, abstime);
	}

out_unlock:
	CLST_EVSYNC_UNLOCK(esp);

out:
	/* Save transition end time. */
	CLST_IPC_LOCK(ctx);
	clst_ipc_mark_trans_end(ctx);
	CLST_IPC_UNLOCK(ctx);

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * clst_event_init(void)
 *	Initialize clstat event source.
 */
int PFC_ATTR_HIDDEN
clst_event_init(void)
{
	clst_evsync_t	*esp = &clevent_sync_ctx;
	pfc_evmask_t	mask;
	pfc_evhandler_t	*hdlrp;
	const char	*evsrc;
	int		err;

	PFC_ASSERT_INT(PFC_MUTEX_INIT(&esp->es_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_init(&esp->es_cond), 0);
	esp->es_flags = 0;

	for (hdlrp = clevent_handlers;
	     hdlrp < PFC_ARRAY_LIMIT(clevent_handlers); hdlrp++) {
		*hdlrp = EVHANDLER_ID_INVALID;
	}

	/*
	 * Register clstat module event source.
	 * clstat uses local event queue which has only one thread.
	 */
	err = pfc_event_register_local(CLST_EVENT_SOURCE, CLST_EVENT_NTHREADS);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to register event source: %s",
			      strerror(err));

		return err;
	}

	/*
	 * Register cluster state change event handler which will be called
	 * at the end of event delivery.
	 */
	pfc_event_mask_empty(&mask);
	pfc_event_mask_add(&mask, CLSTAT_EVTYPE_ACT);
	err = pfc_event_add_handler(&CLST_EVHDLR_SYNC, CLST_EVENT_SOURCE,
				    clevent_sentinel, NULL, &mask,
				    CLST_EVPRI_CLEVENT);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to register sentinel of cluster event "
			      "handlers: %s", strerror(err));
		goto error;
	}

	/*
	 * Register system shutdown event handler.
	 * This is required to cancel cluster event synchronization.
	 */
	evsrc = pfc_event_global_source();
	pfc_event_mask_empty(&mask);
	PFC_ASSERT_INT(pfc_event_mask_add(&mask, PFC_EVTYPE_SYS_STOP), 0);
	err = pfc_event_add_handler(&CLST_EVHDLR_STOP, evsrc, clevent_shutdown,
				    NULL, &mask, CLST_EVPRI_STOP);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to register shutdown event handler: %s",
			      strerror(err));
		goto error_handler;
	}

	return 0;

error_handler:
	for (hdlrp = clevent_handlers;
	     hdlrp < PFC_ARRAY_LIMIT(clevent_handlers); hdlrp++) {
		pfc_evhandler_t	hdlr = *hdlrp;

		if (hdlr == EVHANDLER_ID_INVALID) {
			continue;
		}

		*hdlrp = EVHANDLER_ID_INVALID;
		PFC_ASSERT_INT(pfc_event_remove_handler(hdlr, NULL), 0);
	}

error:
	(void)pfc_event_unregister(CLST_EVENT_SOURCE, NULL);

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * clst_event_fini(const pfc_timespec_t *abstime)
 *	Finalize clstat event source.
 */
int PFC_ATTR_HIDDEN
clst_event_fini(const pfc_timespec_t *abstime)
{
	pfc_timespec_t	ts, *timeout;
	uint32_t	i;
	int		err;

	clevent_sync_fini();
	timeout = (abstime == NULL) ? NULL : &ts;

	/* Remove event handlers. */
	for (i = 0; i < PFC_ARRAY_CAPACITY(clevent_handlers); i++) {
		pfc_evhandler_t	hdlr = clevent_handlers[i];

		if (hdlr == EVHANDLER_ID_INVALID) {
			continue;
		}
		clevent_handlers[i] = EVHANDLER_ID_INVALID;

		if (timeout != NULL) {
			clst_clock_remains(timeout, abstime);
		}

		err = pfc_event_remove_handler(hdlr, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to remove event handler[%u]: %s",
				      i, strerror(err));

			return err;
		}
	}

	if (timeout != NULL) {
		clst_clock_remains(timeout, abstime);
	}

	/* Unregister event source. */
	err = pfc_event_unregister(CLST_EVENT_SOURCE, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to unregister event source: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}

	return err;
}

/*
 * static void
 * clevent_sentinel(pfc_event_t event, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Cluster state change event handler which synchronizes event delivery.
 *
 *	This handler is registered with the greatest priority value.
 *	So this handler must be called at the end of cluster state change
 *	event delivery.
 */
static void
clevent_sentinel(pfc_event_t event, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	clst_evsync_t	*esp = &clevent_sync_ctx;

	/* Clear BUSY flag in clevent_sync_ctx. */
	CLST_EVSYNC_LOCK(esp);
	PFC_ASSERT(esp->es_flags & EVSYNC_BUSY);
	esp->es_flags &= ~EVSYNC_BUSY;
	CLST_EVSYNC_SIGNAL(esp);
	CLST_EVSYNC_UNLOCK(esp);
}

/*
 * static void
 * clevent_shutdown(pfc_event_t event, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Invoked when the system shutdown sequence has been started.
 */
static void
clevent_shutdown(pfc_event_t event, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	/* Stop listening on IPC channel state. */
	clst_ipc_stop_listener(&ipc_ctx);

	/* Finalize cluster event synchronization. */
	clevent_sync_fini();
}
