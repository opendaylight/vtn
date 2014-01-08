/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_event_common.h"

#define EVSRC "evs_post_c"

TEST(pfc_event_post, name)
{
	pfc_log_debug("pfc_event_post_name() is called");

	pfc_log_debug("Post to the event source in global queue");
	test_name_c(GLOBALQ, PFC_FALSE);
	test_name_c(GLOBALQ, PFC_TRUE);

	pfc_log_debug("Post to the event source in async queue");
	test_name_c(ASYNCQ, PFC_FALSE);
	test_name_c(ASYNCQ, PFC_TRUE);

	pfc_log_debug("Post to the event source in local queue");
	test_name_c(LOCALQ, PFC_FALSE);
	test_name_c(LOCALQ, PFC_TRUE);
}

TEST(pfc_event_post, event)
{
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_post_event() is called");
	snprintf(event_name, BUFSIZE, "%s_event", EVSRC);

	pfc_log_debug("Set valid value as event");
	test_event_c_impl(event_name, GLOBALQ, 0, PFC_FALSE);
	test_event_c_impl(event_name, ASYNCQ, 0, PFC_FALSE);
	test_event_c_impl(event_name, LOCALQ, 0, PFC_FALSE);

	test_event_c_impl(event_name, GLOBALQ, 0, PFC_TRUE);
	test_event_c_impl(event_name, ASYNCQ, 0, PFC_TRUE);
	test_event_c_impl(event_name, LOCALQ, 0, PFC_TRUE);
}

#define	EV_COUNT	50U
#define	EV_NOR_TYP	PFC_EVTYPE_MIN
#define	EV_EMR_TYP	PFC_EVTYPE_MAX

static volatile uint32_t	ev_count;
static volatile pfc_bool_t	fist_ev;

#define	EV_SETUP()							\
									\
	char	event_name_emr[BUFSIZE];				\
	pfc_evhandler_t	ehid_emr;					\
	pfc_event_t	ev[EV_COUNT];					\
	pfc_event_t	ev_emr;						\
	uint32_t	i;						\
									\
	ev_count = 0;							\
	fist_ev = PFC_TRUE;						\
									\
	snprintf(event_name_emr, BUFSIZE, "%s_emergency", EVSRC);

#define	EV_POST()							\
									\
	ASSERT_EQ(0,  pfc_event_add_handler(&ehid_emr, event_name_emr,	\
				    (pfc_evfunc_t)&ev_hdr_emergency_c,	\
				    (pfc_ptr_t)NULL,			\
				    (pfc_evmask_t *)NULL, 0));		\
									\
	for(i = 0; i < EV_COUNT; i++) {					\
		ASSERT_EQ(0, PFC_EVENT_CREATE(&ev[i], EV_NOR_TYP));	\
	}								\
	ASSERT_EQ(0, PFC_EVENT_CREATE(&ev_emr, EV_EMR_TYP));		\
									\
	for(i = 0; i < EV_COUNT; i++) {					\
		ASSERT_EQ(0, pfc_event_post(event_name_emr, ev[i]));	\
	}								\
									\
	ASSERT_EQ(0, pfc_event_post_emergency(event_name_emr, ev_emr));	\
									\
	ASSERT_EQ(0, pfc_event_flush(event_name_emr, NULL));		\
	ASSERT_EQ(0, pfc_event_remove_handler(ehid_emr, NULL));		\
	ASSERT_EQ(0, pfc_event_unregister(event_name_emr, NULL));

void *
ev_hdr_emergency_c(pfc_event_t event, pfc_ptr_t arg)
{
	pfc_timespec_t slp = {
		0, 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
	};

	if (pfc_event_type(event) == EV_NOR_TYP) {
		if (fist_ev) {
			fist_ev = PFC_FALSE;
			nanosleep(&slp, NULL);
		}
		pfc_atomic_inc_uint32((uint32_t *)&ev_count);
	} else {
		EXPECT_GT(EV_COUNT, ev_count);
	}
	return arg;
}

TEST(pfc_event_post, emergency_GLB)
{
	EV_SETUP();

	ASSERT_EQ(0, pfc_event_register(event_name_emr));

	EV_POST();
}

TEST(pfc_event_post, emergency_ASY)
{
	EV_SETUP();

	ASSERT_EQ(0, pfc_event_register_async(event_name_emr));

	EV_POST();
}

TEST(pfc_event_post, emergency_LOC1)
{
	EV_SETUP();

	ASSERT_EQ(0, pfc_event_register_local(event_name_emr, 1));

	EV_POST();
}

TEST(pfc_event_post, emergency_LOC2)
{
	EV_SETUP();

	ASSERT_EQ(0, pfc_event_register_local(event_name_emr, 2));

	EV_POST();
}
