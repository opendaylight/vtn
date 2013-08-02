/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * basic tests for vector
 */

#include <pfc/listmodel.h>
#include "test_listm_cmn_basic.hh"


/*
 * Test cases
 */

TEST(listm, llist_create_destroy)
{
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    pfc_listm_destroy(list);
}


TEST(listm, llist_clear)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_clear(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_push)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_push(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_push_tail)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_push_tail(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_pop)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_pop(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_remove)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_remove(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_first)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_first(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_get_size)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_get_size(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_sort_comp)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listm_sort_comp(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_iter_create)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listiter_create(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_iter_next_after_update)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listiter_next_after_update(list);

    // In this case, list is destroyed in test routine
}


TEST(listm, llist_iter_next)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listiter_next(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, llist_iter_destroy)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_llist_create(&list));

    // do test
    test_listiter_destroy(list);

    // clean up
    pfc_listm_destroy(list);
}
