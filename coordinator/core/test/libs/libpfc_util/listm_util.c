/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Implementation of listmodel test utilities
 */

#include "listm_util.h"

static volatile uint64_t count_revaddr_comp = 0;

int
revaddr_comp (pfc_cptr_t x, pfc_cptr_t y)
{
    pfc_atomic_inc_uint64((uint64_t *)&count_revaddr_comp);

    int x_val = (uintptr_t)x;
    int y_val = (uintptr_t)y;

    int ans = 0;
    if (x_val > y_val) {
        ans = -1;
    } else if (x_val < y_val) {
        ans = 1;
    }

    return ans;
}

uint64_t
get_call_count_revaddr_comp()
{
    return count_revaddr_comp;
}
