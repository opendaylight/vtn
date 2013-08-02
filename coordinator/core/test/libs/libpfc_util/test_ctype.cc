/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_ctype.cc - Test for functions defined in pfc/ctype.h.
 */

#include <cstdio>
#include <cctype>
#include <gtest/gtest.h>
#include <pfc/ctype.h>

/*
 * Test case for functions which take interger argument.
 */
#define	CTYPE_TEST_DECL(func)                           \
    TEST(ctype, func)                                   \
    {                                                   \
        for (int c(EOF); c <= (int)UINT8_MAX; c++) {    \
            bool	required(func(c) != 0);         \
            bool	result(pfc_##func(c) != 0);     \
                                                        \
            ASSERT_EQ(required, result) << "c = " << c; \
        }                                               \
    }

CTYPE_TEST_DECL(isdigit);
CTYPE_TEST_DECL(isspace);
CTYPE_TEST_DECL(isalpha);
CTYPE_TEST_DECL(isalnum);
CTYPE_TEST_DECL(islower);
CTYPE_TEST_DECL(isupper);
CTYPE_TEST_DECL(isascii);

/*
 * Test case for character conversion functions which take integer argument.
 */
#define	CTYPE_CONV_TEST_DECL(func)                              \
    TEST(ctype, func)                                           \
    {                                                           \
        for (int c(EOF); c <= (int)UINT8_MAX; c++) {            \
            ASSERT_EQ(func(c), pfc_##func(c)) << "c = " << c;   \
        }                                                       \
    }

CTYPE_CONV_TEST_DECL(tolower);
CTYPE_CONV_TEST_DECL(toupper);

/*
 * Test case for functions which take uint8_t argument.
 */
#define	CTYPE_TEST_U_DECL(func)                                         \
    TEST(ctype, func##_u)                                               \
    {                                                                   \
        for (int i(0); (uint32_t)i <= (uint32_t)UINT8_MAX; i++) {       \
            char	c(static_cast<char>(i));                        \
            uint8_t	uc(static_cast<uint8_t>(i));                    \
            bool	required(func(uc) != 0);                        \
            bool	result(pfc_##func##_u(c) != 0);                 \
                                                                        \
            ASSERT_EQ(required, result) << "c = " << c;                 \
        }                                                               \
    }

CTYPE_TEST_U_DECL(isdigit);
CTYPE_TEST_U_DECL(isspace);
CTYPE_TEST_U_DECL(isalpha);
CTYPE_TEST_U_DECL(isalnum);
CTYPE_TEST_U_DECL(islower);
CTYPE_TEST_U_DECL(isupper);
CTYPE_TEST_U_DECL(isascii);

/*
 * Test case for character conversion functions which take uint8_t argument.
 */
#define	CTYPE_CONV_U_TEST_DECL(func)                                    \
    TEST(ctype, func##_u)                                               \
    {                                                                   \
        for (int i(0); (uint32_t)i <= (uint32_t)UINT8_MAX; i++) {       \
            char	c(static_cast<char>(i));                        \
            uint8_t	uc(static_cast<uint8_t>(i));                    \
            uint8_t	required(func(uc));                             \
                                                                        \
            ASSERT_EQ(required, pfc_##func##_u(c)) << "c = " << c;      \
        }                                                               \
    }

CTYPE_CONV_U_TEST_DECL(tolower);
CTYPE_CONV_U_TEST_DECL(toupper);
