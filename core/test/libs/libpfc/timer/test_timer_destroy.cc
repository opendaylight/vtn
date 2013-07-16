/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_timer_common.h"

TEST(pfc_timer_destroy, tid)
{
	pfc_log_debug("pfc_timer_destroy() is called");

	{
		pfc_log_debug("Set a valid value as tid");
		test_timer_c_impl(PFC_TRUE, PFC_FALSE, ts_50msec, ts_20msec);
		test_timer_c_impl(PFC_TRUE, PFC_TRUE, ts_200msec, ts_100msec);
	}

	// Test2:
	{
		pfc_log_debug("Set an invalid value as tid");
		ASSERT_EQ(ENOENT, pfc_timer_destroy(PFC_TIMER_INVALID_ID));
	}
}
