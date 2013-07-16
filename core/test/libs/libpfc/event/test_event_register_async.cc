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

#define EVSRC "evs_register_async_c"

TEST(pfc_event_register_async, name)
{
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_register_async() is called");

	pfc_log_debug("Register event source to async queue");
	test_name_c(ASYNCQ, 0);

	{
		pfc_log_debug("Overwrite event source in async queue");
		snprintf(event_name, BUFSIZE, "%s_name_overwrite", EVSRC);
		ASSERT_EQ(0, pfc_event_register_async(event_name));
#ifndef SKIP_ON_LRT
		// Overwrite
		ASSERT_EQ(EEXIST, pfc_event_register_async(event_name));
#endif
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_)); // Cleanup
	}

	{
		pfc_log_debug("Rewrite event source in async queue");
		snprintf(event_name, BUFSIZE, "%s_name_rewrite", EVSRC);
		ASSERT_EQ(0, pfc_event_register_async(event_name));
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));
		ASSERT_EQ(0, pfc_event_register_async(event_name)); // Rewrite
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_)); // Cleanup
	}
}
