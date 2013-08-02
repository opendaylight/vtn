/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for hash test utilities
 */

#include "hash_util.h"
#include "test_hash.hh"


TEST(hash_util, wtchops_kv_nofree_count_equals)
{
    // same pointers (non-NULL)
    {
        int x = 0;

        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        EXPECT_EQ(PFC_TRUE, hash_wtchops_kv_nofree->equals(&x, &x));

        EXPECT_EQ(old_count_equals + 1,    wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,      wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor,      wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref, wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor,      wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref, wtchops_get_count_val_dtor_vref());
    }

    // different pointers
    {
        int x = 0;
        int y = 0;

        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        EXPECT_EQ(PFC_FALSE, hash_wtchops_kv_nofree->equals(&x, &y));

        EXPECT_EQ(old_count_equals + 1,    wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,      wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor,      wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref, wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor,      wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref, wtchops_get_count_val_dtor_vref());
    }
}


TEST(hash_util, wtchops_kv_nofree_count_hashfunc)
{
    uint64_t old_count_equals        = wtchops_get_count_equals();
    uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
    uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
    uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
    uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
    uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

    hash_wtchops_kv_nofree->hashfunc(NULL);

    EXPECT_EQ(old_count_equals,        wtchops_get_count_equals());
    EXPECT_EQ(old_count_hashfunc + 1,  wtchops_get_count_hashfunc());
    EXPECT_EQ(old_count_key_dtor,      wtchops_get_count_key_dtor());
    EXPECT_EQ(old_count_key_dtor_kref, wtchops_get_count_key_dtor_kref());
    EXPECT_EQ(old_count_val_dtor,      wtchops_get_count_val_dtor());
    EXPECT_EQ(old_count_val_dtor_vref, wtchops_get_count_val_dtor_vref());
}


TEST(hash_util, wtchops_kv_nofree_count_key_dtor)
{
    // no flags
    {
        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        hash_wtchops_kv_nofree->key_dtor(NULL, 0);

        EXPECT_EQ(old_count_equals,        wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,      wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor + 1,  wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref, wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor,      wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref, wtchops_get_count_val_dtor_vref());
    }

    // with VREF flag
    {
        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        hash_wtchops_kv_nofree->key_dtor(NULL, PFC_HASHOP_VAL_REFPTR);

        EXPECT_EQ(old_count_equals,        wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,      wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor + 1,  wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref, wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor,      wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref, wtchops_get_count_val_dtor_vref());
    }
}


TEST(hash_util, wtchops_kv_nofree_count_key_dtor_kref)
{
    // with KREF flag
    {
        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        hash_wtchops_kv_nofree->key_dtor(NULL, PFC_HASHOP_KEY_REFPTR);

        EXPECT_EQ(old_count_equals,            wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,          wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor      + 1, wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref + 1, wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor,          wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref,     wtchops_get_count_val_dtor_vref());
    }

    // with KREF and VREF flags
    {
        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        hash_wtchops_kv_nofree->key_dtor(NULL, PFC_HASHOP_KEYVAL_REFPTR);

        EXPECT_EQ(old_count_equals,            wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,          wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor      + 1, wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref + 1, wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor,          wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref,     wtchops_get_count_val_dtor_vref());
    }
}


TEST(hash_util, wtchops_kv_nofree_count_val_dtor)
{
    // no flags
    {
        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        hash_wtchops_kv_nofree->value_dtor(NULL, 0);

        EXPECT_EQ(old_count_equals,        wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,      wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor,      wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref, wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor + 1,  wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref, wtchops_get_count_val_dtor_vref());
    }

    // with KREF flag
    {
        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        hash_wtchops_kv_nofree->value_dtor(NULL, PFC_HASHOP_KEY_REFPTR);

        EXPECT_EQ(old_count_equals,        wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,      wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor,      wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref, wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor + 1,  wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref, wtchops_get_count_val_dtor_vref());
    }
}


TEST(hash_util, wtchops_kv_nofree_count_val_dtor_vref)
{
    // with VREF flag
    {
        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        hash_wtchops_kv_nofree->value_dtor(NULL, PFC_HASHOP_VAL_REFPTR);

        EXPECT_EQ(old_count_equals,            wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,          wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor,          wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref,     wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor      + 1, wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref + 1, wtchops_get_count_val_dtor_vref());
    }

    // with KREF and VREF flags
    {
        uint64_t old_count_equals        = wtchops_get_count_equals();
        uint64_t old_count_hashfunc      = wtchops_get_count_hashfunc();
        uint64_t old_count_key_dtor      = wtchops_get_count_key_dtor();
        uint64_t old_count_key_dtor_kref = wtchops_get_count_key_dtor_kref();
        uint64_t old_count_val_dtor      = wtchops_get_count_val_dtor();
        uint64_t old_count_val_dtor_vref = wtchops_get_count_val_dtor_vref();

        hash_wtchops_kv_nofree->value_dtor(NULL, PFC_HASHOP_KEYVAL_REFPTR);

        EXPECT_EQ(old_count_equals,            wtchops_get_count_equals());
        EXPECT_EQ(old_count_hashfunc,          wtchops_get_count_hashfunc());
        EXPECT_EQ(old_count_key_dtor,          wtchops_get_count_key_dtor());
        EXPECT_EQ(old_count_key_dtor_kref,     wtchops_get_count_key_dtor_kref());
        EXPECT_EQ(old_count_val_dtor      + 1, wtchops_get_count_val_dtor());
        EXPECT_EQ(old_count_val_dtor_vref + 1, wtchops_get_count_val_dtor_vref());
    }
}


TEST(hash_util, wtchops_kv_nofree_lastaddr_key_dtor)
{
    // NULL address
    {
        hash_wtchops_kv_nofree->key_dtor(NULL, 0);
        EXPECT_EQ((pfc_cptr_t)NULL, wtchops_get_lastaddr_key_dtor());
    }

    // non-NULL address
    {
        int x = 10;
        hash_wtchops_kv_nofree->key_dtor((pfc_ptr_t)&x, 0);
        EXPECT_EQ((pfc_ptr_t)&x, wtchops_get_lastaddr_key_dtor());
    }
}


TEST(hash_util, wtchops_kv_nofree_lastaddr_val_dtor)
{
    // NULL address
    {
        hash_wtchops_kv_nofree->value_dtor(NULL, 0);
        EXPECT_EQ((pfc_cptr_t)NULL, wtchops_get_lastaddr_val_dtor());
    }

    // non-NULL address
    {
        int x = 10;
        hash_wtchops_kv_nofree->value_dtor((pfc_ptr_t)&x, 0);
        EXPECT_EQ((pfc_ptr_t)&x, wtchops_get_lastaddr_val_dtor());
    }
}


TEST(hash_util, intref_count_dtor)
{
    int x = 10;

    // put just after creating
    {
        pfc_refptr_t *iref = intref_create(x);
        EXPECT_NE((pfc_refptr_t *)NULL, iref);

        // get old count
        uint64_t old_count_dtor = intref_get_count_dtor();

        // do put (refcnt: 1 -> 0)
        pfc_refptr_put(iref);

        // check
        EXPECT_EQ(old_count_dtor + 1, intref_get_count_dtor());
    }

    // put after getting
    {
        pfc_refptr_t *iref = intref_create(x);
        EXPECT_NE((pfc_refptr_t *)NULL, iref);

        // get old count
        uint64_t old_count_dtor = intref_get_count_dtor();

        // do get (refcnt: 1 -> 2)
        pfc_refptr_get(iref);

        // check
        EXPECT_EQ(old_count_dtor, intref_get_count_dtor());

        // do put (refcnt: 2 -> 1)
        pfc_refptr_put(iref);

        // check
        EXPECT_EQ(old_count_dtor, intref_get_count_dtor());

        // do put (refcnt: 1 -> 0)
        pfc_refptr_put(iref);

        // check
        EXPECT_EQ(old_count_dtor + 1, intref_get_count_dtor());
    }
}


TEST(hash_util, intref_lastaddr_dtor)
{
    int x = 10;

    // put just after creating
    {
        pfc_refptr_t *iref = intref_create(x);
        EXPECT_NE((pfc_refptr_t *)NULL, iref);

        // get object address
        pfc_cptr_t obj_addr = PFC_REFPTR_VALUE(iref, pfc_cptr_t);

        // do put (refcnt: 1 -> 0)
        pfc_refptr_put(iref);

        // check
        EXPECT_EQ(obj_addr, intref_get_lastaddr_dtor());
    }

    // put after getting
    {
        pfc_refptr_t *iref = intref_create(x);
        EXPECT_NE((pfc_refptr_t *)NULL, iref);

        // get object address
        pfc_cptr_t obj_addr = PFC_REFPTR_VALUE(iref, pfc_cptr_t);

        // do get (refcnt: 1 -> 2)
        pfc_refptr_get(iref);

        // do put (refcnt: 2 -> 1)
        pfc_refptr_put(iref);

        // do put (refcnt: 1 -> 0)
        pfc_refptr_put(iref);

        // check
        EXPECT_EQ(obj_addr, intref_get_lastaddr_dtor());
    }
}
