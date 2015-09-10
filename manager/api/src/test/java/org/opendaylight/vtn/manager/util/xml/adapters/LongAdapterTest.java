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
 * JUnit test for {@link LongAdapter}.
 */
public class LongAdapterTest extends TestBase {
    /**
     * Test case for {@link LongAdapter#marshal(Long)}.
     */
    @Test
    public void testMarshal() {
        LongAdapter adapter = new LongAdapter();
        assertEquals(null, adapter.marshal((Long)null));

        Long[] values = {
            Long.MIN_VALUE,
            Long.MIN_VALUE + 1,
            -123456789012L,
            -999999999999L,
            -88888888888L,
            -7777777777L,
            -666666666L,
            -5555555L,
            -444444L,
            -33333L,
            -2222L,
            -100L,
            -3L,
            -2L,
            -1L,
            0L,
            1L,
            2L,
            3L,
            111L,
            2345L,
            34567L,
            456789L,
            5678901L,
            67890123L,
            789012345L,
            8901234567L,
            90123456789L,
            Long.MAX_VALUE - 1,
            Long.MAX_VALUE,
        };

        for (Long value: values) {
            assertEquals(value.toString(), adapter.marshal(value));
        }
    }

    /**
     * Test case for {@link LongAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        LongAdapter adapter = new LongAdapter();
        assertEquals(null, adapter.unmarshal((String)null));

        Long[] values = {
            Long.MIN_VALUE,
            Long.MIN_VALUE + 1,
            -123456789012L,
            -999999999999L,
            -88888888888L,
            -7777777777L,
            -666666666L,
            -5555555L,
            -444444L,
            -33333L,
            -2222L,
            -100L,
            -3L,
            -2L,
            -1L,
            0L,
            1L,
            2L,
            3L,
            111L,
            2345L,
            34567L,
            456789L,
            5678901L,
            67890123L,
            789012345L,
            8901234567L,
            90123456789L,
            Long.MAX_VALUE - 1,
            Long.MAX_VALUE,
        };

        for (Long value: values) {
            long v = value.longValue();

            // Decimal
            String dec = value.toString();
            assertEquals(value, adapter.unmarshal(dec));

            String hex;
            String oct;
            if (v < 0) {
                long l = v * -1;
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
            "9223372036854775808",
            "9223372036854775809",
            "12345678901234567890",
            "-9223372036854775809",
            "-9223372036854775810",
            "-9999999999999999999",
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
