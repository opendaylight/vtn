/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Basic test routines for list model
 *
 * The routines are called by for each specific list model
 * implementation.
 */

#include <pfc/listmodel.h>
#include "pseudo_rand.hh"
#include "string_util.h"
#include "test_listm_cmn_basic.hh"


/*
 * Definitions of test data
 */

typedef struct {
    pfc_listm_t list;
    listm_impl_t listm_impl;
    int nents;
    pfc_ptr_t *vals;
    pfc_ptr_t no_such_val;
} test_data_t;

#define INVALID_PTR ((pfc_ptr_t)-123)

static test_data_t *test_data_create (pfc_listm_t list,
                                      listm_impl_t listm_impl,
                                      int nents);
static void test_data_destroy (test_data_t *test_data);
static void test_data_fill_list (test_data_t *test_data, int *nfill_p);
static void test_data_del_all (test_data_t *test_data, int *ndel_p);


/*
 * Test routines
 */

void
test_listm_clear (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    // clear empty list
    EXPECT_EQ(0, pfc_listm_get_size(list));
    EXPECT_EQ(0, pfc_listm_clear(list));

    // fill list
    int expected_size = -1;
    test_data_fill_list(test_data, &expected_size);
    PFC_ASSERT(expected_size > 0);

    // clear filled list
    EXPECT_EQ(expected_size, pfc_listm_get_size(list));
    EXPECT_EQ(expected_size, pfc_listm_clear(list));
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // use list
    test_data_fill_list(test_data, NULL);

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_push (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // inject NULL value
    int null_val_idx = nents / 2;
    free(vals[null_val_idx]);
    vals[null_val_idx] = NULL;

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // push and check for each values
    int expected_size = 0;
    for (int i = 0; i < nents; ++i) {
        // remember old entries
        EXPECT_EQ(i, pfc_listm_get_size(list));
        pfc_cptr_t *old_ents = NULL;
        if (i > 0) {
            old_ents = (pfc_cptr_t *)calloc(i, sizeof(pfc_cptr_t));
            for (int j = 0; j < i; ++j) {
                EXPECT_EQ(0, pfc_listm_getat(list, j, &(old_ents[j])));
            }
        }

        // do push
        EXPECT_EQ(0, pfc_listm_push(list, vals[i]));
        ++expected_size;

        // check list size
        EXPECT_EQ(expected_size, pfc_listm_get_size(list));

        // check new first entry
        pfc_cptr_t new_first_ent = INVALID_PTR;
        EXPECT_EQ(0, pfc_listm_first(list, &new_first_ent));
        EXPECT_EQ(vals[i], new_first_ent);

        // compare new and old entries
        for (int j = 0; j < i; ++j) {
            pfc_cptr_t x = NULL;
            EXPECT_EQ(0, pfc_listm_getat(list, j + 1, &x));
            EXPECT_EQ(old_ents[j], x);
        }

        // clean up
        free(old_ents);
    }

    // push duplicated elements
    for (int i = 0; i < nents; ++i) {
        int err = pfc_listm_push(list, vals[i]);
        if (test_data->listm_impl == LISTM_IMPL_HASHLIST) {
            EXPECT_EQ(EEXIST, err);
        } else {
            EXPECT_EQ(0, err);
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_push_tail (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // inject NULL value
    int null_val_idx = nents / 2;
    free(vals[null_val_idx]);
    vals[null_val_idx] = NULL;

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // push_tail and check for each values
    int expected_size = 0;
    for (int i = 0; i < nents; ++i) {
        // remember old entries
        EXPECT_EQ(i, pfc_listm_get_size(list));
        pfc_cptr_t *old_ents = NULL;
        if (i > 0) {
            old_ents = (pfc_cptr_t *)calloc(i, sizeof(pfc_cptr_t));
            for (int j = 0; j < i; ++j) {
                EXPECT_EQ(0, pfc_listm_getat(list, j, &(old_ents[j])));
            }
        }

        // do push_tail
        EXPECT_EQ(0, pfc_listm_push_tail(list, vals[i]));
        ++expected_size;

        // check list size
        EXPECT_EQ(expected_size, pfc_listm_get_size(list));

        // check new last entry
        pfc_cptr_t new_last_ent = INVALID_PTR;
        EXPECT_EQ(0, pfc_listm_getat(list, pfc_listm_get_size(list) - 1,
                                     &new_last_ent));
        EXPECT_EQ(vals[i], new_last_ent);

        if (i == 0) {
            pfc_cptr_t x = NULL;
            EXPECT_EQ(0, pfc_listm_first(list, &x));
            EXPECT_EQ(new_last_ent, x);
        }

        // compare new and old entries
        for (int j = 0; j < i; ++j) {
            pfc_cptr_t x = NULL;
            EXPECT_EQ(0, pfc_listm_getat(list, j, &x));
            EXPECT_EQ(old_ents[j], x);
        }

        // clean up
        free(old_ents);
    }

    // push_tail duplicated elements
    for (int i = 0; i < nents; ++i) {
        int err = pfc_listm_push_tail(list, vals[i]);
        if (test_data->listm_impl == LISTM_IMPL_HASHLIST) {
            EXPECT_EQ(EEXIST, err);
        } else {
            EXPECT_EQ(0, err);
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_pop (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // inject NULL value
    int null_val_idx = nents / 2;
    free(vals[null_val_idx]);
    vals[null_val_idx] = NULL;

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // fill list
    int expected_size = -1;
    test_data_fill_list(test_data, &expected_size);
    PFC_ASSERT(expected_size >= nents);

    // pop and check for each values
    while (expected_size >= 0) {
        // remember old entries
        EXPECT_EQ(expected_size, pfc_listm_get_size(list));
        pfc_cptr_t old_first_ent = NULL;
        pfc_cptr_t *old_ents = NULL;
        if (expected_size > 0) {
            EXPECT_EQ(0, pfc_listm_first(list, &old_first_ent));
            old_ents = (pfc_cptr_t *)calloc(expected_size, sizeof(pfc_cptr_t));
            for (int j = 0; j < expected_size; ++j) {
                EXPECT_EQ(0, pfc_listm_getat(list, j, &(old_ents[j])));
            }
        }

        // do pop
        pfc_cptr_t pop_ent = INVALID_PTR;
        int pop_err = pfc_listm_pop(list, &pop_ent);

        // check
        if (expected_size == 0) {
            EXPECT_EQ(ENOENT, pop_err);
            EXPECT_TRUE(INVALID_PTR == pop_ent);
            break;
        } else {
            EXPECT_EQ(0, pop_err);
            --expected_size;

            // check list size
            EXPECT_EQ(expected_size, pfc_listm_get_size(list));

            // compare with old first
            EXPECT_EQ(old_first_ent, pop_ent);
            EXPECT_EQ(old_ents[0],   pop_ent);

            // compare new and old entries
            for (int j = 0; j < expected_size; ++j) {
                pfc_cptr_t x = NULL;
                EXPECT_EQ(0, pfc_listm_getat(list, j, &x));
                EXPECT_EQ(old_ents[j + 1], x);
            }

            // clean up
            free(old_ents);
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_remove (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // inject NULL value
    int null_val_idx = nents / 8;
    free(vals[null_val_idx]);
    vals[null_val_idx] = NULL;

    // inject duplicated values
    int duplicated_val_idxs[] = { nents / 7,
                                  nents / 7 * 2,
                                  nents / 7 * 2 + 1,
                                  nents / 7 * 3,
                                  nents / 7 * 3 + 1,
                                  nents / 7 * 3 + 2,
                                  nents / 7 * 4,
                                  nents / 7 * 4 + 1,
                                  nents / 7 * 4 + 2,
                                  nents / 7 * 4 + 3,
                                  nents / 7 * 5,
                                  nents / 7 * 6,
                                  -1 };
    pfc_ptr_t duplicated_val = vals[duplicated_val_idxs[0]];
    if (listm_impl == LISTM_IMPL_HASHLIST) {
        // disable duplication settings
        duplicated_val_idxs[1] = -1;
    } else {
        for (int *idx_p = duplicated_val_idxs + 1; *idx_p != -1; ++idx_p) {
            PFC_ASSERT(*idx_p != null_val_idx);
            PFC_ASSERT(vals[*idx_p] != NULL);
            free(vals[*idx_p]);
            vals[*idx_p] = duplicated_val;
        }
    }

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // try remove from empty list
    EXPECT_EQ(0, pfc_listm_get_size(list));
    EXPECT_EQ(ENOENT, pfc_listm_remove(list, vals[0]));
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // fill list
    int expected_size = -1;
    test_data_fill_list(test_data, &expected_size);
    PFC_ASSERT(expected_size >= nents);
    EXPECT_EQ(expected_size, pfc_listm_get_size(list));

    // try remove unregistered value
    EXPECT_EQ(expected_size, pfc_listm_get_size(list));
    EXPECT_EQ(ENOENT, pfc_listm_remove(list, test_data->no_such_val));
    EXPECT_EQ(expected_size, pfc_listm_get_size(list));

    // remove and check for each values
    while (expected_size > 0) {
        // remember old entries
        EXPECT_EQ(expected_size, pfc_listm_get_size(list));
        pfc_cptr_t *old_ents = NULL;
        old_ents = (pfc_cptr_t *)calloc(expected_size, sizeof(pfc_cptr_t));
        for (int j = 0; j < expected_size; ++j) {
            EXPECT_EQ(0, pfc_listm_getat(list, j, &(old_ents[j])));
        }

        // determine value to be removed and its position
        pfc_cptr_t remove_val = NULL;
        switch (expected_size % 3) {
            case 0: // head
                remove_val = old_ents[0];
                break;
            case 1: // middle
                remove_val = old_ents[expected_size / 2];
                break;
            case 2: // tail
                remove_val = old_ents[expected_size - 1];
                break;
            default:
                abort();
        }
        int remove_idx = -1;
        for (int j = 0; j < expected_size; ++j) {
            if (remove_val == old_ents[j]) {
                remove_idx = j;
                break;
            }
        }
        PFC_ASSERT(remove_idx != -1);

        // do remove
        EXPECT_EQ(0, pfc_listm_remove(list, remove_val));
        --expected_size;

        // check list size
        EXPECT_EQ(expected_size, pfc_listm_get_size(list));

        // compare new and old entries (before removed entry)
        for (int j = 0; j < remove_idx; ++j) {
            pfc_cptr_t x = NULL;
            EXPECT_EQ(0, pfc_listm_getat(list, j, &x));
            EXPECT_EQ(old_ents[j], x);
        }

        // compare new and old entries (after removed entry)
        for (int j = remove_idx; j < expected_size; ++j) {
            pfc_cptr_t x = NULL;
            EXPECT_EQ(0, pfc_listm_getat(list, j, &x));
            EXPECT_EQ(old_ents[j + 1], x);
        }

        // clean up
        free(old_ents);
    }

    // clean up
    pfc_listm_clear(list);
    for (int *idx_p = duplicated_val_idxs + 1; *idx_p != -1; ++idx_p) {
        vals[*idx_p] = NULL;
    }
    test_data_destroy(test_data);
}


void
test_listm_first (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // inject NULL value
    int null_val_idx = nents / 2;
    free(vals[null_val_idx]);
    vals[null_val_idx] = NULL;

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // do first for empty list
    {
        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(ENOENT, pfc_listm_first(list, &v));
        EXPECT_EQ(INVALID_PTR, v);
    }

    // push, first and check for each values
    for (int i = 0; i < nents; ++i) {
        // push value
        EXPECT_EQ(0, pfc_listm_push(list, vals[i]));
        EXPECT_EQ(i + 1, pfc_listm_get_size(list));

        // do first
        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(0, pfc_listm_first(list, &v));

        // check size (should be unchanged)
        EXPECT_EQ(i + 1, pfc_listm_get_size(list));

        // cehck value
        EXPECT_EQ(vals[i], v);
    }

    // remove, first and check for each values
    for (int i = 0; i < nents; ++i) {
        // Initially, fill list with test data
        EXPECT_GE(pfc_listm_clear(list), 0);
        for (int j = 0; j < nents; ++j) {
            EXPECT_EQ(0, pfc_listm_push_tail(list, vals[j]));
        }

        // pop entries before position `i'
        for (int j = 0; j < i; ++j) {
            EXPECT_EQ(0, pfc_listm_pop(list, NULL));
        }
        EXPECT_EQ(nents - i, pfc_listm_get_size(list));

        // do first
        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(0, pfc_listm_first(list, &v));

        // check size (should be unchanged)
        EXPECT_EQ(nents - i, pfc_listm_get_size(list));

        // cehck value
        EXPECT_EQ(vals[i], v);
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_getat (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // getat for empty list
    {
        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(ENOENT, pfc_listm_getat(list, 0, &v));
        EXPECT_EQ(INVALID_PTR, v);
    }

    // pop_tail, getat and check for each values
    for (int i = 0; i < nents; ++i) {
        EXPECT_EQ(i, pfc_listm_get_size(list));

        // append value at tail
        EXPECT_EQ(0, pfc_listm_push_tail(list, vals[i]));

        // do getat for each registered values
        for (int j = 0; j <= i; ++j) {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(0, pfc_listm_getat(list, j, &v));
            EXPECT_NE(vals[j], v);
        }

        // do getat with too large positon
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_listm_getat(list, i + 1, &v));
            EXPECT_EQ(INVALID_PTR, v);
        }

        // do getat with negative positon
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_NE(0, pfc_listm_getat(list, -1, &v));
            EXPECT_EQ(INVALID_PTR, v);
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_get_size (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // regist / un-regist operations, get_size and check for each value
    for (int nins = 0; nins <= nents; ++nins) {
        for (int nouts = 0; nouts <= nins; ++nouts) {
            // reset list
            EXPECT_EQ(0, pfc_listm_clear(list));
            EXPECT_EQ(0, pfc_listm_get_size(list));
            int expected_size = 0;

            // regist, get_size and check
            for (int i = 0; i < nins; ++i) {
                switch (i % 2) {
                    case 0:     // push
                        EXPECT_EQ(0, pfc_listm_push(list, vals[i]));
                        break;
                    case 1:     // push_tail
                        EXPECT_EQ(0, pfc_listm_push_tail(list, vals[i]));
                        break;
                    default:
                        abort();
                }
                ++expected_size;
                EXPECT_EQ(expected_size, pfc_listm_get_size(list));
            }

            // un-regist, get_size and check
            for (int i = 0; i < nins; ++i) {
                switch (i % 2) {
                    case 0:     // pop
                        EXPECT_EQ(0, pfc_listm_pop(list, NULL));
                        break;
                    case 1: {   // remove
                        pfc_cptr_t v = INVALID_PTR;
                        int pos = expected_size / 2;
                        EXPECT_EQ(0, pfc_listm_getat(list, pos, &v));
                        EXPECT_NE(INVALID_PTR, v);
                        EXPECT_EQ(0, pfc_listm_remove(list, v));
                        break;
                    }
                    default:
                        abort();
                }
                --expected_size;
                EXPECT_EQ(expected_size, pfc_listm_get_size(list));
            }
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_sort_comp (pfc_listm_t list, listm_impl_t listm_impl)
{
    pfc_listcomp_t comps[] = { string_comp,
                               NULL,
                               (pfc_listcomp_t)INVALID_PTR };

    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // inject duplicated values
    int duplicated_val_idxs[] = { nents / 7,
                                  nents / 7 * 2,
                                  nents / 7 * 2 + 1,
                                  nents / 7 * 3,
                                  nents / 7 * 3 + 1,
                                  nents / 7 * 3 + 2,
                                  nents / 7 * 4,
                                  nents / 7 * 4 + 1,
                                  nents / 7 * 4 + 2,
                                  nents / 7 * 4 + 3,
                                  nents / 7 * 5,
                                  nents / 7 * 6,
                                  -1 };
    pfc_ptr_t duplicated_val = vals[duplicated_val_idxs[0]];
    if (listm_impl == LISTM_IMPL_HASHLIST) {
        // disable duplication settings
        duplicated_val_idxs[1] = -1;
    } else {
        for (int *idx_p = duplicated_val_idxs + 1; *idx_p != -1; ++idx_p) {
            PFC_ASSERT(vals[*idx_p] != NULL);
            free(vals[*idx_p]);
            vals[*idx_p] = duplicated_val;
        }
    }

    // sort_comp for empty list
    {
        EXPECT_EQ(0, pfc_listm_sort_comp(list, comps[0]));
        EXPECT_EQ(0, pfc_listm_get_size(list));

        // try many operations on list
        test_data_fill_list(test_data, NULL);
        test_data_del_all(test_data, NULL);
    }

    // sort_comp and check for each size of list
    for (pfc_listcomp_t *comp_p = comps; *comp_p != INVALID_PTR; ++comp_p) {
        pfc_listcomp_t comp = *comp_p;

        for (int i = 0; i < nents; ++i) {
            // fill list with entries indexed from 0 to i
            EXPECT_GE(pfc_listm_clear(list), 0);
            for (int j = 0; j <= i; ++j) {
                EXPECT_EQ(0, pfc_listm_push(list, vals[j]));
            }
            int expected_size = i + 1;
            EXPECT_EQ(expected_size, pfc_listm_get_size(list));

            // do sort_comp
            EXPECT_EQ(0, pfc_listm_sort_comp(list, comp));

            // check size
            EXPECT_EQ(expected_size, pfc_listm_get_size(list));

            // check order
            for (int j = 0; j < i; ++j) {
                pfc_cptr_t v1 = INVALID_PTR;
                pfc_cptr_t v2 = INVALID_PTR;
                EXPECT_EQ(0, pfc_listm_getat(list, j,     &v1));
                EXPECT_EQ(0, pfc_listm_getat(list, j + 1, &v2));
                EXPECT_NE(INVALID_PTR, v1);
                EXPECT_NE(INVALID_PTR, v2);

                if (comp == NULL) {
                    EXPECT_LE((uintptr_t)v1, (uintptr_t)v2);
                } else {
                    EXPECT_LE(comp(v1, v2), 0);
                }
            }

            // check entries that should be same before sort
            {
                pfc_cptr_t tmp_vals[i + 1];
                memcpy(tmp_vals, vals, sizeof(pfc_ptr_t) * (i + 1));

                // cehck sorted list has all members that are set before sort
                for (int j = 0; j <= i; ++j) {
                    pfc_cptr_t v;
                    EXPECT_EQ(0, pfc_listm_getat(list, j, &v));

                    pfc_bool_t found = PFC_FALSE;
                    for (int k = 0; k <= i; ++k) {
                        if (tmp_vals[k] == v) {
                            tmp_vals[k] = INVALID_PTR;
                            found = PFC_TRUE;
                            break;
                        }
                    }
                    EXPECT_EQ(PFC_TRUE, found);
                }

                // cehck all members that are set before sort are found
                for (int k = 0; k <= i; ++k) {
                    EXPECT_EQ(INVALID_PTR, tmp_vals[k]);
                }
            }

            // try many operations and clean up
            if (test_data->listm_impl != LISTM_IMPL_HASHLIST) {
                test_data_fill_list(test_data, NULL);
            }
            test_data_del_all(test_data, NULL);
        }
    }

    // clean up
    pfc_listm_clear(list);
    for (int *idx_p = duplicated_val_idxs + 1; *idx_p != -1; ++idx_p) {
        vals[*idx_p] = NULL;
    }
    test_data_destroy(test_data);
}


void
test_listiter_create (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    // clear list
    EXPECT_GE(pfc_listm_clear(list), 0);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // listiter_create and listiter_destroy for empty list
    {
        pfc_listiter_t it = pfc_listiter_create(list);
        EXPECT_TRUE(it != NULL);
        pfc_listiter_destroy(it);
    }

    // fill list
    int nfill = -1;
    test_data_fill_list(test_data, &nfill);
    PFC_ASSERT(nfill > 0);

    // listiter_create and listiter_destroy for filled list
    {
        pfc_listiter_t it = pfc_listiter_create(list);
        EXPECT_TRUE(it != NULL);

        for (int i = 0; i < MAX(nfill, 10); ++i) {
            EXPECT_EQ(0, pfc_listiter_next(it, NULL));
        }

        pfc_listiter_destroy(it);
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


/*
 * CAUTION: This function destroys list passed in argument.
 */
void
test_listiter_next_after_update (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // listiter_next and check error number for each update operations
    typedef enum {
        UPDATE_OP_PUSH,
        UPDATE_OP_PUSH_TAIL,
        UPDATE_OP_POP,
        UPDATE_OP_REMOVE,
        UPDATE_OP_CLEAR,
        UPDATE_OP_DESTROY,
        UPDATE_OP_END,
    } update_op_t;

    update_op_t update_ops[] = {
        UPDATE_OP_PUSH,
        UPDATE_OP_PUSH_TAIL,
        UPDATE_OP_POP,
        UPDATE_OP_REMOVE,
        UPDATE_OP_CLEAR,
        UPDATE_OP_DESTROY,
        UPDATE_OP_END,
    } ;

    for (update_op_t *update_op_p = update_ops;
         *update_op_p != UPDATE_OP_END;
         ++update_op_p)
    {
        // reset list
        EXPECT_GE(pfc_listm_clear(list), 0);
        EXPECT_EQ(0, pfc_listm_get_size(list));
        int nfill = -1;
        test_data_fill_list(test_data, &nfill);
        PFC_ASSERT(nfill > 0);

        // avoid duplication that's not allowed in hash list
        EXPECT_EQ(0, pfc_listm_remove(list, vals[0]));

        // create iterator
        pfc_listiter_t it = pfc_listiter_create(list);
        EXPECT_TRUE(it != NULL);

        // update operation
        switch (*update_op_p) {
            case UPDATE_OP_PUSH:
                EXPECT_EQ(0, pfc_listm_push(list, vals[0]));
                break;
            case UPDATE_OP_PUSH_TAIL:
                EXPECT_EQ(0, pfc_listm_push_tail(list, vals[0]));
                break;
            case UPDATE_OP_POP:
                EXPECT_EQ(0, pfc_listm_push_tail(list, NULL));
                break;
            case UPDATE_OP_REMOVE:
                EXPECT_EQ(0, pfc_listm_push_tail(list, vals[0]));
                break;
            case UPDATE_OP_CLEAR:
                EXPECT_EQ(nfill - 1, pfc_listm_clear(list));
                break;
            case UPDATE_OP_DESTROY:
                pfc_listm_destroy(list);
                break;
            default: abort();
        }

        // do listiter_next
        pfc_cptr_t v = NULL;
        int err = pfc_listiter_next(it, &v);

        // check error number
        if (*update_op_p == UPDATE_OP_DESTROY) {
            EXPECT_EQ(EBADF, err);
        } else {
            EXPECT_EQ(EINVAL, err);
        }

        // clean up
        pfc_listiter_destroy(it);
    }

    // clean up
    // In this case, list is destroyed in update operations
    test_data_destroy(test_data);
}


void
test_listiter_next (pfc_listm_t list, listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list, listm_impl, nents);
    PFC_ASSERT(test_data != NULL);

    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT(vals != NULL);

    // listiter_next for empty list
    {
        // clear list
        EXPECT_GE(pfc_listm_clear(list), 0);
        EXPECT_EQ(0, pfc_listm_get_size(list));

        // create iterator
        pfc_listiter_t it = pfc_listiter_create(list);
        EXPECT_TRUE(it != NULL);

        // do listiter_next
        pfc_cptr_t v = NULL;
        EXPECT_EQ(ENOENT, pfc_listiter_next(it, &v));

        // clean up
        pfc_listiter_destroy(it);
    }

    // listiter_next and check for each size of list
    for (int i = 0; i < nents; ++i) {
        // fill list with only `i' entries
        EXPECT_GE(pfc_listm_clear(list), 0);
        EXPECT_EQ(0, pfc_listm_get_size(list));
        for (int j = 0; j <= i; ++j) {
            EXPECT_EQ(0, pfc_listm_push_tail(list, vals[j]));
        }

        // create iterator
        pfc_listiter_t it = pfc_listiter_create(list);

        // listiter_next and check until encountering ENOENT
        for (int iter_cnt = 0; iter_cnt <= i + 1; ++iter_cnt) {
            // do listiter_next
            pfc_cptr_t v = INVALID_PTR;
            pfc_cptr_t *v_p = (iter_cnt % 2 == 0) ? &v : NULL;
            int err =pfc_listiter_next(it, v_p);

            // check
            if (iter_cnt > i) {
                EXPECT_EQ(ENOENT, err);
            } else {
                EXPECT_EQ(0, err);
                if (v_p != NULL) {
                    EXPECT_EQ(vals[iter_cnt], v);
                }
            }
        }

        // clean up
        pfc_listiter_destroy(it);
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listiter_destroy (pfc_listm_t list, listm_impl_t listm_impl)
{
    test_listiter_create(list);
}


/*
 * Implementation of test data functions
 */

static test_data_t *
test_data_create (pfc_listm_t list,
                  listm_impl_t listm_impl,
                  int nents)
{
    prand_generator_t prand_gen = prand_create(LISTM_RANDOM_SEED);

    test_data_t *test_data = (test_data_t *)calloc(1, sizeof(test_data_t));
    PFC_ASSERT(test_data != NULL);

    test_data->list = list;
    test_data->listm_impl = listm_impl;
    test_data->nents = nents;

    test_data->no_such_val =
            (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~');
    PFC_ASSERT(test_data->no_such_val != NULL);

    test_data->vals = (pfc_ptr_t *)calloc(nents, sizeof(pfc_ptr_t));
    PFC_ASSERT(test_data->vals != NULL);

    for (int i = 0; i < nents; ++i) {
      retry:
        pfc_ptr_t val = (pfc_ptr_t)random_string_with_prand_gen(prand_gen,
                                                                100, '!', '~');
        PFC_ASSERT(val != NULL);

        if (string_equals(val, test_data->no_such_val)) {
            free(val);
            goto retry;
        }

        for (int j = 0; j < i; ++j) {
            if (string_equals(val, test_data->vals[j])) {
                free(val);
                goto retry;
            }
        }

        test_data->vals[i] = val;
    }

    // clean up
    prand_destroy(prand_gen);

    return test_data;
}


static void
test_data_destroy (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    PFC_ASSERT(test_data->vals != NULL);

    pfc_ptr_t *vals = test_data->vals;
    for (int i = 0; i < test_data->nents; ++i) {
        if (vals[i] != NULL) {
            free(vals[i]);
        }
    }
    free(vals);

    free(test_data->no_such_val);
    free(test_data);
}


static void
test_data_fill_list (test_data_t *test_data, int *nfill_p)
{
    PFC_ASSERT(test_data != NULL);
    pfc_listm_t list = test_data->list;
    int nents = test_data->nents;
    pfc_ptr_t *vals = test_data->vals;

    int nfill = 0;
    for (int i = 0; i < nents; ++i) {
        switch (i % 3) {
            case 0: // push
                EXPECT_EQ(0, pfc_listm_push(list, vals[i]));
                break;
            case 1: // push_tail
                EXPECT_EQ(0, pfc_listm_push_tail(list, vals[i]));
                break;
            case 2: // insert at
                EXPECT_EQ(0, pfc_listm_insertat(list, nfill / 2, vals[i]));
                break;
            default:
                abort();
        }
        ++nfill;
    }

    if (nfill_p != NULL) {
        *nfill_p = nfill;
    }
}


static void
test_data_del_all (test_data_t *test_data, int *ndel_p)
{
    PFC_ASSERT(test_data != NULL);
    pfc_listm_t list = test_data->list;

    int size = pfc_listm_get_size(list);
    int ndel = 0;
    for (int i = 0; i < size; ++i) {
        switch (i % 2) {
            case 0:     // pop
                EXPECT_EQ(0, pfc_listm_pop(list, NULL));
                break;
            case 1: {   // remove
                int idx = -1;
                switch ((i / 2) % 3) {
                    case 0: idx = 0;                 break; // head
                    case 1: idx = (size - ndel) / 2; break; // middle
                    case 2: idx = size - ndel - 1;   break; // tail
                    default: abort();
                }
                pfc_cptr_t v = INVALID_PTR;
                EXPECT_EQ(0, pfc_listm_getat(list, idx, &v));
                // If duplicated value exists at before `idx',
                // pfc_listm_remove() will delete duplicated one.
                EXPECT_EQ(0, pfc_listm_remove(list, v));
                break;
            }
            default:
                abort();
        }
        ++ndel;
    }
    EXPECT_EQ(0, pfc_listm_get_size(list));

    if (ndel_p != NULL) {
        *ndel_p = ndel;
    }
}
