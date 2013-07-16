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

#define TOID_NUM 10000

static void
test_func(pfc_bool_t with_dtor, pfc_timespec_t resolution)
{
	pfc_timespec_t tout;

	// Test1:
	pfc_log_debug("Invoke a task function in 0.0 sec.");
	tout = ts_0sec;
	test_timer_c_impl(PFC_TRUE, with_dtor, tout, resolution);

	// Test2:
	pfc_log_debug("Invoke a task function in 1 nsec.");
	tout = ts_1nsec;
	test_timer_c_impl(PFC_TRUE, with_dtor, tout, resolution);

	// Test3:
	pfc_log_debug("Invoke a task function in 1 usec.");
	tout = ts_1usec;
	test_timer_c_impl(PFC_TRUE, with_dtor, tout, resolution);

	// Test4:
	pfc_log_debug("Invoke a task function in 1 msec.");
	tout =  ts_1msec;
	test_timer_c_impl(PFC_TRUE, with_dtor, tout, resolution);

	// Test5:
	pfc_log_debug("Invoke a task function in 50 msec.");
	tout =  ts_50msec;
	test_timer_c_impl(PFC_TRUE, with_dtor, tout, resolution);
}

TEST(pfc_timer_post, func)
{
	pfc_log_debug("pfc_timer_post_func() is called");

	pfc_log_debug("Apply 0sec as resolution");
	test_func(PFC_FALSE, ts_0sec);

	pfc_log_debug("Apply 100msec as resolution");
	test_func(PFC_FALSE, ts_100msec);

	pfc_log_debug("Apply a negative value as resolution");
	test_func(PFC_FALSE, ts_invalid);
}

TEST(pfc_timer_post_dtor, func)
{
	pfc_log_debug("pfc_timer_post_dtor_func() is called");

	pfc_log_debug("Apply 0sec resolution");
	test_func(PFC_TRUE, ts_0sec);

	pfc_log_debug("Apply 100msec resolution");
	test_func(PFC_TRUE, ts_100msec);

	pfc_log_debug("Apply a negative value as resolution");
	test_func(PFC_TRUE, ts_invalid);
}

static void
test_tid(pfc_bool_t with_dtor)
{
	pfc_taskq_t taskq;
	pfc_timer_t tid;
	pfc_timeout_t toid;
	pfc_timespec_t tout = {1, 0};

	// Create a timer and delete it immediately
	initialize_timer_c_env(&taskq, &tid);
	cleanup_timer_c_env(taskq, tid);

	// Test1:
	// Post a task to the not-exist timer
	if (!with_dtor) {
		EXPECT_EQ(ENOENT, pfc_timer_post(tid, &tout,
		  (pfc_taskfunc_t)cb_timer_c_dummy, NULL, &toid));
	} else {
		EXPECT_EQ(ENOENT, pfc_timer_post_dtor(tid, &tout,
			  (pfc_taskfunc_t)cb_timer_c_dummy, NULL, 
			  (pfc_taskdtor_t)dtor_timer_c_dummy, &toid));
	}

	// Setup a valid test environment
	initialize_timer_c_env(&taskq, &tid);

	// Test2:
	// Give a valid value
	if (!with_dtor) {
		EXPECT_EQ(0, pfc_timer_post(tid, &tout, (pfc_taskfunc_t)cb_timer_c_dummy,
				NULL, &toid));
	} else {
		EXPECT_EQ(0,
				pfc_timer_post_dtor(tid, &tout, (pfc_taskfunc_t)cb_timer_c_dummy, NULL,
						(pfc_taskdtor_t)dtor_timer_c_dummy, &toid));
	}

	// Cleanup
	cleanup_timer_c_env(taskq, tid);
}

TEST(pfc_timer_post, tid)
{
	pfc_log_debug("pfc_timer_post_tid() is called");
	test_tid(PFC_FALSE);
}

TEST(pfc_timer_post_dtor, tid)
{
	pfc_log_debug("pfc_timer_post_dtor_tid() is called");
	test_tid(PFC_TRUE);
}

static void
test_timeout(pfc_bool_t with_dtor)
{
	pfc_taskq_t taskq;
	pfc_timer_t tid;
	pfc_timeout_t toid;
	pfc_timespec_t tout;

	// Initialize
	initialize_timer_c_env(&taskq, &tid);

	// Test1:
	// Post a valid timespec
	tout = ts_1sec;
	if (!with_dtor) {
		EXPECT_EQ(0, pfc_timer_post(tid, &tout, (pfc_taskfunc_t)cb_timer_c_dummy,
				NULL, &toid));
	} else {
		EXPECT_EQ(0,
				pfc_timer_post_dtor(tid, &tout, (pfc_taskfunc_t)cb_timer_c_dummy,
						&toid, (pfc_taskdtor_t)dtor_timer_c_dummy, &toid));
	}

	// Test2:
	// Give an invalid value
	tout.tv_nsec = -1;
	EXPECT_EQ(EINVAL, pfc_timer_post(tid, &tout,
		  (pfc_taskfunc_t)cb_timer_c_dummy, NULL, &toid));

	// Cleanup
	cleanup_timer_c_env(taskq, tid);
}

TEST(pfc_timer_post, timeout)
{
	pfc_log_debug("pfc_timer_post_timeout() is called");
	test_timeout(0);
}

TEST(pfc_timer_post_dtor, timeout)
{
	pfc_log_debug("pfc_timer_post_dtor_timeout() is called");
	test_timeout(1);
}

static void
test_arg(pfc_bool_t with_dtor)
{
	pfc_timespec_t res = ts_100msec;
	pfc_timespec_t tout = ts_100msec;

	pfc_log_debug("Verify that arg is passed to func correctly.");
	test_timer_c_impl(PFC_TRUE, with_dtor, tout, res);

	{
		pfc_taskq_t taskq;
		pfc_timer_t tid;
		pfc_timeout_t toid;
		mytimeout_t *mtp = NULL;

		pfc_log_debug("Verify that NULL value can be set as arg");

		// Initialize
		initialize_mytimeout(&mtp, ts_100msec, PFC_TRUE, with_dtor);
		initialize_timer_c_env(&taskq, &tid);

		// Post a task to the poller
		if (!with_dtor) {
			EXPECT_EQ(0, pfc_timer_post(tid, &tout,
					(pfc_taskfunc_t)cb_timer_c_dummy, (void *)NULL, &toid));
		} else {
			EXPECT_EQ(0, pfc_timer_post_dtor(tid, &tout,
					(pfc_taskfunc_t)cb_timer_c_dummy, (void *)NULL,
					(pfc_taskdtor_t)dtor_timer_c_dummy, &toid));
		}

		// Cleanup
		cleanup_timer_c_env(taskq, tid);
		cleanup_mytimeout(mtp);
	}
}

TEST(pfc_timer_post, arg)
{
	pfc_log_debug("pfc_timer_post_arg() is called");
	test_arg(PFC_FALSE);
}

TEST(pfc_timer_post_dtor, arg)
{
	pfc_log_debug("pfc_timer_post_dtor_arg() is called");
	test_arg(PFC_TRUE);
}

static void
test_toidp(pfc_bool_t with_dtor)
{
	pfc_taskq_t taskq;
	pfc_timer_t tid;
	mytimeout_t *mtp = NULL;

	// Initialize
	initialize_mytimeout(&mtp, ts_100msec, PFC_TRUE, with_dtor);
	initialize_timer_c_env(&taskq, &tid);

	// Test1:
	// Post tasks to the poller
	{
		int i, j;
		pfc_timeout_t toid[TOID_NUM];

		for (i = 0; i < TOID_NUM; i++) {
			if (!with_dtor) {
				EXPECT_EQ(0, pfc_timer_post(tid, &ts_100msec,
						(pfc_taskfunc_t)cb_timer_c_dummy, (void *)NULL, &toid[i]));
			} else {
				EXPECT_EQ(0, pfc_timer_post_dtor(tid, &ts_100msec,
						(pfc_taskfunc_t)cb_timer_c_dummy, (void *)NULL,
						(pfc_taskdtor_t)dtor_timer_c_dummy, &toid[i]));
			}
		}
		for (i = 0; i < TOID_NUM; i++) {
			for (j = i + 1; j < TOID_NUM; j++) {
				EXPECT_NE(toid[i], toid[j]);
			}
		}
	}

	// Test2:
	// Set NULL as toidp
	{
		pfc_thread_t tc_cb, tc_dtor;
		const pfc_timespec_t *tout = &ts_100msec;
		pfc_timespec_t curtime;

		// Create the checker threads
		ASSERT_EQ(0,
				pfc_thread_create(&tc_cb, &cb_timer_c_checker, (void *)mtp, 0));
		pfc_sem_wait(&mtp->sem);
		if (with_dtor) {
			ASSERT_EQ(0,
					pfc_thread_create(&tc_dtor, &dtor_timer_c_checker, (void *)mtp, 0));
			pfc_sem_wait(&mtp->sem);
		}

		// Get the current time
		ASSERT_EQ(0, pfc_clock_gettime(&curtime));
		pfc_log_debug("Post a task to the timer at %ld.%09ld sec",
				curtime.tv_sec, curtime.tv_nsec);

		// Post a task to the poller
		ASSERT_EQ(0, pfc_clock_abstime(&mtp->abs_tout, tout));
		if (!with_dtor) {
			ASSERT_EQ(0, pfc_timer_post(tid, tout, (pfc_taskfunc_t)cb_timer_c,
					(void *)mtp, NULL));
		} else {
			ASSERT_EQ(0, pfc_timer_post_dtor(tid, tout, (pfc_taskfunc_t)cb_timer_c,
					(void *)mtp, (pfc_taskdtor_t)dtor_timer_c, NULL));
		}

		// Wait for the threads to exit
		pfc_thread_join(tc_cb, NULL);
		if (with_dtor) {
			pfc_thread_join(tc_dtor, NULL);
		}
	}
	// Cleanup
	cleanup_timer_c_env(taskq, tid);
	cleanup_mytimeout(mtp);
}

TEST(pfc_timer_post, toidp)
{
	pfc_log_debug("pfc_timer_post_toidp() is called");
	test_toidp(0);
	test_toidp(1);
}

TEST(pfc_timer_post_dtor, dtor)
{
	pfc_timespec_t res = ts_100msec;
	pfc_timespec_t tout = ts_100msec;

	pfc_log_debug("pfc_timer_post_dtor_dtor() is called");

	{
		pfc_log_debug("Examine to be called when the task is invoked");
		test_func(PFC_TRUE, res);
	}

	{
		pfc_thread_t tc_dtor;
		pfc_taskq_t taskq;
		pfc_timer_t tid;
		pfc_timeout_t toid;
		mytimeout_t *mtp;

		pfc_log_debug("Examine to be called when canceling the timeout");

		// Initialize
		initialize_mytimeout(&mtp, res, PFC_TRUE, PFC_TRUE);
		initialize_timer_c_env(&taskq, &tid);

		// Create the checker thread
		ASSERT_EQ(0,
				pfc_thread_create(&tc_dtor, &dtor_timer_c_checker, (void *)mtp, 0));
		pfc_sem_wait(&mtp->sem);

		// Post a task to the timer
		ASSERT_EQ(0,
				pfc_timer_post_dtor(tid, &tout, (pfc_taskfunc_t)cb_timer_c_dummy,
						(void *)mtp, (pfc_taskdtor_t)dtor_timer_c, &toid));

		// Cancel the timer task
		ASSERT_EQ(0, pfc_timer_cancel(tid, toid));

		// Wait for the thread to exit
		pfc_thread_join(tc_dtor, NULL);

		// Cleanup
		cleanup_timer_c_env(taskq, tid);
		cleanup_mytimeout(mtp);
	}

	{
		pfc_thread_t tc_dtor;
		pfc_taskq_t taskq;
		pfc_timer_t tid;
		pfc_timeout_t toid;
		mytimeout_t *mtp;

		pfc_log_debug("Examine to be called when deleting the timer");

		// Initialize
		initialize_mytimeout(&mtp, res, PFC_TRUE, PFC_TRUE);
		initialize_timer_c_env(&taskq, &tid);

		// Create the checker thread
		ASSERT_EQ(0,
				pfc_thread_create(&tc_dtor, &dtor_timer_c_checker, (void *)mtp, 0));
		pfc_sem_wait(&mtp->sem);

		// Post a task to the timer
		ASSERT_EQ(0,
				pfc_timer_post_dtor(tid, &tout,	(pfc_taskfunc_t)cb_timer_c_dummy,
						(void *)mtp, (pfc_taskdtor_t)dtor_timer_c, &toid));

		// Overwrite the polling
		ASSERT_EQ(0, pfc_timer_destroy(tid));

		// Wait for the thread to exit
		pfc_thread_join(tc_dtor, NULL);

		// Cleanup
		cleanup_timer_c_env(taskq, PFC_TIMER_INVALID_ID);
		cleanup_mytimeout(mtp);
	}

	{
		pfc_taskq_t taskq;
		pfc_timer_t tid;
		pfc_timeout_t toid;
		mytimeout_t *mtp;
		pfc_timespec_t tout = ts_100msec;

		pfc_log_debug("Set NULL as destructor function");

		// Initialize
		initialize_mytimeout(&mtp, ts_100msec, PFC_TRUE, PFC_TRUE);
		initialize_timer_c_env(&taskq, &tid);

		// Post a task to the timer
		ASSERT_EQ(0, pfc_timer_post_dtor(tid, &tout,
				(pfc_taskfunc_t)cb_timer_c_dummy, (void *)mtp,
				(pfc_taskdtor_t)dtor_timer_c, &toid));

		// Cleanup
		cleanup_timer_c_env(taskq, tid);
		cleanup_mytimeout(mtp);
	}
}
