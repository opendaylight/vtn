/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_bitops.cc - Test for functions defined in pfc/byteorder.h.
 */

#include <gtest/gtest.h>
#include <pfc/base.h>
#include "random.hh"

#define	NBITS_PER_BYTE		8
#define	TEST_LOOP		4096

#ifdef	PFC_LITTLE_ENDIAN
/* ntohll() and htonll() must swap bytes in uint64_t. */
#define	LL_SWAP_REQUIRED	true
#else	/* !PFC_LITTLE_ENDIAN */
/* ntohll() and htonll() must not change bytes in uint64_t. */
#define	LL_SWAP_REQUIRED	false
#endif	/* PFC_LITTLE_ENDIAN */

/*
 * template <class T>
 * static void
 * swapBytes(T &value)
 *	Swap bytes in the specified integer value.
 */
template <class T>
static void
swapBytes(T &value)
{
    T	v(value), result(0);

    for (int nbytes(sizeof(T)); nbytes > 0; nbytes--) {
        uint8_t	byte(static_cast<uint8_t>(v & 0xff));

        result = (result << NBITS_PER_BYTE) | byte;
        v >>= NBITS_PER_BYTE;
    }

    value = result;
}

/*
 * template <class T>
 * static void
 * networkByteOrder(T &value)
 *	Convert byte order between host and network byte order.
 */
template <class T>
static void
networkByteOrder(T &value)
{
#ifdef	PFC_LITTLE_ENDIAN
    swapBytes(value);
#endif	/* PFC_LITTLE_ENDIAN */
}

/*
 * Declare body of test case which tests the specified macro.
 */
#define	BYTEORDER_TEST(type, macro, swap_required)                      \
    {                                                                   \
        type	value(0);                                               \
        			                                        \
        /* No bit set in value. */                                      \
        ASSERT_EQ(0U, macro(value));                                    \
        			                                        \
        /* All bits are set. */                                         \
        value = static_cast<type>(-1);                                  \
        ASSERT_EQ(static_cast<type>(-1), macro(value));                 \
                                                                        \
        /* Random value test. */                                        \
        RandomGenerator rand;                                           \
        for (int i(0); i < TEST_LOOP; i++) {                            \
            RANDOM_INTEGER(rand, value);                                \
            type	swapped(value);                                 \
                                                                        \
            if (swap_required) {                                        \
                swapBytes(swapped);                                     \
            }                                                           \
            ASSERT_EQ(swapped, macro(value));                           \
        }                                                               \
    }

/*
 * Declare test case for PFC_BSWAP_XXX() macro.
 */
#define	BSWAP_TEST_DECL(bits)                                           \
    TEST(byteorder, BSWAP_##bits)                                       \
    {                                                                   \
        BYTEORDER_TEST(uint##bits##_t, PFC_BSWAP_##bits, true);         \
    }

/*
 * Declare test case for ntohll() and htonll().
 */
#define	LL_TEST_DECL(macro)                                      \
    TEST(byteorder, macro)                                       \
    {                                                            \
        BYTEORDER_TEST(uint64_t, macro, LL_SWAP_REQUIRED);       \
    }

/*
 * Below are test cases.
 */

/*
 * Ensure that PFC_LITTLE_ENDIAN or PFC_BIG_ENDIAN is defined correctly.
 */
TEST(byteorder, endianness)
{
    // Detect endianness.
    uint32_t	value;
    volatile uint8_t	*ptr(reinterpret_cast<volatile uint8_t *>(&value));

    *ptr = 0x12;
    *(ptr + 1) = 0x34;
    *(ptr + 2) = 0x56;
    *(ptr + 3) = 0x78;

#ifdef	PFC_LITTLE_ENDIAN

#ifdef	PFC_BIG_ENDIAN
    FAIL() << "Both PFC_LITTLE_ENDIAN and PFC_BIG_ENDIAN are defined.";
#endif	/* PFC_BIG_ENDIAN */
    ASSERT_EQ(0x78563412U, value);

#else	/* !PFC_LITTLE_ENDIAN */

#ifndef	PFC_BIG_ENDIAN
    FAIL() << "Neither PFC_LITTLE_ENDIAN nor PFC_BIG_ENDIAN is defined.";
#endif	/* !PFC_BIG_ENDIAN */
    ASSERT_EQ(0x12345678U, value);

#endif	/* PFC_LITTLE_ENDIAN */
}

/*
 * Test for PFC_BSWAP_XX() macros.
 */
BSWAP_TEST_DECL(16);
BSWAP_TEST_DECL(32);
BSWAP_TEST_DECL(64);

/*
 * Test for ntohll() and htonll().
 */
LL_TEST_DECL(ntohll);
LL_TEST_DECL(htonll);
