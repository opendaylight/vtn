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

#define EVSRC "evs_unreg_c"

typedef struct {
	pfc_mutex_t mutex;
	pfc_cond_t cond;
	pfc_sem_t sem;
} myevent4_t;

TEST(pfc_event_unregister, name)
{
	pfc_log_debug("pfc_event_unregister_name() is called");

	pfc_log_debug("Unregister the event source in global queue");
	test_name_c(GLOBALQ, 0);

	pfc_log_debug("Unregister the event source in async queue");
	test_name_c(ASYNCQ, 0);

	pfc_log_debug("Unregister the event source in local queue");
	test_name_c(LOCALQ, 0);
}
