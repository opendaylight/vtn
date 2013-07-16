/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include <pfc/log.h>
#include "test_timer_common.hh"


extern "C" {
extern void libpfc_init();
extern void libpfc_fini();
}

const pfc_timespec_t   timeout_t_cc_ = {
	0, TIMEOUT * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC)
};

class TestEnvironment : public ::testing::Environment {
    protected:
	virtual void SetUp() {
		pfc_log_init("gtest", stderr, PFC_LOGLVL_NOTICE, NULL);
		libpfc_init();
	}
	virtual void TearDown() {
		pfc_log_fini();
		libpfc_fini();
	}
};

::testing::Environment  *global_env =
	  ::testing::AddGlobalTestEnvironment(new TestEnvironment());

using namespace std;
using namespace pfc::core;

void
cb_timer_cxx(MyTimeout *data)
{
	int err;
	pfc_timespec_t curtime;
	pfc_timespec_t tmptime;
#ifndef HEAVY_LOAD_TEST
	pfc_timespec_t margin;
#endif	// HEAVY_LOAD_TEST

	// Get the current time
	err = pfc_clock_gettime(&curtime);
	if (err != 0) {
		pfc_log_fatal("Failed to get time");
		abort();
	}
	pfc_log_debug("Timer task function:");
	pfc_log_debug("    invoked at %ld.%09ld sec", curtime.tv_sec, curtime.tv_nsec);

	// Verify that the callback function receive the user data correctly
	EXPECT_EQ(EXPECTED_VAL, data->val());
	tmptime = data->getAbsTimeout();
	pfc_timespec_sub(&curtime, &tmptime);

#ifndef HEAVY_LOAD_TEST
	// Check this function is called in the scheduled time
	if (data->getResolution().tv_sec < 0 || data->getResolution().tv_nsec < 0) {
		margin = resolution_cc_;
	} else {
		tmptime = data->getResolution();
		margin = pfc_clock_compare(&resolution_cc_, &tmptime) > 0
				? resolution_cc_: data->getResolution();
	}
	pfc_timespec_add(&margin, &margin);
	pfc_log_debug("Difference: %ld.%09ld sec", curtime.tv_sec, curtime.tv_nsec);
	pfc_log_debug("Threshold: %ld.%09ld sec", margin.tv_sec, margin.tv_nsec);
	err = pfc_clock_compare(&curtime, &margin);
	if (err > 0) {
		pfc_log_fatal("Failed to assure the resolution");
		abort();
	}
#endif	// HEAVY_LOAD_TEST

	// Notify that this callback function has been called
	data->mutex()->lock();
	data->cond()->signal();
	data->mutex()->unlock();
}

// Dummy pfc_phfunc_t function
void
cb_timer_cxx_dummy(void)
{
	pfc_log_debug("Timer func:");
}

void
initialize_timer_cxx_env(TaskQueue *&taskq, Timer *&timer)
{
#ifdef TPOOL_NAME
	string tpool_name(TPOOL_NAME);
	ASSERT_EQ(0, ThreadPool::create(tpool_name));
	taskq = TaskQueue::create(tpool_name, TASKQ_THREADS);
	ASSERT_NE((TaskQueue *)NULL, taskq);
	timer = Timer::create(taskq->getId(), tpool_name, &resolution_cc_);
	ASSERT_NE((Timer *)NULL, timer);
#else // TPOOL_NAME
	taskq = TaskQueue::create(TASKQ_THREADS);
	ASSERT_NE((TaskQueue *)NULL, taskq);
	timer = Timer::create(taskq->getId(), &resolution_cc_);
	ASSERT_NE((Timer *)NULL, timer);
#endif // TPOOL_NAME
}

void
cleanup_timer_cxx_env(TaskQueue *&taskq, Timer *&timer)
{
	if (timer != PFC_TIMER_INVALID_ID) {
		delete timer;
	}

	if (taskq != PFC_TASKQ_INVALID_ID) {
		delete taskq;
	}

#ifdef TPOOL_NAME
	ASSERT_EQ(0, ThreadPool::destroy(TPOOL_NAME, NULL));
#endif // TPOOL_NAME
}

// Verify that a poller callback function is called.
void *
cb_timer_cxx_checker(MyTimeout *data)
{
	int err;

	pfc_log_debug("Wait for callback function to be called.");
	data->mutex()->lock();
	data->sema()->post();

	if (data->isInvoked()) {
		data->cond()->wait(*data->mutex());
	} else {
		err = data->cond()->timedwait(*data->mutex(), timeout_t_cc_);
		EXPECT_EQ(ETIMEDOUT, err);
	}
	data->mutex()->unlock();
	pfc_log_debug("Wait for callback function to be called2.");
	return (void *)data;
}

void
test_timer_cxx_impl(pfc_bool_t invoke_cb, pfc_timespec_t tout, pfc_timespec_t resolution)
{
	TaskQueue *taskq = NULL;
	Timer *timer = NULL;
	MyTimeout *data = NULL;
	Thread *tc_cb = NULL;
	pfc_timeout_t toid;
	pfc_timespec_t curtime;
	pfc_timespec_t absTout;
	pfc_log_debug("invoke_cb=%d", invoke_cb);

	// Initialize
	initialize_timer_cxx_env(taskq, timer);
	data = new MyTimeout(EXPECTED_VAL, invoke_cb, resolution);

	// Create the checker threads
	pfc_log_debug("Create a checker thread");
	if (invoke_cb) {
		Thread::thr_func_t f_cb_chk = boost::bind(cb_timer_cxx_checker, data);
		tc_cb = Thread::create(f_cb_chk);
		ASSERT_NE((Thread *)NULL, tc_cb);
		data->sema()->wait();
	}

	// Get the current time
	ASSERT_EQ(0, pfc_clock_gettime(&curtime));

	// Post a task to the poller
	ASSERT_EQ(0, pfc_clock_abstime(&absTout, &tout));
	data->setAbsTimeout(absTout);
	timer_func_t tfunc = boost::bind(cb_timer_cxx, data);
	ASSERT_EQ(0, timer->post(&tout, tfunc, &toid));

	pfc_log_debug("Post a task at %ld.%09ld sec", curtime.tv_sec, curtime.tv_nsec);
	pfc_log_debug("  scheduled at %ld.%09ld sec", absTout.tv_sec, absTout.tv_nsec);

	if (invoke_cb) {
		// Wait for the threads to exit
		tc_cb->join(NULL);
		delete tc_cb;
	}

	// Cleanup
	cleanup_timer_cxx_env(taskq, timer);
	delete data;
}
