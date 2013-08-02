/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_bitops.cc - Test for functions defined in pfc/bitpos.h.
 */

#include <gtest/gtest.h>
#include <pfc/bitpos.h>
#include "random.hh"

#define	NBITSOF(type)		(sizeof(type) << 3)
#define	TEST_RANDOM_BIT_LOOP	1024

/*
 * template <class T>
 * static int
 * get_highbit(T value)
 *	Return the highest bit number set in the given value.
 *	Negative value is returned if no bit is set.
 */
template <class T>
static int
get_highbit(T value)
{
    const int	nbits(NBITSOF(T));
    int	num(nbits - 1);

    for (; num >= 0; num--) {
        T	mask(static_cast<T>(1) << num);

        if (value & mask) {
            break;
        }
    }

    return num;
}

/*
 * template <class T>
 * static int
 * get_lowbit(T value)
 *	Return the lowest bit number set in the given value.
 *	Negative value is returned if no bit is set.
 */
template <class T>
static int
get_lowbit(T value)
{
    const int	nbits(NBITSOF(T));
    int	num(0);

    for (; num < nbits; num++) {
        T	mask(static_cast<T>(1) << num);

        if (value & mask) {
            break;
        }
    }

    return num;
}

/*
 * Below are test cases.
 */

/*
 * Test for pfc_highbit_uint32(), which returns number of the highest bit set
 * in the given 32-bit integer.
 */
TEST(bitpos, highbit_uint32)
{
    uint32_t	value(0);
    const int	nbits(NBITSOF(value));

    // No bit set in value.
    ASSERT_LT(pfc_highbit_uint32(value), 0);

    // All bits are set.
    value = static_cast<uint32_t>(-1);
    ASSERT_EQ(nbits - 1, pfc_highbit_uint32(value));

    // Check all bits.
    for (int i(0); i < nbits; i++) {
        value = (static_cast<uint32_t>(1) << i);
        ASSERT_EQ(i, pfc_highbit_uint32(value));
    }

    // Random value test.
    RandomGenerator rand;
    for (int i(0); i < TEST_RANDOM_BIT_LOOP; i++) {
        RANDOM_INTEGER(rand, value);
        int high(get_highbit(value));

        if (high < 0) {
            ASSERT_LT(pfc_highbit_uint32(value), 0);
        }
        else {
            ASSERT_EQ(high, pfc_highbit_uint32(value));
        }
    }
}

/*
 * Test for pfc_highbit_uint64(), which returns number of the highest bit set
 * in the given 64-bit integer.
 */
TEST(bitpos, highbit_uint64)
{
    uint64_t	value(0);
    const int	nbits(NBITSOF(value));

    if (PFC_EXPECT_FALSE(HasFatalFailure())) {
        return;
    }

    // No bit set in value.
    ASSERT_LT(pfc_highbit_uint64(value), 0);

    // All bits are set.
    value = static_cast<uint64_t>(-1);
    ASSERT_EQ(nbits - 1, pfc_highbit_uint64(value));

    // Check all bits.
    for (int i(0); i < nbits; i++) {
        value = (static_cast<uint64_t>(1) << i);
        ASSERT_EQ(i, pfc_highbit_uint64(value));
    }

    // Random value test.
    RandomGenerator rand;
    for (int i(0); i < TEST_RANDOM_BIT_LOOP; i++) {
        RANDOM_INTEGER(rand, value);
        int high(get_highbit(value));

        if (high < 0) {
            ASSERT_LT(pfc_highbit_uint64(value), 0);
        }
        else {
            ASSERT_EQ(high, pfc_highbit_uint64(value));
        }
    }
}

/*
 * Test for pfc_highbit_ulong(), which returns number of the highest bit set
 * in the given unsigned long integer.
 */
TEST(bitpos, highbit_ulong)
{
    pfc_ulong_t	value(0);
    const int	nbits(NBITSOF(value));

    if (PFC_EXPECT_FALSE(HasFatalFailure())) {
        return;
    }

    // No bit set in value.
    ASSERT_LT(pfc_highbit_ulong(value), 0);

    // All bits are set.
    value = static_cast<pfc_ulong_t>(-1);
    ASSERT_EQ(nbits - 1, pfc_highbit_ulong(value));

    // Check all bits.
    for (int i(0); i < nbits; i++) {
        value = (static_cast<pfc_ulong_t>(1) << i);
        ASSERT_EQ(i, pfc_highbit_ulong(value));
    }

    // Random value test.
    RandomGenerator rand;
    for (int i(0); i < TEST_RANDOM_BIT_LOOP; i++) {
        RANDOM_INTEGER(rand, value);
        int high(get_highbit(value));

        if (high < 0) {
            ASSERT_LT(pfc_highbit_ulong(value), 0);
        }
        else {
            ASSERT_EQ(high, pfc_highbit_ulong(value));
        }
    }
}

/*
 * Test for pfc_lowbit_uint32(), which returns number of the lowest bit set
 * in the given 32-bit integer.
 */
TEST(bitpos, lowbit_uint32)
{
    uint32_t	value(0);
    const int	nbits(NBITSOF(value));

    if (PFC_EXPECT_FALSE(HasFatalFailure())) {
        return;
    }

    // No bit set in value.
    ASSERT_LT(pfc_lowbit_uint32(value), 0);

    // All bits are set.
    value = static_cast<uint32_t>(-1);
    ASSERT_EQ(0, pfc_lowbit_uint32(value));

    // Check all bits.
    for (int i(0); i < nbits; i++) {
        value = (static_cast<uint32_t>(1) << i);
        ASSERT_EQ(i, pfc_lowbit_uint32(value));
    }

    // Random value test.
    RandomGenerator rand;
    for (int i(0); i < TEST_RANDOM_BIT_LOOP; i++) {
        RANDOM_INTEGER(rand, value);
        int low(get_lowbit(value));

        if (low < 0) {
            ASSERT_LT(pfc_lowbit_uint32(value), 0);
        }
        else {
            ASSERT_EQ(low, pfc_lowbit_uint32(value));
        }
    }
}

/*
 * Test for pfc_lowbit_uint64(), which returns number of the lowest bit set
 * in the given 64-bit integer.
 */
TEST(bitpos, lowbit_uint64)
{
    uint64_t	value(0);
    const int	nbits(NBITSOF(value));

    if (PFC_EXPECT_FALSE(HasFatalFailure())) {
        return;
    }

    // No bit set in value.
    ASSERT_LT(pfc_lowbit_uint64(value), 0);

    // All bits are set.
    value = static_cast<uint64_t>(-1);
    ASSERT_EQ(0, pfc_lowbit_uint64(value));

    // Check all bits.
    for (int i(0); i < nbits; i++) {
        value = (static_cast<uint64_t>(1) << i);
        ASSERT_EQ(i, pfc_lowbit_uint64(value));
    }

    // Random value test.
    RandomGenerator rand;
    for (int i(0); i < TEST_RANDOM_BIT_LOOP; i++) {
        RANDOM_INTEGER(rand, value);
        int low(get_lowbit(value));

        if (low < 0) {
            ASSERT_LT(pfc_lowbit_uint64(value), 0);
        }
        else {
            ASSERT_EQ(low, pfc_lowbit_uint64(value));
        }
    }
}

/*
 * Test for pfc_lowbit_ulong(), which returns number of the lowest bit set
 * in the given unsigned long integer.
 */
TEST(bitpos, lowbit_ulong)
{
    pfc_ulong_t	value(0);
    const int	nbits(NBITSOF(value));

    if (PFC_EXPECT_FALSE(HasFatalFailure())) {
        return;
    }

    // No bit set in value.
    ASSERT_LT(pfc_lowbit_ulong(value), 0);

    // All bits are set.
    value = static_cast<pfc_ulong_t>(-1);
    ASSERT_EQ(0, pfc_lowbit_ulong(value));

    // Check all bits.
    for (int i(0); i < nbits; i++) {
        value = (static_cast<pfc_ulong_t>(1) << i);
        ASSERT_EQ(i, pfc_lowbit_ulong(value));
    }

    // Random value test.
    RandomGenerator rand;
    for (int i(0); i < TEST_RANDOM_BIT_LOOP; i++) {
        RANDOM_INTEGER(rand, value);
        int low(get_lowbit(value));

        if (low < 0) {
            ASSERT_LT(pfc_lowbit_ulong(value), 0);
        }
        else {
            ASSERT_EQ(low, pfc_lowbit_ulong(value));
        }
    }
}
