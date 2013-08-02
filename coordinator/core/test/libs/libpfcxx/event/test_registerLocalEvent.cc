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

#define EVS "evs_cxx_regLocalEvent"

TEST(registerLocalEvent, name)
{
	char event_name[BUFSIZE];
	snprintf(event_name, BUFSIZE, "%s_name", EVS);

	test_name_cxx(LOCALQ, PFC_FALSE);

	{
		pfc_log_debug("Overwrite");
		snprintf(event_name, BUFSIZE, "%s_name_ow", EVS);
		ASSERT_EQ(0, Event::registerLocalEvent(event_name, LOCALQ_THREAD_NUM));
#ifndef SKIP_ON_LRT
		ASSERT_EQ(EEXIST, Event::registerLocalEvent(event_name, LOCALQ_THREAD_NUM));
#endif
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_)); // Cleanup
	}

	{
		pfc_log_debug("Rewrite");
		snprintf(event_name, BUFSIZE, "%s_name_rw", EVS);
		ASSERT_EQ(0, Event::registerLocalEvent(event_name, LOCALQ_THREAD_NUM));
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_));
		ASSERT_EQ(0, Event::registerLocalEvent(event_name, LOCALQ_THREAD_NUM)); // Rewrite
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout_ev_cc_)); // Cleanup
	}
}

TEST(registerLocalEevnt, maxthreads)
{
	char event_name[BUFSIZE];

	pfc_log_debug("registerLocalEvent_name() is called");
	snprintf(event_name, BUFSIZE, "%s_mt", EVS);

	pfc_log_debug("Specify 0 as maxthreads");
	ASSERT_EQ(0, Event::registerLocalEvent(event_name, 0));
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_cc_)); // Cleanup

	pfc_log_debug("Specify 1 as maxthreads");
	ASSERT_EQ(0, Event::registerLocalEvent(event_name, 1));
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_cc_)); // Cleanup

	pfc_log_debug("Specify -1 as maxthreads");
	ASSERT_EQ(0, Event::registerLocalEvent(event_name, -1));
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_cc_)); // Cleanup
}
