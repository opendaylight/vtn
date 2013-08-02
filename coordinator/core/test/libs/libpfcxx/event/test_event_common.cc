/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <pfc/log.h>
#include "test_event_common.hh"

#define EVS_COMMON "e_x_common"

extern "C" {
extern void libpfc_init();
extern void libpfc_fini();
}

const pfc_timespec_t timeout_ev_cc_ = {
	0, TIMEOUT_COMMON * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC)
};

class TestEnvironment : public ::testing::Environment {
    protected:
	virtual void SetUp() {
		pfc_log_init("gtest", stdout, PFC_LOGLVL_NOTICE, NULL);
		libpfc_init();
	}
	virtual void TearDown() {
		pfc_log_fini();
		libpfc_fini();
	}
};

::testing::Environment  *global_env =
	  ::testing::AddGlobalTestEnvironment(new TestEnvironment());

using namespace pfc::core;

void
cb_event_cxx(Event *ev)
{
	MyData *data;

	pfc_log_debug("cb_event_cxx() is called");

	// Verify that the callback function receive the user data correctly
	data = (MyData *)ev->getData();
	ASSERT_EQ(EXPECTED_VAL, data->val());

	// Notify that this callback function has been called
	data->mutex()->lock();
	data->cond()->signal();
	data->mutex()->unlock();
}

void *
cb_event_checker_cxx(MyData *data)
{
	int err = 0;

	pfc_log_debug("Wait for callback function to be called.");
	data->mutex()->lock();
	data->sema()->post();  // sem post

	if (data->isInvoked()) {
		data->cond()->wait(*data->mutex());
		EXPECT_EQ(0, err);
	} else {
		err = data->cond()->timedwait(*data->mutex(), timeout_ev_cc_);
		EXPECT_EQ(ETIMEDOUT, err);
	}

	data->mutex()->unlock();

	return data;
}

void
register_event_cxx(char *event_name, TARGET_QUEUE qtype)
{
	pfc_log_debug("Register \"%s\" as event source.", event_name);

	switch (qtype) {
	case GLOBALQ:
		ASSERT_EQ(0, Event::registerEvent(event_name));
		break;
	case ASYNCQ:
		ASSERT_EQ(0, Event::registerAsyncEvent(event_name));
		break;
	case LOCALQ:
		ASSERT_EQ(0, Event::registerLocalEvent(event_name, LOCALQ_THREAD_NUM));
		break;
	default:
		pfc_log_fatal("Unknown event queue type");
		abort();
		// NOT REACHED
		break;
	}
}

void
test_event_cxx_impl(char *event_name, TARGET_QUEUE qtype, pfc_bool_t orphan, pfc_bool_t emergency,
		TARGET_ADDHANDLER targetAddHandler)
{
	Thread *tc_cb;
	pfc_evhandler_t ehid;
	EventMask mask;
	std::string sname;

	// Setup
	MyData *data = new MyData(EXPECTED_VAL, PFC_TRUE);

	// Register an event if this isn't an orphan handler test
	if (!orphan) {
		register_event_cxx(event_name, qtype);
	}

	// Create a checker thread
	pfc_log_debug("Create a checker thread");
	Thread::thr_func_t f_cb_chk = boost::bind(cb_event_checker_cxx, data);
	tc_cb = Thread::create(f_cb_chk);
	ASSERT_NE((Thread *)NULL, tc_cb);
	data->sema()->wait();

	// Add an event handler
	pfc_log_debug("Add an event handler");
	event_handler_t f_eh = boost::bind(cb_event_cxx, _1);
	mask.fill();
	switch (targetAddHandler) {
	case WITH_SNAME:
		sname = event_name;
		ASSERT_EQ(0,  Event::addHandler(ehid, sname, f_eh, static_cast<uint32_t>(0)));
		break;
	case WITH_CNAME:
		ASSERT_EQ(0,  Event::addHandler(ehid, event_name, f_eh, static_cast<uint32_t>(0)));
		break;
	case WITH_MASK_SNAME:
		sname = event_name;
		ASSERT_EQ(0,  Event::addHandler(ehid, sname, f_eh, mask, static_cast<uint32_t>(0)));
		break;
	case WITH_MASK_CNAME:
		ASSERT_EQ(0,  Event::addHandler(ehid, event_name, f_eh, mask, static_cast<uint32_t>(0)));
		break;
	default:
		pfc_log_fatal("Unknown test type");
		abort();
		// NOT REACHED
		break;
	}

	// Register an event if this test is an orphan handler test.
	if (orphan) {
		register_event_cxx(event_name, qtype);
	}

	// Create an event
	pfc_log_debug("Create an event object");
	pfc_evtype_t type = 0;
	Event *ev = new Event(type, (pfc_ptr_t)data);

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	if (!emergency) {
		ASSERT_EQ(0, Event::post(event_name, ev));
	} else {
		ASSERT_EQ(0, Event::postEmergency(event_name, ev));
	}

	// Wait for the checker threads to exit
	tc_cb->join(NULL);
	delete tc_cb;

	// Unregister the event
	pfc_log_debug("Unregister \"%s\" event.", event_name);
	ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_));

	// Cleanup
	delete data;
}

static void
test_name_abnormal_null(TARGET_QUEUE qtype, pfc_bool_t emergency, TARGET_ADDHANDLER targetAddHandler)
{
	pfc_evhandler_t ehid;
	EventMask mask;

	// Register an event
	pfc_log_debug("Register NULL as event source.");
	switch (qtype) {
	case GLOBALQ:
		ASSERT_EQ(EINVAL, Event::registerEvent(NULL));
		break;
	case ASYNCQ:
		ASSERT_EQ(EINVAL, Event::registerAsyncEvent(NULL));
		break;
	case LOCALQ:
		ASSERT_EQ(EINVAL, Event::registerLocalEvent(NULL, LOCALQ_THREAD_NUM));
		break;
	default:
		pfc_log_fatal("Unknown event queue type");
		abort();
		// NOT REACHED
		break;
	}

	// Add an event handler
	pfc_log_debug("Add an event handler associated with NULL event source");
	event_handler_t f_eh = boost::bind(cb_event_cxx, _1);
	mask.fill();
	ASSERT_EQ(EINVAL, Event::addHandler(ehid, NULL, f_eh, mask, static_cast<uint32_t>(0)));

	// Create an event object
	pfc_log_debug("Create an event object");
	Event *ev = new Event(0, (pfc_ptr_t)NULL);

	// Post the event object
	pfc_log_debug("Post the event object to NULL event source");
	if (!emergency) {
		ASSERT_EQ(EINVAL, Event::post(NULL, ev));
	} else {
		ASSERT_EQ(EINVAL, Event::postEmergency(NULL, ev));
	}

	// Unregister the event handler
	pfc_log_debug("Unregister NULL as event source.");
	ASSERT_EQ(EINVAL, Event::unregisterEvent(NULL, &timeout_ev_cc_));
}

static void
test_name_abnormal_noentry(char *event_name, TARGET_QUEUE qtype, pfc_bool_t emergency,
		TARGET_ADDHANDLER targetAddHandler)
{
	pfc_evhandler_t ehid;
	EventMask mask;

	// Setup
	ASSERT_NE(ETIMEDOUT, Event::unregisterEvent(NULL, &timeout_ev_cc_));

	// Add an event handler
	pfc_log_debug("Add an event handler associated with not-exists event source");
	event_handler_t f_eh = boost::bind(cb_event_cxx, _1);
	mask.fill();
	ASSERT_EQ(0,  Event::addHandler(ehid, event_name, f_eh, mask, static_cast<uint32_t>(0)));

	// Create an event object
	pfc_log_debug("Create an event object");
	Event *ev = new Event(0, (pfc_ptr_t)NULL);
	ASSERT_NE(PFC_EVENT_INVALID, ev->getEvent());

	// Post the event object
	pfc_log_debug("Post the event object to not-exists event source");
	if (!emergency) {
		ASSERT_EQ(ENOENT, Event::post(event_name, ev));
	} else {
		ASSERT_EQ(ENOENT, Event::postEmergency(event_name, ev));
	}

	// Unregister the event handler
	pfc_log_debug("Unregister not-exists event source.");
	ASSERT_EQ(0, Event::removeHandler(ehid, &timeout_ev_cc_));

	// Unregister the event source
	pfc_log_debug("Unregister not-exists event source.");
	ASSERT_EQ(ENOENT, Event::unregisterEvent(event_name, &timeout_ev_cc_));
}

void test_name_cxx(TARGET_QUEUE queue, pfc_bool_t emergency)
{
	char name[1024];

	pfc_log_debug("Apply the normal string as event source");
	snprintf(name, sizeof(name), "%s", EVS_COMMON);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_CNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_SNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_MASK_CNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_MASK_SNAME);

	pfc_log_debug("Apply a long string as event source");
	for (int i = 0; i < 100; i++) {
		strncat(name, "9876543210", 11);
	}
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_CNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_SNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_MASK_CNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_MASK_SNAME);

	pfc_log_debug("Apply a short string as an event source");
	snprintf(name, 2, "x");
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_CNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_SNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_MASK_CNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_MASK_SNAME);

	pfc_log_debug("Apply meta characters as an event source");
	snprintf(name, 15, "@] ^[]....[*.$^");
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_CNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_SNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_MASK_CNAME);
	test_event_cxx_impl(name, queue, PFC_FALSE, emergency, WITH_MASK_SNAME);

	pfc_log_debug("Apply NULL as an event source");
	test_name_abnormal_null(queue, emergency, WITH_CNAME);
	test_name_abnormal_null(queue, emergency, WITH_SNAME);
	test_name_abnormal_null(queue, emergency, WITH_MASK_CNAME);
	test_name_abnormal_null(queue, emergency, WITH_MASK_SNAME);

	pfc_log_debug("Apply not-exists event source as an event source");
	snprintf(name, 21, "not-exists cxx event");
	test_name_abnormal_noentry(name, queue, emergency, WITH_CNAME);
	test_name_abnormal_noentry(name, queue, emergency, WITH_SNAME);
	test_name_abnormal_noentry(name, queue, emergency, WITH_MASK_CNAME);
	test_name_abnormal_noentry(name, queue, emergency, WITH_MASK_SNAME);

	pfc_log_debug("Check if an orphan handler is called correctly");
	test_event_cxx_impl(name, queue, PFC_TRUE, emergency, WITH_CNAME);
	test_event_cxx_impl(name, queue, PFC_TRUE, emergency, WITH_SNAME);
	test_event_cxx_impl(name, queue, PFC_TRUE, emergency, WITH_MASK_CNAME);
	test_event_cxx_impl(name, queue, PFC_TRUE, emergency, WITH_MASK_SNAME);
}

