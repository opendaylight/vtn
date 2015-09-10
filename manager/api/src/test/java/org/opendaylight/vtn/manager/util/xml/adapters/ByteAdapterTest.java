/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util.xml.adapters;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link ByteAdapter}.
 */
public class ByteAdapterTest extends TestBase {
    /**
     * Test case for {@link ByteAdapter#marshal(Byte)}.
     */
    @Test
    public void testMarshal() {
        ByteAdapter adapter = new ByteAdapter();
        assertEquals(null, adapter.marshal((Byte)null));

        byte v = Byte.MIN_VALUE;
        do {
            assertEquals(Byte.toString(v), adapter.marshal(v));
            v++;
        } while (v != Byte.MIN_VALUE);
    }

    /**
     * Test case for {@link ByteAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        ByteAdapter adapter = new ByteAdapter();
        assertEquals(null, adapter.unmarshal((String)null));

        byte v = Byte.MIN_VALUE;
        do {
            Byte value = Byte.valueOf(v);

            // Decimal
            String dec = value.toString();
            assertEquals(value, adapter.unmarshal(dec));

            String hex;
            String oct;
            if (v < 0) {
                int i = (int)v * -1;
                hex = String.format("-0x%x", i);
                oct = String.format("-0%o", i);
            } else {
                hex = String.format("0x%x", v);
                oct = String.format("0%o", v);
            }

            // Hexadecimal
            assertEquals(value, adapter.unmarshal(hex));

            // Octal
            assertEquals(value, adapter.unmarshal(oct));

            v++;
        } while (v != Byte.MIN_VALUE);

        String[] invalid = {
            // Invalid format.
            "",
            "a",
            "bad string",
            "123x45",

            // value out of range
            "128",
            "129",
            "99999999",
            "-129",
            "-130",
            "-12345",
            "10000000000000000000000000",
            "-10000000000000000000000000",
        };

        for (String str: invalid) {
            try {
                adapter.unmarshal(str);
                unexpected();
            } catch (NumberFormatException e) {
            }
        }
    }
}
