/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

extern "C" {
#include <unistd.h>
#include <semaphore.h>
#include <pfc/base.h>
#include <pfc/log.h>
#include <pfc/clock.h>
}

#include <cxx/pfcxx/synch.hh>
#include <cxx/pfcxx/thread_pool.hh>
#include <cxx/pfcxx/thread.hh>
#include <cxx/pfcxx/task_queue.hh>
#include <cxx/pfcxx/timer.hh>
#include <boost/bind.hpp>
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
extern const pfc_timespec_t timeout_t_cc_;
const pfc_timespec_t resolution_cc_ = {0, RES_MIN};

namespace pfc {
namespace core {
class MyTimeout {
public:
	inline MyTimeout(int val, pfc_bool_t isInvoked,
			const pfc_timespec_t &resolution) :
			val_(val), isInvoked_(isInvoked), resolution_(resolution) {
		mutex_ = new Mutex();
		condv_ = new Condition();
		sem_ = new Semaphore(0);
	}

	inline ~MyTimeout() {
		delete mutex_;
		delete condv_;
		delete sem_;
	}

	inline Mutex *mutex() {
		return mutex_;
	}

	inline Condition *cond() {
		return condv_;
	}

	inline Semaphore *sema() {
		return sem_;
	}

	inline pfc_bool_t isInvoked() {
		return isInvoked_;
	}

	inline int val() {
		return val_;
	}

	inline pfc_timespec_t getResolution() {
		return resolution_;
	}

	inline void setAbsTimeout(pfc_timespec_t absTimeout) {
		absTimeout_ = absTimeout;
	}

	inline pfc_timespec_t getAbsTimeout() {
		return absTimeout_;
	}

private:
	int val_;
	pfc_bool_t isInvoked_;
	pfc_timespec_t resolution_;
	pfc_timespec_t absTimeout_;
	Mutex *mutex_;
	Condition *condv_;
	Semaphore *sem_;
};
} // namespace core
} // namespace pfc

using namespace pfc::core;

// Poller callback function.
extern void cb_timer_cxx(MyTimeout *);
extern void cb_timer_cxx_dummy(void);

// Verify that a poller callback function is called.
extern void *cb_timer_cxx_checker(MyTimeout *);

// Initialize libpfc resources.
extern void initialize_timer_cxx_env(TaskQueue *&, Timer *&);

// Cleanup libpfc resources.
extern void cleanup_timer_cxx_env(TaskQueue *&, Timer *&);

// Examine the standard timer processing
extern void test_timer_cxx_impl(pfc_bool_t, pfc_timespec_t, pfc_timespec_t);
