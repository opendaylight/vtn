/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for hash basic operations by multi-threads
 */

#include <sys/time.h>
#include <pfc/hash.h>
#include <pfc/thread.h>
#include "test_hash.hh"


/*
 * Types, prototypes and constants for test data
 */

const size_t nbuckets = 31;
const size_t nentries = 1000;
const size_t val_nsets = 5;

const useconds_t loop_usec = 100 * 1000;

const int side_nthrs = 15;

const size_t nents_before_loop = nentries / 2;


typedef enum {
    HASH_OP_GET,
    HASH_OP_GET_SIZE,
    HASH_OP_GET_CAPACITY,
    HASH_OP_PUT,
    HASH_OP_UPDATE,
    HASH_OP_REMOVE,
    HASH_OP_CLEAR,
    HASH_OP_SET_CAPACITY,
    HASH_OP_NUM,
    HASH_OP_NOLIMIT,
} test_hash_op_t;
#define hash_op_get          HASH_OP_GET
#define hash_op_get_size     HASH_OP_GET_SIZE
#define hash_op_get_capacity HASH_OP_GET_CAPACITY
#define hash_op_put          HASH_OP_PUT
#define hash_op_update       HASH_OP_UPDATE
#define hash_op_remove       HASH_OP_REMOVE
#define hash_op_clear        HASH_OP_CLEAR
#define hash_op_set_capacity HASH_OP_SET_CAPACITY
#define hash_op_nolimit      HASH_OP_NOLIMIT

typedef struct {
    int nbuckets;
    int nentries;
    int val_nsets;

    pfc_hash_t hash;

    pfc_cptr_t *key;
    pfc_cptr_t **val;

    test_hash_op_t target_op;
    pfc_cptr_t target_key;
    pfc_cptr_t target_val;
} test_data_t;

static test_data_t *test_data_alloc (int nbuckets,
                                     int nentries,
                                     int val_nsets,
                                     test_hash_op_t target_op);
static void test_data_hash_create (test_data_t *test_data);
static void test_data_set_entries (test_data_t *test_data, size_t size);
static void test_data_hash_destroy (test_data_t *test_data);
static void test_data_free (test_data_t *test_data);


/*
 * Other utility functions
 */

static useconds_t
cur_usec()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) abort();
    return (useconds_t)tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}



/*
 * Thread functions
 */

static void *
thr_target (void *arg)
{
    test_data_t *test_data = (test_data_t *)arg;
    pfc_hash_t     hash       = test_data->hash;
    test_hash_op_t target_op  = test_data->target_op;
    pfc_cptr_t     target_key = test_data->target_key;
    pfc_cptr_t     target_val = test_data->target_val;

    useconds_t start_usec = cur_usec();

    while (cur_usec() - start_usec < loop_usec) {
        pfc_cptr_t v = NULL;
        switch (target_op) {
            case HASH_OP_GET: {
                EXPECT_EQ(0, pfc_hash_get(hash, target_key, &v, 0));
                EXPECT_TRUE(v == target_val);
                break;
            }
            case HASH_OP_GET_SIZE: {
                size_t size = pfc_hash_get_size(hash);
                size_t min_size = nents_before_loop - side_nthrs;
                size_t max_size = nents_before_loop + side_nthrs;
                EXPECT_TRUE(min_size <= size && size <= max_size);
                break;
            }
            case HASH_OP_GET_CAPACITY: {
                size_t capa = pfc_hash_get_capacity(hash);
                EXPECT_EQ(capa, nbuckets);
                break;
            }
            case HASH_OP_PUT: {
                pfc_cptr_t v = NULL;
                EXPECT_EQ(0, pfc_hash_put(hash, target_key, target_val, 0));
                EXPECT_EQ(0, pfc_hash_get(hash, target_key, &v, 0));
                EXPECT_TRUE(target_val == v);
                EXPECT_EQ(0, pfc_hash_remove(hash, target_key, 0));
                break;
            }
            case HASH_OP_UPDATE: {
                pfc_cptr_t v = NULL;
                EXPECT_EQ(0, pfc_hash_update(hash, target_key, target_val, 0));
                EXPECT_EQ(0, pfc_hash_get(hash, target_key, &v, 0));
                EXPECT_TRUE(target_val == v);
                break;
            }
            case HASH_OP_REMOVE: {
                pfc_cptr_t v = NULL;
                EXPECT_EQ(0, pfc_hash_update(hash, target_key, &v, 0));
                EXPECT_EQ(0, pfc_hash_remove(hash, target_key, 0));
                EXPECT_EQ(ENOENT, pfc_hash_get(hash, target_key, NULL, 0));
                break;
            }
            case HASH_OP_NOLIMIT: {
                // nothing to do
                return NULL;
            }
            default:
                // unsupported operation
                abort();
        }
    }

    return NULL;
}


static void *
thr_side (void *arg)
{
    test_data_t *test_data = (test_data_t *)arg;
    int            nentries   = test_data->nentries;
    int            val_nsets  = test_data->val_nsets;
    pfc_hash_t     hash       = test_data->hash;
    pfc_cptr_t    *key        = test_data->key;
    pfc_cptr_t   **val        = test_data->val;
    test_hash_op_t target_op  = test_data->target_op;
    pfc_cptr_t     target_key = test_data->target_key;
    // pfc_cptr_t     target_val = test_data->target_val;

    const int ENT_DELTA_DEC  = -1;
    const int ENT_DELTA_ZERO =  0;
    const int ENT_DELTA_INC  =  1;
    int nents_delta = ENT_DELTA_ZERO;

    useconds_t start_usec = cur_usec();
    unsigned int count = 0;
    while (cur_usec() - start_usec < loop_usec) {
        ++count;
        pfc_cptr_t side_key = key[count % nentries];
        pfc_cptr_t side_val =
                val[(count / nentries) % val_nsets][count % nentries];
        test_hash_op_t side_op = (test_hash_op_t)(count % HASH_OP_NUM);

        // skip if key, value or operation is restricted by target_op
        switch (target_op) {
            case HASH_OP_GET: {
                if (side_key == target_key) continue;
                if (side_op  == HASH_OP_CLEAR) continue;
                break;
            }
            case HASH_OP_GET_SIZE: {
                if (side_op  == HASH_OP_UPDATE) continue;
                if (side_op  == HASH_OP_CLEAR)  continue;
                if (nents_delta == ENT_DELTA_DEC)
                    if (side_op == HASH_OP_REMOVE) continue;
                if (nents_delta == ENT_DELTA_INC)
                    if (side_op == HASH_OP_PUT) continue;
                break;
            }
            case HASH_OP_GET_CAPACITY: {
                if (side_op == HASH_OP_SET_CAPACITY)  continue;
                break;
            }
            case HASH_OP_PUT: {
                if (side_key == target_key) continue;
                if (side_op  == HASH_OP_CLEAR) continue;
                break;
            }
            case HASH_OP_UPDATE: {
                if (side_key == target_key) continue;
                if (side_op  == HASH_OP_CLEAR) continue;
                break;
            }
            case HASH_OP_REMOVE: {
                if (side_key == target_key) continue;
                if (side_op  == HASH_OP_CLEAR) continue;
                break;
            }
            case HASH_OP_NOLIMIT: {
                if (side_op == HASH_OP_CLEAR) {
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
            case HASH_OP_GET: {
                pfc_cptr_t v = NULL;
                int err = pfc_hash_get(hash, side_key, &v, 0);
                EXPECT_TRUE( (err == 0      && v != NULL) ||
                             (err == ENOENT && v == NULL) );
                break;
            }
            case HASH_OP_GET_SIZE: {
                size_t size = pfc_hash_get_size(hash);
                // EXPECT_TRUE(0 <= size && size <= nentries);
                EXPECT_TRUE(size <= (size_t)nentries);
                break;
            }
            case HASH_OP_GET_CAPACITY: {
                size_t capa = pfc_hash_get_capacity(hash);
                EXPECT_TRUE(1 <= capa && capa <= PFC_HASH_MAX_NBUCKETS);
                break;
            }
            case HASH_OP_PUT: {
                int err = pfc_hash_put(hash, side_key, side_val, 0);
                EXPECT_TRUE( err == 0 || err == EEXIST );
                if (target_op == HASH_OP_GET_SIZE) {
                    if (err == 0) {
                        PFC_ASSERT(nents_delta != ENT_DELTA_INC);
                        ++nents_delta;
                    }
                }
                break;
            }
            case HASH_OP_UPDATE: {
                int err = pfc_hash_update(hash, side_key, side_val, 0);
                EXPECT_TRUE( err == 0 );
                // update operation is not target to record `nents_delta'
                break;
            }
            case HASH_OP_REMOVE: {
                int err = pfc_hash_remove(hash, side_key, 0);
                EXPECT_TRUE( err == 0 || err == ENOENT );
                if (target_op == HASH_OP_GET_SIZE) {
                    if (err == 0) {
                        PFC_ASSERT(nents_delta != ENT_DELTA_DEC);
                        --nents_delta;
                    }
                }
                break;
            }
            case HASH_OP_CLEAR: {
                size_t size = pfc_hash_clear(hash);
                // EXPECT_TRUE(0 <= size && size <= nentries);
                EXPECT_TRUE(size <= (size_t)nentries);
                // clear operation is not target to record `nents_delta'
                break;
            }
            case HASH_OP_SET_CAPACITY: {
                int new_nbuckets = nbuckets + count % nbuckets;
                int err = pfc_hash_set_capacity(hash, new_nbuckets);
                EXPECT_TRUE( err == 0 );
                break;
            }
            default:
                // unsupported operation
                abort();
        }
    }

    return NULL;
}



/*
 * Macro to define tests
 */

#define TEST_HASH_MT_OP(OP_NAME, INIT_ACTION)                           \
    TEST(hash, gen_mt_ ## OP_NAME)                                      \
    {                                                                   \
        pfc_thread_t target_thr;                                        \
        pfc_thread_t side_thr[side_nthrs];                              \
                                                                        \
        test_data_t *test_data = test_data_alloc(nbuckets,              \
                                                 nentries,              \
                                                 val_nsets,             \
                                                 hash_op_ ## OP_NAME);  \
        test_data_hash_create(test_data);                               \
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
        test_data_hash_destroy(test_data);                              \
        test_data_free(test_data);                                      \
    }




/*
 * Test cases
 */

TEST_HASH_MT_OP(get,
                EXPECT_EQ(0, pfc_hash_update(test_data->hash,
                                             test_data->target_key,
                                             test_data->target_val,
                                             0))
                );

TEST_HASH_MT_OP(get_size, /* no init action */ );

TEST_HASH_MT_OP(get_capacity, /* no init action */ );

TEST_HASH_MT_OP(put,
                pfc_hash_remove(test_data->hash,
                                test_data->target_key,
                                0)
                );

TEST_HASH_MT_OP(update, /* no init action */ );

TEST_HASH_MT_OP(remove, /* no init action */ );

TEST_HASH_MT_OP(nolimit, /* no init action */ );




/*
 * Operations on test data
 */

static test_data_t *
test_data_alloc (int nbuckets,
                 int nentries,
                 int val_nsets,
                 test_hash_op_t target_op)
{
    PFC_ASSERT(val_nsets >= 0);
    PFC_ASSERT(nentries > 0);
    test_data_t *test_data = (test_data_t *)calloc(1, sizeof(test_data_t));

    //
    // allocate a set of keys
    //
    void **key = (void **)calloc(nentries, sizeof(void *));
    PFC_ASSERT(key != NULL);

    for (int i = 0; i < nentries; ++i) {
        int *int_p = (int *)calloc(1, sizeof(int));
        PFC_ASSERT(int_p != NULL);
        *int_p = i;
        key[i] = int_p;
    }

    //
    // allocate sets of values
    //
    void ***val = NULL;

    if (val_nsets > 0) {
        val = (void ***)calloc(val_nsets, sizeof(void **));
        PFC_ASSERT(val != NULL);

        for (int n = 0; n < val_nsets; ++n) {
            val[n] = (void **)calloc(nentries, sizeof(void *));

            for (int i = 0; i < nentries; ++i) {
                const int maxlen = 100;
                val[n][i] = (char *)calloc(maxlen, sizeof(char));
                PFC_ASSERT(val[n][i] != NULL);
                snprintf((char *)(val[n][i]), maxlen,
                         "value/%d with key = %d (at %p)",
                         n, i, key[i]);
            }
        }
    }

    // store members
    test_data->nbuckets  = nbuckets;
    test_data->nentries  = nentries;
    test_data->val_nsets = val_nsets;

    test_data->key = (pfc_cptr_t *)key;
    test_data->val = (pfc_cptr_t **)val;

    test_data->target_op  = target_op;
    test_data->target_key = test_data->key[nentries / 2];
    test_data->target_val = test_data->val[val_nsets /2][nentries / 2];

    return test_data;
}


static void
test_data_hash_create (test_data_t *test_data)
{
    EXPECT_EQ(0, pfc_hash_create(&(test_data->hash),
                                 NULL,
                                 test_data->nbuckets,
                                 0));
}


static void
test_data_set_entries (test_data_t *test_data, size_t size)
{
    PFC_ASSERT(test_data != NULL);
    int nentries  = test_data->nentries;
    int val_nsets = test_data->val_nsets;
    if ((size_t)nentries < size) abort();
    if ((size_t)val_nsets == 0) abort();

    pfc_hash_t hash  = test_data->hash;
    pfc_cptr_t *key  = test_data->key;
    pfc_cptr_t **val = test_data->val;
    for (size_t i = 0; i < size; ++i) {
        int err = pfc_hash_put(hash, key[i], val[0][i], 0);
        if (err != 0) abort();
    }
}


static void
test_data_hash_destroy (test_data_t *test_data)
{
    pfc_hash_destroy(test_data->hash);
}


static void
test_data_free (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    int nentries  = test_data->nentries;
    int val_nsets = test_data->val_nsets;

    pfc_cptr_t *key  = test_data->key;
    pfc_cptr_t **val = test_data->val;

    // release keys
    for (int i = 0; i < nentries; ++i) {
        free((void *)key[i]);
    }
    free(key);

    // release values
    for (int n = 0; n < val_nsets; n++) {
        for (int i = 0; i < nentries; ++i) {
            free((void *)val[n][i]);
        }
        free(val[n]);
    }
    free(val);

    // release top
    free(test_data);
}
