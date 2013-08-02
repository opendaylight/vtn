/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <unistd.h>
#include <pfc/event.h>
#include <pfc/thread.h>
#include <pfc/tpool.h>
#include <pfc/synch.h>
#include <pfc/log.h>
#include "params.h"

#define EXPECTED_VAL 100
#define BUFSIZE 256

// Timeout value to check if callback or destructor functions aren't called
extern const pfc_timespec_t timeout_ev_c_;

enum TARGET_QUEUE {
	GLOBALQ = 0,
			ASYNCQ,
			LOCALQ
};

typedef struct myevent {
	int val;
	int invoke_cb;
	int invoke_dtor;
	pfc_sem_t sem;
	pfc_mutex_t mutex;
	pfc_cond_t cb_called;
	pfc_cond_t dtor_called;
} myevent_t;

// Poller callback function.
void *cb_event_c(pfc_event_t, pfc_ptr_t);
void *cb_event_dummy_c(void *);

// Poller destructor function.
extern void *dtor_event_c(void *);
extern void *dtor_event_dummy_c(void *);

// Verify that a poller callback function is called.
extern void *cb_event_checker_c(void *);

// Verify that a poller destructor function is called.
extern void *dtor_checker_c(void *arg);

// Initialize resources.
extern void initialize_myevent_c(myevent_t**, pfc_bool_t, pfc_bool_t);

// Cleanup resources.
extern void cleanup_myevent_c(myevent_t *);

// Initialize libpfc resources.
extern void initialize_event_c_env();

// Cleanup libpfc resources.
extern void cleanup_event_c_env();

// Examine the standard event processing
extern void test_event_c_impl(char *, TARGET_QUEUE, pfc_bool_t, pfc_bool_t);

// Examine various event source names in event processing
extern void test_name_c(TARGET_QUEUE queue, pfc_bool_t);

// Register an event source in the target queue
extern void register_event_c(char *name, TARGET_QUEUE queue);
