/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_timer_common.hh"

/*
 * Undefine TPOOL_NAME at test in test_timer_common.hh
 */
TEST(create1, tqid) {

	pfc_log_debug("create1_tqid() is called");

	{
		pfc_log_debug("Set a valid value as tqid");
		test_timer_cxx_impl(PFC_TRUE, ts_500msec, ts_100msec);
	}

	{
		//pfc_log_debug("Set an invalid value1 as tqid");
		// Undefined
	}

	{
		//pfc_log_debug("Set an invalid value2 as tqid");
		// Undefined
	}
}

/*
 * Undefine TPOOL_NAME at test in test_timer_common.hh
 */
TEST(create1, resolution) {

	pfc_log_debug("create1_resolution() is called");

	// variation check
	{
		pfc_log_debug("Set resolution as 50msec");
		test_timer_cxx_impl(PFC_TRUE, ts_100msec, ts_50msec);
		pfc_log_debug("Set resolution as 100msec");
		test_timer_cxx_impl(PFC_TRUE, ts_500msec, ts_100msec);
	}

	{
		TaskQueue *taskq = NULL;
		Timer *timer = NULL;

		pfc_log_debug("Set resolution as NULL");

		taskq = TaskQueue::create(TASKQ_THREADS);
		ASSERT_NE((TaskQueue *)NULL, taskq);
		timer = Timer::create(taskq->getId());
		ASSERT_NE((Timer *)NULL, timer);

		delete timer;
		delete taskq;
	}

	// Test3:
	{
		TaskQueue *taskq = NULL;
		Timer *timer = NULL;

		pfc_log_debug("Set resolution as negative value");

		taskq = TaskQueue::create(TASKQ_THREADS);
		ASSERT_NE((TaskQueue *)NULL, taskq);
		timer = Timer::create(taskq->getId(), &ts_invalid);
		ASSERT_EQ((Timer *)NULL, timer);

		delete timer;
		delete taskq;
	}
}

/**
 * Define TPOOL_NAME at test in test_timer_common.hh
 */
TEST(create2, tqid) {

	pfc_log_debug("create2_tqid() is called");

	{
		pfc_log_debug("Set a valid value as tqid");
		test_timer_cxx_impl(PFC_TRUE, ts_100msec, ts_50msec);
	}

	{
		//pfc_log_debug("Set an invalid value1 as tqid");
		// Undefined
	}

	{
		//pfc_log_debug("Set an invalid value2 as tqid");
		// Undefined
	}
}

TEST(create2, poolname) {

	pfc_log_debug("create2_poolname() is called");

	{
		std::string tpool_name("create2_resolution_NULL");
		TaskQueue *taskq = NULL;
		Timer *timer = NULL;

		pfc_log_debug("Set poolname as a valid value");

		ASSERT_EQ(0, ThreadPool::create(tpool_name));
		taskq = TaskQueue::create(TASKQ_THREADS);
		ASSERT_NE((TaskQueue *)NULL, taskq);
		timer = Timer::create(taskq->getId(), tpool_name, &resolution_cc_);
		ASSERT_NE((Timer *)NULL, timer);

		delete timer;
		delete taskq;
		ASSERT_EQ(0, ThreadPool::destroy(tpool_name));
	}

	// Test2:
	{
		std::string tpool_name("create2_resolution_NULL");
		TaskQueue *taskq = NULL;
		Timer *timer = NULL;

		pfc_log_debug("Set poolname as an invalid value");

		ASSERT_EQ(0, ThreadPool::create(tpool_name));
		taskq = TaskQueue::create(TASKQ_THREADS);
		ASSERT_NE((TaskQueue *)NULL, taskq);
		ASSERT_EQ(0, ThreadPool::destroy(tpool_name));
		timer = Timer::create(taskq->getId(), tpool_name, &resolution_cc_);
		ASSERT_EQ((Timer *)NULL, timer);

		delete timer;
		delete taskq;
	}
}

/**
 * Define TPOOL_NAME at test in test_timer_common.hh
 */
TEST(create2, resolution) {

	pfc_log_debug("create2_resolution() is called");

	// variation check
	{
		pfc_log_debug("Set resolution as 100msec");
		test_timer_cxx_impl(PFC_TRUE, ts_500msec, ts_100msec);
		pfc_log_debug("Set resolution as 200msec");
		test_timer_cxx_impl(PFC_TRUE, ts_200msec, ts_200msec);
	}

	{
		std::string tpool_name("create2_resolution_NULL");
		TaskQueue *taskq;
		Timer *timer;

		pfc_log_debug("Set resolution as NULL");

		ASSERT_EQ(0, ThreadPool::create(tpool_name));
		taskq = TaskQueue::create(TASKQ_THREADS);
		ASSERT_NE((TaskQueue *)NULL, taskq);
		timer = Timer::create(taskq->getId(), tpool_name, NULL);
		ASSERT_NE((Timer *)NULL, timer);

		delete timer;
		delete taskq;
		ASSERT_EQ(0, ThreadPool::destroy(tpool_name));
	}

	{
		std::string tpool_name("create2_resolution_negative");
		TaskQueue *taskq;
		Timer *timer;

		pfc_log_debug("Set resolution as negative value");

		ASSERT_EQ(0, ThreadPool::create(tpool_name));
		taskq = TaskQueue::create(tpool_name, TASKQ_THREADS);
		ASSERT_NE((TaskQueue *)NULL, taskq);
		timer = Timer::create(taskq->getId(), tpool_name, &ts_invalid);
		ASSERT_EQ((Timer *)NULL, timer);

		delete timer;
		delete taskq;
		ASSERT_EQ(0, ThreadPool::destroy(tpool_name));
	}
}
