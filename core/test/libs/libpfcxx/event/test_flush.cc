/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_event_common.hh"

#define EVS "evs_cxx_flush"
#define	CB_FLUSH_TIMEOUT_MSEC	(10)
#define	CB_FLUSH_TIMEOUT_NSEC						\
	(CB_FLUSH_TIMEOUT_MSEC * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC))
#define	CB_SLEEP_MSEC		(CB_FLUSH_TIMEOUT_MSEC + 50)
#define	CB_SLEEP_USEC							\
	(CB_SLEEP_MSEC * (PFC_CLOCK_MICROSEC / PFC_CLOCK_MILLISEC))

static void
cb_flush(Event *ev)
{
	MyData *mdp;

	pfc_log_debug("Event handler:");

	// Verify that the callback function receive the user data correctly
	mdp = static_cast<MyData *>(ev->getData());
	EXPECT_EQ(EXPECTED_VAL, mdp->val());

	// Wait for the notification.
	mdp->sema()->wait();
}

static void
test_flush_impl(char *event_name, TARGET_QUEUE qtype, pfc_bool_t orphan,
		pfc_bool_t emergency, const pfc_timespec_t *timeout)
{
	pfc_evhandler_t ehid;
	MyData *data;

	// Setup
	data = new MyData(EXPECTED_VAL, PFC_TRUE);

	// Register an event if this isn't an orphan handler test
	if (!orphan) {
		register_event_cxx(event_name, qtype);
	}

	// Add an event handler
	pfc_log_debug("Add an event handler");
	event_handler_t f_eh = boost::bind(cb_flush, _1);
	ASSERT_EQ(0, Event::addHandler(ehid, event_name, f_eh, static_cast<uint32_t>(0)));

	// Register an event if this test is an orphan handler test.
	if (orphan) {
		register_event_cxx(event_name, qtype);
	}

	// Create an event
	pfc_log_debug("Create an event object");
	Event *ev = new Event((pfc_evtype_t)1, static_cast<pfc_ptr_t>(data));

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	if (!emergency) {
		ASSERT_EQ(0, Event::post(event_name, ev));
	} else {
		ASSERT_EQ(0, Event::post(event_name, ev));
	}

	if (timeout != NULL) {
		int  err(Event::flush(event_name, timeout));

		if (timeout->tv_sec == 0 &&
		    timeout->tv_nsec == CB_FLUSH_TIMEOUT_NSEC) {
			ASSERT_EQ(ETIMEDOUT, err);
		} else {
			ASSERT_EQ(EINVAL, err);
		}
	}

	usleep(CB_SLEEP_USEC);
	pfc_timespec_t to = {0, 0};
	ASSERT_EQ(ETIMEDOUT, Event::flush(event_name, &to));

	// Wake up the event handler.
	data->sema()->post();
	ASSERT_EQ(0, Event::flush(event_name));

	// Unregister the event
	pfc_log_debug("Unregister \"%s\" event.", event_name);
	ASSERT_EQ(0, Event::unregisterEvent(event_name, NULL/*&timeout_ev_cc_*/));

	// Cleanup
	delete data;
}

TEST(flush, name)
{
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_flush_name() is called");

	pfc_log_debug("Set a valid value as event_name");
	snprintf(event_name, BUFSIZE, "%s_name", EVS);
	test_flush_impl(event_name, GLOBALQ, 0, 0, NULL);
	test_flush_impl(event_name, ASYNCQ, 0, 0, NULL);
	test_flush_impl(event_name, LOCALQ, 0, 0, NULL);

	pfc_log_debug("Set a not-exits value as event_name");
	snprintf(event_name, BUFSIZE, "%s_not_exist", EVS);
	Event::unregisterEvent(event_name, &timeout_ev_cc_);
	ASSERT_EQ(ENOENT, Event::flush(event_name, &timeout_ev_cc_));
}

TEST(flush, timeout)
{
	char event_name[BUFSIZE];
	pfc_timespec_t timeout;

	pfc_log_debug("pfc_event_flush_timeout() is called");

	snprintf(event_name, BUFSIZE, "%s_timeout1", EVS);

	pfc_log_debug("Set a value which results in timeout ");
	snprintf(event_name, BUFSIZE, "%s_timeout2", EVS);
	timeout.tv_sec = 0;
	timeout.tv_nsec = CB_FLUSH_TIMEOUT_NSEC;
	test_flush_impl(event_name, GLOBALQ, 1, 0, &timeout);
	test_flush_impl(event_name, ASYNCQ, 1, 0, &timeout);
	test_flush_impl(event_name, LOCALQ, 1, 0, &timeout);

	pfc_log_debug("Set NULL as timeout ");
	snprintf(event_name, BUFSIZE, "%s_timeout3", EVS);
	test_flush_impl(event_name, GLOBALQ, 1, 0, NULL);
	test_flush_impl(event_name, ASYNCQ, 1, 0, NULL);
	test_flush_impl(event_name, LOCALQ, 1, 0, NULL);

	pfc_log_debug("Set a negative value as timeout ");
	snprintf(event_name, BUFSIZE, "%s_timeout4", EVS);
	timeout.tv_sec = -1;
	timeout.tv_nsec = -1;
	test_flush_impl(event_name, GLOBALQ, 1, 0, &timeout);
	test_flush_impl(event_name, ASYNCQ, 1, 0, &timeout);
	test_flush_impl(event_name, LOCALQ, 1, 0, &timeout);
}
