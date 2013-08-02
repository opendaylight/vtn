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

#define EVS "evs_cxx_post"

TEST(post, name)
{
	char event_name[BUFSIZE];

	pfc_log_debug("post_name() is called");
	snprintf(event_name, sizeof(event_name), "event.post.name");

	test_name_cxx(GLOBALQ, PFC_FALSE);
	test_name_cxx(ASYNCQ, PFC_FALSE);
	test_name_cxx(LOCALQ, PFC_FALSE);
}

TEST(post, event)
{
	char event_name[BUFSIZE];

	pfc_log_debug("post_event() is called");
	snprintf(event_name, BUFSIZE, "%s_event", EVS);

	pfc_log_debug("Set a valid value as event");
	test_event_cxx_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
	test_event_cxx_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE, WITH_CNAME);
}
