/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_event_common.hh"

#define EVS "evs_cxx_addHandler"
#define PRI_LOWEST 100
#define PRI_HIGHEST 0

namespace pfc {
namespace core {
	class MyData2 : public MyData {
	public:
		inline MyData2(int val, pfc_bool_t isInvoked, uint32_t next_priority):
		MyData(val, isInvoked), counter_(0), next_priority_(next_priority)
		{};

		inline uint32_t getCounter() {
			return counter_;
		}

		inline void countUp() {
			counter_++;
		}

		inline uint32_t getNextPriority() {
			return next_priority_;
		}

		inline void setNextPriority(uint32_t nextp) {
			next_priority_ = nextp;
		}

	private:
		uint32_t counter_;
		uint32_t next_priority_;
	};
} // namespace pfc
} // namespace core

using namespace pfc::core;

TEST(addHandler, id)
{
	char event_name[BUFSIZE];

	pfc_log_debug("addHandler_id() is called");

	snprintf(event_name, sizeof(event_name), "%s_id", EVS);
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_SNAME);
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_MASK_CNAME);
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_MASK_SNAME);

	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_SNAME);
	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_MASK_CNAME);
	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_MASK_SNAME);

	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_SNAME);
	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_MASK_CNAME);
	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_MASK_SNAME);
}


TEST(addHandler, name)
{
	char event_name[BUFSIZE];

	pfc_log_debug("addHandler_name() is called");

	snprintf(event_name, sizeof(event_name), "%s_name_g", EVS);
	test_name_cxx(GLOBALQ, PFC_FALSE);
	snprintf(event_name, sizeof(event_name), "%s_name_a", EVS);
	test_name_cxx(ASYNCQ, PFC_FALSE);
	snprintf(event_name, sizeof(event_name), "%s_name_l", EVS);
	test_name_cxx(LOCALQ, PFC_FALSE);

	pfc_log_debug("Overwrite");
	{
		snprintf(event_name, sizeof(event_name), "%s_name_overwrite", EVS);
		ASSERT_EQ(0, Event::registerEvent(event_name));
#ifndef SKIP_ON_LRT
		ASSERT_EQ(EEXIST, Event::registerEvent(event_name));
#endif
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_)); // Cleanup
	}

	pfc_log_debug("Rewrite");
	{
		snprintf(event_name, sizeof(event_name), "%s_name_rewrite", EVS);
		ASSERT_EQ(0, Event::registerEvent(event_name));
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_));
		ASSERT_EQ(0, Event::registerEvent(event_name)); // Rewrite
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_)); // Cleanup
	}
}

static void
cb_event_cxx2_1(Event *ev, const uint32_t &mypriority)
{
	MyData2 *data;

	pfc_log_debug("Event handler: priority=%u", mypriority);
	data = static_cast<MyData2 *>(ev->getData());

	while (true) {
		if (data->mutex()->trylock() != 0) {
			continue;
		}
		pfc_log_debug("mutex_lock");

		if (data->getNextPriority() == mypriority) {
			data->countUp();

			pfc_log_debug("update the next event handler");
			data->setNextPriority(mypriority + 1);  // next priority

			pfc_log_debug("cond_broadcast");
			data->cond()->broadcast();

			pfc_log_debug("mutex_unlock");
			data->mutex()->unlock();

			break;
		}

		// Wait for the event data to be updated
		pfc_log_debug("cond_timedwait");
		data->cond()->timedwait(*data->mutex(), timeout_ev_cc_);

		pfc_log_debug("mutex_unlock");
		data->mutex()->unlock();
	}

	if (PRI_LOWEST == mypriority) {
		data->sema()->post();
	}

}

static void
cb_event_cxx2_2(Event *ev, const uint32_t &mypriority)
{
	MyData2 *data;

	pfc_log_debug("Event handler: priority=%u", mypriority);
	data = static_cast<MyData2 *>(ev->getData());

	data->mutex()->lock();
	data->countUp();
	data->cond()->broadcast();

	if (data->getCounter() == PRI_LOWEST - PRI_HIGHEST + 1) {
		data->sema()->post();
	}

	pfc_log_debug("mutex_unlock");
	data->mutex()->unlock();
}

static void
addHandler_impl(TARGET_ADDHANDLER htype, char *cname, event_handler_t evh, uint32_t priority)
{
	std::string sname;
	pfc_evhandler_t ehid;
	EventMask mask;

	mask.fill();

	switch (htype) {
	case WITH_CNAME:
		ASSERT_EQ(0, Event::addHandler(ehid, cname, evh, priority));
		break;
	case WITH_SNAME:
		sname = cname;
		ASSERT_EQ(0, Event::addHandler(ehid, sname, evh, priority));
		break;
	case WITH_MASK_CNAME:
		ASSERT_EQ(0, Event::addHandler(ehid, cname, evh, mask, priority));
		break;
	case WITH_MASK_SNAME:
		sname = cname;
		ASSERT_EQ(0, Event::addHandler(ehid, sname, evh, mask, priority));
		break;
	default:
		pfc_log_fatal("Unknown test type");
		abort();
		// NOT REACHED
		break;
	}
}

static void
test_priority(pfc_bool_t count_only, TARGET_QUEUE qtype, TARGET_ADDHANDLER htype)
{
	char cname[BUFSIZE];
	uint32_t id[PRI_LOWEST - PRI_HIGHEST + 1];
	event_handler_t evh;

	EventMask mask;
	MyData2 *data2;

	// Setup
	mask.fill();
	data2 = new MyData2(EXPECTED_VAL, PFC_TRUE, PRI_HIGHEST);
	for (int i = PRI_HIGHEST; i <= PRI_LOWEST; i++) {
		id[i] = i;
	}

	// Register an event
	snprintf(cname, sizeof(cname), "%s_priority", EVS);
	register_event_cxx(cname, qtype);

	// Add an event handler
	pfc_log_debug("Add event handlers with various priority");
	if (!count_only) {
		for (int i = PRI_LOWEST; i >= PRI_HIGHEST; i--) {
			evh = boost::bind(cb_event_cxx2_1, _1, id[i]);
			pfc_log_debug("handler priority: %u", id[i]);
			addHandler_impl(htype, cname, evh, id[i]);
		}
	} else {
		evh = boost::bind(cb_event_cxx2_2, _1, id[PRI_LOWEST]);
		addHandler_impl(htype, cname, evh, id[PRI_LOWEST]);
		for (int i = PRI_LOWEST -1; i >= PRI_HIGHEST; i--) {
			evh = boost::bind(cb_event_cxx2_2, _1, id[i]);
			addHandler_impl(htype, cname, evh, id[i / 5]);
		}
	}

	// Create an event
	pfc_log_debug("Create an event object");
	Event *ev = new Event((pfc_evtype_t)0, static_cast<pfc_ptr_t>(data2));

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", cname);
	ASSERT_EQ(0, Event::post(cname, ev));

	// Wait for the lowest handler to exit
	pfc_log_debug("Wait for the lowest(%d) handler to exit", PRI_LOWEST);
	data2->sema()->wait();

	// Check the event handler counter
	pfc_log_debug("Total %u handlers have been called.", data2->getCounter());
	ASSERT_EQ(static_cast<uint32_t>(PRI_LOWEST - PRI_HIGHEST + 1),
		  data2->getCounter());

	// Unregister the event
	pfc_log_debug("Unregister \"%s\" event.", cname);
	ASSERT_EQ(0, Event::unregisterEvent(cname, &timeout_ev_cc_));

	// Cleanup
	delete data2;
}

TEST(addHandler, priority)
{
	pfc_log_debug("addHandler_priority() is called");

	pfc_log_debug("Test for the global queue");
	test_priority(PFC_FALSE, GLOBALQ, WITH_CNAME);
	test_priority(PFC_FALSE, GLOBALQ, WITH_SNAME);
	test_priority(PFC_FALSE, GLOBALQ, WITH_MASK_CNAME);
	test_priority(PFC_FALSE, GLOBALQ, WITH_MASK_SNAME);

	pfc_log_debug("Test for the async queue");
	test_priority(PFC_TRUE, ASYNCQ, WITH_CNAME);
	test_priority(PFC_TRUE, ASYNCQ, WITH_SNAME);
	test_priority(PFC_TRUE, ASYNCQ, WITH_MASK_CNAME);
	test_priority(PFC_TRUE, ASYNCQ, WITH_MASK_SNAME);

	pfc_log_debug("Test for the local queue");
	pfc_bool_t count_only = LOCALQ_THREAD_NUM > 1 ? PFC_TRUE : PFC_FALSE;
	test_priority(count_only, LOCALQ, WITH_CNAME);
	test_priority(count_only, LOCALQ, WITH_SNAME);
	test_priority(count_only, LOCALQ, WITH_MASK_CNAME);
	test_priority(count_only, LOCALQ, WITH_MASK_SNAME);
}

TEST(addHandler, handler)
{
	char event_name[BUFSIZE];

	pfc_log_debug("addHandler_handler() is called");
	snprintf(event_name, sizeof(event_name), "%s_handler", EVS);
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_SNAME);
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_MASK_CNAME);
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_MASK_SNAME);

	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_SNAME);
	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_MASK_CNAME);
	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_MASK_SNAME);

	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_SNAME);
	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_MASK_CNAME);
	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_MASK_SNAME);
}

static void
test_mask(char *event_name, TARGET_QUEUE qtype, TARGET_ADDHANDLER targetAddHandler, pfc_bool_t isMatched)
{
	Thread *tc_cb;
	pfc_evhandler_t ehid;
	EventMask mask;
	std::string sname;

	// Setup
	MyData *data = new MyData(EXPECTED_VAL, isMatched);
	event_handler_t f_eh = boost::bind(cb_event_cxx, _1);

	// Register an event
	register_event_cxx(event_name, qtype);

	// EINVAL (*evmask is 0)
	mask.empty();
	ASSERT_EQ(EINVAL, Event::addHandler(ehid, event_name, f_eh, mask,
					    static_cast<uint32_t>(0)));

	if (isMatched) {
		mask.fill();
	} else {
		mask.empty();
	}

	// Create a checker thread
	pfc_log_debug("Create a checker thread");
	Thread::thr_func_t f_cb_chk = boost::bind(cb_event_checker_cxx, data);
	tc_cb = Thread::create(f_cb_chk);
	ASSERT_NE((Thread *)NULL, tc_cb);
	data->sema()->wait();

	// Add an event handler
	pfc_log_debug("Add an event handler");
	switch (targetAddHandler) {
	case WITH_MASK_SNAME:
		sname = event_name;
		if (isMatched) {
			ASSERT_EQ(0,
				  Event::addHandler(ehid, sname, f_eh, mask,
						    static_cast<uint32_t>(0)));
		} else {
			ASSERT_EQ(EINVAL,
				  Event::addHandler(ehid, sname, f_eh, mask,
						    static_cast<uint32_t>(0)));
		}
		break;
	case WITH_MASK_CNAME:
		if (isMatched) {
			ASSERT_EQ(0,
				  Event::addHandler(ehid, event_name, f_eh, mask,
						    static_cast<uint32_t>(0)));
		} else {
			ASSERT_EQ(EINVAL,
				  Event::addHandler(ehid, sname, f_eh, mask,
						    static_cast<uint32_t>(0)));
		}
		break;
	default:
		pfc_log_fatal("Unknown test type");
		abort();
		// NOT REACHED
		break;
	}

	// Create an event
	pfc_log_debug("Create an event object");
	pfc_evtype_t type = 0;
	Event *ev = new Event(type, (pfc_ptr_t)data);

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	ASSERT_EQ(0, Event::post(event_name, ev));

	// Wait for the checker threads to exit
	tc_cb->join(NULL);
	delete tc_cb;

	// Unregister the event
	pfc_log_debug("Unregister \"%s\" event.", event_name);
	ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_));

	// Cleanup
	delete data;
}

TEST(addHandler, mask)
{
	char event_name[BUFSIZE];

	pfc_log_debug("addHandler_mask() is called");

	snprintf(event_name, sizeof(event_name), "%s_mask", EVS);
	pfc_log_debug("Match the event mask");
	test_mask(event_name, GLOBALQ, WITH_MASK_CNAME, PFC_TRUE);
	test_mask(event_name, GLOBALQ, WITH_MASK_SNAME, PFC_TRUE);
	test_mask(event_name, ASYNCQ, WITH_MASK_CNAME, PFC_TRUE);
	test_mask(event_name, ASYNCQ, WITH_MASK_SNAME, PFC_TRUE);
	test_mask(event_name, LOCALQ, WITH_MASK_CNAME, PFC_TRUE);
	test_mask(event_name, LOCALQ, WITH_MASK_SNAME, PFC_TRUE);

	pfc_log_debug("Unmatch the event mask");
	test_mask(event_name, GLOBALQ, WITH_MASK_CNAME, PFC_FALSE);
	test_mask(event_name, GLOBALQ, WITH_MASK_SNAME, PFC_FALSE);
	test_mask(event_name, ASYNCQ, WITH_MASK_CNAME, PFC_FALSE);
	test_mask(event_name, ASYNCQ, WITH_MASK_SNAME, PFC_FALSE);
	test_mask(event_name, LOCALQ, WITH_MASK_CNAME, PFC_FALSE);
	test_mask(event_name, LOCALQ, WITH_MASK_SNAME, PFC_FALSE);
}
