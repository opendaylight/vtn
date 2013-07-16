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

#define EVS "evs_cxx_remHdlr"

static void
cb_event3_1(Event *event, MyData *data)
{
	pfc_log_debug("Event handler:");

	// Notify that this event handler has been called
	data->mutex()->lock();
	if (data->isInvoked() != PFC_TRUE) {
		data->setInvoked();
	}
	data->cond()->signal();
	data->mutex()->unlock();

	// Wait for a signal permitting this event handler to exit
	data->sema()->wait();
}

static void
test_timeout(TARGET_QUEUE qtype)
{
	pfc_evhandler_t ehid;
	EventMask mask;
	MyData *data;
	char event_name[BUFSIZE];

	// Setup
	data = new MyData(EXPECTED_VAL, PFC_TRUE);

	// Register an event
	snprintf(event_name, BUFSIZE, "%s_timeout", EVS);
	register_event_cxx(event_name, qtype);

	// Add an event handler
	pfc_log_debug("Add an event handler");
	mask.fill();
	event_handler_t f_eh = boost::bind(cb_event3_1, _1, data);
	ASSERT_EQ(0,  Event::addHandler(ehid, event_name, f_eh, mask, 0));

	// Create an event
	pfc_log_debug("Create an event object");
	Event *ev = new Event(0, static_cast<pfc_ptr_t>(data));

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	ASSERT_EQ(0, Event::post(event_name, ev));

	// Certify that the event handler has been invoked
	data->mutex()->lock();
	while (data->isInvoked() == PFC_FALSE) {
		data->cond()->wait(*data->mutex());
	}
	data->mutex()->unlock();

#ifdef TEST_ETIMEDOUT
	// Try to remove the event handler.
	pfc_log_debug("Try to remove the event handler which is still alive");
	ASSERT_EQ(ETIMEDOUT, Event::removeHandler(ehid, &timeout_ev_cc_));

	// Permit the event handler to exit
	pfc_log_debug("Permit the event handler to exit");
	data->sema()->post();

	// Remove the event handler
	pfc_log_debug("Remove the event handler, but handler is not found");

	ASSERT_EQ(ENOENT, Event::removeHandler(ehid, &timeout_ev_cc_));
#else

	// Permit the event handler to exit
	pfc_log_debug("Permit the event handler to exit");
	data->sema()->post();
	ASSERT_EQ(0, Event::removeHandler(ehid, &timeout_ev_cc_));
	data->resetInvoked();

	// Create an event
	pfc_log_debug("Create an event object");
	Event *ev2 = new Event(0, static_cast<pfc_ptr_t>(data));

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	ASSERT_EQ(0, Event::post(event_name, ev2));

	// Certify that the removed event handler will never be invoked
	data->mutex()->lock();
	ASSERT_EQ(ETIMEDOUT, data->cond()->timedwait(*data->mutex(), timeout_ev_cc_));
	data->mutex()->unlock();

	// Remove the not-exists handler
	pfc_log_debug("Remove the removed event handler");
	ASSERT_EQ(ENOENT, Event::removeHandler(ehid, &timeout_ev_cc_));
#endif

	// Unregister the event
	ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_));

	// Cleanup
	delete data;
}

TEST(removeHandler, id)
{
}

TEST(removeHandler, timeout)
{
	pfc_log_debug("removeHandler_timeout() is called");

	test_timeout(GLOBALQ);
	test_timeout(ASYNCQ);
	test_timeout(LOCALQ);
}

TEST(removeHandler, orphan)
{
	EventMask mask;
	pfc_evhandler_t ehid;
	char event_name[BUFSIZE];

	pfc_log_debug("removeHandler_orphan() is called");

	// Setup
	// do nothing.

	// Register an event
	snprintf(event_name, BUFSIZE, "%s_orphan", EVS);

	// Add an event handler
	pfc_log_debug("Add an event handler");
	mask.fill();
	event_handler_t f_eh = boost::bind(cb_event3_1, _1, (MyData *)NULL);
	ASSERT_EQ(0,  Event::addHandler(ehid, event_name, f_eh, mask, 0));

	// Remove the event handler.
	pfc_log_debug("Remove the event handler which is still alive");
	ASSERT_EQ(0, Event::removeHandler(ehid, &timeout_ev_cc_));

	// Cleanup
	// do nothing.
}
