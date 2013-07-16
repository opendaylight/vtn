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

TEST(pfc_event_serial, event)
{
	pfc_log_debug("pfc_event_serial_event() is called");

	{
		int i, j;
		pfc_evid_t evid[EVENT_NUM];
		pfc_event_t event[EVENT_NUM];

		pfc_log_debug("Set a valid event as event");

		// Create events
		for (i = 0; i < EVENT_NUM; i++) {
			ASSERT_EQ(0, pfc_event_create(&event[i], (pfc_evtype_t)0,
					(pfc_ptr_t)NULL, (pfc_evdtor_t)dtor_event_dummy_c));
		}

		// Get the serial of the event
		for (i = 0; i < EVENT_NUM; i++) {
			evid[i] = pfc_event_serial(event[i]);
		}

		// Compare
		for (i = 0; i < EVENT_NUM; i++) {
			for (j = i + 1; j < EVENT_NUM; j++) {
				ASSERT_NE(evid[i], evid[j]);
			}
		}

		// Delete the event
		for (i = 0; i < EVENT_NUM; i++) {
			pfc_event_release(event[i]);
		}
	}
}
