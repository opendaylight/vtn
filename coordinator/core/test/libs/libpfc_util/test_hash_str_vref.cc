/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for strhash using refptr value
 */

#include <pfc/hash.h>
#include "pseudo_rand.hh"
#include "string_util.h"
#include "strint_refptr.h"
#include "test_hash.hh"


/*
 * Types, prototypes and macros for test data
 */

typedef enum {
    REFPTR_NONE,        // not a refptr
    REFPTR_DEFAULT,     // defualt refptr
    REFPTR_STRINT,      // string refptr
} test_refptr_mode_t;

typedef enum {
    INPUT_OP_PUT,
    INPUT_OP_UPDATE,
} test_input_op_t;

typedef struct {
    // Hash table size
    int nbuckets;

    // Size of test data set
    int nentries;
    int val_nsets;

    // refptr type
    test_refptr_mode_t val_refptr_mode;

    // refptr ops and flags  (set automatically by test_data_alloc())
    const pfc_refptr_ops_t *v_ops;
    uint32_t v_opflag;

    // Keys and values  (set automatically by test_data_alloc())
    const char **key;
    pfc_ptr_t **val;

    pfc_ptr_t *head_val;
    pfc_ptr_t *tail_val;

    // Status keys and values
    //   After initialize by test_data_alloc(), they should
    //   be maintained by TEST() cases.
    uint32_t **v_refcnt;

    int **v_registered;

    // flags to check TP itself
    struct {
        bool put_unreg_key;
        bool put_existing_key;

        bool update_unreg_key;
        bool update_existing_key;
    } pass_flags;
} test_data_t;

#define SET_FLAG(test_data, flag)               \
    ((test_data)->pass_flags.flag = true)

#define GET_FLAG(test_data, flag)               \
    ((test_data)->pass_flags.flag)

#define VAL_IS_REFPTR(test_data)                            \
    ( ((test_data)->val_refptr_mode == REFPTR_DEFAULT) ||   \
      ((test_data)->val_refptr_mode == REFPTR_STRINT) )

#define VAL_IS_STRINT(test_data)                    \
    ((test_data)->val_refptr_mode == REFPTR_STRINT)

#define INVALID_REFCNT  (-9999)

#define VAL_STATUS_REFCNT(test_data, n, i)              \
    ((VAL_IS_REFPTR(test_data)) ?                       \
     (test_data)->v_refcnt[(n)][(i)] : INVALID_REFCNT)

#define VAL_STATUS_REFCNT_CHANGE(test_data, n, i, delta)    \
    do {                                                    \
        if (VAL_IS_REFPTR(test_data)) {                     \
            (test_data)->v_refcnt[(n)][(i)] += delta;       \
        }                                                   \
    } while(0)

#define VAL_STATUS_REFCNT_UP(test_data, n, i)       \
    VAL_STATUS_REFCNT_CHANGE(test_data, n, i, 1)

#define VAL_STATUS_REFCNT_DOWN(test_data, n, i)     \
    VAL_STATUS_REFCNT_CHANGE(test_data, n, i, -1)

#define VAL_STATUS_HASHED(test_data, n, i)      \
    ((test_data)->v_registered[(n)][(i)])

#define VAL_STATUS_HASH(test_data, n, i, x)         \
    do {                                            \
        (test_data)->v_registered[(n)][(i)] = (x);  \
    } while(0)

#define VAL_STATUS_HASH_IN(test_data, n, i)     \
    VAL_STATUS_HASH(test_data, n, i, 1)

#define VAL_STATUS_HASH_OUT(test_data, n, i)    \
    VAL_STATUS_HASH(test_data, n, i, 0)

static test_data_t *test_data_alloc(int nbuckets,
                                    int nentries,
                                    int val_nsets,
                                    test_refptr_mode_t val_refptr_mode);
static void test_data_create_hash (test_data_t *test_data, pfc_hash_t *hashp);
static void test_data_clear_registered_status(test_data_t *test_data);
static void test_data_free(test_data_t *test_data);


/*
 * common test functions
 */

static void
test_put_ents (test_data_t *test_data, test_input_op_t input_op)
{
    PFC_ASSERT(test_data != NULL);
    int nentries  = test_data->nentries;
    int val_nsets = test_data->val_nsets;
    int opflags   = test_data->v_opflag;

    // create ref-hash
    pfc_hash_t hash;
    test_data_create_hash(test_data, &hash);

    // put / update for each value
    for (int i = 0; i < nentries; ++i) {
        for (int n = 0; n < val_nsets; ++n) {
            const char *key = test_data->key[i];
            pfc_cptr_t val = test_data->val[n][i];

            // get status before action
            pfc_refptr_t *val_refptr = NULL;
            uint32_t old_val_refcnt = -1;
            if (VAL_IS_REFPTR(test_data)) {
                val_refptr = (pfc_refptr_t *)val;
                old_val_refcnt = val_refptr->pr_refcnt;
            }

            // do put / update
            int err;
            switch (input_op) {
                case INPUT_OP_PUT:
                    err = pfc_strhash_put(hash, key, val, opflags);
                    break;
                case INPUT_OP_UPDATE:
                    err = pfc_strhash_update(hash, key, val, opflags);
                    break;
                default:
                    abort();
            }

            // check after action
            if (n == 0) {
                // first put / update operation

                // error of operation
                EXPECT_EQ(0, err);
                VAL_STATUS_HASH_IN(test_data, n, i);

                // ref-counter of value
                if (VAL_IS_REFPTR(test_data)) {
                    EXPECT_EQ(old_val_refcnt + 1, val_refptr->pr_refcnt);
                    VAL_STATUS_REFCNT_UP(test_data, n, i);
                }
            } else {
                // second or later put / update operation

                // error of operation
                if (input_op == INPUT_OP_PUT) {
                    // check for put
                    EXPECT_EQ(EEXIST, err);
                } else {
                    // check for update
                    EXPECT_EQ(0, err);
                    PFC_ASSERT(!VAL_STATUS_HASHED(test_data, n, i));
                    PFC_ASSERT(VAL_STATUS_HASHED(test_data, n - 1, i));
                    VAL_STATUS_HASH_IN(test_data, n, i);
                    VAL_STATUS_HASH_OUT(test_data, n - 1, i);
                }

                // ref-counter of value
                if (VAL_IS_REFPTR(test_data)) {
                    if (input_op == INPUT_OP_PUT) {
                        // check for after put
                        EXPECT_EQ(old_val_refcnt, val_refptr->pr_refcnt);
                    } else {
                        // check for after update
                        EXPECT_EQ(old_val_refcnt + 1, val_refptr->pr_refcnt);
                        VAL_STATUS_REFCNT_UP(test_data, n, i);
                        VAL_STATUS_REFCNT_DOWN(test_data, n - 1, i);
                    }
                }
            }

            // set passed route flags
            if (n == 0) {
                switch (input_op) {
                    case INPUT_OP_PUT:
                        SET_FLAG(test_data, put_unreg_key); break;
                    case INPUT_OP_UPDATE:
                        SET_FLAG(test_data, update_unreg_key); break;
                    default:
                        abort();
                }
            } else {
                switch (input_op) {
                    case INPUT_OP_PUT:
                        SET_FLAG(test_data, put_existing_key); break;
                    case INPUT_OP_UPDATE:
                        SET_FLAG(test_data, update_existing_key); break;
                    default:
                        abort();
                }
            }
        }
    }

    // clean up ref-hash
    pfc_hash_destroy(hash);
    test_data_clear_registered_status(test_data);
}


static void
test_get_remove (test_refptr_mode_t val_refptr_mode)
{
    // init test data
    int nbuckets = 31;
    int nentries = nbuckets / 2;
    int val_nsets = 2;
    test_data_t *test_data = test_data_alloc(nbuckets,
                                             nentries,
                                             val_nsets,
                                             val_refptr_mode);
    const char **key = test_data->key;
    pfc_ptr_t **val = test_data->val;

    // create ref-hash
    pfc_hash_t hash;
    test_data_create_hash(test_data, &hash);

    // do test
    int v_opflag = test_data->v_opflag;
    for (int n = 0; n < val_nsets; ++n) {
        // set values (and failour tests of pfc_strhash_get / pfc_hash__remove)
        for (int i = 0; i < nentries; ++i) {
            // failure tests before set
            EXPECT_EQ(ENOENT, pfc_strhash_get(hash, key[i], NULL));
            EXPECT_EQ(ENOENT, pfc_strhash_remove(hash, key[i]));

            // set
            EXPECT_EQ(0, pfc_strhash_put(hash, key[i], val[n][i], v_opflag));

            if (VAL_IS_REFPTR(test_data)) {
                if (val[n][i] == NULL) {
                    // check TP itself
                    PFC_ASSERT(VAL_STATUS_REFCNT(test_data, n, i) == 0);
                } else {
                    PFC_ATTR_UNUSED uint32_t val_refcnt =
                            ((pfc_refptr_t *)val[n][i])->pr_refcnt;
                    PFC_ASSERT(val_refcnt >= 2);
                    PFC_ASSERT(val_refcnt
                               == VAL_STATUS_REFCNT(test_data, n, i) + 1);
                    VAL_STATUS_REFCNT_UP(test_data, n, i);
                }
            }

            VAL_STATUS_HASH_IN(test_data, n, i);
        }

        // test get operation
        for (int i = 0; i < nentries; ++i) {
            // get status before action
            pfc_refptr_t *val_refptr = NULL;
            uint32_t old_val_refcnt = -1;
            if (VAL_IS_REFPTR(test_data)) {
                val_refptr = (pfc_refptr_t *)val[n][i];
                old_val_refcnt = val_refptr->pr_refcnt;
            }

            // do get
            pfc_cptr_t v = NULL;
            EXPECT_EQ(0, pfc_strhash_get(hash, key[i], &v));
            EXPECT_EQ(val[n][i], v);

            //
            // check refcnt
            //

            // ref-counter of value
            if (VAL_IS_REFPTR(test_data)) {
                EXPECT_EQ(old_val_refcnt, val_refptr->pr_refcnt);
            }
        }

        // At last iteration, decrement ref-coutner each values and
        // they shuld be destructed at remove operation
        if (n == (val_nsets - 1)) {
            if (VAL_IS_REFPTR(test_data)) {
                for (int i = 0; i < nentries; ++i) {
                    PFC_ASSERT(VAL_STATUS_REFCNT(test_data, n, i) == 2);
                    pfc_refptr_put((pfc_refptr_t *)val[n][i]);
                    VAL_STATUS_REFCNT_DOWN(test_data, n, i);
                }
            }
        }

        // test remove operation
        for (int i = 0; i < nentries; ++i) {
            // get status before action
            pfc_refptr_t *val_refptr = NULL;
            uint32_t old_val_refcnt = -1;
            uint64_t old_val_cnt_dtor = -1;
            if (VAL_IS_REFPTR(test_data)) {
                val_refptr = (pfc_refptr_t *)val[n][i];;
                old_val_refcnt = val_refptr->pr_refcnt;
            }
            if (VAL_IS_STRINT(test_data)) {
                // use same counter when case of strint key
                old_val_cnt_dtor = strint_get_count_dtor();
            }

            // do remove
            EXPECT_EQ(0, pfc_strhash_remove(hash, key[i]));

            //
            // check refcnt
            //

            // ref-counter of value
            if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                PFC_ASSERT(VAL_STATUS_REFCNT(test_data, n, i) > 0);
                if (VAL_STATUS_REFCNT(test_data, n, i) == 1) {
                    if (VAL_IS_STRINT(test_data)) {
                        // dtor counter of val
                        EXPECT_GT(strint_get_count_dtor(), old_val_cnt_dtor);
                    }
                } else {
                    EXPECT_EQ(old_val_refcnt - 1, val_refptr->pr_refcnt);
                }
                VAL_STATUS_REFCNT_DOWN(test_data, n, i);
            }
            VAL_STATUS_HASH_OUT(test_data, n, i);
        }
    }

    // clean up test data
    pfc_hash_destroy(hash);
    test_data_clear_registered_status(test_data);
    test_data_free(test_data);
}


static void
test_clear (test_refptr_mode_t val_refptr_mode)
{
    // init test data
    int nbuckets = 31;
    int nentries = nbuckets / 2;
    int val_nsets = 2;
    test_data_t *test_data = test_data_alloc(nbuckets,
                                             nentries,
                                             val_nsets,
                                             val_refptr_mode);
    const char **key = test_data->key;
    pfc_ptr_t **val = test_data->val;

    // create ref-hash
    pfc_hash_t hash;
    test_data_create_hash(test_data, &hash);

    // do test
    int v_opflag = test_data->v_opflag;
    for (int n = 0; n < val_nsets; ++n) {
        // set values
        for (int i = 0; i < nentries; ++i) {
            EXPECT_EQ(0, pfc_strhash_put(hash, key[i], val[n][i], v_opflag));

            // check TP itself about values status
            if (VAL_IS_REFPTR(test_data)) {
                if (val[n][i] == NULL) {
                    PFC_ASSERT(VAL_STATUS_REFCNT(test_data, n, i) == 0);
                } else {
                    PFC_ATTR_UNUSED uint32_t val_refcnt =
                            ((pfc_refptr_t *)val[n][i])->pr_refcnt;
                    PFC_ASSERT(val_refcnt >= 2);
                    PFC_ASSERT(val_refcnt
                               == VAL_STATUS_REFCNT(test_data, n, i) + 1);
                    VAL_STATUS_REFCNT_UP(test_data, n, i);
                }
            }

            VAL_STATUS_HASH_IN(test_data, n, i);
        }

        // At last iteration, decrement ref-coutner each values.
        // They should be destructed by clear operation
        if (n == (val_nsets - 1)) {
            for (int i = 0; i < nentries; ++i) {
                if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                    PFC_ASSERT(VAL_STATUS_REFCNT(test_data, n, i) == 2);
                    pfc_refptr_put((pfc_refptr_t *)val[n][i]);
                    VAL_STATUS_REFCNT_DOWN(test_data, n, i);
                }
            }
        }

        // test clear operation
        {
            // check real ref-counter and status recorded in test_data
            for (int i = 0; i < nentries; i++) {
                if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                    EXPECT_EQ(((pfc_refptr_t *)val[n][i])->pr_refcnt,
                              VAL_STATUS_REFCNT(test_data, n, i));
                }
            }

#ifndef	PARALLEL_TEST
            // get status before action
            uint64_t old_strint_cnt_dtor = -1;
            if (VAL_IS_REFPTR(test_data)) {
                old_strint_cnt_dtor = strint_get_count_dtor();
            }
#endif	/* !PARALLEL_TEST */

            // do clear
            EXPECT_EQ((size_t)nentries, pfc_hash_clear(hash));

            if (n < (val_nsets - 1)) {
                // check emptiness by `get'
                for (int i = 0; i < nentries; ++i) {
                    EXPECT_EQ(ENOENT,
                              pfc_strhash_get(hash, key[i], NULL));
                }
            }

            // check emptiness by iterator
            {
                int iter_cnt = 0;
                pfc_hashiter_t it = pfc_hashiter_get(hash);
                while (pfc_hashiter_next(it, NULL, NULL) == 0) {
                    ++iter_cnt;
                }
                EXPECT_EQ(0, iter_cnt);
            }

            //
            // check refcnt
            //
            uint32_t expected_dtor_count = 0;

            for (int i = 0; i < nentries; ++i) {
                // ref-counter of value
                if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                    pfc_refptr_t *val_refptr = (pfc_refptr_t *)val[n][i];
                    PFC_ASSERT(VAL_STATUS_REFCNT(test_data, n, i) > 0);
                    if (VAL_STATUS_REFCNT(test_data, n, i) == 1) {
                        if (VAL_IS_STRINT(test_data)) {
                            ++expected_dtor_count;
                        }
                    } else {
                        EXPECT_EQ(VAL_STATUS_REFCNT(test_data, n, i) - 1,
                                  val_refptr->pr_refcnt);
                    }
                    VAL_STATUS_REFCNT_DOWN(test_data, n, i);
                }
                VAL_STATUS_HASH_OUT(test_data, n, i);
            }

            // check destructor count
#ifndef PARALLEL_TEST
            if (VAL_IS_REFPTR(test_data)) {
                EXPECT_EQ(old_strint_cnt_dtor + expected_dtor_count,
                          strint_get_count_dtor());
            }
#endif  /* !PARALLEL_TEST */
        }
    }

    // clean up test data
    pfc_hash_destroy(hash);
    test_data_clear_registered_status(test_data);
    test_data_free(test_data);
}


static void
test_destroy_sub (test_refptr_mode_t val_refptr_mode,
                  pfc_bool_t test_dtor)
{
    // init test data
    int nbuckets = 31;
    int nentries = nbuckets / 2;
    int val_nsets = 1;
    test_data_t *test_data = test_data_alloc(nbuckets,
                                             nentries,
                                             val_nsets,
                                             val_refptr_mode);
    const char **key = test_data->key;
    pfc_ptr_t **val = test_data->val;

    // create ref-hash
    pfc_hash_t hash;
    test_data_create_hash(test_data, &hash);

    int v_opflag = test_data->v_opflag;

    // In this test, only one value set is used.
    // Then index of value set is fixed to zero.
    int n = 0;

    // set values
    for (int i = 0; i < nentries; ++i) {
        EXPECT_EQ(0, pfc_strhash_put(hash, key[i], val[n][i], v_opflag));

        // check and update status of value ref-counter
        if (VAL_IS_REFPTR(test_data)) {
            if (val[n][i] == NULL) {
                PFC_ASSERT(VAL_STATUS_REFCNT(test_data, n, i) == 0);
            } else {
                uint32_t val_refcnt = ((pfc_refptr_t *)val[n][i])->pr_refcnt;
                EXPECT_EQ(2U, val_refcnt);
                VAL_STATUS_REFCNT_UP(test_data, n, i);
                PFC_ASSERT(val_refcnt == VAL_STATUS_REFCNT(test_data, n, i));
            }
        }

        // update using status
        VAL_STATUS_HASH_IN(test_data, n, i);

        if (test_dtor) {
            // Decrement value ref-coutner by release
            // from test data set.
            // They should be destructed by destroy operation.
            if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                pfc_refptr_put((pfc_refptr_t *)val[n][i]);
                VAL_STATUS_REFCNT_DOWN(test_data, n, i);
            }
        }
    }

    // get user dtor count before action
#ifndef	PARALLEL_TEST
    uint64_t old_strint_dtor_cnt = -1;
    uint64_t expected_dtor_inc = -1;
    if (test_dtor) {
        if (VAL_IS_STRINT(test_data)) {
            old_strint_dtor_cnt = strint_get_count_dtor();
            expected_dtor_inc = 0;
            for(int i = 0; i < nentries; ++i) {
                if (VAL_IS_STRINT(test_data) && (val[n][i] != NULL)) {
                    ++expected_dtor_inc;
                }
            }
        }
    }
#endif	/* !PARALLEL_TEST */

    // do destroy
    pfc_hash_destroy(hash);

    // check after destroy
    if (test_dtor) {
        // check destructor count
#ifndef PARALLEL_TEST
        if (VAL_IS_STRINT(test_data)) {
            EXPECT_EQ(old_strint_dtor_cnt + expected_dtor_inc,
                      strint_get_count_dtor());
        }
#endif  /* !PARALLEL_TEST */
    } else {
        // check ref-counter
        for (int i = 0; i < nentries; ++i) {
            if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                uint32_t val_refcnt = ((pfc_refptr_t *)val[n][i])->pr_refcnt;
                EXPECT_EQ(1U, val_refcnt);
                VAL_STATUS_REFCNT_DOWN(test_data, n, i);
                PFC_ASSERT(val_refcnt == VAL_STATUS_REFCNT(test_data, n, i));
            }
        }
    }

    // clean up test data
    if (test_dtor) {
        test_data_clear_registered_status(test_data);
    }
    test_data_free(test_data);
}


static void
test_destroy (test_refptr_mode_t val_refptr_mode)
{
    test_destroy_sub(val_refptr_mode, PFC_TRUE);
    test_destroy_sub(val_refptr_mode, PFC_FALSE);
}


static void
test_set_capacity_sub (test_refptr_mode_t val_refptr_mode,
                       int nentries,
                       ssize_t *new_caps)
{
    // init test data
    int nbuckets = 31;
    int val_nsets = 2;
    test_data_t *test_data = test_data_alloc(nbuckets,
                                             nentries,
                                             val_nsets,
                                             val_refptr_mode);
    const char **key = test_data->key;
    pfc_ptr_t **val = test_data->val;

    // create ref-hash
    pfc_hash_t hash;
    test_data_create_hash(test_data, &hash);

    // do test
    int v_opflag = test_data->v_opflag;
    for (ssize_t *next_cap = new_caps; *next_cap > 0; ++next_cap) {
        for (int n = 0; n < val_nsets; ++n) {
            // set values
            for (int i = 0; i < nentries; ++i) {
                EXPECT_EQ(0, pfc_strhash_put(hash, key[i], val[n][i], v_opflag));

                // check TP itself about values status
                if (VAL_IS_REFPTR(test_data)) {
                    if (val[n][i] == NULL) {
                        PFC_ASSERT(VAL_STATUS_REFCNT(test_data, n, i) == 0);
                    } else {
                        PFC_ATTR_UNUSED uint32_t val_refcnt =
                                ((pfc_refptr_t *)val[n][i])->pr_refcnt;
                        PFC_ASSERT(val_refcnt >= 2);
                        PFC_ASSERT(val_refcnt
                                   == VAL_STATUS_REFCNT(test_data, n, i) + 1);
                        VAL_STATUS_REFCNT_UP(test_data, n, i);
                    }
                }

                VAL_STATUS_HASH_IN(test_data, n, i);
            }
            EXPECT_EQ((size_t)nentries, pfc_hash_get_size(hash));

            // check ref-counters before action
            for (int i = 0; i < nentries; i++) {
                if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                    EXPECT_EQ(((pfc_refptr_t *)val[n][i])->pr_refcnt,
                              VAL_STATUS_REFCNT(test_data, n, i));
                }
            }

#ifndef	PARALLEL_TEST
            // get status before action
            uint64_t old_strint_cnt_dtor = -1;
            if (VAL_IS_REFPTR(test_data)) {
                old_strint_cnt_dtor = strint_get_count_dtor();
            }
#endif	/* !PARALLEL_TEST */

            // do set_capacity
            EXPECT_EQ(0, pfc_hash_set_capacity(hash, *next_cap));

            // cehck new capacity
            EXPECT_EQ((uint32_t)*next_cap, pfc_hash_get_capacity(hash));

            // check each elemets in hash by get action
            for (int i = 0; i < nentries; ++i) {
                EXPECT_EQ(0, pfc_strhash_get(hash, key[i], NULL));
            }

            // check each elemets in hash by iterator
            {
                int iter_cnt = 0;
                pfc_hashiter_t it = pfc_hashiter_get(hash);
                while (pfc_hashiter_next(it, NULL, NULL) == 0) {
                    ++iter_cnt;
                }
                EXPECT_EQ(nentries, iter_cnt);
            }

            // check ref-counters after action
            for (int i = 0; i < nentries; i++) {
                if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                    EXPECT_EQ(((pfc_refptr_t *)val[n][i])->pr_refcnt,
                              VAL_STATUS_REFCNT(test_data, n, i));
                }
            }

            // check destructor count
#ifndef PARALLEL_TEST
            if (VAL_IS_REFPTR(test_data)) {
                EXPECT_EQ(old_strint_cnt_dtor, strint_get_count_dtor());
            }
#endif  /* !PARALLEL_TEST */

            // clear hash
            EXPECT_EQ((size_t)nentries, pfc_hash_clear(hash));

            // check emptiness by `get'
            for (int i = 0; i < nentries; ++i) {
                EXPECT_EQ(ENOENT, pfc_strhash_get(hash, key[i], NULL));
            }

            // check emptiness by iterator
            {
                int iter_cnt = 0;
                pfc_hashiter_t it = pfc_hashiter_get(hash);
                while (pfc_hashiter_next(it, NULL, NULL) == 0) {
                    ++iter_cnt;
                }
                EXPECT_EQ(0, iter_cnt);
            }

            // update test data status
            for (int i = 0; i < nentries; i++) {
                if (VAL_IS_REFPTR(test_data) && (val[n][i] != NULL)) {
                    VAL_STATUS_REFCNT_DOWN(test_data, n, i);
                }
                VAL_STATUS_HASH_OUT(test_data, n, i);
            }
        }
    }

    // clean up test data
    pfc_hash_destroy(hash);
    test_data_clear_registered_status(test_data);
    test_data_free(test_data);
}


static void
test_set_capacity (test_refptr_mode_t val_refptr_mode)
{
    // nbuckets pattern 1:
    //   each of [1, 20) and variations
    ssize_t max_nbuckets1 = ((!is_under_heavy_load()) ? 20 : 10);
    for (ssize_t nbuckets = 1;
         nbuckets < max_nbuckets1;
         ++nbuckets)
    {
        ssize_t new_caps[] = { nbuckets,
                               nbuckets * 2,
                               -1 /* end mark */ };
        test_set_capacity_sub(val_refptr_mode,
                              200,
                              new_caps);
    }

    // nbuckets pattern 2:
    //   divied points in [500, PFC_HASH_MAX_NBUCKETS]
    //   and variations
    int nsplit = ((!is_under_heavy_load()) ? 4 : 3);
    ssize_t max_nbuckets2 = PFC_HASH_MAX_NBUCKETS / 2;
    for (ssize_t nbuckets = 500;
         nbuckets < max_nbuckets2;
         nbuckets += max_nbuckets2 / nsplit)
    {
        ssize_t new_caps[] = { nbuckets,
                               nbuckets * 2 - 123,
                               -1 /* end mark */ };
        test_set_capacity_sub(val_refptr_mode,
                              200,
                              new_caps);
    }

    // nbuckets pattern 3:
    //   neighborhood of PFC_HASH_MAX_NBUCKETS
    ssize_t start_nbuckets = (PFC_HASH_MAX_NBUCKETS
                              - ((!is_under_heavy_load()) ? 1 : 0));
    for (ssize_t nbuckets = start_nbuckets;
         nbuckets <= static_cast<ssize_t>(PFC_HASH_MAX_NBUCKETS);
         ++nbuckets)
    {
        ssize_t new_caps[] = { nbuckets,
                               nbuckets / 2 - 123,
                               -1 /* end mark */ };
        test_set_capacity_sub(val_refptr_mode,
                              50,
                              new_caps);
    }
}



/*
 * Test cases
 */

TEST(hash, str_vref_put_null)
{
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_strhash_create(&hash, NULL, 31, 0));

    // test data
    const char *key = "1234";
    pfc_cptr_t val = NULL;

    // put
    EXPECT_EQ(0, pfc_strhash_put(hash, key, val, PFC_HASHOP_VAL_REFPTR));

    // check with get / remove operation
    {
        // get
        pfc_cptr_t v =(pfc_cptr_t)-1;
        EXPECT_EQ(0, pfc_strhash_get(hash, key, &v));
        EXPECT_EQ(val, v);

        // remove
        EXPECT_EQ(0, pfc_strhash_remove(hash, key));
    }

    // clean up
    pfc_hash_destroy(hash);
}


TEST(hash, str_vref_update_null)
{
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_strhash_create(&hash, NULL, 31, 0));

    // test data
    const char *key = "5678";
    pfc_cptr_t val1 = NULL;
    const char *val2_body = "abcd";
    pfc_cptr_t val2 = pfc_refptr_create(NULL, (pfc_ptr_t)val2_body);
    pfc_cptr_t val3 = NULL;

    EXPECT_EQ(1U, ((pfc_refptr_t *)val2)->pr_refcnt);

    // update (unregistered key)
    EXPECT_EQ(0, pfc_strhash_update(hash, key, val1, PFC_HASHOP_VAL_REFPTR));

    // check
    {
        pfc_cptr_t v = (pfc_cptr_t)-1;
        EXPECT_EQ(0, pfc_strhash_get(hash, key, &v));
        EXPECT_EQ(val1, v);
    }

    // update (replace registered NULL value with refptr)
    EXPECT_EQ(0, pfc_strhash_update(hash, key, val2, PFC_HASHOP_VAL_REFPTR));

    // check
    {
        EXPECT_EQ(2U, ((pfc_refptr_t *)val2)->pr_refcnt);

        pfc_cptr_t v = (pfc_cptr_t)-1;
        EXPECT_EQ(0, pfc_strhash_get(hash, key, &v));
        EXPECT_EQ(val2, v);
    }

    // update (replace registered refptr with NULL)
    EXPECT_EQ(0, pfc_strhash_update(hash, key, val3, PFC_HASHOP_VAL_REFPTR));

    // check
    {
        EXPECT_EQ(1U, ((pfc_refptr_t *)val2)->pr_refcnt);

        pfc_cptr_t v = (pfc_cptr_t)-1;
        EXPECT_EQ(0, pfc_strhash_get(hash, key, &v));
        EXPECT_EQ(val3, v);
    }

    // clean up
    pfc_refptr_put((pfc_refptr_t *)val2);
    pfc_hash_destroy(hash);
}


TEST(hash, str_vref_put_update_null_ops)
{
    // init test data
    int nbuckets = 31;
    int nentries = nbuckets / 2;
    int val_nsets = 2;
    test_data_t *test_data = test_data_alloc(nbuckets,
                                             nentries,
                                             val_nsets,
                                             REFPTR_DEFAULT);

    // do test
    test_put_ents(test_data, INPUT_OP_PUT);
    EXPECT_TRUE(GET_FLAG(test_data, put_unreg_key));
    EXPECT_TRUE(GET_FLAG(test_data, put_existing_key));

    test_put_ents(test_data, INPUT_OP_UPDATE);
    EXPECT_TRUE(GET_FLAG(test_data, update_unreg_key));
    EXPECT_TRUE(GET_FLAG(test_data, update_existing_key));

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, str_vref_get_remove_null_ops)
{
    test_get_remove(REFPTR_DEFAULT); /* type of value */
}


TEST(hash, str_vref_clear_null_ops)
{
    test_clear(REFPTR_DEFAULT); /* type of value */
}


TEST(hash, str_vref_destroy_null_ops)
{
    test_destroy(REFPTR_DEFAULT); /* type of value */
}


TEST(hash, str_vref_set_capacity_null_ops)
{
    test_set_capacity(REFPTR_DEFAULT); /* type of value */
}


TEST(hash, str_vref_put_update_user_ops)
{
    // init test data
    int nbuckets = 31;
    int nentries = nbuckets / 2;
    int val_nsets = 2;
    test_data_t *test_data = test_data_alloc(nbuckets,
                                             nentries,
                                             val_nsets,
                                             REFPTR_STRINT);

    // do test
    test_put_ents(test_data, INPUT_OP_PUT);
    EXPECT_TRUE(GET_FLAG(test_data, put_unreg_key));
    EXPECT_TRUE(GET_FLAG(test_data, put_existing_key));

    test_put_ents(test_data, INPUT_OP_UPDATE);
    EXPECT_TRUE(GET_FLAG(test_data, update_unreg_key));
    EXPECT_TRUE(GET_FLAG(test_data, update_existing_key));

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, str_vref_get_remove_user_ops)
{
    test_get_remove(REFPTR_STRINT);  /* type of value */
}


TEST(hash, str_vref_clear_user_ops)
{
    test_clear(REFPTR_STRINT);  /* type of value */
}


TEST(hash, str_vref_destroy_user_ops)
{
    test_destroy(REFPTR_STRINT);  /* type of value */
}


TEST(hash, str_vref_set_capacity_user_ops)
{
    test_set_capacity(REFPTR_STRINT);  /* type of value */
}



/*
 * Operations on test data
 */

const pfc_refptr_ops_t *
refptr_mode2ops (test_refptr_mode_t refptr_mode)
{
    const pfc_refptr_ops_t *ops;
    void *invalid_ptr = (void *)(uintptr_t)-1;
    switch (refptr_mode) {
        case REFPTR_NONE:    ops = (pfc_refptr_ops_t *)invalid_ptr; break;
        case REFPTR_DEFAULT: ops = NULL;                            break;
        case REFPTR_STRINT:  ops = strint_ops;                      break;
        default: abort();
    }
    return ops;
}


static test_data_t *
test_data_alloc(int nbuckets,
                int nentries,
                int val_nsets,
                test_refptr_mode_t val_refptr_mode)
{
    PFC_ASSERT(val_nsets >= 0);
    PFC_ASSERT(nentries > 0);
    prand_generator_t prand_gen = prand_create(HASH_RANDOM_SEED);
    test_data_t *test_data = (test_data_t *)calloc(1, sizeof(test_data_t));

    // determine refptr ops and flags
    const pfc_refptr_ops_t *v_ops = refptr_mode2ops(val_refptr_mode);

    uint32_t v_opflag = ((val_refptr_mode == REFPTR_NONE)
                         ? 0 : PFC_HASHOP_VAL_REFPTR);

    //
    // allocate a set of keys
    //
    char **key = (char **)calloc(nentries, sizeof(char *));
    PFC_ASSERT(key != NULL);
    for (int i = 0; i < nentries; ++i) {
      retry:
        key[i] = random_string_with_prand_gen(prand_gen, 100, '!', '~');
        PFC_ASSERT(key[i] != NULL);
        for (int j = 0; j < i; ++j) {
            if (strcmp(key[i], key[j]) == 0) {
                free(key[i]);
                goto retry;
            }
        }
    }

    //
    // allocate sets of values
    //
    void ***val = NULL;
    uint32_t **v_refcnt = NULL;
    int **v_registered = NULL;

    if (val_nsets > 0) {
        val = (void ***)calloc(val_nsets, sizeof(void **));
        PFC_ASSERT(val != NULL);

        if (val_refptr_mode != REFPTR_NONE) {
            v_refcnt = (uint32_t **)calloc(val_nsets, sizeof(uint32_t *));
            PFC_ASSERT(v_refcnt != NULL);
        }
        v_registered = (int **)calloc(val_nsets, sizeof(int *));
        PFC_ASSERT(v_registered != NULL);

        for (int n = 0; n < val_nsets; ++n) {
            val[n] = (void **)calloc(nentries, sizeof(void *));

            if (val_refptr_mode != REFPTR_NONE) {
                v_refcnt[n] = (uint32_t *)calloc(nentries, sizeof(uint32_t));
                PFC_ASSERT(v_refcnt != NULL);
            }
            v_registered[n] = (int *)calloc(nentries, sizeof(int));
            PFC_ASSERT(v_registered != NULL);

            for (int i = 0; i < nentries; ++i) {

                switch (val_refptr_mode) {
                    case REFPTR_NONE: {
                        const int maxlen = 200;
                        val[n][i] = (char *)calloc(maxlen, sizeof(char));
                        PFC_ASSERT(val[n][i] != NULL);
                        snprintf((char *)(val[n][i]), maxlen,
                                 "value/%d with key = %p = \"%s\"",
                                 n, key[i], key[i]);
                        break;
                    }
                    case REFPTR_DEFAULT: {
                        uint32_t x = (n << 16) + i;
                        val[n][i] = pfc_refptr_create(NULL, (void *)(uintptr_t)x);
                        PFC_ASSERT(val[n][i] != NULL);
                        v_refcnt[n][i] = 1;
                        break;
                    }
                    case REFPTR_STRINT: {
                        uint32_t x = i * 100 + n;
                        val[n][i] = strint_create_from_int(x);
                        PFC_ASSERT(val[n][i] != NULL);
                        v_refcnt[n][i] = 1;
                        break;
                    }
                    default:
                        fprintf(stderr, "unknown: val_refpt_mode = %d\n",
                                val_refptr_mode);
                        abort();
                }

            }
        }
    }

    // store members
    test_data->nbuckets  = nbuckets;
    test_data->nentries  = nentries;
    test_data->val_nsets = val_nsets;
    test_data->val_refptr_mode = val_refptr_mode;

    test_data->v_ops      = v_ops;
    test_data->v_opflag   = v_opflag;

    test_data->key      = (const char **)key;
    test_data->val      = val;
    test_data->head_val = (val == NULL) ? NULL : val[0];
    test_data->tail_val = (val == NULL) ? NULL : val[val_nsets-1];

    test_data->v_refcnt = v_refcnt;

    test_data->v_registered = v_registered;

    // clean up
    prand_destroy(prand_gen);

    return test_data;
}


static void
test_data_create_hash (test_data_t *test_data, pfc_hash_t *hashp)
{
    PFC_ASSERT(test_data != NULL);
    EXPECT_EQ(0, pfc_strhash_create(hashp,
                                    NULL,
                                    test_data->nbuckets,
                                    0));
}


static void
test_data_clear_registered_status (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    int nentries        = test_data->nentries;
    int val_nsets       = test_data->val_nsets;
    pfc_ptr_t **val     = test_data->val;
    uint32_t **v_refcnt = test_data->v_refcnt;
    int **v_registered  = test_data->v_registered;

    if (VAL_IS_REFPTR(test_data)) {
        for (int n = 0; n < val_nsets; ++n) {
            for (int i = 0; i < nentries; ++i) {
                if (v_registered[n][i]) {
                    if (val[n][i] == NULL) continue;
                    PFC_ASSERT(v_refcnt[n][i] > 0);
                    --v_refcnt[n][i];
                    v_registered[n][i] = 0;
                }
            }
        }
    }
}


static void
test_data_free (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    int nentries  = test_data->nentries;
    int val_nsets = test_data->val_nsets;

    const char **key = test_data->key;
    void ***val = test_data->val;

    uint32_t **v_refcnt = test_data->v_refcnt;
    int **v_registered = test_data->v_registered;

    // release keys
    for (int i = 0; i < nentries; ++i) {
        if (key[i] != NULL) {
            free((char *)key[i]);
        }
    }
    free(key);

    // release values
    for (int n = 0; n < val_nsets; n++) {
        for (int i = 0; i < nentries; ++i) {
            if (VAL_IS_REFPTR(test_data)) {
                for (; v_refcnt[n][i] > 0; --v_refcnt[n][i]) {
                    pfc_refptr_put((pfc_refptr_t *)val[n][i]);
                }
            } else {
                free(val[n][i]);
            }
        }
        free(val[n]);
    }
    free(val);

    if (VAL_IS_REFPTR(test_data)) {
        for (int n = 0; n < val_nsets; n++) {
            free(v_refcnt[n]);
        }
        free(v_refcnt);
    }

    for (int n = 0; n < val_nsets; n++) {
        free(v_registered[n]);
    }
    free(v_registered);

    // release top
    free(test_data);
}
