/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * refptr tests for vector
 */

#include <pfc/listmodel.h>
#include "test_listm_cmn_ref.hh"


/*
 * Test cases
 */

TEST(listm, vector_create_ref)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        pfc_listm_destroy(list);
    }
}


TEST(listm, vector_mismatching_refops_error)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        // create vector wiht refptr
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        // do test
        test_listm_mismatching_refops_error(list, *refops_p);

        // clean up
        pfc_listm_destroy(list);
    }
}


TEST(listm, vector_push_and_push_tail_refptr)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        // create vector wiht refptr
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        // do test
        test_listm_push_and_push_tail_refptr(list, *refops_p);

        // clean up
        pfc_listm_destroy(list);
    }
}


TEST(listm, vector_pop_refptr)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        // create vector wiht refptr
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        // do test
        test_listm_pop_refptr(list, *refops_p);

        // clean up
        pfc_listm_destroy(list);
    }
}


TEST(listm, vector_remove_refptr)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        // create vector wiht refptr
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        // do test
        test_listm_remove_refptr(list, *refops_p);

        // clean up
        pfc_listm_destroy(list);
    }
}


TEST(listm, vector_clear_refptr)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        // create vector wiht refptr
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        // do test
        test_listm_clear_refptr(list, *refops_p);

        // clean up
        pfc_listm_destroy(list);
    }
}


TEST(listm, vector_destroy_refptr)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        for (pfc_bool_t test_dtor = 0; test_dtor < 2; ++test_dtor) {
            // create vector wiht refptr
            pfc_listm_t list;
            EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

            // do test
            test_listm_destroy_refptr(list, *refops_p, test_dtor);

            // clean up is not necessary because list has been
            // already destroyed.
        }
    }
}


TEST(listm, vector_first_refptr)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        // create vector wiht refptr
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        // do test
        test_listm_first_refptr(list, *refops_p);

        // clean up
        pfc_listm_destroy(list);
    }
}


TEST(listm, vector_getat_refptr)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        // create vector wiht refptr
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        // do test
        test_listm_getat_refptr(list, *refops_p);

        // clean up
        pfc_listm_destroy(list);
    }
}


TEST(listm, vector_sort_comp_refptr)
{
    for (const pfc_refptr_ops_t **refops_p = listm_refops_set;
         *refops_p != END_OF_REFOPS;
         ++refops_p)
    {
        // create vector wiht refptr
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE_REF(&list, *refops_p));

        // do test
        test_listm_sort_comp_refptr(list, *refops_p);

        // clean up
        pfc_listm_destroy(list);
    }
}
