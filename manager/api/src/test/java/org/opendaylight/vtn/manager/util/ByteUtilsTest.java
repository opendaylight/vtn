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
}
