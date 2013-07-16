/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * basic tests for u64hash
 */

#include <pfc/hash.h>
#include <string.h>
#include "pseudo_rand.hh"
#include "test_hash.hh"


/*
 * Constants
 */
#define INVALID_PTR     ((pfc_cptr_t)-123)
#define INVALID_U64_VAL ((uint64_t)-1)

/*
 * Type, prototypes and macros for test data
 */
typedef struct {
    int nentries;
    int val_nsets;
    uint64_t *key;
    char ***val;
    char **val_at_first;
    char **val_at_last;

    // flags to check TP itself
    struct {
        bool put_unreg_key;
        bool put_existing_key;
        bool put_null_value;

        bool update_unreg_key;
        bool update_existing_key;
        bool update_null_value;

        bool get_unreg_key;
        bool get_existing_key;
        bool get_existing_key_check;
        bool get_null_valuep;

        bool remove_unreg_key;
        bool remove_existing_key;
    } tp_flags;
} test_data_t;

#define SET_FLAG(test_data, flag)               \
    ((test_data)->tp_flags.flag = true)


typedef enum {
    INPUT_OP_PUT,
    INPUT_OP_UPDATE,
    INPUT_OP_MIX,
} input_op_t;

static test_data_t *test_data_alloc(int nentries, int val_nsets);
static void test_data_free(test_data_t *test_data);
static void test_data_inject_null(test_data_t *test_data);
static void test_in_out_entries(test_data_t *test_data,
                                pfc_hash_t hash,
                                bool empty_hash = true,
                                input_op_t input_op_for_1st = INPUT_OP_MIX,
                                int remove_all_foreach = 3);

/*
 * Test cases
 */

TEST(hash, u64_create_zero_bucket)
{
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, 0, 0);
    EXPECT_NE(0, err);
}


TEST(hash, u64_create_over_buckets)
{
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, PFC_HASH_MAX_NBUCKETS + 1, 0);
    EXPECT_NE(0, err);
}


TEST(hash, u64_create_good_buckets_1)
{
    for (size_t nbuckets = 1;
         nbuckets <= 100;
         ++nbuckets)
    {
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0));
        pfc_hash_destroy(hash);
    }

}


TEST(hash, u64_create_good_buckets_2)
{
    for (size_t nbuckets = PFC_HASH_MAX_NBUCKETS - 5;
         nbuckets <= PFC_HASH_MAX_NBUCKETS;
         ++nbuckets)
    {
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0));
        pfc_hash_destroy(hash);
    }

}


TEST(hash, u64_create_good_buckets_3)
{
    for (size_t nbuckets = 1;
         nbuckets <= PFC_HASH_MAX_NBUCKETS;
         nbuckets += PFC_HASH_MAX_NBUCKETS / 10)
    {
        pfc_hash_t hash;
        EXPECT_EQ(0, pfc_u64hash_create(&hash, NULL, nbuckets, 0));
        pfc_hash_destroy(hash);
    }
}


TEST(hash, u64_clear)
{
    // init
    int nbuckets = 31;
    int nentries = nbuckets * 2;
    int val_nsets = 10;
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();

    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
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
        EXPECT_EQ(nentries, clear_num);

        uint32_t capa = pfc_hash_get_capacity(hash);
        EXPECT_EQ((uint32_t)nbuckets, capa);

        uint32_t size = pfc_hash_get_size(hash);
        EXPECT_EQ(0U, size);

        for (int i = 0; i < nentries; i++) {
            pfc_cptr_t val = NULL;
            int get_err = pfc_u64hash_get(hash, test_data->key[i], &val);
            EXPECT_EQ(ENOENT, get_err);
            int get_err2 = pfc_u64hash_get(hash, test_data->key[i], NULL);
            EXPECT_EQ(ENOENT, get_err2);
        }

        test_in_out_entries(test_data, hash);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, u64_put_update)
{
    // init test data
    int nbuckets = 31;
    int nentries = nbuckets * 4; // keep more than '2' for injecting NULL cases
    int val_nsets = 4;           // keep more than '2' for injecting NULL cases
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();
    test_data_inject_null(test_data);

    for (int i = 0; i < 4; ++i) {
        // init hash
        pfc_hash_t hash;
        int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
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
            EXPECT_TRUE(test_data->tp_flags.put_null_value);
        } else {
            // do many operations including `update'
            test_in_out_entries(test_data, hash, true, INPUT_OP_UPDATE);

            // check test cases are passed
            EXPECT_TRUE(test_data->tp_flags.update_unreg_key);
            EXPECT_TRUE(test_data->tp_flags.update_existing_key);
            EXPECT_TRUE(test_data->tp_flags.update_null_value);
        }

        // clean up hash
        pfc_hash_destroy(hash);
    }

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, u64_put_update2)
{
    // This test complements `TEST(hash, u64_put_update)' which
    // doesn't sufficiently test the case of (key, value) =
    // (NULL, NULL).

    // test cond
    int nbuckets = 31;

    // init hash
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);


    // Do put unregistered (NULL, NULL) entry  (It should success)
    {
        pfc_cptr_t v;

        // check precondition
        v = INVALID_PTR;
        EXPECT_EQ(ENOENT, pfc_u64hash_get(hash, 0, &v));
        EXPECT_EQ(INVALID_PTR, v);  // v isn't changed

        // do put
        EXPECT_EQ(0, pfc_u64hash_put(hash, 0, NULL, 0));

        // check postcondition
        v = INVALID_PTR;
        EXPECT_EQ(0, pfc_u64hash_get(hash, 0, &v));
        EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
    }

    // Do put egistered (NULL, NULL) entry  (It should fail)
    {
        pfc_cptr_t v;

        // check precondition
        v = INVALID_PTR;
        EXPECT_EQ(0, pfc_u64hash_get(hash, 0, &v));
        EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed

        // do put
        EXPECT_EQ(EEXIST, pfc_u64hash_put(hash, 0, NULL, 0));

        // check postcondition
        v = INVALID_PTR;
        EXPECT_EQ(0, pfc_u64hash_get(hash, 0, &v));
        EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
    }


    // reset hash
    EXPECT_EQ(1U, pfc_hash_clear(hash));


    // Do update unregistered (NULL, NULL) entry  (It should success)
    {
        pfc_cptr_t v;

        // check precondition
        v = INVALID_PTR;
        EXPECT_EQ(ENOENT, pfc_u64hash_get(hash, 0, &v));
        EXPECT_EQ(INVALID_PTR, v);  // v isn't changed

        // do put
        EXPECT_EQ(0, pfc_u64hash_update(hash, 0, NULL, 0));

        // check postcondition
        v = INVALID_PTR;
        EXPECT_EQ(0, pfc_u64hash_get(hash, 0, &v));
        EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
    }

    // Do update egistered (NULL, NULL) entry  (It should fail)
    {
        pfc_cptr_t v;

        // check precondition
        v = INVALID_PTR;
        EXPECT_EQ(0, pfc_u64hash_get(hash, 0, &v));
        EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed

        // do put
        EXPECT_EQ(0, pfc_u64hash_update(hash, 0, NULL, 0));

        // check postcondition
        v = INVALID_PTR;
        EXPECT_EQ(0, pfc_u64hash_get(hash, 0, &v));
        EXPECT_EQ((pfc_cptr_t)NULL, v);  // v is changed
    }


    // clean up
    pfc_hash_destroy(hash);
}



TEST(hash, u64_get)
{
    // init test data
    int nbuckets = 31;
    int nentries = nbuckets * 4; // keep more than '2' for injecting NULL cases
    int val_nsets = 4;           // keep more than '2' for injecting NULL cases
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();
    test_data_inject_null(test_data);

    // init hash
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);

    // get for each unregistered entries
    for (int i = 0; i < nentries; ++i) {
        uint64_t key = test_data->key[i];
        {
            int get_err = pfc_u64hash_get(hash, key, NULL);
            EXPECT_EQ(ENOENT, get_err);
        }
        {
            pfc_cptr_t get_val = NULL;
            int get_err = pfc_u64hash_get(hash, key, &get_val);
            EXPECT_EQ(ENOENT, get_err);
            EXPECT_TRUE(get_val == NULL);
        }
    }

    // do many operations including `get'
    test_in_out_entries(test_data, hash);

    // check test cases are passed
    EXPECT_TRUE(test_data->tp_flags.get_unreg_key);
    EXPECT_TRUE(test_data->tp_flags.get_existing_key);
    EXPECT_TRUE(test_data->tp_flags.get_null_valuep);

    // get all entries
    for (int i = 0; i < nentries; ++i) {
        uint64_t key = test_data->key[i];
        {
            int get_err = pfc_u64hash_get(hash, key, NULL);
            EXPECT_EQ(0, get_err);
        }
        {
            pfc_cptr_t get_val = NULL;
            int get_err = pfc_u64hash_get(hash, key, &get_val);
            EXPECT_EQ(0, get_err);
            EXPECT_TRUE(get_val == test_data->val[val_nsets-1][i]);
        }
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, u64_get_uint_val)
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
    int nentries = nbuckets * 4;
    int val_nsets = 0;  // values are set later by this function,
    // now don't require common routine to create them
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);

    // create uint values
    uint_val_t uint_val[nentries];
    for (int i = 0; i < nentries; ++i) {
        uint_val[i] = max_uint_val / nentries * i;
    }

    for (int i = 0; i < nentries; ++i) {
        // put/get for each uint64_t or uint32_t values
        uint64_t key = test_data->key[i];

        // try to get uint value as pointer with unregistered key,
        // and it should fail
        {
            uint_val_t get_val[3] = { check_val, 0, check_val };
            pfc_cptr_t *get_valp = (pfc_cptr_t *)(uintptr_t)(&get_val[1]);
            int get_err = pfc_u64hash_get(hash, key, get_valp);
            EXPECT_EQ(ENOENT, get_err);
            EXPECT_EQ(0U, get_val[1]);
            EXPECT_EQ(check_val, get_val[0]);
            EXPECT_EQ(check_val, get_val[2]);
        }

        // put uint value as pointer
        {
            pfc_cptr_t put_val = (pfc_cptr_t)(uintptr_t)(uint_val[i]);
            int put_err = pfc_u64hash_put(hash, key, put_val, 0);
            EXPECT_EQ(0, put_err);
        }

        // get uint value as pointer
        {
            uint_val_t get_val[3] = { check_val, 0, check_val };
            pfc_cptr_t *get_valp = (pfc_cptr_t *)(uintptr_t)(&get_val[1]);
            int get_err = pfc_u64hash_get(hash, key, get_valp);
            EXPECT_EQ(0, get_err);
            EXPECT_EQ(uint_val[i], get_val[1]);
            EXPECT_EQ(check_val, get_val[0]);
            EXPECT_EQ(check_val, get_val[2]);
        }
    }

    // get all uint values as pointer
    for (int i = 0; i < nentries; ++i) {
        uint_val_t get_val[3] = { check_val, 0, check_val };
        pfc_cptr_t *get_valp = (pfc_cptr_t *)(uintptr_t)(&get_val[1]);
        int get_err = pfc_u64hash_get(hash, test_data->key[i], get_valp);
        EXPECT_EQ(0, get_err);
        EXPECT_EQ(uint_val[i], get_val[1]);
        EXPECT_EQ(check_val, get_val[0]);
        EXPECT_EQ(check_val, get_val[2]);
    }

    // clean up
    pfc_hash_destroy(hash);
    test_data_free(test_data);
}


TEST(hash, u64_remove)
{
    // init test data
    int nbuckets = 31;
    int nentries = nbuckets * 4; // keep more than '2' for injecting NULL cases
    int val_nsets = 4;           // keep more than '2' for injecting NULL cases
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();
    test_data_inject_null(test_data);

    // init hash
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);

    // remove for each unregistered entries
    for (int i = 0; i < nentries; ++i) {
        uint64_t key = test_data->key[i];
        {
            int remove_err = pfc_u64hash_remove(hash, key);
            EXPECT_EQ(ENOENT, remove_err);
        }
    }

    // set entries into hash
    test_in_out_entries(test_data, hash);

    // remove all entries
    for (int i = 0; i < nentries; ++i) {
        uint64_t key = test_data->key[i];
        {
            int get_err1 = pfc_u64hash_get(hash, key, NULL);
            EXPECT_EQ(0, get_err1);

            int remove_err = pfc_u64hash_remove(hash, key);
            EXPECT_EQ(0, remove_err);

            int get_err2 = pfc_u64hash_get(hash, key, NULL);
            EXPECT_EQ(ENOENT, get_err2);
        }
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


static void
test_get_size (int nbuckets, int nentries)
{
    // init test data
    int val_nsets = 1;
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);

    // init hash
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);

    // get size and put for each entries
    for (int i = 0; i < nentries; ++i) {
        size_t size = pfc_hash_get_size(hash);
        EXPECT_EQ((size_t)i, size);

        int put_err = pfc_u64hash_put(hash, test_data->key[i],
                                      test_data->val[0][i], 0);
        EXPECT_EQ(0, put_err);
    }

    // get size after putting all entries
    {
        size_t size = pfc_hash_get_size(hash);
        EXPECT_EQ((size_t)nentries, size);
    }

    // get size and remove for each entries
    for (int i = 0; i < nentries; ++i) {
        size_t size = pfc_hash_get_size(hash);
        EXPECT_EQ((size_t)(nentries - i), size);

        int remove_err = pfc_u64hash_remove(hash, test_data->key[i]);
        EXPECT_EQ(0, remove_err);
    }

    // get size after removing all entries
    {
        size_t size = pfc_hash_get_size(hash);
        EXPECT_EQ((size_t)0, size);
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, u64_get_size_1)
{
    for (int nbuckets = 1;
         nbuckets < 500;
         ++nbuckets)
    {
        test_get_size(nbuckets, nbuckets * 2);
    }
}


TEST(hash, u64_get_size_2)
{
    for (int nbuckets = 500;
         nbuckets < (int)PFC_HASH_MAX_NBUCKETS;
         nbuckets += 1000000)
    {
        test_get_size(nbuckets, 1000);
    }
}


static void
test_get_set_capacity (int nbuckets, int nentries, ssize_t *new_caps)
{
    // init test data
    int val_nsets = 1;
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);

    // init hash
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);

    // capacity right after creating
    EXPECT_EQ((size_t)nbuckets, pfc_hash_get_capacity(hash));

    size_t cur_cap = nbuckets;
    for (ssize_t *next_cap = new_caps; *next_cap > 0; ++next_cap) {
        EXPECT_EQ(cur_cap, pfc_hash_get_capacity(hash));

        // many operations on hash
        test_in_out_entries(test_data, hash, false /* not empty */ );

        // change capacity
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
        EXPECT_EQ(cur_cap, pfc_hash_get_capacity(hash));

        // check key-value matching for each entries
        for (int i = 0; i < test_data->nentries; ++i) {
            pfc_cptr_t get_val = NULL;
            int get_err = pfc_u64hash_get(hash, test_data->key[i], &get_val);
            EXPECT_EQ(0, get_err);
            EXPECT_EQ(test_data->val_at_last[i], get_val);
        }

        // many operations on hash
        test_in_out_entries(test_data, hash, false /* not empty */ );
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, u64_get_set_capacity_1)
{
    ssize_t max_nbuckets = ((!is_under_heavy_load()) ? 100 : 30);
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
        int nentries = (nbuckets < 500) ? (nbuckets * 2) : 1000;
        test_get_set_capacity(nbuckets, nentries, new_caps);
    }
}


TEST(hash, u64_get_set_capacity_2)
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


TEST(hash, u64_get_set_capacity_3)
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


TEST(hash, u64_set_capacity_error)
{
    // init hash
    int nbuckets = 31;
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);
    EXPECT_EQ((size_t)nbuckets, pfc_hash_get_capacity(hash));

    EXPECT_NE(0, pfc_hash_set_capacity(hash, 0));
    EXPECT_NE(0, pfc_hash_set_capacity(hash, PFC_HASH_MAX_NBUCKETS+1));

    // clean up hash
    pfc_hash_destroy(hash);
}


TEST(hash, u64_report)
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
    int nentries = nbuckets * 2;
    int val_nsets = 1;
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();

    // init hash
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);

    // report of empty hash
    pfc_hash_report(hash, report_fp);

    // put and report for each entries
    for (int i = 0; i < test_data->nentries; ++i) {
        int put_err = pfc_u64hash_put(hash, test_data->key[i],
                                      test_data->val_at_last[i], 0);
        EXPECT_EQ(0, put_err);
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


TEST(hash, u64_iterator_get_next_put)
{
    // init test data
    size_t nbuckets = 31;
    int nentries = nbuckets * 2;
    int val_nsets = 1;
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();

    // init hash
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);

    // put an entry and hashiter operations for each entires
    for (int i = -1; i < test_data->nentries; ++i) {
        if (i >= 0) {
            int put_err = pfc_u64hash_put(hash, test_data->key[i],
                                          test_data->val_at_first[i], 0);
            EXPECT_EQ(0, put_err);
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
        int max_iters = test_data->nentries / 2;    // iter cut off threshold
        int nents_in_hash = i + 1;                  // current entries in hash
        for (int kv_bitmap = 0; kv_bitmap < 0x4; ++kv_bitmap) {
            uint64_t key;
            pfc_cptr_t val;
            uint64_t *keyp = (kv_bitmap & 0x1) ? (&key) : NULL;
            pfc_cptr_t *valp = (kv_bitmap & 0x2) ? (&val) : NULL;

            // copy keys and values to verify
            uint64_t rest_keys[nents_in_hash];
            pfc_cptr_t rest_vals[nents_in_hash];
            if (keyp != NULL)
                memcpy(rest_keys, test_data->key, sizeof(rest_keys));
            if (valp != NULL)
                memcpy(rest_vals, test_data->val_at_first, sizeof(rest_vals));

            // start of iteration
            pfc_hashiter_t it = pfc_hashiter_get(hash);
            EXPECT_TRUE(it != NULL);
            int n_iters = 0;
            int iter_next_err;
            while ((iter_next_err = pfc_hashiter_uint64_next(it, keyp, valp))
                   == 0)
            {
                int kpos = -1;
                if (keyp != NULL) {
                    for (int j = 0; j < nents_in_hash; ++j) {
                        if (rest_keys[j] == key) {
                            kpos = j;
                            break;
                        }
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

                if (kpos >= 0) rest_keys[kpos] = INVALID_U64_VAL;
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
                        EXPECT_TRUE(rest_keys[j] == INVALID_U64_VAL);
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


TEST(hash, u64_iterator_next_err_einval)
{
    // init test data
    size_t nbuckets = 31;
    int nentries = nbuckets * 2;
    int val_nsets = 2;
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();

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

        // init hash
        pfc_hash_t hash;
        int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
        EXPECT_EQ(0, err);

        // set entries into hash
        test_in_out_entries(test_data, hash);

        uint64_t put_key = INVALID_U64_VAL;
        pfc_cptr_t put_val = NULL;
        if (update_op == UPDATE_OP_PUT) {
            // preparation for later `put' operation
            put_key = test_data->key[0];
            put_val = test_data->val_at_first[0];
            EXPECT_EQ(0, pfc_u64hash_remove(hash, put_key));
        }

        // get iteration
        pfc_hashiter_t it = pfc_hashiter_get(hash);
        EXPECT_TRUE(it != NULL);

        if (call_next_before_update) {
            // once call pfc_hashiter_uint64_next() before update
            uint64_t key;
            pfc_cptr_t val;
            EXPECT_EQ(0, pfc_hashiter_uint64_next(it, &key, &val));
        }

        // update hash table before pfc_hashiter_uint64_next()
        switch (update_op) {
            case UPDATE_OP_PUT: {
                EXPECT_EQ(0, pfc_u64hash_put(hash, put_key, put_val, 0));
                break;
            }
            case UPDATE_OP_UPDATE: {
                uint64_t update_key = test_data->key[0];
                pfc_cptr_t update_val = test_data->val_at_first[0];
                EXPECT_EQ(0, pfc_u64hash_update(hash, update_key, update_val, 0));
                break;
            }
            case UPDATE_OP_REMOVE: {
                uint64_t rm_key = test_data->key[0];
                EXPECT_EQ(0, pfc_u64hash_remove(hash, rm_key));
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
                test_in_out_entries(test_data, hash, false /* not emtpy */);
                break;
            }
            case UPDATE_OP_END_MARK:
                abort();
                break;
        }

        // pfc_hashiter_uint64_next() should return EINVAL after update
        {
            uint64_t key;
            pfc_cptr_t val;
            EXPECT_EQ(EINVAL, pfc_hashiter_uint64_next(it, &key, &val));
        }

        // clean up hash
        pfc_hash_destroy(hash);
    }

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, u64_iterator_next_err_ebadf)
{
    // init test data
    size_t nbuckets = 31;
    int nentries = nbuckets * 2;
    int val_nsets = 1;
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();

    // destroy hash before getting iterator
    for (int n = 0; n < 2; ++n) {
        pfc_hash_t hash;
        int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
        EXPECT_EQ(0, err);

        // set entries into hash
        test_in_out_entries(test_data, hash);

        pfc_hashiter_t it = pfc_hashiter_get(hash);
        EXPECT_TRUE(it != NULL);

        if (n % 2) {
            // once call pfc_hashiter_uint64_next() before destruction
            uint64_t key;
            pfc_cptr_t val;
            EXPECT_EQ(0, pfc_hashiter_uint64_next(it, &key, &val));
        }

        pfc_hash_destroy(hash);

        {
            // pfc_hashiter_uint64_next() should return EBADF after destruction
            uint64_t key;
            pfc_cptr_t val;
            EXPECT_EQ(EBADF, pfc_hashiter_uint64_next(it, &key, &val));
        }
    }

    // clean up test data
    test_data_free(test_data);
}


TEST(hash, u64_iterator_remove)
{
    // init test data
    size_t nbuckets = 31;
    int nentries = nbuckets * 2;
    int val_nsets = 1;
    test_data_t *test_data = test_data_alloc(nentries, val_nsets);
    if (test_data == NULL) abort();

    // init hash
    pfc_hash_t hash;
    int err = pfc_u64hash_create(&hash, NULL, nbuckets, 0);
    EXPECT_EQ(0, err);

    for (int n = 0; n < 3; ++n) {
        // set entries into hash
        test_in_out_entries(test_data, hash);

        // get iterator
        pfc_hashiter_t it = pfc_hashiter_get(hash);
        EXPECT_TRUE(it != NULL);

        // pfc_hashiter_remove should fail right after getting hashiter
        EXPECT_EQ(ENOENT, pfc_hashiter_remove(it));

        // traverse and remove for each entries
        int rm_cnt = 0;
        uint64_t key;
        while (pfc_hashiter_uint64_next(it, &key, NULL) == 0) {
            EXPECT_EQ(0,      pfc_u64hash_get(hash, key, NULL));
            EXPECT_EQ(0,      pfc_hashiter_remove(it));
            EXPECT_EQ(ENOENT, pfc_u64hash_get(hash, key, NULL));
            EXPECT_EQ(ENOENT, pfc_hashiter_remove(it));

            ++rm_cnt;
            EXPECT_EQ((size_t)(test_data->nentries - rm_cnt),
                      pfc_hash_get_size(hash));
        }
        EXPECT_EQ(0U, pfc_hash_get_size(hash));
    }

    // clean up hash
    pfc_hash_destroy(hash);

    // clean up test data
    test_data_free(test_data);
}



/*
 * Operations on test data
 */
static test_data_t *
test_data_alloc (int nentries, int val_nsets)
{
    prand_generator_t prand_gen = prand_create(HASH_RANDOM_SEED);

    PFC_ASSERT(val_nsets >= 0);
    PFC_ASSERT(nentries > 0);
    test_data_t *test_data = (test_data_t *)calloc(1, sizeof(test_data_t));

    // allocate a set of keys
    uint64_t *key = (uint64_t *)calloc(nentries, sizeof(uint64_t));
    PFC_ASSERT(key != NULL);
    for (int i = 0; i < nentries; ++i) {
      retry:
        key[i] = prand_get_long(prand_gen);
        for (int j = 0; j < i; ++j) {
            if (key[i] == key[j]) {
                goto retry;
            }
        }
    }

    // allocate sets of values
    char ***val = NULL;
    if (val_nsets > 0) {
        val = (char ***)calloc(val_nsets, sizeof(char **));
        PFC_ASSERT(val != NULL);
        for (int n = 0; n < val_nsets; ++n) {
            val[n] = (char **)calloc(nentries, sizeof(char *));
            for (int i = 0; i < nentries; ++i) {
                const int maxlen = 100;
                val[n][i] = (char *)calloc(maxlen, sizeof(char));
                PFC_ASSERT(val[n][i] != NULL);
                if (n == 0) {
                    snprintf(val[n][i], maxlen,
                             "value to put with key = %"PFC_PFMT_u64,
                             key[i]);
                } else {
                    snprintf(val[n][i], maxlen,
                             "value to update/%d with key = %"PFC_PFMT_u64,
                             n, key[i]);
                }
            }
        }
    }

    // store members
    test_data->nentries = nentries;
    test_data->val_nsets = val_nsets;
    test_data->key = key;
    test_data->val = val;
    test_data->val_at_first = (val == NULL) ? NULL : val[0];
    test_data->val_at_last  = (val == NULL) ? NULL : val[val_nsets-1];

    // clean up
    prand_destroy(prand_gen);

    return test_data;
}


static void
test_data_free (test_data_t *test_data)
{
    int nentries  = test_data->nentries;
    int val_nsets = test_data->val_nsets;
    uint64_t *key = test_data->key;
    char ***val   = test_data->val;

    // release values
    for (int n = 0; n < val_nsets; n++) {
        for (int i = 0; i < nentries; ++i) {
            free(val[n][i]);
        }
        free(val[n]);
    }
    free(val);

    // release keys
    free(key);

    // release top
    free(test_data);
}


static void
test_data_inject_null (test_data_t *test_data)
{
    // keep `nentries and' `val_nsets' more than '2'
    if (test_data->nentries < 2) abort();
    if (test_data->val_nsets < 2) abort();

    // inject NULL value cases
    for (int i = 0; i < test_data->nentries; ++i) {
        for (int n = 0; n < test_data->val_nsets; ++n) {
            if ((n % 2 + i % 2) == 1) {
                if (test_data->val[n][i] != NULL) {
                    free(test_data->val[n][i]);
                }
                test_data->val[n][i] = NULL;
            }
        }
    }
}


static void
test_in_out_entries (test_data_t *test_data,
                     pfc_hash_t hash,
                     bool empty_hash,
                     input_op_t input_op_for_1st,
                     int remove_all_foreach)
{
    int nentries  = test_data->nentries;
    int val_nsets = test_data->val_nsets;
    uint64_t *key = test_data->key;
    char ***val   = test_data->val;

    for (int n = 0; n < val_nsets; ++n) {
        // put or update for each entries
        for (int i = 0; i < nentries; ++i) {
            if (n == 0 && empty_hash) {
                // get before putting
                {
                    pfc_cptr_t get_val = NULL;
                    int get_err = pfc_u64hash_get(hash, key[i], &get_val);
                    EXPECT_EQ(ENOENT, get_err);
                    EXPECT_TRUE(get_val == NULL);

                    // set TP flags
                    SET_FLAG(test_data, get_unreg_key);
                }

                input_op_t input_op;
                if (input_op_for_1st) {
                    input_op = (i % 2) ? INPUT_OP_PUT :  INPUT_OP_UPDATE;
                } else {
                    input_op = input_op_for_1st;
                }

                if (input_op == INPUT_OP_PUT) {
                    // put for unregistered key
                    int put_err = pfc_u64hash_put(hash, key[i],
                                                  val[n][i], 0);
                    EXPECT_EQ(0, put_err);
                    test_data->tp_flags.put_unreg_key = true;

                    // set TP flags
                    if (val[n][i] == NULL) {
                        SET_FLAG(test_data, put_null_value);
                    }
                } else {
                    // update for unregistered key
                    int update_err = pfc_u64hash_update(hash, key[i],
                                                        val[n][i], 0);
                    EXPECT_EQ(0, update_err);
                    test_data->tp_flags.update_unreg_key = true;

                    // set TP flags
                    if (val[n][i] == NULL) {
                        SET_FLAG(test_data, update_null_value);
                    }
                }
            } else {
                bool existing = (pfc_u64hash_get(hash, key[i], NULL) == 0);

                // update for existing entry
                int update_err = pfc_u64hash_update(hash, key[i], val[n][i], 0);
                EXPECT_EQ(0, update_err);

                // set TP flags of key status
                if (existing)  {
                    SET_FLAG(test_data, update_existing_key);
                } else {
                    SET_FLAG(test_data, update_unreg_key);
                }

                // set TP flags of NULL case
                if (val[n][i] == NULL) {
                    SET_FLAG(test_data, update_null_value);
                }
            }

            // get for existing entry
            {
                pfc_cptr_t get_val = NULL;
                int get_err = pfc_u64hash_get(hash, key[i], &get_val);
                EXPECT_EQ(0, get_err);
                EXPECT_TRUE(get_val == val[n][i]);
                SET_FLAG(test_data, get_existing_key);
            }

            // get for existing entry (check only)
            {
                int get_err = pfc_u64hash_get(hash, key[i], NULL);
                EXPECT_EQ(0, get_err);
                SET_FLAG(test_data, get_existing_key_check);
                SET_FLAG(test_data, get_null_valuep);
            }

            // put for existing entry
            {
                int put_err = pfc_u64hash_put(hash, key[i], val[n][i], 0);
                EXPECT_EQ(EEXIST, put_err);
                SET_FLAG(test_data, put_existing_key);
            }
        }

        // get all entries
        for (int i = 0; i < nentries; ++i) {
            pfc_cptr_t get_val = NULL;
            int get_err = pfc_u64hash_get(hash, key[i], &get_val);
            EXPECT_EQ(0, get_err);
            EXPECT_TRUE(get_val == val[n][i]);
        }

        // don't remove at final iteration
        if ((remove_all_foreach != 0) && (n < val_nsets - 1)) {
            if (n % remove_all_foreach == 0) {
                // remove for each entries
                for (int i = 0; i < nentries; ++i) {
                    // remove for existing entry
                    {
                        int remove_err = pfc_u64hash_remove(hash, key[i]);
                        EXPECT_EQ(0, remove_err);
                        SET_FLAG(test_data, remove_existing_key);
                    }

                    // get for unregistered entry
                    {
                        pfc_cptr_t get_val = NULL;
                        int get_err = pfc_u64hash_get(hash, key[i], &get_val);
                        EXPECT_EQ(ENOENT, get_err);
                        SET_FLAG(test_data, get_unreg_key);
                    }

                    // remove for unregistered entry
                    {
                        int remove_err = pfc_u64hash_remove(hash, key[i]);
                        EXPECT_EQ(ENOENT, remove_err);
                        SET_FLAG(test_data, remove_unreg_key);
                    }
                }
            }
        }
    }
}
