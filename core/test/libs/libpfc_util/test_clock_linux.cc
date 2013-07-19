/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for clock_linux.c
 */

#include <boost/random.hpp>
#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/time.h>
#include <pfc/clock.h>
#include <pfc/log.h>

#define LOOP	100000

/* set pfc_timespec_t  zero */
static inline void
timespec_clear(pfc_timespec_t *pt)
{
    pt->tv_sec  = 0;
    pt->tv_nsec = 0;
}

/*
 *
 * test cases
 *
 */ 

/* test for pfc_clock_gettime() */
TEST(clock, test_pfc_clock_gettime)
{
    pfc_timespec_t pt1, pt2;
    struct timespec before, after;

    /* test normal */
    timespec_clear(&pt1);
    timespec_clear(&pt2);
    ASSERT_EQ(0, pfc_clock_gettime(&pt1));
    ASSERT_TRUE((pt1.tv_sec != 0) || (pt1.tv_nsec != 0));
    ASSERT_EQ(0, pfc_clock_gettime(&pt2));
    ASSERT_TRUE((pt2.tv_sec != 0) || (pt2.tv_nsec != 0));

    /* test correctness clock value */
    timespec_clear(&pt1);
    EXPECT_EQ(0, clock_gettime(CLOCK_MONOTONIC, &before));
    EXPECT_EQ(0, pfc_clock_gettime(&pt1));
    EXPECT_EQ(0, clock_gettime(CLOCK_MONOTONIC, &after));
    ASSERT_GE(0, pfc_clock_compare((pfc_timespec_t *)&before, &pt1));
    ASSERT_GE(0, pfc_clock_compare(&pt1, (pfc_timespec_t *)&after));

    /* test pt1 < pt2 */
    struct timespec ts = {0, 1};
    timespec_clear(&pt1);
    timespec_clear(&pt2);
    EXPECT_EQ(0, pfc_clock_gettime(&pt1));
    nanosleep(&ts, NULL);
    EXPECT_EQ(0, pfc_clock_gettime(&pt2));
    ASSERT_LT(0, pfc_clock_compare(&pt2, &pt1));

#ifndef	PFC_LP64
    // EFAULT test.
    ASSERT_EQ(EFAULT, pfc_clock_gettime(NULL));
#endif	/* PFC_LP64 */
}

/* test for pfc_clock_get_realtime() */
TEST(clock, test_pfc_clock_get_realtime)
{
    pfc_timespec_t pt1, pt2;
    struct timespec before, after;

    /* test normal */
    timespec_clear(&pt1);
    timespec_clear(&pt2);
    ASSERT_EQ(0, pfc_clock_get_realtime(&pt1));
    ASSERT_TRUE((pt1.tv_sec != 0) || (pt1.tv_nsec != 0));
    ASSERT_EQ(0, pfc_clock_get_realtime(&pt2));
    ASSERT_TRUE((pt2.tv_sec != 0) || (pt2.tv_nsec != 0));

    /* test correctness clock value */
    timespec_clear(&pt1);
    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &before));
    EXPECT_EQ(0, pfc_clock_get_realtime(&pt1));
    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &after));
    ASSERT_GE(0, pfc_clock_compare((pfc_timespec_t *)&before, &pt1));
    ASSERT_GE(0, pfc_clock_compare(&pt1, (pfc_timespec_t *)&after));

    /* pt1 is not pt2. It is different value. */
    struct timespec ts = {0, 1};
    timespec_clear(&pt1);
    timespec_clear(&pt2);
    EXPECT_EQ(0, pfc_clock_get_realtime(&pt1));
    nanosleep(&ts, NULL);
    EXPECT_EQ(0, pfc_clock_get_realtime(&pt2));
    ASSERT_LT(0, pfc_clock_compare(&pt2, &pt1));

#ifndef	PFC_LP64
    // EFAULT test.
    ASSERT_EQ(EFAULT, pfc_clock_get_realtime(NULL));
#endif	/* PFC_LP64 */
}

/* test for pfc_clock_abstime() */
TEST(clock, test_pfc_clock_abstime)
{
    pfc_timespec_t pt1, pt2, pt3, pt_inval;
    struct timespec before, after;
    time_t diff = 10;

    /* set relative time */
    pt2.tv_sec = 1000;
    pt2.tv_nsec = 0;

    /* test normal */
    timespec_clear(&pt1);
    ASSERT_EQ(0, pfc_clock_abstime(&pt1, &pt2));
    ASSERT_TRUE((pt1.tv_sec != 0) || (pt1.tv_nsec != 0));

    /* get monotonic time */ 
    EXPECT_EQ(0, pfc_clock_gettime(&pt3));

    /* abstime > gettime test */
    ASSERT_LT(0, pfc_clock_compare(&pt1, &pt3));

    /* test diff between before relative time and absolute time */
    timespec_clear(&pt1);
    timespec_clear(&pt3);
    EXPECT_EQ(0, pfc_clock_abstime(&pt1, &pt2));
    pt2.tv_sec += diff;
    EXPECT_EQ(0, pfc_clock_abstime(&pt3, &pt2));
    pfc_timespec_sub(&pt3, &pt1);
    ASSERT_GE(diff, pt3.tv_sec);
    ASSERT_LE(pt3.tv_sec, diff + 1); /* 1 is small error */

    /* test correctness clock value */
    timespec_clear(&pt1);
    EXPECT_EQ(0, clock_gettime(CLOCK_MONOTONIC, &before));
    pfc_timespec_add((pfc_timespec_t *)&before, &pt2);
    EXPECT_EQ(0, pfc_clock_abstime(&pt1, &pt2));
    EXPECT_EQ(0, clock_gettime(CLOCK_MONOTONIC, &after));
    pfc_timespec_add((pfc_timespec_t *)&after, &pt2);
    ASSERT_GE(0, pfc_clock_compare((pfc_timespec_t *)&before, &pt1));
    ASSERT_GE(0, pfc_clock_compare(&pt1, (pfc_timespec_t *)&after));

    /* EINVAL test */
    timespec_clear(&pt1);
    pt_inval.tv_sec = 0;
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1;
    EXPECT_EQ(0, pfc_clock_abstime(&pt1, &pt_inval));
    timespec_clear(&pt1);
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC;
    ASSERT_EQ(EINVAL, pfc_clock_abstime(&pt1, &pt_inval));
    pt_inval.tv_sec = -1;
    pt_inval.tv_nsec = 0;
    ASSERT_EQ(EINVAL, pfc_clock_abstime(&pt1, &pt_inval));

    /* ERANGE test */
    timespec_clear(&pt1);
    pt_inval.tv_sec = PFC_LONG_MAX;
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1;
    ASSERT_EQ(ERANGE, pfc_clock_abstime(&pt1, &pt_inval));
    timespec_clear(&pt1);
    pt_inval.tv_sec = PFC_LONG_MAX;
    pt_inval.tv_nsec = 0;
    ASSERT_EQ(ERANGE, pfc_clock_abstime(&pt1, &pt_inval));

    ASSERT_EQ(0, pfc_clock_gettime(&pt2));
    pt_inval.tv_sec = (pfc_ulong_t)PFC_LONG_MAX + 1 - pt2.tv_sec;
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1 - pt2.tv_nsec;
    ASSERT_EQ(ERANGE, pfc_clock_abstime(&pt1, &pt_inval));

    pt_inval.tv_sec -= 2;
    ASSERT_EQ(0, pfc_clock_abstime(&pt1, &pt_inval));

#ifndef	PFC_LP64
    // EFAULT test.
    timespec_clear(&pt1);
    ASSERT_EQ(EFAULT, pfc_clock_abstime(NULL, &pt1));
#endif	/* PFC_LP64 */
}

/* test for pfc_clock_real_abstime */
TEST(clock, test_pfc_clock_real_abstime)
{
    pfc_timespec_t pt1, pt2, pt3, pt_inval;
    struct timespec before, after;
    time_t diff = 10;

    /* set relative time */
    pt2.tv_sec = 1000;
    pt2.tv_nsec = 0;

    /* test normal */
    timespec_clear(&pt1);
    ASSERT_EQ(0, pfc_clock_real_abstime(&pt1, &pt2));
    ASSERT_TRUE((pt1.tv_sec != 0) || (pt1.tv_nsec != 0));

    /* get realtime */
    EXPECT_EQ(0, pfc_clock_get_realtime(&pt3));

    /* abstime > gettime test */
    ASSERT_LT(0, pfc_clock_compare(&pt1, &pt3));

    /* test diff between before relative time and absolute time */
    timespec_clear(&pt1);
    timespec_clear(&pt3);
    EXPECT_EQ(0, pfc_clock_real_abstime(&pt1, &pt2));
    pt2.tv_sec += diff;
    EXPECT_EQ(0, pfc_clock_real_abstime(&pt3, &pt2));
    pfc_timespec_sub(&pt3, &pt1);
    ASSERT_GE(diff, pt3.tv_sec);
    ASSERT_LE(pt3.tv_sec, diff + 1); /* 1 is small error */

    /* test correctness clock value */
    timespec_clear(&pt1);
    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &before));
    pfc_timespec_add((pfc_timespec_t *)&before, &pt2);
    EXPECT_EQ(0, pfc_clock_real_abstime(&pt1, &pt2));
    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &after));
    pfc_timespec_add((pfc_timespec_t *)&after, &pt2);
    ASSERT_GE(0, pfc_clock_compare((pfc_timespec_t *)&before, &pt1));
    ASSERT_GE(0, pfc_clock_compare(&pt1, (pfc_timespec_t *)&after));

    /* EINVAL test */
    timespec_clear(&pt1);
    pt_inval.tv_sec = 0;
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1;
    EXPECT_EQ(0, pfc_clock_real_abstime(&pt1, &pt_inval));
    timespec_clear(&pt1);
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC;
    ASSERT_EQ(EINVAL, pfc_clock_real_abstime(&pt1, &pt_inval));
    pt_inval.tv_sec = -1;
    pt_inval.tv_nsec = 0;
    ASSERT_EQ(EINVAL, pfc_clock_real_abstime(&pt1, &pt_inval));

    /* ERANGE test */
    timespec_clear(&pt1);
    pt_inval.tv_sec = PFC_LONG_MAX;
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1;
    ASSERT_EQ(ERANGE, pfc_clock_real_abstime(&pt1, &pt_inval));
    timespec_clear(&pt1);
    pt_inval.tv_sec = PFC_LONG_MAX;
    pt_inval.tv_nsec = 0;
    ASSERT_EQ(ERANGE, pfc_clock_real_abstime(&pt1, &pt_inval));

    ASSERT_EQ(0, pfc_clock_get_realtime(&pt2));
    pt_inval.tv_sec = (pfc_ulong_t)PFC_LONG_MAX + 1 - pt2.tv_sec;
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1 - pt2.tv_nsec;
    ASSERT_EQ(ERANGE, pfc_clock_real_abstime(&pt1, &pt_inval));

    pt_inval.tv_sec -= 2;
    ASSERT_EQ(0, pfc_clock_real_abstime(&pt1, &pt_inval));

#ifndef	PFC_LP64
    // EFAULT test.
    timespec_clear(&pt1);
    ASSERT_EQ(EFAULT, pfc_clock_real_abstime(NULL, &pt1));
#endif	/* PFC_LP64 */
}

/* test for pfc_clock_mono2real */
TEST(clock, test_pfc_clock_mono2real)
{
    pfc_timespec_t pt1, pt2, pt3, pt_inval;
    struct timespec before, after;
    time_t diff = 10;

    /* prepare monotonic time */
    EXPECT_EQ(0, pfc_clock_gettime(&pt2));

    /* get real_abstime test */
    timespec_clear(&pt1);
    ASSERT_EQ(0, pfc_clock_mono2real(&pt1, &pt2));
    ASSERT_TRUE((pt1.tv_sec != 0) || (pt1.tv_nsec != 0));

    /* test correctness real time value */
    timespec_clear(&pt1);
    timespec_clear(&pt2);
    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &before));
    EXPECT_EQ(0, pfc_clock_gettime(&pt2));
    EXPECT_EQ(0, pfc_clock_mono2real(&pt1, &pt2));
    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &after));
    ASSERT_GE(0, pfc_clock_compare((pfc_timespec_t *)&before, &pt1));
    ASSERT_GE(0, pfc_clock_compare(&pt1, (pfc_timespec_t *)&after));

    /* test diff between before relative time and absolute time */
    timespec_clear(&pt1);
    timespec_clear(&pt2);
    EXPECT_EQ(0, pfc_clock_gettime(&pt2));
    EXPECT_EQ(0, pfc_clock_mono2real(&pt1, &pt2));
    pt2.tv_sec += diff;
    EXPECT_EQ(0, pfc_clock_mono2real(&pt3, &pt2));
    pfc_timespec_sub(&pt3, &pt1);
    ASSERT_GE(diff, pt3.tv_sec);
    ASSERT_LE(pt3.tv_sec, diff + 1); /* 1 is small error */

    /* EINVAL test */
    timespec_clear(&pt1);
    timespec_clear(&pt2);
    EXPECT_EQ(0, pfc_clock_gettime(&pt2));
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1;
    EXPECT_EQ(0, pfc_clock_mono2real(&pt1, &pt2));
    timespec_clear(&pt1);
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC;
    ASSERT_EQ(EINVAL, pfc_clock_mono2real(&pt1, &pt_inval));
    pt_inval.tv_sec = -1;
    pt_inval.tv_nsec = 0;
    ASSERT_EQ(EINVAL, pfc_clock_mono2real(&pt1, &pt_inval));

    /* ERANGE test */
    timespec_clear(&pt1);
    pt_inval.tv_sec = PFC_LONG_MAX;
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1;
    ASSERT_EQ(ERANGE, pfc_clock_mono2real(&pt1, &pt_inval));
    timespec_clear(&pt1);
    pt_inval.tv_sec = PFC_LONG_MAX;
    pt_inval.tv_nsec = 0;
    ASSERT_EQ(ERANGE, pfc_clock_mono2real(&pt1, &pt_inval));

    ASSERT_EQ(0, pfc_clock_get_realtime(&pt2));
    ASSERT_EQ(0, pfc_clock_gettime(&pt3));
    pt_inval.tv_sec = (pfc_ulong_t)PFC_LONG_MAX + 1 - pt2.tv_sec;
    pt_inval.tv_nsec = PFC_CLOCK_NANOSEC - 1 - pt2.tv_nsec;
    pfc_timespec_add(&pt_inval, &pt3);
    ASSERT_EQ(ERANGE, pfc_clock_mono2real(&pt1, &pt_inval));

    pt_inval.tv_sec -= 2;
    ASSERT_EQ(0, pfc_clock_mono2real(&pt1, &pt_inval));
    

#ifndef	PFC_LP64
    // EFAULT test.
    timespec_clear(&pt1);
    ASSERT_EQ(EFAULT, pfc_clock_mono2real(NULL, &pt1));
#endif	/* PFC_LP64 */
}

/* test for pfc_clock_compare */
TEST(clock, test_pfc_clock_compare)
{
    pfc_timespec_t pt1, pt2;

    /* test pt1 == pt2 */
    pt1.tv_sec  = 0;
    pt1.tv_nsec = 1;
    pt2.tv_sec  = 0;
    pt2.tv_nsec = 1;
    ASSERT_EQ(0, pfc_clock_compare(&pt1, &pt2));
    ASSERT_EQ(0, pfc_clock_compare(&pt1, &pt1));
    ASSERT_EQ(0, pfc_clock_compare(&pt2, &pt2));

    /* test pt1 > pt2 */
    pt1.tv_nsec = 2;
    ASSERT_EQ(1, pfc_clock_compare(&pt1, &pt2));

    /* test pt1 > pt2 */
    pt1.tv_sec  = 1;
    pt1.tv_nsec = 1;
    ASSERT_EQ(1, pfc_clock_compare(&pt1, &pt2));

    /* test pt1 < pt2 */
    pt1.tv_sec  = 0;
    pt2.tv_nsec = 2;
    ASSERT_EQ(-1, pfc_clock_compare(&pt1, &pt2));

    /* test pt1 < pt2 */
    pt2.tv_sec  = 1;
    pt2.tv_nsec = 0;
    ASSERT_EQ(-1, pfc_clock_compare(&pt1, &pt2));

    /* real time != monotonic time check */
    int i;
    for (i = 0; i < LOOP; i++) {
        pfc_timespec_t test1, test2;
        EXPECT_EQ(0, pfc_clock_gettime(&test1));
        EXPECT_EQ(0, pfc_clock_get_realtime(&test2));

        EXPECT_NE(0, pfc_clock_compare(&test1, &test2));
    }
}

/* test for pfc_clock_isexpired */
TEST(clock, test_pfc_clock_isexpired)
{
    pfc_timespec_t pt1, pt2, pt3;

    /* prepare interval */
    pt3.tv_sec = 1;
    pt3.tv_nsec = 1;

    /* set abs clock */
    EXPECT_EQ(0, pfc_clock_abstime(&pt2, &pt3));

    /* test normal */
    ASSERT_EQ(0, pfc_clock_isexpired(&pt1, &pt2));
    ASSERT_LE(0, pt1.tv_sec);

    /* prepare test ETIMEDOUT */
    EXPECT_EQ(0, pfc_clock_gettime(&pt3)); 

    /* test ETIMEDOUT */
    ASSERT_EQ(ETIMEDOUT, pfc_clock_isexpired(&pt1, &pt3));
}

/* test function for test_pf c_timespec_add */
pfc_timespec_t*
test_timespec_add(const pfc_timespec_t* pt1, const pfc_timespec_t* pt2)
{
    pfc_timespec_t* tmp = (pfc_timespec_t*) malloc(sizeof(pfc_timespec_t));
    if (tmp == NULL) {
        return NULL;
    }
    /* add arguments */
    tmp->tv_sec = pt1->tv_sec + pt2->tv_sec;
    tmp->tv_nsec = pt1->tv_nsec + pt2->tv_nsec;

    /* fix nanosec */
    tmp->tv_sec += tmp->tv_nsec / PFC_CLOCK_NANOSEC;
    tmp->tv_nsec = tmp->tv_nsec % PFC_CLOCK_NANOSEC;

    return tmp;
}

/* test for pfc_timespec_add */
TEST(clock, test_pfc_timespec_add)
{
    int i;

    /* create random generator */
    struct timeval tv;
    uint32_t seed;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        pfc_log_warn("errno = %d", errno);
        ASSERT_EQ(0, ret);
    }

    seed = tv.tv_usec % UINT32_MAX;
    boost::mt19937 gen(seed);
    boost::uniform_int<> dst(1, INT32_MAX);
    boost::variate_generator<boost::mt19937&,
                             boost::uniform_int<> > rndm(gen, dst);

    /* random test */
    for (i = 0; i < LOOP; i++) {
        pfc_timespec_t pt1, pt2;
        pfc_timespec_t* ptr;
        /* prepare test data */
        if (i == 0) {
            /* for test fix up
             * 500,000,000 < tv_nsec < 1,000,000,000
             */
            pt1.tv_sec = 0;
            pt1.tv_nsec = (PFC_CLOCK_NANOSEC / 2) +
                          rndm() % (PFC_CLOCK_NANOSEC / 2);
            pt2.tv_sec = 0;
            pt2.tv_nsec = (PFC_CLOCK_NANOSEC / 2) +
                          rndm() % (PFC_CLOCK_NANOSEC / 2);
        } else {
            pt1.tv_sec = rndm();
            pt1.tv_nsec = rndm();
            pt2.tv_sec = rndm();
            pt2.tv_nsec = rndm();
        }
        /* test add timespec */
        ptr = test_timespec_add(&pt1, &pt2);
        pfc_timespec_add(&pt1, &pt2);
        if (ptr == NULL) {
            EXPECT_TRUE(false);
        }

        ret = pfc_clock_compare(&pt1, ptr);
        if (ret != 0) {
            pfc_log_warn("pt1.tv_sec = %lu", pt1.tv_sec);
            pfc_log_warn("pt1.tv_nsec = %lu", pt1.tv_nsec);
            pfc_log_warn("ptr->tv_sec = %lu", ptr->tv_sec);
            pfc_log_warn("ptr->tv_nsec = %lu", ptr->tv_nsec);
            ASSERT_EQ(0, ret);
        }
        if (i == 0) {
            ASSERT_EQ(1, pt1.tv_sec);
            ASSERT_GT(PFC_CLOCK_NANOSEC, (pfc_ulong_t)pt1.tv_nsec);
        }
        free(ptr);
    }
}

/* test for commutativeity of pfc_timespec_add */
TEST(clock, test_pfc_timespec_add_commutativity)
{
    int i;

    /* create random generator */
    struct timeval tv;
    uint32_t seed;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        pfc_log_warn("errno = %d", errno);
        ASSERT_EQ(0, ret);
    }

    seed = tv.tv_usec % UINT32_MAX;
    boost::mt19937 gen(seed);
    boost::uniform_int<> dst(1, INT32_MAX);
    boost::variate_generator<boost::mt19937&,
                             boost::uniform_int<> > rndm(gen, dst);

    /* random test */
    for (i = 0; i < LOOP; i++) {
        pfc_timespec_t pt1, pt2, pt1_copy, pt2_copy;

        /* prepare test data */
        pt1.tv_sec = rndm();
        pt1.tv_nsec = rndm();
        pt1_copy.tv_sec = pt1.tv_sec;
        pt1_copy.tv_nsec = pt1.tv_nsec;
        pt2.tv_sec = rndm();
        pt2.tv_nsec = rndm();
        pt2_copy.tv_sec = pt2.tv_sec;
        pt2_copy.tv_nsec = pt2.tv_nsec;

        /* test add timespec */
        pfc_timespec_add(&pt1, &pt2);
        pfc_timespec_add(&pt2_copy, &pt1_copy);

        /*
         * check value
         */
        ret = pfc_clock_compare(&pt1, &pt2_copy);
        if (ret != 0) {
            pfc_log_warn("pt1.tv_sec = %lu", pt1.tv_sec);
            pfc_log_warn("pt1.tv_nsec = %lu", pt1.tv_nsec);
            pfc_log_warn("pt2.tv_sec = %lu", pt2_copy.tv_sec);
            pfc_log_warn("pt2.tv_nsec = %lu", pt2_copy.tv_nsec);
            ASSERT_EQ(0, ret);
        }


    }
}

/* test for associativity of pfc_timespec_add */
TEST(clock, test_pfc_timespec_add_associativity)
{
    int i;

    /* create random generator */
    struct timeval tv;
    uint32_t seed;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        pfc_log_warn("errno = %d", errno);
        ASSERT_EQ(0, ret);
    }

    seed = tv.tv_usec % UINT32_MAX;
    boost::mt19937 gen(seed);
    boost::uniform_int<> dst(1, INT32_MAX);
    boost::variate_generator<boost::mt19937&,
                             boost::uniform_int<> > rndm(gen, dst);

    /* random test */
    for (i = 0; i < LOOP; i++) {
        pfc_timespec_t pt1, pt2, pt3, pt1_copy, pt2_copy, pt3_copy;

        /* prepare test data */
        pt1.tv_sec = rndm();
        pt1.tv_nsec = rndm();
        pt1_copy.tv_sec = pt1.tv_sec;
        pt1_copy.tv_nsec = pt1.tv_nsec;
        pt2.tv_sec = rndm();
        pt2.tv_nsec = rndm();
        pt2_copy.tv_sec = pt2.tv_sec;
        pt2_copy.tv_nsec = pt2.tv_nsec;
        pt3.tv_sec = rndm();
        pt3.tv_nsec = rndm();
        pt3_copy.tv_sec = pt3.tv_sec;
        pt3_copy.tv_nsec = pt3.tv_nsec;

        /* test add timespec */
        pfc_timespec_add(&pt1, &pt2);
        pfc_timespec_add(&pt1, &pt3);
        pfc_timespec_add(&pt2_copy, &pt3_copy);
        pfc_timespec_add(&pt2_copy, &pt1_copy);

        /*
         * check value
         */
        ret = pfc_clock_compare(&pt1, &pt2_copy);
        if (ret != 0) {
            pfc_log_warn("pt1.tv_sec = %lu", pt1.tv_sec);
            pfc_log_warn("pt1.tv_nsec = %lu", pt1.tv_nsec);
            pfc_log_warn("pt2.tv_sec = %lu", pt2_copy.tv_sec);
            pfc_log_warn("pt2.tv_nsec = %lu", pt2_copy.tv_nsec);
            ASSERT_EQ(0, ret);
        }
    }
}

/* test function for test_pfc_timespec_sub */
pfc_timespec_t*
test_timespec_sub(const pfc_timespec_t* pt1, const pfc_timespec_t* pt2)
{
    pfc_timespec_t* tmp = (pfc_timespec_t*) malloc(sizeof(pfc_timespec_t));
    if (tmp == NULL) {
        return NULL;
    }
    /* sub arguments */
    tmp->tv_sec = pt1->tv_sec - pt2->tv_sec;
    tmp->tv_nsec = pt1->tv_nsec - pt2->tv_nsec;

    /* fix up */
    if (tmp->tv_nsec < 0) {
        tmp->tv_sec -= 1;
        tmp->tv_nsec += PFC_CLOCK_NANOSEC;
    }

    return tmp;
}


/* test for pfc_timespec_sub */
TEST(clock, test_pfc_timespec_sub)
{
    int i;

    /* create random generator */
    struct timeval tv;
    uint32_t seed;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        pfc_log_warn("errno = %d", errno);
        ASSERT_EQ(0, ret);
    }

    seed = tv.tv_usec % UINT32_MAX;
    boost::mt19937 gen(seed);
    boost::uniform_int<> dst(1, INT32_MAX);
    boost::variate_generator<boost::mt19937&,
                             boost::uniform_int<> > rndm(gen, dst);

    /* random test */
    for (i = 0; i < LOOP; i++) {
        pfc_timespec_t pt1, pt2;
        pfc_timespec_t* ptr;

        /* prepare interval */
        do {
            if (i == 0) { /* check cut off */
                pt1.tv_sec = 1;
                pt1.tv_nsec = 0;
                pt2.tv_sec = 0;
                pt2.tv_nsec = rndm() % PFC_CLOCK_NANOSEC;
            } else if (i == 2) { /* check same value */
                pt1.tv_sec = rndm();
                pt1.tv_nsec = rndm() % PFC_CLOCK_NANOSEC;
                pt1.tv_sec = pt1.tv_sec;
                pt1.tv_nsec = pt1.tv_nsec;
            } else {
                pt1.tv_sec = rndm();
                pt1.tv_nsec = rndm() % PFC_CLOCK_NANOSEC;
                pt2.tv_sec = rndm();
                pt2.tv_nsec = rndm() % PFC_CLOCK_NANOSEC;
            }

            /* check (pt1 >= pt2) */
            if (pt1.tv_sec > pt2.tv_sec || (pt1.tv_sec == pt2.tv_sec &&
                                            pt1.tv_nsec >= pt2.tv_nsec)) {
                break;
            }
        } while (1);

        /* test sub timespec */
        ptr = test_timespec_sub(&pt1, &pt2);
        pfc_timespec_sub(&pt1, &pt2);
        if (ptr == NULL) {
            EXPECT_TRUE(false);
        }

        /* check value */
        ret = pfc_clock_compare(&pt1, ptr);
        if (ret != 0) {
            pfc_log_warn("pt1.tv_sec = %lu", pt1.tv_sec);
            pfc_log_warn("pt1.tv_nsec = %lu", pt1.tv_nsec);
            pfc_log_warn("pt2.tv_sec = %lu", ptr->tv_sec);
            pfc_log_warn("pt2.tv_nsec = %lu", ptr->tv_nsec);
            ASSERT_EQ(0, ret);
        }

        /* check cut off */
        if (i == 0) {
            ASSERT_EQ(0, pt1.tv_sec);
            ASSERT_GT(PFC_CLOCK_NANOSEC, (pfc_ulong_t)pt1.tv_nsec);
        }

        free(ptr);
    }
}

/* test for pfc_clock_sec2time() */
TEST(clock, test_pfc_clock_sec2time)
{
    // Convert zero.
    pfc_timespec_t ts;
    pfc_clock_sec2time(&ts, 0);
    ASSERT_EQ(0, ts.tv_sec);
    ASSERT_EQ(0, ts.tv_nsec);

    // Convert one second.
    pfc_clock_sec2time(&ts, 1);
    ASSERT_EQ(1, ts.tv_sec);
    ASSERT_EQ(0, ts.tv_nsec);

    // Create random generator.
    struct timeval tv;
    uint32_t seed;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        pfc_log_warn("errno = %d", errno);
        ASSERT_EQ(0, ret);
    }

    seed = tv.tv_usec % UINT32_MAX;
    boost::mt19937 gen(seed);
    boost::uniform_int<pfc_long_t> dst(1, PFC_LONG_MAX);
    boost::variate_generator<boost::mt19937&,
                             boost::uniform_int<pfc_long_t> > rndm(gen, dst);

    // Test with random value.
    for (int i(0); i < LOOP; i++) {
        time_t	sec(rndm());

        pfc_clock_sec2time(&ts, sec);
        ASSERT_EQ(sec, ts.tv_sec);
        if (ts.tv_nsec != 0) {
            pfc_log_warn("sec = %lu", static_cast<pfc_ulong_t>(sec));
            ASSERT_EQ(0, ts.tv_nsec);
        }
    }
}

/* test for pfc_clock_time2sec() */
TEST(clock, test_pfc_clock_time2sec)
{
    // Convert zero.
    pfc_timespec_t ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    ASSERT_EQ(0ULL, pfc_clock_time2sec(&ts));

    // Convert one second.
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    ASSERT_EQ(1ULL, pfc_clock_time2sec(&ts));

    // Verify rounding off.
    const pfc_ulong_t halfdiv(PFC_CLOCK_NANOSEC >> 1);
    for (time_t sec(0); sec < 10; sec++) {
        ts.tv_sec = sec;

        for (time_t nsec(1); nsec <= 10; nsec++) {
            ts.tv_nsec = nsec;
            if (static_cast<pfc_ulong_t>(sec) != pfc_clock_time2sec(&ts)) {
                pfc_log_warn("tv_sec = %lu", ts.tv_sec);
                pfc_log_warn("tv_nsec = %lu", ts.tv_nsec);
                ASSERT_EQ(static_cast<pfc_ulong_t>(sec),
                          pfc_clock_time2sec(&ts));
            }
        }

        for (time_t nsec(1); nsec <= 10; nsec++) {
            ts.tv_nsec = halfdiv - nsec;
            if (static_cast<pfc_ulong_t>(sec) != pfc_clock_time2sec(&ts)) {
                pfc_log_warn("tv_sec = %lu", ts.tv_sec);
                pfc_log_warn("tv_nsec = %lu", ts.tv_nsec);
                ASSERT_EQ(static_cast<pfc_ulong_t>(sec),
                          pfc_clock_time2sec(&ts));
            }
        }

        for (time_t nsec(0); nsec <= 10; nsec++) {
            ts.tv_nsec = halfdiv + nsec;
            if (static_cast<pfc_ulong_t>(sec + 1) != pfc_clock_time2sec(&ts)) {
                pfc_log_warn("tv_sec = %lu", ts.tv_sec);
                pfc_log_warn("tv_nsec = %lu", ts.tv_nsec);
                ASSERT_EQ(static_cast<pfc_ulong_t>(sec + 1),
                          pfc_clock_time2sec(&ts));
            }
        }
    }

    // Maximum value conversion.
    {
        ts.tv_sec = PFC_LONG_MAX;
        ts.tv_nsec = PFC_CLOCK_NANOSEC - 1;
        pfc_ulong_t expected(static_cast<pfc_ulong_t>(PFC_LONG_MAX) + 1);
        ASSERT_EQ(expected, pfc_clock_time2sec(&ts));
    }

    // Create random generator.
    struct timeval tv;
    uint32_t seed;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        pfc_log_warn("errno = %d", errno);
        ASSERT_EQ(0, ret);
    }

    seed = tv.tv_usec % UINT32_MAX;
    boost::mt19937 gen(seed);
    boost::uniform_int<pfc_long_t> dst(1, PFC_LONG_MAX);
    boost::variate_generator<boost::mt19937&,
                             boost::uniform_int<pfc_long_t> > rndm(gen, dst);

    // Test with random value.
    for (int i(0); i < LOOP; i++) {
        ts.tv_sec = rndm();
        ts.tv_nsec = rndm() % PFC_CLOCK_NANOSEC;

        pfc_ulong_t	expected(static_cast<pfc_ulong_t>(ts.tv_sec));
        if (static_cast<pfc_ulong_t>(ts.tv_nsec) >= halfdiv) {
            expected++;
        }

        if (expected != pfc_clock_time2sec(&ts)) {
            pfc_log_warn("tv_sec = %lu", ts.tv_sec);
            pfc_log_warn("tv_nsec = %lu", ts.tv_nsec);
            ASSERT_EQ(expected, pfc_clock_time2sec(&ts));
        }

        // Reconversion test.
        pfc_timespec_t  reconv;
        pfc_clock_sec2time(&reconv, static_cast<time_t>(expected));
        if (expected != static_cast<pfc_ulong_t>(reconv.tv_sec)) {
            pfc_log_warn("tv_sec = %lu", reconv.tv_sec);
            pfc_log_warn("tv_nsec = %lu", reconv.tv_nsec);
            ASSERT_EQ(expected, static_cast<pfc_ulong_t>(reconv.tv_sec));
        }
        ASSERT_EQ(0, reconv.tv_nsec);
    }
}

/* test for pfc_clock_msec2time */
TEST(clock, test_pfc_clock_msec2time)
{
    // Convert zero.
    pfc_timespec_t ts;
    pfc_clock_msec2time(&ts, 0);
    ASSERT_EQ(0, ts.tv_sec);
    ASSERT_EQ(0, ts.tv_nsec);

    // Convert one millisecond.
    pfc_clock_msec2time(&ts, 1);
    ASSERT_EQ(0, ts.tv_sec);
    ASSERT_EQ(PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC,
              static_cast<pfc_ulong_t>(ts.tv_nsec));

    // Convert one second.
    pfc_clock_msec2time(&ts, PFC_CLOCK_MILLISEC);
    ASSERT_EQ(1, ts.tv_sec);
    ASSERT_EQ(0, ts.tv_nsec);

    // Convert maximum value.
    pfc_clock_msec2time(&ts, PFC_ULONG_MAX);
#ifdef	PFC_LP64
    ASSERT_EQ(18446744073709551ULL, static_cast<pfc_ulong_t>(ts.tv_sec));
    ASSERT_EQ(615000000UL, static_cast<pfc_ulong_t>(ts.tv_nsec));
#else	/* !PFC_LP64 */
    ASSERT_EQ(4294967UL, static_cast<pfc_ulong_t>(ts.tv_sec));
    ASSERT_EQ(295000000UL, static_cast<pfc_ulong_t>(ts.tv_nsec));
#endif	/* PFC_LP64 */

    // Create random generator.
    struct timeval tv;
    uint32_t seed;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        pfc_log_warn("errno = %d", errno);
        ASSERT_EQ(0, ret);
    }

    seed = tv.tv_usec % UINT32_MAX;
    boost::mt19937 gen(seed);
    boost::uniform_real<> dst(1, PFC_ULONG_MAX);
    boost::variate_generator<boost::mt19937&,
                             boost::uniform_real<> > rndm(gen, dst);

    // Test with random value.
    for (int i(0); i < LOOP; i++) {
        uint64_t  msec(static_cast<uint64_t>(rndm()));

        pfc_clock_msec2time(&ts, msec);
        pfc_ulong_t  exp_sec(static_cast<pfc_ulong_t>
                             (msec / PFC_CLOCK_MILLISEC));
        pfc_ulong_t  exp_nsec(static_cast<pfc_ulong_t>
                              ((msec % PFC_CLOCK_MILLISEC) *
                               (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC)));
        if (exp_sec != static_cast<pfc_ulong_t>(ts.tv_sec)) {
            pfc_log_warn("msec = %" PFC_PFMT_u64, msec);
            ASSERT_EQ(exp_sec, static_cast<pfc_ulong_t>(ts.tv_sec));
        }
        if (exp_nsec != static_cast<pfc_ulong_t>(ts.tv_nsec)) {
            pfc_log_warn("msec = %" PFC_PFMT_u64, msec);
             ASSERT_EQ(exp_nsec, static_cast<pfc_ulong_t>(ts.tv_nsec));
        }


        // Reconversion test.
        if (msec != pfc_clock_time2msec(&ts)) {
            pfc_log_warn("msec = %" PFC_PFMT_u64, msec);
            pfc_log_warn("tv_sec = %lu", ts.tv_sec);
            pfc_log_warn("tv_nsec = %lu", ts.tv_nsec);
            ASSERT_EQ(msec, pfc_clock_time2msec(&ts));
        }
    }
}

/* test for pfc_clock_time2msec() */
TEST(clock, test_pfc_clock_time2msec)
{
    // Convert zero.
    pfc_timespec_t ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    ASSERT_EQ(0ULL, pfc_clock_time2msec(&ts));

    // Convert one millisecond.
    ts.tv_sec = 0;
    ts.tv_nsec = PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC;
    ASSERT_EQ(1ULL, pfc_clock_time2msec(&ts));

    // Convert one second.
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    ASSERT_EQ(PFC_CLOCK_MILLISEC, pfc_clock_time2msec(&ts));

    // Verify rounding off.
    const pfc_ulong_t divisor(PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);
    const pfc_ulong_t halfdiv(divisor >> 1);
    ts.tv_sec = 0;
    for (int i(1); i <= 10; i++) {
        ts.tv_nsec = i;
        ASSERT_EQ(0ULL, pfc_clock_time2msec(&ts));
    }

    {
        uint64_t msec(0);
        for (pfc_ulong_t nsec(0); nsec < PFC_CLOCK_NANOSEC;
             nsec += divisor, msec++) {
            for (int i(1); i <= 10; i++) {
                ts.tv_nsec = nsec + halfdiv - i;
                ASSERT_EQ(msec, pfc_clock_time2msec(&ts));
            }
            for (int i(0); i <= 10; i++) {
                ts.tv_nsec = nsec + halfdiv + i;
                ASSERT_EQ(msec + 1, pfc_clock_time2msec(&ts));
            }
        }
    }

    // Maximum value conversion.
    {
        ts.tv_sec = PFC_LONG_MAX / PFC_CLOCK_MILLISEC;
        ts.tv_nsec = PFC_CLOCK_NANOSEC - 1;
#ifdef	PFC_LP64
        ASSERT_EQ(9223372036854776000ULL, pfc_clock_time2msec(&ts));
#else	/* !PFC_LP64 */
        ASSERT_EQ(2147484000ULL, pfc_clock_time2msec(&ts));
#endif	/* PFC_LP64 */
    }

    // Create random generator.
    struct timeval tv;
    uint32_t seed;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        pfc_log_warn("errno = %d", errno);
        ASSERT_EQ(0, ret);
    }

    seed = tv.tv_usec % UINT32_MAX;
    boost::mt19937 gen(seed);
    boost::uniform_int<pfc_long_t> dst(1, PFC_LONG_MAX / PFC_CLOCK_MILLISEC);
    boost::variate_generator<boost::mt19937&,
                             boost::uniform_int<pfc_long_t> > rndm(gen, dst);

    // Test with random value.
    for (int i(0); i < LOOP; i++) {
        ts.tv_sec = rndm();
        ts.tv_nsec = rndm() % PFC_CLOCK_NANOSEC;

        uint64_t  expected(static_cast<uint64_t>(ts.tv_sec) *
                           PFC_CLOCK_MILLISEC + ts.tv_nsec / divisor);
        bool roundoff(false);
        if ((ts.tv_nsec % divisor) >= halfdiv) {
            expected++;
            roundoff = true;
        }

        if (expected != pfc_clock_time2msec(&ts)) {
            pfc_log_warn("tv_sec = %lu", ts.tv_sec);
            pfc_log_warn("tv_nsec = %lu", ts.tv_nsec);
            ASSERT_EQ(expected, pfc_clock_time2msec(&ts));
        }

        // Reconversion test.
        pfc_timespec_t  reconv;
        pfc_clock_msec2time(&reconv, expected);
        pfc_ulong_t  exp_sec(ts.tv_sec);
        pfc_ulong_t  exp_nsec(static_cast<pfc_ulong_t>(ts.tv_nsec) / divisor *
                              divisor);
        if (roundoff) {
            exp_nsec += divisor;
            if (exp_nsec >= PFC_CLOCK_NANOSEC) {
                exp_nsec -= PFC_CLOCK_NANOSEC;
                exp_sec++;
            }
        }

        if (exp_sec != static_cast<pfc_ulong_t>(reconv.tv_sec)) {
            pfc_log_warn("msec = %" PFC_PFMT_u64, expected);
            pfc_log_warn("tv_sec = %lu", reconv.tv_sec);
            pfc_log_warn("tv_nsec = %lu", reconv.tv_nsec);
            pfc_log_warn("ts.tv_sec = %lu", ts.tv_sec);
            pfc_log_warn("ts.tv_nsec = %lu", ts.tv_nsec);
            ASSERT_EQ(exp_sec, static_cast<pfc_ulong_t>(reconv.tv_sec));
        }
        if (exp_nsec != static_cast<pfc_ulong_t>(reconv.tv_nsec)) {
            pfc_log_warn("msec = %" PFC_PFMT_u64, expected);
            pfc_log_warn("tv_sec = %lu", reconv.tv_sec);
            pfc_log_warn("tv_nsec = %lu", reconv.tv_nsec);
            pfc_log_warn("ts.tv_sec = %lu", ts.tv_sec);
            pfc_log_warn("ts.tv_nsec = %lu", ts.tv_nsec);
            ASSERT_EQ(exp_nsec, static_cast<pfc_ulong_t>(reconv.tv_nsec));
        }
    }
}
