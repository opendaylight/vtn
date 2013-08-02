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

#include <pfc/listmodel.h>
#include <pfc/thread.h>
#include "test_listm_cmn_mt.hh"


/*
 * Test cases
 */

TEST(listm, vector_get_size_with_threads)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_get_size_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_first_with_threads)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_first_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_getat_with_threads)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_getat_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_pop_with_threads)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_pop_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_nolimit_with_threads)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_nolimit_with_threads(list);

    // clean up
    pfc_listm_destroy(list);
}
