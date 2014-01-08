/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_string.cc - Test for PFC string utilities.
 */

#include <string>
#include <pfc/util.h>
#include "test.h"
#include "misc.hh"

static void
test_strlcpy(const char *src)
{
    std::string  source(src);
    size_t       slen(source.length());

    // Specify zero to destination buffer size.
    {
        char    buf[1] = {'a'};

        ASSERT_EQ(slen, pfc_strlcpy(buf, src, 0));
        ASSERT_EQ('a', buf[0]);
    }

    // Specify 1 to destination buffer size.
    {
        char    buf[1] = {'a'};

        ASSERT_EQ(slen, pfc_strlcpy(buf, src, 1));
        ASSERT_EQ('\0', buf[0]);
    }

    // Copied string must be truncated if the destination buffer is too small.
    for (size_t size(2); size <= slen; size++) {
        TmpBuffer  tmp(size);
        char       *buf(reinterpret_cast<char *>(*tmp));
        ASSERT_TRUE(buf != NULL);

        ASSERT_EQ(slen, pfc_strlcpy(buf, src, size));
        std::string  required(source.substr(0, size - 1));
        ASSERT_STREQ(required.c_str(), buf);
    }

    // Whole string must be copied if the destination buffer size is
    // sufficient.
    for (size_t size(slen + 1); size < (slen * 2); size++) {
        TmpBuffer  tmp(size);
        char       *buf(reinterpret_cast<char *>(*tmp));
        ASSERT_TRUE(buf != NULL);

        ASSERT_EQ(slen, pfc_strlcpy(buf, src, size));
        ASSERT_STREQ(source.c_str(), buf);
    }
}

/*
 * Test for pfc_strlcpy().
 */
TEST(string, strlcpy)
{
    // Ensure that pfc_strlcpy() never touches the destination address if
    // zero is specified as buffer size.
    ASSERT_EQ(3U, pfc_strlcpy(NULL, "abc", 0));

    static const char   *sources[] = {
        "",
        "a",
        "ab",
        "abc",
        "abcd",
        "abcde",
        "This is a test string.",
        "This is a test string\nwhich contains newline.\n",
    };

    for (const char **srcp(sources); srcp < PFC_ARRAY_LIMIT(sources);
         srcp++) {
        test_strlcpy(*srcp);
        RETURN_ON_ERROR();
    }
}
