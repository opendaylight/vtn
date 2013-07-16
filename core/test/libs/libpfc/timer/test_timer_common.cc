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
#include "test_timer_common.h"

extern "C" {
extern void libpfc_init();
extern void libpfc_fini();
}

const pfc_timespec_t	timeout_t_c_ = {
	0, TIMEOUT * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC)
};

/*
 * Global test environment.
 */
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

void *
cb_timer_c(void *arg)
{
	int err;
	mytimeout_t *mtp;
	pfc_timespec_t curtime;
#ifndef HEAVY_LOAD_TEST
	pfc_timespec_t margin;
#endif	// HEAVY_LOAD_TEST

	// Get the current time
	err = pfc_clock_gettime(&curtime);
	if (err != 0) {
		pfc_log_fatal("Failed to get time");
		abort();
	}
	pfc_log_debug("Timer task function at %ld.%09ld sec",
			curtime.tv_sec, curtime.tv_nsec);

	// Verify that the callback function receive the user data correctly
	mtp = (mytimeout_t *)arg;
	EXPECT_EQ(EXPECTED_VAL, mtp->val);
#ifndef HEAVY_LOAD_TEST
	// Check this function is called in the scheduled time
	pfc_timespec_sub(&curtime, &mtp->abs_tout);
	if (mtp->resolution.tv_sec < 0 || mtp->resolution.tv_nsec < 0) {
		margin = resolution_c_;
	} else {
		margin = pfc_clock_compare(&resolution_c_, &mtp->resolution) > 0
				? resolution_c_: mtp->resolution;
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
	pfc_mutex_lock(&mtp->mutex);
	pfc_cond_signal(&mtp->cb_called);
	pfc_mutex_unlock(&mtp->mutex);

	return arg;
}

void *
dtor_timer_c(void *arg)
{
	mytimeout_t *mtp;

	mtp = (mytimeout_t *)arg;
	pfc_log_debug("Timer destructor function:");

	// Notify that the destructor has been called
	pfc_mutex_lock(&mtp->mutex);
	pfc_cond_signal(&mtp->dtor_called);
	pfc_mutex_unlock(&mtp->mutex);

	// Not cleanup resouces because they will be cleanuped 
	// after verifying this function is called

	return arg;
}

// Dummy pfc_phfunc_t function
void *
cb_timer_c_dummy(void *arg)
{
	return arg;
}

// Dummy pfc_phdtor_t function
void *
dtor_timer_c_dummy(void *arg)
{
	return arg;
}

void
initialize_mytimeout(mytimeout_t **mtpp, pfc_timespec_t resolution,
		pfc_bool_t invoke_cb, pfc_bool_t invoke_dtor)
{
	pfc_log_debug("Initialize the resources.");
	*mtpp = (mytimeout_t *)malloc(sizeof(mytimeout_t));
	(*mtpp)->val = EXPECTED_VAL;
	(*mtpp)->invoke_dtor = invoke_dtor;
	(*mtpp)->invoke_cb = invoke_cb;
	(*mtpp)->resolution = resolution;
	(*mtpp)->abs_tout.tv_sec = 0;
	(*mtpp)->abs_tout.tv_nsec = 0;
	ASSERT_EQ(0, pfc_sem_init(&(*mtpp)->sem, 0));
	ASSERT_EQ(0, PFC_MUTEX_INIT(&(*mtpp)->mutex));
	ASSERT_EQ(0, pfc_cond_init(&(*mtpp)->cb_called));
	ASSERT_EQ(0, pfc_cond_init(&(*mtpp)->dtor_called));

	pfc_log_debug("resolution=%ld.%09ld", resolution.tv_sec, resolution.tv_nsec);
}

void
cleanup_mytimeout(mytimeout_t *mtp)
{
	pfc_log_debug("Cleanup the resources.");
	ASSERT_EQ(0, pfc_sem_destroy(&mtp->sem));
	ASSERT_EQ(0, pfc_mutex_destroy(&mtp->mutex));
	ASSERT_EQ(0, pfc_cond_destroy(&mtp->cb_called));
	ASSERT_EQ(0, pfc_cond_destroy(&mtp->dtor_called));
	free(mtp);
	mtp = NULL;
}

void
initialize_timer_c_env(pfc_taskq_t *taskq, pfc_timer_t *tidp)
{
#ifdef TPOOL_NAME
	ASSERT_EQ(0, pfc_tpool_create(TPOOL_NAME));
	ASSERT_EQ(0, pfc_taskq_create(taskq, TPOOL_NAME, TASKQ_THREADS));
	ASSERT_EQ(0, pfc_timer_create(tidp, TPOOL_NAME, *taskq, NULL));
#else // TPOOL_NAME
	ASSERT_EQ(0, pfc_taskq_create(taskq, NULL, TASKQ_THREADS));
	ASSERT_EQ(0, pfc_timer_create(tidp, NULL, *taskq, NULL));
#endif // TPOOL_NAME
}

void
cleanup_timer_c_env(pfc_taskq_t taskq, pfc_timer_t tid)
{
	if (tid != PFC_TIMER_INVALID_ID) {
		ASSERT_EQ(0, pfc_timer_destroy(tid));
	}

	if (taskq != PFC_TASKQ_INVALID_ID) {
		ASSERT_EQ(0, pfc_taskq_destroy(taskq));
	}

#ifdef TPOOL_NAME
	ASSERT_EQ(0, pfc_tpool_destroy(TPOOL_NAME, NULL));
#endif // TPOOL_NAME
}

// Verify that a poller callback function is called.
void *
cb_timer_c_checker(void *arg)
{
	int err;
	mytimeout_t *mtp = NULL;

	pfc_log_debug("Wait for callback function to be called.");
	mtp = (mytimeout_t *)arg;
	pfc_mutex_lock(&mtp->mutex);
	pfc_sem_post(&mtp->sem);

	if (mtp->invoke_cb) {
		err = pfc_cond_wait(&mtp->cb_called, &mtp->mutex);
		EXPECT_EQ(0, err);
	} else {
		err = pfc_cond_timedwait(&mtp->cb_called, &mtp->mutex, &timeout_t_c_);
		EXPECT_EQ(ETIMEDOUT, err);
	}
	pfc_mutex_unlock(&mtp->mutex);
	
	return mtp;
}

// Verify that a poller destructor function is called.
void *
dtor_timer_c_checker(void *arg)
{
	int err;
	mytimeout_t *mtp;

	pfc_log_debug("Wait for destructor function to be called.");
	mtp = (mytimeout_t *)arg;
	pfc_mutex_lock(&mtp->mutex);
	pfc_sem_post(&mtp->sem);

	if (mtp->invoke_dtor) {
		err = pfc_cond_wait(&mtp->dtor_called, &mtp->mutex);
		EXPECT_EQ(0, err);
	} else {
		err = pfc_cond_timedwait(&mtp->dtor_called, &mtp->mutex, &timeout_t_c_);
		EXPECT_EQ(ETIMEDOUT, err);
	}
	pfc_mutex_unlock(&mtp->mutex);
	
	return arg;
}

void
test_timer_c_impl(pfc_bool_t invoke_cb, pfc_bool_t with_dtor, pfc_timespec_t tout,
		pfc_timespec_t resolution)
{
	pfc_thread_t tc_cb, tc_dtor;
	pfc_taskq_t taskq;
	pfc_timer_t tid;
	pfc_timeout_t toid;
	mytimeout_t *mtp = NULL;
	pfc_timespec_t curtime;
	pfc_bool_t invoke_dtor = with_dtor && invoke_cb;

	pfc_log_debug("invoke_cb=%d invoke_dtor=%d", invoke_cb, invoke_dtor);

	// Initialize
	initialize_mytimeout(&mtp, resolution, invoke_cb, invoke_dtor);
	initialize_timer_c_env(&taskq, &tid);

	// Create the checker threads
	ASSERT_EQ(0, pfc_thread_create(&tc_cb, &cb_timer_c_checker, (void *)mtp, 0));
	pfc_sem_wait(&mtp->sem);
	if (invoke_dtor) {
		ASSERT_EQ(0,
				pfc_thread_create(&tc_dtor, &dtor_timer_c_checker, (void *)mtp, 0));
		pfc_sem_wait(&mtp->sem);
	}

	// Get the current time
	ASSERT_EQ(0, pfc_clock_gettime(&curtime));
	pfc_log_debug("Post a task to the timer at %ld.%09ld sec",
			curtime.tv_sec, curtime.tv_nsec);

	// Post a task to the poller
	ASSERT_EQ(0, pfc_clock_abstime(&mtp->abs_tout, &tout));
	if (!with_dtor) {
		ASSERT_EQ(0, pfc_timer_post(tid, &tout, (pfc_taskfunc_t)cb_timer_c,
				(void *)mtp, &toid));
	} else {
		ASSERT_EQ(0, pfc_timer_post_dtor(tid, &tout, (pfc_taskfunc_t)cb_timer_c,
				(void *)mtp, (pfc_taskdtor_t)dtor_timer_c, &toid));
	}

	// Wait for the threads to exit
	pfc_thread_join(tc_cb, NULL);
	if (invoke_dtor) {
		pfc_thread_join(tc_dtor, NULL);
	}

	// Cleanup
	cleanup_timer_c_env(taskq, tid);
	cleanup_mytimeout(mtp);
}
