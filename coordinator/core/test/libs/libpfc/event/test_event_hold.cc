/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <time.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "test_event_common.h"

#define	HOLD_COUNT	10

static void *
dtor_holder(void *arg)
{
	pfc_sem_t *sem;

	pfc_log_debug("dtor_holder() is called");
	sem = (pfc_sem_t *)arg;
	pfc_sem_post(sem);

	return arg;
}

static void *
dtor_holder_checker(void *arg)
{
	pfc_sem_t *sem;

	pfc_log_debug("dtor_holder_checker() is called");
	sem = (pfc_sem_t *)arg;
	pfc_sem_wait(sem);

	return arg;
}

static void
set_random_ts(pfc_timespec_t *ts)
{
	ts->tv_sec = 0;
	ts->tv_nsec = (random() % 10) * 5000000;
}

static void *
event_holder(void *arg)
{
	int i;
	pfc_event_t *event;
	pfc_timespec_t breaktime;

	pfc_log_debug("Event handler(holder) is called");
	set_random_ts(&breaktime);
	event = (pfc_event_t *)arg;
	for (i = 0; i < HOLD_COUNT; i++) {
		pfc_event_hold(*event);
		set_random_ts(&breaktime);
		::nanosleep(&breaktime, NULL);
	}
	return arg;
}

static void *
event_releaser(void *arg)
{
	int i;
	pfc_event_t *event;
	pfc_timespec_t breaktime;

	pfc_log_debug("Event handler(releaser) is called");
	event = (pfc_event_t *)arg;
	for (i = 0; i < HOLD_COUNT; i++) {
		pfc_event_release(*event);
		set_random_ts(&breaktime);
		::nanosleep(&breaktime, NULL);
	}
	return arg;
}

TEST(pfc_event_hold, event)
{
	pfc_log_debug("pfc_event_hold_event() is called");
	{
		int i;
		pfc_sem_t sem;
		pfc_event_t event;
		pfc_refptr_t *refptr;
		pfc_thread_t ts[HOLDER_NUM], tr[HOLDER_NUM], tc;

		pfc_log_debug("Set a valid value as event");

		// Setup
		ASSERT_EQ(0, pfc_sem_init(&sem, 0));

		// Create a event
		ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)&sem,
				(pfc_evdtor_t)dtor_holder));

		// Execute many pfc_event_holds at the same time
		for (i = 0; i < HOLDER_NUM; i++) {
			ASSERT_EQ(0, pfc_thread_create(&ts[i], &event_holder, (void *)&event, 0));
		}
		for (i = 0; i < HOLDER_NUM; i++) {
			pfc_thread_join(ts[i], NULL);
			pfc_log_debug("join thread %d", i);
		}

		// Execute many pfc_event_releases and pfc_event_holds at the same time
		for (i = 0; i < HOLDER_NUM; i++) {
			ASSERT_EQ(0, pfc_thread_create(&tr[i], &event_releaser, (void *)&event, 0));
		}
		for (i = 0; i < HOLDER_NUM; i++) {
			ASSERT_EQ(0, pfc_thread_create(&ts[i], &event_holder, (void *)&event, 0));
		}
		for (i = 0; i < HOLDER_NUM; i++) {
			pfc_thread_join(ts[i], NULL);
			pfc_thread_join(tr[i], NULL);
		}

		// Execute many pfc_event_releases at the same time
		for (i = 0; i < HOLDER_NUM; i++) {
			ASSERT_EQ(0, pfc_thread_create(&tr[i], &event_releaser, (void *)&event, 0));
		}
		for (i = 0; i < HOLDER_NUM; i++) {
			pfc_thread_join(tr[i], NULL);
		}

		// Get the refptr of the released event
		refptr = pfc_event_refptr(event);
		ASSERT_EQ(1U, refptr->pr_refcnt);

		// Create checker thread
		ASSERT_EQ(0, pfc_thread_create(&tc, &dtor_holder_checker, (void *)&sem, 0));

		pfc_event_release(event); // for this main thread

		// Certify that dtor function has been called
		pfc_log_debug("Wait for dtor to be called.");
		pfc_thread_join(tc, NULL);
	}
}
