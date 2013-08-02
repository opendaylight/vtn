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

TEST(pfc_event_type, event)
{
	pfc_log_debug("pfc_event_type_event() is called");

	{
		pfc_evtype_t evtype;
		pfc_event_t event;

		pfc_log_debug("Set valid event as event");

		// Setup
		evtype = (pfc_evtype_t)31;

		// Create an event
		ASSERT_EQ(0, pfc_event_create(&event, evtype, (pfc_ptr_t)NULL,
				(pfc_evdtor_t)dtor_event_dummy_c));

		// Compare
		ASSERT_EQ(evtype, pfc_event_type(event));

		// Delete the event
		pfc_event_release(event);
	}
}
