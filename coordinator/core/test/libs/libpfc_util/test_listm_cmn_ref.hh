/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_LISTM_REF_HH
#define _TEST_LISTM_REF_HH

/*
 * Header file of refptr test routines for list model
 *
 * The routines are called by for each specific list model
 * implementation.
 */

#include <pfc/listmodel.h>
#include "listm_util.h"
#include "strint_refptr.h"
#include "test_listm.hh"


#define END_OF_REFOPS  ((pfc_refptr_ops_t *)-123)
extern const pfc_refptr_ops_t *listm_refops_set[];

extern void test_listm_mismatching_refops_error (pfc_listm_t list,
                                                 const pfc_refptr_ops_t *refops,
                                                 listm_impl_t listm_impl
                                                 = LISTM_IMPL_UNKNOWN);

extern void test_listm_push_and_push_tail_refptr (pfc_listm_t list,
                                                  const pfc_refptr_ops_t *refops,
                                                  listm_impl_t listm_impl
                                                  = LISTM_IMPL_UNKNOWN);

extern void test_listm_pop_refptr (pfc_listm_t list,
                                   const pfc_refptr_ops_t *refops,
                                   listm_impl_t listm_impl
                                   = LISTM_IMPL_UNKNOWN);

extern void test_listm_remove_refptr (pfc_listm_t list,
                                      const pfc_refptr_ops_t *refops,
                                      listm_impl_t listm_impl
                                      = LISTM_IMPL_UNKNOWN);

extern void test_listm_clear_refptr (pfc_listm_t list,
                                     const pfc_refptr_ops_t *refops,
                                     listm_impl_t listm_impl
                                     = LISTM_IMPL_UNKNOWN);

/*
 * CAUTION: This function destroys list passed in argument.
 */
extern void test_listm_destroy_refptr (pfc_listm_t list,
                                       const pfc_refptr_ops_t *refops,
                                       pfc_bool_t test_dtor,
                                       listm_impl_t listm_impl
                                       = LISTM_IMPL_UNKNOWN);

extern void test_listm_first_refptr (pfc_listm_t list,
                                     const pfc_refptr_ops_t *refops,
                                     listm_impl_t listm_impl
                                     = LISTM_IMPL_UNKNOWN);

extern void test_listm_getat_refptr (pfc_listm_t list,
                                     const pfc_refptr_ops_t *refops,
                                     listm_impl_t listm_impl
                                     = LISTM_IMPL_UNKNOWN);

extern void test_listm_sort_comp_refptr (pfc_listm_t list,
                                         const pfc_refptr_ops_t *refops,
                                         listm_impl_t listm_impl
                                         = LISTM_IMPL_UNKNOWN);

#endif  /* !_TEST_LISTM_REF_HH */
