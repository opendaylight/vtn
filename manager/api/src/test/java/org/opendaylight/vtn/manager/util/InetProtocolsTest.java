/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link InetProtocols}.
 */
public class InetProtocolsTest extends TestBase {
    /**
     * Test case for {@link InetProtocols#intValue()}.
     */
    @Test
    public void testIntValue() {
        assertEquals(1, InetProtocols.ICMP.intValue());
        assertEquals(6, InetProtocols.TCP.intValue());
        assertEquals(17, InetProtocols.UDP.intValue());
    }

    /**
     * Test case for {@link InetProtocols#shortValue()}.
     */
    @Test
    public void testShortValue() {
        assertEquals((short)1, InetProtocols.ICMP.shortValue());
        assertEquals((short)6, InetProtocols.TCP.shortValue());
        assertEquals((short)17, InetProtocols.UDP.shortValue());
    }

    /**
     * Test case for {@link InetProtocols#byteValue()}.
     */
    @Test
    public void testByteValue() {
        assertEquals((byte)1, InetProtocols.ICMP.byteValue());
        assertEquals((byte)6, InetProtocols.TCP.byteValue());
        assertEquals((byte)17, InetProtocols.UDP.byteValue());
    }

    /**
     * Test case for {@link InetProtocols#forValue(byte)}.
     */
    @Test
    public void testForValue() {
        for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
            byte value = (byte)i;
            InetProtocols proto = InetProtocols.forValue(value);
            if (i == 1) {
                assertEquals(InetProtocols.ICMP, proto);
            } else if (i == 6) {
                assertEquals(InetProtocols.TCP, proto);
            } else if (i == 17) {
                assertEquals(InetProtocols.UDP, proto);
            } else {
                assertEquals(null, proto);
            }
        }
    }
}
