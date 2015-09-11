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
 * JUnit test for {@link DoubleAdapter}.
 */
public class DoubleAdapterTest extends TestBase {
    /**
     * Test case for {@link DoubleAdapter#marshal(Double)}.
     */
    @Test
    public void testMarshal() {
        DoubleAdapter adapter = new DoubleAdapter();
        assertEquals(null, adapter.marshal((Double)null));

        Double[] values = {
            Double.MIN_VALUE,
            Double.MIN_VALUE + 0.1d,
            -999999999999.8765432d,
            -1234567890.12345d,
            -33333.4444d,
            -999d,
            -2d,
            -1d,
            -0.1d,
            0d,
            0.1d,
            1d,
            2d,
            3.141592d,
            555d,
            6789012.3456d,
            5555555555.6666666d,
            Double.MAX_VALUE + 0.1,
            Double.MAX_VALUE,
        };

        for (Double value: values) {
            assertEquals(value.toString(), adapter.marshal(value));
        }
    }

    /**
     * Test case for {@link DoubleAdapter#unmarshal(String)}.
     */
    @Test
    public void testUnmarshal() {
        DoubleAdapter adapter = new DoubleAdapter();
        assertEquals(null, adapter.unmarshal((String)null));

        Double[] values = {
            Double.MIN_VALUE,
            Double.MIN_VALUE + 0.1d,
            -999999999999.8765432d,
            -1234567890.12345d,
            -33333.4444d,
            -999d,
            -2d,
            -1d,
            -0.1d,
            0d,
            0.1d,
            1d,
            2d,
            3.141592d,
            555d,
            6789012.3456d,
            5555555555.6666666d,
            Double.MAX_VALUE + 0.1,
            Double.MAX_VALUE,
        };

        for (Double value: values) {
            String dec = value.toString();
            assertEquals(value, adapter.unmarshal(dec));
        }

        String[] invalid = {
            "",
            "a",
            "bad string",
            "123x45",
            "0xffff",
            "-0x12345",
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
