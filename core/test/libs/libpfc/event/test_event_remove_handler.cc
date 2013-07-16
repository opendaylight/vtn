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

#define EVSRC "evs_remove_handler_c"

typedef struct myevent3 {
	pfc_mutex_t mutex;
	pfc_cond_t cond;
	pfc_sem_t sem;
	pfc_sem_t sem2;
	pfc_bool_t is_invoked;
	pfc_bool_t destroyed;
} myevent3_t;

#define	MYEVENT3_DTOR_WAIT(me3p, abstime)				\
	do {								\
		while (!(me3p)->destroyed) {				\
			ASSERT_EQ(0,					\
				  pfc_cond_timedwait_abs(&(me3p)->cond,	\
							 &(me3p)->mutex, \
							 (abstime)));	\
		}							\
	} while (0)

pfc_evhandler_t ehid_ebusy;

static void *
cb_event3_1(pfc_event_t event, pfc_ptr_t arg)
{
	int err;
	myevent3_t *me3p;

	pfc_log_debug("Event handler is called");

	// Notify that this event handler has been called
	me3p = (myevent3_t *)arg;

	err = pfc_mutex_lock(&me3p->mutex);
	if (err != 0) {
		pfc_log_fatal("pfc_mutex_lock failed(%d)", err);
		abort();
	}
	if (me3p->is_invoked != PFC_TRUE) {
		me3p->is_invoked = PFC_TRUE;
	}
	err = pfc_cond_signal(&me3p->cond);
	if (err != 0) {
		pfc_log_fatal("pfc_cond_signal failed(%d)", err);
		abort();
	}
	err = pfc_mutex_unlock(&me3p->mutex);
	if (err != 0) {
		pfc_log_fatal("pfc_mutex_unlock failed(%d)", err);
		abort();
	}

	// Wait for a signal permitting this event handler to exit
	pfc_sem_wait(&me3p->sem);

	return arg;
}

#ifdef TEST_EBUSY
static void *
cb_event3_2(pfc_ptr_t arg)
{
	myevent3_t *me3p;

	pfc_log_debug("cb_event3_2() is called");
	// Notify that this event handler has been called
	me3p = (myevent3_t *)arg;

	// Wait for a signal permitting this event handler to exit
	pfc_sem_wait(&me3p->sem2);

	EXPECT_EQ(EBUSY, pfc_event_remove_handler(ehid_ebusy, &timeout_ev_c_));

	return arg;
}
#endif /* TEST_EBUSY */

static void
dtor_timeout(pfc_ptr_t arg)
{
	myevent3_t *me3p = (myevent3_t *)arg;

	pfc_log_debug("dtor_timeout: Destructor function is called");
	pfc_mutex_lock(&me3p->mutex);
	me3p->destroyed = PFC_TRUE;
	pfc_cond_signal(&me3p->cond);
	pfc_mutex_unlock(&me3p->mutex);
}

static void
test_timeout(TARGET_QUEUE qtype, bool etimedout_test=false)
{
	pfc_evhandler_t ehid;
	pfc_evmask_t mask;
	pfc_event_t event;
	myevent3_t *me3p;
	char event_name[BUFSIZE];

	// Setup
	me3p = (myevent3_t *)malloc(sizeof(myevent3_t));
	ASSERT_EQ(0, pfc_mutex_init(&me3p->mutex, PFC_MUTEX_TYPE_NORMAL));
	ASSERT_EQ(0, pfc_cond_init(&me3p->cond));
	ASSERT_EQ(0, pfc_sem_init(&me3p->sem, 0));
	me3p->is_invoked = PFC_FALSE;
	me3p->destroyed = PFC_FALSE;

	// Register an event
	snprintf(event_name, BUFSIZE, "%s_timeout_check", EVSRC);
	register_event_c(event_name, qtype);

	// Add an event handler
	pfc_log_debug("Add event handler");
	pfc_event_mask_fill(&mask);
	ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
			(pfc_evfunc_t)cb_event3_1, (pfc_ptr_t)me3p, &mask, 0));

	// Create an event
	pfc_log_debug("Create event object");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)me3p,
				      dtor_timeout));

	// Post the event
	pfc_log_debug("Post event object to event source(\"%s\")", event_name);
	ASSERT_EQ(0, pfc_event_post(event_name, event));

	// Certify that the event handler has been invoked
	ASSERT_EQ(0, pfc_mutex_lock(&me3p->mutex));
	while (me3p->is_invoked == PFC_FALSE) {
		ASSERT_EQ(0, pfc_cond_wait(&me3p->cond, &me3p->mutex));
	}
	ASSERT_FALSE(me3p->destroyed);
	ASSERT_EQ(0, pfc_mutex_unlock(&me3p->mutex));

	pfc_timespec_t abstime;

	if (etimedout_test) {
		pfc_timespec_t  to = {
			0,
			200 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
		};

		// Try to remove the event handler.
		pfc_log_debug("Try to remove event handler which is still "
			      "alive");
		ASSERT_EQ(ETIMEDOUT, pfc_event_remove_handler(ehid, &to));

		// Permit the event handler to exit
		pfc_log_debug("Permit event handler to exit");
		ASSERT_EQ(0, pfc_sem_post(&me3p->sem));

		ASSERT_EQ(0, pfc_clock_abstime(&abstime, &timeout_ev_c_));
		ASSERT_EQ(0, pfc_mutex_lock(&me3p->mutex));
		MYEVENT3_DTOR_WAIT(me3p, &abstime);
		ASSERT_EQ(0, pfc_mutex_unlock(&me3p->mutex));

		// Remove the event handler
		pfc_log_debug("Remove event handler");
		ASSERT_EQ(ENOENT,
			  pfc_event_remove_handler(ehid, &timeout_ev_c_));
	}
	else {
		// Permit the event handler to exit
		pfc_log_debug("Permit event handler to exit");
		ASSERT_EQ(0, pfc_sem_post(&me3p->sem));

		// Remove the event handler
		pfc_log_debug("Remove event handler");
		ASSERT_EQ(0, pfc_event_remove_handler(ehid, &timeout_ev_c_));

		ASSERT_EQ(0, pfc_clock_abstime(&abstime, &timeout_ev_c_));
		ASSERT_EQ(0, pfc_mutex_lock(&me3p->mutex));
		MYEVENT3_DTOR_WAIT(me3p, &abstime);
		me3p->is_invoked = PFC_FALSE;
		me3p->destroyed = PFC_FALSE;
		ASSERT_EQ(0, pfc_mutex_unlock(&me3p->mutex));

		// Create an event
		pfc_log_debug("Create event object");
		ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0,
					      (pfc_ptr_t)me3p, dtor_timeout));

		// Post the event
		pfc_log_debug("Post event object to event source(\"%s\")",
			      event_name);
		ASSERT_EQ(0, pfc_event_post(event_name, event));

		// Certify that the removed event handler will be never invoked
		ASSERT_EQ(0, pfc_clock_abstime(&abstime, &timeout_ev_c_));
		ASSERT_EQ(0, pfc_mutex_lock(&me3p->mutex));
		MYEVENT3_DTOR_WAIT(me3p, &abstime);
		ASSERT_FALSE(me3p->is_invoked);
		ASSERT_EQ(0, pfc_mutex_unlock(&me3p->mutex));

		// Remove the not-exists handler
		pfc_log_debug("Remove the removed event handler");
		ASSERT_EQ(ENOENT,
			  pfc_event_remove_handler(ehid, &timeout_ev_c_));
	}

	// Unregister the event
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));

	// Cleanup
	ASSERT_EQ(0, pfc_mutex_destroy(&me3p->mutex));
	ASSERT_EQ(0, pfc_cond_destroy(&me3p->cond));
	ASSERT_EQ(0, pfc_sem_destroy(&me3p->sem));
	free(me3p);
}

#ifdef TEST_EBUSY
static void
test_event_remove_handler_ebusy(TARGET_QUEUE qtype)
{
	pfc_evmask_t mask;
	pfc_event_t event;
	myevent3_t *me3p;
	char event_name[BUFSIZE];
	pfc_thread_t pt;

	// Setup
	me3p = (myevent3_t *)malloc(sizeof(myevent3_t));
	ASSERT_EQ(0, pfc_mutex_init(&me3p->mutex, PFC_MUTEX_TYPE_NORMAL));
	ASSERT_EQ(0, pfc_cond_init(&me3p->cond));
	ASSERT_EQ(0, pfc_sem_init(&me3p->sem, 0));
	ASSERT_EQ(0, pfc_sem_init(&me3p->sem2, 0));
	me3p->is_invoked = PFC_FALSE;

	// Register an event
	snprintf(event_name, BUFSIZE, "%s_ebusy_check", EVSRC);
	register_event_c(event_name, qtype);

	// Add an event handler
	pfc_log_debug("Add event handler");
	pfc_event_mask_fill(&mask);
	ASSERT_EQ(0,  pfc_event_add_handler(&ehid_ebusy, event_name,
					    (pfc_evfunc_t)cb_event3_1,
					    (pfc_ptr_t)me3p, &mask, 0));

	// Create an event
	pfc_log_debug("Create event object");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0,
				      (pfc_ptr_t)me3p, (pfc_evdtor_t)NULL));

	// Post the event
	pfc_log_debug("Post event object to event source(\"%s\")", event_name);
	ASSERT_EQ(0, pfc_event_post(event_name, event));

	// Certify that the event handler has been invoked
	ASSERT_EQ(0, pfc_mutex_lock(&me3p->mutex));
	while (me3p->is_invoked == PFC_FALSE) {
		ASSERT_EQ(0, pfc_cond_wait(&me3p->cond, &me3p->mutex));
	}
	ASSERT_EQ(0, pfc_mutex_unlock(&me3p->mutex));

	// Try to remove the event handler.
	pfc_log_debug("Try to remove event handler which is still alive");
	ASSERT_EQ(0, pfc_thread_create(&pt, (pfc_thfunc_t)cb_event3_2,
				       (pfc_ptr_t)me3p, 0));
	ASSERT_EQ(0, pfc_sem_post(&me3p->sem2));
	ASSERT_EQ(ETIMEDOUT, pfc_event_remove_handler(ehid_ebusy,
						      &timeout_ev_c_));
	ASSERT_EQ(0, pfc_thread_join(pt, NULL));

	// Permit the event handler to exit
	pfc_log_debug("Permit event handler to exit");
	ASSERT_EQ(0, pfc_sem_post(&me3p->sem));

	// Remove the event handler
	pfc_log_debug("Remove event handler");
	ASSERT_EQ(0, pfc_event_remove_handler(ehid_ebusy, &timeout_ev_c_));

	// Create an event
	pfc_log_debug("Create event object");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)me3p,
				      (pfc_evdtor_t)NULL));

	// Post the event
	pfc_log_debug("Post event object to event source(\"%s\")", event_name);
	ASSERT_EQ(0, pfc_event_post(event_name, event));

	// Certify that the removed event handler will be never invoked
	ASSERT_EQ(0, pfc_mutex_lock(&me3p->mutex));
	ASSERT_EQ(ETIMEDOUT, pfc_cond_timedwait(&me3p->cond, &me3p->mutex,
						&timeout_ev_c_));
	ASSERT_EQ(0, pfc_mutex_unlock(&me3p->mutex));

	// Remove the not-exists handler
	pfc_log_debug("Remove the removed event handler");
	ASSERT_EQ(ENOENT, pfc_event_remove_handler(ehid_ebusy, 
						   &timeout_ev_c_));

	// Unregister the event
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));

	// Cleanup
	ASSERT_EQ(0, pfc_mutex_destroy(&me3p->mutex));
	ASSERT_EQ(0, pfc_cond_destroy(&me3p->cond));
	ASSERT_EQ(0, pfc_sem_destroy(&me3p->sem));
	ASSERT_EQ(0, pfc_sem_destroy(&me3p->sem2));
	free(me3p);
}
#endif /* TEST_EBUSY */

TEST(pfc_event_remove_handler, timeout)
{
	pfc_log_debug("pfc_event_remove_handler_timeout() is called");

	pfc_log_debug("Check timeout of removing event handler in global queue");
	test_timeout(GLOBALQ);
	test_timeout(GLOBALQ, true);

	pfc_log_debug("Check timeout of removing event handler in async queue");
	test_timeout(ASYNCQ);
	test_timeout(ASYNCQ, true);

	pfc_log_debug("Check timeout of removing event handler in local queue");
	test_timeout(LOCALQ);
	test_timeout(LOCALQ, true);
}

TEST(pfc_event_remove_handler, orphan)
{
	pfc_evhandler_t ehid;
	pfc_evmask_t mask;
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_remove_handler_orphan() is called");

	// Setup
	// Do nothing.

	// Register an event
	snprintf(event_name, BUFSIZE, "%s_remove_orphan", EVSRC);

	// Add an event handler
	pfc_log_debug("Add event handler");
	pfc_event_mask_fill(&mask);
	ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
			(pfc_evfunc_t)cb_event3_1, (pfc_ptr_t)NULL, &mask, 0));

	// Remove the event handler
	pfc_log_debug("Try to remove event handler which is still alive");
	ASSERT_EQ(0, pfc_event_remove_handler(ehid, &timeout_ev_c_));

	// Cleanup
	// Do nothing.
}

TEST(pfc_event_remove_handler, ebusy)
{
#ifdef TEST_EBUSY
	pfc_log_debug("pfc_event_remove_handler_ebusy() is called");
	test_event_remove_handler_ebusy(GLOBALQ);
	test_event_remove_handler_ebusy(ASYNCQ);
	test_event_remove_handler_ebusy(LOCALQ);
#endif /* TEST_EBUSY */
}
