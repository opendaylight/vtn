/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Refptr test routines for list model
 *
 * The routines are called by for each specific list model
 * implementation.
 */

#include <pfc/listmodel.h>
#include "pseudo_rand.hh"
#include "string_util.h"
#include "test_listm_cmn_ref.hh"



/*
 * constants
 */
const pfc_refptr_ops_t *listm_refops_set[] = {
    NULL,
    strint_ops,
    END_OF_REFOPS,
};



/*
 * Definitions of test data
 */

typedef struct {
    pfc_listm_t list;
    listm_impl_t listm_impl;
    const pfc_refptr_ops_t *refops;
    int nents;
    pfc_ptr_t *vals;
    pfc_refptr_t **refs;
    pfc_ptr_t no_such_val;
    pfc_refptr_t *no_such_ref;
} test_data_t;

#define INVALID_PTR ((pfc_cptr_t)-123)

static test_data_t *test_data_create (pfc_listm_t list,
                                      const pfc_refptr_ops_t *refops,
                                      listm_impl_t listm_impl,
                                      int nents);
static void test_data_destroy (test_data_t *test_data);
static void test_data_inject_null (test_data_t *test_data);
static void test_data_fill_list (test_data_t *test_data);
static void test_data_del_all (test_data_t *test_data);



/*
 * Other definitions
 */

#define EXPECTED_REFCNT(listm_impl, base, n_registered)                 \
    ((uint32_t)((base) +                                                \
                (n_registered) +                                        \
                (((listm_impl) == LISTM_IMPL_HASHLIST) ? (n_registered) : 0)))




/*
 * Test routines
 */

void
test_listm_mismatching_refops_error (pfc_listm_t list,
                                     const pfc_refptr_ops_t *refops,
                                     listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);

    // create mismatching refops value
    pfc_refptr_t *invalid_refptr = NULL;
    pfc_ptr_t invalid_refptr_body = NULL;
    if (refops == NULL) {
        invalid_refptr = strint_create_from_int(123456);
    } else if (refops == strint_ops) {
        invalid_refptr_body = (pfc_ptr_t)strdup("body of invalid refptr value");
        invalid_refptr = pfc_refptr_create(NULL, invalid_refptr_body);
    }
    EXPECT_NE((pfc_refptr_t *)NULL, invalid_refptr);

    // try to push / push_tail into empty list
    EXPECT_EQ(EINVAL, pfc_listm_push(list, invalid_refptr));
    EXPECT_EQ(EINVAL, pfc_listm_push_tail(list, invalid_refptr));

    // fill list with elements
    test_data_fill_list(test_data);
    EXPECT_EQ(nents, pfc_listm_get_size(list));

    // try to push / push_tail into filled list
    EXPECT_EQ(EINVAL, pfc_listm_push(list, invalid_refptr));
    EXPECT_EQ(EINVAL, pfc_listm_push_tail(list, invalid_refptr));

    // delete elements from list
    test_data_del_all(test_data);
    EXPECT_EQ(0, pfc_listm_get_size(list));

    // try to push / push_tail into cleared list
    EXPECT_EQ(EINVAL, pfc_listm_push(list, invalid_refptr));
    EXPECT_EQ(EINVAL, pfc_listm_push_tail(list, invalid_refptr));

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
    pfc_refptr_put(invalid_refptr);
    if (invalid_refptr_body != NULL) {
        free(invalid_refptr_body);
    }
}


void
test_listm_push_and_push_tail_refptr (pfc_listm_t list,
                                      const pfc_refptr_ops_t *refops,
                                      listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);
    test_data_inject_null(test_data);

    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    // check ref counts of each elements
    for (int i = 0; i < nents; ++i) {
        pfc_refptr_t *ref = refs[i];
        if (ref != NULL) {
            EXPECT_EQ(1U, ref->pr_refcnt);
        }
    }

    int ndups = (listm_impl == LISTM_IMPL_HASHLIST) ? 0 : 10;

    // push / push_tail for each elements
    for (int flag_tail = 0; flag_tail <= 1; ++flag_tail) {
        for (int k = 0; k < 1 + ndups; ++k) {
            for (int i = 0; i < nents; ++i) {
                // get old ref count
                uint32_t old_refcnt = (refs[i] != NULL) ? refs[i]->pr_refcnt : 0;

                // do push or push_tail
                if (flag_tail) {
                    EXPECT_EQ(0, pfc_listm_push_tail(list, refs[i]));
                } else {
                    EXPECT_EQ(0, pfc_listm_push(list, refs[i]));
                }

                if (refs[i] != NULL) {
                    // check ref count
                    uint32_t expected_refcnt = EXPECTED_REFCNT(listm_impl,
                                                               old_refcnt,
                                                               1);
                    EXPECT_EQ(expected_refcnt, refs[i]->pr_refcnt);
                }
            }
        }

        // remove each elements pushed
        for (int i = 0; i < nents; ++i) {
            EXPECT_EQ(0, pfc_listm_remove(list, refs[i]));
        }

        pfc_listm_clear(list);
    }


    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_pop_refptr (pfc_listm_t list,
                       const pfc_refptr_ops_t *refops,
                       listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);
    test_data_inject_null(test_data);

    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    int ndups = (listm_impl == LISTM_IMPL_HASHLIST) ? 0 : 10;

    // try to pop from empty list
    {
        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(ENOENT, pfc_listm_pop(list, &v));
        EXPECT_EQ(INVALID_PTR, v);
    }

    // fill list with elements
    for (int k = 0; k < 1 + ndups; ++k) {
        for (int i = 0; i < nents; ++i) {
            EXPECT_EQ(0, pfc_listm_push(list, refs[i]));
        }
        EXPECT_EQ(nents * (k + 1), pfc_listm_get_size(list));
    }

    // release elements from test_data to make pfc_listm_pop()
    // decliments last ref counter
    for (int i = 0; i < nents; ++i) {
        pfc_refptr_t *ref = refs[i];
        if (ref != NULL) {
            EXPECT_EQ(EXPECTED_REFCNT(listm_impl, 1, ndups + 1), ref->pr_refcnt);
            pfc_refptr_put(ref);
            EXPECT_EQ(EXPECTED_REFCNT(listm_impl, 0, ndups + 1), ref->pr_refcnt);
            refs[i] = NULL;
        }
    }

    // pop for each elements
    for (int k = 0; k < 1 + ndups; ++k) {
        pfc_bool_t last_loop = (k == ndups);

        for (int i = 0; i < nents; ++i) {
            // fetch an element non-destructively before pop
            pfc_refptr_t *ref = NULL;
            EXPECT_EQ(0, pfc_listm_first(list, (pfc_cptr_t *)&ref));

            uint32_t old_refcnt = 0;
            uint32_t expected_refcnt = 0;
            if (ref != NULL) {
                // get old ref count
                old_refcnt = ref->pr_refcnt;
                EXPECT_GE(EXPECTED_REFCNT(listm_impl, 0, ndups + 1 - k),
                          old_refcnt);

                // determine expected refcnt
                expected_refcnt = EXPECTED_REFCNT(listm_impl, 0, ndups - k);
                if (!last_loop) {
                    EXPECT_GT(expected_refcnt, 0U);
                }
            }

#ifndef	PARALLEL_TEST
            uint64_t old_dtor_count = 0;
            if (last_loop && (ref != NULL) && (refops == strint_ops)) {
                // get dtor count
                old_dtor_count = strint_get_count_dtor();
            }
#endif	/* !PARALLEL_TEST */

            // do pop
            if (i % 2 == 0) {
                pfc_refptr_t *ref2 = NULL;
                EXPECT_EQ(0, pfc_listm_pop(list, (pfc_cptr_t *)&ref2));
                EXPECT_EQ(ref, ref2);
            } else {
                EXPECT_EQ(0, pfc_listm_pop(list, NULL));
            }

            // check ref count
            if (ref != NULL && expected_refcnt > 0) {
                if ((old_refcnt - 1) != ref->pr_refcnt) {
                    pfc_log_error("last_loop = %d", last_loop);
                    pfc_log_error("ndups = %d", ndups);
                    pfc_log_error("k = %d", k);
                    pfc_log_error("i = %d", i);
                    pfc_log_error("listm_impl = %d", listm_impl);
                    pfc_log_error("old_refcnt = %u", old_refcnt);
                    pfc_log_error("expected_refcnt = %u", expected_refcnt);
                    pfc_log_error("ref->pr_refcnt  = %u", ref->pr_refcnt);
                }
                EXPECT_EQ(old_refcnt - 1, ref->pr_refcnt);
            }

#ifndef PARALLEL_TEST
            if (last_loop && (ref != NULL) && (refops == strint_ops)) {
                // check dtor count
                EXPECT_EQ(old_dtor_count + 1, strint_get_count_dtor());
            }
#endif  /* !PARALLEL_TEST */
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_remove_refptr (pfc_listm_t list,
                          const pfc_refptr_ops_t *refops,
                          listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);
    test_data_inject_null(test_data);

    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    int ndups = (listm_impl == LISTM_IMPL_HASHLIST) ? 0 : 10;

    // try to remove from empty list
    EXPECT_EQ(ENOENT, pfc_listm_remove(list, test_data->no_such_ref));

    // fill list with elements
    for (int k = 0; k < 1 + ndups; ++k) {
        for (int i = 0; i < nents; ++i) {
            EXPECT_EQ(0, pfc_listm_push(list, refs[i]));
        }
        EXPECT_EQ(nents * (k + 1), pfc_listm_get_size(list));
    }

    // try to remove with unregisterd
    EXPECT_EQ(ENOENT, pfc_listm_remove(list, test_data->no_such_ref));

    // release elements from test_data to make pfc_listm_remove()
    // decliments last ref counter
    pfc_refptr_t *local_refs[nents];
    for (int i = 0; i < nents; ++i) {
        pfc_refptr_t *ref = refs[i];
        if (ref != NULL) {
            uint32_t expected_refcnt = EXPECTED_REFCNT(listm_impl, 1, 1 + ndups);
            EXPECT_EQ(expected_refcnt, ref->pr_refcnt); // check TP itself
            pfc_refptr_put(ref);
            refs[i] = NULL;
        }
        local_refs[i] = ref;
    }

    // remove for each elements
    for (int k = 0; k < 1 + ndups; ++k) {
        pfc_bool_t last_loop = (k == ndups);

        for (int i = 0; i < nents; ++i) {
            pfc_refptr_t *ref = local_refs[i];

            uint32_t old_refcnt = 0;
            uint32_t expected_refcnt = 0;
            if (ref != NULL) {
                // get old ref count
                old_refcnt = ref->pr_refcnt;
                EXPECT_GE(EXPECTED_REFCNT(listm_impl, 0, ndups + 1 - k),
                          old_refcnt);

                // determine expected refcnt
                expected_refcnt = EXPECTED_REFCNT(listm_impl, 0, ndups - k);
                if (!last_loop) {
                    EXPECT_GT(expected_refcnt, 0U);
                }
            }

            uint64_t old_equals_count = 0;
            if ((ref != NULL) && (refops == strint_ops)) {
                // get equals count
                old_equals_count = strint_get_count_equals();
            }

#ifndef	PARALLEL_TEST
            uint64_t old_dtor_count = 0;
            if (last_loop && (ref != NULL) && (refops == strint_ops)) {
                // get dtor count
                old_dtor_count = strint_get_count_dtor();
            }
#endif	/* !PARALLEL_TEST */

            // do remove
            EXPECT_EQ(0, pfc_listm_remove(list, ref));

            // check ref count
            if (ref != NULL && expected_refcnt > 0) {
                if (expected_refcnt != ref->pr_refcnt) {
                    pfc_log_error("last_loop = %d", last_loop);
                    pfc_log_error("ndups = %d", ndups);
                    pfc_log_error("k = %d", k);
                    pfc_log_error("i = %d", i);
                    pfc_log_error("listm_impl = %d", listm_impl);
                    pfc_log_error("old_refcnt = %u", old_refcnt);
                    pfc_log_error("expected_refcnt = %u", expected_refcnt);
                    pfc_log_error("ref->pr_refcnt  = %u", ref->pr_refcnt);
                }
                EXPECT_EQ(expected_refcnt, ref->pr_refcnt);
            }

            if ((ref != NULL) && (refops == strint_ops)) {
                // check equals count
                EXPECT_GT(strint_get_count_equals(), old_equals_count);
            }

#ifndef PARALLEL_TEST
            if (last_loop && (ref != NULL) && (refops == strint_ops)) {
                // check dtor count
                EXPECT_EQ(old_dtor_count + 1, strint_get_count_dtor());
            }
#endif  /* !PARALLEL_TEST */
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_clear_refptr (pfc_listm_t list,
                         const pfc_refptr_ops_t *refops,
                         listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);
    test_data_inject_null(test_data);

    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    pfc_refptr_t *local_refs[nents];
    for (int i = 0; i < nents; ++i) local_refs[i] = refs[i];

    // do clear on empty list
    EXPECT_EQ(0, pfc_listm_clear(list));

    // do clear after filling elements
    for (int k = 0; k < 2; ++k) {
        pfc_bool_t last_loop = (k == 1);

        // fill list with elements
        test_data_fill_list(test_data);
        EXPECT_EQ(nents, pfc_listm_get_size(list));

        // check ref counts of each elements before clear
        for (int i = 0; i < nents; ++i) {
            PFC_ASSERT(local_refs[i] == refs[i]);
            if (local_refs[i] != NULL) {
                EXPECT_GE(EXPECTED_REFCNT(listm_impl, 1, 1),
                          local_refs[i]->pr_refcnt);
                if (last_loop) {
                    // release elements from test_data to make
                    // pfc_listm_clear() decliment last ref counter
                    pfc_refptr_put(local_refs[i]);
                    EXPECT_GE(EXPECTED_REFCNT(listm_impl, 0, 1),
                              local_refs[i]->pr_refcnt);
                    refs[i] = NULL;
                }
            }
        }

#ifndef	PARALLEL_TEST
        // get status before clear
        uint64_t old_dtor_count = 0;
        if (last_loop && (refops == strint_ops)) {
            old_dtor_count = strint_get_count_dtor();
        }
#endif	/* !PARALLEL_TEST */

        // do clear
        EXPECT_EQ(nents, pfc_listm_clear(list));

        // check
        if (last_loop) {
#ifndef PARALLEL_TEST
            if (refops == strint_ops) {
                int non_null_ents = 0;
                for (int i = 0; i < nents; ++i) {
                    if (local_refs[i] != NULL) ++non_null_ents;
                }

                // check dtor count
                EXPECT_EQ(old_dtor_count + non_null_ents,
                          strint_get_count_dtor());
            }
#endif  /* !PARALLEL_TEST */
        } else {
            // check ref counts of each elements
            for (int i = 0; i < nents; ++i) {
                if (local_refs[i] != NULL) {
                    EXPECT_GE(EXPECTED_REFCNT(listm_impl, 1, 0),
                              local_refs[i]->pr_refcnt);
                }
            }
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}



void
test_listm_destroy_refptr (pfc_listm_t list,
                           const pfc_refptr_ops_t *refops,
                           pfc_bool_t test_dtor,
                           listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);
    test_data_inject_null(test_data);

    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    pfc_refptr_t *local_refs[nents];
    for (int i = 0; i < nents; ++i) local_refs[i] = refs[i];

    // fill list with elements
    test_data_fill_list(test_data);
    EXPECT_EQ(nents, pfc_listm_get_size(list));

    // check ref counts of each elements before clear
    for (int i = 0; i < nents; ++i) {
        pfc_refptr_t *ref = local_refs[i];
        if (ref != NULL) {
            uint32_t expected_refcnt = EXPECTED_REFCNT(listm_impl, 1, 1);
            EXPECT_EQ(expected_refcnt, ref->pr_refcnt); // check TP itself
            if (test_dtor) {
                // release elements from test_data to make
                // pfc_listm_clear() decliment last ref counter
                pfc_refptr_put(ref);
                PFC_ASSERT(refs[i] == ref);
                refs[i] = NULL;
            }
        }
    }

#ifndef	PARALLEL_TEST
    // get status before destroy
    uint64_t old_dtor_count = 0;
    if (test_dtor && (refops == strint_ops)) {
        old_dtor_count = strint_get_count_dtor();
    }
#endif	/* !PARALLEL_TEST */

    // do destroy
    pfc_listm_destroy(list);

    // check
    if (test_dtor) {
#ifndef PARALLEL_TEST
        if (refops == strint_ops) {
            int non_null_ents = 0;
            for (int i = 0; i < nents; ++i) {
                if (local_refs[i] != NULL) ++non_null_ents;
            }

            // check dtor count
            EXPECT_EQ(old_dtor_count + non_null_ents,
                      strint_get_count_dtor());
        }
#endif  /* !PARALLEL_TEST */
    } else {
        // check ref counts of each elements
        for (int i = 0; i < nents; ++i) {
            if (local_refs[i] != NULL) {
                EXPECT_EQ((uint32_t)(1), local_refs[i]->pr_refcnt);
            }
        }
    }

    // clean up
    test_data_destroy(test_data);
}


void
test_listm_first_refptr (pfc_listm_t list,
                         const pfc_refptr_ops_t *refops,
                         listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);
    test_data_inject_null(test_data);

    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    // determine number of iterations
    int ndups = (listm_impl == LISTM_IMPL_HASHLIST) ? 0 : 10;
    int total_regists = (ndups + 1) * nents;

    // try to fisrt from empty list
    {
        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(ENOENT, pfc_listm_first(list, &v));
        EXPECT_EQ(INVALID_PTR, v);
    }

    //  fill list with entries
    int cur_regists = 0;
    pfc_refptr_t *refs_at[total_regists];
    for (int k = 0; k < 1 + ndups; ++k) {
        for (int i = 0; i < nents; ++i) {
            EXPECT_EQ(0, pfc_listm_push_tail(list, refs[i]));
            refs_at[cur_regists] = refs[i];
            ++cur_regists;
            PFC_ASSERT(cur_regists <= total_regists);
        }
    }

    // check TP itself
    PFC_ASSERT(total_regists == cur_regists);
    for (int i = 0; i < nents; ++i) {
        if (refs[i] != NULL) {
            uint32_t expected_refcnt = EXPECTED_REFCNT(listm_impl, 1, 1 + ndups);
            EXPECT_EQ(expected_refcnt, refs[i]->pr_refcnt);
        }
    }

    // get each elements as first
    cur_regists = 0;
    for (int k = 0; k < 1 + ndups; ++k) {
        for (int i = 0; i < nents; ++i) {
            pfc_refptr_t *expected_ref = refs_at[cur_regists];
            uint32_t old_refcnt = 0;
            if (expected_ref != NULL) {
                old_refcnt = expected_ref->pr_refcnt;
            }

            // do first
            pfc_refptr_t *actual_ref;
            EXPECT_EQ(0, pfc_listm_first(list, (pfc_cptr_t *)&actual_ref));

            // check TP itself (equivalence of refptr)
            EXPECT_EQ(expected_ref, actual_ref);

            // check ref counter
            if (expected_ref != NULL) {
                EXPECT_EQ(old_refcnt, actual_ref->pr_refcnt);
            }

            // preparation for next iterations
            EXPECT_EQ(0, pfc_listm_pop(list, NULL));
            ++cur_regists;
            PFC_ASSERT(cur_regists <= total_regists);
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_getat_refptr (pfc_listm_t list,
                         const pfc_refptr_ops_t *refops,
                         listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);
    test_data_inject_null(test_data);

    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    // determine number of iterations
    int ndups = (listm_impl == LISTM_IMPL_HASHLIST) ? 0 : 10;

    // try to getat from empty list
    {
        pfc_cptr_t v = INVALID_PTR;
        EXPECT_EQ(ENOENT, pfc_listm_getat(list, -1, &v));
        EXPECT_EQ(INVALID_PTR, v);
        EXPECT_EQ(ENOENT, pfc_listm_getat(list, 0, &v));
        EXPECT_EQ(INVALID_PTR, v);
        EXPECT_EQ(ENOENT, pfc_listm_getat(list, 1, &v));
        EXPECT_EQ(INVALID_PTR, v);
    }

    // get each elements by index
    for (int k = 0; k < 1 + ndups; ++k) {
        // add elements into list
        test_data_fill_list(test_data);
        EXPECT_EQ(nents * (k + 1), pfc_listm_get_size(list));

        uint32_t expected_refcnt = EXPECTED_REFCNT(listm_impl, 1, k + 1);

        // try to getat with too small / large index
        {
            pfc_cptr_t v = INVALID_PTR;
            EXPECT_EQ(ENOENT, pfc_listm_getat(list, -1, &v));
            EXPECT_EQ(INVALID_PTR, v);
            EXPECT_EQ(ENOENT, pfc_listm_getat(list, (k + 1) * nents, &v));
            EXPECT_EQ(INVALID_PTR, v);
        }

        // check ref counter before do getat
        for (int i = 0; i < nents; ++i) {
            if (refs[i] != NULL) {
                EXPECT_EQ(expected_refcnt, refs[i]->pr_refcnt);
            }
        }

        for (int j = 0; j <= k; ++j) {
            for (int i = 0; i < nents; ++i) {
                // do getat
                int idx = j * nents + i;
                pfc_refptr_t *ref;
                EXPECT_EQ(0, pfc_listm_getat(list, idx, (pfc_cptr_t *)&ref));

                // check ref counter
                if (ref != NULL) {
                    EXPECT_EQ(expected_refcnt, ref->pr_refcnt);
                }
            }
        }
    }

    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}


void
test_listm_sort_comp_refptr (pfc_listm_t list,
                             const pfc_refptr_ops_t *refops,
                             listm_impl_t listm_impl)
{
    // create test data
    int nents = 100;
    test_data_t *test_data = test_data_create(list,
                                              refops,
                                              listm_impl,
                                              nents);
    PFC_ASSERT(test_data != NULL);
    test_data_inject_null(test_data);

    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    const pfc_listcomp_t END_OF_COMP_ARGS = (pfc_listcomp_t)-123;
    pfc_listcomp_t comp_args[] = {
        NULL,
        revaddr_comp,
        END_OF_COMP_ARGS,
    };

    int ndups = (listm_impl == LISTM_IMPL_HASHLIST) ? 0 : 10;

    // test for each comp arguments
    for (pfc_listcomp_t *comp_arg_p = comp_args;
         *comp_arg_p != END_OF_COMP_ARGS;
         ++comp_arg_p)
    {
        // do sort_comp on empty list
        EXPECT_EQ(0, pfc_listm_sort_comp(list, *comp_arg_p));

        for (int k = 0; k < 1 + ndups; ++k) {
            // add elements into list
            test_data_fill_list(test_data);

            uint32_t expected_refcnt = EXPECTED_REFCNT(listm_impl, 1, k + 1);

            // check ref counter before sort
            for (int i = 0; i < nents; ++i) {
                if (refs[i] != NULL) {
                    EXPECT_EQ(expected_refcnt, refs[i]->pr_refcnt);
                }
            }

            uint64_t old_revaddr_comp_count = 0;
            if (*comp_arg_p == revaddr_comp) {
                // get comp counter of revaddr_comp
                old_revaddr_comp_count = get_call_count_revaddr_comp();
            }

            uint64_t old_strint_comp_count = 0;
            if (refops == strint_ops) {
                // get comp counter of strint_comp
                old_strint_comp_count = strint_get_count_compare();
            }

            // do sort_comp
            EXPECT_EQ(0, pfc_listm_sort_comp(list, *comp_arg_p));

            // check ref counter
            for (int i = 0; i < nents; ++i) {
                if (refs[i] != NULL) {
                    EXPECT_EQ(expected_refcnt, refs[i]->pr_refcnt);
                }
            }

            if (refops == strint_ops) {
                // check comp counter
                if (*comp_arg_p == NULL) {
                    // count of comp function that's specific in refops
                    EXPECT_GT(strint_get_count_compare(),
                              old_strint_comp_count);
                } else if (*comp_arg_p == revaddr_comp) {
#ifndef PARALLEL_TEST
                    // count of comp function that's specific in refops
                    EXPECT_EQ(strint_get_count_compare(),
                              old_strint_comp_count);
#endif  /* !PARALLEL_TEST */

                    // count of comp function that's specific as an argument
                    EXPECT_GT(get_call_count_revaddr_comp(),
                              old_revaddr_comp_count);
                } else {
                    abort();
                }
            }
        }

        // clean up
        pfc_listm_clear(list);
    }


    // clean up
    pfc_listm_clear(list);
    test_data_destroy(test_data);
}



/*
 * Implementation of test data functions
 */

static test_data_t *
test_data_create (pfc_listm_t list,
                  const pfc_refptr_ops_t *refops,
                  listm_impl_t listm_impl,
                  int nents)
{
    prand_generator_t prand_gen = prand_create(LISTM_RANDOM_SEED);
    test_data_t *test_data = (test_data_t *)calloc(1, sizeof(test_data_t));
    PFC_ASSERT(test_data != NULL);

    test_data->list = list;
    test_data->refops = refops;
    test_data->listm_impl = listm_impl;
    test_data->nents = nents;

    test_data->refs = (pfc_refptr_t **)calloc(nents, sizeof(pfc_refptr_t *));
    PFC_ASSERT(test_data->refs != NULL);

    if (refops == NULL) {
        // val and refptr for testing ENOENT
        test_data->no_such_val =
                (pfc_ptr_t)random_string_with_prand_gen(prand_gen, 100, '!', '~');
        EXPECT_NE((pfc_ptr_t *)NULL, test_data->no_such_val);
        PFC_ASSERT(test_data->no_such_val != NULL);

        test_data->no_such_ref = pfc_refptr_create(NULL,
                                                   test_data->no_such_val);
        EXPECT_NE((pfc_refptr_t *)NULL, test_data->no_such_ref);

        // set of values and refptrs
        test_data->vals = (pfc_ptr_t *)calloc(nents, sizeof(pfc_cptr_t));
        PFC_ASSERT(test_data->vals != NULL);

        for (int i = 0; i < nents; ++i) {
          retry:
            pfc_ptr_t val =
                    (pfc_ptr_t)random_string_with_prand_gen(prand_gen,
                                                            100, '!', '~');
            PFC_ASSERT(val != NULL);

            if (string_equals(val, test_data->no_such_val)) {
                free(val);
                goto retry;
            }

            for (int j = 0; j < i; ++j) {
                if (string_equals(val, test_data->refs[j])) {
                    free(val);
                    goto retry;
                }
            }

            test_data->vals[i] = val;

            pfc_refptr_t *refptr = pfc_refptr_create(NULL, val);
            EXPECT_NE((pfc_refptr_t *)NULL, refptr);
            test_data->refs[i] = refptr;
        }
    } else if (refops == strint_ops) {
        // refptr for testing ENOENT
        test_data->no_such_val = NULL;
        test_data->no_such_ref = strint_create_from_int(nents * 2 + 1);

        // set of refptrs
        test_data->vals = NULL;
        for (int i = 0; i < nents; ++i) {
            test_data->refs[i] = strint_create_from_int(i);
            if (test_data->refs[i] == NULL) abort();
        }
    } else {
        abort();
    }

    // clean up
    prand_destroy(prand_gen);

    return test_data;
}


static void
test_data_destroy (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    PFC_ASSERT(test_data->refs != NULL);

    pfc_refptr_t **refs = test_data->refs;
    for (int i = 0; i < test_data->nents; ++i) {
        if (refs[i] != NULL) {
            pfc_refptr_put(refs[i]);
        }
    }
    free(refs);

    pfc_ptr_t *vals = test_data->vals;
    if (vals != NULL) {
        for (int i = 0; i < test_data->nents; ++i) {
            if (vals[i] != NULL) {
                free(vals[i]);
            }
        }
        free(vals);
    }

    if (test_data->no_such_ref != NULL) {
        pfc_refptr_put(test_data->no_such_ref);
    }
    if (test_data->no_such_val != NULL) {
        free(test_data->no_such_val);
    }

    free(test_data);
}


static void
test_data_inject_null (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);
    const pfc_refptr_ops_t *refops = test_data->refops;
    pfc_ptr_t *vals = test_data->vals;
    PFC_ASSERT((refops == strint_ops) || (vals != NULL));

    if (refops == NULL) {
        pfc_refptr_put(refs[0]);
        free(vals[0]);
        refs[0] = NULL;
        vals[0] = NULL;
    } else if (refops == strint_ops) {
        pfc_refptr_put(refs[0]);
        refs[0] = NULL;
    } else {
        abort();
    }
}


static void
test_data_fill_list (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    pfc_listm_t list = test_data->list;
    int nents = test_data->nents;
    pfc_refptr_t **refs = test_data->refs;
    PFC_ASSERT(refs != NULL);

    int nfill = 0;
    for (int i = 0; i < nents; ++i) {
        switch (i % 3) {
            case 0: // push
                EXPECT_EQ(0, pfc_listm_push(list, refs[i]));
                break;
            case 1: // push_tail
                EXPECT_EQ(0, pfc_listm_push_tail(list, refs[i]));
                break;
            case 2: // insert at
                EXPECT_EQ(0, pfc_listm_insertat(list, nfill / 2, refs[i]));
                break;
            default:
                abort();
        }
        ++nfill;
    }
}


static void
test_data_del_all (test_data_t *test_data)
{
    PFC_ASSERT(test_data != NULL);
    pfc_listm_t list = test_data->list;

    int size = pfc_listm_get_size(list);
    for (int i = 0; i < size; ++i) {
        switch (i % 2) {
            case 0:     // pop
                EXPECT_EQ(0, pfc_listm_pop(list, NULL));
                break;
            case 1: {   // remove
                int idx = -1;
                switch ((i / 2) % 3) {
                    case 0: idx = 0;                break; // head
                    case 1: idx = (size - i) / 2;   break; // middle
                    case 2: idx = size - i - 1;     break; // tail
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
    }
    EXPECT_EQ(0, pfc_listm_get_size(list));
}
