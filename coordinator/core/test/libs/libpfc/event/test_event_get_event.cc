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

TEST(pfc_event_get_event, event)
{
	pfc_log_debug("pfc_event_get_event_event() is called");

	{
		pfc_event_t event, event2;
		pfc_refptr_t *refptr;

		pfc_log_debug("Set valid event as event");

		ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)NULL,
				(pfc_evdtor_t)dtor_event_dummy_c));
		refptr = pfc_event_refptr(event);
		event2 = pfc_event_get_event(refptr);
		ASSERT_EQ(event, event2);

		pfc_event_release(event);
	}

	{
		pfc_log_debug("Set invalid event as event");
		ASSERT_EQ(pfc_event_get_event(NULL), PFC_EVENT_INVALID);
	}

}
