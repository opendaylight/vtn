/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.util.HashMap;
import java.util.Map;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link ByteUtils}.
 */
public class ByteUtilsTest extends TestBase {
    /**
     * Verify static constants.
     */
    @Test
    public void testConstants() {
        assertEquals(':', ByteUtils.HEX_SEPARATOR_CHAR);
        assertEquals(":", ByteUtils.HEX_SEPARATOR);
        assertEquals(16, ByteUtils.HEX_RADIX);
    }

    /**
     * Test case for {@link ByteUtils#toHexString(byte[])} and
     * {@link ByteUtils#toBytes(String)}.
     */
    @Test
    public void testToHexString() {
        HashMap<String, byte[]> cases = new HashMap<>();
        cases.put(null, null);
        cases.put("", new byte[0]);
        cases.put("00", new byte[]{0});
        cases.put("01", new byte[]{1});
        cases.put("10:f3", new byte[]{(byte)0x10, (byte)0xf3});
        cases.put("11:ab:cd", new byte[]{(byte)0x11, (byte)0xab, (byte)0xcd});
        cases.put("aa:bb:cc:dd:ee:ff",
                  new byte[]{(byte)0xaa, (byte)0xbb, (byte)0xcc, (byte)0xdd,
                             (byte)0xee, (byte)0xff});
        cases.put("01:23:45:67:89:ab:cd:ef:11:22:33:44:55:66:77:88",
                  new byte[]{(byte)0x01, (byte)0x23, (byte)0x45, (byte)0x67,
                             (byte)0x89, (byte)0xab, (byte)0xcd, (byte)0xef,
                             (byte)0x11, (byte)0x22, (byte)0x33, (byte)0x44,
                             (byte)0x55, (byte)0x66, (byte)0x77, (byte)0x88});

        for (Map.Entry<String, byte[]> entry: cases.entrySet()) {
            String hex = entry.getKey();
            byte[] bytes = entry.getValue();
            assertEquals(hex, ByteUtils.toHexString(bytes));
            assertArrayEquals(bytes, ByteUtils.toBytes(hex));
        }

        String[] invalid = {
            "bad value", "12345abcdefg", "aaa bbb", "fffffffff",
        };
        for (String hex: invalid) {
            try {
                ByteUtils.toBytes(hex);
                unexpected();
            } catch (NumberFormatException e) {
            }
        }

        try {
            ByteUtils.toBytes("00:11:22:33:100:ff");
            unexpected();
        } catch (NumberFormatException e) {
            assertEquals("Octet out of range: 100", e.getMessage());
        }

        try {
            ByteUtils.toBytes("00:11:-1:22:33:ff");
            unexpected();
        } catch (NumberFormatException e) {
            assertEquals("Octet out of range: -1", e.getMessage());
        }
    }

    /**
     * Test case for {@link ByteUtils#parseHexOctet(String)}.
     */
    @Test
    public void testParseHexOctet() {
        for (int i = 0; i < 256; i++) {
            String hex = String.format("%02x", i);
            assertEquals(i, ByteUtils.parseHexOctet(hex));
        }

        int[] invalid = {
            Integer.MIN_VALUE, -999999, -12345, -100, -10, -3, -2, -1,
            256, 257, 300, 44444, 1234567, Integer.MAX_VALUE,
        };
        for (int i: invalid) {
            String hex = Integer.toString(i, 16);
            try {
                ByteUtils.parseHexOctet(hex);
                unexpected();
            } catch (NumberFormatException e) {
                assertEquals("Octet out of range: " + hex, e.getMessage());
            }

            if (i < 0) {
                hex = Integer.toHexString(i);
                try {
                    ByteUtils.parseHexOctet(hex);
                    unexpected();
                } catch (NumberFormatException e) {
                }
            }
        }

        String[] invalidHex = {
            null, "", "bad value", "12345abcdefg", "fffffffff",
        };
        for (String hex: invalidHex) {
            try {
                ByteUtils.parseHexOctet(hex);
                unexpected();
            } catch (NumberFormatException e) {
            }
        }
    }

    /**
     * Test case for {@link ByteUtils#getBits(byte[], int, int)}.
     */
    @Test
    public void testGetBits() {
        byte[] data = {10, 12, 14, 20, 55, 69, 82, 97, 109, 117, 127, -50};
        byte[] bits;

        // Zero bits
        bits = ByteUtils.getBits(data, 10, 0);
        assertEquals(0, bits.length);

        // In case of simple byte copy
        bits = ByteUtils.getBits(data, 88, 8);
        assertEquals((byte)-50, bits[0]);
        assertEquals(1, bits.length);

        bits = ByteUtils.getBits(data, 8, 16);
        assertEquals((byte)12, bits[0]);
        assertEquals((byte)14, bits[1]);
        assertEquals(2, bits.length);

        bits = ByteUtils.getBits(data, 32, 32);
        assertEquals((byte)55, bits[0]);
        assertEquals((byte)69, bits[1]);
        assertEquals((byte)82, bits[2]);
        assertEquals((byte)97, bits[3]);
        assertEquals(4, bits.length);

        bits = ByteUtils.getBits(data, 16, 48);
        assertEquals((byte)14, bits[0]);
        assertEquals((byte)20, bits[1]);
        assertEquals((byte)55, bits[2]);
        assertEquals((byte)69, bits[3]);
        assertEquals((byte)82, bits[4]);
        assertEquals((byte)97, bits[5]);
        assertEquals(6, bits.length);

        // Partial bits
        bits = ByteUtils.getBits(data, 40, 7);
        assertEquals((byte)34, bits[0]);
        assertEquals(1, bits.length);

        bits = ByteUtils.getBits(data, 8, 13);
        assertEquals((byte)1, bits[0]);
        assertEquals((byte)-127, bits[1]);
        assertEquals(2, bits.length);

        bits = ByteUtils.getBits(data, 32, 28);
        assertEquals((byte)3, bits[0]);
        assertEquals((byte)116, bits[1]);
        assertEquals((byte)85, bits[2]);
        assertEquals((byte)38, bits[3]);
        assertEquals(4, bits.length);

        bits = ByteUtils.getBits(data, 16, 41);
        assertEquals((byte)0, bits[0]);
        assertEquals((byte)28, bits[1]);
        assertEquals((byte)40, bits[2]);
        assertEquals((byte)110, bits[3]);
        assertEquals((byte)-118, bits[4]);
        assertEquals((byte)-92, bits[5]);
        assertEquals(6, bits.length);

        // Unaligned
        bits = ByteUtils.getBits(data, 3, 8);
        assertEquals((byte)80, bits[0]);
        assertEquals(1, bits.length);

        bits = ByteUtils.getBits(data, 13, 16);
        assertEquals((byte)-127, bits[0]);
        assertEquals((byte)-62, bits[1]);
        assertEquals(2, bits.length);

        bits = ByteUtils.getBits(data, 5, 32);
        assertEquals((byte)65, bits[0]);
        assertEquals((byte)-127, bits[1]);
        assertEquals((byte)-62, bits[2]);
        assertEquals((byte)-122, bits[3]);
        assertEquals(4, bits.length);

        bits = ByteUtils.getBits(data, 23, 48);
        assertEquals((byte)10, bits[0]);
        assertEquals((byte)27, bits[1]);
        assertEquals((byte)-94, bits[2]);
        assertEquals((byte)-87, bits[3]);
        assertEquals((byte)48, bits[4]);
        assertEquals((byte)-74, bits[5]);
        assertEquals(6, bits.length);

        // Unaligned and partial bits
        bits = ByteUtils.getBits(data, 66, 9);
        assertEquals((byte)1, bits[0]);
        assertEquals((byte)107, bits[1]);
        assertEquals(2, bits.length);

        bits = ByteUtils.getBits(data, 13, 15);
        assertEquals((byte)64, bits[0]);
        assertEquals((byte)-31, bits[1]);
        assertEquals(2, bits.length);

        bits = ByteUtils.getBits(data, 5, 29);
        assertEquals((byte)8, bits[0]);
        assertEquals((byte)48, bits[1]);
        assertEquals((byte)56, bits[2]);
        assertEquals((byte)80, bits[3]);
        assertEquals(4, bits.length);

        bits = ByteUtils.getBits(data, 31, 43);
        assertEquals((byte)0, bits[0]);
        assertEquals((byte)-35, bits[1]);
        assertEquals((byte)21, bits[2]);
        assertEquals((byte)73, bits[3]);
        assertEquals((byte)-123, bits[4]);
        assertEquals((byte)-75, bits[5]);
        assertEquals(6, bits.length);

        bits = ByteUtils.getBits(data, 4, 12);
        assertEquals((byte)10, bits[0]);
        assertEquals((byte)12, bits[1]);
        assertEquals(2, bits.length);

        byte[] data1 = {0, 8};
        bits = ByteUtils.getBits(data1, 7, 9);
        assertEquals((byte)0, bits[0]);
        assertEquals((byte)8, bits[1]);
        assertEquals(2, bits.length);

        byte[] data2 = {2, 8};
        bits = ByteUtils.getBits(data2, 0, 7);
        assertEquals((byte)1, bits[0]);
        assertEquals(1, bits.length);

        bits = ByteUtils.getBits(data2, 7, 9);
        assertEquals((byte)0, bits[0]);
        assertEquals((byte)8, bits[1]);
        assertEquals(2, bits.length);
    }

    /**
     * Test case for {@link ByteUtils#setBits(byte[], byte[], int, int)}.
     */
    @Test
    public void testSetBits() {
        byte[] data = {
            74, 79, 14, -110, 55, -3, 82, 97,
            -63, 117, 127, -50, 127, 95, -31, 3,
        };
        byte[] original = data.clone();
        byte[] expected = data.clone();

        // Zero bits
        byte[] input = {};
        ByteUtils.setBits(data, input, 3, 0);
        assertArrayEquals(original, data);

        // In case of simple byte copy
        int index = 1;
        input = new byte[]{31};
        ByteUtils.setBits(data, input, index * 8, input.length * Byte.SIZE);
        expected[index] = input[0];
        assertArrayEquals(expected, data);

        index = 3;
        input = new byte[]{91, -116};
        ByteUtils.setBits(data, input, index * 8, input.length * Byte.SIZE);
        for (int i = 0; i < input.length; i++) {
            expected[index + i] = input[i];
        }
        assertArrayEquals(expected, data);

        index = 7;
        input = new byte[]{1, 93, -89, 73};
        ByteUtils.setBits(data, input, index * 8, input.length * Byte.SIZE);
        for (int i = 0; i < input.length; i++) {
            expected[index + i] = input[i];
        }
        assertArrayEquals(expected, data);

        index = 8;
        input = new byte[]{33, 81, 124, -47, 10, 43, 77, -93};
        ByteUtils.setBits(data, input, index * 8, input.length * Byte.SIZE);
        for (int i = 0; i < input.length; i++) {
            expected[index + i] = input[i];
        }
        assertArrayEquals(expected, data);

        // Partial bits
        data = original.clone();
        expected = original.clone();
        input = new byte[]{45};
        index = 15;
        ByteUtils.setBits(data, input, index * 8, 6);
        expected[index] = -73;
        assertArrayEquals(expected, data);

        input = new byte[]{54, 77};
        index = 2;
        ByteUtils.setBits(data, input, index * 8, 14);
        expected[index] = -39;
        expected[index + 1] = 54;
        assertArrayEquals(expected, data);

        input = new byte[]{1, -36, 25, -121};
        index = 7;
        ByteUtils.setBits(data, input, index * 8, 25);
        expected[index] = -18;
        expected[index + 1] = 12;
        expected[index + 2] = -61;
        expected[index + 3] = -1;
        assertArrayEquals(expected, data);

        input = new byte[]{5, 57, 28, -66, 118, -13, -100, -75};
        index = 6;
        ByteUtils.setBits(data, input, index * 8, 59);
        expected[index] = -89;
        expected[index + 1] = 35;
        expected[index + 2] = -105;
        expected[index + 3] = -50;
        expected[index + 4] = -34;
        expected[index + 5] = 115;
        expected[index + 6] = -106;
        expected[index + 7] = -65;
        assertArrayEquals(expected, data);

        // Unaligned
        data = original.clone();
        expected = original.clone();
        input = new byte[]{-74};
        index = 0;
        int off = 3;
        ByteUtils.setBits(data, input, index * 8 + off,
                          input.length * Byte.SIZE);
        expected[index] = 86;
        expected[index + 1] = -49;
        assertArrayEquals(expected, data);

        input = new byte[]{106, -51};
        index = 13;
        off = 7;
        ByteUtils.setBits(data, input, index * 8 + off,
                          input.length * Byte.SIZE);
        expected[index] = 94;
        expected[index + 1] = -43;
        expected[index + 2] = -101;
        assertArrayEquals(expected, data);

        input = new byte[]{-121, 18, 58, -111};
        index = 3;
        off = 1;
        ByteUtils.setBits(data, input, index * 8 + off,
                          input.length * Byte.SIZE);
        expected[index] = -61;
        expected[index + 1] = -119;
        expected[index + 2] = 29;
        expected[index + 3] = 72;
        expected[index + 4] = -31;
        assertArrayEquals(expected, data);

        input = new byte[]{-77, 26, -104, 54, 51, 21, -20, -45};
        index = 7;
        off = 6;
        ByteUtils.setBits(data, input, index * 8 + off,
                          input.length * Byte.SIZE);
        expected[index] = -30;
        expected[index + 1] = -52;
        expected[index + 2] = 106;
        expected[index + 3] = 96;
        expected[index + 4] = -40;
        expected[index + 5] = -52;
        expected[index + 6] = 87;
        expected[index + 7] = -77;
        expected[index + 8] = 79;
        assertArrayEquals(expected, data);

        // Unaligned and partial bits
        data = original.clone();
        expected = original.clone();
        input = new byte[]{9};
        index = 11;
        off = 6;
        ByteUtils.setBits(data, input, index * 8 + off, 5);
        expected[index] = -51;
        expected[index + 1] = 63;
        assertArrayEquals(expected, data);

        input = new byte[]{29, 109};
        index = 8;
        off = 1;
        ByteUtils.setBits(data, input, index * 8 + off, 14);
        expected[index] = -70;
        expected[index + 1] = -37;
        assertArrayEquals(expected, data);

        input = new byte[]{26, 53, -56, 59};
        index = 1;
        off = 3;
        ByteUtils.setBits(data, input, index * 8 + off, 29);
        expected[index] = 90;
        expected[index + 1] = 53;
        expected[index + 2] = -56;
        expected[index + 3] = 59;
        assertArrayEquals(expected, data);

        input = new byte[]{40, -37, 78, -95, 53, 23, -52, 35};
        index = 7;
        off = 7;
        ByteUtils.setBits(data, input, index * 8 + off, 63);
        expected[index] = 96;
        expected[index + 1] = -93;
        expected[index + 2] = 109;
        expected[index + 3] = 58;
        expected[index + 4] = -124;
        expected[index + 5] = -44;
        expected[index + 6] = 95;
        expected[index + 7] = 48;
        expected[index + 8] = -113;
        assertArrayEquals(expected, data);

        input = new byte[]{0, 1};
        data = new byte[]{6, 0};
        ByteUtils.setBits(data, input, 7, 9);
        assertEquals((byte)6, data[0]);
        assertEquals((byte)1, data[1]);

        input = new byte[]{1};
        ByteUtils.setBits(data, input, 0, 1);
        assertEquals((byte)-122, data[0]);
        assertEquals((byte)1, data[1]);

        ByteUtils.setBits(data, input, 3, 1);
        assertEquals((byte)-106, data[0]);
        assertEquals((byte)1, data[1]);

        input = new byte[]{0x3};
        data = new byte[]{0, -88, 0, 0};
        ByteUtils.setBits(data, input, 14, 2);
        assertEquals((byte)0, data[0]);
        assertEquals((byte)-85, data[1]);
        assertEquals((byte)0, data[2]);
        assertEquals((byte)0, data[3]);

        input = new byte[1];
        for (int nbits = 1; nbits <= 8; nbits++) {
            int mask = (1 << nbits) - 1;
            input[0] = (byte)mask;
            for (int boff = 0; boff <= 32 - nbits; boff++) {
                data = new byte[]{0, 0, 0, 0};
                ByteUtils.setBits(data, input, boff, nbits);
                int exmask = mask << (32 - nbits - boff);
                assertEquals(exmask, NumberUtils.toInteger(data));
            }
        }
    }
}
