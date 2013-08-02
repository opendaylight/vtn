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

#include <string.h>
#include <gtest/gtest.h>
#include <pfc/util.h>
#include <pfc/clock.h>

#define BUF_SIZE 32
#define STRFTIME_LEN 19 /* strftime format string length */

/*
 *
 * test cases
 *
 */ 

/* test form pfc_time_ctime() */
TEST(clock, test_pfc_time_ctime)
{
    size_t buf_size = BUF_SIZE;
    char buf[buf_size];

    /* open file to write report */
    const char *path = getenv("TEST_LIBPFC_UTIL_TIME_PATH");
    if (path == NULL) {
        path = (const char *)"/dev/null";
    }
    FILE* fp;
    if (strcmp("-", path) == 0) {
        fp = stdout;
    }  else {
        fp = fopen(path, "w");
        if (fp == NULL) {
            perror(path);
            fprintf(stderr, "Failed to open test time report file\n");
            abort();
        }
    }

    /* test normal */
    ASSERT_EQ(0, pfc_time_ctime(buf, buf_size));
    fprintf(fp, "pfc_time_ctime is %s\n", buf);

    /* size ZERO test */
    ASSERT_EQ(ENOSPC, pfc_time_ctime(buf, 0));
    /* test don't convert all time string to buffer
     * if ZERO < arg2 < time_format size == 19
     * strftime format string "%Y-%m-%d %H:%M:%S"
     * string is "year:month:day hour:minute:second" 
     * string size is 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 = 19
     */
    for (int j = 0; j < STRFTIME_LEN; j++) {
        /* init buffer */
        for (int i = 0; i < BUF_SIZE; i++) {
            if (i != BUF_SIZE - 1) {
                buf[i] = 'a';
            } else {
                buf[i] = '\0';
            }
        }

        /* check return ENOSPC */
        EXPECT_EQ(ENOSPC, pfc_time_ctime(buf, j + 1));

        /* check to fill rest buffer with 'a' */
        for (int h = j + 1; h < BUF_SIZE; h++) {
            if (h != BUF_SIZE - 1) {
                ASSERT_EQ('a', buf[h]);
            } else {
                ASSERT_EQ('\0', buf[h]);
            }
        }
    }

    /*
     * NULL buf test
     * pfc_time_ctime return 0
     * this is not expected use case.
     */
    ASSERT_EQ(0, pfc_time_ctime(NULL, buf_size));
    fclose(fp);
}

/* test for pfc_time_asctime() */
TEST(clock, test_pfc_time_asctime)
{
    size_t buf_size = BUF_SIZE;
    char buf[buf_size];

    time_t timer;
    struct tm *t_st;

    /* open file to write report */
    const char *path = getenv("TEST_LIBPFC_UTIL_TIME_PATH");
    if (path == NULL) {
        path = (const char *)"/dev/null";
    }
    FILE* fp;
    if (strcmp("-", path) == 0) {
        fp = stdout;
    }  else {
        fp = fopen(path, "a");
        if (fp == NULL) {
            perror(path);
            fprintf(stderr, "Failed to open test time report file\n");
            abort();
        }
    }

    /* prepare tm structure */
    time(&timer);
    t_st = localtime(&timer);
    EXPECT_TRUE(t_st != NULL);

    /* test normal */
    ASSERT_EQ(0, pfc_time_asctime(t_st, buf, buf_size));
    fprintf(fp, "pfc_time_asctime is %s\n", buf);

    /* size ZERO test */
    ASSERT_EQ(ENOSPC, pfc_time_asctime(t_st, buf, 0));

    /* test don't convert all time string to buffer
     * if ZERO < arg3 < time_format size == 19
     * strftime format string "%Y-%m-%d %H:%M:%S"
     * string is "year:month:day hour:minute:second"
     * string size is 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 = 19
     */
    for (int j = 0; j < STRFTIME_LEN; j++) {
        /* init buffer */
        for (int i = 0; i < BUF_SIZE; i++) {
            if (i != BUF_SIZE - 1) {
                buf[i] = 'a';
            } else {
                buf[i] = '\0';
            }
        }

        /* check return ENOSPC */
        EXPECT_EQ(ENOSPC, pfc_time_asctime(t_st, buf, j + 1));

        /* check to fill rest buffer with 'a' */
        for (int h = j + 1; h < BUF_SIZE; h++) {
            if (h != BUF_SIZE - 1) {
                ASSERT_EQ('a', buf[h]);
            } else {
                ASSERT_EQ('\0', buf[h]);
            }
        }
    }

    /*
     * NULL buf test
     * pfc_time_asctime return 0
     * but this is not expected use case.
     */
    ASSERT_EQ(0, pfc_time_asctime(t_st, NULL, buf_size)); 
    fclose(fp);
}

/* test for pfc_time_clock_ctime() */
TEST(clock, test_pfc_time_clock_ctime)
{
    size_t buf_size = BUF_SIZE;
    char buf[BUF_SIZE];

    /* open file to write report */
    const char *path = getenv("TEST_LIBPFC_UTIL_TIME_PATH");
    if (path == NULL) {
        path = (const char *)"/dev/null";
    }
    FILE* fp;
    if (strcmp("-", path) == 0) {
        fp = stdout;
    }  else {
        fp = fopen(path, "a");
        if (fp == NULL) {
            perror(path);
            fprintf(stderr, "Failed to open test time report file\n");
            abort();
        }
    }

    /* test normal */
    ASSERT_EQ(0, pfc_time_clock_ctime(buf, buf_size));
    /* check by eye */
    fprintf(fp, "pfc_time_clock_ctime is %s\n", buf);

    /* size ZERO test */
    ASSERT_EQ(ENOSPC, pfc_time_clock_ctime(buf, 0));

    /* test don't convert all time string to buffer
     * if ZERO < arg3 < time_format size == 19
     * strftime format string "%Y-%m-%d %H:%M:%S"
     * string is "year:month:day hour:minute:second"
     * string size is 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 = 19
     */
    for (int j = 0; j < STRFTIME_LEN; j++) {
        /* init buffer */
        for (int i = 0; i < BUF_SIZE; i++) {
            if (i != BUF_SIZE - 1) {
                buf[i] = 'a';
            } else {
                buf[i] = '\0';
            }
        }

        /* check return ENOSPC */
        EXPECT_EQ(ENOSPC, pfc_time_clock_ctime(buf, j + 1));

        /* check to fill rest buffer with 'a' */
        for (int h = j + 1; h < BUF_SIZE; h++) {
            if (h != BUF_SIZE - 1) {
                ASSERT_EQ('a', buf[h]);
            } else {
                ASSERT_EQ('\0', buf[h]);
            }
        }
    }

    fclose(fp);
}

/* test for pfc_time_clock_asctime */
TEST(clock, test_pfc_time_clock_asctime)
{
    size_t buf_size = BUF_SIZE;
    char buf[BUF_SIZE];

     pfc_timespec_t t_st;

    /* open file to write report */
    const char *path = getenv("TEST_LIBPFC_UTIL_TIME_PATH");
    if (path == NULL) {
        path = (const char *)"/dev/null";
    }
    FILE* fp;
    if (strcmp("-", path) == 0) {
        fp = stdout;
    }  else {
        fp = fopen(path, "a");
        if (fp == NULL) {
            perror(path);
            fprintf(stderr, "Failed to open test time report file\n");
            abort();
        }
    }

    /* prepare timespec structure */
    EXPECT_EQ(0, pfc_clock_get_realtime(&t_st));

    /* test normal */
    ASSERT_EQ(0, pfc_time_clock_asctime(&t_st, buf, buf_size));
    fprintf(fp, "pfc_time_clock_asctime is %s\n", buf);

    /* size ZERO test */
    ASSERT_EQ(ENOSPC, pfc_time_clock_asctime(&t_st, buf, 0));

    /* test don't convert all time string to buffer
     * if ZERO < arg3 < time_format size == 19
     * strftime format string "%Y-%m-%d %H:%M:%S"
     * string is "year:month:day hour:minute:second"
     * string size is 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 = 19
     */
    for (int j = 0; j < STRFTIME_LEN; j++) {
        /* init buffer */
        for (int i = 0; i < BUF_SIZE; i++) {
            if (i != BUF_SIZE - 1) {
                buf[i] = 'a';
            } else {
                buf[i] = '\0';
            }
        }

        /* check return ENOSPC */
        EXPECT_EQ(ENOSPC, pfc_time_clock_asctime(&t_st, buf, j + 1));

        /* check to fill rest buffer with 'a' */
        for (int h = j + 1; h < BUF_SIZE; h++) {
            if (h != BUF_SIZE - 1) {
                ASSERT_EQ('a', buf[h]);
            } else {
                ASSERT_EQ('\0', buf[h]);
            }
        }
    }

    /* NULL pfc_timespec_t test */
    ASSERT_EQ(EINVAL, pfc_time_clock_asctime(NULL, buf, buf_size));

    fclose(fp);
}

/* test for pfc_time_gettime */
TEST(clock, test_pfc_time_gettime)
{
    struct tm t_st, before, after;
    struct timespec before_t, after_t;

    /* test normal */
    ASSERT_EQ(0, pfc_time_gettime(&t_st));

    /* test correct tm value */
    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &before_t));
    EXPECT_TRUE(localtime_r(&(before_t.tv_sec), &before) != NULL);

    struct timespec delay;
    if (before_t.tv_nsec == 0) {
        delay.tv_sec = 1;
        delay.tv_nsec = 0;
    }
    else {
        delay.tv_sec = 0;
        delay.tv_nsec = PFC_CLOCK_NANOSEC - before_t.tv_nsec;
    }
    nanosleep(&delay, NULL);

    EXPECT_EQ(0, pfc_time_gettime(&t_st));
    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &after_t));

    if (after_t.tv_nsec == 0) {
        delay.tv_sec = 1;
        delay.tv_nsec = 0;
    }
    else {
        delay.tv_sec = 0;
        delay.tv_nsec = PFC_CLOCK_NANOSEC - after_t.tv_nsec;
    }
    nanosleep(&delay, NULL);

    EXPECT_EQ(0, clock_gettime(CLOCK_REALTIME, &after_t));
    EXPECT_TRUE(localtime_r(&(after_t.tv_sec), &after) != NULL);
    ASSERT_LT(mktime(&before), mktime(&t_st));
    ASSERT_LT(mktime(&t_st), mktime(&after));
}
