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

#define EVSRC "evs_register_local_c"

TEST(pfc_event_register_local, name)
{
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_register_local_name() is called");

	pfc_log_debug("Register event source to local queue");
	test_name_c(LOCALQ, 0);

	{
		pfc_log_debug("Overwrite event source in local queue");
		snprintf(event_name, BUFSIZE, "%s_name_overwrite", EVSRC);
		ASSERT_EQ(0, pfc_event_register_local(event_name, LOCALQ_THREAD_NUM));
#ifndef SKIP_ON_LRT
		// Overwrite
		ASSERT_EQ(EEXIST,
			  pfc_event_register_local(event_name,
						   LOCALQ_THREAD_NUM));
#endif
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_)); // Cleanup
	}

	{
		pfc_log_debug("Rewrite event source in local queue");
		snprintf(event_name, BUFSIZE, "%s_name_rewrite", EVSRC);
		ASSERT_EQ(0, pfc_event_register_local(event_name, LOCALQ_THREAD_NUM));
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));
		ASSERT_EQ(0, pfc_event_register_local(event_name, LOCALQ_THREAD_NUM)); // Rewrite
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_)); // Cleanup
	}
}

TEST(pfc_event_register_local, maxthreads)
{
	char event_name[BUFSIZE];

	pfc_log_debug("Set 0 as maxthreads");
	snprintf(event_name, BUFSIZE, "%s_mt_0", EVSRC);
	ASSERT_EQ(0, pfc_event_register_local(event_name, 0));
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_)); // Cleanup

	pfc_log_debug("Set 1 as maxthreads");
	snprintf(event_name, BUFSIZE, "%s_mt_1", EVSRC);
	ASSERT_EQ(0, pfc_event_register_local(event_name, 1));
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_)); // Cleanup

	pfc_log_debug("Set UINT32_MAX as maxthreads");
	snprintf(event_name, BUFSIZE, "%s_mt_uint32_max", EVSRC);
	ASSERT_EQ(0, pfc_event_register_local(event_name, UINT32_MAX));
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_)); // Cleanup
}
