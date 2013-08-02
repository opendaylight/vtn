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

static void
test_cancel_on_tid(pfc_bool_t with_dtor)
{
	pfc_timespec_t res = ts_100msec;
	pfc_timespec_t tout = ts_100msec;

	{
		pfc_taskq_t taskq;
		pfc_timer_t tid;
		pfc_timeout_t toid;
		mytimeout_t *mtp;

		pfc_log_debug("Set a valid value as tid");

		// Initialize
		initialize_mytimeout(&mtp, res, PFC_TRUE, PFC_TRUE);
		initialize_timer_c_env(&taskq, &tid);

		// Post a task to the timer
		if (!with_dtor) {
			ASSERT_EQ(0, pfc_timer_post(tid, &tout,
								(pfc_taskfunc_t)cb_timer_c_dummy, (void *)mtp, &toid));
		} else {
			ASSERT_EQ(0, pfc_timer_post_dtor(tid, &tout,
					(pfc_taskfunc_t)cb_timer_c_dummy, (void *)mtp,
					(pfc_taskdtor_t)dtor_timer_c_dummy, &toid));
		}

		// Cancel the timer task
		ASSERT_EQ(0, pfc_timer_cancel(tid, toid));

		// Cleanup
		cleanup_timer_c_env(taskq, tid);
		cleanup_mytimeout(mtp);
	}

	{
		pfc_taskq_t taskq;
		pfc_timer_t tid;
		pfc_timeout_t toid;
		mytimeout_t *mtp;

		pfc_log_debug("Set an invalid value as tid");

		// Initialize
		initialize_mytimeout(&mtp, res, PFC_TRUE, PFC_TRUE);
		initialize_timer_c_env(&taskq, &tid);

		// Post a task to the timer
		if (!with_dtor) {
			ASSERT_EQ(0, pfc_timer_post(tid, &tout,
					(pfc_taskfunc_t)cb_timer_c_dummy, (void *)mtp, &toid));
		} else {
			ASSERT_EQ(0,
					pfc_timer_post_dtor(tid, &tout,	(pfc_taskfunc_t)cb_timer_c_dummy,
							(void *)mtp, (pfc_taskdtor_t)dtor_timer_c_dummy, &toid));
		}

		// Destroy the timer
		ASSERT_EQ(0, pfc_timer_destroy(tid));

		// Cancel the timer task with the invalid tid
		ASSERT_EQ(ENOENT, pfc_timer_cancel(tid, toid));

		// Cleanup
		cleanup_timer_c_env(taskq, PFC_TIMER_INVALID_ID);
		cleanup_mytimeout(mtp);
	}
}

TEST(pfc_timer_cancel, tid)
{
	pfc_log_debug("pfc_timer_cancel_tid() is called");

	pfc_log_debug("Cancel with pid posted by pfc_timer_post");
	test_cancel_on_tid(PFC_FALSE);

	pfc_log_debug("Cancel with pid posted by pfc_timer_post_dtor");
	test_cancel_on_tid(PFC_TRUE);
}

static void
test_cancel_on_toid(pfc_bool_t with_dtor)
{
	pfc_timespec_t res = ts_100msec;
	pfc_timespec_t tout = ts_100msec;

	{
		pfc_taskq_t taskq;
		pfc_timer_t tid;
		pfc_timeout_t toid;
		mytimeout_t *mtp;

		pfc_log_debug("Set a valid value as tid");

		// Initialize
		initialize_mytimeout(&mtp, res, PFC_TRUE, PFC_TRUE);
		initialize_timer_c_env(&taskq, &tid);

		// Post a task to the timer
		if (!with_dtor) {
			ASSERT_EQ(0, pfc_timer_post(tid, &tout,
					(pfc_taskfunc_t)cb_timer_c_dummy, (void *)mtp, &toid));
		} else {
			ASSERT_EQ(0,
					pfc_timer_post_dtor(tid, &tout,	(pfc_taskfunc_t)cb_timer_c_dummy,
							(void *)mtp, (pfc_taskdtor_t)dtor_timer_c_dummy, &toid));
		}

		// Cancel the timer task
		ASSERT_EQ(0, pfc_timer_cancel(tid, toid));

		// Cleanup
		cleanup_timer_c_env(taskq, tid);
		cleanup_mytimeout(mtp);
	}

	{
		pfc_taskq_t taskq;
		pfc_timer_t tid;
		pfc_timeout_t toid;
		mytimeout_t *mtp;

		pfc_log_debug("Set an invalid value as tid");

		// Initialize
		pfc_log_debug("Setup");
		initialize_mytimeout(&mtp, res, PFC_TRUE, PFC_TRUE);
		initialize_timer_c_env(&taskq, &tid);

		// Post a task to the timer
		pfc_log_debug("Post a task to the timer");
		if (!with_dtor) {
			ASSERT_EQ(0, pfc_timer_post(tid, &tout,
					(pfc_taskfunc_t)cb_timer_c_dummy, (void *)mtp, &toid));
		} else {
			ASSERT_EQ(0,
					pfc_timer_post_dtor(tid, &tout, (pfc_taskfunc_t)cb_timer_c_dummy,
							(void *)mtp, (pfc_taskdtor_t)dtor_timer_c_dummy, &toid));
		}

		// Cancel the timer task
		pfc_log_debug("Cancel the timer");
		ASSERT_EQ(0, pfc_timer_cancel(tid, toid));

		// Cancel the timer task with the invalid toid
		pfc_log_debug("Cancel the timer");
		ASSERT_EQ(ESRCH, pfc_timer_cancel(tid, toid));

		// Cleanup
		pfc_log_debug("Cleanup");
		cleanup_timer_c_env(taskq, tid);
		cleanup_mytimeout(mtp);
	}
}

TEST(pfc_timer_cancel, toid)
{
	pfc_log_debug("pfc_timer_cancel_toid() is called");

	pfc_log_debug("Cancel with fd posted by pfc_poller_post");
	test_cancel_on_toid(PFC_FALSE);

	pfc_log_debug("Cancel with fd posted by pfc_timer_post_dtor");
	test_cancel_on_toid(PFC_TRUE);
}

static void
test_cancel(pfc_bool_t with_dtor, pfc_timespec_t tout)
{
	pfc_thread_t tc_cb, tc_dtor;
	pfc_taskq_t taskq;
	pfc_timer_t tid;
	pfc_timeout_t toid;
	mytimeout_t *mtp = NULL;
	pfc_timespec_t curtime;
	pfc_timespec_t res = ts_100msec;

	// Initialize
	initialize_mytimeout(&mtp, res, PFC_FALSE, PFC_FALSE);
	initialize_timer_c_env(&taskq, &tid);

	// Create the checker threads
	ASSERT_EQ(0, pfc_thread_create(&tc_cb, &cb_timer_c_checker, (void *)mtp, 0));
	pfc_sem_wait(&mtp->sem);
	if (with_dtor) {
		ASSERT_EQ(0, pfc_thread_create(&tc_dtor, &dtor_timer_c_checker, (void *)mtp, 0));
		pfc_sem_wait(&mtp->sem);
	}

	// Get the current time
	ASSERT_EQ(0, pfc_clock_gettime(&curtime));
	pfc_log_debug("Post a task to the timer at %ld.%09ld sec",
			curtime.tv_sec, curtime.tv_nsec);

	// Post a task to the timer
	ASSERT_EQ(0, pfc_clock_abstime(&mtp->abs_tout, &tout));
	if (with_dtor) {
		ASSERT_EQ(0, pfc_timer_post(tid, &tout, (pfc_taskfunc_t)cb_timer_c,
				(void *)mtp, &toid));
	} else {
		ASSERT_EQ(0, pfc_timer_post_dtor(tid, &tout, (pfc_taskfunc_t)cb_timer_c,
				(void *)mtp, (pfc_taskdtor_t)dtor_timer_c, &toid));
	}

	// Cancel the task
	ASSERT_EQ(0, pfc_timer_cancel(tid, toid));

	// Wait for the threads to exit
	pfc_thread_join(tc_cb, NULL);
	if (with_dtor) {
		pfc_thread_join(tc_dtor, NULL);
	}

	// Cleanup
	cleanup_timer_c_env(taskq, tid);
	cleanup_mytimeout(mtp);
}

TEST(pfc_timer_cancel, canceling)
{
	pfc_timespec_t tout = timeout_t_c_;  // Set the enough value to cancel

	tout.tv_sec = tout.tv_sec * 10;
	pfc_log_debug("pfc_timer_cancel_canceling() is called");

	pfc_log_debug("Cancel the polling posted by pfc_timer_post");
	test_cancel(PFC_FALSE, tout);

	pfc_log_debug("Cancel the polling posted by pfc_timer_post_dtor");
	test_cancel(PFC_TRUE, tout);
}
