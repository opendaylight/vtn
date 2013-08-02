/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_strtoint.cc - Test for string to integer converter.
 */

#include <string>
#include <pfc/strtoint.h>
#include <pfc/ctype.h>
#include <pfc/util.h>
#include "test.h"
#include "random.hh"

#define RANDOM_LOOP     30000

/*
 * Test for pfc_strtoi32()
 * - Decimal string
 */
TEST(strtoint, strtoi32_dec)
{
    // Successful tests.
    const char *strings[] = {
        "-2147483648",          // INT32_MIN
        "-2147483647",
        "-2147483646",
        "-2147483645",
        "-268435457",
        "-268435456",           // 0xf0000000
        "-268435455",
        "-3",
        "-2",
        "-1",
        "-0",
        "0",
        "1",
        "2",
        "3",
        "1879048191",
        "1879048192",           // 0x70000000
        "1879048193",
        "2147483645",
        "2147483646",
        "2147483647",           // INT32_MAX
    };

    int32_t required[] = {
        INT32_MIN,
        -2147483647,
        -2147483646,
        -2147483645,
        -268435457,
        -268435456,
        -268435455,
        -3,
        -2,
        -1,
        -0,
        0,
        1,
        2,
        3,
        1879048191,
        1879048192,
        1879048193,
        2147483645,
        2147483646,
        INT32_MAX,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        int32_t    value;

        ASSERT_EQ(0, pfc_strtoi32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        if (*str != '-') {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtoi32(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }
    }

    // Invalid strings.
    const char *invalid[] = {
        "",
        "+",
        "-",
        " -1",
        "10 ",
        "12abc",
        "777\n",
        "123,456,789",
        "12345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        int32_t  value;

        ASSERT_EQ(EINVAL, pfc_strtoi32(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[6];
        for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(5U, pfc_strlcpy(buf, "12345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if (pfc_isdigit_u(c) || c == '\0' ||
                    (i == 0 && (c == '-' || c == '+'))) {
                    continue;
                }

                int32_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtoi32(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "-2147483649",
        "-2147483650",
        "-2147483651",
        "-3000000000",
        "-10000000000",
        "2147483648",
        "2147483649",
        "2147483650",
        "3000000000",
        "10000000000",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        int32_t  value;

        ASSERT_EQ(ERANGE, pfc_strtoi32(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        int32_t  value, v;

        RANDOM_INTEGER(rand, value);
        snprintf(str, sizeof(str), "%d", value);
        ASSERT_EQ(0, pfc_strtoi32(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtoi32()
 * - Octal string
 */
TEST(strtoint, strtoi32_oct)
{
    // Successful tests.
    const char *strings[] = {
        "-020000000000",        // INT32_MIN
        "-017777777777",
        "-017777777776",
        "-017777777775",
        "-02000000001",
        "-02000000000",         // 0xf0000000
        "-01777777777",
        "-03",
        "-02",
        "-01",
        "-00",
        "00",
        "01",
        "02",
        "03",
        "015777777777",
        "016000000000",         // 0x70000000
        "016000000001",
        "017777777775",
        "017777777776",
        "017777777777",         // INT32_MAX
    };

    int32_t required[] = {
        INT32_MIN,
        -2147483647,
        -2147483646,
        -2147483645,
        -268435457,
        -268435456,
        -268435455,
        -3,
        -2,
        -1,
        -0,
        0,
        1,
        2,
        3,
        1879048191,
        1879048192,
        1879048193,
        2147483645,
        2147483646,
        INT32_MAX,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        int32_t    value;

        ASSERT_EQ(0, pfc_strtoi32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        if (*str != '-') {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtoi32(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }

        // Leading zeroes must be ignored.
        std::string newstr;
        if (*str == '-') {
            newstr.append("-");
            str++;
        }
        newstr.append("00000000");
        newstr.append(str);
        str = newstr.c_str();
        ASSERT_EQ(0, pfc_strtoi32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;
    }

    // Invalid strings.
    const char *invalid[] = {
        " -01",
        "010 ",
        "012abc",
        "0777\n",
        "0123,456,777",
        "012345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        int32_t  value;

        ASSERT_EQ(EINVAL, pfc_strtoi32(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[7];
        for (uint32_t i(1); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(6U, pfc_strlcpy(buf, "012345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if ((c >= '0' && c <= '7') || c == '\0' ||
                    (i == 1 && (c == '\0' || c == 'x' || c == 'X'))) {
                    continue;
                }

                int32_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtoi32(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "-020000000001",
        "-020000000002",
        "-020000000003",
        "-030000000000",
        "-0200000000000",
        "-0111111111111",
        "-0222222222222",
        "-0777777777777",
        "020000000000",
        "020000000001",
        "020000000002",
        "030000000000",
        "0200000000000",
        "0222222222222",
        "0333333333333",
        "0777777777777",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        int32_t  value;

        ASSERT_EQ(ERANGE, pfc_strtoi32(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        int32_t  value, v;

        RANDOM_INTEGER(rand, value);
        if (value == INT32_MIN) {
            snprintf(str, sizeof(str), "-0020000000000");
        }
        else if (value < 0) {
            snprintf(str, sizeof(str), "-0%o", -value);
        }
        else {
            snprintf(str, sizeof(str), "0%o", value);
        }
        ASSERT_EQ(0, pfc_strtoi32(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtoi32()
 * - Hexadecimal string
 */
TEST(strtoint, strtoi32_hex)
{
    // Successful tests.
    const char *strings[] = {
        "-0x80000000",          // INT32_MIN
        "-0x7fffffff",
        "-0x7ffffffe",
        "-0x7ffffffd",
        "-0x10000001",
        "-0x10000000",          // 0xf0000000
        "-0xfffffff",
        "-0x3",
        "-0x2",
        "-0x1",
        "-0x0",
        "0x0",
        "0x1",
        "0x2",
        "0x3",
        "0x6fffffff",
        "0x70000000",
        "0x70000001",
        "0x7ffffffd",
        "0x7ffffffe",
        "0x7fffffff",           // INT32_MAX
    };

    int32_t required[] = {
        INT32_MIN,
        -2147483647,
        -2147483646,
        -2147483645,
        -268435457,
        -268435456,
        -268435455,
        -3,
        -2,
        -1,
        -0,
        0,
        1,
        2,
        3,
        1879048191,
        1879048192,
        1879048193,
        2147483645,
        2147483646,
        INT32_MAX,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        int32_t    value;

        ASSERT_EQ(0, pfc_strtoi32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        if (*str != '-') {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtoi32(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }

        // Leading zeroes must be ignored.
        std::string newstr;
        if (*str == '-') {
            newstr.append("-");
            str++;
        }
        newstr.append("0x00000000");
        str += 2;
        newstr.append(str);
        str = newstr.c_str();
        ASSERT_EQ(0, pfc_strtoi32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Upper case must be accepted.
        std::string upper;
        size_t sz(newstr.size());
        for (size_t j(0); j < sz; j++) {
            char  c(newstr.at(j));
            c = pfc_toupper(c);
            upper.append(1, c);
        }

        str = upper.c_str();
        ASSERT_EQ(0, pfc_strtoi32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;
    }

    // Invalid strings.
    const char *invalid[] = {
        "0x",
        " -0x1",
        "0x10 ",
        "0x12abcg",
        "#12abc",
        "0x777\n",
        "0x0123,456,777",
        "0x012345;6789h",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        int32_t  value;

        ASSERT_EQ(EINVAL, pfc_strtoi32(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[8];
        for (uint32_t i(2); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(7U, pfc_strlcpy(buf, "0x12345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if (pfc_isdigit_u(c) || (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F') || c == '\0') {
                    continue;
                }

                int32_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtoi32(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "-0x80000001",
        "-0x80000002",
        "-0x80000003",
        "-0x100000000",
        "-0x111111111",
        "-0x222222222",
        "-0xaaaaaaaaa",
        "0x80000000",
        "0x80000001",
        "0x80000002",
        "0x100000000",
        "0x111111111",
        "0x222222222",
        "0xaaaaaaaaa",
        "0xfffffffff",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        int32_t  value;

        ASSERT_EQ(ERANGE, pfc_strtoi32(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        int32_t  value, v, c;

        RANDOM_INTEGER(rand, value);
        RANDOM_INTEGER(rand, c);

        if (c & 1) {
            if (value == INT32_MIN) {
                snprintf(str, sizeof(str), "-0x80000000");
            }
            else if (value < 0) {
                snprintf(str, sizeof(str), "-0x%x", -value);
            }
            else {
                snprintf(str, sizeof(str), "0x%x", value);
            }
        }
        else {
            if (value == INT32_MIN) {
                snprintf(str, sizeof(str), "-0X80000000");
            }
            else if (value < 0) {
                snprintf(str, sizeof(str), "-0X%X", -value);
            }
            else {
                snprintf(str, sizeof(str), "0x%X", value);
            }
        }
        ASSERT_EQ(0, pfc_strtoi32(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtoi64()
 * - Decimal string
 */
TEST(strtoint, strtoi64_dec)
{
    // Successful tests.
    const char *strings[] = {
        "-9223372036854775808",         // INT64_MIN
        "-9223372036854775807",
        "-9223372036854775806",
        "-9223372036854775805",
        "-1152921504606846977",
        "-1152921504606846976",         // 0xf000000000000000
        "-1152921504606846975",
        "-2147483649",
        "-2147483648",                  // 0xffffffff80000000
        "-2147483647",
        "-3",
        "-2",
        "-1",
        "-0",
        "0",
        "1",
        "2",
        "3",
        "2147483647",
        "2147483648",                   // 0x80000000
        "2147483649",
        "4294967295",
        "4294967296",                   // 0x100000000
        "4294967297",
        "9223372036854775805",
        "9223372036854775806",
        "9223372036854775807",          // INT64_MAX
    };

    int64_t required[] = {
        INT64_MIN,
        -9223372036854775807LL,
        -9223372036854775806LL,
        -9223372036854775805LL,
        -1152921504606846977LL,
        -1152921504606846976LL,
        -1152921504606846975LL,
        -2147483649LL,
        -2147483648LL,
        -2147483647LL,
        -3LL,
        -2LL,
        -1LL,
        -0LL,
        0LL,
        1LL,
        2LL,
        3LL,
        2147483647LL,
        2147483648LL,
        2147483649LL,
        4294967295LL,
        4294967296LL,
        4294967297LL,
        9223372036854775805LL,
        9223372036854775806LL,
        INT64_MAX,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        int64_t    value;

        ASSERT_EQ(0, pfc_strtoi64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        if (*str != '-') {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtoi64(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }
    }

    // Invalid strings.
    const char *invalid[] = {
        "",
        "-",
        "+",
        " -1",
        "10 ",
        "12abc",
        "777\n",
        "123,456,789",
        "12345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        int64_t  value;

        ASSERT_EQ(EINVAL, pfc_strtoi64(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[6];
        for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(5U, pfc_strlcpy(buf, "12345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if (pfc_isdigit_u(c) || c == '\0' ||
                    (i == 0 && (c == '-' || c == '+'))) {
                    continue;
                }

                int64_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtoi64(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "-9223372036854775809",
        "-9223372036854775810",
        "-9223372036854775811",
        "-10000000000000000000",
        "-100000000000000000000",
        "9223372036854775808",
        "9223372036854775809",
        "9223372036854775810",
        "10000000000000000000",
        "100000000000000000000",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        int64_t  value;

        ASSERT_EQ(ERANGE, pfc_strtoi64(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        int64_t  value, v;

        RANDOM_INTEGER(rand, value);
        snprintf(str, sizeof(str), "%" PFC_PFMT_d64, value);
        ASSERT_EQ(0, pfc_strtoi64(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtoi64()
 * - Octal string
 */
TEST(strtoint, strtoi64_oct)
{
    // Successful tests.
    const char *strings[] = {
        "-01000000000000000000000",     // INT64_MIN
        "-0777777777777777777777",
        "-0777777777777777777776",
        "-0777777777777777777775",
        "-0100000000000000000001",
        "-0100000000000000000000",      // 0xf000000000000000
        "-077777777777777777777",
        "-020000000001",
        "-020000000000",                // 0xffffffff80000000
        "-017777777777",
        "-03",
        "-02",
        "-01",
        "-00",
        "00",
        "01",
        "02",
        "03",
        "017777777777",
        "020000000000",                 // 0x80000000
        "020000000001",
        "037777777777",
        "040000000000",                 // 0x100000000
        "040000000001",
        "0777777777777777777775",
        "0777777777777777777776",
        "0777777777777777777777",       // INT64_MAX
    };

    int64_t required[] = {
        INT64_MIN,
        -9223372036854775807LL,
        -9223372036854775806LL,
        -9223372036854775805LL,
        -1152921504606846977LL,
        -1152921504606846976LL,
        -1152921504606846975LL,
        -2147483649LL,
        -2147483648LL,
        -2147483647LL,
        -3LL,
        -2LL,
        -1LL,
        -0LL,
        0LL,
        1LL,
        2LL,
        3LL,
        2147483647LL,
        2147483648LL,
        2147483649LL,
        4294967295LL,
        4294967296LL,
        4294967297LL,
        9223372036854775805LL,
        9223372036854775806LL,
        INT64_MAX,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        int64_t  value;

        ASSERT_EQ(0, pfc_strtoi64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        if (*str != '-') {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtoi64(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }

        // Leading zeroes must be ignored.
        std::string newstr;
        if (*str == '-') {
            newstr.append("-");
            str++;
        }
        newstr.append("00000000");
        newstr.append(str);
        str = newstr.c_str();
        ASSERT_EQ(0, pfc_strtoi64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;
    }

    // Invalid strings.
    const char *invalid[] = {
        " -01",
        "010 ",
        "012abc",
        "0777\n",
        "0123,456,789",
        "012345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        int64_t  value;

        ASSERT_EQ(EINVAL, pfc_strtoi64(*pp, &value)) << "str=" << *pp;
    }

    // Out of range.
    const char *badrange[] = {
        "-01000000000000000000001",
        "-01000000000000000000002",
        "-01000000000000000000003",
        "-02000000000000000000000",
        "-010000000000000000000000",
        "-011111111111111111111111",
        "-022222222222222222222222",
        "-077777777777777777777777",
        "01000000000000000000000",
        "01000000000000000000001",
        "01000000000000000000002",
        "02000000000000000000000",
        "010000000000000000000000",
        "011111111111111111111111",
        "022222222222222222222222",
        "077777777777777777777777",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        int64_t  value;

        ASSERT_EQ(ERANGE, pfc_strtoi64(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[7];
        for (uint32_t i(1); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(6U, pfc_strlcpy(buf, "012345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if ((c >= '0' && c <= '7') || c == '\0' ||
                    (i == 1 && (c == '\0' || c == 'x' || c == 'X'))) {
                    continue;
                }

                int64_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtoi64(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        int64_t  value, v;

        RANDOM_INTEGER(rand, value);
        if (value == INT64_MIN) {
            snprintf(str, sizeof(str), "-01000000000000000000000");
        }
        else if (value < 0) {
            snprintf(str, sizeof(str), "-0%" PFC_PFMT_o64, -value);
        }
        else {
            snprintf(str, sizeof(str), "0%" PFC_PFMT_o64, value);
        }
        ASSERT_EQ(0, pfc_strtoi64(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtoi64()
 * - Hexadecimal string
 */
TEST(strtoint, strtoi64_hex)
{
    // Successful tests.
    const char *strings[] = {
        "-0x8000000000000000",          // INT64_MAX
        "-0x7fffffffffffffff",
        "-0x7ffffffffffffffe",
        "-0x7ffffffffffffffd",
        "-0x1000000000000001",
        "-0x1000000000000000",          // 0xf000000000000000
        "-0xfffffffffffffff",
        "-0x80000001",
        "-0x80000000",                  // 0xffffffff80000000
        "-0x7fffffff",
        "-0x3",
        "-0x2",
        "-0x1",
        "-0x0",
        "0x0",
        "0x1",
        "0x2",
        "0x3",
        "0x7fffffff",
        "0x80000000",                   // 0x80000000
        "0x80000001",
        "0xffffffff",
        "0x100000000",                  // 0x100000000
        "0x100000001",
        "0x7ffffffffffffffd",
        "0x7ffffffffffffffe",
        "0x7fffffffffffffff",           // INT64_MAX
    };

    int64_t required[] = {
        INT64_MIN,
        -9223372036854775807LL,
        -9223372036854775806LL,
        -9223372036854775805LL,
        -1152921504606846977LL,
        -1152921504606846976LL,
        -1152921504606846975LL,
        -2147483649LL,
        -2147483648LL,
        -2147483647LL,
        -3LL,
        -2LL,
        -1LL,
        -0LL,
        0LL,
        1LL,
        2LL,
        3LL,
        2147483647LL,
        2147483648LL,
        2147483649LL,
        4294967295LL,
        4294967296LL,
        4294967297LL,
        9223372036854775805LL,
        9223372036854775806LL,
        INT64_MAX,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        int64_t  value;

        ASSERT_EQ(0, pfc_strtoi64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        if (*str != '-') {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtoi64(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }

        // Leading zeroes must be ignored.
        std::string newstr;
        if (*str == '-') {
            newstr.append("-");
            str++;
        }
        newstr.append("0x00000000");
        str += 2;
        newstr.append(str);
        str = newstr.c_str();
        ASSERT_EQ(0, pfc_strtoi64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Upper case must be accepted.
        std::string upper;
        size_t sz(newstr.size());
        for (size_t j(0); j < sz; j++) {
            char  c(newstr.at(j));
            c = pfc_toupper(c);
            upper.append(1, c);
        }

        str = upper.c_str();
        ASSERT_EQ(0, pfc_strtoi64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;
    }

    // Invalid strings.
    const char *invalid[] = {
        "0x",
        " -0x1",
        "0x10 ",
        "0x12abcg",
        "#12abc",
        "0x777\n",
        "0x0123,456,777",
        "0x012345;6789h",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        int64_t  value;

        ASSERT_EQ(EINVAL, pfc_strtoi64(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[8];
        for (uint32_t i(2); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(7U, pfc_strlcpy(buf, "0x12345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if (pfc_isdigit_u(c) || (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F') || c == '\0') {
                    continue;
                }

                int64_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtoi64(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "-0x8000000000000001",
        "-0x8000000000000002",
        "-0x8000000000000003",
        "-0x9000000000000000",
        "-0x10000000000000000",
        "-0x001111111111111111111",
        "-0x002222222222222222222",
        "-0xaaaaaaaaaaaaaaaaa",
        "-0xfffffffffffffffff",
        "0x8000000000000000",
        "0x8000000000000001",
        "0x8000000000000002",
        "0x9000000000000000",
        "0x10000000000000000",
        "0x11111111111111111",
        "0x22222222222222222",
        "0xbbbbbbbbbbbbbbbbb",
        "0xfffffffffffffffff",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        int64_t  value;

        ASSERT_EQ(ERANGE, pfc_strtoi64(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        int64_t  value, v, c;

        RANDOM_INTEGER(rand, value);
        RANDOM_INTEGER(rand, c);

        if (c & 1) {
            if (value == INT64_MIN) {
                snprintf(str, sizeof(str), "-0x8000000000000000");
            }
            else if (value < 0) {
                snprintf(str, sizeof(str), "-0x%" PFC_PFMT_x64, -value);
            }
            else {
                snprintf(str, sizeof(str), "0x%" PFC_PFMT_x64, value);
            }
        }
        else {
            if (value == INT64_MIN) {
                snprintf(str, sizeof(str), "-0X8000000000000000");
            }
            else if (value < 0) {
                snprintf(str, sizeof(str), "-0X%" PFC_PFMT_X64, -value);
            }
            else {
                snprintf(str, sizeof(str), "0X%" PFC_PFMT_X64, value);
            }
        }
        ASSERT_EQ(0, pfc_strtoi64(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtou32()
 * - Decimal string
 */
TEST(strtoint, strtou32_dec)
{
    // Successful tests.
    const char *strings[] = {
        "0",
        "1",
        "2",
        "3",
        "268435455",
        "268435456",            // 0x10000000
        "268435457",
        "1879048191",
        "1879048192",           // 0x70000000
        "1879048193",
        "2147483647",
        "2147483648",           // 0x80000000
        "2147483649",
        "4294967293",
        "4294967294",
        "4294967295",           // UINT32_MAX
    };

    uint32_t required[] = {
        0U,
        1U,
        2U,
        3U,
        268435455U,
        268435456U,
        268435457U,
        1879048191U,
        1879048192U,
        1879048193U,
        2147483647U,
        2147483648U,
        2147483649U,
        4294967293U,
        4294967294U,
        4294967295U,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        uint32_t   value;

        ASSERT_EQ(0, pfc_strtou32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtou32(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }
    }

    // Invalid strings.
    const char *invalid[] = {
        "",
        "+",
        "-",
        " 1",
        "10 ",
        "-12345",
        "12abc",
        "777\n",
        "123,456,789",
        "12345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        uint32_t  value;

        ASSERT_EQ(EINVAL, pfc_strtou32(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[6];
        for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(5U, pfc_strlcpy(buf, "12345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if (pfc_isdigit_u(c) || c == '\0' || (i == 0 && c == '+')) {
                    continue;
                }

                uint32_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtou32(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "4294967296",
        "4294967297",
        "4294967298",
        "5000000000",
        "10000000000",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        uint32_t  value;

        ASSERT_EQ(ERANGE, pfc_strtou32(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        uint32_t value, v;

        RANDOM_INTEGER(rand, value);
        snprintf(str, sizeof(str), "%u", value);
        ASSERT_EQ(0, pfc_strtou32(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtou32()
 * - Octal string
 */
TEST(strtoint, strtou32_oct)
{
    // Successful tests.
    const char *strings[] = {
        "00",
        "01",
        "02",
        "03",
        "01777777777",
        "02000000000",          // 0x10000000
        "02000000001",
        "015777777777",
        "016000000000",         // 0x70000000
        "016000000001",
        "017777777777",
        "020000000000",         // 0x80000000
        "020000000001",
        "037777777775",
        "037777777776",
        "037777777777",         // UINT32_MAX
    };

    uint32_t required[] = {
        0U,
        1U,
        2U,
        3U,
        268435455U,
        268435456U,
        268435457U,
        1879048191U,
        1879048192U,
        1879048193U,
        2147483647U,
        2147483648U,
        2147483649U,
        4294967293U,
        4294967294U,
        4294967295U,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        uint32_t   value;

        ASSERT_EQ(0, pfc_strtou32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtou32(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }

        // Leading zeroes must be ignored.
        std::string newstr;
        if (*str == '-') {
            newstr.append("-");
            str++;
        }
        newstr.append("00000000");
        newstr.append(str);
        str = newstr.c_str();
        ASSERT_EQ(0, pfc_strtou32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;
    }

    // Invalid strings.
    const char *invalid[] = {
        " 01",
        "010 ",
        "-012345",
        "012abc",
        "0777\n",
        "0123,456,789",
        "012345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        uint32_t  value;

        ASSERT_EQ(EINVAL, pfc_strtou32(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[7];
        for (uint32_t i(1); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(6U, pfc_strlcpy(buf, "012345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if ((c >= '0' && c <= '7') || c == '\0' ||
                    (i == 1 && (c == '\0' || c == 'x' || c == 'X'))) {
                    continue;
                }

                uint32_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtou32(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "040000000000",
        "040000000001",
        "040000000002",
        "050000000000",
        "0100000000000",
        "0111111111111",
        "0222222222222",
        "0444444444444",
        "0777777777777",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        uint32_t  value;

        ASSERT_EQ(ERANGE, pfc_strtou32(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        uint32_t value, v;

        RANDOM_INTEGER(rand, value);
        snprintf(str, sizeof(str), "0%o", value);
        ASSERT_EQ(0, pfc_strtou32(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtou32()
 * - Hexadecimal string
 */
TEST(strtoint, strtou32_hex)
{
    // Successful tests.
    const char *strings[] = {
        "0x0",
        "0x1",
        "0x2",
        "0x3",
        "0xfffffff",
        "0x10000000",
        "0x10000001",
        "0x6fffffff",
        "0x70000000",
        "0x70000001",
        "0x7fffffff",
        "0x80000000",
        "0x80000001",
        "0xfffffffd",
        "0xfffffffe",
        "0xffffffff",           // UINT32_MAX
    };

    uint32_t required[] = {
        0U,
        1U,
        2U,
        3U,
        268435455U,
        268435456U,
        268435457U,
        1879048191U,
        1879048192U,
        1879048193U,
        2147483647U,
        2147483648U,
        2147483649U,
        4294967293U,
        4294967294U,
        4294967295U,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        uint32_t   value;

        ASSERT_EQ(0, pfc_strtou32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtou32(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }

        // Leading zeroes must be ignored.
        std::string newstr;
        if (*str == '-') {
            newstr.append("-");
            str++;
        }
        newstr.append("0x00000000");
        str += 2;
        newstr.append(str);
        str = newstr.c_str();
        ASSERT_EQ(0, pfc_strtou32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Upper case must be accepted.
        std::string upper;
        size_t sz(newstr.size());
        for (size_t j(0); j < sz; j++) {
            char  c(newstr.at(j));
            c = pfc_toupper(c);
            upper.append(1, c);
        }

        str = upper.c_str();
        ASSERT_EQ(0, pfc_strtou32(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;
    }

    // Invalid strings.
    const char *invalid[] = {
        "0x",
        " 0x1",
        "0x10 ",
        "-0x12345",
        "0x12abcg",
        "#12abc",
        "0x777\n",
        "0x123,456,789",
        "0x12345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        uint32_t  value;

        ASSERT_EQ(EINVAL, pfc_strtou32(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[8];
        for (uint32_t i(2); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(7U, pfc_strlcpy(buf, "0x12345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if (pfc_isdigit_u(c) || (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F') || c == '\0') {
                    continue;
                }

                uint32_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtou32(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "0x100000000",
        "0x100000001",
        "0x100000002",
        "0x200000000",
        "0x500000000",
        "0x1000000000",
        "0x1111111111",
        "0x2222222222",
        "0xaaaaaaaaaa",
        "0xbbbbbbbbbb",
        "0xcccccccccc",
        "0xffffffffff",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        uint32_t  value;

        ASSERT_EQ(ERANGE, pfc_strtou32(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        uint32_t value, v, c;

        RANDOM_INTEGER(rand, value);
        RANDOM_INTEGER(rand, c);

        if (c & 1) {
            snprintf(str, sizeof(str), "0x%x", value);
        }
        else {
            snprintf(str, sizeof(str), "0X%X", value);
        }
        ASSERT_EQ(0, pfc_strtou32(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtou64()
 * - Decimal string
 */
TEST(strtoint, strtou64_dec)
{
    // Successful tests.
    const char *strings[] = {
        "0",
        "1",
        "2",
        "3",
        "268435455",
        "268435456",            // 0x10000000
        "268435457",
        "1879048191",
        "1879048192",           // 0x70000000
        "1879048193",
        "2147483647",
        "2147483648",           // 0x80000000
        "2147483649",
        "4294967293",
        "4294967294",
        "4294967295",
        "4294967296",           // 0x100000000
        "4294967297",
        "9223372036854775807",
        "9223372036854775808",  // 0x8000000000000000
        "9223372036854775809",
        "18446744071562067967",
        "18446744071562067968", // 0xffffffff80000000
        "18446744071562067969",
        "18446744073709551613",
        "18446744073709551614",
        "18446744073709551615"  // UINT64_MAX
    };

    uint64_t required[] = {
        0ULL,
        1ULL,
        2ULL,
        3ULL,
        268435455ULL,
        268435456ULL,
        268435457ULL,
        1879048191ULL,
        1879048192ULL,
        1879048193ULL,
        2147483647ULL,
        2147483648ULL,
        2147483649ULL,
        4294967293ULL,
        4294967294ULL,
        4294967295ULL,
        4294967296ULL,
        4294967297ULL,
        9223372036854775807ULL,
        9223372036854775808ULL,
        9223372036854775809ULL,
        18446744071562067967ULL,
        18446744071562067968ULL,
        18446744071562067969ULL,
        18446744073709551613ULL,
        18446744073709551614ULL,
        18446744073709551615ULL,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        uint64_t   value;

        ASSERT_EQ(0, pfc_strtou64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtou64(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }
    }

    // Invalid strings.
    const char *invalid[] = {
        " 1",
        "10 ",
        "-12345",
        "12abc",
        "777\n",
        "123,456,789",
        "12345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        uint64_t  value;

        ASSERT_EQ(EINVAL, pfc_strtou64(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[6];
        for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(5U, pfc_strlcpy(buf, "12345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if (pfc_isdigit_u(c) || c == '\0' || (i == 0 && c == '+')) {
                    continue;
                }

                uint64_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtou64(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "18446744073709551616",
        "18446744073709551617",
        "18446744073709551618",
        "19000000000000000000",
        "20000000000000000000",
        "100000000000000000000",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        uint64_t  value;

        ASSERT_EQ(ERANGE, pfc_strtou64(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        uint64_t value, v;

        RANDOM_INTEGER(rand, value);
        snprintf(str, sizeof(str), "%" PFC_PFMT_u64, value);
        ASSERT_EQ(0, pfc_strtou64(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtou64()
 * - Octal string
 */
TEST(strtoint, strtou64_oct)
{
    // Successful tests.
    const char *strings[] = {
        "00",
        "01",
        "02",
        "03",
        "01777777777",
        "02000000000",                  // 0x10000000
        "02000000001",
        "015777777777",
        "016000000000",                 // 0x70000000
        "016000000001",
        "017777777777",
        "020000000000",                 // 0x80000000
        "020000000001",
        "037777777775",
        "037777777776",
        "037777777777",
        "040000000000",                 // 0x100000000
        "040000000001",
        "0777777777777777777777",
        "01000000000000000000000",      // 0x8000000000000000
        "01000000000000000000001",
        "01777777777757777777777",
        "01777777777760000000000",      // 0xffffffff80000000
        "01777777777760000000001",
        "01777777777777777777775",
        "01777777777777777777776",
        "01777777777777777777777",      // UINT64_MAX
    };

    uint64_t required[] = {
        0ULL,
        1ULL,
        2ULL,
        3ULL,
        268435455ULL,
        268435456ULL,
        268435457ULL,
        1879048191ULL,
        1879048192ULL,
        1879048193ULL,
        2147483647ULL,
        2147483648ULL,
        2147483649ULL,
        4294967293ULL,
        4294967294ULL,
        4294967295ULL,
        4294967296ULL,
        4294967297ULL,
        9223372036854775807ULL,
        9223372036854775808ULL,
        9223372036854775809ULL,
        18446744071562067967ULL,
        18446744071562067968ULL,
        18446744071562067969ULL,
        18446744073709551613ULL,
        18446744073709551614ULL,
        18446744073709551615ULL,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        uint64_t  value;

        ASSERT_EQ(0, pfc_strtou64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtou64(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }

        // Leading zeroes must be ignored.
        std::string newstr;
        if (*str == '-') {
            newstr.append("-");
            str++;
        }
        newstr.append("00000000");
        newstr.append(str);
        str = newstr.c_str();
        ASSERT_EQ(0, pfc_strtou64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;
    }

    // Invalid strings.
    const char *invalid[] = {
        " 01",
        "010 ",
        "-012345",
        "012abc",
        "0777\n",
        "0123,456,789",
        "012345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        uint64_t  value;

        ASSERT_EQ(EINVAL, pfc_strtou64(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[7];
        for (uint32_t i(1); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(6U, pfc_strlcpy(buf, "012345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if ((c >= '0' && c <= '7') || c == '\0' ||
                    (i == 1 && (c == '\0' || c == 'x' || c == 'X'))) {
                    continue;
                }

                uint64_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtou64(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "02000000000000000000000",
        "02000000000000000000001",
        "02000000000000000000002",
        "03000000000000000000000",
        "04000000000000000000000",
        "05000000000000000000000",
        "010000000000000000000000",
        "011111111111111111111111",
        "022222222222222222222222",
        "044444444444444444444444",
        "077777777777777777777777",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        uint64_t  value;

        ASSERT_EQ(ERANGE, pfc_strtou64(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        uint64_t value, v;

        RANDOM_INTEGER(rand, value);
        snprintf(str, sizeof(str), "0%" PFC_PFMT_o64, value);
        ASSERT_EQ(0, pfc_strtou64(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}

/*
 * Test for pfc_strtou64()
 * - Hexadecimal string
 */
TEST(strtoint, strtou64_hex)
{
    // Successful tests.
    const char *strings[] = {
        "0x0",
        "0x1",
        "0x2",
        "0x3",
        "0xfffffff",
        "0x10000000",
        "0x10000001",
        "0x6fffffff",
        "0x70000000",
        "0x70000001",
        "0x7fffffff",
        "0x80000000",
        "0x80000001",
        "0xfffffffd",
        "0xfffffffe",
        "0xffffffff",
        "0x100000000",
        "0x100000001",
        "0x7fffffffffffffff",
        "0x8000000000000000",
        "0x8000000000000001",
        "0xffffffff7fffffff",
        "0xffffffff80000000",
        "0xffffffff80000001",
        "0xfffffffffffffffd",
        "0xfffffffffffffffe",
        "0xffffffffffffffff",           // UINT64_MAX
    };

    uint64_t required[] = {
        0ULL,
        1ULL,
        2ULL,
        3ULL,
        268435455ULL,
        268435456ULL,
        268435457ULL,
        1879048191ULL,
        1879048192ULL,
        1879048193ULL,
        2147483647ULL,
        2147483648ULL,
        2147483649ULL,
        4294967293ULL,
        4294967294ULL,
        4294967295ULL,
        4294967296ULL,
        4294967297ULL,
        9223372036854775807ULL,
        9223372036854775808ULL,
        9223372036854775809ULL,
        18446744071562067967ULL,
        18446744071562067968ULL,
        18446744071562067969ULL,
        18446744073709551613ULL,
        18446744073709551614ULL,
        18446744073709551615ULL,
    };
    for (uint32_t i(0); i < PFC_ARRAY_CAPACITY(strings); i++) {
        const char *str(strings[i]);
        uint64_t  value;

        ASSERT_EQ(0, pfc_strtou64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Plus sign must be accepted.
        {
            std::string  plus("+");
            plus.append(str);
            const char   *pstr(plus.c_str());
            ASSERT_EQ(0, pfc_strtou64(pstr, &value)) << "str=" << pstr;
            ASSERT_EQ(required[i], value) << "str=" << str;
        }

        // Leading zeroes must be ignored.
        std::string newstr;
        if (*str == '-') {
            newstr.append("-");
            str++;
        }
        newstr.append("0x00000000");
        str += 2;
        newstr.append(str);
        str = newstr.c_str();
        ASSERT_EQ(0, pfc_strtou64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;

        // Upper case must be accepted.
        std::string upper;
        size_t sz(newstr.size());
        for (size_t j(0); j < sz; j++) {
            char  c(newstr.at(j));
            c = pfc_toupper(c);
            upper.append(1, c);
        }

        str = upper.c_str();
        ASSERT_EQ(0, pfc_strtou64(str, &value)) << "str=" << str;
        ASSERT_EQ(required[i], value) << "str=" << str;
    }

    // Invalid strings.
    const char *invalid[] = {
        "0x",
        " 0x1",
        "0x10 ",
        "-0x12345",
        "#12abc",
        "0x12abcg",
        "0777\n",
        "0123,456,789",
        "012345;6789",
    };
    for (const char **pp(invalid); pp < PFC_ARRAY_LIMIT(invalid); pp++) {
        uint64_t  value;

        ASSERT_EQ(EINVAL, pfc_strtou64(*pp, &value)) << "str=" << *pp;
    }
    {
        char  buf[8];
        for (uint32_t i(2); i < PFC_ARRAY_CAPACITY(buf) - 1; i++) {
            ASSERT_EQ(7U, pfc_strlcpy(buf, "0x12345", sizeof(buf)));

            for (uint16_t j(0); j < UINT8_MAX; j++) {
                uint8_t c(static_cast<uint8_t>(j));

                if (pfc_isdigit_u(c) || (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F') || c == '\0') {
                    continue;
                }

                uint64_t value;

                buf[i] = c;
                ASSERT_EQ(EINVAL, pfc_strtou64(buf, &value))
                    << "str=" << buf;
            }
        }
    }

    // Out of range.
    const char *badrange[] = {
        "0x10000000000000000",
        "0x10000000000000001",
        "0x10000000000000002",
        "0x10000000000000003",
        "0x20000000000000000",
        "0x30000000000000000",
        "0x40000000000000000",
        "0x100000000000000000",
        "0x1000000000000000000",
        "0x1111111111111111111",
        "0x2222222222222222222",
        "0x4444444444444444444",
        "0xaaaaaaaaaaaaaaaaaaa",
        "0xddddddddddddddddddd",
        "0xeeeeeeeeeeeeeeeeeee",
        "0xfffffffffffffffffff",
    };
    for (const char **pp(badrange); pp < PFC_ARRAY_LIMIT(badrange); pp++) {
        uint64_t  value;

        ASSERT_EQ(ERANGE, pfc_strtou64(*pp, &value)) << "str=" << *pp;
    }

    // Random value test.
    RandomGenerator rand;
    for (int loop(0); loop < RANDOM_LOOP; loop++) {
        char     str[32];
        uint64_t value, v;

        RANDOM_INTEGER(rand, value);
        snprintf(str, sizeof(str), "0x%" PFC_PFMT_x64, value);
        ASSERT_EQ(0, pfc_strtou64(str, &v)) << "str=" << str;
        ASSERT_EQ(value, v);
    }
}
