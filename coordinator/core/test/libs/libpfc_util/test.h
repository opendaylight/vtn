/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_H
#define _TEST_H

#include <pfc/base.h>
#include <pfc/log.h>

#ifdef  __cplusplus
#include <gtest/gtest.h>

#define RETURN_ON_ERROR()                               \
    do {                                                \
        if (::testing::Test::HasFailure()) {            \
            FAIL() << __FILE__ << ":" << __LINE__;      \
            return;                                     \
        }                                               \
    } while (0)

#endif  /* __cplusplus */


PFC_C_BEGIN_DECL

/* check test running under valgrind or not */
PFC_ATTR_UNUSED
inline static
pfc_bool_t is_under_valgrind ()
{
    char *env_ld_preload = getenv("LD_PRELOAD");
    if (env_ld_preload == NULL)
        return PFC_FALSE;

    if (strstr(env_ld_preload, "/vgpreload_memcheck") == NULL)
        return PFC_FALSE;

    return PFC_TRUE;
}


/* check test running under head load environment or not */
PFC_ATTR_UNUSED
inline static
pfc_bool_t is_under_heavy_load ()
{
#ifdef  HEAVY_LOAD_TEST
    return PFC_TRUE;
#endif  /* HEAVY_LOAD_TEST */
    return is_under_valgrind();
}


PFC_C_END_DECL

#endif  /* !_TEST_HH */
