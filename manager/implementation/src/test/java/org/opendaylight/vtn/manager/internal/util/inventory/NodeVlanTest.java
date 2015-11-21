/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
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
 * JUnit test for {@link NodeVlan}.
 */
public class NodeVlanTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        SalNode[] snodes = {
            null,
            new SalNode(1L),
            new SalNode(12345L),
            new SalNode(-1L),
        };
        int[] vids = {
            0, 1, 300, 555, 1234, 4095,
        };

        for (SalNode snode: snodes) {
            for (int vid: vids) {
                NodeVlan nv = new NodeVlan(snode, vid);
                assertEquals(snode, nv.getNode());
                assertEquals(vid, nv.getVlanId());
            }
        }
    }

    /**
     * Test case for {@link NodeVlan#NodeVlan(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructorString() throws Exception {
        SalNode[] snodes = {
            null,
            new SalNode(1L),
            new SalNode(12345L),
            new SalNode(9999999999L),
            new SalNode(-1L),
        };
        int[] vids = {
            0, 1, 300, 555, 1234, 4095,
        };

        for (SalNode snode: snodes) {
            for (int vid: vids) {
                String nodeId = (snode == null) ? "" : snode.toString();
                String value = nodeId + "@" + vid;
                NodeVlan nv = new NodeVlan(value);
                assertEquals(snode, nv.getNode());
                assertEquals(vid, nv.getVlanId());

                // The given value should be cached.
                assertSame(value, nv.toString());
            }
        }

        // Null argument.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "node-vlan cannot be null";
        try {
            new NodeVlan((String)null);
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
            emsg = "Invalid VLAN ID in node-vlan: " + value;
            try {
                new NodeVlan(value);
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

        // Invalid node ID.
        String[] badNodeIds = {
            "a",
            "openflow",
            "openflow:",
            "openflow:1:2",
            "openflow1:2",
            "unknown:1",
        };
        for (String nodeId: badNodeIds) {
            String value = nodeId + "@0";
            emsg = "Invalid node-id in node-vlan: " + value;
            try {
                new NodeVlan(value);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link NodeVlan#equals(Object)} and
     * {@link NodeVlan#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();
        SalNode[] snodes = {
            null,
            new SalNode(1L),
            new SalNode(12345L),
            new SalNode(9999999999L),
            new SalNode(-1L),
        };
        int[] vids = {
            0, 1, 300, 555, 1234, 4095,
        };

        for (SalNode snode: snodes) {
            for (int vid: vids) {
                NodeVlan nv1 = new NodeVlan(snode, vid);
                NodeVlan nv2 = new NodeVlan(snode, vid);
                testEquals(set, nv1, nv2);
            }
        }

        int required = snodes.length * vids.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link NodeVlan#toString()}.
     */
    @Test
    public void testToString() {
        SalNode[] snodes = {
            null,
            new SalNode(1L),
            new SalNode(12345L),
            new SalNode(9999999999L),
            new SalNode(-1L),
        };
        int[] vids = {
            0, 1, 300, 555, 1234, 3333, 4095,
        };

        for (SalNode snode: snodes) {
            for (int vid: vids) {
                NodeVlan nv = new NodeVlan(snode, vid);
                String n = (snode == null) ? "" : snode.toString();
                String expected = n + "@" + vid;
                String value = nv.toString();
                assertEquals(expected, value);

                // Result should be cached.
                assertSame(value, nv.toString());
            }
        }
    }
}
