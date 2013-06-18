/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.c - PFC event management.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pfc/clock.h>
#include <event_impl.h>
#include "pfcd.h"

/* System global event source. */
static const char	*evsrc_global;

/* System async event source. */
static const char	*evsrc_async;

/* System debug event source. */
static const char	*evsrc_debug;

/*
 * Timeout, in seconds, for shutdown start event acknowledgement.
 */
static const char	param_sd_ack_timeout[] = "shutdown_ack_timeout";

#define	SD_ACK_TIMEOUT_DEFAULT		30U

/*
 * void
 * event_init(void)
 *	Initialize PFC events.
 */
void
event_init(void)
{
	evsrc_global = pfc_event_global_source();
	evsrc_async = pfc_event_async_source();
	evsrc_debug = pfc_event_debug_source();
}

/*
 * void
 * event_post_global(pfc_evtype_t type)
 *	Post a PFC system global event which has the specified event type.
 */
void
event_post_global(pfc_evtype_t type)
{
	pfc_event_t	event;
	pfc_bool_t	emergency = PFC_FALSE;
	pfc_ptr_t	data = NULL;
	int	err;

	if (type == PFC_EVTYPE_SYS_STOP) {
		if (pfcd_getstate() == PFCD_STATE_FATAL) {
			/* This should be an emergency event. */
			emergency = PFC_TRUE;
		}
		data = (pfc_ptr_t)(uintptr_t)emergency;
	}

	err = pfc_event_create(&event, type, data, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to generate global event: %u: %s",
			      type, strerror(err));

		return;
	}

	if (emergency) {
		err = pfc_event_post_emergency(evsrc_global, event);
	}
	else {
		err = pfc_event_post(evsrc_global, event);
	}
	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_timespec_t	*timeout, tspec;
		uint32_t	ack_timeout;

		/*
		 * We should wait for shutdown start event to be delivered
		 * to all handlers.
		 */
		ack_timeout = pfc_conf_get_uint32(pfcd_options,
						  param_sd_ack_timeout,
						  SD_ACK_TIMEOUT_DEFAULT);
		if (ack_timeout == 0) {
			timeout = NULL;
		}
		else {
			tspec.tv_sec = ack_timeout;
			tspec.tv_nsec = 0;
			timeout = &tspec;
		}
		err = pfc_event_flush(evsrc_global, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			if (err == ETIMEDOUT) {
				pfc_log_error("Shutdown event handler seems "
					      "to stuck.");
			}
			else {
				pfc_log_error("Shutdown event sync failed: %s",
					      strerror(err));
			}
		}
	}
	else {
		pfc_log_error("Failed to deliver global event: %u: %s",
			      type, strerror(err));
	}
}

/*
 * pfc_bool_t
 * event_post_signal(int sig)
 *	Deliver an event that represents the specified signal.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned on success. Otherwise PFC_FALSE.
 *
 */
pfc_bool_t
event_post_signal(int sig)
{
	pfc_event_t	event;
	int	err;

	err = pfc_event_create(&event, PFC_EVTYPE_ASYNC_SIGNAL,
			       (pfc_ptr_t)(uintptr_t)sig, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to generate event for signal %u: %s",
			      sig, strerror(err));

		return PFC_FALSE;
	}

	err = pfc_event_post(evsrc_async, event);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to deliver event for signal %u: %s",
			      sig, strerror(err));

		return PFC_FALSE;
	}

	pfc_log_verbose("Deliver an event for signal %u", sig);

	return PFC_TRUE;
}
