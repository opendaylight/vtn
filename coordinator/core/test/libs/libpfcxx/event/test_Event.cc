/*
 * Copyright (c) 2010-2014 NEC Corporation
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
#include "params.h"

#define	HOLD_COUNT	10

namespace pfc {
namespace core {

class Event2 : public Event {
public:
	inline Event2(pfc_evtype_t type, pfc_ptr_t data, Semaphore *sem) :
	Event(type, data), sem_(sem){};

	inline void destroyData(pfc_ptr_t data)
	{
		uint64_t *ui64p;

		pfc_log_debug("Event2::destroyData");
		ui64p = static_cast<uint64_t *>(data);
		ASSERT_EQ(static_cast<uint64_t>(99999999), *ui64p);
		sem_->post();
	}
private:
	Semaphore *sem_;
};

} // namespace core
} // namespace pfc



using namespace pfc::core;

TEST(Event, Event_type)
{
	Event *e1, *e2, *e3, *e4, *e5;
	pfc_evtype_t type;

	pfc_log_debug("Event_Event_type() is called");

	// normal test1
	type = 0;
	e1 = new Event(type, NULL);
	ASSERT_NE(PFC_EVENT_INVALID, e1->getEvent());
	Event::release(e1);

	// normal test2
	type = 15;
	e2 = new Event(type, NULL);
	ASSERT_NE(PFC_EVENT_INVALID, e2->getEvent());
	Event::release(e2);

	// normal test3
	type = 31;
	e3 = new Event(type, NULL);
	ASSERT_NE(PFC_EVENT_INVALID, e3->getEvent());
	Event::release(e3);

	// abnormal test1
	type = -1;
	e4 = new Event(type, NULL);
	ASSERT_EQ(PFC_EVENT_INVALID, e4->getEvent());
	delete(e4);

	// abnormal test2
	type = 32;
	e5 = new Event(type, NULL);
	ASSERT_EQ(PFC_EVENT_INVALID, e5->getEvent());
	delete(e5);
}

TEST(Event, Event_data)
{
	Event *e1, *e2;
	uint64_t ui64 = -1;
	pfc_ptr_t data;
	pfc_evtype_t type = 0;

	pfc_log_debug("Event_Event_data() is called");

	// normal test1
	data = (pfc_ptr_t)&ui64;
	e1 = new Event(type, data);
	ASSERT_NE(PFC_EVENT_INVALID, e1->getEvent());
	ASSERT_EQ(data, e1->getData());
	Event::release(e1);

	// normal test2
	data = (pfc_ptr_t)NULL;
	e2 = new Event(type, data);
	ASSERT_NE(PFC_EVENT_INVALID, e2->getEvent());
	ASSERT_EQ(data, e2->getData());
	Event::release(e2);
}

TEST(Event, getEvent)
{
	Event *e1, *e2;

	pfc_log_debug("Event_getEvent() is called");

	// normal test1
	e1 = new Event((pfc_evtype_t)0, NULL);
	ASSERT_NE(PFC_EVENT_INVALID, e1->getEvent());
	Event::release(e1);

	// abnormal test1
	e2 = new Event((pfc_evtype_t)-1, NULL);  // invalid event
	ASSERT_EQ(PFC_EVENT_INVALID, e2->getEvent());
	delete(e2);
}

TEST(Event, getRefPointer)
{
	Event *e1, *e2;

	pfc_log_debug("Event_getRefPointer() is called");

	// normal test1
	e1 = new Event((pfc_evtype_t)0, NULL);
	ASSERT_NE((pfc_refptr_t *)NULL, e1->getRefPointer());
	Event::release(e1);

	// abnormal test1
	e2 = new Event((pfc_evtype_t)-1, NULL);  // invalid event
	ASSERT_EQ((pfc_refptr_t *)NULL, e2->getRefPointer());
	delete(e2);
}

TEST(Event, getSerial)
{
	Event *e[EVENT_NUM], *ea;

	pfc_log_debug("Event_getSerial() is called");

	// normal test1
	for (int i = 0; i < EVENT_NUM; i++) {
		e[i] = new Event((pfc_evtype_t)0, NULL);
	}
	for (int i = 0; i < EVENT_NUM; i++) {
		for (int j = i + 1; j < EVENT_NUM; j++) {
			ASSERT_NE((pfc_evid_t)0, e[i]->getSerial());
			ASSERT_NE(e[i]->getSerial(), e[j]->getSerial());
		}
	}
	for (int i = 0; i < EVENT_NUM; i++)	{
		Event::release(e[i]);
	}

	// abnormal test1
	ea = new Event((pfc_evtype_t)-1, NULL);  // invalid event
	ASSERT_EQ((pfc_evid_t)0, ea->getSerial());
	delete(ea);
}

TEST(Event, getType)
{
	Event *e1, *e2;

	pfc_log_debug("Event_getType() is called");

	// normal test1
	e1 = new Event((pfc_evtype_t)1, NULL);
	ASSERT_EQ((pfc_evtype_t)1, e1->getType());
	Event::release(e1);

	// abnormal test1
	e2 = new Event((pfc_evtype_t)-1, NULL);  // invalid event
	ASSERT_EQ((pfc_evtype_t)0, e2->getType());
	delete(e2);
}

TEST(Event, getData)
{
	Event *e1, *e2;
	uint64_t data = 100;

	pfc_log_debug("Event_getData() is called");

	// normal test1
	e1 = new Event((pfc_evtype_t)1, (pfc_ptr_t)&data);
	ASSERT_EQ((pfc_ptr_t)&data, e1->getData());
	ASSERT_EQ((uint64_t)data, *((uint64_t *)e1->getData()));
	Event::release(e1);

	// abnormal test1
	e2 = new Event((pfc_evtype_t)-1, (pfc_ptr_t)&data);  // invalid event
	ASSERT_EQ((pfc_ptr_t)NULL, e2->getData());
	delete(e2);
}

TEST(Event, destroyData)
{
	Event2 *e2;
	uint64_t data;
	Semaphore sem(0);

	pfc_log_debug("Event_destroyData() is called");

	data = static_cast<uint64_t>(99999999);
	e2 = new Event2((pfc_evtype_t)1, (pfc_ptr_t)&data, &sem);
	Event::release(e2);
	sem.wait();
}

TEST(Event, getInstance)
{
	Event *e1;
	pfc_refptr_t *refp;

	pfc_log_debug("Event_getInstance() is called");

	// normal test1
	e1 = new Event((pfc_evtype_t)1, (pfc_ptr_t)NULL);
	ASSERT_NE(PFC_EVENT_INVALID, e1->getEvent());
	refp = e1->getRefPointer();
	ASSERT_EQ(e1, Event::getInstance(refp));
	Event::release(e1);

	// abnormal test1
	ASSERT_EQ(0, Event::getInstance((pfc_refptr_t*)NULL));
}

static void
set_random_ts(pfc_timespec_t *ts)
{
	ts->tv_sec = 0;
	ts->tv_nsec = (random() % 10) * 5000000;
}

static void *
holder(Event *e)
{
	pfc_timespec_t breaktime;

	set_random_ts(&breaktime);
	for (int i = 0; i < HOLD_COUNT; i++) {
		Event::hold(e);
		set_random_ts(&breaktime);
		nanosleep(&breaktime, NULL);
	}

	return (void *)NULL;
}

static void *
releaser(Event *e)
{
	pfc_timespec_t breaktime;

	set_random_ts(&breaktime);
	for (int i = 0; i < HOLD_COUNT; i++) {
		Event::release(e);
		set_random_ts(&breaktime);
		nanosleep(&breaktime, NULL);
	}

	return (void *)NULL;
}

static void
test_hold_release(Event *e)
{
	Thread *holders[HOLDER_NUM], *releasers[HOLDER_NUM];

	srandom(time(NULL));

	// Execute many holds at the same time
	for (int i = 0; i < HOLDER_NUM; i++) {
		Thread::thr_func_t f_hold = boost::bind(holder, e);
		holders[i] = new Thread(f_hold);
		ASSERT_EQ(0, holders[i]->getCreateStatus());
	}
	for (int i = 0; i < HOLDER_NUM; i++) {
		holders[i]->join(NULL);
		delete holders[i];
	}

	// Execute many holds and releases at the same time
	for (int i = 0; i < HOLDER_NUM; i++) {
		Thread::thr_func_t f_rele = boost::bind(releaser, e);
		releasers[i] = new Thread(f_rele);
		ASSERT_EQ(0, releasers[i]->getCreateStatus());
	}
	for (int i = 0; i < HOLDER_NUM; i++) {
		Thread::thr_func_t f_hold = boost::bind(holder, e);
		holders[i] = new Thread(f_hold);
		ASSERT_EQ(0, holders[i]->getCreateStatus());
	}
	for (int i = 0; i < HOLDER_NUM; i++) {
		holders[i]->join(NULL);
		delete holders[i];
		releasers[i]->join(NULL);
		delete releasers[i];
	}

	// Execute many releases at the same time
	for (int i = 0; i < HOLDER_NUM; i++) {
		Thread::thr_func_t f_rele = boost::bind(releaser, e);
		releasers[i] = new Thread(f_rele);
		ASSERT_EQ(0, releasers[i]->getCreateStatus());
	}
	for (int i = 0; i < HOLDER_NUM; i++) {
		releasers[i]->join(NULL);
		delete releasers[i];
	}
	ASSERT_EQ(1U, e->getRefPointer()->pr_refcnt);
}

TEST(Event, hold)
{
	pfc_log_debug("Event_hold() is called");

	// normal test:
	Event *e1 = new Event(1, NULL);
	ASSERT_NE(PFC_EVENT_INVALID, e1->getEvent());
	test_hold_release(e1);
	Event::release(e1);
}

TEST(Event, release)
{
	pfc_log_debug("Event_release() is called");

	// normal test:
	// done with Event.hold
}

TEST(Event, getRefOps)
{
	pfc_log_debug("Event_getRefOps() is called");

	const pfc_refptr_ops_t *refOps = Event::getRefOperation();
	ASSERT_NE((pfc_refptr_ops_t *)NULL, refOps);

	// Check that we've gotten refOps correctly
	Event *e1, *e2;

	e1 = new Event(1, NULL);
	e2 = new Event(2, NULL);
	EXPECT_EQ(PFC_TRUE, refOps->equals(e1->getEvent(), e1->getEvent()));
	EXPECT_EQ(PFC_FALSE, refOps->equals(e1->getEvent(), e2->getEvent()));
	Event::release(e1);
	Event::release(e2);
}
