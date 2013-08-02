/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for u64hash synch block used by multi-threads
 */

#include <pfc/hash.h>
#include <pfc/thread.h>
#include <unistd.h>
#include "test_hash.hh"


/*
 * Test data
 */

static const size_t nbuckets = 31;
static const char *test_val_body  = "abcd";
static const char *test_val_body2 = "efgh";
static const char *test_val_body3 = "ijkl";

/* stored into hash before do ACTION in TEST_HASH_SYNC macro */
static uint64_t test_key  = 123;
static pfc_cptr_t test_val  = (pfc_cptr_t)test_val_body;

/* not stored into hash before do ACTION in TEST_HASH_SYNC macro */
static uint64_t test_key2 = 456;
static pfc_cptr_t test_val2 = (pfc_cptr_t)test_val_body2;
static pfc_cptr_t test_val3 = (pfc_cptr_t)test_val_body3;


/*
 * Macros to define tests
 */

#define TEST_HASH_WSYNC(NAME, ACTION)           \
    TEST_HASH_SYNC(wsync, NAME, ACTION)

#define TEST_HASH_RSYNC(NAME, ACTION)           \
    TEST_HASH_SYNC(rsync, NAME, ACTION)


#define TEST_HASH_SYNC(SYNC, NAME, ACTION)                          \
    TEST(hash, u64_ ## SYNC ## _ ## NAME)                           \
    {                                                               \
        pfc_hash_t hash;                                            \
        EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0)); \
        EXPECT_EQ(0, pfc_u64hash_put(hash, test_key, test_val, 0)); \
        pfc_hash_ ## SYNC ## _begin(hash);                          \
        do {                                                        \
            ACTION;                                                 \
        } while(0);                                                 \
        pfc_hash_sync_end();                                        \
        pfc_hash_destroy(hash);                                     \
    }


/*
 * Test cases
 */

TEST_HASH_WSYNC(get,
                pfc_cptr_t val;
                EXPECT_EQ(0, pfc_u64hash_get(hash, test_key, &val));
                EXPECT_TRUE(test_val == val);
                );

TEST_HASH_WSYNC(get_size,
                EXPECT_EQ(1U, pfc_hash_get_size(hash));
                );

TEST_HASH_WSYNC(get_capacity,
                EXPECT_EQ(nbuckets, pfc_hash_get_capacity(hash));
                );

TEST_HASH_WSYNC(put,
                EXPECT_EQ(0, pfc_u64hash_put(hash, test_key2, &test_val2, 0));
                );

TEST_HASH_WSYNC(update,
                EXPECT_EQ(0, pfc_u64hash_update(hash, test_key, &test_val, 0));
                );

TEST_HASH_WSYNC(remove,
                EXPECT_EQ(0, pfc_u64hash_remove(hash, test_key));
                );

TEST_HASH_WSYNC(clear,
                EXPECT_EQ(1U, pfc_hash_clear(hash));
                );

TEST_HASH_WSYNC(set_capacity,
                EXPECT_EQ(0, pfc_hash_set_capacity(hash, 37));
                );


static void *
test_ww_sync_thr1 (void *arg)
{
    pfc_hash_t hash = (pfc_hash_t)arg;
    pfc_hash_wsync_begin(hash);     // start wsync block
    EXPECT_EQ(0, pfc_u64hash_put(hash, test_key2, test_val2, 0));
    usleep(200 * 1000);             // sleep 200 msec
    pfc_cptr_t val;
    EXPECT_EQ(0, pfc_u64hash_get(hash, test_key2, &val));
    EXPECT_TRUE(test_val2 == val);
    pfc_hash_sync_end();            // end wsync block
    return NULL;
}

static void *
test_ww_sync_thr2 (void *arg)
{
    pfc_hash_t hash = (pfc_hash_t)arg;
    usleep(100 * 1000);             // sleep 100 msec
    pfc_hash_wsync_begin(hash);     // start wsync block
    EXPECT_EQ(0, pfc_u64hash_update(hash, test_key2, test_val3, 0));
    pfc_hash_sync_end();            // end wsync block
    return NULL;
}

TEST(hash, u64_wsync_and_wsync)
{
    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0));

    // start threads
    pfc_thread_t thr1, thr2;
    EXPECT_EQ(0, pfc_thread_create(&thr1, test_ww_sync_thr1, hash, 0));
    EXPECT_EQ(0, pfc_thread_create(&thr2, test_ww_sync_thr2, hash, 0));

    // wait for threads to terminate
    EXPECT_EQ(0, pfc_thread_join(thr1, NULL));
    EXPECT_EQ(0, pfc_thread_join(thr2, NULL));

    // clean up
    pfc_hash_destroy(hash);
}


static void *
test_wr_sync_thr1 (void *arg)
{
    pfc_hash_t hash = (pfc_hash_t)arg;
    pfc_hash_wsync_begin(hash);     // start wsync block
    usleep(200 * 1000);             // sleep 200 msec
    EXPECT_EQ(0, pfc_u64hash_put(hash, test_key2, test_val2, 0));
    pfc_hash_sync_end();            // end wsync block
    return NULL;
}

static void *
test_wr_sync_thr2 (void *arg)
{
    pfc_hash_t hash = (pfc_hash_t)arg;
    usleep(100 * 1000);             // sleep 100 msec
    pfc_hash_rsync_begin(hash);     // start rsync block
    pfc_cptr_t val;
    EXPECT_EQ(0, pfc_u64hash_get(hash, test_key2, &val));
    EXPECT_TRUE(test_val2 == val);
    pfc_hash_sync_end();            // end rsync block
    EXPECT_EQ(0, pfc_u64hash_update(hash, test_key2, test_val3, 0));
    return NULL;
}

TEST(hash, u64_wsync_and_rsync)
{
    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0));

    // start threads
    pfc_thread_t thr1, thr2;
    EXPECT_EQ(0, pfc_thread_create(&thr1, test_wr_sync_thr1, hash, 0));
    EXPECT_EQ(0, pfc_thread_create(&thr2, test_wr_sync_thr2, hash, 0));

    // wait for threads to terminate
    EXPECT_EQ(0, pfc_thread_join(thr1, NULL));
    EXPECT_EQ(0, pfc_thread_join(thr2, NULL));

    // clean up
    pfc_hash_destroy(hash);
}


static void *
test_rw_sync_thr1 (void *arg)
{
    pfc_hash_t hash = (pfc_hash_t)arg;
    EXPECT_EQ(0, pfc_u64hash_put(hash, test_key2, test_val2, 0));
    pfc_hash_rsync_begin(hash);     // start rsync block
    usleep(200 * 1000);             // sleep 200 msec
    pfc_cptr_t val;
    EXPECT_EQ(0, pfc_u64hash_get(hash, test_key2, &val));
    EXPECT_TRUE(test_val2 == val);
    pfc_hash_sync_end();            // end rsync block
    return NULL;
}

static void *
test_rw_sync_thr2 (void *arg)
{
    pfc_hash_t hash = (pfc_hash_t)arg;
    usleep(100 * 1000);             // sleep 100 msec
    pfc_hash_wsync_begin(hash);     // start wsync block
    EXPECT_EQ(0, pfc_u64hash_remove(hash, test_key2));
    pfc_hash_sync_end();            // end rsync block
    return 0;
}

TEST(hash, u64_rsync_and_wsync)
{
    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0));

    // start threads
    pfc_thread_t thr1, thr2;
    EXPECT_EQ(0, pfc_thread_create(&thr1, test_rw_sync_thr1, hash, 0));
    EXPECT_EQ(0, pfc_thread_create(&thr2, test_rw_sync_thr2, hash, 0));

    // wait for threads to terminate
    EXPECT_EQ(0, pfc_thread_join(thr1, NULL));
    EXPECT_EQ(0, pfc_thread_join(thr2, NULL));

    // clean up
    pfc_hash_destroy(hash);
}


// MEMO:
//
//   This test may depend on specific implementations, and
//   unfortunately Linux dosn't behave expectedly.
//
// static void *
// test_rwr_sync_thr1 (void *arg)
// {
//     pfc_hash_t hash = (pfc_hash_t)arg;
//     EXPECT_EQ(0, pfc_u64hash_put(hash, test_key2, test_val2, 0));
//     pfc_hash_rsync_begin(hash);     // start rsync block
//     usleep(3 * 1000 * 1000);        // sleep 3 sec
//     pfc_cptr_t val;
//     EXPECT_EQ(0, pfc_u64hash_get(hash, test_key2, &val));
//     EXPECT_TRUE(test_val2 == val);
//     pfc_hash_sync_end();            // end rsync block
//     return NULL;
// }
//
// static void *
// test_rwr_sync_thr2 (void *arg)
// {
//     pfc_hash_t hash = (pfc_hash_t)arg;
//     usleep(1000 * 1000);            // sleep 1 sec
//     pfc_hash_wsync_begin(hash);     // start wsync block
//     usleep(4 * 1000 * 1000);        // sleep 4 sec
//     EXPECT_EQ(0, pfc_u64hash_remove(hash, test_key2));
//     pfc_hash_sync_end();            // end wsync block
//     return 0;
// }
//
// static void *
// test_rwr_sync_thr3 (void *arg)
// {
//     pfc_hash_t hash = (pfc_hash_t)arg;
//     usleep(2 * 1000 * 1000);        // sleep 2 sec
//     pfc_hash_rsync_begin(hash);     // start rsync block
//     EXPECT_EQ(ENOENT, pfc_u64hash_get(hash, test_key2, NULL));
//     pfc_hash_sync_end();            // end rsync block
//     return 0;
// }
//
// TEST(hash, u64_rsync_wsync_and_rsync)
// {
//     // create hash
//     pfc_hash_t hash;
//     EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0));
//
//     // start threads
//     pfc_thread_t thr1, thr2, thr3;
//     EXPECT_EQ(0, pfc_thread_create(&thr1, test_rwr_sync_thr1, hash, 0));
//     EXPECT_EQ(0, pfc_thread_create(&thr2, test_rwr_sync_thr2, hash, 0));
//     EXPECT_EQ(0, pfc_thread_create(&thr3, test_rwr_sync_thr3, hash, 0));
//
//     // wait for threads to terminate
//     EXPECT_EQ(0, pfc_thread_join(thr1, NULL));
//     EXPECT_EQ(0, pfc_thread_join(thr2, NULL));
//     EXPECT_EQ(0, pfc_thread_join(thr3, NULL));
//
//     // clean up
//     pfc_hash_destroy(hash);
// }


TEST_HASH_RSYNC(get,
                pfc_cptr_t val;
                EXPECT_EQ(0, pfc_u64hash_get(hash, test_key, &val));
                EXPECT_TRUE(test_val == val);
                );

TEST_HASH_RSYNC(get_size,
                EXPECT_EQ(1U, pfc_hash_get_size(hash));
                );

TEST_HASH_RSYNC(get_capacity,
                EXPECT_EQ(nbuckets, pfc_hash_get_capacity(hash));
                );


static void *
test_multi_rsync_thr (void *arg)
{
    pfc_hash_t hash = (pfc_hash_t)arg;
    pfc_hash_rsync_begin(hash);     // start rsync block
    // usleep(100 * 1000 * 1000);     // sleep 100 msec
    usleep(10);                     // sleep 10 usec
    pfc_cptr_t val;
    EXPECT_EQ(0, pfc_u64hash_get(hash, test_key, &val));
    EXPECT_TRUE(test_val == val);
    pfc_hash_sync_end();            // end rsync block
    return NULL;
}

TEST(hash, u64_rsync_many_run)
{
    const int nthrs = 15;
    pfc_thread_t thr[nthrs];
    pfc_timespec_t tout = { 10, 0 }; // 10 sec

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0));

    // set data to get later
    EXPECT_EQ(0, pfc_u64hash_put(hash, test_key, test_val, 0));

    // start threads
    for (int i = 0; i < nthrs; ++i) {
        EXPECT_EQ(0, pfc_thread_create(&thr[i],
                                       test_multi_rsync_thr, hash, 0));
    }

    // wait for threads to terminate
    for (int i = 0; i < nthrs; ++i) {
#ifndef PARALLEL_TEST
        EXPECT_EQ(0, pfc_thread_timedjoin(thr[i], NULL, &tout));
#else  /* !PARALLEL_TEST */
      retry:
        int err = pfc_thread_timedjoin(thr[i], NULL, &tout);
        if (err != 0) {
            pfc_log_error("Retry to pfc_thread_timedjoin(thr[%d], NULL, &tout) (errno = %d)", i, err);
            goto retry;
        }
#endif /* !PARALLEL_TEST */
    }

    // clean up
    pfc_hash_destroy(hash);
}
