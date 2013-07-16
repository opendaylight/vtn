/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for strint
 */

#include "strint_refptr.h"
#include <gtest/gtest.h>
#include <stdlib.h>


static const int nelems = 10;


TEST(strint, create_and_dtor)
{
    // allocate
    pfc_refptr_t *sint_refp = strint_create_from_int(1);
    EXPECT_TRUE(sint_refp != NULL);

    if (getenv("STRINT_LEAK_TEST") != NULL) {
        // cause leak
        fprintf(stderr, "a strint refptr is leaked!\n");
    } else {
        // release
        pfc_cptr_t obj_ptr = PFC_REFPTR_VALUE(sint_refp, pfc_cptr_t);
        uint64_t old_dtor_cnt = strint_get_count_dtor();
        pfc_refptr_put(sint_refp);
#ifndef PARALLEL_TEST
        EXPECT_EQ(old_dtor_cnt + 1, strint_get_count_dtor());
        EXPECT_EQ(obj_ptr, strint_get_dtor_last_addr());
#else  /* !PARALLEL_TEST */
        EXPECT_GT(strint_get_count_dtor(), old_dtor_cnt);
#endif /* !PARALLEL_TEST */
    }
}


TEST(strint, equals_and_compare)
{
    pfc_refptr_t *sint_refp[2][nelems] = { { 0 }, { 0 } };

    // allocate
    for (int n = 0; n < 2; n++) {
        for (int i = 0; i < nelems; i++) {
            sint_refp[n][i] = strint_create_from_int(i);
            EXPECT_TRUE(sint_refp[n][i] != NULL);
        }
    }

    for (int i = 0; i < nelems; i++) {
        for (int j = 0; j < nelems; j++) {
            char *s1 = str_of_strint_refptr(sint_refp[0][i]);
            char *s2 = str_of_strint_refptr(sint_refp[1][j]);

            // call strint_compare()
            uint64_t old_compare_cnt = strint_get_count_compare();
            if (i == j) {
                EXPECT_EQ(0, strint_compare(s1, s2));
            } else if (i > j) {
                EXPECT_EQ(1, strint_compare(s1, s2));
            } else {
                EXPECT_EQ(-1, strint_compare(s1, s2));
            }
            EXPECT_TRUE(strint_get_count_compare() > old_compare_cnt);

            // call strint_equals()
            uint64_t old_equals_cnt = strint_get_count_equals();
            if (i == j) {
                EXPECT_TRUE(strint_equals(s1, s2));
            } else {
                EXPECT_FALSE(strint_equals(s1, s2));
            }
            EXPECT_TRUE(strint_get_count_equals() > old_equals_cnt);

            // call strint_equals() via pfc_refptr_equals
            old_equals_cnt = strint_get_count_equals();
            if (i == j) {
                EXPECT_TRUE(pfc_refptr_equals(sint_refp[0][i], sint_refp[1][j]));
            } else {
                EXPECT_FALSE(pfc_refptr_equals(sint_refp[0][i], sint_refp[1][j]));
            }
            EXPECT_TRUE(strint_get_count_equals() > old_equals_cnt);
        }
    }

    // relase
    for (int n = 0; n < 2; n++) {
        for (int i = 0; i < nelems; i++) {
            uint64_t old_dtor_cnt = strint_get_count_dtor();
            pfc_refptr_put(sint_refp[n][i]);
            uint64_t new_dtor_cnt = strint_get_count_dtor();
            EXPECT_TRUE(new_dtor_cnt > old_dtor_cnt);
        }
    }
}


TEST(strint, hashfunc)
{
    pfc_refptr_t *sint_refp[nelems] = { 0 };

    // allocate
    for (int i = 0; i < nelems; i++) {
        sint_refp[i] = strint_create_from_int(i);
        EXPECT_TRUE(sint_refp[i] != NULL);
    }

    for (int i = 0; i < nelems; i++) {
        // call strint_hashfunc()
        char *s = str_of_strint_refptr(sint_refp[i]);
        uint64_t old_hashfunc_cnt = strint_get_count_hashfunc();
        EXPECT_EQ((uint64_t)i, strint_hashfunc(s));
        EXPECT_TRUE(strint_get_count_hashfunc() > old_hashfunc_cnt);
    }

    // relase
    for (int i = 0; i < nelems; i++) {
        uint64_t old_dtor_cnt = strint_get_count_dtor();
        pfc_refptr_put(sint_refp[i]);
        uint64_t new_dtor_cnt = strint_get_count_dtor();
        EXPECT_TRUE(new_dtor_cnt > old_dtor_cnt);
    }
}


TEST(strint, str_of_strint_refptr)
{
    pfc_refptr_t *sint_refp[nelems] = { 0 };

    // allocate
    for (int i = 0; i < nelems; i++) {
        sint_refp[i] = strint_create_from_int(i);
        EXPECT_TRUE(sint_refp[i] != NULL);
    }

    for (int i = 0; i < nelems; i++) {
        // get string object
        char *s = str_of_strint_refptr(sint_refp[i]);
        EXPECT_EQ((char *)(sint_refp[i]->pr_object), s);
    }

    // relase
    for (int i = 0; i < nelems; i++) {
        uint64_t old_dtor_cnt = strint_get_count_dtor();
        pfc_refptr_put(sint_refp[i]);
        uint64_t new_dtor_cnt = strint_get_count_dtor();
        EXPECT_TRUE(new_dtor_cnt > old_dtor_cnt);
    }
}


TEST(strint, int_of_strint_refptr)
{
    pfc_refptr_t *sint_refp[nelems] = { 0 };

    // allocate
    for (int i = 0; i < nelems; i++) {
        sint_refp[i] = strint_create_from_int(i);
        EXPECT_TRUE(sint_refp[i] != NULL);
    }

    for (int i = 0; i < nelems; i++) {
        // get int value
        int int_val = int_of_strint_refptr(sint_refp[i]);
        int ans = str_to_int(str_of_strint_refptr(sint_refp[i]));
        EXPECT_EQ(ans, int_val);
    }

    // relase
    for (int i = 0; i < nelems; i++) {
        uint64_t old_dtor_cnt = strint_get_count_dtor();
        pfc_refptr_put(sint_refp[i]);
        uint64_t new_dtor_cnt = strint_get_count_dtor();
        EXPECT_TRUE(new_dtor_cnt > old_dtor_cnt);
    }
}


TEST(strint, str_to_int)
{
    for (uint64_t i = 0; i < UINT32_MAX; i += (UINT32_MAX / 42953)) {
        char s[100] = { 0 };
        snprintf(s, sizeof(s), "%u", (uint32_t)i);
        EXPECT_EQ((uint32_t)i, str_to_int(s));
    }
}


TEST(strint, int_to_str)
{
    for (uint64_t i = 0; i < UINT32_MAX; i += (UINT32_MAX / 42953)) {
        char *str_val = int_to_str((uint32_t)i);
        char ans[100] = { 0 };
        snprintf(ans, sizeof(ans), "%u", (uint32_t)i);
        EXPECT_STREQ(ans, str_val);
        free(str_val);
    }
}
