/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.HashSet;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link PortVlan}.
 */
public class PortVlanTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        SalPort[] sports = {
            new SalPort(1L, 1L),
            new SalPort(1L, 2L),
            new SalPort(12345L, 678L),
            new SalPort(-1L, 0xffffff00L),
        };
        int[] vids = {
            0, 1, 300, 555, 1234, 4095,
        };

        for (SalPort sport: sports) {
            for (int vid: vids) {
                PortVlan pv = new PortVlan(sport, vid);
                assertEquals(sport, pv.getPort());
                assertEquals(vid, pv.getVlanId());
            }
        }
    }

    /**
     * Test case for {@link PortVlan#PortVlan(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructorString() throws Exception {
        SalPort[] sports = {
            new SalPort(1L, 1L),
            new SalPort(1L, 2L),
            new SalPort(12345L, 678L),
            new SalPort(-1L, 0xffffff00L),
        };
        int[] vids = {
            0, 1, 300, 555, 1234, 4095,
        };

        for (SalPort sport: sports) {
            for (int vid: vids) {
                String value = sport.toString() + "@" + vid;
                PortVlan pv = new PortVlan(value);
                assertEquals(sport, pv.getPort());
                assertEquals(vid, pv.getVlanId());

                // The given value should be cached.
                assertSame(value, pv.toString());
            }
        }

        // Null argument.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "port-vlan cannot be null";
        try {
            new PortVlan((String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Invalid VLAN ID.
        etag = RpcErrorTag.BAD_ELEMENT;
        long[] badVids = {
            4096, 4097, 10000, 30000, 12345678, Integer.MAX_VALUE,
            0x80000000L, 0x1000000000L, Long.MAX_VALUE,
        };
        for (long vid: badVids) {
            String value = "openflow:1@" + vid;
            emsg = "Invalid VLAN ID in port-vlan: " + value;
            try {
                new PortVlan(value);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());

                Throwable cause = e.getCause();
                if (vid <= Integer.MAX_VALUE) {
                    assertTrue(cause instanceof RpcException);
                    RpcException re = (RpcException)cause;
                    assertEquals(etag, re.getErrorTag());
                    assertEquals(vtag, re.getVtnErrorTag());
                    assertEquals("Invalid VLAN ID: " + vid, re.getMessage());
                } else {
                    assertTrue(cause instanceof NumberFormatException);
                }
            }
        }

        // Invalid node connector ID.
        String[] badPortIds = {
            "",
            "a",
            "openflow",
            "openflow:",
            "openflow:1",
            "openflow:1:2:3",
            "openflow1:2",
            "unknown:1",
        };
        for (String portId: badPortIds) {
            String value = portId + "@0";
            emsg = "Invalid node-connector-id in port-vlan: " + value;
            try {
                new PortVlan(value);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link PortVlan#equals(Object)} and
     * {@link PortVlan#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();
        SalPort[] sports = {
            new SalPort(1L, 1L),
            new SalPort(1L, 2L),
            new SalPort(12345L, 678L),
            new SalPort(-1L, 0xffffff00L),
        };
        int[] vids = {
            0, 1, 300, 555, 1234, 4095,
        };

        for (SalPort sport: sports) {
            for (int vid: vids) {
                PortVlan pv1 = new PortVlan(sport, vid);
                PortVlan pv2 = new PortVlan(sport, vid);
                testEquals(set, pv1, pv2);
            }
        }

        int required = sports.length * vids.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link PortVlan#toString()}.
     */
    @Test
    public void testToString() {
        SalPort[] sports = {
            new SalPort(1L, 1L),
            new SalPort(1L, 2L),
            new SalPort(12345L, 678L),
            new SalPort(-1L, 0xffffff00L),
        };
        int[] vids = {
            0, 1, 300, 555, 1234, 4095,
        };

        for (SalPort sport: sports) {
            for (int vid: vids) {
                PortVlan pv = new PortVlan(sport, vid);
                String expected = sport.toString() + "@" + vid;
                String value = pv.toString();
                assertEquals(expected, value);

                // Result should be cached.
                assertSame(value, pv.toString());
            }
        }
    }
}
