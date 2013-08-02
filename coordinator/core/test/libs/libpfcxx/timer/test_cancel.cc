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

static void
test_cancel_on_toid()
{
	TaskQueue *taskq = NULL;
	Timer *timer = NULL;
	timer_func_t t_func;
	pfc_timeout_t toid;
	MyTimeout *data;
	pfc_timespec_t res = ts_100msec;
	pfc_timespec_t tout = ts_1sec;

	{
		pfc_log_debug("Set a valid value as tid");

		// Initialize
		data = new MyTimeout(EXPECTED_VAL, PFC_TRUE, res);
		initialize_timer_cxx_env(taskq, timer);

		// Post a task to the timer
		t_func = boost::bind(cb_timer_cxx_dummy);
		ASSERT_EQ(0, timer->post(&tout, t_func, &toid));

		// Cancel the timer task
		ASSERT_EQ(0, timer->cancel(toid));

		// Cleanup
		cleanup_timer_cxx_env(taskq, timer);
		delete data;
	}

	{
		pfc_log_debug("Set an invalid value as tid");

		// Initialize
		data = new MyTimeout(EXPECTED_VAL, PFC_TRUE, res);
		initialize_timer_cxx_env(taskq, timer);

		// Post a task to the timer
		t_func = boost::bind(cb_timer_cxx_dummy);
		ASSERT_EQ(0, timer->post(&tout, t_func, &toid));

		// Cancel the timer task
		ASSERT_EQ(0, timer->cancel(toid));
		ASSERT_NE(0, timer->cancel(toid));

		// Cleanup
		cleanup_timer_cxx_env(taskq, timer);
		delete data;
	}
}

TEST(cancel, toid)
{
	test_cancel_on_toid();
}

static void
test_cancel(pfc_timespec_t tout)
{
	Thread *tc_cb = NULL;
	TaskQueue *taskq = NULL;
	Timer *timer = NULL;
	pfc_timeout_t toid;
	timer_func_t tfunc;
	MyTimeout *data = NULL;
	pfc_timespec_t curtime;
	pfc_timespec_t res = ts_100msec;
	pfc_timespec_t absTout;

	// Initialize
	data = new MyTimeout(EXPECTED_VAL, PFC_FALSE, res);
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
	ASSERT_EQ(0, pfc_clock_abstime(&absTout, &tout));
	data->setAbsTimeout(absTout);
	tfunc = boost::bind(cb_timer_cxx, data);
	ASSERT_EQ(0, timer->post(&tout, tfunc, &toid));

	// Wait for the threads to exit
	tc_cb->join(NULL);
	delete tc_cb;

	// Cleanup
	cleanup_timer_cxx_env(taskq, timer);
	delete data;
}

TEST(cancel, cancel_timeout)
{
	pfc_timespec_t tout = timeout_t_cc_;  // Set the enough value to cancel

	tout.tv_sec = tout.tv_sec * 10;
	pfc_log_debug("Cancel timer task");
	test_cancel(tout);
}
