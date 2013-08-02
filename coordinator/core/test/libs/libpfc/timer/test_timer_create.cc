/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <gtest/gtest.h>
#include "test_timer_common.h"

TEST(pfc_timer_create, tidp)
{
	pfc_log_debug("pfc_timer_create_tidp() is called");

	{
		pfc_log_debug("Set a valid value as tidp");
		test_timer_c_impl(PFC_TRUE, PFC_FALSE, ts_100msec, ts_50msec);
		test_timer_c_impl(PFC_TRUE, PFC_TRUE, ts_200msec, ts_100msec);
	}
}

TEST(pfc_timer_create, poolname)
{
	pfc_log_debug("pfc_timer_create_poolname() is called");

	// Test1:
	{
		pfc_log_debug("Set a valid value as poolname");
		test_timer_c_impl(PFC_TRUE, PFC_FALSE, ts_50msec, ts_20msec);
		test_timer_c_impl(PFC_TRUE, PFC_TRUE, ts_100msec, ts_50msec);
	}

	// Test2:
	{
		pfc_timer_t tid;
		pfc_taskq_t taskq;
		char pool_name[BUFSIZE];

		pfc_log_debug("Set an invalid value as poolname");

		snprintf(pool_name, BUFSIZE, "poolname_invalid");
		ASSERT_EQ(0, pfc_tpool_create(pool_name));
		ASSERT_EQ(0, pfc_taskq_create(&taskq, pool_name, TASKQ_THREADS));
		ASSERT_EQ(0, pfc_tpool_destroy(pool_name, &timeout_t_c_));
		ASSERT_EQ(ENOENT, pfc_timer_create(&tid, pool_name, taskq,
			  NULL));
		ASSERT_EQ(0, pfc_taskq_destroy(taskq));
	}
}

TEST(pfc_timer_create, tqid)
{
	pfc_log_debug("pfc_timer_create_tqid() is called");

	{
		pfc_log_debug("Set a valid value as tqid");
		test_timer_c_impl(PFC_TRUE, PFC_FALSE, ts_50msec, ts_20msec);
		test_timer_c_impl(PFC_TRUE, PFC_TRUE, ts_100msec, ts_50msec);
	}

	{
		//pfc_log_debug("Set an invalid value as tqid");
		//undefined
	}
}

TEST(pfc_timer_create, resolution)
{
	pfc_log_debug("pfc_timer_create_resolution() is called");

	pfc_log_debug("Variation check of resolution");
	test_timer_c_impl(PFC_TRUE, PFC_FALSE, ts_100msec, ts_50msec);
	test_timer_c_impl(PFC_TRUE, PFC_TRUE, ts_20msec, ts_20msec);
	test_timer_c_impl(PFC_TRUE, PFC_TRUE, ts_200msec, ts_200msec);

	{
		pfc_timer_t	tid;
		pfc_taskq_t	taskq;
		pfc_timespec_t	res;

		res.tv_sec = PFC_TIMER_MAXRES - 1;
		res.tv_nsec = PFC_CLOCK_NANOSEC - 1;
		ASSERT_EQ(0, pfc_taskq_create(&taskq, NULL, TASKQ_THREADS));
		ASSERT_EQ(0, pfc_timer_create(&tid, NULL, taskq, &res));
		ASSERT_EQ(0, pfc_timer_destroy(tid));
		ASSERT_EQ(0, pfc_taskq_destroy(taskq));
	}

	{
		pfc_timer_t tid;
		pfc_taskq_t taskq;

		pfc_log_debug("Set NULL as resolution");

#ifdef TPOOL_NAME
		ASSERT_EQ(0, pfc_tpool_create(TPOOL_NAME));
		ASSERT_EQ(0, pfc_taskq_create(&taskq, TPOOL_NAME, TASKQ_THREADS));
		ASSERT_EQ(0, pfc_timer_create(&tid, TPOOL_NAME, taskq, NULL));
		ASSERT_EQ(0, pfc_timer_destroy(tid));
		ASSERT_EQ(0, pfc_taskq_destroy(taskq));
		ASSERT_EQ(0, pfc_tpool_destroy(TPOOL_NAME, &timeout_t_c_));
#else // TPOOL_NAME
		ASSERT_EQ(0, pfc_taskq_create(&taskq, NULL, TASKQ_THREADS));
		ASSERT_EQ(0, pfc_timer_create(&tid, NULL, taskq, NULL));
		ASSERT_EQ(0, pfc_timer_destroy(tid));
		ASSERT_EQ(0, pfc_taskq_destroy(taskq));
#endif // TPOOL_NAME
	}

	{
		pfc_timer_t tid;
		pfc_taskq_t taskq;
		static const pfc_timespec_t	invalid_res[] = {
			{ -1, 0 },
			{ 0, -1 },
			{ -1, -1 },
			{ (time_t)PFC_ULONG_MAX, 0 },
			{ 0, (time_t)PFC_ULONG_MAX },
			{ (time_t)PFC_ULONG_MAX, (time_t)PFC_ULONG_MAX },
			{ PFC_LONG_MAX, 0 },
			{ 0, PFC_LONG_MAX },
			{ PFC_LONG_MAX, PFC_LONG_MAX },
			{ PFC_TIMER_MAXRES, 0 },
			{ PFC_TIMER_MAXRES + 1, 0 },
			{ 0, PFC_CLOCK_NANOSEC },
			{ 0, PFC_CLOCK_NANOSEC + 1 },
		};
		const pfc_timespec_t	*resp;

		pfc_log_debug("Set invalid resolution");

#ifdef TPOOL_NAME
		ASSERT_EQ(0, pfc_tpool_create(TPOOL_NAME));
		ASSERT_EQ(0, pfc_taskq_create(&taskq, TPOOL_NAME, TASKQ_THREADS));
		for (resp = invalid_res; resp < PFC_ARRAY_LIMIT(invalid_res);
                     resp++) {
			ASSERT_EQ(EINVAL, pfc_timer_create(&tid, TPOOL_NAME,
							   taskq, resp));
		}
		ASSERT_EQ(0, pfc_taskq_destroy(taskq));
		ASSERT_EQ(0, pfc_tpool_destroy(TPOOL_NAME, &timeout_t_c_));
#else // TPOOL_NAME
		ASSERT_EQ(0, pfc_taskq_create(&taskq, NULL, TASKQ_THREADS));
		for (resp = invalid_res; resp < PFC_ARRAY_LIMIT(invalid_res);
		     resp++) {
			ASSERT_EQ(EINVAL, pfc_timer_create(&tid, NULL,
							   taskq, resp));
		}
		ASSERT_EQ(0, pfc_taskq_destroy(taskq));
#endif // TPOOL_NAME
	}
}

#define	TPOOL_DEFAULT_DEF_FREE_MAX	50U
#define	TIMER_COUNT			(TPOOL_DEFAULT_DEF_FREE_MAX + 1)

TEST(pfc_timer_create, eagain)
{
	char pool_name_eagain[BUFSIZE];
	pfc_timer_t	tid[TIMER_COUNT];
	pfc_taskq_t	taskq;
	uint32_t	i;
	pfc_cfblk_t	map;
	uint32_t	max_thr;

	snprintf(pool_name_eagain, BUFSIZE, "poolname_eagain");
	map = pfc_sysconf_get_map("thread_pool", "poolname_eagain");
	max_thr = pfc_conf_get_uint32(map, "max_threads",
				      TPOOL_DEFAULT_DEF_FREE_MAX);
	ASSERT_EQ(0, pfc_tpool_create(pool_name_eagain));
	ASSERT_EQ(0, pfc_taskq_create(&taskq, pool_name_eagain,
				      TASKQ_THREADS));

	for(i = 0; i < max_thr; i++){
		ASSERT_EQ(0, pfc_timer_create(&tid[i], pool_name_eagain,
					      taskq, NULL));
	}

	ASSERT_EQ(EAGAIN, pfc_timer_create(&tid[i], pool_name_eagain,
					   taskq, NULL));

	for(i = 0; i < max_thr; i++){
		ASSERT_EQ(0, pfc_timer_destroy(tid[i]));
	}
	ASSERT_EQ(0, pfc_taskq_destroy(taskq));
	ASSERT_EQ(0, pfc_tpool_destroy(pool_name_eagain, &timeout_t_c_));
}
