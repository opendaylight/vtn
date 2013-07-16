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
#include <pfcxx/thread.hh>
#include <pfcxx/event.hh>
#include <pfcxx/synch.hh>

#include <boost/bind.hpp>

#define EVSRC_TEST_GLOBAL "evs_cxx_inheritance_global"
#define EVSRC_TEST_ASYNC "evs_cxx_inheritance_async"
#define EVSRC_TEST_LOCAL1 "evs_cxx_inheritance_local1"
#define EVSRC_TEST_LOCAL2 "evs_cxx_inheritance_local2"
#define EVTYPE_TEST 1
#define EXPECTED_VAL 999

namespace pfc {
namespace core {

class Event3 : public Event {
public:
	inline Event3(pfc_evtype_t type, pfc_ptr_t data, int val) :
	Event(type, data), val_(val){};

	inline void destroyData(pfc_ptr_t data)
	{
		Semaphore *sem;

		pfc_log_debug("Event3::destroyData");
		sem = static_cast<Semaphore *>(data);
		sem->post();
	}

	inline int getVal() {
		return val_;
	}
private:
	int val_;
};

} // namespace core
} // namespace pfc



using namespace pfc::core;

static void
handler(Event *e)
{
	Event3 *e3;

	pfc_log_debug("event handler:");
	e3 = dynamic_cast<Event3 *>(e);
	ASSERT_EQ(EXPECTED_VAL, e3->getVal());
}

TEST(Event, inheritance)
{
	pfc_evhandler_t evh;
	event_handler_t eh;
	EventMask mask;

	pfc_log_debug("Event_inheritance() is called");

	// Register event sources
	ASSERT_EQ(0, Event::registerEvent(EVSRC_TEST_GLOBAL));
	ASSERT_EQ(0, Event::registerAsyncEvent(EVSRC_TEST_ASYNC));
	ASSERT_EQ(0, Event::registerLocalEvent(EVSRC_TEST_LOCAL1, 1));
	ASSERT_EQ(0, Event::registerLocalEvent(EVSRC_TEST_LOCAL2, 10));

	// Add event handlers
	mask.empty();
	mask.add(EVTYPE_TEST);
	eh = boost::bind(&handler, _1);
	ASSERT_EQ(0, Event::addHandler(evh, EVSRC_TEST_GLOBAL, eh, mask,
			static_cast<uint32_t>(0)));
	ASSERT_EQ(0, Event::addHandler(evh, EVSRC_TEST_ASYNC, eh, mask,
			static_cast<uint32_t>(0)));
	ASSERT_EQ(0, Event::addHandler(evh, EVSRC_TEST_LOCAL1, eh, mask,
			static_cast<uint32_t>(0)));
	ASSERT_EQ(0, Event::addHandler(evh, EVSRC_TEST_LOCAL2, eh, mask,
			static_cast<uint32_t>(0)));

	// Create events
	Semaphore *sem;
	sem = new Semaphore(0);
	Event3 *e3g = new Event3(EVTYPE_TEST, (pfc_ptr_t)sem, EXPECTED_VAL);
	ASSERT_EQ(0, Event::post(EVSRC_TEST_GLOBAL, e3g));
	sem->wait();
	delete sem;

	sem = new Semaphore(0);
	Event3 *e3a = new Event3(EVTYPE_TEST, (pfc_ptr_t)sem, EXPECTED_VAL);
	ASSERT_EQ(0, Event::post(EVSRC_TEST_ASYNC, e3a));
	sem->wait();
	delete sem;

	sem = new Semaphore(0);
	Event3 *e3l1 = new Event3(EVTYPE_TEST, (pfc_ptr_t)sem, EXPECTED_VAL);
	ASSERT_EQ(0, Event::post(EVSRC_TEST_LOCAL1, e3l1));
	sem->wait();
	delete sem;

	sem = new Semaphore(0);
	Event3 *e3l2 = new Event3(EVTYPE_TEST, (pfc_ptr_t)sem, EXPECTED_VAL);
	ASSERT_EQ(0, Event::post(EVSRC_TEST_LOCAL2, e3l2));
	sem->wait();
	delete sem;
	
	// Unregister event sources
	ASSERT_EQ(0, Event::unregisterEvent(EVSRC_TEST_GLOBAL));
	ASSERT_EQ(0, Event::unregisterEvent(EVSRC_TEST_ASYNC));
	ASSERT_EQ(0, Event::unregisterEvent(EVSRC_TEST_LOCAL1));
	ASSERT_EQ(0, Event::unregisterEvent(EVSRC_TEST_LOCAL2));
}
