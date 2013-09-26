/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * multi-thread tests for linked list
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include <pfc/listmodel.h>
#include <pfc/thread.h>
#include <pfc/synch.h>

#include "pseudo_rand.hh"
#include "string_util.h"
#include "test_listm_cmn_mt.hh"


/*
 * Definitions for test utility functions
 */

#define NSEC_IN_A_SEC    (1000000000U)
#define NSEC_IN_A_MSEC   (1000000U)
#define USEC_IN_A_MSEC   (1000U)
typedef struct timespec timespec_t;
static timespec_t timespec_get_mono ();
static uint64_t timespec_nsec_sub (const timespec_t &ts1,
                                   const timespec_t &ts2);
#define timespec2nsec(ts)                                               \
    ((uint64_t)((ts).tv_nsec) + (uint64_t)((ts).tv_sec) * NSEC_IN_A_SEC)

#define INVALID_PTR     ((void *)-123)


/*
 * Test cases
 */

TEST(listm, llist_pop_wait_for_filled_list)
{
    // test data
    prand_generator_t prand_gen = prand_create(LISTM_RANDOM_SEED);
    pfc_ptr_t vals[] = {
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        NULL,
    };

    // create linked list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    for (int k = 0; k < 2; ++k) {
        for (int i = 0; vals[i] != NULL; ++i) {
            // fill list with only `i' elements
            pfc_listm_clear(list);
            for (int j = 0; j <= i; ++j) {
                EXPECT_EQ(0, pfc_listm_push(list, vals[j]));
            }

            // do pop_wait
            pfc_cptr_t v = INVALID_PTR;
            timespec_t ts1 = timespec_get_mono();
            if (k == 0) {
                EXPECT_EQ(0, pfc_llist_pop_wait(list, &v));
            } else {
                EXPECT_EQ(0, pfc_llist_pop_wait(list, NULL));
            }
            timespec_t ts2 = timespec_get_mono();

            // check value
            if (k == 0) {
                EXPECT_EQ(vals[i], v);
            }

            // cehck list size
            EXPECT_EQ(i, pfc_listm_get_size(list));

            if (!is_under_heavy_load()) {
                // check elapsed time
                EXPECT_LT(timespec_nsec_sub(ts1, ts2), NSEC_IN_A_MSEC);
            }
        }
    }

    // clean up
    pfc_listm_destroy(list);
    for (pfc_ptr_t *p = vals; *p != NULL; ++p) {
        free(*p);
    }
    prand_destroy(prand_gen);
}


#define ITVL_MSEC_FOR_IDX(i)    (100)
#define ITVL_MSEC_MARGIN        (100)

typedef struct {
    pfc_listm_t list;
    pfc_ptr_t *vals;
    pfc_sem_t sem;
    int max_idx;
} args_for_sleep_and_push_t;

void *
sleep_and_push (void *args) {
    args_for_sleep_and_push_t *my_args_p = (args_for_sleep_and_push_t *)args;
    pfc_listm_t list = my_args_p->list;
    pfc_ptr_t *vals  = my_args_p->vals;
    int max_idx      = my_args_p->max_idx;

    for (int i = 0; i <= max_idx; ++i) {
        useconds_t wait_usec = ITVL_MSEC_FOR_IDX(i) * 50;
        if (is_under_heavy_load()) {
            wait_usec *= 10;
        }
        EXPECT_EQ(0, usleep(wait_usec));
        EXPECT_EQ(0, pfc_sem_wait(&my_args_p->sem));
        EXPECT_EQ(0, pfc_listm_push(list, vals[i]));
    }

    return NULL;
}

TEST(listm, llist_pop_wait_for_empty_list)
{
    // test data
    prand_generator_t prand_gen = prand_create(LISTM_RANDOM_SEED);
    pfc_ptr_t vals[] = {
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        NULL,
    };

    // create linked list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    for (int k = 0; k < 1; ++k) {
        for (int i = 0; vals[i] != NULL; ++i) {
            // start `sleep_and_push' thread
            pfc_thread_t thr_id;
            args_for_sleep_and_push_t args;
            pfc_sem_t *sem(&args.sem);

            args.list = list;
            args.vals = vals;
            args.max_idx = i;
            int err(pfc_sem_init(sem, 0));
            if (err != 0) {
                EXPECT_EQ(0, err);
                break;
            }
            EXPECT_EQ(0, pfc_thread_create(&thr_id, sleep_and_push,
                                           (pfc_ptr_t)&args, 0));

            pfc_bool_t skip_val_eq_check = PFC_FALSE;
            
            for (int j = 0; j <= i; ++j) {
                EXPECT_EQ(0, pfc_sem_post(sem));

                // do pop_wait
                pfc_cptr_t v = INVALID_PTR;
                timespec_t ts1 = timespec_get_mono();
                if (k == 0) {
                    EXPECT_EQ(0, pfc_llist_pop_wait(list, &v));
                } else {
                    EXPECT_EQ(0, pfc_llist_pop_wait(list, NULL));
                }
                timespec_t ts2 = timespec_get_mono();

                // cehck list size
                if (is_under_heavy_load()) {
                    if (!skip_val_eq_check && pfc_listm_get_size(list) > 0) {
                        pfc_log_notice("Multiple entires are puts. "
                                       "Skip over later equivalence checks.");
                        skip_val_eq_check = PFC_TRUE;
                    }
                } else {
                    EXPECT_EQ(0, pfc_listm_get_size(list));
                }

                // check value
                if ((k == 0) && (!skip_val_eq_check)) {
                    if (vals[j] != v) {
                        pfc_log_error("k = %d, i = %d, j = %d", k, i, j);
                        pfc_log_error("lhs = vals[%d] = %p", j, vals[j]);
                        pfc_log_error("rhs = v = %p", v);
                        for (int i = 0; vals[i] != NULL; ++i) {
                            pfc_log_error("vals[%d] = %p", i, vals[i]);
                        }
                    }
                    EXPECT_EQ(vals[j], v);
                }

                if (!is_under_heavy_load()) {
                    // check elapsed time
                    uint64_t elapsed_nsec = timespec_nsec_sub(ts1, ts2);
                    uint64_t itvl_msec = ITVL_MSEC_FOR_IDX(j);
                    uint64_t max_msec = itvl_msec + ITVL_MSEC_MARGIN;
                    uint64_t min_msec = itvl_msec - ITVL_MSEC_MARGIN;
                    EXPECT_LT(elapsed_nsec, max_msec * NSEC_IN_A_MSEC);
                    if (itvl_msec > ITVL_MSEC_MARGIN) {
                        EXPECT_GT(elapsed_nsec, min_msec * NSEC_IN_A_MSEC);
                    }
                }
            }

            // join with thread
            EXPECT_EQ(0, pfc_thread_join(thr_id, NULL));
        }
    }

    // clean up
    pfc_listm_destroy(list);
    for (pfc_ptr_t *p = vals; *p != NULL; ++p) {
        free(*p);
    }
    prand_destroy(prand_gen);
}


TEST(listm, llist_shutdown)
{
    // test data
    prand_generator_t prand_gen = prand_create(LISTM_RANDOM_SEED);
    pfc_ptr_t vals[] = {
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~'),
        NULL,
    };


    for (int k = 0; k < 2; ++k) {
        for (int i = 0; vals[i] != NULL; ++i) {
            pfc_bool_t clear_mode = (k == 0);
            int nelems = i;

            // create linked list
            pfc_listm_t list;
            EXPECT_EQ(0, pfc_llist_create(&list));

            // fill list with only `nelems' elements
            pfc_listm_clear(list);
            for (int j = 0; j < nelems; ++j) {
                EXPECT_EQ(0, pfc_listm_push(list, vals[j]));
            }

            // do shutdown
            pfc_llist_shutdown(list, clear_mode);

            // try to push and push_tail
            EXPECT_EQ(ESHUTDOWN, pfc_listm_push(list, vals[0]));
            EXPECT_EQ(ESHUTDOWN, pfc_listm_push_tail(list, vals[0]));

            if (clear_mode) {
                // check for clear mode
                EXPECT_EQ(0, pfc_listm_get_size(list));

                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(ESHUTDOWN, pfc_llist_pop_wait(list, &v));
            } else {
                // check for non-clear mode
                EXPECT_EQ(nelems, pfc_listm_get_size(list));

                for (int j = nelems - 1; j >= 0; --j) {
                    pfc_cptr_t v = INVALID_PTR;
                    EXPECT_EQ(0, pfc_llist_pop_wait(list, &v));
                    EXPECT_EQ(vals[j], v);
                }

                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(ESHUTDOWN, pfc_llist_pop_wait(list, &v));
            }

            // clean up
            pfc_listm_destroy(list);
        }
    }

    // clean up
    for (pfc_ptr_t *p = vals; *p != NULL; ++p) {
        free(*p);
    }
    prand_destroy(prand_gen);
}


TEST(listm, llist_get_size_with_threads)
{
    // create linked list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_get_size_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_first_with_threads)
{
    // create linked list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_first_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_getat_with_threads)
{
    // create linked list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_getat_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_pop_with_threads)
{
    // create linked list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_pop_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_nolimit_with_threads)
{
    // create linked list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_nolimit_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}



/*
 * Body of test utility functions
 */

static timespec_t
timespec_get_mono ()
{
    timespec_t ts;
    int err = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (err != 0) abort();
    return ts;
}


static uint64_t
timespec_nsec_sub (const timespec_t &ts1,
                   const timespec_t &ts2)
{
    uint64_t nsec1 = timespec2nsec(ts1);
    uint64_t nsec2 = timespec2nsec(ts2);
    return nsec2 - nsec1;
}

