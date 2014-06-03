/*
 * Copyright (c) 2010-2014 NEC Corporation
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

/* System async event source. */
static const char	*evsrc_async;

/* System debug event source. */
static const char	*evsrc_debug;

/*
 * Timeout, in seconds, of system event delivery.
 */
static const char	param_sysev_ack_timeout[] = "sysevent_ack_timeout";
#define	SYSEV_ACK_TIMEOUT_DEFAULT	30U

/*
 * void
 * event_init(void)
 *	Initialize PFC events.
 */
void
event_init(void)
{
	evsrc_async = pfc_event_async_source();
	evsrc_debug = pfc_event_debug_source();
}

/*
 * int
 * event_post_global(pfc_evtype_t type)
 *	Post a PFC system global event which has the specified event type.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
event_post_global(pfc_evtype_t type)
{
	pfc_timespec_t	*timeout, tspec;
	uint32_t	ack_timeout;

	/* We should wait for completion of system event delivery. */
	ack_timeout = pfc_conf_get_uint32(pfcd_options,
					  param_sysev_ack_timeout,
					  SYSEV_ACK_TIMEOUT_DEFAULT);
	if (ack_timeout == 0) {
		timeout = NULL;
	}
	else {
		tspec.tv_sec = ack_timeout;
		tspec.tv_nsec = 0;
		timeout = &tspec;
	}

	return libpfc_post_sysevent(type, timeout);
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
