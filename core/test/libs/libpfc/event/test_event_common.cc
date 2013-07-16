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
#include "test_event_common.h"
#include "test_conf.h"

#define EVS_COMMON "e_c_common"

/*
 * Global test environment.
 */
extern "C" {
extern void libpfc_init();
extern void libpfc_fini();
}

const pfc_timespec_t timeout_ev_c_ = {
    0, TIMEOUT_COMMON * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC)
};

class TestEnvironment : public ::testing::Environment {
    protected:
	virtual void SetUp() {
		pfc_log_init("gtest", stderr, PFC_LOGLVL_NOTICE, NULL);
		test_sysconf_init("./pfcd.conf");
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
cb_event_c(pfc_event_t event, pfc_ptr_t arg)
{
	myevent_t *mep;
	myevent_t *mep2;

	pfc_log_debug("Event handler is invoked");

	// Verify that the callback function receive the user data correctly
	mep = (myevent_t *)arg;
	EXPECT_EQ(EXPECTED_VAL, mep->val);
	mep2 = (myevent_t *)pfc_event_data(event);
	EXPECT_EQ(EXPECTED_VAL, mep2->val);

	// Notify that this callback function has been called
	pfc_mutex_lock(&mep->mutex);
	pfc_cond_signal(&mep->cb_called);
	pfc_mutex_unlock(&mep->mutex);

	return arg;
}

void *
dtor_event_c(void *arg)
{
	myevent_t *mep;

	mep = (myevent_t *)arg;
	pfc_log_debug("Destructor function is invoked");

	// Notify that the destructor has been called
	pfc_mutex_lock(&mep->mutex);
	pfc_cond_signal(&mep->dtor_called);
	pfc_mutex_unlock(&mep->mutex);

	// Not cleanup resouces because they will be cleanuped 
	// after verifying this function is called

	return arg;
}

// Dummy pfc_phfunc_t function
void *
cb_event_dummy_c(void *arg)
{
	return arg;
}

// Dummy pfc_phdtor_t function
void *
dtor_event_dummy_c(void *arg)
{
	pfc_log_debug("dtor_event_dummy_c() is called");
	return arg;
}

void
initialize_myevent_c(myevent_t **mepp, pfc_bool_t invoke_cb, pfc_bool_t invoke_dtor)
{
	pfc_log_debug("Initialize the resources.");
	*mepp = (myevent_t *)malloc(sizeof(myevent_t));
	(*mepp)->val = EXPECTED_VAL;
	(*mepp)->invoke_dtor = invoke_dtor;
	(*mepp)->invoke_cb = invoke_cb;
	ASSERT_EQ(0, pfc_sem_init(&(*mepp)->sem, 0));
	ASSERT_EQ(0, PFC_MUTEX_INIT(&(*mepp)->mutex));
	ASSERT_EQ(0, pfc_cond_init(&(*mepp)->cb_called));
	ASSERT_EQ(0, pfc_cond_init(&(*mepp)->dtor_called));
}

void
cleanup_myevent_c(myevent_t *mep)
{
	pfc_log_debug("Cleanup the resources.");
	ASSERT_EQ(0, pfc_sem_destroy(&mep->sem));
	ASSERT_EQ(0, pfc_mutex_destroy(&mep->mutex));
	ASSERT_EQ(0, pfc_cond_destroy(&mep->cb_called));
	ASSERT_EQ(0, pfc_cond_destroy(&mep->dtor_called));
	free(mep);
	mep = NULL;
}

void
initialize_event_c_env()
{
	// Do nothing.
}

void
cleanup_event_c_env()
{
	// Do nothing.
}

// Verify that a poller callback function is called.
void *
cb_event_checker_c(void *arg)
{
	int err;
	myevent_t *mep = NULL;

	pfc_log_debug("Wait for callback function to be called.");
	mep = (myevent_t *)arg;
	pfc_mutex_lock(&mep->mutex);
	pfc_sem_post(&mep->sem);

	if (mep->invoke_cb) {
		err = pfc_cond_wait(&mep->cb_called, &mep->mutex);
		EXPECT_EQ(0, err);
	} else {
		err = pfc_cond_timedwait(&mep->cb_called, &mep->mutex, &timeout_ev_c_);
		EXPECT_EQ(ETIMEDOUT, err);
	}
	pfc_mutex_unlock(&mep->mutex);
	
	return mep;
}

// Verify that a poller destructor function is called.
void *
dtor_checker_c(void *arg)
{
	int err;
	myevent_t *mep;

	pfc_log_debug("Wait for destructor function to be called.");
	mep = (myevent_t *)arg;
	pfc_mutex_lock(&mep->mutex);
	pfc_sem_post(&mep->sem);

	if (mep->invoke_dtor) {
		err = pfc_cond_wait(&mep->dtor_called, &mep->mutex);
		EXPECT_EQ(0, err);
	} else {
		err = pfc_cond_timedwait(&mep->dtor_called, &mep->mutex, &timeout_ev_c_);
		EXPECT_EQ(ETIMEDOUT, err);
	}
	pfc_mutex_unlock(&mep->mutex);
	
	return arg;
}

void
register_event_c(char *event_name, TARGET_QUEUE qtype)
{
	pfc_log_debug("Register \"%s\" as event source.", event_name);
	switch (qtype) {
	case GLOBALQ:
		ASSERT_EQ(0, pfc_event_register(event_name));
		break;
	case ASYNCQ:
		ASSERT_EQ(0, pfc_event_register_async(event_name));
		break;
	case LOCALQ:
		ASSERT_EQ(0, pfc_event_register_local(event_name, LOCALQ_THREAD_NUM));
		break;
	default:
		pfc_log_fatal("Unknown event queue type");
		abort();
		// NOT REACHED
		break;
	}
}

void
test_event_c_impl(char *event_name, TARGET_QUEUE qtype, pfc_bool_t orphan, pfc_bool_t emergency)
{
	pfc_thread_t tc_cb, tc_dtor;
	pfc_event_t event;
	pfc_evhandler_t ehid;
	myevent_t *mep;

	// Setup
	initialize_myevent_c(&mep, 1, 1);

	// Register an event if this isn't an orphan handler test
	if (!orphan) {
		register_event_c(event_name, qtype);
	}

	// Create a checker thread
	pfc_log_debug("Create a checker thread");
	ASSERT_EQ(0, pfc_thread_create(&tc_cb, &cb_event_checker_c, (void *)mep, 0));
	pfc_sem_wait(&mep->sem);
	ASSERT_EQ(0, pfc_thread_create(&tc_dtor, &dtor_checker_c, (void *)mep, 0));
	pfc_sem_wait(&mep->sem);

	// Add an event handler
	pfc_log_debug("Add an event handler");
	ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
			(pfc_evfunc_t)&cb_event_c, (pfc_ptr_t)mep, (pfc_evmask_t)0, 0));

	// Register an event if this test is an orphan handler test.
	if (orphan) {
		register_event_c(event_name, qtype);
	}

	// Create an event
	pfc_log_debug("Create an event object");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)1, (pfc_ptr_t)mep,
			(pfc_evdtor_t)dtor_event_c));

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	if (!emergency) {
		ASSERT_EQ(0, pfc_event_post(event_name, event));
	} else {
		ASSERT_EQ(0, pfc_event_post_emergency(event_name, event));
	}

	// Wait for the checker threads to exit
	pfc_thread_join(tc_cb, NULL);
	pfc_thread_join(tc_dtor, NULL);

	// Unregister the event
	pfc_log_debug("Unregister \"%s\" event.", event_name);
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));

	// Cleanup
	cleanup_myevent_c(mep);
}

static void
test_name_abnormal_null(TARGET_QUEUE qtype, pfc_bool_t emergency)
{
	pfc_event_t event;
	pfc_evhandler_t ehid;

	// Register an event
	pfc_log_debug("Register NULL as event source.");
	switch (qtype) {
	case GLOBALQ:
		ASSERT_EQ(EINVAL, pfc_event_register(NULL));
		break;
	case ASYNCQ:
		ASSERT_EQ(EINVAL, pfc_event_register_async(NULL));
		break;
	case LOCALQ:
		ASSERT_EQ(EINVAL,
			  pfc_event_register_local(NULL, LOCALQ_THREAD_NUM));
		break;
	default:
		pfc_log_fatal("Unknown event queue type");
		abort();
		// NOT REACHED
		break;
	}

	// Add an event handler
	pfc_log_debug("Add an event handler associated with NULL event source");
	ASSERT_EQ(EINVAL,  pfc_event_add_handler(&ehid, NULL,
		  (pfc_evfunc_t)&cb_event_c, (pfc_ptr_t)NULL,
		  (pfc_evmask_t)0, 0));

	// Create an event object
	pfc_log_debug("Create an event object");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)1, (pfc_ptr_t)NULL,
			(pfc_evdtor_t)NULL));

	// Post the event object
	pfc_log_debug("Post the event object to NULL event source");
	if (!emergency) {
		ASSERT_EQ(EINVAL, pfc_event_post(NULL, event));
	} else {
		ASSERT_EQ(EINVAL, pfc_event_post_emergency(NULL, event));
	}

        pfc_log_debug("Flush NULL event source.");
        pfc_timespec_t  to;
        to.tv_sec = 1;
        to.tv_nsec = 0;
        ASSERT_EQ(EINVAL, pfc_event_flush(NULL, &to));

	// Unregister the event handler
	pfc_log_debug("Unregister NULL as event source.");
	ASSERT_EQ(EINVAL, pfc_event_unregister(NULL, &timeout_ev_c_));
}

static void
test_name_abnormal_noentry(char *event_name, TARGET_QUEUE qtype, pfc_bool_t emergency)
{
	pfc_event_t event;
	pfc_evhandler_t ehid;

	// Setup
	ASSERT_NE(ETIMEDOUT, pfc_event_unregister(event_name, &timeout_ev_c_));

	// Add an event handler
	pfc_log_debug("Add an event handler associated with not-exists event source");
	ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
			(pfc_evfunc_t)&cb_event_c, (pfc_ptr_t)NULL, (pfc_evmask_t)0, 0));

	// Create an event object
	pfc_log_debug("Create an event object");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)1, (pfc_ptr_t)NULL,
			(pfc_evdtor_t)NULL));

	// Post the event object
	pfc_log_debug("Post the event object to not-exists event source");
	if (!emergency) {
		ASSERT_EQ(ENOENT, pfc_event_post(event_name, event));
	} else {
		ASSERT_EQ(ENOENT, pfc_event_post_emergency(event_name, event));
	}

	// Unregister the event handler
	pfc_log_debug("Unregister not-exists event source.");
	ASSERT_EQ(0, pfc_event_remove_handler(ehid, &timeout_ev_c_));

	// Unregister the event source
	pfc_log_debug("Unregister not-exists event source.");
	ASSERT_EQ(ENOENT, pfc_event_unregister(event_name, &timeout_ev_c_));
}

void test_name_c(TARGET_QUEUE queue, pfc_bool_t emergency)
{
	char name[1024];

	pfc_log_debug("Apply the normal string as event source");
	snprintf(name, 11, "%s", EVS_COMMON);
	test_event_c_impl(name, queue, 0, emergency);

	pfc_log_debug("Apply a long string as event source");
	for (int i = 0; i < 100; i++) {
		strncat(name, "0123456789", 11);
	}
	test_event_c_impl(name, queue, 0, emergency);

	pfc_log_debug("Apply a short string as an event source");
	snprintf(name, 2, "e");
	test_event_c_impl(name, queue, 0, emergency);

	pfc_log_debug("Apply meta characters as an event source");
	snprintf(name, 15, "^$.*[...][^ ]@");
	test_event_c_impl(name, queue, 0, emergency);

	pfc_log_debug("Apply NULL as an event source");
	test_name_abnormal_null(queue, emergency);

	pfc_log_debug("Apply not-exists event source as an event source");
	snprintf(name, 19, "not-exists c event");
	test_name_abnormal_noentry(name, queue, emergency);

	pfc_log_debug("Check if an orphan handler is called correctly");
	test_event_c_impl(name, queue, 1, emergency);
}
