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
 * JUnit test for {@link ShortAdapter}.
 */
public class ShortAdapterTest extends TestBase {
    /**
     * Test case for {@link ShortAdapter#marshal(Short)}.
     */
    @Test
    public void testMarshal() {
        ShortAdapter adapter = new ShortAdapter();
        assertEquals(null, adapter.marshal((Short)null));

        Short[] values = {
            Short.MIN_VALUE,
            Short.MIN_VALUE + 1,
            -12345,
            -100,
            -3,
            -2,
            -1,
            0,
            1,
            2,
            3,
            99,
            333,
            12345,
            23456,
            Short.MAX_VALUE - 1,
            Short.MAX_VALUE,
        };

        for (Short value: values) {
            assertEquals(value.toString(), adapter.marshal(value));
        }
    }

    /**
     * Test case for {@link ShortAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        ShortAdapter adapter = new ShortAdapter();
        assertEquals(null, adapter.unmarshal((String)null));

        Short[] values = {
            Short.MIN_VALUE,
            Short.MIN_VALUE + 1,
            -12345,
            -100,
            -3,
            -2,
            -1,
            0,
            1,
            2,
            3,
            101,
            789,
            12345,
            30000,
            Short.MAX_VALUE - 1,
            Short.MAX_VALUE,
        };

        for (Short value: values) {
            short v = value.shortValue();

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
        }

        String[] invalid = {
            // Invalid format.
            "",
            "a",
            "bad string",
            "123x45",

            // value out of range
            "32768",
            "32769",
            "99999999",
            "-32769",
            "-32770",
            "-1234567",
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
