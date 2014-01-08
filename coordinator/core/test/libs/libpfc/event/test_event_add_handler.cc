/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <inttypes.h>
#include <pfc/base.h>
#include <gtest/gtest.h>
#include "test_event_common.h"

#define EVSRC "evs_add_handler_c"
#define PRI_LOWEST 100
#define PRI_HIGHEST 0

typedef struct myevent2 {
	uint32_t next_priority;
	uint32_t counter;
	pfc_sem_t sem;
	pfc_cond_t cond;
	pfc_mutex_t mutex;
} myevent2_t;

TEST(pfc_event_add_handler, idp)
{
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_add_handler_idp() is called");

	{
		pfc_log_debug("Set a valid value as idp to each queue");
		test_name_c(GLOBALQ, PFC_FALSE);
		test_name_c(ASYNCQ, PFC_FALSE);
		test_name_c(LOCALQ, PFC_FALSE);
	}

	{
		pfc_log_debug("Set NULL as idp in global queue");

		snprintf(event_name, BUFSIZE, "%s_idp_global", EVSRC);
		ASSERT_EQ(0, pfc_event_register(event_name));
		ASSERT_EQ(EINVAL, pfc_event_add_handler(NULL, event_name,
			  (pfc_evfunc_t)&cb_event_dummy_c,
			  (pfc_ptr_t)NULL, (pfc_evmask_t)0, 0));
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));
	}

	{
		pfc_log_debug("Set NULL as idp in async queue");

		snprintf(event_name, BUFSIZE, "%s_idp_async", EVSRC);
		ASSERT_EQ(0, pfc_event_register_async(event_name));
		ASSERT_EQ(EINVAL, pfc_event_add_handler(NULL, event_name,
			  (pfc_evfunc_t)&cb_event_dummy_c,
			  (pfc_ptr_t)NULL, (pfc_evmask_t)0, 0));
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));
	}

	{
		pfc_log_debug("Set NULL as idp in local queue");

		snprintf(event_name, BUFSIZE, "%s_idp_local", EVSRC);
		ASSERT_EQ(0, pfc_event_register_local(event_name, LOCALQ_THREAD_NUM));
		ASSERT_EQ(EINVAL, pfc_event_add_handler(NULL, event_name,
			  (pfc_evfunc_t)&cb_event_dummy_c,
			  (pfc_ptr_t)NULL, (pfc_evmask_t)0, 0));
		ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));
	}

}

TEST(pfc_event_add_handler, name)
{
	pfc_log_debug("pfc_event_add_handler_name() is called");

	{
		pfc_log_debug("Add handler to the event source in global queue");
		test_name_c(GLOBALQ, PFC_FALSE);
	}

	{
		pfc_log_debug("Add handler to the event source in async queue");
		test_name_c(ASYNCQ, PFC_FALSE);
	}

	{
		pfc_log_debug("Add handler to the event source in local queue");
		test_name_c(LOCALQ, PFC_FALSE);
	}
}

static void *
cb_event2_1(pfc_event_t event, pfc_ptr_t arg)
{
	int err;
	myevent2_t *data;
	uint32_t *mypriority;

	mypriority= (uint32_t *)arg;
	pfc_log_debug("Event handler: priority=%u", *mypriority);

	data = (myevent2_t *)pfc_event_data(event);

	while(1) {
		pfc_log_debug("mutex_trylock");
		err = pfc_mutex_trylock(&data->mutex);
		if (err != 0) {
			pfc_log_fatal("pfc_mutex_lock:%d", err);
			abort();
		}

		if (data->next_priority == *mypriority) {
			data->counter++;

			pfc_log_debug("update the next event handler");
			data->next_priority = *mypriority + 1;  // next priority

			pfc_log_debug("cond_broadcast");
			err = pfc_cond_broadcast(&data->cond);
			if (err != 0) {
				pfc_log_fatal("pfc_cond_broadcast:%d", err);
				abort();
			}

			pfc_log_debug("mutex_unlock");
			err = pfc_mutex_unlock(&data->mutex);
			if (err != 0) {
				pfc_log_fatal("pfc_mutex_unlock:%d", err);
				abort();
			}
			break;
		}

		// Wait for the event data to be updated
		pfc_log_debug("cond_timedwait");
		err = pfc_cond_timedwait(&data->cond, &data->mutex, &timeout_ev_c_);
		if (err != 0) {
			pfc_log_fatal("pfc_cond_timedwait:%d", err);
			abort();
		}

		pfc_log_debug("mutex_unlock");
		err = pfc_mutex_unlock(&data->mutex);
		if (err != 0) {
			pfc_log_fatal("pfc_mutex_unlock:%d", err);
			abort();
		}
	}

	if (PRI_LOWEST == *mypriority) {
		pfc_sem_post(&data->sem);
	}

	return arg;
}

static void *
cb_event2_2(pfc_event_t event, pfc_ptr_t arg)
{
	int err;
	myevent2_t *data;
	uint32_t *mypriority;

	mypriority= (uint32_t *)arg;
	pfc_log_debug("Event handler: priority=%u", *mypriority);

	data = (myevent2_t *)pfc_event_data(event);

	err = pfc_mutex_lock(&data->mutex);
	if (err != 0) {
		pfc_log_fatal("pfc_mutex_lock:%d", err);
		abort();
	}
	data->counter++;

	err = pfc_cond_broadcast(&data->cond);
	if (err != 0) {
		pfc_log_fatal("pfc_cond_broadcast:%d", err);
		abort();
	}

	if (data->counter == PRI_LOWEST - PRI_HIGHEST + 1) {
		pfc_sem_post(&data->sem);
	}

	pfc_log_debug("mutex_unlock");
	err = pfc_mutex_unlock(&data->mutex);
	if (err != 0) {
		pfc_log_fatal("pfc_mutex_unlock:%d", err);
		abort();
	}

	return arg;
}

static void
test_priority(pfc_bool_t count_only, TARGET_QUEUE qtype)
{
	int i;
	char event_name[BUFSIZE];
	uint32_t id[PRI_LOWEST - PRI_HIGHEST + 1];
	pfc_evfunc_t cb;
	pfc_event_t event;
	pfc_evhandler_t ehid;
	myevent2_t *me2p;

	// Setup
	me2p = (myevent2_t *)malloc(sizeof(myevent2_t));
	ASSERT_EQ(0, pfc_sem_init(&me2p->sem, 0));
	ASSERT_EQ(0, pfc_mutex_init(&me2p->mutex, PFC_MUTEX_TYPE_NORMAL));
	ASSERT_EQ(0, pfc_cond_init(&me2p->cond));
	me2p->next_priority = PRI_HIGHEST; // Event handler which will be called at first
	me2p->counter = 0;
	cb = count_only ? (pfc_evfunc_t)cb_event2_2 : (pfc_evfunc_t)cb_event2_1;
	for (i = PRI_HIGHEST; i <= PRI_LOWEST; i++) {
		id[i] = (uint32_t)i;
	}

	// Register an event
	snprintf(event_name, BUFSIZE, "%s_priority", EVSRC);
	register_event_c(event_name, qtype);

	// Add an event handler
	pfc_log_debug("Add event handlers with various priority");
	if (!count_only) {
		for (i = PRI_LOWEST; i >= PRI_HIGHEST; i--) {
			pfc_log_debug("handler priority: %u", id[i]);
			ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
					cb, (pfc_ptr_t)&id[i], (pfc_evmask_t)0, id[i]));
		}
	} else {
		ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
				cb, (pfc_ptr_t)&id[PRI_LOWEST], (pfc_evmask_t)0, id[PRI_LOWEST]));
		for (i = PRI_LOWEST -1; i >= PRI_HIGHEST; i--) {
			ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
					cb, (pfc_ptr_t)&id[i / 5], (pfc_evmask_t)0, id[i / 5]));
		}
	}

	// Create an event
	pfc_log_debug("Create an event object");
	ASSERT_EQ(0, pfc_event_create(&event, (pfc_evtype_t)0, (pfc_ptr_t)me2p, NULL));

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	ASSERT_EQ(0, pfc_event_post(event_name, event));

	// Wait for the lowest handler to exit
	pfc_log_debug("Wait for the lowest(%d) handler to exit", PRI_LOWEST);
	ASSERT_EQ(0, pfc_sem_timedwait(&me2p->sem, &timeout_ev_c_));

	// Check the event handler counter
	pfc_log_debug("Total %u handlers have been called.", me2p->counter);
	ASSERT_EQ(static_cast<uint32_t>(PRI_LOWEST - PRI_HIGHEST + 1),
		  me2p->counter);

	// Unregister the event
	pfc_log_debug("Unregister \"%s\" event.", event_name);
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));

	// Cleanup
	ASSERT_EQ(0, pfc_sem_destroy(&me2p->sem));
	ASSERT_EQ(0, pfc_mutex_destroy(&me2p->mutex));
	ASSERT_EQ(0, pfc_cond_destroy(&me2p->cond));
	free(me2p);
}

TEST(pfc_event_add_handler, priority)
{
	pfc_log_debug("pfc_event_add_handler_priority() is called");

	{
		pfc_log_debug("With global queue.");
		test_priority(PFC_FALSE, GLOBALQ);
	}

	{
		pfc_log_debug("With async queue.");
		test_priority(PFC_TRUE, ASYNCQ);
	}

	{
		pfc_log_debug("With local queue");
		if (LOCALQ_THREAD_NUM == 1) {
			test_priority(PFC_FALSE, LOCALQ);
		} else {
			test_priority(PFC_TRUE, LOCALQ);
		}
	}
}

TEST(pfc_event_add_handler, arg)
{
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_add_handler_arg() is called");
	snprintf(event_name, BUFSIZE, "%s_arg", EVSRC);

	{
		pfc_log_debug("Check that arg is passed to the handler in global queue");
		test_event_c_impl(event_name, GLOBALQ, PFC_FALSE, PFC_FALSE);
	}

	{
		pfc_log_debug("Check that arg is passed to the handler in async queue");
		test_event_c_impl(event_name, ASYNCQ, PFC_FALSE, PFC_FALSE);
	}

	{
		pfc_log_debug("Check that arg is passed to the handler in local queue");
		test_event_c_impl(event_name, LOCALQ, PFC_FALSE, PFC_FALSE);
	}

	{
		pfc_evhandler_t ehid;

		pfc_log_debug("Set NULL as arg");
		ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
				(pfc_evfunc_t)&cb_event_dummy_c, (pfc_ptr_t)NULL, (pfc_evmask_t)0, 0));
	}
}

static void
test_evmask(char *event_name, TARGET_QUEUE qtype, pfc_bool_t evmask_match)
{
	pfc_thread_t tc_cb, tc_dtor;
	pfc_event_t event;
	pfc_evhandler_t ehid;
	pfc_evmask_t evmask;
	pfc_evtype_t evtype;
	myevent_t *mep;

	// Setup
	initialize_myevent_c(&mep, evmask_match, PFC_TRUE);

	// Register an event
	register_event_c(event_name, qtype);

	// EINVAL (*evmask is 0)
	pfc_event_mask_empty(&evmask);
	ASSERT_EQ(EINVAL, pfc_event_add_handler(&ehid, event_name,
						(pfc_evfunc_t)&cb_event_c,
						(pfc_ptr_t)NULL,
						&evmask, 0));

	if (evmask_match) {
		evtype = (pfc_evtype_t)10;
		pfc_event_mask_empty(&evmask);
		pfc_event_mask_add(&evmask, evtype);
	} else {
		evtype = (pfc_evtype_t)10;
		pfc_event_mask_fill(&evmask);
		pfc_event_mask_delete(&evmask, evtype);
	}

	// Create a checker thread
	pfc_log_debug("Create a checker thread");
	ASSERT_EQ(0, pfc_thread_create(&tc_cb, &cb_event_checker_c, (void *)mep, 0));
	pfc_sem_wait(&mep->sem);
	ASSERT_EQ(0, pfc_thread_create(&tc_dtor, &dtor_checker_c, (void *)mep, 0));
	pfc_sem_wait(&mep->sem);

	// Add an event handler with mask
	pfc_log_debug("Add an event handler (mask=0x%x)", evmask);
	ASSERT_EQ(0,  pfc_event_add_handler(&ehid, event_name,
			(pfc_evfunc_t)&cb_event_c, (pfc_ptr_t)mep, &evmask, 0));

	// Create an event
	pfc_event_mask_empty(&evmask);
	pfc_log_debug("Create an event object (type=0x%x)", evtype);
	ASSERT_EQ(0, pfc_event_create(&event, evtype, (pfc_ptr_t)mep,
			(pfc_evdtor_t)dtor_event_c));

	// Post the event
	pfc_log_debug("Post the event object to \"%s\" event source", event_name);
	ASSERT_EQ(0, pfc_event_post(event_name, event));

	// Wait for the checker threads to exit
	pfc_thread_join(tc_cb, NULL);
	pfc_thread_join(tc_dtor, NULL);

	// Unregister the event
	pfc_log_debug("Unregister \"%s\" event.", event_name);
	ASSERT_EQ(0, pfc_event_unregister(event_name, &timeout_ev_c_));

	// Cleanup
	cleanup_myevent_c(mep);
}

TEST(pfc_event_add_handler, evmask)
{
	char event_name[BUFSIZE];

	pfc_log_debug("pfc_event_add_handler_evmask() is called");
	snprintf(event_name, BUFSIZE, "%s_evmask", EVSRC);

	{
		pfc_log_debug("Examine to unmatch with posted event type in global queue");
		test_evmask(event_name, GLOBALQ, PFC_FALSE);
	}

	{
		pfc_log_debug("Examine to match with posted event type in global queue");
		test_evmask(event_name, GLOBALQ, PFC_TRUE);
	}

	{
		pfc_log_debug("Examine to unmatch with posted event type in async queue");
		test_evmask(event_name, ASYNCQ, PFC_FALSE);
	}

	{
		pfc_log_debug("Examine to match with posted event type in async queue");
		test_evmask(event_name, ASYNCQ, PFC_TRUE);
	}

	{
		pfc_log_debug("Examine to unmatch with posted event type in local queue");
		test_evmask(event_name, LOCALQ, PFC_FALSE);
	}

	{
		pfc_log_debug("Examine to match with posted event type in local queue");
		test_evmask(event_name, LOCALQ, PFC_TRUE);
	}
}
