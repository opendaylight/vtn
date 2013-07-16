/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_timer_common.hh"

#define TOID_NUM 1000

static void
test_timeout()
{
	TaskQueue *taskq = NULL;
	Timer *timer = NULL;
	timer_func_t t_func;
	pfc_timeout_t toid;
	pfc_timespec_t tout;

	// Test1:
	{
		// Initialize
		initialize_timer_cxx_env(taskq, timer);

		// Post a valid timespec
		tout = ts_1sec;
		t_func = boost::bind(cb_timer_cxx_dummy);
		EXPECT_EQ(0, timer->post(&tout, t_func, &toid));

		// Cleanup
		cleanup_timer_cxx_env(taskq, timer);
	}

	// Test2:
	// Give an invalid value
	{
		// Initialize
		initialize_timer_cxx_env(taskq, timer);

		tout.tv_sec = 0;
		tout.tv_nsec = -1;
		EXPECT_EQ(EINVAL, timer->post(&tout, t_func, &toid));

		// Cleanup
		cleanup_timer_cxx_env(taskq, timer);
	}
}

TEST(post, timeout) {
	pfc_log_debug("post_timeout() is called");

	test_timeout();
}

static void
test_func(pfc_timespec_t resolution)
{
	pfc_timespec_t tout;

	// Test1:
	pfc_log_debug("Invoke a task function in 0.0 sec.");
	tout = ts_0sec;
	test_timer_cxx_impl(PFC_TRUE, tout, resolution);

	// Test2:
	pfc_log_debug("Invoke a task function in 1 nsec.");
	tout = ts_1nsec;
	test_timer_cxx_impl(PFC_TRUE, tout, resolution);

	// Test3:
	pfc_log_debug("Invoke a task function in 1 usec.");
	tout = ts_1usec;
	test_timer_cxx_impl(PFC_TRUE, tout, resolution);

	// Test4:
	pfc_log_debug("Invoke a task function in 1 msec.");
	tout =  ts_1msec;
	test_timer_cxx_impl(PFC_TRUE, tout, resolution);

	// Test5:
	pfc_log_debug("Invoke a task function in 100 msec.");
	tout =  ts_100msec;
	test_timer_cxx_impl(PFC_TRUE, tout, resolution);
}

TEST(post, func) {
	pfc_log_debug("post_func() is called");

	pfc_log_debug("Apply 0sec as resolution");
	test_func(ts_0sec);

	pfc_log_debug("Apply 50msec as resolution");
	test_func(ts_50msec);

	pfc_log_debug("Apply a negative value as resolution");
	test_func(ts_invalid);
}

static void
test_toidp()
{
	TaskQueue *taskq = NULL;
	Timer *timer = NULL;
	timer_func_t t_func;
	MyTimeout *data = NULL;

	// Test1:
	// Post tasks to the poller
	{
		int i, j;
		pfc_timeout_t toid[TOID_NUM];

		// Initialize
		data = new MyTimeout(EXPECTED_VAL, PFC_TRUE, resolution_cc_);
		initialize_timer_cxx_env(taskq, timer);

		t_func = boost::bind(cb_timer_cxx_dummy);
		for (i = 0; i < TOID_NUM; i++) {
			EXPECT_EQ(0, timer->post(&ts_100msec, t_func, &toid[i]));
		}
		for (i = 0; i < TOID_NUM; i++) {
			for (j = i + 1; j < TOID_NUM; j++) {
				EXPECT_NE(toid[i], toid[j]);
			}
		}

		// Cleanup
		cleanup_timer_cxx_env(taskq, timer);
		delete data;
	}

	// Test2:
	// Set NULL as toidp
	{
		Thread *tc_cb = NULL;
		pfc_timeout_t toid;
		const pfc_timespec_t *tout = &ts_100msec;
		pfc_timespec_t curtime;
		pfc_timespec_t absTout;

		// Initialize
		data = new MyTimeout(EXPECTED_VAL, PFC_TRUE, resolution_cc_);
		initialize_timer_cxx_env(taskq, timer);

		// Create the checker threads
		pfc_log_debug("Create a checker thread");
		Thread::thr_func_t f_cb_chk = boost::bind(cb_timer_cxx_checker, data);
		tc_cb = Thread::create(f_cb_chk);
		ASSERT_NE((Thread *)NULL, tc_cb);
		data->sema()->wait();

		// Get the current time
		ASSERT_EQ(0, pfc_clock_gettime(&curtime));
		pfc_log_debug("Post a task to the timer at %ld.%09ld sec", curtime.tv_sec, curtime.tv_nsec);

		// Post a task to the poller
		ASSERT_EQ(0, pfc_clock_abstime(&absTout, tout));
		data->setAbsTimeout(absTout);
		t_func = boost::bind(cb_timer_cxx, data);
		ASSERT_EQ(0, timer->post(tout, t_func, &toid));

		// Wait for the threads to exit
		tc_cb->join(NULL);
		delete tc_cb;

		// Cleanup
		cleanup_timer_cxx_env(taskq, timer);
		delete data;
	}
}

TEST(post, toidp) {
	pfc_log_debug("post_toidp() is called");

	test_toidp();
}
