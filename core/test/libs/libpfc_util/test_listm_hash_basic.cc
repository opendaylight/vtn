/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * basic tests for hash list
 */

#include <pfc/listmodel.h>
#include "test_listm_cmn_basic.hh"


/*
 * constants
 */

static const pfc_hash_ops_t *hash_ops = NULL;
static const uint32_t nbuckets = 31;
static const uint32_t hflags = 0;


/*
 * Test cases
 */

TEST(listm, hashlist_create_destroy)
{
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    pfc_listm_destroy(list);
}


TEST(listm, hashlist_clear)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_clear(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_push)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_push(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_push_tail)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_push_tail(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_pop)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_pop(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_remove)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_remove(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_first)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_first(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_get_size)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_get_size(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_sort_comp)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_sort_comp(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_iter_create)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listiter_create(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_iter_next_after_update)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listiter_next_after_update(list, LISTM_IMPL_HASHLIST);

    // In this case, list is destroyed in test routine
}


TEST(listm, hashlist_iter_next)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listiter_next(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_iter_destroy)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listiter_destroy(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}
