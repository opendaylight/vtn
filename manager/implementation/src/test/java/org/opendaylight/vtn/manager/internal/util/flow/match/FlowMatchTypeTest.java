/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.Set;
import java.util.EnumSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * Test case for {@link FlowMatchType}.
 */
public class FlowMatchTypeTest extends TestBase {
    /**
     * Verify values.
     */
    @Test
    public void testValues() {
        int i = 0;
        assertEquals(i++, FlowMatchType.IN_PORT.ordinal());
        assertEquals(i++, FlowMatchType.DL_SRC.ordinal());
        assertEquals(i++, FlowMatchType.DL_DST.ordinal());
        assertEquals(i++, FlowMatchType.DL_TYPE.ordinal());
        assertEquals(i++, FlowMatchType.DL_VLAN.ordinal());
        assertEquals(i++, FlowMatchType.DL_VLAN_PCP.ordinal());
        assertEquals(i++, FlowMatchType.IP_SRC.ordinal());
        assertEquals(i++, FlowMatchType.IP_DST.ordinal());
        assertEquals(i++, FlowMatchType.IP_PROTO.ordinal());
        assertEquals(i++, FlowMatchType.IP_DSCP.ordinal());
        assertEquals(i++, FlowMatchType.TCP_SRC.ordinal());
        assertEquals(i++, FlowMatchType.TCP_DST.ordinal());
        assertEquals(i++, FlowMatchType.UDP_SRC.ordinal());
        assertEquals(i++, FlowMatchType.UDP_DST.ordinal());
        assertEquals(i++, FlowMatchType.ICMP_TYPE.ordinal());
        assertEquals(i++, FlowMatchType.ICMP_CODE.ordinal());
        assertEquals(i, FlowMatchType.values().length);
    }

    /**
     * Test case for {@link FlowMatchType#addUnicastTypes(Set)}.
     */
    @Test
    public void testAddUnicastTypes() {
        Set<FlowMatchType> set = EnumSet.noneOf(FlowMatchType.class);
        FlowMatchType.addUnicastTypes(set);
        assertEquals(2, set.size());
        assertTrue(set.contains(FlowMatchType.DL_SRC));
        assertTrue(set.contains(FlowMatchType.DL_DST));
    }

    /**
     * Test case for {@link FlowMatchType#getUnicastTypeCount(Set)}.
     */
    @Test
    public void testGetUnicastTypeCount() {
        Set<FlowMatchType> set = EnumSet.noneOf(FlowMatchType.class);
        assertEquals(0, FlowMatchType.getUnicastTypeCount(set));

        for (FlowMatchType type: FlowMatchType.values()) {
            int count;
            switch (type) {
            case DL_SRC:
            case DL_DST:
                count = 1;
                break;

            default:
                count = 0;
                break;
            }

            set = EnumSet.of(type);
            assertEquals(count, FlowMatchType.getUnicastTypeCount(set));
        }

        set = EnumSet.of(FlowMatchType.DL_SRC, FlowMatchType.DL_DST);
        assertEquals(2, FlowMatchType.getUnicastTypeCount(set));
        set = EnumSet.allOf(FlowMatchType.class);
        assertEquals(2, FlowMatchType.getUnicastTypeCount(set));
    }
}
