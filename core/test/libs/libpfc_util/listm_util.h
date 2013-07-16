/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_LISTMODEL_UTIL_H
#define _TEST_LISTMODEL_UTIL_H

/*
 * Utility for list model tests
 */

#include <pfc/listmodel.h>

PFC_C_BEGIN_DECL

#define MAX(x, y)   (((x) > (y)) ? (x) : (y))

typedef enum {
    LISTM_IMPL_UNKNOWN,
    LISTM_IMPL_LLIST,
    LISTM_IMPL_VECTOR,
    LISTM_IMPL_HASHLIST,
}  listm_impl_t;

extern int revaddr_comp (pfc_cptr_t x, pfc_cptr_t y);
extern uint64_t get_call_count_revaddr_comp ();

PFC_C_END_DECL

#endif  /* !_TEST_LISTMODEL_UTIL_H */
