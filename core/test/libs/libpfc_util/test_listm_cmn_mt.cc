/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Multi-thread test routines for list model
 *
 * The routines are called by for each specific list model
 * implementation.
 */

#include <unistd.h>
#include <sys/time.h>
#include <pfc/listmodel.h>
#include <pfc/thread.h>
#include "pseudo_rand.hh"
#include "string_util.h"
#include "test_listm_cmn_mt.hh"


/*
 * Definitions of test data
 */

const size_t nents = 1000;

const useconds_t loop_usec = 100 * 1000;

const int side_nthrs = 15;

const size_t nents_before_loop = nents / 2;


typedef enum {
    LISTM_OP_GET_SIZE,
    LISTM_OP_FIRST,
    LISTM_OP_GETAT,
    LISTM_OP_PUSH,
    LISTM_OP_PUSH_TAIL,
    LISTM_OP_POP,
    LISTM_OP_REMOVE,
    LISTM_OP_CLEAR,
    LISTM_OP_SORT_COMP,
    LISTM_OP_NUM,
    LISTM_OP_NOLIMIT,
} test_listm_op_t;
#define listm_op_get_size  LISTM_OP_GET_SIZE
#define listm_op_first     LISTM_OP_FIRST
#define listm_op_getat     LISTM_OP_GETAT
#define listm_op_push      LISTM_OP_PUSH
#define listm_op_push_tail LISTM_OP_PUSH_TAIL
#define listm_op_pop       LISTM_OP_POP
#define listm_op_revemo    LISTM_OP_REVEMO
#define listm_op_clear     LISTM_OP_CLEAR
#define listm_op_sort_comp LISTM_OP_SORT_COMP
#define listm_op_nolimit   LISTM_OP_NOLIMIT


typedef struct {
    pfc_listm_t listm;
    listm_impl_t listm_impl;

    int        nents;
    pfc_ptr_t *vals;
    pfc_ptr_t  no_such_val;

    test_listm_op_t target_op;
    int             target_idx;
    pfc_cptr_t      target_val;
} test_data_t;

PFC_ATTR_UNUSED
static pfc_ptr_t INVALID_PTR = (pfc_ptr_t)-123;

static test_data_t * test_data_create (pfc_listm_t listm,
                                       listm_impl_t listm_impl,
                                       int nents,
                                       test_listm_op_t target_op);
static void test_data_destroy (test_data_t *test_data);
static void test_data_set_entries (test_data_t *test_data, int set_nents);
PFC_ATTR_UNUSED
static void test_data_del_all (test_data_t *test_data, int *ndel_p);



/*
 * Other utilities
 */
#define MAX(x, y)   (((x) > (y)) ? (x) : (y))

static useconds_t
cur_usec()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        int err = errno;
        fprintf(stderr, "errno = %d\n", err);
        abort();
    }
    return (useconds_t)tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}


/*
 * Thread functions
 */

static void *
thr_target (void *arg)
{
    test_data_t    *test_data  = (test_data_t *)arg;
    pfc_listm_t     listm      = test_data->listm;
    PFC_ATTR_UNUSED
            pfc_ptr_t      *vals       = test_data->vals;
    test_listm_op_t target_op  = test_data->target_op;
    int             target_idx = test_data->target_idx;
    pfc_cptr_t      target_val = test_data->target_val;

    useconds_t start_usec = cur_usec();
    int count = 0;
    while (cur_usec() - start_usec < loop_usec) {
        PFC_ATTR_UNUSED
                pfc_cptr_t v = NULL;
        switch (target_op) {
            case LISTM_OP_GET_SIZE: {
                size_t size = pfc_listm_get_size(listm);
                size_t min_size = nents_before_loop - side_nthrs;
                size_t max_size = nents_before_loop + side_nthrs;
                EXPECT_TRUE(min_size <= size && size <= max_size);
                break;
            }
            case LISTM_OP_FIRST: {
                pfc_cptr_t v = NULL;
                EXPECT_EQ(0, pfc_listm_first(listm, &v));
                EXPECT_TRUE(v != NULL);
                EXPECT_TRUE(v == target_val);
                break;
            }
            case LISTM_OP_GETAT: {
                pfc_cptr_t v = NULL;
                EXPECT_EQ(0, pfc_listm_getat(listm, target_idx, &v));
                EXPECT_TRUE(v != NULL);
                EXPECT_TRUE(v == target_val);
                break;
            }
            case LISTM_OP_POP: {
                if (count > target_idx) return NULL;
                pfc_cptr_t v = NULL;
                EXPECT_EQ(0, pfc_listm_pop(listm, &v));
                EXPECT_TRUE(v != NULL);
                EXPECT_TRUE(v == vals[count]);
                break;
            }
            case LISTM_OP_NOLIMIT: {
                // nothing to do
                break;
            }
            default:
                // unsupported operation
                abort();
        }
        ++count;
    }

    return NULL;
}


static void *
thr_side (void *arg)
{
    test_data_t    *test_data  = (test_data_t *)arg;
    pfc_listm_t     listm      = test_data->listm;
    listm_impl_t    listm_impl = test_data->listm_impl;
    int             nents      = test_data->nents;
    pfc_ptr_t      *vals       = test_data->vals;
    test_listm_op_t target_op  = test_data->target_op;
    int             target_idx = test_data->target_idx;
    pfc_cptr_t      target_val = test_data->target_val;

    const int ENT_DELTA_DEC  = -1;
    const int ENT_DELTA_ZERO =  0;
    const int ENT_DELTA_INC  =  1;
    int nents_delta = ENT_DELTA_ZERO;

    useconds_t start_usec = cur_usec();
    int count = 0;
    while (cur_usec() - start_usec < loop_usec) {
        pfc_cptr_t side_val = vals[count % nents];
        test_listm_op_t side_op = (test_listm_op_t)(count % LISTM_OP_NUM);

        // skip if value or operation is restricted by target_op
        switch (target_op) {
            case LISTM_OP_GET_SIZE: {
                if (nents_delta == ENT_DELTA_DEC) {
                    if (side_op == LISTM_OP_POP)    continue;
                    if (side_op == LISTM_OP_REMOVE) continue;
                }
                if (nents_delta == ENT_DELTA_INC) {
                    if (side_op == LISTM_OP_PUSH)      continue;
                    if (side_op == LISTM_OP_PUSH_TAIL) continue;
                }
                if (side_op == LISTM_OP_CLEAR) continue;
            }
            case LISTM_OP_FIRST: {
                if (side_val == target_val) {
                    if (side_op == LISTM_OP_REMOVE) continue;
                }
                if (nents_delta == ENT_DELTA_DEC) {
                    if (side_op == LISTM_OP_REMOVE) continue;
                }
                if (nents_delta == ENT_DELTA_INC) {
                    if (side_op == LISTM_OP_PUSH_TAIL) continue;
                }
                if (side_op == LISTM_OP_POP)       continue;
                if (side_op == LISTM_OP_PUSH)      continue;
                if (side_op == LISTM_OP_CLEAR)     continue;
                if (side_op == LISTM_OP_SORT_COMP) continue;
            }
            case LISTM_OP_GETAT: {
                if (side_op == LISTM_OP_REMOVE) {
                    pfc_bool_t matched = PFC_FALSE;
                    for (int j = 0; j <= target_idx; ++j) {
                        if (side_val == vals[j]) {
                            matched = PFC_TRUE;
                            break;
                        }
                    }
                    if (matched) continue;
                }
                if (nents_delta == ENT_DELTA_DEC) {
                    if (side_op == LISTM_OP_REMOVE) continue;
                }
                if (nents_delta == ENT_DELTA_INC) {
                    if (side_op == LISTM_OP_PUSH_TAIL) continue;
                }
                if (side_op == LISTM_OP_POP)       continue;
                if (side_op == LISTM_OP_PUSH)      continue;
                if (side_op == LISTM_OP_CLEAR)     continue;
                if (side_op == LISTM_OP_SORT_COMP) continue;

            }
            case LISTM_OP_POP: {
                continue;
                if (side_op == LISTM_OP_REMOVE) {
                    pfc_bool_t matched = PFC_FALSE;
                    for (int j = 0; j <= target_idx; ++j) {
                        if (side_val == vals[j]) {
                            matched = PFC_TRUE;
                            break;
                        }
                    }
                    if (matched) continue;
                }
                if (nents_delta == ENT_DELTA_DEC) {
                    if (side_op == LISTM_OP_REMOVE) continue;
                }
                if (nents_delta == ENT_DELTA_INC) {
                    if (side_op == LISTM_OP_PUSH_TAIL) continue;
                }
                if (side_op == LISTM_OP_POP)       continue;
                if (side_op == LISTM_OP_PUSH)      continue;
                if (side_op == LISTM_OP_CLEAR)     continue;
                if (side_op == LISTM_OP_SORT_COMP) continue;

            }
            case LISTM_OP_NOLIMIT: {
                if (side_op == LISTM_OP_CLEAR) {
                    // restrict CLEAR because it too strongly change list
                    static int clear_cnt = 0;
                    ++clear_cnt;
                    if (!PFC_IS_POW2(clear_cnt)) {
                        continue;
                    }
                }
                break;
            }
            default:
                // unsupported operation
                abort();
        }

        // do side action
        switch (side_op) {
            case LISTM_OP_GET_SIZE: {
                int sz = pfc_listm_get_size(listm);
                EXPECT_GE(sz, 0);
                break;
            }
            case LISTM_OP_FIRST: {
                pfc_cptr_t v = NULL;
                int err = pfc_listm_first(listm, &v);
                EXPECT_TRUE( (err == 0      && v != NULL) ||
                             (err == ENOENT && v == NULL) );
                break;
            }
            case LISTM_OP_GETAT: {
                int sz = pfc_listm_get_size(listm);
                pfc_cptr_t v = NULL;
                int err = pfc_listm_getat(listm, sz / 2, &v);
                EXPECT_TRUE( (err == 0      && v != NULL) ||
                             (err == ENOENT && v == NULL) );
                break;
            }
            case LISTM_OP_PUSH: {
                int err = pfc_listm_push(listm, side_val);
                if (listm_impl == LISTM_IMPL_HASHLIST) {
                    EXPECT_TRUE(err == 0 || err == EEXIST);
                } else {
                    EXPECT_TRUE(err == 0);
                }
                if (target_op == LISTM_OP_GET_SIZE) {
                    if (err == 0) {
                        PFC_ASSERT(nents_delta != ENT_DELTA_INC);
                        ++nents_delta;
                    }
                }
                break;
            }
            case LISTM_OP_PUSH_TAIL: {
                int err = pfc_listm_push_tail(listm, side_val);
                if (listm_impl == LISTM_IMPL_HASHLIST) {
                    EXPECT_TRUE(err == 0 || err == EEXIST);
                } else {
                    EXPECT_TRUE(err == 0);
                }
                if (target_op == LISTM_OP_GET_SIZE) {
                    if (err == 0) {
                        PFC_ASSERT(nents_delta != ENT_DELTA_INC);
                        ++nents_delta;
                    }
                }
                break;
            }
            case LISTM_OP_POP: {
                pfc_cptr_t v = NULL;
                int err = pfc_listm_pop(listm, &v);
                EXPECT_TRUE( (err == 0      && v != NULL) ||
                             (err == ENOENT && v == NULL) );
                if (target_op == LISTM_OP_GET_SIZE) {
                    if (err == 0) {
                        PFC_ASSERT(nents_delta != ENT_DELTA_DEC);
                        --nents_delta;
                    }
                }
                break;
            }
            case LISTM_OP_REMOVE: {
                int err = pfc_listm_remove(listm, side_val);
                EXPECT_TRUE( err == 0 || err == ENOENT );
                if (target_op == LISTM_OP_GET_SIZE) {
                    if (err == 0) {
                        PFC_ASSERT(nents_delta != ENT_DELTA_DEC);
                        --nents_delta;
                    }
                }
                break;
            }
            case LISTM_OP_CLEAR: {
                int sz = pfc_listm_clear(listm);
                EXPECT_GE(sz, 0);
                break;
            }
            case LISTM_OP_SORT_COMP: {
                if ((count % LISTM_OP_NUM % 2) == 0) {
                    pfc_listm_sort_comp(listm, NULL);
                } else {
                    pfc_listm_sort_comp(listm, string_comp);
                }
                break;
            }
            default:
                // unsupported operation
                abort();
        }
        ++count;
    }

    return NULL;
}



/*
 * Macro to define tests
 */

#define TEST_LISTM_MT_OP(OP_NAME, INIT_ACTION)                          \
    void                                                                \
    test_listm_ ## OP_NAME ## _with_threads (pfc_listm_t listm,         \
                                             listm_impl_t listm_impl)   \
    {                                                                   \
        pfc_thread_t target_thr;                                        \
        pfc_thread_t side_thr[side_nthrs];                              \
                                                                        \
        test_data_t *test_data = test_data_create(listm,                \
                                                  listm_impl,           \
                                                  nents,                \
                                                  listm_op_ ##OP_NAME); \
        test_data_set_entries(test_data, nents_before_loop);            \
                                                                        \
        do {                                                            \
            INIT_ACTION;                                                \
        } while(0);                                                     \
                                                                        \
        EXPECT_EQ(0, pfc_thread_create(&target_thr,                     \
                                       thr_target,                      \
                                       (void *)test_data,               \
                                       0));                             \
                                                                        \
        for (int i = 0; i < side_nthrs; i++) {                          \
            EXPECT_EQ(0, pfc_thread_create(&side_thr[i],                \
                                           thr_side,                    \
                                           (void *)test_data,           \
                                           0));                         \
        }                                                               \
                                                                        \
        EXPECT_EQ(0, pfc_thread_join(target_thr, NULL));                \
        for (int i = 0; i < side_nthrs; i++) {                          \
            EXPECT_EQ(0, pfc_thread_join(side_thr[i], NULL));           \
        }                                                               \
                                                                        \
        pfc_listm_clear(listm);                                         \
        test_data_destroy(test_data);                                   \
    }




/*
 * Test routines
 */

TEST_LISTM_MT_OP(get_size, /* no init action */ );
// --> void test_listm_get_size_with_threads(...) { ... }

TEST_LISTM_MT_OP(first,
                 EXPECT_EQ(0, pfc_listm_remove(test_data->listm,
                                               test_data->target_val));
                 EXPECT_EQ(0, pfc_listm_push(test_data->listm,
                                             test_data->target_val));
                 );
// --> void test_listm_first_with_threads(...) { ... }

TEST_LISTM_MT_OP(getat, /* no init action */ );
// --> void test_listm_getat_with_threads(...) { ... }

TEST_LISTM_MT_OP(pop, /* no init action */ );
// --> void test_listm_pop_with_threads(...) { ... }

TEST_LISTM_MT_OP(nolimit, /* no init action */ );
// --> void test_listm_nolimit_with_threads(...) { ... }




/*
 * Implementation of test data functions
 */

static test_data_t *
test_data_create (pfc_listm_t listm,
                  listm_impl_t listm_impl,
                  int nents,
                  test_listm_op_t target_op)
{
    PFC_ASSERT(nents > 0);
    prand_generator_t prand_gen = prand_create(LISTM_RANDOM_SEED);

    test_data_t *test_data = (test_data_t *)calloc(1, sizeof(test_data_t));
    PFC_ASSERT(test_data != NULL);

    test_data->listm = listm;
    test_data->listm_impl = listm_impl;
    test_data->nents = nents;

    test_data->no_such_val =
            (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~');
    PFC_ASSERT(test_data->no_such_val != NULL);

    test_data->vals = (pfc_ptr_t *)calloc(nents, sizeof(pfc_ptr_t));
    PFC_ASSERT(test_data->vals != NULL);

    for (int i = 0; i < nents; ++i) {
      retry:
        pfc_ptr_t val = (pfc_ptr_t)random_string_with_prand_gen(prand_gen,
                                                                100, '!', '~');
        PFC_ASSERT(val != NULL);

        if (string_equals(val, test_data->no_such_val)) {
            free(val);
            goto retry;
        }

        for (int j = 0; j < i; ++j) {
            if (string_equals(val, test_data->vals[j])) {
                free(val);
                goto retry;
            }
        }

        test_data->vals[i] = val;
    }

    test_data->target_op  = target_op;

    int target_idx = nents_before_loop / 3;
    test_data->target_idx = target_idx;
    test_data->target_val = (pfc_cptr_t)(test_data->vals[target_idx]);

    // clean up
    prand_destroy(prand_gen);

    return test_data;
}


static void
test_data_destroy (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    PFC_ASSERT(test_data->vals != NULL);

    pfc_ptr_t *vals = test_data->vals;
    for (int i = 0; i < test_data->nents; ++i) {
        if (vals[i] != NULL) {
            free(vals[i]);
        }
    }
    free(vals);

    free(test_data->no_such_val);
    free(test_data);
}


static void
test_data_set_entries (test_data_t *test_data, int set_nents)
{
    PFC_ASSERT(test_data != NULL);
    pfc_listm_t listm = test_data->listm;
    pfc_ptr_t *vals = test_data->vals;
#ifdef PFC_DEBUG_VERBOSE
    int nents = test_data->nents;
    PFC_ASSERT(nents >= set_nents);
#endif /* PFC_DEBUG_VERBOSE */

    for (int i = 0; i < set_nents; ++i) {
        EXPECT_EQ(0, pfc_listm_push_tail(listm, vals[i]));
    }
}


static void
test_data_del_all (test_data_t *test_data, int *ndel_p)
{
    PFC_ASSERT(test_data != NULL);
    pfc_listm_t listm = test_data->listm;

    int size = pfc_listm_get_size(listm);
    int ndel = 0;
    for (int i = 0; i < size; ++i) {
        switch (i % 2) {
            case 0:     // pop
                EXPECT_EQ(0, pfc_listm_pop(listm, NULL));
                break;
            case 1: {   // remove
                int idx = -1;
                switch ((i / 2) % 3) {
                    case 0: idx = 0;                 break; // head
                    case 1: idx = (size - ndel) / 2; break; // middle
                    case 2: idx = size - ndel - 1;   break; // tail
                    default: abort();
                }
                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(0, pfc_listm_getat(listm, idx, &v));
                // If duplicated value exists at before `idx',
                // pfc_listm_remove() will delete duplicated one.
                EXPECT_EQ(0, pfc_listm_remove(listm, v));
                break;
            }
            default:
                abort();
        }
        ++ndel;
    }
    EXPECT_EQ(0, pfc_listm_get_size(listm));

    if (ndel_p != NULL) {
        *ndel_p = ndel;
    }
}
