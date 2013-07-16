/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_LISTM_BASIC_HH
#define _TEST_LISTM_BASIC_HH

/*
 * Header file of basic test routines for list model
 *
 * The routines are called by for each specific list model
 * implementation.
 */

#include <pfc/listmodel.h>
#include "listm_util.h"
#include "test_listm.hh"


extern void test_listm_clear (pfc_listm_t list,
                              listm_impl_t listm_impl
                              = LISTM_IMPL_UNKNOWN);
extern void test_listm_push (pfc_listm_t list,
                             listm_impl_t listm_impl
                             = LISTM_IMPL_UNKNOWN);
extern void test_listm_push_tail (pfc_listm_t list,
                                  listm_impl_t listm_impl
                                  = LISTM_IMPL_UNKNOWN);
extern void test_listm_pop (pfc_listm_t list,
                            listm_impl_t listm_impl
                            = LISTM_IMPL_UNKNOWN);
extern void test_listm_remove (pfc_listm_t list,
                               listm_impl_t listm_impl
                               = LISTM_IMPL_UNKNOWN);
extern void test_listm_first (pfc_listm_t list,
                              listm_impl_t listm_impl
                              = LISTM_IMPL_UNKNOWN);
extern void test_listm_get_size (pfc_listm_t list,
                                 listm_impl_t listm_impl
                                 = LISTM_IMPL_UNKNOWN);
extern void test_listm_sort_comp (pfc_listm_t list,
                                  listm_impl_t listm_impl
                                  = LISTM_IMPL_UNKNOWN);
extern void test_listiter_create (pfc_listm_t list,
                                  listm_impl_t listm_impl
                                  = LISTM_IMPL_UNKNOWN);

/*
 * CAUTION: This function destroys list passed in argument.
 */
extern void test_listiter_next_after_update (pfc_listm_t list,
                                             listm_impl_t listm_impl
                                             = LISTM_IMPL_UNKNOWN);

extern void test_listiter_next (pfc_listm_t list,
                                listm_impl_t listm_impl
                                = LISTM_IMPL_UNKNOWN);
extern void test_listiter_destroy (pfc_listm_t list,
                                   listm_impl_t listm_impl
                                   = LISTM_IMPL_UNKNOWN);

#endif  /* !_TEST_LISTM_BASIC_HH */
