/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.math.BigInteger;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link NumberUtils}.
 */
public class NumberUtilsTest extends TestBase {
    /**
     * Verify static constants.
     */
    @Test
    public void testConstants() {
        assertEquals(4, NumberUtils.NUM_OCTETS_INTEGER);
        assertEquals(0xff, NumberUtils.MASK_BYTE);
        assertEquals(0xffff, NumberUtils.MASK_SHORT);
    }

    /**
     * Test case for {@link NumberUtils#hashCode(long)} and
     * {@link NumberUtils#hashCode(double)}.
     */
    @Test
    public void testHashCode() {
        Random rand = new Random();
        for (int i = 0; i < 50; i++) {
            int hi = rand.nextInt();
            int lo = rand.nextInt();
            long l = ((long)hi << Integer.SIZE) | (long)(lo & 0xffffffffL);
            assertEquals(hi ^ lo, NumberUtils.hashCode(l));

            double d = rand.nextDouble();
            int h = NumberUtils.hashCode(d);
            assertEquals(h, NumberUtils.hashCode(d));
        }
    }

    /**
     * Test case for {@link NumberUtils#setInt(byte[], int, int)}.
     */
    @Test
    public void testSetInt() {
        int size = 32;
        int value = 0xa1b2c3d4;
        byte[] bytes = {(byte)0xa1, (byte)0xb2, (byte)0xc3, (byte)0xd4};
        for (int off = 0; off <= size - 4; off++) {
            byte[] buffer = new byte[size];
            NumberUtils.setInt(buffer, off, value);
            for (int i = 0; i < buffer.length; i++) {
                if (i >= off && i < off + 4) {
                    assertEquals(bytes[i - off], buffer[i]);
                } else {
                    assertEquals((byte)0, buffer[i]);
                }
            }
        }
    }

    /**
     * Test case for {@link NumberUtils#setShort(byte[], int, short)}.
     */
    @Test
    public void testSetShort() {
        int size = 32;
        short value = (short)0xa1b2;
        byte[] bytes = {(byte)0xa1, (byte)0xb2};
        for (int off = 0; off <= size - 2; off++) {
            byte[] buffer = new byte[size];
            NumberUtils.setShort(buffer, off, value);
            for (int i = 0; i < buffer.length; i++) {
                if (i >= off && i < off + 2) {
                    assertEquals(bytes[i - off], buffer[i]);
                } else {
                    assertEquals((byte)0, buffer[i]);
                }
            }
        }
    }

    /**
     * Test case for {@link NumberUtils#toInteger(byte[])} and
     * {@link NumberUtils#toBytes(int)}.
     */
    @Test
    public void testToIntegerBytes() {
        try {
            NumberUtils.toInteger((byte[])null);
            unexpected();
        } catch (NullPointerException e) {
        }

        for (int i = 0; i <= 10; i++) {
            if (i == 4) {
                continue;
            }

            byte[] b = new byte[i];
            try {
                NumberUtils.toInteger(b);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid byte array length: " + i,
                             e.getMessage());
            }
        }

        HashMap<byte[], Integer> cases = new HashMap<>();
        cases.put(new byte[]{0, 0, 0, 0}, 0);
        cases.put(new byte[]{0, 0, 0, 1}, 1);
        cases.put(new byte[]{0, 0, 0, 2}, 2);
        cases.put(new byte[]{0, 0, 0, (byte)0xff}, 255);
        cases.put(new byte[]{0, 0, (byte)0x10, (byte)0xff}, 4351);
        cases.put(new byte[]{0, (byte)0xc3, (byte)0x08, (byte)0xac}, 12781740);
        cases.put(new byte[]{(byte)0x18, (byte)0xed, (byte)0x3c, (byte)0x19},
                  418200601);
        cases.put(new byte[]{(byte)0x1f, (byte)0xff, (byte)0xff, (byte)0xff},
                  536870911);
        cases.put(new byte[]{(byte)0x7f, (byte)0x80, (byte)0xc1, (byte)0xd2},
                  2139144658);
        cases.put(new byte[]{(byte)0x7f, (byte)0xff, (byte)0xff, (byte)0xfe},
                  2147483646);
        cases.put(new byte[]{(byte)0x7f, (byte)0xff, (byte)0xff, (byte)0xff},
                  2147483647);
        cases.put(new byte[]{(byte)0x80, (byte)0x00, (byte)0x00, (byte)0x00},
                  -2147483648);
        cases.put(new byte[]{(byte)0x80, (byte)0x00, (byte)0x00, (byte)0x01},
                  -2147483647);
        cases.put(new byte[]{(byte)0x9a, (byte)0x1b, (byte)0x7c, (byte)0x83},
                  -1709474685);
        cases.put(new byte[]{(byte)0xde, (byte)0xad, (byte)0xbe, (byte)0xef},
                  -559038737);
        cases.put(new byte[]{(byte)0xff, (byte)0xfc, (byte)0x83, (byte)0x7a},
                  -228486);
        cases.put(new byte[]{(byte)0xff, (byte)0xff, (byte)0xff, (byte)0xfd},
                  -3);
        cases.put(new byte[]{(byte)0xff, (byte)0xff, (byte)0xff, (byte)0xfe},
                  -2);
        cases.put(new byte[]{(byte)0xff, (byte)0xff, (byte)0xff, (byte)0xff},
                  -1);

        for (Map.Entry<byte[], Integer> entry: cases.entrySet()) {
            byte[] b = entry.getKey();
            int i = entry.getValue().intValue();
            assertEquals(i, NumberUtils.toInteger(b));
            assertArrayEquals(b, NumberUtils.toBytes(i));
        }
    }

    /**
     * Test case for {@link NumberUtils#getUnsigned(byte)}.
     */
    @Test
    public void testGetUnsignedByte() {
        for (int i = 0; i < 256; i++) {
            byte b = (byte)i;
            assertEquals(i, NumberUtils.getUnsigned(b));
        }
    }

    /**
     * Test case for {@link NumberUtils#getUnsigned(short)}.
     */
    @Test
    public void testGetUnsignedShort() {
        int[] values = {
            0, 1, 2, 3, 4, 5, 0x10, 0x123, 0x1234, 0x4567, 0x5678, 0x6789,
            0x789a, 0x7fe0, 0x7ffe, 0x7fff,
            0x8000, 0x8001, 0x8002, 0x8100, 0x8138, 0x9abc, 0xabcd, 0xbcde,
            0xcdef, 0xdef0, 0xef01, 0xffe0, 0xfffa, 0xfffc, 0xfffe, 0xffff,
        };
        for (int value: values) {
            short s = (short)value;
            if (value >= 0x8000) {
                assertTrue(s < 0);
            } else {
                assertTrue(s >= 0);
            }
            assertEquals(value, NumberUtils.getUnsigned(s));
        }
    }

    /**
     * Test case for {@link NumberUtils#getUnsigned(int)}.
     */
    @Test
    public void testGetUnsignedInt() {
        long[] values = {
            0L, 1L, 2L, 3L, 4L, 5L, 0x10L, 0x123L, 0x1234L, 0x4567L, 0x5678L,
            0x12345L, 0x123456L, 0x1234567L, 0x12345678L, 0x5abcdef0L,
            0x70000000L, 0x7ffffff0L, 0x7ffffffdL, 0x7ffffffeL, 0x7fffffffL,
            0x80000000L, 0x80000001L, 0x80000002L, 0x81000000L, 0x81234567L,
            0x9abcdef0L, 0xaabbccddL, 0xbbccddeeL, 0xccddeeffL, 0xddeeff11L,
            0xe0000000L, 0xfff00000L, 0xfffca000L, 0xfffff000L, 0xfffff812L,
            0xfffffff0L, 0xfffffffcL, 0xfffffffdL, 0xfffffffeL, 0xffffffffL,
        };
        for (long value: values) {
            int i = (int)value;
            if (value >= 0x80000000L) {
                assertTrue(i < 0);
            } else {
                assertTrue(i >= 0);
            }
            assertEquals(value, NumberUtils.getUnsigned(i));
        }
    }

    /**
     * Test case for {@link NumberUtils#getUnsigned(long)}.
     */
    @Test
    public void testGetUnsignedLong() {
        long[] positives = {
            0L, 1L, 2L, 3L, 4L, 5L, 0x10L, 0x123L, 0x1234L, 0x4567L,
            0x1fffffffL, 0x7fffffffL, 0x80000000L, 0x80000001L, 0xaaaaaaaaL,
            0xfffffff0L, 0xffffffffL, 0x100000000L, 0xabcdef0123456L,
            0xfffffffffffffffL, 0x1000000000000000L, 0x5555555555555555L,
            0x7000000000000000L, 0x7fffffffffffff00L, 0x7ffffffffffffff0L,
            0x7ffffffffffffffdL, 0x7ffffffffffffffeL, 0x7fffffffffffffffL,
        };
        for (long value: positives) {
            assertTrue(value >= 0);
            BigInteger expected = BigInteger.valueOf(value);
            assertEquals(expected, NumberUtils.getUnsigned(value));
        }

        Map<Long, BigInteger> negatives = new HashMap<>();
        negatives.put(0x8000000000000000L,
                      new BigInteger("9223372036854775808"));
        negatives.put(0x8000000000000001L,
                      new BigInteger("9223372036854775809"));
        negatives.put(0x8000000000000002L,
                      new BigInteger("9223372036854775810"));
        negatives.put(0x80000000ffffffffL,
                      new BigInteger("9223372041149743103"));
        negatives.put(0x8000aaaabbbbccccL,
                      new BigInteger("9223559687125585100"));
        negatives.put(0xabcdef1234567890L,
                      new BigInteger("12379813812177893520"));
        negatives.put(0xbbccddeeff001122L,
                      new BigInteger("13532434998891647266"));
        negatives.put(0xccccdddd12345678L,
                      new BigInteger("14757414020549203576"));
        negatives.put(0xddddddddddddddddL,
                      new BigInteger("15987178197214944733"));
        negatives.put(0xe000f000a000b000L,
                      new BigInteger("16141164949970923520"));
        negatives.put(0xf000000000000000L,
                      new BigInteger("17293822569102704640"));
        negatives.put(0xffffaaaabcdef012L,
                      new BigInteger("18446650249022730258"));
        negatives.put(0xffffffffffffff00L,
                      new BigInteger("18446744073709551360"));
        negatives.put(0xfffffffffffffffaL,
                      new BigInteger("18446744073709551610"));
        negatives.put(0xfffffffffffffffbL,
                      new BigInteger("18446744073709551611"));
        negatives.put(0xfffffffffffffffcL,
                      new BigInteger("18446744073709551612"));
        negatives.put(0xfffffffffffffffdL,
                      new BigInteger("18446744073709551613"));
        negatives.put(0xfffffffffffffffeL,
                      new BigInteger("18446744073709551614"));
        negatives.put(0xffffffffffffffffL,
                      new BigInteger("18446744073709551615"));

        for (Map.Entry<Long, BigInteger> entry: negatives.entrySet()) {
            long value = entry.getKey().longValue();
            assertTrue(value < 0);
            BigInteger expected = entry.getValue();
            assertEquals(expected, NumberUtils.getUnsigned(value));
        }
    }

    /**
     * Test case for {@link NumberUtils#toByte(Number)}.
     */
    @Test
    public void testToByte() {
        byte b = Byte.MIN_VALUE;
        do {
            Byte expected = Byte.valueOf(b);
            assertEquals(expected, NumberUtils.toByte(expected));
            assertEquals(expected, NumberUtils.toByte((short)b));
            assertEquals(expected, NumberUtils.toByte((int)b));
            assertEquals(expected, NumberUtils.toByte((long)b));
            b++;
        } while (b != Byte.MIN_VALUE);

        assertEquals(null, NumberUtils.toByte((Byte)null));
        assertEquals(null, NumberUtils.toByte((Short)null));
        assertEquals(null, NumberUtils.toByte((Integer)null));
        assertEquals(null, NumberUtils.toByte((Long)null));
    }

    /**
     * Test case for {@link NumberUtils#toShort(Number)}.
     */
    @Test
    public void testToShort() {
        int[] values = {
            0, 1, 2, 3, 4, 5, 0x10, 0x123, 0x1234, 0x4567, 0x5678, 0x6789,
            0x789a, 0x7fe0, 0x7ffe, 0x7fff,
            0x8000, 0x8001, 0x8002, 0x8100, 0x8138, 0x9abc, 0xabcd, 0xbcde,
            0xcdef, 0xdef0, 0xef01, 0xffe0, 0xfffa, 0xfffc, 0xfffe, 0xffff,
        };
        for (int value: values) {
            short s = (short)(value & 0xff);
            if ((value & 0x80) != 0) {
                s |= 0xff00;
            }
            assertEquals(Short.valueOf(s), NumberUtils.toShort((byte)value));

            Short expected = Short.valueOf((short)value);
            assertEquals(expected, NumberUtils.toShort(expected));
            assertEquals(expected, NumberUtils.toShort(value));
            assertEquals(expected, NumberUtils.toShort((long)value));
        }

        assertEquals(null, NumberUtils.toShort((Byte)null));
        assertEquals(null, NumberUtils.toShort((Short)null));
        assertEquals(null, NumberUtils.toShort((Integer)null));
        assertEquals(null, NumberUtils.toShort((Long)null));
    }

    /**
     * Test case for {@link NumberUtils#toInteger(Number)}.
     */
    @Test
    public void testToInteger() {
        long[] values = {
            0L, 1L, 2L, 3L, 4L, 5L, 0x10L, 0x123L, 0x1234L, 0x4567L, 0x5678L,
            0x12345L, 0x123456L, 0x1234567L, 0x12345678L, 0x5abcdef0L,
            0x70000000L, 0x7ffffff0L, 0x7ffffffdL, 0x7ffffffeL, 0x7fffffffL,
            0x80000000L, 0x80000001L, 0x80000002L, 0x81000000L, 0x81234567L,
            0x9abcdef0L, 0xaabbccddL, 0xbbccddeeL, 0xccddeeffL, 0xddeeff11L,
            0xe0000000L, 0xfff00000L, 0xfffca000L, 0xfffff000L, 0xfffff812L,
            0xfffffff0L, 0xfffffffcL, 0xfffffffdL, 0xfffffffeL, 0xffffffffL,
        };
        for (long value: values) {
            int i = (int)(value & 0xff);
            if ((value & 0x80) != 0) {
                i |= 0xffffff00;
            }
            assertEquals(Integer.valueOf(i),
                         NumberUtils.toInteger((byte)value));

            i = (int)(value & 0xffff);
            if ((value & 0x8000) != 0) {
                i |= 0xffff0000;
            }
            assertEquals(Integer.valueOf(i),
                         NumberUtils.toInteger((short)value));

            Integer expected = Integer.valueOf((int)value);
            assertEquals(expected, NumberUtils.toInteger(expected));
            assertEquals(expected, NumberUtils.toInteger(value));
        }

        assertEquals(null, NumberUtils.toInteger((Byte)null));
        assertEquals(null, NumberUtils.toInteger((Short)null));
        assertEquals(null, NumberUtils.toInteger((Integer)null));
        assertEquals(null, NumberUtils.toInteger((Long)null));
    }

    /**
     * Test case for {@link NumberUtils#toLong(Number)}.
     */
    @Test
    public void testToLong() {
        long[] values = {
            0L, 1L, 2L, 3L, 4L, 5L, 0x10L, 0x123L, 0x1234L, 0x4567L,
            0x1fffffffL, 0x7fffffffL, 0x80000000L, 0x80000001L, 0xaaaaaaaaL,
            0xfffffff0L, 0xffffffffL, 0x100000000L, 0xabcdef0123456L,
            0xfffffffffffffffL, 0x1000000000000000L, 0x5555555555555555L,
            0x7000000000000000L, 0x7fffffffffffff00L, 0x7ffffffffffffff0L,
            0x7ffffffffffffffdL, 0x7ffffffffffffffeL, 0x7fffffffffffffffL,
            0x8000000000000000L, 0x8000000000000001L, 0x8000000000000002L,
            0x80000000ffffffffL, 0x8000aaaabbbbccccL, 0xabcdef1234567890L,
            0xbbccddeeff001122L, 0xccccdddd12345678L, 0xddddddddddddddddL,
            0xe000f000a000b000L, 0xf000000000000000L, 0xffffaaaabcdef012L,
            0xfffffffffffffffaL, 0xfffffffffffffffbL, 0xfffffffffffffffcL,
            0xfffffffffffffffdL, 0xfffffffffffffffeL, 0xffffffffffffffffL,
        };
        for (long value: values) {
            long l = (long)(value & 0xffL);
            if ((value & 0x80L) != 0) {
                l |= 0xffffffffffffff00L;
            }
            assertEquals(Long.valueOf(l), NumberUtils.toLong((byte)value));

            l = (long)(value & 0xffffL);
            if ((value & 0x8000L) != 0) {
                l |= 0xffffffffffff0000L;
            }
            assertEquals(Long.valueOf(l), NumberUtils.toLong((short)value));

            l = (long)(value & 0xffffffffL);
            if ((value & 0x80000000L) != 0) {
                l |= 0xffffffff00000000L;
            }
            assertEquals(Long.valueOf(l), NumberUtils.toLong((int)value));

            Long expected = Long.valueOf(value);
            assertEquals(expected, NumberUtils.toLong(expected));
        }

        assertEquals(null, NumberUtils.toLong((Byte)null));
        assertEquals(null, NumberUtils.toLong((Short)null));
        assertEquals(null, NumberUtils.toLong((Integer)null));
        assertEquals(null, NumberUtils.toLong((Long)null));
    }

    /**
     * Test case for {@link NumberUtils#equalsDouble(double, double)}.
     */
    @Test
    public void testEqualsDouble() {
        Random rand = new Random();
        for (int i = 0; i < 50; i++) {
            double d = rand.nextDouble();
            assertEquals(true, NumberUtils.equalsDouble(d, d));

            double[] values = {
                d + 0.0001,
                d - 0.00001,
            };
            for (double d1: values) {
                assertEquals(false, NumberUtils.equalsDouble(d, d1));
                assertEquals(false, NumberUtils.equalsDouble(d1, d));
            }
        }
    }
}
