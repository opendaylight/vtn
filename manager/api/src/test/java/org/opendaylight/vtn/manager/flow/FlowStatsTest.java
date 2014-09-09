/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.flow;

import java.util.HashSet;
import org.junit.Test;

public class FlowStatsTest extends TestBase {
    long packets = 0;
    long bytes = 0;
    long duration = 0;

    @Test
    public void testGetterMethod() {
        for (int i = 0; i < 3; i++) {
            packets = (long)i + 1;
            bytes = (long)i * 2;
            duration = (long)i * 60;
            FlowStats flowStats = new FlowStats(packets, bytes, duration);
            assertEquals(bytes, flowStats.getByteCount());
            assertEquals(packets, flowStats.getPacketCount());
            assertEquals(duration, flowStats.getDuration());
            FlowStats flowStatsobj = new FlowStats(-1, 0, -1);
            assertEquals(0, flowStatsobj.getByteCount());
            assertEquals(-1, flowStatsobj.getPacketCount());
            assertEquals(-1, flowStatsobj.getDuration());
        }
    }

    /**
     * Test case for {@link FlowStatslist#equals(Object)}
     *
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        packets = (long)1 + 1;
        bytes = (long)4 * 2;
        duration = (long)8 * 60;
        FlowStats flowStats = new FlowStats(packets, bytes, duration);
        toTestEquals(set, flowStats, new FlowStats(packets, bytes, duration));
    }
    /**
     * Test case for {@link FlowStatslist#toString(Object)}
     *
     */
    @Test
    public void testToString() {
        HashSet<Object> set = new HashSet<Object>();
        packets = (long)1 + 6;
        bytes = (long)4 * 5;
        duration = (long)3 * 60;
        FlowStats flowStats = new FlowStats(packets, bytes, duration);
        String flowStatStr = "FlowStats[packets=" + packets + ",bytes=" + bytes
               + ",duration=" + duration + "]]";
        assertEquals(flowStatStr, flowStats.toString());
    }
}
