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

#define EVS "evs_cxx_postEmergency"

TEST(postEmergency, name)
{
	char event_name[BUFSIZE];

	pfc_log_debug("postEmergency_name() is called");
	snprintf(event_name, BUFSIZE, "%s_name", EVS);

	test_name_cxx(GLOBALQ, PFC_TRUE);
	test_name_cxx(ASYNCQ, PFC_TRUE);
	test_name_cxx(LOCALQ, PFC_TRUE);
}

TEST(postEmergency, event)
{
	char event_name[BUFSIZE];

	pfc_log_debug("postEmergency_event() is called");
	snprintf(event_name, BUFSIZE, "%s_event", EVS);

	pfc_log_debug("Set a valid value as event");
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
}

#define	NOR_EV_CNT	50U
#define	NOR_EV_TYP	PFC_EVTYPE_MIN
#define	EMR_EV_TYP	PFC_EVTYPE_MAX

static volatile uint32_t	ev_cnt;
static volatile pfc_bool_t	ev_first;

#define	POST_EMERGENCY_SETUP()						\
	pfc_evhandler_t ehid;						\
	std::string sname;						\
	char event_name[BUFSIZE];					\
	Event *ev[NOR_EV_CNT];						\
	Event *ev_emr;							\
	uint32_t i;							\
									\
	ev_cnt = 0;							\
	ev_first = PFC_TRUE;						\
	snprintf(event_name, BUFSIZE, "%s_emergency", EVS);

#define	POST_EMERGENCY_POST()						\
	event_handler_t f_eh = boost::bind(cb_emr_hdr_cxx, _1);		\
	ASSERT_EQ(0,  Event::addHandler(ehid, event_name, f_eh));	\
									\
	for (i = 0; i < NOR_EV_CNT; i++) {				\
		ev[i] = new Event(NOR_EV_TYP, (pfc_ptr_t)NULL);		\
	}								\
	ev_emr = new Event(EMR_EV_TYP, (pfc_ptr_t)NULL);		\
									\
	for (i = 0; i < NOR_EV_CNT; i++) {				\
		ASSERT_EQ(0, Event::post(event_name, ev[i]));		\
	}								\
	ASSERT_EQ(0, Event::postEmergency(event_name, ev_emr));		\
									\
	ASSERT_EQ(0, Event::flush(event_name, NULL));			\
									\
	ASSERT_EQ(0, Event::removeHandler(ehid, NULL));			\
									\
	ASSERT_EQ(0, Event::unregisterEvent(event_name,			\
					    &timeout_ev_cc_));

void
cb_emr_hdr_cxx(Event *ev)
{
	pfc_timespec_t slp = {
		0, 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
	};

	if (ev->getType() == NOR_EV_TYP) {
		if (ev_first) {
			ev_first = PFC_FALSE;
			nanosleep(&slp, NULL);
		}
		pfc_atomic_inc_uint32((uint32_t *)&ev_cnt);
	} else {
		EXPECT_GT(NOR_EV_CNT, ev_cnt);
	}
}

TEST(postEmergency, global)
{
	POST_EMERGENCY_SETUP();

	// registration (Global Queue)
	ASSERT_EQ(0, Event::registerEvent(event_name));

	POST_EMERGENCY_POST();
}

TEST(postEmergency, async)
{
	POST_EMERGENCY_SETUP();

	// registration (Async Queue)
	ASSERT_EQ(0, Event::registerAsyncEvent(event_name));

	POST_EMERGENCY_POST();
}

TEST(postEmergency, local_1)
{
	POST_EMERGENCY_SETUP();

	// registration (Local Queue, single thread)
	ASSERT_EQ(0, Event::registerLocalEvent(event_name, 1));

	POST_EMERGENCY_POST();
}

TEST(postEmergency, local_2)
{
	POST_EMERGENCY_SETUP();

	// registration (Local Queue, double thread)
	ASSERT_EQ(0, Event::registerLocalEvent(event_name, 2));

	POST_EMERGENCY_POST();
}
