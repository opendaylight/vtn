/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for refptr
 */

#include <cstdio>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <gtest/gtest.h>
#include <pfc/refptr.h>
#include <pfc/thread.h>
#include <pfc/tpool.h>
#include <pfc/clock.h>
#include <pfc/util.h>
#include <pfcxx/refptr.hh>
#include "test.h"
#include "thread_subr.hh"
#include "random.hh"

using namespace  pfc::core;

#define TEST_NUM 1000

static int check_dtor;

/* test structure, test function */
typedef struct {
    int  num;
    char *c;
} test_t;

/* test destructor for refptr */
static void
test_refptr_dtor(pfc_ptr_t obj)
{
    check_dtor++;
    free((test_t *)obj);
}

/* test compare function for refptr */
static int
test_refptr_compare(pfc_cptr_t obj1, pfc_cptr_t obj2)
{
    if (((test_t *)obj1)->num > ((test_t *)obj2)->num) {
        return 1;
    } else if (((test_t *)obj1)->num == ((test_t *)obj2)->num) {
        return 0;
    } else {
        return -1;
    }
}

/* test equals function for refptr */
static pfc_bool_t
test_refptr_equals(pfc_cptr_t obj1, pfc_cptr_t obj2)
{
    if (((test_t *)obj1)->num == ((test_t *)obj2)->num) {
        return PFC_TRUE;
    } else {
        return PFC_FALSE;
    }
}

/* test hushfunc function for refptr */
static uint32_t
test_refptr_hashfunc(pfc_cptr_t obj)
{
    return UINT32_MAX;
}

/* test ops */
static const pfc_refptr_ops_t test_ops = {
    test_refptr_dtor,
    test_refptr_compare,
    test_refptr_equals,
    test_refptr_hashfunc
};

/* test null ops */
static const pfc_refptr_ops_t test_null_ops = {
    NULL,
    NULL,
    NULL,
    NULL
};


/*
 *
 * test cases
 *
 */ 

/* test form pfc_refptr_int32_create() */
TEST(refptr, test_pfc_refptr_int32_create)
{
    pfc_refptr_t *int32_ptr1 = NULL;
    pfc_refptr_t *int32_ptr2 = NULL;
    int32_ptr1 = pfc_refptr_int32_create(INT32_MAX);
    int32_ptr2 = pfc_refptr_int32_create(INT32_MIN);

    /* test normal */
    ASSERT_TRUE(int32_ptr1 != NULL);
    ASSERT_TRUE(int32_ptr2 != NULL);
    /* check value */
    ASSERT_EQ(INT32_MAX, pfc_refptr_int32_value(int32_ptr1));
    ASSERT_EQ(INT32_MIN, pfc_refptr_int32_value(int32_ptr2));
    ASSERT_EQ(1U, int32_ptr1->pr_refcnt);
    ASSERT_EQ(1U, int32_ptr2->pr_refcnt);

    pfc_refptr_put(int32_ptr1);
    pfc_refptr_put(int32_ptr2);
}

/* test for pfc_refptr_int32_value() */
TEST(refptr, test_pfc_refptr_int32_value)
{
    pfc_refptr_t *int32_ptr1 = NULL;
    pfc_refptr_t *int32_ptr2 = NULL;
    int32_ptr1 = pfc_refptr_int32_create(INT32_MAX);
    int32_ptr2 = pfc_refptr_int32_create(INT32_MIN);

    /* test normal */
    EXPECT_TRUE(int32_ptr1 != NULL);
    EXPECT_TRUE(int32_ptr2 != NULL);
    /* check value */
    ASSERT_EQ(INT32_MAX, pfc_refptr_int32_value(int32_ptr1));
    ASSERT_EQ(INT32_MIN, pfc_refptr_int32_value(int32_ptr2));

    pfc_refptr_put(int32_ptr1);
    pfc_refptr_put(int32_ptr2);
}

/* test for pfc_refptr_int32_ops() */
TEST(refptr, test_pfc_refptr_int32_ops)
{
    pfc_refptr_t *int32_ptr1 = NULL;
    int32_ptr1 = pfc_refptr_int32_create(INT32_MAX);
    pfc_refptr_t *int32_ptr2 = NULL;
    int32_ptr2 = pfc_refptr_int32_create(INT32_MAX);
    pfc_refptr_t *int32_ptr3 = NULL;
    int32_ptr3 = pfc_refptr_int32_create(0);
    pfc_refptr_t *int32_ptr4 = NULL;
    int32_ptr4 = pfc_refptr_int32_create(INT32_MIN);

    const pfc_refptr_ops_t *ops = pfc_refptr_int32_ops();
    ASSERT_TRUE(ops != NULL);

    /* test ops->compare */
    ASSERT_EQ(0, ops->compare(int32_ptr1->pr_object, int32_ptr2->pr_object));
    ASSERT_EQ(1, ops->compare(int32_ptr1->pr_object, int32_ptr3->pr_object));
    ASSERT_EQ(1, ops->compare(int32_ptr3->pr_object, int32_ptr4->pr_object));
    ASSERT_EQ(-1, ops->compare(int32_ptr3->pr_object, int32_ptr1->pr_object));

    /* test ops->equals */
    ASSERT_EQ(PFC_TRUE,
              ops->equals(int32_ptr1->pr_object, int32_ptr2->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(int32_ptr1->pr_object, int32_ptr3->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(int32_ptr3->pr_object, int32_ptr1->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(int32_ptr2->pr_object, int32_ptr4->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(int32_ptr3->pr_object, int32_ptr4->pr_object));

    ASSERT_TRUE(typeid(ops->hashfunc(int32_ptr1->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(int32_ptr2->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(int32_ptr3->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(int32_ptr4->pr_object)) ==
                typeid(uint32_t));

    ASSERT_EQ(ops->hashfunc(int32_ptr1->pr_object),
              ops->hashfunc(int32_ptr2->pr_object));

    pfc_refptr_put(int32_ptr1);
    pfc_refptr_put(int32_ptr2);
    pfc_refptr_put(int32_ptr3);
    pfc_refptr_put(int32_ptr4);

    /* check hash */
    {
        int i, j, count;
        count = TEST_NUM;
        pfc_refptr_t *int32_ptr[count];
        const pfc_refptr_ops_t *ops = pfc_refptr_int32_ops();
        ASSERT_TRUE(ops != NULL);

        for (i = 0; i < count; i++) {
            int32_ptr[i] = pfc_refptr_int32_create(INT32_MAX);
            EXPECT_TRUE(int32_ptr[i] != NULL);
        }
        for (i = 0; i < count-1; i++) {
            for (j = i + 1; j < count; j++) {
                EXPECT_EQ(ops->hashfunc(int32_ptr[i]->pr_object),
                          ops->hashfunc(int32_ptr[j]->pr_object));
            }
        }
        for (i = 0; i < count; i++) {
            pfc_refptr_put(int32_ptr[i]);
        }
    }
}

/* test form pfc_refptr_uint32_create() */
TEST(refptr, test_pfc_refptr_uint32_create)
{
    pfc_refptr_t *uint32_ptr = NULL;
    uint32_ptr = pfc_refptr_uint32_create(UINT32_MAX);

    /* test normal */
    ASSERT_TRUE(uint32_ptr != NULL);
    /* check value */
    ASSERT_EQ(UINT32_MAX, pfc_refptr_uint32_value(uint32_ptr));
    ASSERT_EQ(1U, uint32_ptr->pr_refcnt);

    pfc_refptr_put(uint32_ptr);
}

/* test for pfc_refptr_uint32_value() */
TEST(refptr, test_pfc_refptr_uint32_value)
{
    pfc_refptr_t *uint32_ptr = NULL;
    uint32_ptr = pfc_refptr_uint32_create(UINT32_MAX);

    /* test normal */
    EXPECT_TRUE(uint32_ptr != NULL);
    /* check value */
    ASSERT_EQ(UINT32_MAX, pfc_refptr_uint32_value(uint32_ptr));

    pfc_refptr_put(uint32_ptr);
}

/* test for pfc_refptr_uint32_ops() */
TEST(refptr, test_pfc_refptr_uint32_ops)
{
    pfc_refptr_t *uint32_ptr1 = NULL;
    uint32_ptr1 = pfc_refptr_uint32_create(UINT32_MAX);
    pfc_refptr_t *uint32_ptr2 = NULL;
    uint32_ptr2 = pfc_refptr_uint32_create(UINT32_MAX);
    pfc_refptr_t *uint32_ptr3 = NULL;
    uint32_ptr3 = pfc_refptr_uint32_create(INT32_MAX);
    pfc_refptr_t *uint32_ptr4 = NULL;
    uint32_ptr4 = pfc_refptr_uint32_create(0);

    const pfc_refptr_ops_t *ops = pfc_refptr_uint32_ops();
    ASSERT_TRUE(ops != NULL);

    /* test ops->compare */
    ASSERT_EQ(0, ops->compare(uint32_ptr1->pr_object, uint32_ptr2->pr_object));
    ASSERT_EQ(1, ops->compare(uint32_ptr1->pr_object, uint32_ptr3->pr_object));
    ASSERT_EQ(1, ops->compare(uint32_ptr3->pr_object, uint32_ptr4->pr_object));
    ASSERT_EQ(-1, ops->compare(uint32_ptr3->pr_object, uint32_ptr1->pr_object));

    /* test ops->equals */
    ASSERT_EQ(PFC_TRUE,
              ops->equals(uint32_ptr1->pr_object, uint32_ptr2->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(uint32_ptr1->pr_object, uint32_ptr3->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(uint32_ptr3->pr_object, uint32_ptr1->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(uint32_ptr2->pr_object, uint32_ptr3->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(uint32_ptr2->pr_object, uint32_ptr4->pr_object));

    ASSERT_TRUE(typeid(ops->hashfunc(uint32_ptr1->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(uint32_ptr2->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(uint32_ptr3->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(uint32_ptr4->pr_object)) ==
                typeid(uint32_t));

    ASSERT_EQ(ops->hashfunc(uint32_ptr1->pr_object),
              ops->hashfunc(uint32_ptr2->pr_object));

    pfc_refptr_put(uint32_ptr1);
    pfc_refptr_put(uint32_ptr2);
    pfc_refptr_put(uint32_ptr3);
    pfc_refptr_put(uint32_ptr4);

    /* check hash */
    {
        int i, j, count;
        count = TEST_NUM;
        pfc_refptr_t *uint32_ptr[count];
        const pfc_refptr_ops_t *ops = pfc_refptr_uint32_ops();
        ASSERT_TRUE(ops != NULL);

        for (i = 0; i < count; i++) {
            uint32_ptr[i] = pfc_refptr_uint32_create(UINT32_MAX);
            EXPECT_TRUE(uint32_ptr[i] != NULL);
        }
        for (i = 0; i < count-1; i++) {
            for (j = i + 1; j < count; j++) {
                EXPECT_EQ(ops->hashfunc(uint32_ptr[i]->pr_object),
                          ops->hashfunc(uint32_ptr[j]->pr_object));
            }
        }
        for (i = 0; i < count; i++) {
            pfc_refptr_put(uint32_ptr[i]);
        }
    }
}

/* test form pfc_refptr_int64_create() */
TEST(refptr, test_pfc_refptr_int64_create)
{
    pfc_refptr_t *int64_ptr1 = NULL;
    pfc_refptr_t *int64_ptr2 = NULL;
    int64_ptr1 = pfc_refptr_int64_create(INT64_MAX);
    int64_ptr2 = pfc_refptr_int64_create(INT64_MIN);

    /* test normal */
    ASSERT_TRUE(int64_ptr1 != NULL);
    ASSERT_TRUE(int64_ptr2 != NULL);
    /* check value */
    ASSERT_EQ(INT64_MAX, pfc_refptr_int64_value(int64_ptr1));
    ASSERT_EQ(INT64_MIN, pfc_refptr_int64_value(int64_ptr2));
    ASSERT_EQ(1U, int64_ptr1->pr_refcnt);
    ASSERT_EQ(1U, int64_ptr2->pr_refcnt);

    pfc_refptr_put(int64_ptr1);
    pfc_refptr_put(int64_ptr2);
}

/* test for pfc_refptr_int64_value() */
TEST(refptr, test_pfc_refptr_int64_value)
{
    pfc_refptr_t *int64_ptr1 = NULL;
    pfc_refptr_t *int64_ptr2 = NULL;
    int64_ptr1 = pfc_refptr_int64_create(INT64_MAX);
    int64_ptr2 = pfc_refptr_int64_create(INT64_MIN);

    /* test normal */
    EXPECT_TRUE(int64_ptr1 != NULL);
    EXPECT_TRUE(int64_ptr2 != NULL);
    /* check value */
    ASSERT_EQ(INT64_MAX, pfc_refptr_int64_value(int64_ptr1));
    ASSERT_EQ(INT64_MIN, pfc_refptr_int64_value(int64_ptr2));

    pfc_refptr_put(int64_ptr1);
    pfc_refptr_put(int64_ptr2);
}

/* test for pfc_refptr_int64_ops() */
TEST(refptr, test_pfc_refptr_int64_ops)
{
    pfc_refptr_t *int64_ptr1 = NULL;
    int64_ptr1 = pfc_refptr_int64_create(INT64_MAX);
    pfc_refptr_t *int64_ptr2 = NULL;
    int64_ptr2 = pfc_refptr_int64_create(INT64_MAX);
    pfc_refptr_t *int64_ptr3 = NULL;
    int64_ptr3 = pfc_refptr_int64_create(0);
    pfc_refptr_t *int64_ptr4 = NULL;
    int64_ptr4 = pfc_refptr_int64_create(INT64_MIN);

    const pfc_refptr_ops_t *ops = pfc_refptr_int64_ops();
    ASSERT_TRUE(ops != NULL);

    /* test ops->compare */
    ASSERT_EQ(0, ops->compare(int64_ptr1->pr_object, int64_ptr2->pr_object));
    ASSERT_EQ(1, ops->compare(int64_ptr1->pr_object, int64_ptr3->pr_object));
    ASSERT_EQ(1, ops->compare(int64_ptr3->pr_object, int64_ptr4->pr_object));
    ASSERT_EQ(-1, ops->compare(int64_ptr3->pr_object, int64_ptr1->pr_object));

    /* test ops->equals */
    ASSERT_EQ(PFC_TRUE,
              ops->equals(int64_ptr1->pr_object, int64_ptr2->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(int64_ptr1->pr_object, int64_ptr3->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(int64_ptr3->pr_object, int64_ptr1->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(int64_ptr2->pr_object, int64_ptr3->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(int64_ptr2->pr_object, int64_ptr4->pr_object));

    ASSERT_TRUE(typeid(ops->hashfunc(int64_ptr1->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(int64_ptr2->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(int64_ptr3->pr_object)) ==
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(int64_ptr4->pr_object)) ==
                typeid(uint32_t));

    ASSERT_EQ(ops->hashfunc(int64_ptr1->pr_object),
              ops->hashfunc(int64_ptr2->pr_object));

    pfc_refptr_put(int64_ptr1);
    pfc_refptr_put(int64_ptr2);
    pfc_refptr_put(int64_ptr3);
    pfc_refptr_put(int64_ptr4);

    /* check hash */
    {
        int i, j, count;
        count = TEST_NUM;
        pfc_refptr_t *int64_ptr[count];
        const pfc_refptr_ops_t *ops = pfc_refptr_int64_ops();
        ASSERT_TRUE(ops != NULL);

        for (i = 0; i < count; i++) {
            int64_ptr[i] = pfc_refptr_int64_create(INT64_MAX);
            EXPECT_TRUE(int64_ptr[i] != NULL);
        }
        for (i = 0; i < count-1; i++) {
            for (j = i + 1; j < count; j++) {
                EXPECT_EQ(ops->hashfunc(int64_ptr[i]->pr_object),
                          ops->hashfunc(int64_ptr[j]->pr_object));
            }
        }
        for (i = 0; i < count; i++) {
            pfc_refptr_put(int64_ptr[i]);
        }
    }
}

/* test form pfc_refptr_uint64_create() */
TEST(refptr, test_pfc_refptr_uint64_create)
{
    pfc_refptr_t *uint64_ptr = NULL;
    uint64_ptr = pfc_refptr_uint64_create(UINT64_MAX);

    /* test normal */
    ASSERT_TRUE(uint64_ptr != NULL);
    /* check value */
    ASSERT_EQ(UINT64_MAX, pfc_refptr_uint64_value(uint64_ptr));
    ASSERT_EQ(1U, uint64_ptr->pr_refcnt);

    pfc_refptr_put(uint64_ptr);
}

/* test for pfc_refptr_uint64_value() */
TEST(refptr, test_pfc_refptr_uint64_value)
{
    pfc_refptr_t *uint64_ptr = NULL;
    uint64_ptr = pfc_refptr_uint64_create(UINT64_MAX);

    /* test normal */
    EXPECT_TRUE(uint64_ptr != NULL);
    /* check value */
    ASSERT_EQ(UINT64_MAX, pfc_refptr_uint64_value(uint64_ptr));

    pfc_refptr_put(uint64_ptr);
}

/* test for pfc_refptr_uint64_ops() */
TEST(refptr, test_pfc_refptr_uint64_ops)
{
    pfc_refptr_t *uint64_ptr1 = NULL;
    uint64_ptr1 = pfc_refptr_uint64_create(UINT64_MAX);
    pfc_refptr_t *uint64_ptr2 = NULL;
    uint64_ptr2 = pfc_refptr_uint64_create(UINT64_MAX);
    pfc_refptr_t *uint64_ptr3 = NULL;
    uint64_ptr3 = pfc_refptr_uint64_create(INT64_MAX);
    pfc_refptr_t *uint64_ptr4 = NULL;
    uint64_ptr4 = pfc_refptr_uint64_create(0);

    const pfc_refptr_ops_t *ops = pfc_refptr_uint64_ops();
    ASSERT_TRUE(ops != NULL);

    /* test ops->compare */
    ASSERT_EQ(0, ops->compare(uint64_ptr1->pr_object, uint64_ptr2->pr_object));
    ASSERT_EQ(1, ops->compare(uint64_ptr1->pr_object, uint64_ptr3->pr_object));
    ASSERT_EQ(1, ops->compare(uint64_ptr3->pr_object, uint64_ptr4->pr_object));
    ASSERT_EQ(-1, ops->compare(uint64_ptr3->pr_object, uint64_ptr1->pr_object));

    /* test ops->equals */
    ASSERT_EQ(PFC_TRUE,
              ops->equals(uint64_ptr1->pr_object, uint64_ptr2->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(uint64_ptr1->pr_object, uint64_ptr3->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(uint64_ptr3->pr_object, uint64_ptr1->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(uint64_ptr2->pr_object, uint64_ptr3->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(uint64_ptr2->pr_object, uint64_ptr4->pr_object));

    ASSERT_TRUE(typeid(ops->hashfunc(uint64_ptr1->pr_object)) == 
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(uint64_ptr2->pr_object)) == 
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(uint64_ptr3->pr_object)) == 
                typeid(uint32_t));

    ASSERT_TRUE(typeid(ops->hashfunc(uint64_ptr4->pr_object)) == 
                typeid(uint32_t));

    ASSERT_EQ(ops->hashfunc(uint64_ptr1->pr_object),
              ops->hashfunc(uint64_ptr2->pr_object));

    pfc_refptr_put(uint64_ptr1);
    pfc_refptr_put(uint64_ptr2);
    pfc_refptr_put(uint64_ptr3);
    pfc_refptr_put(uint64_ptr4);

    /* check hash */
    {
        int i, j, count;
        count = TEST_NUM;
        pfc_refptr_t *uint64_ptr[count];
        const pfc_refptr_ops_t *ops = pfc_refptr_uint64_ops();
        ASSERT_TRUE(ops != NULL);

        for (i = 0; i < count; i++) {
            uint64_ptr[i] = pfc_refptr_uint64_create(UINT64_MAX);
            EXPECT_TRUE(uint64_ptr[i] != NULL);
        }
        for (i = 0; i < count-1; i++) {
            for (j = i + 1; j < count; j++) {
                EXPECT_EQ(ops->hashfunc(uint64_ptr[i]->pr_object),
                          ops->hashfunc(uint64_ptr[j]->pr_object));
            }
        }
        for (i = 0; i < count; i++) {
            pfc_refptr_put(uint64_ptr[i]);
        }
    }
}

/* test for pfc_refptr_string_create() */
TEST(refptr, test_pfc_refptr_string_create)
{
    char str1[] = "hogehoge";
    char str2[] = "";
    char *str4 = NULL;
    char str5[32] = "before";
    const char test_string[] = "test string";

    str4 = (char *)malloc(32);
    EXPECT_TRUE(str4 != NULL);
    EXPECT_TRUE(strncpy(str4, test_string, 32) != NULL); 

    pfc_refptr_t *string_ptr1 = NULL;
    pfc_refptr_t *string_ptr2 = NULL;
    pfc_refptr_t *string_ptr3 = NULL;
    pfc_refptr_t *string_ptr4 = NULL;
    pfc_refptr_t *string_ptr5 = NULL;
    string_ptr1 = pfc_refptr_string_create(str1);
    string_ptr2 = pfc_refptr_string_create(str2);
    string_ptr3 = pfc_refptr_string_create("static string");
    string_ptr4 = pfc_refptr_string_create(str4);
    string_ptr5 = pfc_refptr_string_create(str5);

    /* test normal */
    ASSERT_TRUE(string_ptr1 != NULL);
    ASSERT_TRUE(string_ptr2 != NULL);
    ASSERT_TRUE(string_ptr3 != NULL);

    /* check value */
    EXPECT_EQ(0, strcmp(pfc_refptr_string_value(string_ptr1), str1));
    EXPECT_EQ(strlen(str1), pfc_refptr_string_length(string_ptr1));
    EXPECT_EQ(1U, string_ptr1->pr_refcnt);
    EXPECT_EQ(0, strcmp(pfc_refptr_string_value(string_ptr2), str2));
    EXPECT_EQ(strlen(str2), pfc_refptr_string_length(string_ptr2));
    EXPECT_EQ(1U, string_ptr2->pr_refcnt);
    EXPECT_EQ(0, strcmp(pfc_refptr_string_value(string_ptr3), "static string"));
    EXPECT_EQ(strlen("static string"), pfc_refptr_string_length(string_ptr3));
    EXPECT_EQ(1U, string_ptr3->pr_refcnt);

    /* test to copy string for bringing reference pointer */
    EXPECT_TRUE(strncpy(str4, "", 32) != NULL); 
    free(str4);
    ASSERT_EQ(0, strcmp(pfc_refptr_string_value(string_ptr4), test_string)); 

    EXPECT_TRUE(strncpy(str5, test_string, 16) != NULL);
    EXPECT_NE(0, strcmp(str5, "before"));
    ASSERT_EQ(0, strcmp(pfc_refptr_string_value(string_ptr5), "before"));

    pfc_refptr_put(string_ptr1);
    pfc_refptr_put(string_ptr2);
    pfc_refptr_put(string_ptr3);
    pfc_refptr_put(string_ptr4);
    pfc_refptr_put(string_ptr5);

    // pfc_refptr_string_create() with specifying the same string must return
    // the same pointer.
    pfc_refptr_t  *ref1(pfc_refptr_string_create(test_string));
    ASSERT_TRUE(ref1 != NULL);
    EXPECT_EQ(1U, ref1->pr_refcnt);

    // Reference counter is incremented by one (1 -> 2)
    pfc_refptr_t  *ref2(pfc_refptr_string_create(test_string));
    EXPECT_EQ(2U, ref2->pr_refcnt);

    // Reference counter is incremented by one (2 -> 3)
    pfc_refptr_t  *ref3(pfc_refptr_string_create(test_string));
    EXPECT_EQ(3U, ref3->pr_refcnt);
    EXPECT_EQ(3U, ref2->pr_refcnt);
    EXPECT_EQ(3U, ref1->pr_refcnt);

    // pfc_refptr_sprintf() must return the same pointer if it constructs
    // a string which is already exists in the global refptr string tree.
    pfc_refptr_t  *ref4(pfc_refptr_sprintf("%s", test_string));
    EXPECT_EQ(4U, ref4->pr_refcnt);
    EXPECT_EQ(4U, ref3->pr_refcnt);
    EXPECT_EQ(4U, ref2->pr_refcnt);
    EXPECT_EQ(4U, ref1->pr_refcnt);

    EXPECT_EQ(ref1, ref2);
    EXPECT_EQ(ref1, ref3);
    EXPECT_EQ(ref1, ref4);

    // Reference counter is decremented by one (4 -> 3)
    pfc_refptr_put(ref1);
    EXPECT_EQ(3U, ref3->pr_refcnt);

    // Reference counter is decremented by one (3 -> 2)
    pfc_refptr_put(ref2);
    EXPECT_EQ(2U, ref3->pr_refcnt);

    // Reference counter is decremented by one (2 -> 1)
    pfc_refptr_put(ref3);
    EXPECT_EQ(1U, ref4->pr_refcnt);

    pfc_refptr_put(ref4);
}

/* test for pfc_refptr_sprintf() */
TEST(refptr, test_pfc_refptr_sprintf)
{
    // Static string without format.
    {
        const char  *strings[] = {
            "",
            "a",
            "this is a test.",
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890",
        };

        for (const char **pp(strings); pp < PFC_ARRAY_LIMIT(strings); pp++) {
            const char *p(*pp);

            pfc_refptr_t  *ref1(pfc_refptr_sprintf(p));
            ASSERT_TRUE(ref1 != NULL);
            RefPointer  rp1(ref1);

            ASSERT_STREQ(p, pfc_refptr_string_value(ref1));
            ASSERT_EQ(strlen(p), pfc_refptr_string_length(ref1));
            ASSERT_EQ(1U, ref1->pr_refcnt);

            pfc_refptr_t  *ref2(pfc_refptr_sprintf("%s", p));
            ASSERT_TRUE(ref2 != NULL);
            RefPointer  rp2(ref2);
            ASSERT_EQ(ref2, ref1);
            ASSERT_EQ(2U, ref1->pr_refcnt);

            pfc_refptr_t  *ref3(pfc_refptr_string_create(p));
            ASSERT_TRUE(ref3 != NULL);
            RefPointer  rp3(ref3);
            ASSERT_EQ(ref3, ref1);
            ASSERT_EQ(3U, ref1->pr_refcnt);
        }
    }

    // Format string with one integer argument.
    char  expected[1024];
    {
        pid_t pid(getpid());

        const char  *formats[] = {
            "%u",
            "pid=%u",
            "pid = %08u",
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890"
            ":pid=%#X"
            "12345678901234567890123456789012345678901234567890"
            "12345678901234567890123456789012345678901234567890",
        };

        for (const char **fmtpp(formats); fmtpp < PFC_ARRAY_LIMIT(formats);
             fmtpp++) {
            const char *fmt(*fmtpp);

            snprintf(expected, sizeof(expected), fmt, pid);
            pfc_refptr_t  *ref1(pfc_refptr_sprintf(fmt, pid));
            ASSERT_TRUE(ref1 != NULL);
            RefPointer  rp1(ref1);

            ASSERT_STREQ(expected, pfc_refptr_string_value(ref1));
            ASSERT_EQ(strlen(expected), pfc_refptr_string_length(ref1));
            ASSERT_EQ(1U, ref1->pr_refcnt);

            pfc_refptr_t  *ref2(pfc_refptr_sprintf(fmt, pid));
            ASSERT_TRUE(ref2 != NULL);
            RefPointer  rp2(ref2);
            ASSERT_EQ(ref2, ref1);
            ASSERT_EQ(2U, ref1->pr_refcnt);

            pfc_refptr_t  *ref3(pfc_refptr_string_create(expected));
            ASSERT_TRUE(ref3 != NULL);
            RefPointer  rp3(ref3);
            ASSERT_EQ(ref3, ref1);
            ASSERT_EQ(3U, ref1->pr_refcnt);
        }
    }

    // Complicated format string.
    RandomGenerator  rand;

    for (int loop(0); loop < 10; loop++) {
        int32_t    arg1;
        uint32_t   arg2;
        int64_t    arg3;
        uint64_t   arg4;
        char       arg5[64];

        RANDOM_INTEGER(rand, arg1);
        RANDOM_INTEGER(rand, arg2);
        RANDOM_INTEGER(rand, arg3);
        RANDOM_INTEGER(rand, arg4);
        ASSERT_EQ(0, pfc_time_clock_ctime(arg5, sizeof(arg5)));

        {
            snprintf(expected, sizeof(expected),
                     "arg1=%d arg2=%u arg3=%" PFC_PFMT_d64
                     " arg4=%" PFC_PFMT_x64 " arg5=%s",
                     arg1, arg2, arg3, arg4, arg5);
            pfc_refptr_t  *ref1(pfc_refptr_sprintf
                                ("arg1=%d arg2=%u arg3=%" PFC_PFMT_d64
                                 " arg4=%" PFC_PFMT_x64 " arg5=%s",
                                 arg1, arg2, arg3, arg4, arg5));
            ASSERT_TRUE(ref1 != NULL);
            RefPointer  rp1(ref1);

            ASSERT_STREQ(expected, pfc_refptr_string_value(ref1));
            ASSERT_EQ(strlen(expected), pfc_refptr_string_length(ref1));
            ASSERT_EQ(1U, ref1->pr_refcnt);

            pfc_refptr_t  *ref2(pfc_refptr_sprintf
                                ("arg1=%d arg2=%u arg3=%" PFC_PFMT_d64
                                 " arg4=%" PFC_PFMT_x64 " arg5=%s",
                                 arg1, arg2, arg3, arg4, arg5));
            ASSERT_TRUE(ref2 != NULL);
            RefPointer  rp2(ref2);
            ASSERT_EQ(ref2, ref1);
            ASSERT_EQ(2U, ref1->pr_refcnt);

            pfc_refptr_t  *ref3(pfc_refptr_string_create(expected));
            ASSERT_TRUE(ref3 != NULL);
            RefPointer  rp3(ref3);
            ASSERT_EQ(ref3, ref1);
            ASSERT_EQ(3U, ref1->pr_refcnt);
        }

        {
            const int  arg4len(static_cast<int>(sizeof(arg4) >> 1));
            snprintf(expected, sizeof(expected),
                     "%%arg5 = %2$*1$s, "
                     "%%arg4 = %4$#0*3$" PFC_PFMT_X64 ", "
                     "%%arg3 = %6$*5$" PFC_PFMT_d64 ", "
                     "%%arg2 = %8$*7$u, "
                     "%%arg1 = %10$*9$d\n",
                     -48, arg5,
                     arg4len, arg4,
                     -36, arg3,
                     30, arg2,
                     -18, arg1);
            pfc_refptr_t  *ref1(pfc_refptr_sprintf
                                ("%%arg5 = %2$*1$s, "
                                 "%%arg4 = %4$#0*3$" PFC_PFMT_X64 ", "
                                 "%%arg3 = %6$*5$" PFC_PFMT_d64 ", "
                                 "%%arg2 = %8$*7$u, "
                                 "%%arg1 = %10$*9$d\n",
                                 -48, arg5,
                                 arg4len, arg4,
                                 -36, arg3,
                                 30, arg2,
                                 -18, arg1));
            ASSERT_TRUE(ref1 != NULL);
            RefPointer  rp1(ref1);

            ASSERT_STREQ(expected, pfc_refptr_string_value(ref1));
            ASSERT_EQ(strlen(expected), pfc_refptr_string_length(ref1));
            ASSERT_EQ(1U, ref1->pr_refcnt);

            pfc_refptr_t  *ref2(pfc_refptr_sprintf
                                ("%%arg5 = %2$*1$s, "
                                 "%%arg4 = %4$#0*3$" PFC_PFMT_X64 ", "
                                 "%%arg3 = %6$*5$" PFC_PFMT_d64 ", "
                                 "%%arg2 = %8$*7$u, "
                                 "%%arg1 = %10$*9$d\n",
                                 -48, arg5,
                                 arg4len, arg4,
                                 -36, arg3,
                                 30, arg2,
                                 -18, arg1));
            ASSERT_TRUE(ref2 != NULL);
            RefPointer  rp2(ref2);
            ASSERT_EQ(ref2, ref1);
            ASSERT_EQ(2U, ref1->pr_refcnt);

            pfc_refptr_t  *ref3(pfc_refptr_string_create(expected));
            ASSERT_TRUE(ref3 != NULL);
            RefPointer  rp3(ref3);
            ASSERT_EQ(ref3, ref1);
            ASSERT_EQ(3U, ref1->pr_refcnt);
        }
    }
}

/* test for pfc_refptr_string_length */
TEST(refptr, test_pfc_refptr_string_length)
{
    char str1[] = "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
    char str2[] = "";
    pfc_refptr_t *string_ptr1 = NULL;
    pfc_refptr_t *string_ptr2 = NULL;
    string_ptr1 = pfc_refptr_string_create(str1);
    string_ptr2 = pfc_refptr_string_create(str2);

    /* check object */
    EXPECT_TRUE(string_ptr1 != NULL);
    EXPECT_TRUE(string_ptr2 != NULL);

    /* test normal */
    ASSERT_EQ(strlen(str1), pfc_refptr_string_length(string_ptr1));
    ASSERT_EQ(strlen(str2), pfc_refptr_string_length(string_ptr2));

    pfc_refptr_put(string_ptr1);
    pfc_refptr_put(string_ptr2);
}

/* test for pfc_refptr_string_value */
TEST(refptr, test_pfc_refptr_string_value)
{
    char str1[] = "hogehoge";
    char str2[] = "";

    pfc_refptr_t *string_ptr1 = NULL;
    pfc_refptr_t *string_ptr2 = NULL;
    pfc_refptr_t *string_ptr3 = NULL;
    string_ptr1 = pfc_refptr_string_create(str1);
    string_ptr2 = pfc_refptr_string_create(str2);
    string_ptr3 = pfc_refptr_string_create("static string");

    /* check object */
    ASSERT_TRUE(string_ptr1 != NULL);
    ASSERT_TRUE(string_ptr2 != NULL);
    ASSERT_TRUE(string_ptr3 != NULL);

    /* test normal */
    ASSERT_EQ(0, strcmp(pfc_refptr_string_value(string_ptr1), str1));
    ASSERT_EQ(0, strcmp(pfc_refptr_string_value(string_ptr2), str2));
    ASSERT_EQ(0, strcmp(pfc_refptr_string_value(string_ptr3), "static string"));

    pfc_refptr_put(string_ptr1);
    pfc_refptr_put(string_ptr2);
    pfc_refptr_put(string_ptr3);
}

/* test for pfc_refptr_string_ops */
TEST(refptr, test_pfc_refptr_string_ops)
{
    char str1[] = "hogehoge";
    char str2[] = "hogehoge";
    char str3[] = "hoge";
    char str4[] = "hogehogehoge";

    pfc_refptr_t *string_ptr1 = NULL;
    pfc_refptr_t *string_ptr2 = NULL;
    pfc_refptr_t *string_ptr3 = NULL;
    pfc_refptr_t *string_ptr4 = NULL;
    string_ptr1 = pfc_refptr_string_create(str1);
    string_ptr2 = pfc_refptr_string_create(str2);
    string_ptr3 = pfc_refptr_string_create(str3);
    string_ptr4 = pfc_refptr_string_create(str4);

    /* check object */
    EXPECT_TRUE(string_ptr1 != NULL);
    EXPECT_TRUE(string_ptr2 != NULL);
    EXPECT_TRUE(string_ptr3 != NULL);
    EXPECT_TRUE(string_ptr4 != NULL);

    const pfc_refptr_ops_t *ops = pfc_refptr_string_ops();
    ASSERT_TRUE(ops != NULL);

    /* test ops->compare */
    ASSERT_EQ(0, ops->compare(string_ptr1->pr_object, string_ptr2->pr_object));
    ASSERT_LT(0, ops->compare(string_ptr1->pr_object, string_ptr3->pr_object));
    ASSERT_GT(0, ops->compare(string_ptr3->pr_object, string_ptr1->pr_object));
    ASSERT_GT(0, ops->compare(string_ptr1->pr_object, string_ptr4->pr_object));

    /* test ops->equals */
    ASSERT_EQ(PFC_TRUE,
              ops->equals(string_ptr1->pr_object, string_ptr2->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(string_ptr1->pr_object, string_ptr3->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(string_ptr3->pr_object, string_ptr1->pr_object));
    ASSERT_EQ(PFC_FALSE,
              ops->equals(string_ptr1->pr_object, string_ptr4->pr_object));

    /* test ops->hashfunc */
    ASSERT_TRUE(typeid(ops->hashfunc(string_ptr1->pr_object)) == 
                typeid(uint32_t));
    ASSERT_TRUE(typeid(ops->hashfunc(string_ptr2->pr_object)) == 
                typeid(uint32_t));
    ASSERT_TRUE(typeid(ops->hashfunc(string_ptr3->pr_object)) == 
                typeid(uint32_t));
    ASSERT_TRUE(typeid(ops->hashfunc(string_ptr4->pr_object)) == 
                typeid(uint32_t));

    ASSERT_EQ(ops->hashfunc(string_ptr1->pr_object),
              ops->hashfunc(string_ptr2->pr_object));

    pfc_refptr_put(string_ptr1);
    pfc_refptr_put(string_ptr2);
    pfc_refptr_put(string_ptr3);
    pfc_refptr_put(string_ptr4);

    /* check hash */
    {
        int i, j, count;
        count = TEST_NUM;
        pfc_refptr_t *string_ptr[count];
        const pfc_refptr_ops_t *ops = pfc_refptr_string_ops();
        EXPECT_TRUE(ops != NULL);

        for (i = 0; i < count; i++) {
            string_ptr[i] = pfc_refptr_string_create("foobarfoobar");
            EXPECT_TRUE(string_ptr[i] != NULL);
        }
        for (i = 0; i < count-1; i++) {
            for (j = i + 1; j < count; j++) {
                EXPECT_EQ(ops->hashfunc(string_ptr[i]->pr_object),
                          ops->hashfunc(string_ptr[j]->pr_object));
            }
        }
        for (i = 0; i < count; i++) {
            pfc_refptr_put(string_ptr[i]);
        }
    }

}

/*
 * A temporary thread for refptr string race condition test.
 */
class StringThread
    : public TempThread
{
public:
    StringThread(uint32_t nsec, bool simple=false, bool use_sprintf=false)
        : _simple(simple), _use_sprintf(use_sprintf)
    {
        _delay.tv_sec = 0;
        _delay.tv_nsec = nsec;
    }

    void	run(void);

private:
    void	stringTest(const char *string);

    bool                _simple;
    bool                _use_sprintf;
    pfc_timespec_t      _delay;
};

void
StringThread::run(void)
{
    for (;;) {
        if (_simple) {
            stringTest("race test 1");
        }
        else {
            stringTest("race test 1");
            stringTest("race test 2");
            stringTest("race test 3");
            stringTest("long string test --- "
                       "12345678901234567890123456789012345678901234567890");
            stringTest("race test 2");
            stringTest("race test 1");
        }

        lock();
        condTimedWait(&_delay);
        testStop();
        unlock();
    }
}

void
StringThread::stringTest(const char *string)
{
    pfc_refptr_t	*ref;

    if (_use_sprintf) {
        ref = pfc_refptr_sprintf("%s", string);
    }
    else {
        ref = pfc_refptr_string_create(string);
    }
    if (ref == NULL) {
        setError("Failed to create refptr string.");
        return;
    }

    uint32_t		cnt = ref->pr_refcnt;

    if (cnt == 0) {
        pfc_refptr_put(ref);
        std::ostringstream	stream;

        stream << "Invalid refcnt: " << cnt;
        setError(stream.str().c_str());
        return;
    }

    const char	*str(pfc_refptr_string_value(ref));
    if (strcmp(str, string) != 0) {
        pfc_refptr_put(ref);
        std::ostringstream	stream;

        stream << "Invalid string: " << str;
        setError(stream.str().c_str());
        return;
    }

    size_t	len(pfc_refptr_string_length(ref));
    size_t	rlen(strlen(string));
    if (len != rlen) {
        pfc_refptr_put(ref);
        std::ostringstream	stream;

        stream << "Invalid length: " << len << ": required=" << rlen;
        setError(stream.str().c_str());
    }

    pfc_refptr_put(ref);
}

#define	REFPTR_STRING_RACE_DURATION_MSEC	300

/* Race condition test for string creation and deletion. */
TEST(refptr, test_pfc_refptr_string_race)
{
    StringThread	thr1(1000000);
    StringThread	thr2(2000000);
    StringThread	thr3(3000000, false, true);
    StringThread	thr4(4000000, false, true);
    StringThread	thr5(5000000);
    StringThread	thr6(1300000, true);
    StringThread	thr7(1290000, true, true);
    StringThread	*threads[] = {
        &thr1, &thr2, &thr3, &thr4, &thr5, &thr6, &thr7
    };

    for (StringThread **tpp(threads); tpp < PFC_ARRAY_LIMIT(threads); tpp++) {
        StringThread	*tp = *tpp;

        ASSERT_EQ(0, tp->start());
    }

    pfc_timespec_t  delay;
    pfc_clock_msec2time(&delay, REFPTR_STRING_RACE_DURATION_MSEC);
    nanosleep(&delay, NULL);

    for (StringThread **tpp(threads); tpp < PFC_ARRAY_LIMIT(threads); tpp++) {
        StringThread	*tp = *tpp;

        ASSERT_EQ(0, tp->join());

#ifndef PFC_MODULE_BUILD
        ASSERT_FALSE(tp->hasError()) << "ERROR: " << tp->getError();
#else
        if (tp->hasError() == PFC_TRUE) {
            pfc_log_fatal("ERROR: %s", tp->getError().c_str());
        }
#endif
    }
}

/* fork(2) test while pfc_refptr_string_create() is running. */
TEST(refptr, test_pfc_refptr_string_race_fork)
{
    StringThread	thr1(1000000);
    StringThread	thr2(2000000);
    StringThread	thr3(3000000, false, true);
    StringThread	thr4(4000000, false, true);
    StringThread	thr5(5000000);
    StringThread	thr6(1300000, true);
    StringThread	thr7(1290000, true, true);
    StringThread	*threads[] = {
        &thr1, &thr2, &thr3, &thr4, &thr5, &thr6, &thr7
    };

    for (StringThread **tpp(threads); tpp < PFC_ARRAY_LIMIT(threads); tpp++) {
        StringThread	*tp = *tpp;

        ASSERT_EQ(0, tp->start());
    }

    pfc_timespec_t  duration, limit;
    pfc_clock_msec2time(&duration, REFPTR_STRING_RACE_DURATION_MSEC);
    ASSERT_EQ(0, pfc_clock_abstime(&limit, &duration));
    for (;;) {
        pfc_refptr_t	*r1(pfc_refptr_string_create("race test 1"));
        ASSERT_NE((pfc_refptr_t *)NULL, r1);

        pid_t	pid(fork());
        ASSERT_NE((pid_t)-1, pid);
        if (pid == 0) {
            // Ensure that we can create or delete refptr string.
            pfc_refptr_t	*r2(pfc_refptr_string_create("race test 2"));
            if (r2 == NULL) {
                _exit(1);
            }
            if (strcmp("race test 2", pfc_refptr_string_value(r2)) != 0) {
                _exit(2);
            }

            pfc_refptr_t	*r3(pfc_refptr_string_create("new string"));
            if (r3 == NULL) {
                _exit(3);
            }
            if (strcmp("new string", pfc_refptr_string_value(r3)) != 0) {
                _exit(4);
            }

            pfc_refptr_put(r1);
            pfc_refptr_put(r2);
            pfc_refptr_put(r3);

            _exit(0);
        }

        int	status;
        ASSERT_EQ(pid, waitpid(pid, &status, 0));
        ASSERT_TRUE(WIFEXITED(status));
        ASSERT_EQ(0, WEXITSTATUS(status));
        pfc_refptr_put(r1);

        pfc_timespec_t remains;
        int err(pfc_clock_isexpired(&remains, &limit));
        if (err == ETIMEDOUT) {
            break;
        }
        ASSERT_EQ(0, err);
    }

    for (StringThread **tpp(threads); tpp < PFC_ARRAY_LIMIT(threads); tpp++) {
        StringThread	*tp = *tpp;

        ASSERT_EQ(0, tp->join());

#ifndef PFC_MODULE_BUILD
        ASSERT_FALSE(tp->hasError()) << "ERROR: " << tp->getError();
#else
        if (tp->hasError() == PFC_TRUE) {
            pfc_log_fatal("ERROR: %s", tp->getError().c_str());
        }
#endif
    }
}

/* test for pfc_refptr_create */
TEST(refptr, test_pfc_refptr_create)
{
    /* set up */
    test_t *test1 = NULL;
    test_t *test2 = NULL;
    test_t *test3 = NULL;
    pfc_refptr_t *ptr1 = NULL;
    pfc_refptr_t *ptr2 = NULL;
    pfc_refptr_t *ptr3 = NULL;

    /* malloc and check structure */
    test1 = (test_t *)malloc(sizeof(test_t));
    test2 = (test_t *)malloc(sizeof(test_t));
    test3 = (test_t *)malloc(sizeof(test_t));
    EXPECT_TRUE(test1 != NULL);
    EXPECT_TRUE(test2 != NULL);
    EXPECT_TRUE(test3 != NULL);

    /* create refptr and test pointer */
    ptr1 = pfc_refptr_create(&test_ops, test1);
    ptr2 = pfc_refptr_create(&test_null_ops, test2);
    ptr3 = pfc_refptr_create(NULL, test3);
    ASSERT_TRUE(ptr1 != NULL);
    ASSERT_TRUE(ptr2 != NULL);
    ASSERT_TRUE(ptr3 != NULL);

    /* test value */
    ASSERT_TRUE(pfc_refptr_value(ptr1) == test1);
    ASSERT_TRUE(pfc_refptr_value(ptr2) == test2);
    ASSERT_TRUE(pfc_refptr_value(ptr3) == test3);

    /* test ref counter */
    ASSERT_EQ(1U, ptr1->pr_refcnt);
    ASSERT_EQ(1U, ptr2->pr_refcnt);
    ASSERT_EQ(1U, ptr3->pr_refcnt);

    /* clean up */
    pfc_refptr_put(ptr1);
    pfc_refptr_put(ptr2);
    pfc_refptr_put(ptr3);
    free((test_t *) test2);
    free((test_t *) test3);
}

/* test for pfc_refptr_value */
TEST(refptr, test_pfc_refptr_value)
{
    /* set up */
    test_t *test = NULL;
    pfc_refptr_t *ptr = NULL;

    /* malloc and check structure */
    test = (test_t *)malloc(sizeof(test_t));
    EXPECT_TRUE(test != NULL);

    /* create refptr and check pointer, ref counter */
    ptr = pfc_refptr_create(&test_ops, test);
    EXPECT_TRUE(ptr != NULL);
    EXPECT_EQ(1U, ptr->pr_refcnt);

    /* test value */
    ASSERT_TRUE(pfc_refptr_value(ptr) == test);
    ASSERT_TRUE(ptr->pr_object == test);
    ASSERT_TRUE(pfc_refptr_value(ptr) == ptr->pr_object);

    /* clean up */
    pfc_refptr_put(ptr);
}

/* test for TEST_PFC_REFPTR_VALUE */
TEST(refptr, TEST_PFC_REFPTR_VALUE)
{
    /* set up */
    test_t *test = NULL;
    pfc_refptr_t *ptr = NULL;

    /* malloc and check structure */
    test = (test_t *)malloc(sizeof(test_t));
    EXPECT_TRUE(test != NULL);

    /* create refptr and check pointer, ref counter */
    ptr = pfc_refptr_create(&test_ops, test);
    EXPECT_TRUE(ptr != NULL);
    EXPECT_EQ(1U, ptr->pr_refcnt);

    /* test value */
    ASSERT_TRUE(PFC_REFPTR_VALUE((ptr), test_t *) == test);
    ASSERT_TRUE(ptr->pr_object == test);
    ASSERT_TRUE(PFC_REFPTR_VALUE((ptr), test_t *) == ptr->pr_object);

    /* clean up */
    pfc_refptr_put(ptr);
}

/* test for pfc_refptr_operation */
TEST(refptr, test_pfc_refptr_operation)
{
    /* set up */
    test_t *test1 = NULL;
    test_t *test2 = NULL;
    test_t *test3 = NULL;
    test_t *test4 = NULL;
    pfc_refptr_t *ptr1 = NULL;
    pfc_refptr_t *ptr2 = NULL;
    pfc_refptr_t *ptr3 = NULL;
    pfc_refptr_t *ptr4 = NULL;

    /* malloc and check  structure */
    test1 = (test_t *)malloc(sizeof(test_t));
    test2 = (test_t *)malloc(sizeof(test_t));
    test3 = (test_t *)malloc(sizeof(test_t));
    test4 = (test_t *)malloc(sizeof(test_t));
    EXPECT_TRUE(test1 != NULL);
    EXPECT_TRUE(test2 != NULL);
    EXPECT_TRUE(test3 != NULL);
    EXPECT_TRUE(test4 != NULL);
    test1->num = 1;
    test2->num = 1;
    test3->num = 0;
    test4->num = 2;

    /* create refptr and check pointer, ref counter */
    ptr1 = pfc_refptr_create(&test_ops, test1);
    ptr2 = pfc_refptr_create(&test_ops, test2);
    ptr3 = pfc_refptr_create(&test_ops, test3);
    ptr4 = pfc_refptr_create(&test_ops, test4);
    EXPECT_TRUE(ptr1 != NULL);
    EXPECT_TRUE(ptr2 != NULL);
    EXPECT_TRUE(ptr3 != NULL);
    EXPECT_TRUE(ptr4 != NULL);
    EXPECT_EQ(1U, ptr1->pr_refcnt);
    EXPECT_EQ(1U, ptr2->pr_refcnt);
    EXPECT_EQ(1U, ptr3->pr_refcnt);
    EXPECT_EQ(1U, ptr4->pr_refcnt);

    /* test operation */
    ASSERT_TRUE(pfc_refptr_operation(ptr1) != NULL);
    ASSERT_TRUE(pfc_refptr_operation(ptr2) != NULL);
    ASSERT_TRUE(pfc_refptr_operation(ptr3) != NULL);
    ASSERT_TRUE(pfc_refptr_operation(ptr4) != NULL);
    /* test compare operation */
    ASSERT_EQ(0, pfc_refptr_operation(ptr1)->compare(pfc_refptr_value(ptr1),
                                                     pfc_refptr_value(ptr2)));
    ASSERT_EQ(-1, pfc_refptr_operation(ptr1)->compare(pfc_refptr_value(ptr3),
                                                      pfc_refptr_value(ptr1)));
    ASSERT_EQ(1, pfc_refptr_operation(ptr1)->compare(pfc_refptr_value(ptr4),
                                                     pfc_refptr_value(ptr1)));
    /* test equals operation */
    ASSERT_EQ(PFC_TRUE, pfc_refptr_operation(ptr1)->equals(pfc_refptr_value(ptr1),
                                                           pfc_refptr_value(ptr2)));
    ASSERT_EQ(PFC_FALSE, pfc_refptr_operation(ptr1)->equals(pfc_refptr_value(ptr1),
                                                            pfc_refptr_value(ptr3)));
    ASSERT_EQ(PFC_FALSE, pfc_refptr_operation(ptr1)->equals(pfc_refptr_value(ptr1),
                                                            pfc_refptr_value(ptr4)));
    /* test hashfunc operation */
    ASSERT_EQ(UINT32_MAX, pfc_refptr_operation(ptr1)->hashfunc(ptr1));

    /* clean up */
    pfc_refptr_put(ptr1);
    pfc_refptr_put(ptr2);
    pfc_refptr_put(ptr3);
    pfc_refptr_put(ptr4);
}

/*
 * test for pfc_refptr_get
 * single thread test
 */
TEST(refptr, test_pfc_refptr_get)
{
    /* set up */
    test_t *test = NULL;
    pfc_refptr_t *ptr = NULL;
    int i;

    /* malloc and check structure */
    test = (test_t *)malloc(sizeof(test_t));
    EXPECT_TRUE(test != NULL);

    /* create refptr and check pointer, ref counter */
    ptr = pfc_refptr_create(&test_ops, test);
    EXPECT_TRUE(ptr != NULL);
    EXPECT_EQ(1U, ptr->pr_refcnt);

    /* test to get refptr */
    for (i = 1; i < TEST_NUM; i++) {
        pfc_refptr_get(ptr);
        ASSERT_EQ((uint32_t)(i + 1), ptr->pr_refcnt);
    }

    /* clean up */
    while (i > 0) {
        pfc_refptr_put(ptr);
        i--;
    }
}

/*
 * test for pfc_refptr_string_ops
 * single thread test
 */
TEST(refptr, test_pfc_refptr_put)
{
    /* set up */
    test_t *test = NULL;
    pfc_refptr_t *ptr = NULL;
    int i;

    /* malloc and check structure */
    test = (test_t *)malloc(sizeof(test_t));
    EXPECT_TRUE(test != NULL);

    /* create refptr and check pointer, ref counter */
    ptr = pfc_refptr_create(&test_ops, test);
    EXPECT_TRUE(ptr != NULL);
    EXPECT_EQ(1U, ptr->pr_refcnt);

    /* get refptr and check counter */
    i = ptr->pr_refcnt;
    while (i < TEST_NUM) {
        pfc_refptr_get(ptr);
        EXPECT_EQ((uint32_t)++i, ptr->pr_refcnt);
    }

    /* put refptr and test counter */
    i = ptr->pr_refcnt;
    while (i > 1) {
        pfc_refptr_put(ptr);
        ASSERT_EQ((uint32_t)--i, ptr->pr_refcnt);
    }

    /* clean up */
    check_dtor = 0;
    pfc_refptr_put(ptr);
    ASSERT_EQ(1, check_dtor);
}

/* test for pfc_refptr_equals */
TEST(refptr, test_pfc_refptr_equals)
{
    { /* same pointer case */
        /* set up */
        test_t *test1 = NULL;
        test_t *test2 = NULL;
        pfc_refptr_t *ptr1 = NULL;
        pfc_refptr_t *ptr2 = NULL;
        pfc_refptr_t *ptr_copy1 = NULL;
        pfc_refptr_t *ptr_copy2 = NULL;

        /* malloc and check test structure */
        test1 = (test_t *)malloc(sizeof(test_t));
        test2 = (test_t *)malloc(sizeof(test_t));
        EXPECT_TRUE(test1 != NULL);
        EXPECT_TRUE(test2 != NULL);
        test1->num = 1;
        test2->num = 2;

        /* create refptr and check pointer, ref counter */
        ptr1 = pfc_refptr_create(&test_ops, test1);
        ptr2 = pfc_refptr_create(NULL, test2);
        EXPECT_TRUE(ptr1 != NULL);
        EXPECT_TRUE(ptr2 != NULL);
        EXPECT_EQ(1U, ptr1->pr_refcnt);
        EXPECT_EQ(1U, ptr2->pr_refcnt);

        /* copy pointer */
        pfc_refptr_get(ptr1);
        pfc_refptr_get(ptr2);
        ptr_copy1 = ptr1;
        ptr_copy2 = ptr2;

        /* test two same pointers */
        ASSERT_EQ(PFC_TRUE, pfc_refptr_equals(ptr1, ptr_copy1));
        ASSERT_EQ(PFC_TRUE, pfc_refptr_equals(ptr2, ptr_copy2));
        pfc_refptr_put(ptr1);
        pfc_refptr_put(ptr2);
        ptr_copy1 = NULL;
        ptr_copy2 = NULL;

        /* clean up */
        pfc_refptr_put(ptr1);
        pfc_refptr_put(ptr2);
        free(test2);
    }
    { /* different pointer case */
        /* set up */
        test_t *test1 = NULL;
        test_t *test2 = NULL;
        pfc_refptr_t *ptr1 = NULL;
        pfc_refptr_t *ptr2 = NULL;
        pfc_refptr_t *ptr3 = NULL;
        pfc_refptr_t *ptr4 = NULL;

        /* malloc and check test structure */
        test1 = (test_t *)malloc(sizeof(test_t));
        test2 = (test_t *)malloc(sizeof(test_t));
        EXPECT_TRUE(test1 != NULL);
        EXPECT_TRUE(test2 != NULL);
        test1->num = 1;
        test2->num = 2;

        /* create refptr and check pointer, ref counter */
        ptr1 = pfc_refptr_create(&test_ops, test1);
        ptr2 = pfc_refptr_create(&test_ops, test2);
        ptr3 = pfc_refptr_create(NULL, test1);
        ptr4 = pfc_refptr_create(NULL, test2);
        EXPECT_TRUE(ptr1 != NULL);
        EXPECT_TRUE(ptr2 != NULL);
        EXPECT_TRUE(ptr3 != NULL);
        EXPECT_TRUE(ptr4 != NULL);
        EXPECT_EQ(1U, ptr1->pr_refcnt);
        EXPECT_EQ(1U, ptr2->pr_refcnt);
        EXPECT_EQ(1U, ptr3->pr_refcnt);
        EXPECT_EQ(1U, ptr4->pr_refcnt);

        /* test two different pointers */
        /* different value */
        ASSERT_EQ(PFC_FALSE, pfc_refptr_equals(ptr1, ptr2));
        ASSERT_EQ(PFC_FALSE, pfc_refptr_equals(ptr3, ptr4));
        /* different ops */
        ASSERT_EQ(PFC_FALSE, pfc_refptr_equals(ptr1, ptr3));

        /* clean up */
        pfc_refptr_put(ptr1);
        pfc_refptr_put(ptr2);
        pfc_refptr_put(ptr3);
        pfc_refptr_put(ptr4);
    }
}
