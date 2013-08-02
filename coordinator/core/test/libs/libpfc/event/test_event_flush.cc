/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_event_common.h"

#define EVSRC "evs_flush_c"
#define CB_FLUSH_TIMEOUT_MSEC	(10)
#define CB_FLUSH_TIMEOUT_NSEC						\
	(CB_FLUSH_TIMEOUT_MSEC * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC))
#define	CB_SLEEP_MSEC		(CB_FLUSH_TIMEOUT_MSEC + 50)
#define CB_SLEEP_USEC							\
	(CB_SLEEP_MSEC * (PFC_CLOCK_MICROSEC / PFC_CLOCK_MILLISEC))

static void *
cb_flush(pfc_event_t event, pfc_ptr_t arg)
{
	myevent_t *mep;
	myevent_t *mep2;

	pfc_log_debug("Event handler is called");

	// Verify that the callback function receive the user data correctly
	mep = (myevent_t *)arg;
	EXPECT_EQ(EXPECTED_VAL, mep->val);
	mep2 = (myevent_t *)pfc_event_data(event);
	EXPECT_EQ(EXPECTED_VAL, mep2->val);

	// Wait for the notification.
	pfc_sem_wait(&mep->sem);

	return arg;
}

static void
dtor_flush(pfc_ptr_t arg)
{
	pfc_log_debug("Event destructor function is called");

	// Not cleanup resources because they will be cleanuped
	// after verifying this function is called
}

static void
test_flush_impl(char *event_name, TARGET_QUEUE qtype, pfc_bool_t orphan,
		pfc_bool_t emergency, const pfc_timespec_t *timeout)
{
	pfc_event_t event;
	pfc_evhandler_t ehid;
	myevent_t *mep;

	// Setup
	initialize_myevent_c(&mep, PFC_TRUE, PFC_TRUE);

	// Register an event if this isn't an orphan handler test
	if (!orphan) {
		register_event_c(event_name, qtype);
	}

	// Add an event handler
	pfc_log_debug("Add an event handler");
	ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
			(pfc_evfunc_t)&cb_flush, (pfc_ptr_t)mep, (pfc_evmask_t)0, 0));

	// Register an event if this test is an orphan handler test.
	if (orphan) {
		register_event_c(event_name, qtype);
	}

	// Create an event
	pfc_log_debug("Create an event object");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)1, (pfc_ptr_t)mep,
			(pfc_evdtor_t)dtor_flush));

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	if (!emergency) {
		ASSERT_EQ(0, pfc_event_post(event_name, event));
	} else {
		ASSERT_EQ(0, pfc_event_post_emergency(event_name, event));
	}

	if (timeout != NULL) {
		int err(pfc_event_flush(event_name, timeout));
		if (timeout->tv_sec == 0 &&
		    timeout->tv_nsec == CB_FLUSH_TIMEOUT_NSEC) {
			ASSERT_EQ(ETIMEDOUT, err);
		} else {
			ASSERT_EQ(EINVAL, err);
		}
	}

	usleep(CB_SLEEP_USEC);
	pfc_timespec_t to = {0, 0};
	ASSERT_EQ(ETIMEDOUT, pfc_event_flush(event_name, &to));

	// Wake up the event handler.
	pfc_sem_post(&mep->sem);
	ASSERT_EQ(0, pfc_event_flush(event_name, NULL));

	// Unregister the event
	pfc_log_debug("Unregister \"%s\" event.", event_name);
	ASSERT_EQ(0, pfc_event_unregister(event_name, NULL));

	// Cleanup
	cleanup_myevent_c(mep);
}

TEST(pfc_event_flush, name)
{
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_flush_name() is called");

	pfc_log_debug("Set valid value as event_name");
	snprintf(event_name, BUFSIZE, "%s_name", EVSRC);
	test_flush_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, NULL);
	test_flush_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, NULL);
	test_flush_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, NULL);

	pfc_log_debug("Set NULL as event_name");
	EXPECT_EQ(EINVAL, pfc_event_flush(NULL, &timeout_ev_c_));

	pfc_log_debug("Set not-exits value as event_name");
	snprintf(event_name, BUFSIZE, "%s_name_not_exist", EVSRC);
	pfc_event_unregister(event_name, &timeout_ev_c_);
	EXPECT_EQ(ENOENT, pfc_event_flush(event_name, &timeout_ev_c_));
}

TEST(pfc_event_flush, timeout)
{
	char event_name[BUFSIZE];
	pfc_timespec_t timeout;

	pfc_log_debug("pfc_event_flush_timeout() is called");
	snprintf(event_name, BUFSIZE, "%s_timeout", EVSRC);

	pfc_log_debug("Set a value which results in timeout ");
	timeout.tv_sec = 0;
	timeout.tv_nsec = CB_FLUSH_TIMEOUT_NSEC;
	test_flush_impl(event_name, GLOBALQ, PFC_TRUE, PFC_FALSE, &timeout);
	test_flush_impl(event_name, ASYNCQ, PFC_TRUE, PFC_FALSE, &timeout);
	test_flush_impl(event_name, LOCALQ, PFC_TRUE, PFC_FALSE, &timeout);

	pfc_log_debug("Set NULL as timeout ");
	test_flush_impl(event_name, GLOBALQ, PFC_TRUE, PFC_FALSE, NULL);
	test_flush_impl(event_name, ASYNCQ, PFC_TRUE, PFC_FALSE, NULL);
	test_flush_impl(event_name, LOCALQ, PFC_TRUE, PFC_FALSE, NULL);

	pfc_log_debug("Set a negative value as timeout ");
	timeout.tv_sec = -1;
	timeout.tv_nsec = -1;
	test_flush_impl(event_name, GLOBALQ, PFC_TRUE, PFC_FALSE, &timeout);
	test_flush_impl(event_name, ASYNCQ, PFC_TRUE, PFC_FALSE, &timeout);
	test_flush_impl(event_name, LOCALQ, PFC_TRUE, PFC_FALSE, &timeout);
}
