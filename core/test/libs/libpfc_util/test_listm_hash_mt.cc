/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * multi-thread tests for hash list
 */

#include <pfc/listmodel.h>
#include <pfc/thread.h>
#include "test_listm_cmn_mt.hh"


/*
 * constants
 */

static const pfc_hash_ops_t *hash_ops = NULL;
static const uint32_t nbuckets = 31;
static const uint32_t hflags = 0;


/*
 * Test cases
 */

TEST(listm, hashlist_get_size_with_threads)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_get_size_with_threads(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_first_with_threads)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_first_with_threads(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_getat_with_threads)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_getat_with_threads(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_pop_with_threads)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_pop_with_threads(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, hashlist_nolimit_with_threads)
{
    // create hash list
    pfc_listm_t list;
    EXPECT_EQ(0, pfc_hashlist_create(&list, hash_ops, nbuckets, hflags));

    // do test
    test_listm_nolimit_with_threads(list, LISTM_IMPL_HASHLIST);

    // clean up
    pfc_listm_destroy(list);
}
