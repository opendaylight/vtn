/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * basic tests for linked list
 */

#include <pfc/listmodel.h>
#include "pseudo_rand.hh"
#include "string_util.h"
#include "test_listm_cmn_basic.hh"


/*
 *
 * constants
 *
 */

const pfc_cptr_t INVALID_PTR = (pfc_cptr_t)-123;


/*
 *
 * local utility functions
 *
 */

static pfc_ptr_t *
get_random_strings (int nents)
{
    prand_generator_t prand_gen = prand_create(LISTM_RANDOM_SEED);
    pfc_ptr_t *vals = (pfc_ptr_t *)calloc(nents, sizeof(pfc_ptr_t));
    if (vals == NULL) abort();
    for (int i = 0; i < nents; ++i) {
        vals[i] = (pfc_ptr_t)random_string_with_prand_gen(prand_gen,
                                                          100, '!', '~');
        if (vals[i] == NULL) abort();
    }
    prand_destroy(prand_gen);
    return vals;
}


static void
free_random_strings (pfc_ptr_t *vals, int nents)
{
    for (int i = 0; i < nents; ++i) {
        free(vals[i]);
    }
    free(vals);
}


typedef int vector_add_mode_t;
static const vector_add_mode_t VECTOR_ADD_MODE_HEAD     = 0;
static const vector_add_mode_t VECTOR_ADD_MODE_TAIL     = 1;
static const vector_add_mode_t VECTOR_ADD_MODE_MIX      = 2;
static const vector_add_mode_t VECTOR_ADD_MODE_END_MARK = 3;

void
static add_ents (pfc_listm_t list, pfc_ptr_t *vals, int nents,
                 vector_add_mode_t add_mode = VECTOR_ADD_MODE_MIX)
{
    int size_at_beginning = pfc_listm_get_size(list);

    for (int i = 0; i < nents; ++i) {
        vector_add_mode_t add_action;
        if (add_mode == VECTOR_ADD_MODE_MIX) {
            add_action = (vector_add_mode_t)(i % 2);
        } else {
            add_action = add_mode;
        }

        switch (add_action) {
            case VECTOR_ADD_MODE_HEAD: {
                // add to head
                EXPECT_EQ(0, pfc_listm_push(list, vals[i]));

                // check size
                int expected_size = size_at_beginning + i + 1;
                EXPECT_EQ(expected_size, pfc_listm_get_size(list));

                // check by first
                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(0, pfc_listm_first(list, &v));
                EXPECT_EQ(vals[i], v);

                // check by getat
                v = INVALID_PTR;
                EXPECT_EQ(0, pfc_listm_getat(list, 0, &v));
                EXPECT_EQ(vals[i], v);

                break;
            }
            case VECTOR_ADD_MODE_TAIL: {
                // add to tail
                EXPECT_EQ(0, pfc_listm_push_tail(list, vals[i]));

                // check size
                int expected_size = size_at_beginning + i + 1;
                EXPECT_EQ(expected_size, pfc_listm_get_size(list));

                // check by getat
                int tail_pos = size_at_beginning + i;
                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(0, pfc_listm_getat(list, tail_pos, &v));
                EXPECT_EQ(vals[i], v);

                break;
            }
            default: abort();
        }
    }
}


static void
check_ents (pfc_listm_t list, pfc_ptr_t *vals, int nents,
            vector_add_mode_t add_mode = VECTOR_ADD_MODE_MIX)
{
    int size = pfc_listm_get_size(list);
    EXPECT_EQ(size, nents);

    if (add_mode == VECTOR_ADD_MODE_MIX) {
        // skip if add_mode is MIX
        return;
    }

    switch (add_mode) {
        case VECTOR_ADD_MODE_HEAD:
            for (int i = 0; i  < nents; ++i) {
                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(0, pfc_listm_getat(list, i, &v));
                EXPECT_EQ(vals[nents - i - 1], v);
            }
            break;
        case VECTOR_ADD_MODE_TAIL: {
            for (int i = 0; i < nents; ++i) {
                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(0, pfc_listm_getat(list, i, &v));
                EXPECT_EQ(vals[i], v);
            }
            break;
        }
        default: abort();
    }

}


static void
del_ents (pfc_listm_t list, int nents)
{
    int size_at_beginning = pfc_listm_get_size(list);
    if (size_at_beginning < nents) abort();

    for (int i = 0; i < nents; ++i) {
        switch (i % 3) {
            case 0: {
                // delete from head
                EXPECT_EQ(0, pfc_listm_pop(list, NULL));

                // check size
                int expected_size = size_at_beginning - (i + 1);
                EXPECT_EQ(expected_size, pfc_listm_get_size(list));

                break;
            }
            case 1: {
                // delete from middle
                int idx = pfc_listm_get_size(list) / 2;
                EXPECT_GE(idx, 0);
                pfc_cptr_t v;
                EXPECT_EQ(0, pfc_listm_getat(list, idx, &v));
                EXPECT_EQ(0, pfc_listm_remove(list, v));

                // check size
                int expected_size = size_at_beginning - (i + 1);
                EXPECT_EQ(expected_size, pfc_listm_get_size(list));

                break;
            }
            case 2: {
                // delete from tail
                int idx = pfc_listm_get_size(list) - 1;
                EXPECT_GE(idx, 0);
                pfc_cptr_t v;
                EXPECT_EQ(0, pfc_listm_getat(list, idx, &v));
                EXPECT_EQ(0, pfc_listm_remove(list, v));

                // check size
                int expected_size = size_at_beginning - (i + 1);
                EXPECT_EQ(expected_size, pfc_listm_get_size(list));

                break;
            }
            default: abort();
        }
    }
}



/*
 * Test cases
 */

TEST(listm, vector_create_destroy)
{
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    pfc_listm_destroy(list);
}


TEST(listm, vector_get_capacity)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // create test data
    int base_cap_unit = MAX(PFC_VECTOR_DEFAULT_CAPACITY,
                            PFC_VECTOR_DEFAULT_CAPINC);
    int add_ents_num = base_cap_unit * 13 + base_cap_unit / 7;
    int del_ents_num = base_cap_unit * 5 + base_cap_unit / 3;
    pfc_ptr_t *vals = get_random_strings(add_ents_num);

    // check default capacity
    EXPECT_GE(pfc_vector_get_capacity(list), PFC_VECTOR_DEFAULT_CAPACITY);

    // add elements
    add_ents(list, vals, add_ents_num);
    EXPECT_GE(pfc_vector_get_capacity(list), add_ents_num);
    EXPECT_LE(pfc_vector_get_capacity(list) - add_ents_num,
              PFC_VECTOR_DEFAULT_CAPINC * 2);

    // delete elements
    del_ents(list, del_ents_num);
    EXPECT_GE(pfc_vector_get_capacity(list), add_ents_num - del_ents_num);

    // clean up
    pfc_listm_destroy(list);
    free_random_strings(vals, add_ents_num);
}


TEST(listm, vector_set_capacity_error)
{
    // create test data
    int base_cap_unit = MAX(PFC_VECTOR_DEFAULT_CAPACITY,
                            PFC_VECTOR_DEFAULT_CAPINC);
    int nents = base_cap_unit * 13 + base_cap_unit / 7;
    pfc_ptr_t *vals = get_random_strings(nents);

    for (vector_add_mode_t add_mode = VECTOR_ADD_MODE_HEAD;
         add_mode < VECTOR_ADD_MODE_END_MARK;
         ++add_mode)
    {
        // create vector
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

        // fill elements into vector
        add_ents(list, vals, nents, add_mode);
        int old_cap = pfc_vector_get_capacity(list);
        EXPECT_GE(old_cap, nents);

        // capacities are under minimum
        {
            // try to set zero capacity
            EXPECT_NE(0, pfc_vector_set_capacity(list, 0));
            EXPECT_EQ(pfc_vector_get_capacity(list), old_cap);

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);

            // try to set negative capacity
            EXPECT_NE(0, pfc_vector_set_capacity(list, -1));
            EXPECT_EQ(pfc_vector_get_capacity(list), old_cap);

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);
        }

        // capacities are over maximum
        {
            // try to set MAX + 1
            EXPECT_NE(0, pfc_vector_set_capacity(list,
                                                 PFC_VECTOR_MAX_CAPACITY + 1));
            EXPECT_EQ(pfc_vector_get_capacity(list), old_cap);

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);

            // try to set MAX + 2
            EXPECT_NE(0, pfc_vector_set_capacity(list, PFC_VECTOR_MAX_CAPACITY
                                                 + 2));
            EXPECT_EQ(pfc_vector_get_capacity(list), old_cap);

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);
        }

        // capacities is less than elements in vector
        {
            // try to set nents -1
            EXPECT_NE(0, pfc_vector_set_capacity(list, nents - 1));
            EXPECT_EQ(pfc_vector_get_capacity(list), old_cap);

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);

            // try to set nents -2
            EXPECT_NE(0, pfc_vector_set_capacity(list, nents - 2));
            EXPECT_EQ(pfc_vector_get_capacity(list), old_cap);

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);
        }

        // clean up
        pfc_listm_destroy(list);
    }

    // clean up
    free_random_strings(vals, nents);
}


TEST(listm, vector_set_capacity)
{
    // create test data
    int base_cap_unit = MAX(PFC_VECTOR_DEFAULT_CAPACITY,
                            PFC_VECTOR_DEFAULT_CAPINC);
    int nents = base_cap_unit * 13 + base_cap_unit / 7;
    pfc_ptr_t *vals = get_random_strings(nents);

    for (vector_add_mode_t add_mode = VECTOR_ADD_MODE_HEAD;
         add_mode < VECTOR_ADD_MODE_END_MARK;
         ++add_mode)
    {
        // create vector
        pfc_listm_t list;
        EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

        // set zero capacity just after creating
        {
            // try to set
            EXPECT_EQ(0, pfc_vector_set_capacity(list, 0));
            EXPECT_EQ(0, pfc_vector_get_capacity(list));

            // do many operations on vector
            add_ents(list, vals, nents);
            del_ents(list, nents);

            // clean up
            EXPECT_GE(pfc_listm_clear(list), 0);
        }

        // set zero capacity after used
        {
            // remove elements after filling them
            add_ents(list, vals, nents, add_mode);
            del_ents(list, nents);
            EXPECT_EQ(0, pfc_listm_get_size(list));

            // try to set
            EXPECT_EQ(0, pfc_vector_set_capacity(list, 0));
            EXPECT_EQ(0, pfc_vector_get_capacity(list));

            // do many operations on vector
            add_ents(list, vals, nents);
            del_ents(list, nents);

            // clean up
            EXPECT_GE(pfc_listm_clear(list), 0);
        }

        // set capacity that equlas to current size
        {
            // fill elements
            add_ents(list, vals, nents, add_mode);
            EXPECT_EQ(nents, pfc_listm_get_size(list));

            // try to set
            EXPECT_EQ(0, pfc_vector_set_capacity(list, nents));
            EXPECT_GE(pfc_vector_get_capacity(list), nents);

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);

            // do many operations on vector
            add_ents(list, vals, nents);
            del_ents(list, nents);

            // clean up
            EXPECT_GE(pfc_listm_clear(list), 0);
        }

        // set capacity that's larger than  current size
        {
            // fill elements
            add_ents(list, vals, nents, add_mode);
            EXPECT_EQ(nents, pfc_listm_get_size(list));

            // try to set
            int margin = base_cap_unit * 3 + base_cap_unit / 5;
            EXPECT_EQ(0, pfc_vector_set_capacity(list, nents + margin));
            EXPECT_GE(pfc_vector_get_capacity(list), nents + margin);

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);

            // do many operations on vector
            add_ents(list, vals, nents);
            del_ents(list, nents);

            // clean up
            EXPECT_GE(pfc_listm_clear(list), 0);
        }

        // set capacity to maximum
        {
            // fill elements
            add_ents(list, vals, nents, add_mode);
            EXPECT_EQ(nents, pfc_listm_get_size(list));

            // try to set
            int cap(pfc_vector_get_capacity(list));
            int err(pfc_vector_set_capacity(list, PFC_VECTOR_MAX_CAPACITY));
            if (err == 0) {
                EXPECT_EQ(PFC_VECTOR_MAX_CAPACITY,
                          pfc_vector_get_capacity(list));
            }
            else {
                EXPECT_EQ(cap, pfc_vector_get_capacity(list));
            }

            // check entries (they should not be changed)
            check_ents(list, vals, nents, add_mode);

            // do many operations on vector
            add_ents(list, vals, nents);
            del_ents(list, nents);

            // clean up
            EXPECT_GE(pfc_listm_clear(list), 0);
        }

        // clean up
        pfc_listm_destroy(list);
    }

    // clean up
    free_random_strings(vals, nents);
}


TEST(listm, vector_clear)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_clear(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_push)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_push(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_push_tail)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_push_tail(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_pop)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_pop(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_remove)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_remove(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_first)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_first(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_get_size)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_get_size(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_sort_comp)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listm_sort_comp(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_iter_create)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listiter_create(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_iter_next_after_update)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listiter_next_after_update(list);

    // In this case, list is destroyed in test routine
}


TEST(listm, vector_iter_next)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listiter_next(list);

    // clean up
    pfc_listm_destroy(list);
}


TEST(listm, vector_iter_destroy)
{
    // create vector
    pfc_listm_t list;
    EXPECT_EQ(0, PFC_VECTOR_CREATE(&list));

    // do test
    test_listiter_destroy(list);

    // clean up
    pfc_listm_destroy(list);
}
