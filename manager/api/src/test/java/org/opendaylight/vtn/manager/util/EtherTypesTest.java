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
 * JUnit test for {@link EtherTypes}.
 */
public class EtherTypesTest extends TestBase {
    /**
     * Test case for {@link EtherTypes#intValue()}.
     */
    @Test
    public void testIntValue() {
        assertEquals(0x800, EtherTypes.IPV4.intValue());
        assertEquals(0x806, EtherTypes.ARP.intValue());
        assertEquals(0x8100, EtherTypes.VLAN.intValue());
        assertEquals(0x88a8, EtherTypes.QINQ.intValue());
        assertEquals(0x88cc, EtherTypes.LLDP.intValue());
    }

    /**
     * Test case for {@link EtherTypes#shortValue()}.
     */
    @Test
    public void testShortValue() {
        assertEquals((short)0x800, EtherTypes.IPV4.shortValue());
        assertEquals((short)0x806, EtherTypes.ARP.shortValue());
        assertEquals((short)0x8100, EtherTypes.VLAN.shortValue());
        assertEquals((short)0x88a8, EtherTypes.QINQ.shortValue());
        assertEquals((short)0x88cc, EtherTypes.LLDP.shortValue());
    }
}
