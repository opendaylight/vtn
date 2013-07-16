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

#define EVS "evs_cxx_unregEvent"

TEST(unregisterEvent, name)
{
	char event_name[BUFSIZE];

	pfc_log_debug("unregisterEvent_name() is called");
	snprintf(event_name, BUFSIZE, "%s_name", EVS);

	test_name_cxx(GLOBALQ, PFC_FALSE);
	test_name_cxx(ASYNCQ, PFC_FALSE);
	test_name_cxx(LOCALQ, PFC_FALSE);
}

static void
test_to(const TARGET_QUEUE &qtype)
{
	pfc_timespec_t timeout;
	char event_name[BUFSIZE];

	{
		pfc_log_debug("Set 0.0sec as timeout");

		// Register an event
		snprintf(event_name, BUFSIZE, "%s_timeout_0s", EVS);
		register_event_cxx(event_name, qtype);

		// Unregister the event with timeout
		timeout.tv_sec = 0;
		timeout.tv_nsec = 0;
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout));
	}

	{
		pfc_log_debug("Set 1nsec as timeout");

		// Register an event
		snprintf(event_name, BUFSIZE, "%s_timeout_1ns", EVS);
		register_event_cxx(event_name, qtype);

		// Unregister the event with timeout
		timeout.tv_sec = 0;
		timeout.tv_nsec = 1;
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout));
	}

	{
		pfc_log_debug("Set 1sec as timeout");

		// Register an event
		snprintf(event_name, BUFSIZE, "%s_timeout_10s", EVS);
		register_event_cxx(event_name, qtype);

		// Unregister the event with timeout
		timeout.tv_sec = 1;
		timeout.tv_nsec = 0;
		ASSERT_EQ(0, Event::unregisterEvent(event_name, &timeout));
	}

	{
		pfc_log_debug("Set NULL as timeout");

		// Register an event
		snprintf(event_name, BUFSIZE, "%s_timeout_null", EVS);
		register_event_cxx(event_name, qtype);

		// Unregister the event with timeout
		ASSERT_EQ(0, Event::unregisterEvent(event_name, NULL));
	}
}


TEST(unregisterEvent, to) {

	pfc_log_debug("unregisterEvent_to() is called");

	test_to(GLOBALQ);
	test_to(ASYNCQ);
	test_to(LOCALQ);
}
