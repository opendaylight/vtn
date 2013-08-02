/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for hash under following conditions:
 *    - hash is created by pfc_refhash_create with hash_ops
 *      specified by user
 *    - hash keys are refptr and its operators are specified in
 *      kops and pfc_refptr_ops by user
 *    - values are also refptr and its operators are specified in
 *      pfc_refptr_ops by user (but they are differnt from key's)
 */

#include <pfc/hash.h>
#include <string.h>
#include "hash_util.h"
#include "strint_refptr.h"
#include "test_hash_cmn_hashops.hh"


/*
 * Shorter version definitions imported from "test_hash_cmn_hashops.hh"
 */

typedef hashops_test_data_t test_data_t;
#define SET_FLAG                        HASHOPS_SET_FLAG
#define TESTDATA_DECL_MEMBER_ACCESSOR   HASHOPS_TESTDATA_DECL_MEMBER_ACCESSOR
#define TESTDATA_KREF_REFCNT            HASHOPS_TESTDATA_KREF_REFCNT
#define TESTDATA_VREF_REFCNT            HASHOPS_TESTDATA_VREF_REFCNT
#define TESTDATA_KREF_AUTOPUT           HASHOPS_TESTDATA_KREF_AUTOPUT
#define TESTDATA_VREF_AUTOPUT           HASHOPS_TESTDATA_VREF_AUTOPUT
#define TESTDATA_GET_OLD_COUNTS         HASHOPS_TESTDATA_GET_OLD_COUNTS


static hashops_test_data_t *
test_data_alloc (int nents,
                 int val_nsets,
                 const pfc_refptr_ops_t *kref_ops,
                 const pfc_refptr_ops_t *vref_ops,
                 const pfc_hash_ops_t   *hash_ops)
{
    return hashops_test_data_alloc(nents,
                                   val_nsets,
                                   kref_ops,
                                   vref_ops,
                                   hash_ops);
}

static void
test_data_free (test_data_t *test_data)
{
    hashops_test_data_free(test_data);
}

static void
test_data_inject_null (test_data_t *test_data)
{
    hashops_test_data_inject_null(test_data);
}


/*
 * Prototype for test operations
 */

static void test_in_out_entries(test_data_t *test_data,
                                pfc_hash_t hash,
                                bool empty_hash = true,
                                input_op_t input_op_for_1st = INPUT_OP_MIX);


/*
 * Macros for test data
 */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_KEYDTOR_UNCHANGED(TEST_DATA)               \
    EXPECT_EQ(old_cnt_kref_dtor,              cnt_kref_dtor());         \
    EXPECT_EQ(old_cnt_hash_ops_key_dtor,      cnt_hash_ops_key_dtor()); \
    EXPECT_EQ(old_cnt_hash_ops_key_dtor_kref, cnt_hash_ops_key_dtor_kref());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_KEYDTOR_UNCHANGED(TEST_DATA)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_VALDTOR_UNCHANGED(TEST_DATA)               \
    EXPECT_EQ(old_cnt_vref_dtor,              cnt_vref_dtor());         \
    EXPECT_EQ(old_cnt_hash_ops_val_dtor,      cnt_hash_ops_val_dtor()); \
    EXPECT_EQ(old_cnt_hash_ops_val_dtor_vref, cnt_hash_ops_val_dtor_vref());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_VALDTOR_UNCHANGED(TEST_DATA)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_HASHFUNC_UNCHANGED(TEST_DATA)          \
    EXPECT_EQ(old_cnt_kref_hashfunc,     cnt_kref_hashfunc());      \
    EXPECT_EQ(old_cnt_hash_ops_hashfunc, cnt_hash_ops_hashfunc());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_HASHFUNC_UNCHANGED(TEST_DATA)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_EQUALS_UNCHANGED(TEST_DATA)        \
    EXPECT_EQ(old_cnt_kref_equals,     cnt_kref_equals());      \
    EXPECT_EQ(old_cnt_hash_ops_equals, cnt_hash_ops_equals());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_EQUALS_UNCHANGED(TEST_DATA)
#endif  /* !PARALLEL_TEST */

#define TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(TEST_DATA)    \
    TESTDATA_CHECK_COUNT_KEYDTOR_UNCHANGED(TEST_DATA);          \
    TESTDATA_CHECK_COUNT_VALDTOR_UNCHANGED(TEST_DATA);

#define TESTDATA_CHECK_COUNT_ALL_UNCHANGED(TEST_DATA)       \
    TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(TEST_DATA);   \
    TESTDATA_CHECK_COUNT_HASHFUNC_UNCHANGED(TEST_DATA);     \
    TESTDATA_CHECK_COUNT_EQUALS_UNCHANGED(TEST_DATA);

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_KEYDTOR_INC(TEST_DATA, X)  \
    EXPECT_EQ(old_cnt_kref_dtor + (X),                  \
              cnt_kref_dtor());                         \
    EXPECT_EQ(old_cnt_hash_ops_key_dtor + (X),          \
              cnt_hash_ops_key_dtor());                 \
    EXPECT_EQ(old_cnt_hash_ops_key_dtor_kref + (X),     \
              cnt_hash_ops_key_dtor_kref());
#else  /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_KEYDTOR_INC(TEST_DATA, X)  \
    EXPECT_LE(old_cnt_kref_dtor + (X),                  \
              cnt_kref_dtor());                         \
    EXPECT_LE(old_cnt_hash_ops_key_dtor + (X),          \
              cnt_hash_ops_key_dtor());                 \
    EXPECT_LE(old_cnt_hash_ops_key_dtor_kref + (X),     \
              cnt_hash_ops_key_dtor_kref());
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_VALDTOR_INC(TEST_DATA, X)  \
    EXPECT_EQ(old_cnt_vref_dtor + (X),                  \
              cnt_vref_dtor());                         \
    EXPECT_EQ(old_cnt_hash_ops_val_dtor + (X),          \
              cnt_hash_ops_val_dtor());                 \
    EXPECT_EQ(old_cnt_hash_ops_val_dtor_vref + (X),     \
              cnt_hash_ops_val_dtor_vref());
#else  /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_VALDTOR_INC(TEST_DATA, X)  \
    EXPECT_LE(old_cnt_vref_dtor + (X),                  \
              cnt_vref_dtor());                         \
    EXPECT_LE(old_cnt_hash_ops_val_dtor + (X),          \
              cnt_hash_ops_val_dtor());                 \
    EXPECT_LE(old_cnt_hash_ops_val_dtor_vref + (X),     \
              cnt_hash_ops_val_dtor_vref());
#endif  /* !PARALLEL_TEST */

#define TESTDATA_CHECK_COUNT_KEYVALDTOR_INC(TEST_DATA, X)   \
    TESTDATA_CHECK_COUNT_KEYDTOR_INC(TEST_DATA, X);         \
    TESTDATA_CHECK_COUNT_VALDTOR_INC(TEST_DATA, X);

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_HASHFUNC_INC(TEST_DATA, X)                 \
    EXPECT_EQ(old_cnt_kref_hashfunc + (X), cnt_kref_hashfunc());        \
    EXPECT_EQ(old_cnt_hash_ops_hashfunc,   cnt_hash_ops_hashfunc());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_HASHFUNC_INC(TEST_DATA, X)             \
    EXPECT_LE(old_cnt_kref_hashfunc + (X), cnt_kref_hashfunc());
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_EQUALS_INC(TEST_DATA, X)               \
    EXPECT_EQ(old_cnt_kref_equals + (X), cnt_kref_equals());        \
    EXPECT_EQ(old_cnt_hash_ops_equals,   cnt_hash_ops_equals());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_EQUALS_INC(TEST_DATA, X)           \
    EXPECT_LE(old_cnt_kref_equals + (X), cnt_kref_equals());
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_EQUALS_INC_MORE(TEST_DATA, X)          \
    EXPECT_LE(old_cnt_kref_equals + (X), cnt_kref_equals());        \
    EXPECT_EQ(old_cnt_hash_ops_equals,   cnt_hash_ops_equals());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_EQUALS_INC_MORE(TEST_DATA, X)      \
    EXPECT_LE(old_cnt_kref_equals + (X), cnt_kref_equals());
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_LASTADDR_KEYDTOR(TEST_DATA, REF_ADDR, OBJ_ADDR)  \
    EXPECT_EQ((pfc_cptr_t)(OBJ_ADDR), lastaddr_kref_dtor());            \
    EXPECT_EQ((pfc_cptr_t)(REF_ADDR), lastaddr_hash_ops_key_dtor());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_LASTADDR_KEYDTOR(TEST_DATA, REF_ADDR, OBJ_ADDR)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_LASTADDR_VALDTOR(TEST_DATA, REF_ADDR, OBJ_ADDR)  \
    EXPECT_EQ((pfc_cptr_t)(OBJ_ADDR), lastaddr_vref_dtor());            \
    EXPECT_EQ((pfc_cptr_t)(REF_ADDR), lastaddr_hash_ops_val_dtor());
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_LASTADDR_VALDTOR(TEST_DATA, REF_ADDR, OBJ_ADDR)
#endif  /* !PARALLEL_TEST */


/*
 * Test cases
 */

TEST(hash, kref_vref_hashops_destroy_dtor_count)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    // int val_nsets = 2;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // fill hash with data
    for (int i = 0; i < nents; ++i) {
        // put
        {
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = (pfc_cptr_t)vrefs[0][i];
            EXPECT_EQ(0, pfc_hash_put(hash, k, v, kvref_flags));
        }

        // check with refptr key
        {
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, k, &v, kvref_flags));
            EXPECT_EQ(vrefs[0][i], (pfc_refptr_t *)v);
        }

        // check with normal pointer key
        if (krefs[i] != NULL) {
            pfc_cptr_t k = pfc_refptr_value(krefs[i]);
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, k, &v, 0));
            EXPECT_EQ(vrefs[0][i], (pfc_refptr_t *)v);
        }

        // down refcnt
        TESTDATA_KREF_AUTOPUT(test_data, i);
        TESTDATA_VREF_AUTOPUT(test_data, 0, i);
        PFC_ASSERT(krefs[i]->pr_refcnt > 0);
        PFC_ASSERT(vrefs[0][i]->pr_refcnt > 0);
    }

    // get old counters
    TESTDATA_GET_OLD_COUNTS(test_data);

    // do destroy
    pfc_hash_destroy(hash);

    // check key / value dtor counts
    TESTDATA_CHECK_COUNT_KEYVALDTOR_INC(test_data, nents);

    // check others counts
    TESTDATA_CHECK_COUNT_HASHFUNC_UNCHANGED(test_data);
    TESTDATA_CHECK_COUNT_EQUALS_UNCHANGED(test_data);

    // clean up
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_clear)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 10;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    test_data_t *test_data2 = test_data_alloc(nents,
                                              val_nsets,
                                              kref_ops,
                                              vref_ops,
                                              hash_ops);
    if (test_data2 == NULL) abort();

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // clear empty hash
    {
        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do clear
        EXPECT_EQ(0U, pfc_hash_clear(hash));

        // check all counters are unchanged
        TESTDATA_CHECK_COUNT_ALL_UNCHANGED(test_data);

        // check size
        EXPECT_EQ(0U, pfc_hash_get_size(hash));

        // do many actions on hash
        test_in_out_entries(test_data, hash);
    }

    // clear non-empty hash
    {
        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do claer
        EXPECT_EQ((size_t)nents, pfc_hash_clear(hash));

        // check key / value dtor count
        TESTDATA_CHECK_COUNT_KEYVALDTOR_INC(test_data, nents);

        // check size
        EXPECT_EQ(0U, pfc_hash_get_size(hash));

        // check no elements are found
        for (int i = 0; i < nents; i++) {
            // try to get with normal pointer key
            pfc_cptr_t k = permanent_str_keys[i];
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, k, &v, 0));
            EXPECT_EQ(INVALID_PTR, v);
        }

        // do many actions on hash
        test_in_out_entries(test_data2, hash);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
    test_data_free(test_data2);
}


TEST(hash, kref_vref_hashops_put_update)
{
    for (int k = 0; k < 4; ++k) {
        pfc_bool_t flag_null_injection = ((k / 2) % 2 == 0);

        // init test data
        int nbuckets = 31;
        int nents = nbuckets * 4; // keep more than '2' for injecting NULL
        int val_nsets = 4;        // keep more than '2' for injecting NULL
        const pfc_refptr_ops_t *kref_ops = strint_ops;
        const pfc_refptr_ops_t *vref_ops = intref_ops;
        const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
        test_data_t *test_data = test_data_alloc(nents,
                                                 val_nsets,
                                                 kref_ops,
                                                 vref_ops,
                                                 hash_ops);
        if (test_data == NULL) abort();
        if (flag_null_injection) test_data_inject_null(test_data);

        test_data_t *test_data2 = test_data_alloc(nents,
                                                  val_nsets,
                                                  kref_ops,
                                                  vref_ops,
                                                  hash_ops);
        if (test_data2 == NULL) abort();
        if (flag_null_injection) test_data_inject_null(test_data2);

        // create hash
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                        nbuckets, 0));

        if (k % 2 == 0) {
            // do many operations including `put'
            test_in_out_entries(test_data, hash, true, INPUT_OP_PUT);

            // check test cases are passed
            EXPECT_TRUE(test_data->tp_flags.put_unreg_key);
            EXPECT_TRUE(test_data->tp_flags.put_existing_key);
            if (flag_null_injection) {
                EXPECT_TRUE(test_data->tp_flags.put_null_key);
                EXPECT_TRUE(test_data->tp_flags.put_null_value);
            }
        } else {
            // do many operations including `update'
            test_in_out_entries(test_data2, hash, true, INPUT_OP_UPDATE);

            // check test cases are passed
            EXPECT_TRUE(test_data2->tp_flags.update_unreg_key);
            EXPECT_TRUE(test_data2->tp_flags.update_existing_key);
            if (flag_null_injection) {
                EXPECT_TRUE(test_data2->tp_flags.update_null_key);
                EXPECT_TRUE(test_data2->tp_flags.update_null_value);
                EXPECT_TRUE(test_data2->tp_flags.update_null_key_value);
            }
        }

        // clean up
        pfc_hash_destroy(hash);
        test_data_free(test_data);
        test_data_free(test_data2);
    }
}


TEST(hash, kref_vref_hashops_put_equals_hashfunc_count)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    // int val_nsets = 2;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // put with unregistered keys
    {
        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        for (int i = 0; i < nents; ++i) {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // do put
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = (pfc_cptr_t)vrefs[0][i];
            EXPECT_EQ(0, pfc_hash_put(hash, k, v, kvref_flags));

            // check hashfunc count
            TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

            // check othre counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_date);

            // down refcnt
            TESTDATA_KREF_AUTOPUT(test_data, i);
            TESTDATA_VREF_AUTOPUT(test_data, 0, i);
            PFC_ASSERT(((pfc_refptr_t *)k)->pr_refcnt > 0);
            PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);
        }

        if (nents > nbuckets) {
            // check equals count
            TESTDATA_CHECK_COUNT_EQUALS_INC(test_data, nents - nbuckets);
        }
    }

    // try to put with registered keys
    for (int i = 0; i < nents; ++i) {
        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // try to put
        pfc_cptr_t k = (pfc_cptr_t)krefs[i];
        pfc_cptr_t v = (pfc_cptr_t)vrefs[0][i];
        EXPECT_EQ(EEXIST, pfc_hash_put(hash, k, v, kvref_flags));

        // check hashfunc count
        TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

        // check equals count
        TESTDATA_CHECK_COUNT_EQUALS_INC_MORE(test_data, 1);

        // check othre counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_date);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_update_equals_hashfunc_count)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 2;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // update with unregistered keys
    {
        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        for (int i = 0; i < nents; ++i) {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // do update
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = (pfc_cptr_t)vrefs[0][i];
            EXPECT_EQ(0, pfc_hash_update(hash, k, v, kvref_flags));

            // check hashfunc count
            TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

            // check othre counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_date);

            // down refcnt
            TESTDATA_KREF_AUTOPUT(test_data, i);
            TESTDATA_VREF_AUTOPUT(test_data, 0, i);
            PFC_ASSERT(((pfc_refptr_t *)k)->pr_refcnt > 0);
            PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);
        }

        if (nents > nbuckets) {
            // check equals count
            TESTDATA_CHECK_COUNT_EQUALS_INC(test_data, nents - nbuckets);
        }
    }

    // update with registered keys
    for (int i = 0; i < nents; ++i) {
        // get old counters and object address
        TESTDATA_GET_OLD_COUNTS(test_data);
        PFC_ATTR_UNUSED pfc_cptr_t obj_addr = (vrefs[0][i])->pr_object;

        // do update
        pfc_cptr_t k = (pfc_cptr_t)krefs[i];
        pfc_cptr_t v = (pfc_cptr_t)vrefs[1][i];
        EXPECT_EQ(0, pfc_hash_update(hash, k, v, kvref_flags));

        // check val dtor count and target address
        TESTDATA_CHECK_COUNT_VALDTOR_INC(test_data, 1);
        TESTDATA_CHECK_LASTADDR_VALDTOR(test_data, vrefs[0][i], obj_addr);

        // check key dtor count
        TESTDATA_CHECK_COUNT_KEYDTOR_UNCHANGED(test_data);

        // check hashfunc count
        TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

        // check equals count
        TESTDATA_CHECK_COUNT_EQUALS_INC_MORE(test_data, 1);

        // down refcnt
        TESTDATA_VREF_AUTOPUT(test_data, 1, i);
        PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_put_update_null_key_val)
{
    // This test complements `TEST(hash, kref_vref_hashops_put_update)' which
    // doesn't sufficiently test the case of (key, value) = (NULL, NULL).

    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 0;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // init hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // Do put unregistered (NULL, NULL) entry  (It should success)
    {
        // check precondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, NULL, &v, kvref_flags));
            EXPECT_EQ(INVALID_PTR, v);  // v isn't changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(0, pfc_hash_put(hash, NULL, NULL, kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, kvref_flags));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }

    // Do put registered (NULL, NULL) entry  (It should fail)
    {
        // check precondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, kvref_flags));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(EEXIST, pfc_hash_put(hash, NULL, NULL, kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, kvref_flags));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }


    // reset hash
    EXPECT_EQ(1U, pfc_hash_clear(hash));


    // Do update unregistered (NULL, NULL) entry  (It should success)
    {
        // check precondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, NULL, &v, kvref_flags));
            EXPECT_EQ(INVALID_PTR, v);  // v isn't changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(0, pfc_hash_update(hash, NULL, NULL, kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, kvref_flags));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }

    // Do update registered (NULL, NULL) entry  (It should fail)
    {
        // check precondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, kvref_flags));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(0, pfc_hash_update(hash, NULL, NULL, kvref_flags));

        // check key other counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, kvref_flags));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }


    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_put_dummy_null_key)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 0;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    pfc_refptr_t *null_ref_key = strint_create_from_int(0);
    strint_ops->dtor(null_ref_key->pr_object);
    null_ref_key->pr_object = NULL;

    pfc_refptr_t *null_ref_key2 = strint_create_from_int(0);
    strint_ops->dtor(null_ref_key2->pr_object);
    null_ref_key2->pr_object = NULL;

    // init hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // Do put unregistered (NULL ref key, NULL) entry
    // (It should success)
    {
        // check precondition
        PFC_ASSERT(null_ref_key->pr_refcnt == 1);
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ(INVALID_PTR, v);  // v isn't changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(0, pfc_hash_put(hash, null_ref_key, NULL, kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // douwn refcnt
        PFC_ASSERT(null_ref_key->pr_refcnt == 2);
        pfc_refptr_put(null_ref_key);
    }

    // Do put registered (NULL ref key, NULL) entry
    // (It should fail)
    {
        // check precondition
        PFC_ASSERT(null_ref_key->pr_refcnt == 1);
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(EEXIST, pfc_hash_put(hash, null_ref_key, NULL,
                                       kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }

    // Do put registered (another NULL ref key, NULL) entry
    // (It should fail)
    {
        // check precondition
        PFC_ASSERT(null_ref_key2->pr_refcnt == 1);
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(EEXIST, pfc_hash_put(hash, null_ref_key2, NULL,
                                       kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // douwn refcnt
        PFC_ASSERT(null_ref_key2->pr_refcnt == 1);
        pfc_refptr_put(null_ref_key2);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}



TEST(hash, kref_vref_hashops_update_dummy_null_key)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 0;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    pfc_refptr_t *null_ref_key = strint_create_from_int(0);
    strint_ops->dtor(null_ref_key->pr_object);
    null_ref_key->pr_object = NULL;

    pfc_refptr_t *null_ref_key2 = strint_create_from_int(0);
    strint_ops->dtor(null_ref_key2->pr_object);
    null_ref_key2->pr_object = NULL;

    // init hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // Do update unregistered (NULL ref key, NULL) entry
    // (It should success)
    {
        // check precondition
        PFC_ASSERT(null_ref_key->pr_refcnt == 1);
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ(INVALID_PTR, v);  // v isn't changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(0, pfc_hash_update(hash, null_ref_key, NULL,
                                     kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // douwn refcnt
        PFC_ASSERT(null_ref_key->pr_refcnt == 2);
        pfc_refptr_put(null_ref_key);
    }

    // Do update registered (NULL ref key, NULL) entry
    // (It should success)
    {
        // check precondition
        PFC_ASSERT(null_ref_key->pr_refcnt == 1);
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(0, pfc_hash_update(hash, null_ref_key, NULL,
                                     kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }

    // Do update registered (another NULL ref key, NULL) entry
    // (It should should)
    {
        // check precondition
        PFC_ASSERT(null_ref_key2->pr_refcnt == 1);
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // get old counters and object address
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do put
        EXPECT_EQ(0, pfc_hash_update(hash, null_ref_key2, NULL,
                                     kvref_flags));

        // check dtor counts
        TESTDATA_CHECK_COUNT_KEYDTOR_INC(test_data, 1);
        TESTDATA_CHECK_LASTADDR_KEYDTOR(test_data, null_ref_key, NULL);
        TESTDATA_CHECK_COUNT_VALDTOR_UNCHANGED(test_data);

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // douwn refcnt
        PFC_ASSERT(null_ref_key2->pr_refcnt == 2);
        pfc_refptr_put(null_ref_key2);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_get)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 4; // keep more than '2' for injecting NULL cases
    int val_nsets = 4;        // keep more than '2' for injecting NULL cases
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // try to get for each unregistered entries
    for (int i = 0; i < nents; ++i) {
        // try to get with refptr key and non-NULL valuep
        {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to get
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, k, &v, kvref_flags));
            EXPECT_EQ(INVALID_PTR, v);

            // check hashfunc counts
            if (k != NULL) {
                TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);
            }

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }

        // try to get with refptr key and NULL valuep
        {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to get
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, k, NULL, kvref_flags));

            // check hashfunc counts
            if (k != NULL) {
                TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);
            }

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }

        // try to get with normal pointer key and non-NULL valuep
        if (krefs[i] != NULL) {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to get
            pfc_cptr_t k = PFC_REFPTR_VALUE(krefs[i], pfc_cptr_t);
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, k, &v, 0));
            EXPECT_EQ(INVALID_PTR, v);

            // check hashfunc counts
            TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }

        // try to get with normal pointer key and NULL valuep
        if (krefs[i] != NULL) {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to get
            pfc_cptr_t k = PFC_REFPTR_VALUE(krefs[i], pfc_cptr_t);
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, k, NULL, 0));

            // check hashfunc counts
            TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }
    }

    // do many operations including `get'
    test_in_out_entries(test_data, hash);

    // check test cases are passed
    EXPECT_TRUE(test_data->tp_flags.get_unreg_key);
    EXPECT_TRUE(test_data->tp_flags.get_existing_key);
    EXPECT_TRUE(test_data->tp_flags.get_null_key);
    EXPECT_TRUE(test_data->tp_flags.get_null_valuep);
    EXPECT_TRUE(test_data->tp_flags.get_null_key_valuep);

    // get for each entries
    for (int i = 0; i < nents; ++i) {
        // do get with refptr key and non-NULL valuep
        {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // do get
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, k, &v, kvref_flags));
            EXPECT_EQ(vrefs_at_tail[i], (pfc_refptr_t *)v);

            // check hashfunc and equals counts
            if (k != NULL) {
                TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);
                TESTDATA_CHECK_COUNT_EQUALS_INC_MORE(test_data, 1);
            }

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }

        // do get with refptr key and non-NULL valuep
        {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to get
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            EXPECT_EQ(0, pfc_hash_get(hash, k, NULL, kvref_flags));

            // check hashfunc and equals counts
            if (k != NULL) {
                TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);
                TESTDATA_CHECK_COUNT_EQUALS_INC_MORE(test_data, 1);
            }

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }

        // do get with normal pointer key and non-NULL valuep
        if (krefs[i] != NULL) {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to get
            pfc_cptr_t k = PFC_REFPTR_VALUE(krefs[i], pfc_cptr_t);
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, k, &v, 0));
            EXPECT_EQ(vrefs_at_tail[i], (pfc_refptr_t *)v);

            // check hashfunc counts
            TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }

        // do get with normal pointer key and NULL valuep
        if (krefs[i] != NULL) {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to get
            pfc_cptr_t k = PFC_REFPTR_VALUE(krefs[i], pfc_cptr_t);
            EXPECT_EQ(0, pfc_hash_get(hash, k, NULL, 0));

            // check hashfunc counts
            TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_remove)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 4; // keep more than '2' for injecting NULL cases
    int val_nsets = 4;        // keep more than '2' for injecting NULL cases
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // try to remove for each unregistered entries
    for (int i = 0; i < nents; ++i) {
        // try to remove with refptr key
        {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to remove
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            EXPECT_EQ(ENOENT, pfc_hash_remove(hash, k, kvref_flags));

            // check hashfunc counts
            if (k != NULL) {
                TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);
            }

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }

        // try to remove with normal pointer key
        if (krefs[i] != NULL) {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // try to remove
            pfc_cptr_t k = PFC_REFPTR_VALUE(krefs[i], pfc_cptr_t);
            EXPECT_EQ(ENOENT, pfc_hash_remove(hash, k, 0));

            // check hashfunc counts
            if (k != NULL) {
                TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);
            }

            // check key / value dtor counts
            TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
        }
    }

    // set entries into hash
    test_in_out_entries(test_data, hash);

    // remove all entries
    for (int i = 0; i < nents; ++i) {
        // prepare key and expected value
        pfc_cptr_t k = (pfc_cptr_t)krefs[i];
        pfc_cptr_t k_obj = INVALID_PTR;
        if (krefs[i] != NULL) {
            k_obj = PFC_REFPTR_VALUE(krefs[i], pfc_cptr_t);
        }
        pfc_cptr_t permanent_str_key = (pfc_cptr_t)permanent_str_keys[i];
        pfc_cptr_t expected_v = (pfc_cptr_t)vrefs_at_tail[i];
#ifndef	PARALLEL_TEST
        pfc_cptr_t expected_v_obj = INVALID_PTR;
        if (vrefs_at_tail[i] != NULL) {
            expected_v_obj = PFC_REFPTR_VALUE(vrefs_at_tail[i],
                                              pfc_cptr_t);
        }
#endif	/* !PARALLEL_TEST */

        // check entry is in hash with refptr key
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, k, &v, kvref_flags));
            EXPECT_EQ(expected_v, v);
        }

        // check entry is in hash wiht normal pointer key
        if (k != NULL) {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, permanent_str_key, &v, 0));
            EXPECT_EQ(expected_v, v);
        }

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        if ((k == NULL) || ((i % 2) == 0)) {
            // do remvoe with refptr key
            EXPECT_EQ(0, pfc_hash_remove(hash, k, kvref_flags));
        } else {
            // do remvoe with normal pointer key
            EXPECT_EQ(0, pfc_hash_remove(hash, k_obj, 0));
        }

        if (k != NULL) {
            // check hashfunc count
            TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, 1);

            // check equals count
            TESTDATA_CHECK_COUNT_EQUALS_INC_MORE(test_data, 1);

            // check key dtor counters and last address
            TESTDATA_CHECK_COUNT_KEYDTOR_INC(test_data, 1);
            TESTDATA_CHECK_LASTADDR_KEYDTOR(test_data, k, k_obj);
        }

        if (expected_v != NULL) {
            // check val dtor counters and last address
            TESTDATA_CHECK_COUNT_VALDTOR_INC(test_data, 1);
            TESTDATA_CHECK_LASTADDR_VALDTOR(test_data, expected_v,
                                            expected_v_obj);
        }

        // check entry is not in hash
        EXPECT_EQ(ENOENT, pfc_hash_get(hash, permanent_str_key,
                                       NULL, 0));
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


static void
test_get_size (int nbuckets, int nents)
{
    // init test data
    int val_nsets = 1;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // get size and put for each entries
    for (int i = 0; i < nents; ++i) {
        // do get_size
        EXPECT_EQ((size_t)i, pfc_hash_get_size(hash));

        // put an entry
        pfc_cptr_t k = (pfc_cptr_t)krefs[i];
        pfc_cptr_t v = (pfc_cptr_t)vrefs[0][i];
        EXPECT_EQ(0, pfc_hash_put(hash, k, v, kvref_flags));

        // down refcnt
        TESTDATA_KREF_AUTOPUT(test_data, i);
        TESTDATA_VREF_AUTOPUT(test_data, 0, i);
        PFC_ASSERT(((pfc_refptr_t *)k)->pr_refcnt > 0);
        PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);
    }

    // get size after putting all entries
    EXPECT_EQ((size_t)nents, pfc_hash_get_size(hash));

    // get size and remove for each entries
    for (int i = 0; i < nents; ++i) {
        // do get_size
        EXPECT_EQ((size_t)(nents - i), pfc_hash_get_size(hash));

        // remove an entry
        pfc_cptr_t k = (pfc_cptr_t)krefs[i];
        EXPECT_EQ(0, pfc_hash_remove(hash, k, kvref_flags));
    }

    // get size after removing all entries
    EXPECT_EQ((size_t)0, pfc_hash_get_size(hash));

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_get_size_1)
{
    for (int nbuckets = 1;
         nbuckets < 500;
         ++nbuckets)
    {
        test_get_size(nbuckets, nbuckets * 2);
    }
}


TEST(hash, kref_vref_hashops_get_size_2)
{
    for (int nbuckets = 500;
         nbuckets < (int)PFC_HASH_MAX_NBUCKETS;
         nbuckets += 1000000)
    {
        test_get_size(nbuckets, 1000);
    }
}


static void
test_get_set_capacity (int nbuckets, int nents, ssize_t *new_caps)
{
    // test conditions
    int val_nsets = 1;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // capacity right after creating
    EXPECT_EQ((size_t)nbuckets, pfc_hash_get_capacity(hash));

    size_t cur_cap = nbuckets;
    for (ssize_t *next_cap = new_caps; *next_cap > 0; ++next_cap) {
        EXPECT_EQ(cur_cap, pfc_hash_get_capacity(hash));

        // allocate test data
        test_data_t *test_data = test_data_alloc(nents,
                                                 val_nsets,
                                                 kref_ops,
                                                 vref_ops,
                                                 hash_ops);
        if (test_data == NULL) abort();
        TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

        test_data_t *test_data2 = test_data_alloc(nents,
                                                  val_nsets,
                                                  kref_ops,
                                                  vref_ops,
                                                  hash_ops);
        if (test_data2 == NULL) abort();

        // many operations on hash
        test_in_out_entries(test_data, hash);

        // get size
        size_t hash_size = pfc_hash_get_size(hash);

        // get old counters
        TESTDATA_GET_OLD_COUNTS(test_data);

        // do set_capacity
        pfc_bool_t is_same_cap = (cur_cap == (size_t)(*next_cap));
        int set_cap_err = pfc_hash_set_capacity(hash, *next_cap);
        EXPECT_EQ(0, set_cap_err);
        if (set_cap_err != 0) {
            char buf[1024] = { 0 };
            strerror_r(set_cap_err, buf, sizeof(buf));
            fprintf(stderr,
                    "cur_cap = %"PFC_PFMT_SIZE_T", "
                    "*next_cap = %"PFC_PFMT_SIZE_T", "
                    "err = %d (%s)\n",
                    cur_cap, *next_cap, set_cap_err, buf);
        }
        cur_cap = *next_cap;

        // check by get_capacity
        EXPECT_EQ(cur_cap, pfc_hash_get_capacity(hash));

        if (!is_same_cap) {
            // check hashfunc count
            TESTDATA_CHECK_COUNT_HASHFUNC_INC(test_data, hash_size);
        }

        // get old key / value dtor count
        TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_date);

        // check key-value matching for each entries
        for (int i = 0; i < test_data->nents; ++i) {
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = NULL;
            EXPECT_EQ(0, pfc_hash_get(hash, k, &v, kvref_flags));
            EXPECT_EQ(vrefs_at_tail[i], (pfc_refptr_t *)v);
        }

        // many operations on hash
        test_in_out_entries(test_data2, hash, false /* not emtpy */);

        // clean up
        pfc_hash_clear(hash);
        test_data_free(test_data);
        test_data_free(test_data2);
    }

    // clean up
    pfc_hash_destroy(hash);
}


TEST(hash, kref_vref_hashops_get_set_capacity_1)
{
    ssize_t max_nbuckets = ((!is_under_heavy_load()) ? 50 : 30);
    for (ssize_t nbuckets = 1;
         nbuckets < max_nbuckets;
         ++nbuckets)
    {
        ssize_t new_caps[] = { nbuckets,
                               nbuckets * 2,
                               nbuckets / 2 + (nbuckets < 2) ? 1 : 0,
                               nbuckets * 2 + 123,
                               nbuckets / 2 + 123,
                               -1 /* end mark */ };
        int nents = (nbuckets < 500) ? (nbuckets * 2) : 1000;
        test_get_set_capacity(nbuckets, nents, new_caps);
    }
}


TEST(hash, kref_vref_hashops_get_set_capacity_2)
{
    int nsplit = ((!is_under_heavy_load()) ? 5 : 3);
    ssize_t max_nbuckets = PFC_HASH_MAX_NBUCKETS / 2;
    for (ssize_t nbuckets = 500;
         nbuckets < max_nbuckets;
         nbuckets += max_nbuckets / nsplit)
    {
        ssize_t new_caps[] = { nbuckets,
                               nbuckets * 2,
                               nbuckets / 2,
                               nbuckets * 2 - 123,
                               nbuckets / 2 - 123,
                               -1 /* end mark */ };
        test_get_set_capacity(nbuckets, 1000, new_caps);
    }
}


TEST(hash, kref_vref_hashops_get_set_capacity_3)
{
    ssize_t start_nbuckets = (PFC_HASH_MAX_NBUCKETS
                              - ((!is_under_heavy_load()) ? 1 : 0));
    for (ssize_t nbuckets = start_nbuckets;
         nbuckets <= static_cast<ssize_t>(PFC_HASH_MAX_NBUCKETS);
         ++nbuckets)
    {
        ssize_t new_caps[] = { nbuckets,
                               nbuckets - 1,
                               nbuckets / 2 - 123,
                               -1 /* end mark */ };
        test_get_set_capacity(nbuckets, 100, new_caps);
    }
}


TEST(hash, kref_vref_hashops_set_capacity_error)
{
    // create hash
    int nbuckets = 31;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // check initial bucket size
    EXPECT_EQ((size_t)nbuckets, pfc_hash_get_capacity(hash));

    // try to set zero bucket size
    EXPECT_NE(0, pfc_hash_set_capacity(hash, 0));

    // try to set over bucket size
    EXPECT_NE(0, pfc_hash_set_capacity(hash, PFC_HASH_MAX_NBUCKETS+1));

    // clean up hash
    pfc_hash_destroy(hash);
}


TEST(hash, kref_vref_hashops_report)
{
    // open file to write report
    const char *report_path = getenv("TEST_HASH_REPORT_PATH");;
    if (report_path == NULL) {
        report_path = (const char *)"/dev/null";
    }
    FILE* report_fp;
    if (strcmp("-", report_path) == 0) {
        report_fp = stdout;
    }  else {
        report_fp = fopen(report_path, "w");
        if (report_fp == NULL) {
            perror(report_path);
            fprintf(stderr, "Failed to open hash report file\n");
            abort();
        }
    }

    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // report of empty hash
    pfc_hash_report(hash, report_fp);

    // put and report for each entries
    for (int i = 0; i < nents; ++i) {
        // put an entry
        pfc_cptr_t k = (pfc_cptr_t)krefs[i];
        pfc_cptr_t v = (pfc_cptr_t)vrefs[0][i];
        EXPECT_EQ(0, pfc_hash_put(hash, k, v, kvref_flags));

        // down refcnt
        TESTDATA_KREF_AUTOPUT(test_data, i);
        TESTDATA_VREF_AUTOPUT(test_data, 0, i);
        PFC_ASSERT(((pfc_refptr_t *)k)->pr_refcnt > 0);
        PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);

        // do report
        pfc_hash_report(hash, report_fp);
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data and file
    if (strcmp("-", report_path) != 0) {
        EXPECT_EQ(0, fclose(report_fp));
    }
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_iterator_get_next_put)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;
    test_data_t *test_data = test_data_alloc(nents,
                                             val_nsets,
                                             kref_ops,
                                             vref_ops,
                                             hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    // put an entry and hashiter operations for each entires
    for (int i = -1; i < test_data->nents; ++i) {
        if (i >= 0) {
            // put
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = (pfc_cptr_t)vrefs[0][i];
            EXPECT_EQ(0, pfc_hash_put(hash, k, v, kvref_flags));

            // down refcnt
            TESTDATA_KREF_AUTOPUT(test_data, i);
            TESTDATA_VREF_AUTOPUT(test_data, 0, i);
            PFC_ASSERT(((pfc_refptr_t *)k)->pr_refcnt > 0);
            PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);
        }

        // get hashiter and put immediately
        {
            pfc_hashiter_t it = pfc_hashiter_get(hash);
            EXPECT_TRUE(it != NULL);
            pfc_hashiter_put(it);
        }

        // flags to check test program itself
        bool pass_nullKey_nullVal = false;
        bool pass_nullKey_realVal = false;
        bool pass_realKey_nullVal = false;
        bool pass_realKey_realVal = false;

        // traverse first half entires with hash iterator
        int max_iters = test_data->nents / 2; // iter cut off threshold
        int nents_in_hash = i + 1;            // current entries in hash
        for (int kv_bitmap = 0; kv_bitmap < 0x4; ++kv_bitmap) {
            pfc_cptr_t key, val;
            pfc_cptr_t *keyp = (kv_bitmap & 0x1) ? (&key) : NULL;
            pfc_cptr_t *valp = (kv_bitmap & 0x2) ? (&val) : NULL;

            // copy keys and values to verify
            pfc_cptr_t rest_keys[nents_in_hash];
            pfc_cptr_t rest_vals[nents_in_hash];
            if (keyp != NULL)
                memcpy(rest_keys, krefs, sizeof(rest_keys));
            if (valp != NULL)
                memcpy(rest_vals, vrefs_at_head, sizeof(rest_vals));

            // start of iteration
            pfc_hashiter_t it = pfc_hashiter_get(hash);
            EXPECT_TRUE(it != NULL);
            int n_iters = 0;
            int iter_next_err;
            while ((iter_next_err = pfc_hashiter_next(it, keyp, valp)) == 0) {
                int kpos = -1;
                if (keyp != NULL) {
                    // key should exist in rest_keys
                    for (int j = 0; j < nents_in_hash; ++j) {
                        if (rest_keys[j] == key) { kpos = j; break; }
                    }
                    EXPECT_NE(-1, kpos);
                }

                int vpos = -1;
                if (valp != NULL) {
                    // value should exist in rest_keys
                    for (int j = 0; j < nents_in_hash; ++j) {
                        if (rest_vals[j] == val) { vpos = j; break; }
                    }
                    EXPECT_NE(-1, vpos);
                }

                if (keyp != NULL && valp != NULL) {
                    // positons of key and value should be matching
                    EXPECT_EQ(kpos, vpos);
                }

                if (kpos >= 0) rest_keys[kpos] = NULL;
                if (vpos >= 0) rest_vals[vpos] = NULL;

                ++n_iters;
                if (n_iters >= max_iters) {
                    // break iteration
                    pfc_hashiter_put(it);
                    break;
                }
            }
            // end of iteration

            if (max_iters <= nents_in_hash) {
                // iterations should be broken before reaching end
                // (and some entries should rest)
                EXPECT_EQ(0, iter_next_err);
            } else {
                // iterations should reach end
                EXPECT_EQ(ENOENT, iter_next_err);
                EXPECT_EQ(nents_in_hash, n_iters);

                if (keyp != NULL) {
                    // `rest_kyes' shuld be empty
                    for (int j = 0; j < nents_in_hash; ++j) {
                        EXPECT_TRUE(rest_keys[j] == NULL);
                    }
                }

                if (valp != NULL) {
                    // `rest_kyes' shuld be empty
                    for (int j = 0; j < nents_in_hash; ++j) {
                        EXPECT_TRUE(rest_vals[j] == NULL);
                    }
                }
            }

            // check passed patterns
            if (keyp == NULL && valp == NULL) pass_nullKey_nullVal = true;
            if (keyp == NULL && valp != NULL) pass_nullKey_realVal = true;
            if (keyp != NULL && valp == NULL) pass_realKey_nullVal = true;
            if (keyp != NULL && valp != NULL) pass_realKey_realVal = true;
        }

        // verify that each patters are passed
        EXPECT_TRUE(pass_nullKey_nullVal);
        EXPECT_TRUE(pass_nullKey_realVal);
        EXPECT_TRUE(pass_realKey_nullVal);
        EXPECT_TRUE(pass_realKey_realVal);
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, kref_vref_hashops_iterator_next_err_einval)
{
    // init test data
    size_t nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 2;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;

    // destroy hash before getting iterator
    typedef enum {
        UPDATE_OP_PUT,
        UPDATE_OP_UPDATE,
        UPDATE_OP_REMOVE,
        UPDATE_OP_CLEAR,
        UPDATE_OP_SET_CAPACITY,
        UPDATE_OP_MIX,
        UPDATE_OP_END_MARK,
    } update_ops_t;
    for (int n = 0; n < 2 * UPDATE_OP_END_MARK; ++n) {
        bool call_next_before_update = (n % 2);
        update_ops_t update_op =
                (update_ops_t)((n / 2) % UPDATE_OP_END_MARK);

        // allocate test data
        test_data_t *test_data = test_data_alloc(nents,
                                                 val_nsets,
                                                 kref_ops,
                                                 vref_ops,
                                                 hash_ops);
        if (test_data == NULL) abort();
        TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

        test_data_t *test_data2 = test_data_alloc(nents,
                                                  val_nsets,
                                                  kref_ops,
                                                  vref_ops,
                                                  hash_ops);
        if (test_data2 == NULL) abort();

        int put_key_idx = 0;
        pfc_cptr_t put_key    = test_data2->krefs[put_key_idx];
        pfc_cptr_t put_val    = test_data2->vrefs_at_head[put_key_idx];

        int update_key_idx = 1;
        pfc_cptr_t update_key = test_data2->krefs[update_key_idx];
        pfc_cptr_t update_val = test_data2->vrefs_at_head[update_key_idx];

        test_data_t *test_data3 = test_data_alloc(nents,
                                                  val_nsets,
                                                  kref_ops,
                                                  vref_ops,
                                                  hash_ops);
        if (test_data3 == NULL) abort();

        // create hash
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                        nbuckets, 0));

        // set entries into hash
        test_in_out_entries(test_data, hash);

        if (update_op == UPDATE_OP_PUT) {
            // preparation for later `put' operation
            int err = pfc_hash_remove(hash, put_key, kvref_flags);
#ifndef PARALLEL_TEST
            EXPECT_TRUE(err == 0);
#else   /* !PARALLEL_TEST */
            EXPECT_TRUE((err == 0) || (err == ENOENT));
#endif  /* !PARALLEL_TEST */
        }

        // get iterator
        pfc_hashiter_t it = pfc_hashiter_get(hash);
        EXPECT_TRUE(it != NULL);

        if (call_next_before_update) {
            // once call pfc_hashiter_next() before update
            pfc_cptr_t key, val;
            EXPECT_EQ(0, pfc_hashiter_next(it, &key, &val));
        }

        // update hash table before pfc_hashiter_next()
        switch (update_op) {
            case UPDATE_OP_PUT: {
                EXPECT_EQ(0, pfc_hash_put(hash, put_key, put_val,
                                          kvref_flags));
                // down refcnt
                TESTDATA_KREF_AUTOPUT(test_data2, put_key_idx);
                TESTDATA_VREF_AUTOPUT(test_data2, 0, put_key_idx);
                PFC_ASSERT(((pfc_refptr_t *)put_key)->pr_refcnt > 0);
                PFC_ASSERT(((pfc_refptr_t *)put_val)->pr_refcnt > 0);
                break;
            }
            case UPDATE_OP_UPDATE: {
                EXPECT_EQ(0, pfc_hash_update(hash, update_key,
                                             update_val, kvref_flags));
                // down refcnt
                TESTDATA_KREF_AUTOPUT(test_data2, update_key_idx);
                TESTDATA_VREF_AUTOPUT(test_data2, 0, update_key_idx);
                PFC_ASSERT(((pfc_refptr_t *)update_key)->pr_refcnt > 0);
                PFC_ASSERT(((pfc_refptr_t *)update_val)->pr_refcnt > 0);
                break;
            }
            case UPDATE_OP_REMOVE: {
                pfc_cptr_t rm_key = (pfc_cptr_t)test_data->krefs[0];
                EXPECT_EQ(0, pfc_hash_remove(hash, rm_key, kvref_flags));
                break;
            }
            case UPDATE_OP_CLEAR: {
#ifndef GTEST_REPLACEMENT
                EXPECT_GE(pfc_hash_clear(hash), 0U);
#else  /* !GTEST_REPLACEMENT */
                pfc_hash_clear(hash);
#endif /* !GTEST_REPLACEMENT */
                break;
            }
            case UPDATE_OP_SET_CAPACITY: {
                uint32_t capa = pfc_hash_get_capacity(hash);
                EXPECT_EQ(0, pfc_hash_set_capacity(hash, capa / 2));
                break;
            }
            case UPDATE_OP_MIX: {
                test_in_out_entries(test_data3, hash, false /* not emtpy */);
                break;
            }
            case UPDATE_OP_END_MARK:
                abort();
                break;
        }

        // pfc_hashiter_next() should return EINVAL after update
        {
            pfc_cptr_t key, val;
            EXPECT_EQ(EINVAL, pfc_hashiter_next(it, &key, &val));
        }

        // clean up
        pfc_hash_destroy(hash);
        test_data_free(test_data);
        test_data_free(test_data3);
        test_data_free(test_data2);
    }
}


TEST(hash, kref_vref_hashops_iterator_next_err_ebadf)
{
    // init test data
    size_t nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;

    // destroy hash before getting iterator
    for (int n = 0; n < 2; ++n) {
        // allocate test data
        test_data_t *test_data = test_data_alloc(nents,
                                                 val_nsets,
                                                 kref_ops,
                                                 vref_ops,
                                                 hash_ops);
        if (test_data == NULL) abort();
        TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

        // create hash
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                        nbuckets, 0));

        // set entries into hash
        test_in_out_entries(test_data, hash);

        pfc_hashiter_t it = pfc_hashiter_get(hash);
        EXPECT_TRUE(it != NULL);

        if (n % 2) {
            // once call pfc_hashiter_next() before destruction
            pfc_cptr_t key, val;
            EXPECT_EQ(0, pfc_hashiter_next(it, &key, &val));
        }

        // destroy hash that's refereed by iterator
        pfc_hash_destroy(hash);

        {
            // pfc_hashiter_next() should return EBADF after destruction
            pfc_cptr_t key, val;
            EXPECT_EQ(EBADF, pfc_hashiter_next(it, &key, &val));
        }

        // clean up
        test_data_free(test_data);
    }
}


TEST(hash, kref_vref_hashops_iterator_remove)
{
    // init test data
    size_t nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    const pfc_refptr_ops_t *kref_ops = strint_ops;
    const pfc_refptr_ops_t *vref_ops = intref_ops;
    const pfc_hash_ops_t   *hash_ops = hash_wtchops_kref_vref;

    // create hash
    pfc_hash_t hash;
    EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                    nbuckets, 0));

    for (int n = 0; n < 3; ++n) {
        // allocate test data
        test_data_t *test_data = test_data_alloc(nents,
                                                 val_nsets,
                                                 kref_ops,
                                                 vref_ops,
                                                 hash_ops);
        if (test_data == NULL) abort();
        TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

        // set entries into hash
        test_in_out_entries(test_data, hash);

        // get iterator
        pfc_hashiter_t it = pfc_hashiter_get(hash);
        EXPECT_TRUE(it != NULL);

        {
            // get old counters
            TESTDATA_GET_OLD_COUNTS(test_data);

            // pfc_hashiter_remove should fail right after
            // getting hashiter
            EXPECT_EQ(ENOENT, pfc_hashiter_remove(it));

            // check counts
            TESTDATA_CHECK_COUNT_ALL_UNCHANGED(test_data);
        }

        // traverse and remove for each entries
        int rm_cnt = 0;
        pfc_cptr_t k;
        while (pfc_hashiter_next(it, &k, NULL) == 0) {
            // seek test data index of current entry
            int i; // index to access test_data->key / val
            for (i = 0;
                 krefs[i] != (pfc_refptr_t *)k;
                 ++i)
            {
                if (i > nents) abort();
            }

#ifndef	PARALLEL_TEST
            // prepare key and expected value
            pfc_cptr_t k_obj = INVALID_PTR;
            if (krefs[i] != NULL) {
                k_obj = PFC_REFPTR_VALUE(krefs[i], pfc_cptr_t);
            }
#endif	/* !PARALLEL_TEST */
            pfc_cptr_t permanent_str_key = (pfc_cptr_t)permanent_str_keys[i];
            pfc_cptr_t expected_v = (pfc_cptr_t)vrefs_at_tail[i];
#ifndef	PARALLEL_TEST
            pfc_cptr_t expected_v_obj = INVALID_PTR;
            if (vrefs_at_tail[i] != NULL) {
                expected_v_obj =
                        PFC_REFPTR_VALUE(vrefs_at_tail[i], pfc_cptr_t);
            }
#endif	/* !PARALLEL_TEST */

            // get before removing
            {
                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(0, pfc_hash_get(hash, k, &v, kvref_flags));
                EXPECT_EQ(expected_v, (pfc_refptr_t *)v);
            }

            // remove
            {
                // get old counters
                TESTDATA_GET_OLD_COUNTS(test_data);

                // do hashiter_remove
                EXPECT_EQ(0, pfc_hashiter_remove(it));

                // check key / value dtor count
                if (k != NULL) {
                    // check key dtor counters and last address
                    TESTDATA_CHECK_COUNT_KEYDTOR_INC(test_data, 1);
                    TESTDATA_CHECK_LASTADDR_KEYDTOR(test_data, k, k_obj);
                }

                if (expected_v != NULL) {
                    // check val dtor counters and last address
                    TESTDATA_CHECK_COUNT_VALDTOR_INC(test_data, 1);
                    TESTDATA_CHECK_LASTADDR_VALDTOR(test_data, expected_v,
                                                    expected_v_obj);
                }

                // check hashfunc and equals count
                TESTDATA_CHECK_COUNT_HASHFUNC_UNCHANGED(test_data);
                TESTDATA_CHECK_COUNT_EQUALS_UNCHANGED(test_data);
            }

            // try to get after removing
            EXPECT_EQ(ENOENT,
                      pfc_hash_get(hash, permanent_str_key, NULL, 0));

            // try to remove one more (of courese, it should fail)
            {
                // get old counters
                TESTDATA_GET_OLD_COUNTS(test_data);

                // try to hashiter_remove for unregistered key
                EXPECT_EQ(ENOENT, pfc_hashiter_remove(it));

                // check key / value dtor count
                TESTDATA_CHECK_COUNT_KEYVALDTOR_UNCHANGED(test_data);
            }

            ++rm_cnt;
            EXPECT_EQ((size_t)(test_data->nents - rm_cnt),
                      pfc_hash_get_size(hash));
        }
        EXPECT_EQ(0U, pfc_hash_get_size(hash));

        // clean up test data
        pfc_hash_clear(hash);
        test_data_free(test_data);
    }

    // clean up hash
    pfc_hash_destroy(hash);
}



/*
 * Do many in / out operations
 */

static void
test_in_out_entries (test_data_t *test_data,
                     pfc_hash_t hash,
                     bool empty_hash,
                     input_op_t input_op_for_1st)
{
    int nents     = test_data->nents;
    int val_nsets = test_data->val_nsets;
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    for (int n = 0; n < val_nsets; ++n) {
        // put or update for each entries
        for (int i = 0; i < nents; ++i) {
            // check TP itself
            if (krefs[i] != NULL) {
                if (n == 0) {
                    PFC_ASSERT(skip_key_dtor[i] != PFC_TRUE);
                }
                PFC_ASSERT(TESTDATA_KREF_REFCNT(test_data, i) > 0);
            }
            if (vrefs[n][i] != NULL) {
                PFC_ASSERT(skip_val_dtor[n][i] != PFC_TRUE);
                PFC_ASSERT(TESTDATA_VREF_REFCNT(test_data, n, i) > 0);
            }

            if (n == 0 && empty_hash) {
                // get before putting
                {
                    // try to get
                    pfc_cptr_t k = (pfc_cptr_t)krefs[i];
                    pfc_cptr_t v = INVALID_PTR;
                    EXPECT_EQ(ENOENT,
                              pfc_hash_get(hash, k, &v, kvref_flags));
                    EXPECT_EQ(INVALID_PTR, v);

                    // set TP flags
                    SET_FLAG(test_data, get_unreg_key);
                }

                // determine input operation
                input_op_t input_op;
                if (input_op_for_1st) {
                    input_op = (i % 2) ? INPUT_OP_PUT : INPUT_OP_UPDATE;
                } else {
                    input_op = input_op_for_1st;
                }

                if (input_op == INPUT_OP_PUT) {
                    // put for unregistered key
                    pfc_cptr_t k = (pfc_cptr_t)krefs[i];
                    pfc_cptr_t v = (pfc_cptr_t)vrefs[n][i];
                    EXPECT_EQ(0, pfc_hash_put(hash, k, v, kvref_flags));

                    // down refcnt
                    TESTDATA_KREF_AUTOPUT(test_data, i);
                    TESTDATA_VREF_AUTOPUT(test_data, n, i);
                    if (k != NULL) {
                        PFC_ASSERT(((pfc_refptr_t *)k)->pr_refcnt > 0);
                    }
                    if (v != NULL) {
                        PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);
                    }

                    // set TP flags
                    SET_FLAG(test_data, put_unreg_key);
                    if (krefs[i] == NULL && vrefs[n][i] == NULL) {
                        SET_FLAG(test_data, put_null_key_value);
                    } else if (krefs[i] == NULL) {
                        SET_FLAG(test_data, put_null_key);
                    } else if (vrefs[n][i] == NULL) {
                        SET_FLAG(test_data, put_null_value);
                    }
                } else {
                    // update for unregistered key
                    pfc_cptr_t k = (pfc_cptr_t)krefs[i];
                    pfc_cptr_t v = (pfc_cptr_t)vrefs[n][i];
                    EXPECT_EQ(0, pfc_hash_update(hash, k, v, kvref_flags));

                    // down refcnt
                    TESTDATA_KREF_AUTOPUT(test_data, i);
                    TESTDATA_VREF_AUTOPUT(test_data, n, i);
                    if (k != NULL) {
                        PFC_ASSERT(((pfc_refptr_t *)k)->pr_refcnt > 0);
                    }
                    if (v != NULL) {
                        PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);
                    }

                    // set TP flags
                    SET_FLAG(test_data, update_unreg_key);
                    if (krefs[i] == NULL && vrefs[n][i] == NULL) {
                        SET_FLAG(test_data, update_null_key_value);
                    } else if (krefs[i] == NULL) {
                        SET_FLAG(test_data, update_null_key);
                    } else if (vrefs[n][i] == NULL) {
                        SET_FLAG(test_data, update_null_value);
                    }
                }
            } else {
                int get_err = pfc_hash_get(hash, (pfc_cptr_t)krefs[i],
                                           NULL, kvref_flags);
                if ((get_err != 0) && (get_err != ENOENT)) abort();
                bool existing = (get_err == 0);

                // update for existing entry
                pfc_cptr_t k = (pfc_cptr_t)krefs[i];
                pfc_cptr_t v = (pfc_cptr_t)vrefs[n][i];
                EXPECT_EQ(0, pfc_hash_update(hash, k, v, kvref_flags));

                // down refcnt
                TESTDATA_KREF_AUTOPUT(test_data, i);
                TESTDATA_VREF_AUTOPUT(test_data, n, i);
                if (k != NULL) {
                    PFC_ASSERT(((pfc_refptr_t *)k)->pr_refcnt > 0);
                }
                if (v != NULL) {
                    PFC_ASSERT(((pfc_refptr_t *)v)->pr_refcnt > 0);
                }

                // set TP flags of key status
                if (existing)  {
                    SET_FLAG(test_data, update_existing_key);
                } else {
                    SET_FLAG(test_data, update_unreg_key);
                }

                // set TP flags of NULL case
                if (krefs[i] == NULL && vrefs[n][i] == NULL) {
                    SET_FLAG(test_data, update_null_key_value);
                } else if (krefs[i] == NULL) {
                    SET_FLAG(test_data, update_null_key);
                } else if (vrefs[n][i] == NULL) {
                    SET_FLAG(test_data, update_null_value);
                }
            }

            // get for existing entry
            {
                // do get with refptr key
                {
                    pfc_cptr_t k = (pfc_cptr_t)krefs[i];
                    pfc_cptr_t v = INVALID_PTR;
                    EXPECT_EQ(0, pfc_hash_get(hash, k, &v, kvref_flags));
                    EXPECT_EQ(vrefs[n][i], (pfc_refptr_t *)v);
                }

                // do get with normal pointer key
                if (krefs[i] != NULL) {
                    pfc_cptr_t k = pfc_refptr_value(krefs[i]);
                    pfc_cptr_t v = INVALID_PTR;
                    EXPECT_EQ(0, pfc_hash_get(hash, k, &v, 0));
                    EXPECT_EQ(vrefs[n][i], (pfc_refptr_t *)v);
                }

                // set TP flags
                SET_FLAG(test_data, get_existing_key);
                if (krefs[i] == NULL) {
                    SET_FLAG(test_data, get_null_key);
                }
            }

            // get for existing entry (check only)
            {
                // do get with refptr key
                {
                    pfc_cptr_t k = (pfc_cptr_t)krefs[i];
                    EXPECT_EQ(0, pfc_hash_get(hash, k, NULL, kvref_flags));
                }

                // do get with normal pointer key
                if (krefs[i] != NULL) {
                    pfc_cptr_t k = pfc_refptr_value(krefs[i]);
                    EXPECT_EQ(0, pfc_hash_get(hash, k, NULL, 0));
                }

                // set TP flags
                SET_FLAG(test_data, get_existing_key_check);
                if (krefs[i] == NULL) {
                    SET_FLAG(test_data, get_null_key_valuep);
                } else {
                    SET_FLAG(test_data, get_null_valuep);
                }
            }

            // put for existing entry
            {
                // try do put
                pfc_cptr_t k = (pfc_cptr_t)krefs[i];
                pfc_cptr_t v = (pfc_cptr_t)vrefs[n][i];
                EXPECT_EQ(EEXIST, pfc_hash_put(hash, k, v, kvref_flags));

                // set a TP flag
                SET_FLAG(test_data, put_existing_key);
            }

            // check TP itself
            if (krefs[i] != NULL) {
                PFC_ASSERT(TESTDATA_KREF_REFCNT(test_data, i) > 0);
            }
            if (vrefs[n][i] != NULL) {
                PFC_ASSERT(TESTDATA_VREF_REFCNT(test_data, n, i) > 0);
            }
        }

        // get all entries
        for (int i = 0; i < nents; ++i) {
            pfc_cptr_t k = (pfc_cptr_t)krefs[i];
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, k, &v, kvref_flags));
            EXPECT_EQ(vrefs[n][i], (pfc_refptr_t *)v);
        }
    }
}
