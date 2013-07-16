/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for hash that's created with hash_ops specified by user
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

static hashops_test_data_t *
test_data_alloc (int nents,
                 int val_nsets,
                 const pfc_hash_ops_t *hash_ops)
{
    return hashops_test_data_alloc(nents,
                                   val_nsets,
                                   NULL,
                                   NULL,
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
 * Test cases
 */

TEST(hash, gen_hashops_create_null_ops_members)
{
    pfc_hash_ops_t null_ops;
    memset(&null_ops, 0, sizeof(null_ops));

    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, &null_ops, 0, 0);
    EXPECT_NE(0, err);
}


TEST(hash, gen_hashops_create_zero_bucket)
{
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, strint_kv_hash_ops, 0, 0);
    EXPECT_NE(0, err);
}


TEST(hash, gen_hashops_create_over_buckets)
{
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, strint_kv_hash_ops,
                              PFC_HASH_MAX_NBUCKETS + 1, 0);
    EXPECT_NE(0, err);
}


TEST(hash, gen_hashops_create_good_buckets_1)
{
    for (size_t nbuckets = 1;
         nbuckets <= 100;
         ++nbuckets)
    {
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_hash_create(&hash, strint_kv_hash_ops,
                                     nbuckets, 0));
        pfc_hash_destroy(hash);
    }

}


TEST(hash, gen_hashops_create_good_buckets_2)
{
    for (size_t nbuckets = PFC_HASH_MAX_NBUCKETS - 5;
         nbuckets <= PFC_HASH_MAX_NBUCKETS;
         ++nbuckets)
    {
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_hash_create(&hash, strint_kv_hash_ops, nbuckets, 0));
        pfc_hash_destroy(hash);
    }

}


TEST(hash, gen_hashops_create_good_buckets_3)
{
    for (size_t nbuckets = 1;
         nbuckets <= PFC_HASH_MAX_NBUCKETS;
         nbuckets += PFC_HASH_MAX_NBUCKETS / 10)
    {
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_hash_create(&hash, strint_kv_hash_ops, nbuckets, 0));
        pfc_hash_destroy(hash);
    }
}


TEST(hash, gen_hashops_destroy_dtor_count)
{
    // init
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    // int val_nsets = 2;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // fill hash with data
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ(0, pfc_hash_put(hash, keys[i], vals[0][i], 0));
        skip_key_dtor[i] = PFC_TRUE;
        skip_val_dtor[0][i] = PFC_TRUE;
    }

    // get old key / value dtor count
    uint64_t old_key_dtor_cnt = strint_get_count_key_dtor();
    uint64_t old_val_dtor_cnt = strint_get_count_val_dtor();

    // do destroy
    pfc_hash_destroy(hash);

    // check key / value dtor count
#ifndef PARALLEL_TEST
    EXPECT_EQ(old_key_dtor_cnt + nents, strint_get_count_key_dtor());
    EXPECT_EQ(old_val_dtor_cnt + nents, strint_get_count_val_dtor());
#else  /* !PARALLEL_TEST */
    EXPECT_LE(old_key_dtor_cnt + nents, strint_get_count_key_dtor());
    EXPECT_LE(old_val_dtor_cnt + nents, strint_get_count_val_dtor());
#endif /* !PARALLEL_TEST */

    // clean up
    test_data_free(test_data);
}



TEST(hash, gen_hashops_clear)
{
    // init
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 10;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    char **permanent_str_keys = test_data->permanent_str_keys;

    test_data_t *test_data2 = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data2 == NULL) abort();

    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // clear empty hash
    {
        int clear_num = pfc_hash_clear(hash);
        EXPECT_EQ(0, clear_num);

        uint32_t capa = pfc_hash_get_capacity(hash);
        EXPECT_EQ((uint32_t)nbuckets, capa);

        uint32_t size = pfc_hash_get_size(hash);
        EXPECT_EQ(0U, size);

        test_in_out_entries(test_data, hash);
    }

    // clear non-empty hash
    {
        int clear_num = pfc_hash_clear(hash);
        EXPECT_EQ(nents, clear_num);

        uint32_t capa = pfc_hash_get_capacity(hash);
        EXPECT_EQ((uint32_t)nbuckets, capa);

        uint32_t size = pfc_hash_get_size(hash);
        EXPECT_EQ(0U, size);

        for (int i = 0; i < nents; i++) {
            pfc_cptr_t val = NULL;
            int get_err = pfc_hash_get(hash, permanent_str_keys[i], &val, 0);
            EXPECT_EQ(ENOENT, get_err);
            int get_err2 = pfc_hash_get(hash, permanent_str_keys[i], NULL, 0);
            EXPECT_EQ(ENOENT, get_err2);
        }

        test_in_out_entries(test_data2, hash);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
    test_data_free(test_data2);
}


TEST(hash, gen_hashops_clear_dtor_count)
{
    // init
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    // int val_nsets = 2;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // fill hash with data
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ(0, pfc_hash_put(hash, keys[i], vals[0][i], 0));
        skip_key_dtor[i] = PFC_TRUE;
        skip_val_dtor[0][i] = PFC_TRUE;
    }

    // get old key / value dtor count
    uint64_t old_key_dtor_cnt = strint_get_count_key_dtor();
    uint64_t old_val_dtor_cnt = strint_get_count_val_dtor();

    // do clear
    EXPECT_EQ((size_t)nents, pfc_hash_clear(hash));

    // check key / value dtor count
#ifndef PARALLEL_TEST
    EXPECT_EQ(old_key_dtor_cnt + nents, strint_get_count_key_dtor());
    EXPECT_EQ(old_val_dtor_cnt + nents, strint_get_count_val_dtor());
#else  /* !PARALLEL_TEST */
    EXPECT_LE(old_key_dtor_cnt + nents, strint_get_count_key_dtor());
    EXPECT_LE(old_val_dtor_cnt + nents, strint_get_count_val_dtor());
#endif /* !PARALLEL_TEST */

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, gen_hashops_put_update)
{
    for (int i = 0; i < 4; ++i) {
        // init test data
        int nbuckets = 31;
        int nents = nbuckets * 4; // keep more than '2' for injecting NULL
        int val_nsets = 4;           // keep more than '2' for injecting NULL
        const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
        test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data == NULL) abort();
        test_data_inject_null(test_data);

        test_data_t *test_data2 = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data2 == NULL) abort();
        test_data_inject_null(test_data2);

        // init hash
        pfc_hash_t hash;
        int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
        EXPECT_EQ(0, err);

        if ((i / 2) % 2) {
            test_data_inject_null(test_data);
        }

        if (i % 2 == 0) {
            // do many operations including `put'
            test_in_out_entries(test_data, hash, true, INPUT_OP_PUT);

            // check test cases are passed
            EXPECT_TRUE(test_data->tp_flags.put_unreg_key);
            EXPECT_TRUE(test_data->tp_flags.put_existing_key);
            EXPECT_TRUE(test_data->tp_flags.put_null_key);
            EXPECT_TRUE(test_data->tp_flags.put_null_value);
            // REVISIT: add test for `key == NULL' and `value == NULL'
            // EXPECT_TRUE(test_data->tp_flags.put_null_key_value);
        } else {
            // do many operations including `update'
            test_in_out_entries(test_data2, hash, true, INPUT_OP_UPDATE);

            // check test cases are passed
            EXPECT_TRUE(test_data2->tp_flags.update_unreg_key);
            EXPECT_TRUE(test_data2->tp_flags.update_existing_key);
            EXPECT_TRUE(test_data2->tp_flags.update_null_key);
            EXPECT_TRUE(test_data2->tp_flags.update_null_value);
            EXPECT_TRUE(test_data2->tp_flags.update_null_key_value);
        }

        // clean up
        pfc_hash_destroy(hash);
        test_data_free(test_data);
        test_data_free(test_data2);
    }
}


TEST(hash, gen_hashops_put_equals_hashfunc_count)
{
    // init
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    // int val_nsets = 2;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // put with unregistered keys
    {
        // get old equals count
        uint64_t old_equals_cnt = strint_get_count_equals();
        for (int i = 0; i < nents; ++i) {
            // get old hashfunc count
            uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();

            // do put
            EXPECT_EQ(0, pfc_hash_put(hash, keys[i], vals[0][i], 0));
            skip_key_dtor[i] = PFC_TRUE;
            skip_val_dtor[0][i] = PFC_TRUE;

            // check hashfunc count
#ifndef PARALLEL_TEST
            EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
            EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */
        }

        if (nents > nbuckets) {
            // check equals count
#ifndef PARALLEL_TEST
            EXPECT_EQ(old_equals_cnt + nents - nbuckets,
                      strint_get_count_equals());
#else  /* !PARALLEL_TEST */
            EXPECT_LE(old_equals_cnt + nents - nbuckets,
                      strint_get_count_equals());
#endif /* !PARALLEL_TEST */
        }
    }

    // try to put with registered keys
    for (int i = 0; i < nents; ++i) {
        // get old equals / hashfunc count
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();
        uint64_t old_equals_cnt = strint_get_count_equals();

        // try to put
        EXPECT_EQ(EEXIST, pfc_hash_put(hash, keys[i], vals[0][i], 0));

        // check hashfunc count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */

        // check equals count
        EXPECT_LE(old_equals_cnt + 1, strint_get_count_equals());
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, gen_hashops_update_equals_hashfunc_count)
{
    // init
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 2;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // update with unregistered keys
    {
        // get old equals count
        uint64_t old_equals_cnt = strint_get_count_equals();
        for (int i = 0; i < nents; ++i) {
            // get old  hashfunc count
            uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();

            // do update
            EXPECT_EQ(0, pfc_hash_update(hash, keys[i], vals[0][i], 0));
            skip_key_dtor[i] = PFC_TRUE;
            skip_val_dtor[0][i] = PFC_TRUE;

            // check hashfunc count
#ifndef PARALLEL_TEST
            EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
            EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */
        }

        if (nents > nbuckets) {
            // check equals count
            EXPECT_LE(old_equals_cnt + nents - nbuckets,
                      strint_get_count_equals());
        }
    }

    // update with registered keys
    for (int i = 0; i < nents; ++i) {
        // get old equals / hashfunc count
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();
        uint64_t old_equals_cnt = strint_get_count_equals();

        // get old val dtor count
        uint64_t old_val_dtro_cnt = strint_get_count_val_dtor();

        // do update
        EXPECT_EQ(0, pfc_hash_update(hash, keys[i], vals[1][i], 0));
        skip_key_dtor[i] = PFC_TRUE;
        skip_val_dtor[1][i] = PFC_TRUE;

        // check val dtor count and target address
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_val_dtro_cnt + 1, strint_get_count_val_dtor());
        EXPECT_EQ(vals[0][i], strint_get_val_dtor_last_addr());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_val_dtro_cnt + 1, strint_get_count_val_dtor());
#endif /* !PARALLEL_TEST */

        // check hashfunc count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */

        // check equals count
        EXPECT_LE(old_equals_cnt + 1, strint_get_count_equals());
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, gen_hashops_put_update2)
{
    // This test complements `TEST(hash, gen_hashops_put_update)' which
    // doesn't sufficiently test the case of (key, value) = (NULL, NULL).

    // test cond
    int nbuckets = 31;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;

    // init hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);


    // Do put unregistered (NULL, NULL) entry  (It should success)
    {
        // check precondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ(INVALID_PTR, v);  // v isn't changed
        }

        // do put
        EXPECT_EQ(0, pfc_hash_put(hash, NULL, NULL, 0));

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }

    // Do put registered (NULL, NULL) entry  (It should fail)
    {
        // check precondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // do put
        EXPECT_EQ(EEXIST, pfc_hash_put(hash, NULL, NULL, 0));

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
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
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ(INVALID_PTR, v);  // v isn't changed
        }

        // do put
        EXPECT_EQ(0, pfc_hash_update(hash, NULL, NULL, 0));

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }

    // Do update registered (NULL, NULL) entry  (It should fail)
    {
        // check precondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }

        // do put
        EXPECT_EQ(0, pfc_hash_update(hash, NULL, NULL, 0));

        // check postcondition
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_hash_get(hash, NULL, &v, 0));
            EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
        }
    }


    // clean up
    pfc_hash_destroy(hash);
}


TEST(hash, gen_hashops_get)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 4; // keep more than '2' for injecting NULL cases
    int val_nsets = 4;           // keep more than '2' for injecting NULL cases
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // init hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // get for each unregistered entries
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ(ENOENT, pfc_hash_get(hash, keys[i], NULL, 0));

        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(ENOENT, pfc_hash_get(hash, keys[i], &v, 0));
        EXPECT_EQ(INVALID_PTR, v);
    }

    // do many operations including `get'
    test_in_out_entries(test_data, hash);

    // check test cases are passed
    EXPECT_TRUE(test_data->tp_flags.get_unreg_key);
    EXPECT_TRUE(test_data->tp_flags.get_existing_key);
    EXPECT_TRUE(test_data->tp_flags.get_null_key);
    EXPECT_TRUE(test_data->tp_flags.get_null_valuep);
    EXPECT_TRUE(test_data->tp_flags.get_null_key_valuep);

    // get all entries
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ(0, pfc_hash_get(hash, keys[i], NULL, 0));

        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(0, pfc_hash_get(hash, keys[i], &v, 0));
        EXPECT_EQ(test_data->vals_at_tail[i], v);
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, gen_hashops_get_equals_hashfunc_count)
{
    // init
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    // int val_nsets = 2;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // try to get for unregistered keys from empty list
    for (int i = 0; i < nents; ++i) {
        // get old equals / hashfunc count
#ifndef PARALLEL_TEST
        uint64_t old_equals_cnt = strint_get_count_equals();
#endif /* !PARALLEL_TEST */
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();

        // try to get
        pfc_cptr_t v;
        EXPECT_EQ(ENOENT, pfc_hash_get(hash, keys[i], &v, 0));

        // check equals / hashfunc count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_equals_cnt,       strint_get_count_equals());
        EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */
    }

    // put and get for each unregistered keys
    for (int i = 0; i < nents; ++i) {
        // put
        EXPECT_EQ(0, pfc_hash_put(hash, keys[i], vals[0][i], 0));
        skip_key_dtor[i] = PFC_TRUE;
        skip_val_dtor[0][i] = PFC_TRUE;

        // get old equals / hashfunc count
        uint64_t old_equals_cnt   = strint_get_count_equals();
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();

        // do get
        pfc_cptr_t v;
        EXPECT_EQ(0, pfc_hash_get(hash, keys[i], &v, 0));

        // check value
        EXPECT_EQ(vals[0][i], v);

        // check equals count
        EXPECT_LE(old_equals_cnt   + 1, strint_get_count_equals());

        // check hashfunc count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */
    }

    // get for registered keys
    for (int i = 0; i < nents; ++i) {
        // get old equals / hashfunc count
        uint64_t old_equals_cnt   = strint_get_count_equals();
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();

        // do get
        pfc_cptr_t v;
        EXPECT_EQ(0, pfc_hash_get(hash, keys[i], &v, 0));

        // check value
        EXPECT_EQ(vals[0][i], v);

        // check equals count
        EXPECT_LE(old_equals_cnt   + 1, strint_get_count_equals());

        // check hashfunc count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */
    }

    // get for registered keys wiht in check mode
    for (int i = 0; i < nents; ++i) {
        // get old equals / hashfunc count
        uint64_t old_equals_cnt   = strint_get_count_equals();
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();

        // do get with NULL valuep
        EXPECT_EQ(0, pfc_hash_get(hash, keys[i], NULL, 0));

        // check equals count
        EXPECT_LE(old_equals_cnt   + 1, strint_get_count_equals());

        // check hashfunc count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, gen_hashops_get_uint_val)
{
#ifdef PFC_LP64
    typedef uint64_t uint_val_t;
    const uint_val_t max_uint_val = UINT64_MAX;
    const uint_val_t check_val = 0xaaaaaaaaaaaaaaaa; // = 0b101010....101010
#else /* PFC_LP64 */
    typedef uint32_t uint_val_t;
    const uint_val_t max_uint_val = UINT32_MAX;
    const uint_val_t check_val = 0xaaaaaaaa; // = 0b101010....101010
#endif /* PFC_LP64 */

    // init
    int nbuckets = 31;
    int nents = nbuckets * 4;
    int val_nsets = 0;  // values are set later by this function,
    // now don't require common routine to create them
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    pfc_hash_t hash;
    pfc_hash_ops_t ops = *strint_kv_hash_ops;
    ops.value_dtor = NULL;
    int err = pfc_hash_create(&hash, &ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // create uint values
    uint_val_t uint_vals[nents];
    for (int i = 0; i < nents; ++i) {
        uint_vals[i] = max_uint_val / nents * i;
    }

    for (int i = 0; i < nents; ++i) {
        // put/get for each uint64_t or uint32_t values

        // try to get uint value as pointer with unregistered key,
        // and it should fail
        {
            uint_val_t get_vals[3] = { check_val, 0, check_val };
            pfc_cptr_t *get_valp = (pfc_cptr_t *)(uintptr_t)(&get_vals[1]);
            int get_err = pfc_hash_get(hash, keys[i], get_valp, 0);
            EXPECT_EQ(ENOENT, get_err);
            EXPECT_EQ(0U, get_vals[1]);
            EXPECT_EQ(check_val, get_vals[0]);
            EXPECT_EQ(check_val, get_vals[2]);
        }

        // put uint value as pointer
        {
            pfc_cptr_t put_val = (pfc_cptr_t)(uintptr_t)(uint_vals[i]);
            int put_err = pfc_hash_put(hash, keys[i], put_val, 0);
            EXPECT_EQ(0, put_err);
            skip_key_dtor[i] = PFC_TRUE;;
        }

        // get uint value as pointer
        {
            uint_val_t get_vals[3] = { check_val, 0, check_val };
            pfc_cptr_t *get_valp = (pfc_cptr_t *)(uintptr_t)(&get_vals[1]);
            int get_err = pfc_hash_get(hash, keys[i], get_valp, 0);
            EXPECT_EQ(0, get_err);
            EXPECT_EQ(uint_vals[i], get_vals[1]);
            EXPECT_EQ(check_val, get_vals[0]);
            EXPECT_EQ(check_val, get_vals[2]);
        }
    }

    // get all uint values as pointer
    for (int i = 0; i < nents; ++i) {
        uint_val_t get_vals[3] = { check_val, 0, check_val };
        pfc_cptr_t *get_valp = (pfc_cptr_t *)(uintptr_t)(&get_vals[1]);
        int get_err = pfc_hash_get(hash, keys[i], get_valp, 0);
        EXPECT_EQ(0, get_err);
        EXPECT_EQ(uint_vals[i], get_vals[1]);
        EXPECT_EQ(check_val, get_vals[0]);
        EXPECT_EQ(check_val, get_vals[2]);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, gen_hashops_remove)
{
    // init test data
    int nbuckets = 31;
    int nents = nbuckets * 4; // keep more than '2' for injecting NULL cases
    int val_nsets = 4;           // keep more than '2' for injecting NULL cases
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // init hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // try to remove for each unregistered entries
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ(ENOENT, pfc_hash_remove(hash, keys[i], 0));
    }

    // set entries into hash
    test_in_out_entries(test_data, hash);

    // remove all entries
    for (int i = 0; i < nents; ++i) {
        // check pre condition
        EXPECT_EQ(0, pfc_hash_get(hash, keys[i], NULL, 0));

        // do remvoe
        EXPECT_EQ(0, pfc_hash_remove(hash, keys[i], 0));

        // check post condition
        EXPECT_EQ(ENOENT, pfc_hash_get(hash, permanent_str_keys[i], NULL, 0));
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, gen_hashops_remove_equals_hashfunc_dtor_count)
{
    // init
    int nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    // int val_nsets = 2;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    // test_data_inject_null(test_data);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // create hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // try to remove for each unregistered keys
    for (int i = 0; i < nents; ++i) {
        // get old hashfunc count
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();
#ifndef PARALLEL_TEST
        uint64_t old_key_dtor_cnt = strint_get_count_key_dtor();
        uint64_t old_val_dtor_cnt = strint_get_count_val_dtor();
#endif /* !PARALLEL_TEST */

        // try to remove
        EXPECT_EQ(ENOENT, pfc_hash_remove(hash, keys[i], 0));

        // check hashfunc / key_dtor / val_dtor count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
        EXPECT_EQ(old_key_dtor_cnt,     strint_get_count_key_dtor());
        EXPECT_EQ(old_val_dtor_cnt,     strint_get_count_val_dtor());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */
    }

    // fill hash with keys / values
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ(0, pfc_hash_put(hash, keys[i], vals[0][i], 0));
        skip_key_dtor[i] = PFC_TRUE;
        skip_val_dtor[0][i] = PFC_TRUE;
    }

    // remove for each registered keys
    for (int i = 0; i < nents; ++i) {
        // get old hashfunc count
        uint64_t old_equals_cnt   = strint_get_count_equals();
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();
        uint64_t old_key_dtor_cnt = strint_get_count_key_dtor();
        uint64_t old_val_dtor_cnt = strint_get_count_val_dtor();

        // do remvoe
        EXPECT_EQ(0, pfc_hash_remove(hash, keys[i], 0));

        // check equals count
        EXPECT_LE(old_equals_cnt + 1, strint_get_count_equals());

        // check hashfunc / key_dtor / val_dtor count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
        EXPECT_EQ(old_key_dtor_cnt + 1, strint_get_count_key_dtor());
        EXPECT_EQ(old_val_dtor_cnt + 1, strint_get_count_val_dtor());
#else  /* !PARALLEL_TEST */
        EXPECT_LE(old_hashfunc_cnt + 1, strint_get_count_hashfunc());
        EXPECT_LE(old_key_dtor_cnt + 1, strint_get_count_key_dtor());
        EXPECT_LE(old_val_dtor_cnt + 1, strint_get_count_val_dtor());
#endif /* !PARALLEL_TEST */

        // check target addresses of key / val dtor
#ifndef PARALLEL_TEST
        EXPECT_EQ(keys[i],    strint_get_key_dtor_last_addr());
        EXPECT_EQ(vals[0][i], strint_get_val_dtor_last_addr());
#endif /* !PARALLEL_TEST */

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
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // init hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // get size and put for each entries
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ((size_t)i, pfc_hash_get_size(hash));
        EXPECT_EQ(0, pfc_hash_put(hash, keys[i], vals[0][i], 0));
        skip_key_dtor[i] = PFC_TRUE;
        skip_val_dtor[0][i] = PFC_TRUE;
    }

    // get size after putting all entries
    EXPECT_EQ((size_t)nents, pfc_hash_get_size(hash));

    // get size and remove for each entries
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ((size_t)(nents - i), pfc_hash_get_size(hash));
        EXPECT_EQ(0, pfc_hash_remove(hash, test_data->keys[i], 0));
    }

    // get size after removing all entries
    EXPECT_EQ((size_t)0, pfc_hash_get_size(hash));

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, gen_hashops_get_size_1)
{
    for (int nbuckets = 1;
         nbuckets < 500;
         ++nbuckets)
    {
        test_get_size(nbuckets, nbuckets * 2);
    }
}


TEST(hash, gen_hashops_get_size_2)
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
    int val_nsets = 1;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;

    // init hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // capacity right after creating
    EXPECT_EQ((size_t)nbuckets, pfc_hash_get_capacity(hash));

    size_t cur_cap = nbuckets;
    for (ssize_t *next_cap = new_caps; *next_cap > 0; ++next_cap) {
        EXPECT_EQ(cur_cap, pfc_hash_get_capacity(hash));

        // allocate test data
        test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data == NULL) abort();
        TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

        test_data_t *test_data2 = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data2 == NULL) abort();

        // many operations on hash
        test_in_out_entries(test_data, hash);

        // get size
        size_t hash_size = pfc_hash_get_size(hash);

        // get old hashfunc count
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();

        // get old key / value dtor count
#ifndef PARALLEL_TEST
        uint64_t old_key_dtor_cnt = strint_get_count_key_dtor();
        uint64_t old_val_dtor_cnt = strint_get_count_val_dtor();
#endif /* !PARALLEL_TEST */

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

        // check get_capacity
        EXPECT_EQ(cur_cap, pfc_hash_get_capacity(hash));

        // check hashfunc count
        if (!is_same_cap) {
#ifndef PARALLEL_TEST
            EXPECT_EQ(old_hashfunc_cnt + hash_size, strint_get_count_hashfunc());
#else  /* !PARALLEL_TEST */
            EXPECT_LE(old_hashfunc_cnt + hash_size, strint_get_count_hashfunc());
#endif /* !PARALLEL_TEST */
        }

        // get old key / value dtor count
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_key_dtor_cnt, strint_get_count_key_dtor());
        EXPECT_EQ(old_val_dtor_cnt, strint_get_count_val_dtor());
#endif /* !PARALLEL_TEST */

        // check key-value matching for each entries
        for (int i = 0; i < test_data->nents; ++i) {
            pfc_cptr_t v = NULL;
            EXPECT_EQ(0, pfc_hash_get(hash, keys[i], &v, 0));
            EXPECT_EQ(test_data->vals_at_tail[i], v);
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


TEST(hash, gen_hashops_get_set_capacity_1)
{
    ssize_t max_nbuckets = ((!is_under_heavy_load()) ? 40 : 30);
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


TEST(hash, gen_hashops_get_set_capacity_2)
{
    int nsplit = ((!is_under_heavy_load()) ? 4 : 3);
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


TEST(hash, gen_hashops_get_set_capacity_3)
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


TEST(hash, gen_hashops_set_capacity_error)
{
    // init hash
    int nbuckets = 31;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);
    EXPECT_EQ((size_t)nbuckets, pfc_hash_get_capacity(hash));

    EXPECT_NE(0, pfc_hash_set_capacity(hash, 0));
    EXPECT_NE(0, pfc_hash_set_capacity(hash, PFC_HASH_MAX_NBUCKETS+1));

    // clean up hash
    pfc_hash_destroy(hash);
}


TEST(hash, gen_hashops_report)
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
    size_t nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // init hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // report of empty hash
    pfc_hash_report(hash, report_fp);

    // put and report for each entries
    for (int i = 0; i < nents; ++i) {
        int put_err = pfc_hash_put(hash, keys[i], vals[0][i], 0);
        EXPECT_EQ(0, put_err);
        skip_key_dtor[i] = PFC_TRUE;
        skip_val_dtor[0][i] = PFC_TRUE;
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


TEST(hash, gen_hashops_iterator_get_next_put)
{
    // init test data
    size_t nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;
    test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    // init hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    // put an entry and hashiter operations for each entires
    for (int i = -1; i < test_data->nents; ++i) {
        if (i >= 0) {
            int put_err = pfc_hash_put(hash, keys[i], vals[0][i], 0);
            EXPECT_EQ(0, put_err);
            skip_key_dtor[i] = PFC_TRUE;
            skip_val_dtor[0][i] = PFC_TRUE;
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
        int max_iters = test_data->nents / 2;    // iter cut off threshold
        int nents_in_hash = i + 1;                  // current entries in hash
        for (int kv_bitmap = 0; kv_bitmap < 0x4; ++kv_bitmap) {
            pfc_cptr_t key, val;
            pfc_cptr_t *keyp = (kv_bitmap & 0x1) ? (&key) : NULL;
            pfc_cptr_t *valp = (kv_bitmap & 0x2) ? (&val) : NULL;

            // copy keys and values to verify
            pfc_cptr_t rest_keys[nents_in_hash];
            pfc_cptr_t rest_vals[nents_in_hash];
            if (keyp != NULL)
                memcpy(rest_keys, test_data->keys, sizeof(rest_keys));
            if (valp != NULL)
                memcpy(rest_vals, test_data->vals_at_head, sizeof(rest_vals));

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
                // and some entries should rest
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


TEST(hash, gen_hashops_iterator_next_err_einval)
{
    // init test data
    size_t nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 2;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;

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
        update_ops_t update_op = (update_ops_t)((n / 2) % UPDATE_OP_END_MARK);

        // allocate test data
        test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data == NULL) abort();
        TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

        test_data_t *test_data2 = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data2 == NULL) abort();

        int put_key_idx = 0;
        pfc_cptr_t put_key    = test_data2->keys[put_key_idx];
        pfc_cptr_t put_val    = test_data2->vals[0][put_key_idx];

        int update_key_idx = 1;
        pfc_cptr_t update_key = test_data2->keys[update_key_idx];
        pfc_cptr_t update_val = test_data2->vals[0][update_key_idx];

        test_data_t *test_data3 = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data3 == NULL) abort();

        // init hash
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_hash_create(&hash, hash_ops, nbuckets, 0));

        // set entries into hash
        test_in_out_entries(test_data, hash);

        if (update_op == UPDATE_OP_PUT) {
            // preparation for later `put' operation
            int err = pfc_hash_remove(hash, put_key, 0);
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
                EXPECT_EQ(0, pfc_hash_put(hash, put_key, put_val, 0));
                test_data2->skip_key_dtor[put_key_idx] = PFC_TRUE;
                test_data2->skip_val_dtor[0][put_key_idx] = PFC_TRUE;
                break;
            }
            case UPDATE_OP_UPDATE: {
                EXPECT_EQ(0, pfc_hash_update(hash, update_key, update_val, 0));
                test_data2->skip_key_dtor[update_key_idx] = PFC_TRUE;
                test_data2->skip_val_dtor[0][update_key_idx] = PFC_TRUE;
                break;
            }
            case UPDATE_OP_REMOVE: {
                pfc_cptr_t rm_key = test_data->keys[0];
                EXPECT_EQ(0, pfc_hash_remove(hash, rm_key, 0));
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
        test_data_free(test_data2);
        test_data_free(test_data3);
    }
}


TEST(hash, gen_hashops_iterator_next_err_ebadf)
{
    // init test data
    size_t nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;

    // destroy hash before getting iterator
    for (int n = 0; n < 2; ++n) {
        // allocate test data
        test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data == NULL) abort();

        // create hash
        pfc_hash_t hash;
        int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
        EXPECT_EQ(0, err);

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


TEST(hash, gen_hashops_iterator_remove)
{
    // init test data
    size_t nbuckets = 31;
    int nents = nbuckets * 2;
    int val_nsets = 1;
    const pfc_hash_ops_t *hash_ops = strint_kv_hash_ops;

    // init hash
    pfc_hash_t hash;
    int err = pfc_hash_create(&hash, hash_ops, nbuckets, 0);
    EXPECT_EQ(0, err);

    for (int n = 0; n < 3; ++n) {
        // allocate test data
        test_data_t *test_data = test_data_alloc(nents, val_nsets, hash_ops);
        if (test_data == NULL) abort();
        char **permanent_str_keys = test_data->permanent_str_keys;

        // set entries into hash
        test_in_out_entries(test_data, hash);

        // get iterator
        pfc_hashiter_t it = pfc_hashiter_get(hash);
        EXPECT_TRUE(it != NULL);

        {
            // get old key / value dtor count
#ifndef PARALLEL_TEST
            uint64_t old_key_dtor_cnt = strint_get_count_key_dtor();
            uint64_t old_val_dtor_cnt = strint_get_count_val_dtor();
#endif /* !PARALLEL_TEST */

            // pfc_hashiter_remove should fail right after getting hashiter
            EXPECT_EQ(ENOENT, pfc_hashiter_remove(it));

            // check key / value dtor count
#ifndef PARALLEL_TEST
            EXPECT_EQ(old_key_dtor_cnt, strint_get_count_key_dtor());
            EXPECT_EQ(old_val_dtor_cnt, strint_get_count_val_dtor());
#endif /* !PARALLEL_TEST */
        }

        // traverse and remove for each entries
        int rm_cnt = 0;
        pfc_cptr_t key;
        while (pfc_hashiter_next(it, &key, NULL) == 0) {
            int i; // index to access test_data->key / val
            for (i = 0; test_data->keys[i] != key; ++i) {
                if (i > nents) abort();
            }


            // get before removing
            EXPECT_EQ(0, pfc_hash_get(hash, key, NULL, 0));

            // remove
            {
                // get old key / value dtor count
                uint64_t old_key_dtor_cnt = strint_get_count_key_dtor();
                uint64_t old_val_dtor_cnt = strint_get_count_val_dtor();

                // do hashiter_remove
                EXPECT_EQ(0, pfc_hashiter_remove(it));

                // check key / value dtor count
#ifndef PARALLEL_TEST
                EXPECT_EQ(old_key_dtor_cnt + 1, strint_get_count_key_dtor());
                EXPECT_EQ(old_val_dtor_cnt + 1, strint_get_count_val_dtor());
#else  /* !PARALLEL_TEST */
                EXPECT_LE(old_key_dtor_cnt + 1, strint_get_count_key_dtor());
                EXPECT_LE(old_val_dtor_cnt + 1, strint_get_count_val_dtor());
#endif /* !PARALLEL_TEST */

                // check target addresses of key / val dtor
#ifndef PARALLEL_TEST
                EXPECT_EQ(test_data->keys[i],    strint_get_key_dtor_last_addr());
                EXPECT_EQ(test_data->vals[0][i], strint_get_val_dtor_last_addr());
#endif /* !PARALLEL_TEST */
            }

            // try to get after removing
            EXPECT_EQ(ENOENT, pfc_hash_get(hash, permanent_str_keys[i], NULL, 0));

            // remove one more (Of courese, it should fail)
            {
                // get old key / value dtor count
#ifndef PARALLEL_TEST
                uint64_t old_key_dtor_cnt = strint_get_count_key_dtor();
                uint64_t old_val_dtor_cnt = strint_get_count_val_dtor();
#endif /* !PARALLEL_TEST */

                // try to hashiter_remove for unregistered key
                EXPECT_EQ(ENOENT, pfc_hashiter_remove(it));

                // check key / value dtor count
#ifndef PARALLEL_TEST
                EXPECT_EQ(old_key_dtor_cnt, strint_get_count_key_dtor());
                EXPECT_EQ(old_val_dtor_cnt, strint_get_count_val_dtor());
#endif /* !PARALLEL_TEST */
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
            if (n == 0 && empty_hash) {
                // get before putting
                {
                    pfc_cptr_t get_val = NULL;
                    int get_err = pfc_hash_get(hash, keys[i], &get_val, 0);
                    EXPECT_EQ(ENOENT, get_err);
                    EXPECT_TRUE(get_val == NULL);

                    // set TP flags
                    SET_FLAG(test_data, get_unreg_key);
                }

                input_op_t input_op;
                if (input_op_for_1st) {
                    input_op = (i % 2) ? INPUT_OP_PUT : INPUT_OP_UPDATE;
                } else {
                    input_op = input_op_for_1st;
                }

                if (input_op == INPUT_OP_PUT) {
                    // put for unregistered key
                    int put_err = pfc_hash_put(hash, keys[i], vals[n][i], 0);
                    EXPECT_EQ(0, put_err);
                    skip_key_dtor[i] = PFC_TRUE;
                    skip_val_dtor[n][i] = PFC_TRUE;
                    test_data->tp_flags.put_unreg_key = true;

                    // set TP flags
                    if (keys[i] == NULL && vals[n][i] == NULL) {
                        SET_FLAG(test_data, put_null_key_value);
                    } else if (keys[i] == NULL) {
                        SET_FLAG(test_data, put_null_key);
                    } else if (vals[n][i] == NULL) {
                        SET_FLAG(test_data, put_null_value);
                    }
                } else {
                    // update for unregistered key
                    int update_err = pfc_hash_update(hash, keys[i], vals[n][i], 0);
                    EXPECT_EQ(0, update_err);
                    skip_key_dtor[i] = PFC_TRUE;
                    skip_val_dtor[n][i] = PFC_TRUE;
                    test_data->tp_flags.update_unreg_key = true;

                    // set TP flags
                    if (keys[i] == NULL && vals[n][i] == NULL) {
                        SET_FLAG(test_data, update_null_key_value);
                    } else if (keys[i] == NULL) {
                        SET_FLAG(test_data, update_null_key);
                    } else if (vals[n][i] == NULL) {
                        SET_FLAG(test_data, update_null_value);
                    }
                }
            } else {
                bool existing = (pfc_hash_get(hash, keys[i], NULL, 0) == 0);

                // update for existing entry
                int update_err = pfc_hash_update(hash, keys[i], vals[n][i], 0);
                EXPECT_EQ(0, update_err);
                skip_key_dtor[i] = PFC_TRUE;
                skip_val_dtor[n][i] = PFC_TRUE;

                // set TP flags of key status
                if (existing)  {
                    SET_FLAG(test_data, update_existing_key);
                } else {
                    SET_FLAG(test_data, update_unreg_key);
                }

                // set TP flags of NULL case
                if (keys[i] == NULL && vals[n][i] == NULL) {
                    SET_FLAG(test_data, update_null_key_value);
                } else if (keys[i] == NULL) {
                    SET_FLAG(test_data, update_null_key);
                } else if (vals[n][i] == NULL) {
                    SET_FLAG(test_data, update_null_value);
                }
            }

            // get for existing entry
            {
                pfc_cptr_t get_val = NULL;
                int get_err = pfc_hash_get(hash, keys[i], &get_val, 0);
                EXPECT_EQ(0, get_err);
                EXPECT_TRUE(get_val == vals[n][i]);
                SET_FLAG(test_data, get_existing_key);
                if (keys[i] == NULL) {
                    SET_FLAG(test_data, get_null_key);
                }
            }

            // get for existing entry (check only)
            {
                int get_err = pfc_hash_get(hash, keys[i], NULL, 0);
                EXPECT_EQ(0, get_err);
                SET_FLAG(test_data, get_existing_key_check);
                if (keys[i] == NULL) {
                    SET_FLAG(test_data, get_null_key_valuep);
                } else {
                    SET_FLAG(test_data, get_null_valuep);
                }
            }

            // put for existing entry
            {
                int put_err = pfc_hash_put(hash, keys[i], vals[n][i], 0);
                EXPECT_EQ(EEXIST, put_err);
                SET_FLAG(test_data, put_existing_key);
            }
        }

        // get all entries
        for (int i = 0; i < nents; ++i) {
            pfc_cptr_t get_val = NULL;
            int get_err = pfc_hash_get(hash, keys[i], &get_val, 0);
            EXPECT_EQ(0, get_err);
            EXPECT_TRUE(get_val == vals[n][i]);
        }
    }
}
