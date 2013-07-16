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

TEST(pfc_event_refops, event)
{
	int i, j;
	uint32_t ehash[EVENT_NUM];
	pfc_event_t event[EVENT_NUM];
	const pfc_refptr_ops_t* ops;

	pfc_log_debug("pfc_event_refops_event() is called");

	// Create events
	for (i = 0; i < EVENT_NUM; i++) {
		ASSERT_EQ(0, pfc_event_create(&event[i], (pfc_evtype_t)0, (pfc_ptr_t)NULL,
				(pfc_evdtor_t)dtor_event_dummy_c));
	}

	// Get refops
	ops = pfc_event_refops();
	ASSERT_NE((const pfc_refptr_ops_t *)NULL, ops);

	pfc_log_debug("Test of compare()");
	ASSERT_EQ(0, ops->compare(event[0], event[0]));
	ASSERT_EQ(-1, ops->compare(event[0], event[1]));
	ASSERT_EQ(1, ops->compare(event[1], event[0]));

	pfc_log_debug("Test of equals()");
	ASSERT_EQ(PFC_TRUE, ops->equals(event[0], event[0]));
	ASSERT_EQ(PFC_FALSE, ops->equals(event[0], event[1]));

	pfc_log_debug("Test of hashfunc()");
	for (i = 0; i < EVENT_NUM; i++) {
		ehash[i] = ops->hashfunc(event[i]);
		ASSERT_EQ(ehash[i], ops->hashfunc(event[i]));
	}
	for (i = 0; i < EVENT_NUM; i++) {
		for (j = i + 1; j < EVENT_NUM; j++) {
			ASSERT_NE(ehash[i], ehash[j]);
		}
	}

	for (i = 0; i < EVENT_NUM; i++) {
		pfc_event_release(event[i]);
	}
}
