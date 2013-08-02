/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>

#include <pfc/clock.h>
#include <pfc/log.h>
#include <pfcxx/thread.hh>
#include <pfcxx/event.hh>
#include <pfcxx/synch.hh>
#include <boost/bind.hpp>
#include "params.h"

#define BUFSIZE 256
#define EXPECTED_VAL 10

extern const pfc_timespec_t timeout_ev_cc_;

namespace pfc {
namespace core {
	class MyData {
	public:
		inline MyData(int val, pfc_bool_t isInvoked) : val_(val), isInvoked_(isInvoked) {
			mutex_ = new Mutex();
			condv_ = new Condition();
			sem_ = new Semaphore(0);
		}

		inline ~MyData() {
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

		// The caller must lock mutex before the call of this function.
		inline void setInvoked() {
                	isInvoked_ = PFC_TRUE;
		}

		inline void resetInvoked() {
			mutex_->lock();
                	isInvoked_ = PFC_FALSE;
			mutex_->unlock();
		}

		inline int val() {
			return val_;
		}

	private:
		int val_;
		pfc_bool_t isInvoked_;
		Mutex *mutex_;
		Condition *condv_;
		Semaphore *sem_;
	};
} // namespace core
} // namespace pfc

enum TARGET_ADDHANDLER {
	WITH_SNAME,
	WITH_CNAME,
	WITH_MASK_SNAME,
	WITH_MASK_CNAME
};

enum TARGET_QUEUE {
	GLOBALQ,
	ASYNCQ,
	LOCALQ
};

using namespace pfc::core;

extern void cb_event_cxx(Event *);
extern void *cb_event_checker_cxx(MyData *);
extern void register_event_cxx(char *, TARGET_QUEUE);
extern void test_event_cxx_impl(char *, TARGET_QUEUE, pfc_bool_t, pfc_bool_t, TARGET_ADDHANDLER);
extern void test_name_cxx(TARGET_QUEUE, pfc_bool_t);
