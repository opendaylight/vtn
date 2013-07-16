/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_LISTM_MT_HH
#define _TEST_LISTM_MT_HH

/*
 * Header file of multi-thread test routines for list model
 *
 * The routines are called by for each specific list model
 * implementation.
 */

#include <pfc/listmodel.h>
#include "listm_util.h"
#include "test_listm.hh"


extern void test_listm_get_size_with_threads (pfc_listm_t listm,
                                              listm_impl_t listm_impl
                                              = LISTM_IMPL_UNKNOWN);
extern void test_listm_first_with_threads (pfc_listm_t listm,
                                           listm_impl_t listm_impl
                                           = LISTM_IMPL_UNKNOWN);
extern void test_listm_getat_with_threads (pfc_listm_t listm,
                                           listm_impl_t listm_impl
                                           = LISTM_IMPL_UNKNOWN);
extern void test_listm_pop_with_threads (pfc_listm_t listm,
                                         listm_impl_t listm_impl
                                         = LISTM_IMPL_UNKNOWN);
extern void test_listm_nolimit_with_threads (pfc_listm_t listm,
                                             listm_impl_t listm_impl
                                             = LISTM_IMPL_UNKNOWN);

#endif  /* !_TEST_LISTM_MT_HH */
