/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_event_common.h"

#define EVSRC "evs_create_c"

TEST(pfc_event_create, idp)
{
	pfc_event_t event;

	pfc_log_debug("pfc_event_create_idp() is called");

	pfc_log_debug("Set valid value as idp.");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)NULL,
			(pfc_evdtor_t)NULL));
	pfc_event_release(event);
}

TEST(pfc_event_create, type)
{
	pfc_event_t event, old;

	pfc_log_debug("pfc_event_create_type() is called");

	pfc_log_debug("Set -1 as type");
	EXPECT_EQ(EINVAL,
		  pfc_event_create(&event, (pfc_evtype_t)-1, (pfc_ptr_t)NULL,
				   (pfc_evdtor_t)dtor_event_dummy_c));

	pfc_log_debug("Set 32 as type");
	EXPECT_EQ(EINVAL,
		  pfc_event_create(&event, (pfc_evtype_t)32, (pfc_ptr_t)NULL,
				   (pfc_evdtor_t)dtor_event_dummy_c));

	pfc_log_debug("Set 0 as type");
	EXPECT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)NULL,
			(pfc_evdtor_t)dtor_event_dummy_c));
	old = event;

	pfc_log_debug("Set 15 as type");
	EXPECT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)15, (pfc_ptr_t)NULL,
			(pfc_evdtor_t)dtor_event_dummy_c));
	EXPECT_NE(old, event);
	pfc_event_release(old);
	old = event;

	pfc_log_debug("Set 31 as type");
	EXPECT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)31, (pfc_ptr_t)NULL,
			(pfc_evdtor_t)dtor_event_dummy_c));
	EXPECT_NE(old, event);
	pfc_event_release(old);

	pfc_event_release(event);
}

TEST(pfc_event_create, data)
{
	pfc_event_t event;
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_create_data is called");
	snprintf(event_name, BUFSIZE, "%s_data", EVSRC);

	pfc_log_debug("Set valid value as data");
	test_event_c_impl(event_name, GLOBALQ, 0, 0);

	pfc_log_debug("Set NULL as data");
	EXPECT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)NULL,
			(pfc_evdtor_t)dtor_event_dummy_c));

	pfc_event_release(event);
}

TEST(pfc_event_create, dtor)
{
	pfc_event_t event;
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_create_dtor() is called");
	snprintf(event_name, BUFSIZE, "%s_dtor", EVSRC);

	pfc_log_debug("Set valid value as dtor");
	test_event_c_impl(event_name, GLOBALQ, 0, 0);

	pfc_log_debug("Set NULL as dtor");
	EXPECT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)NULL,
			(pfc_evdtor_t)NULL));

	pfc_event_release(event);
}

TEST(PFC_EVENT_CREATE, eventp)
{
	pfc_event_t event;

	pfc_log_debug("PFC_EVENT_CREATE_eventp() is called");

	pfc_log_debug("Set valid value as idp.");
	ASSERT_EQ(0, PFC_EVENT_CREATE(&event, (pfc_evtype_t)0));

	pfc_event_release(event);
}

TEST(PFC_EVENT_CREATE, type)
{
	pfc_event_t event, old;

	pfc_log_debug("PFC_EVENT_CREATE_type() is called");

	pfc_log_debug("Set -1 as type");
	EXPECT_EQ(EINVAL, PFC_EVENT_CREATE(&event, (pfc_evtype_t)-1));

	pfc_log_debug("Set 32 as type");
	EXPECT_EQ(EINVAL, PFC_EVENT_CREATE(&event, (pfc_evtype_t)32));

	pfc_log_debug("Set 0 as type");
	EXPECT_EQ(0, PFC_EVENT_CREATE(&event, (pfc_evtype_t)0));
	old = event;

	pfc_log_debug("Set 15 as type");
	EXPECT_EQ(0, PFC_EVENT_CREATE(&event, (pfc_evtype_t)15));
	EXPECT_NE(old, event);
	pfc_event_release(old);
	old = event;

	pfc_log_debug("Set 31 as type");
	EXPECT_EQ(0, PFC_EVENT_CREATE(&event, (pfc_evtype_t)31));
	EXPECT_NE(old, event);
	pfc_event_release(old);
	pfc_event_release(event);
}

