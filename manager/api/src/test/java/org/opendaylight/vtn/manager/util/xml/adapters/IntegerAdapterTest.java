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
 * JUnit test for {@link IntegerAdapter}.
 */
public class IntegerAdapterTest extends TestBase {
    /**
     * Test case for {@link IntegerAdapter#marshal(Integer)}.
     */
    @Test
    public void testMarshal() {
        IntegerAdapter adapter = new IntegerAdapter();
        assertEquals(null, adapter.marshal((Integer)null));

        Integer[] values = {
            Integer.MIN_VALUE,
            Integer.MIN_VALUE + 1,
            -12345678,
            -999999,
            -8888,
            -777,
            -66,
            -10,
            -3,
            -2,
            -1,
            0,
            1,
            2,
            3,
            11,
            222,
            3333,
            44444,
            555555,
            6666666,
            Integer.MAX_VALUE - 1,
            Integer.MAX_VALUE,
        };

        for (Integer value: values) {
            assertEquals(value.toString(), adapter.marshal(value));
        }
    }

    /**
     * Test case for {@link IntegerAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        IntegerAdapter adapter = new IntegerAdapter();
        assertEquals(null, adapter.unmarshal((String)null));

        Integer[] values = {
            Integer.MIN_VALUE,
            Integer.MIN_VALUE + 1,
            -12345678,
            -999999,
            -8888,
            -777,
            -66,
            -10,
            -3,
            -2,
            -1,
            0,
            1,
            2,
            3,
            11,
            222,
            3333,
            44444,
            555555,
            6666666,
            Integer.MAX_VALUE - 1,
            Integer.MAX_VALUE,
        };

        for (Integer value: values) {
            int v = value.intValue();

            // Decimal
            String dec = value.toString();
            assertEquals(value, adapter.unmarshal(dec));

            String hex;
            String oct;
            if (v < 0) {
                long l = (long)v * -1;
                hex = String.format("-0x%x", l);
                oct = String.format("-0%o", l);
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
            "2147483648",
            "2147483649",
            "12345678901",
            "-2147483649",
            "-2147483650",
            "-3333333333",
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
