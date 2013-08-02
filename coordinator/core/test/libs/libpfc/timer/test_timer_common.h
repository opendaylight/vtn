/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <unistd.h>
#include <semaphore.h>

#include <pfc/base.h>
#include <pfc/clock.h>
#include <pfc/timer.h>
#include <pfc/thread.h>
#include <pfc/tpool.h>
#include <pfc/synch.h>
#include <pfc/log.h>

#include "params.h"

#define BUFSIZE 256
#define EXPECTED_VAL 100
#define RES_MIN PFC_CONST_U(100000000)

const pfc_timespec_t ts_0sec = {PFC_CONST_L(0), PFC_CONST_L(0)};
const pfc_timespec_t ts_1sec = {PFC_CONST_L(1), PFC_CONST_L(0)};
const pfc_timespec_t ts_1_2sec = {PFC_CONST_L(1), PFC_CONST_L(200000000)};
const pfc_timespec_t ts_2sec = {PFC_CONST_L(2), PFC_CONST_L(0)};
const pfc_timespec_t ts_2_5sec = {PFC_CONST_L(2), PFC_CONST_L(500000000)};
const pfc_timespec_t ts_3sec = {PFC_CONST_L(2), PFC_CONST_L(0)};
const pfc_timespec_t ts_5sec = {PFC_CONST_L(5), PFC_CONST_L(0)};
const pfc_timespec_t ts_10sec = {PFC_CONST_L(10), PFC_CONST_L(0)};
const pfc_timespec_t ts_15sec = {PFC_CONST_L(15), PFC_CONST_L(0)};
const pfc_timespec_t ts_20sec = {PFC_CONST_L(20), PFC_CONST_L(0)};
const pfc_timespec_t ts_1nsec = {PFC_CONST_L(0), PFC_CONST_L(1)};
const pfc_timespec_t ts_1usec = {PFC_CONST_L(0), PFC_CONST_L(1000)};
const pfc_timespec_t ts_1msec = {PFC_CONST_L(0), PFC_CONST_L(1000000)};
const pfc_timespec_t ts_20msec = {PFC_CONST_L(0), PFC_CONST_L(200000)};
const pfc_timespec_t ts_50msec = {PFC_CONST_L(0), PFC_CONST_L(500000)};
const pfc_timespec_t ts_100msec = {PFC_CONST_L(0), PFC_CONST_L(100000000)};
const pfc_timespec_t ts_200msec = {PFC_CONST_L(0), PFC_CONST_L(200000000)};
const pfc_timespec_t ts_500msec = {PFC_CONST_L(0), PFC_CONST_L(500000000)};
const pfc_timespec_t ts_invalid = {PFC_CONST_L(0), PFC_CONST_L(-100000000)};

// Timeout value to check if callback or destructor functions aren't called
extern const pfc_timespec_t timeout_t_c_;
const pfc_timespec_t resolution_c_ = {0, RES_MIN};

typedef struct mytimeout {
	int val;
	pfc_bool_t invoke_cb;
	pfc_bool_t invoke_dtor;
	pfc_sem_t sem;
	pfc_mutex_t mutex;
	pfc_cond_t cb_called;
	pfc_cond_t dtor_called;
	pfc_timespec_t resolution;
	pfc_timespec_t abs_tout;
} mytimeout_t;

// Poller callback function.
void *cb_timer_c(void *);
void *cb_timer_c_dummy(void *);

// Poller destructor function.
extern void *dtor_timer_c(void *);
extern void *dtor_timer_c_dummy(void *);

// Verify that a poller callback function is called.
extern void *cb_timer_c_checker(void *);

// Verify that a poller destructor function is called.
extern void *dtor_timer_c_checker(void *arg);

// Initialize resources.
extern void initialize_mytimeout(mytimeout_t**, pfc_timespec_t, pfc_bool_t, pfc_bool_t);

// Cleanup resources.
extern void cleanup_mytimeout(mytimeout_t *);

// Initialize libpfc resources.
extern void initialize_timer_c_env(pfc_taskq_t *, pfc_timer_t *);

// Cleanup libpfc resources.
extern void cleanup_timer_c_env(pfc_taskq_t, pfc_timer_t);

// Examine the standard timer processing
extern void test_timer_c_impl(pfc_bool_t, pfc_bool_t, pfc_timespec_t, pfc_timespec_t);
