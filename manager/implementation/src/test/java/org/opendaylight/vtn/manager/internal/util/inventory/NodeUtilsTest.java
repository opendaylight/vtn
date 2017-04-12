/*
 * Copyright (c) 2014, 2017 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import static org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils.equalsNodeConnectorId;

import java.math.BigInteger;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestNodeConnectorId;
import org.opendaylight.vtn.manager.internal.TestVtnPortDesc;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link NodeUtils}.
 */
public class NodeUtilsTest extends TestBase {
    /**
     * Test case for utility methods that return an exception.
     *
     * <ul>
     *   <li>{@link NodeUtils#getNullPortDescException()}</li>
     * </ul>
     */
    @Test
    public void testGetException() {
        // getNullPortDescException()
        RpcException e = NodeUtils.getNullPortDescException();
        assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
        assertEquals(null, e.getCause());
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals("vtn-port-desc cannot be null", e.getMessage());
    }

    /**
     * Test case for
     * {@link NodeUtils#checkPortLocation(SalNode, String, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckPortLocation() throws Exception {
        SalNode[] snodes = {
            new SalNode(1L),
            new SalNode(123456L),
            new SalNode(-1L),
        };
        Long[] portIds = {
            null, 1L, 2L, 12345L, 4294967040L,
        };
        String[] portNames = {
            null, "port-1", "port-2", "port-123", "a,b,c",
        };

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "Target port name or ID must be specified";
        for (SalNode snode: snodes) {
            long dpid = snode.getNodeNumber();
            for (Long pnum: portIds) {
                String id = (pnum == null) ? null : pnum.toString();
                for (String name: portNames) {
                    if (id == null && name == null) {
                        try {
                            NodeUtils.checkPortLocation(snode, id, name);
                            unexpected();
                        } catch (RpcException e) {
                            assertEquals(etag, e.getErrorTag());
                            assertEquals(vtag, e.getVtnErrorTag());
                            assertEquals(msg, e.getMessage());
                        }
                    } else {
                        SalPort expected = (pnum == null)
                            ? null
                            : new SalPort(dpid, pnum.longValue());
                        assertEquals(expected,
                                     NodeUtils.checkPortLocation(snode, id,
                                                                 name));
                    }
                }
            }
        }

        // Invalid port ID.
        String[] badPortIds = {
            "a", "bad port ID", "99999999999999999999999", "4294967041",
        };
        for (String portId: badPortIds) {
            msg = "Invalid port ID: " + portId;
            try {
                NodeUtils.checkPortLocation(new SalNode(1L), portId, "port-1");
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Empty port name.
        msg = "Port name cannot be empty";
        try {
            NodeUtils.checkPortLocation(new SalNode(1L), "1", "");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link NodeUtils#checkVtnPortDesc(VtnPortDesc)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckVtnPortDesc() throws Exception {
        String[] nodeIds = {
            "openflow:1",
            "openflow:12345",
            "openflow:18446744073709551615",
        };
        String[] portIds = {
            null, "1", "2", "12345", "4294967040",
        };
        String[] portNames = {
            null, "port-1", "port-2", "port-123", "a,b,c",
        };

        for (String nodeId: nodeIds) {
            for (String id: portIds) {
                for (String name: portNames) {
                    StringBuilder builder = new StringBuilder(nodeId).
                        append(',');
                    if (id != null) {
                        builder.append(id);
                    }
                    builder.append(',');
                    if (name != null) {
                        builder.append(name);
                    }

                    VtnPortDesc vdesc = new VtnPortDesc(builder.toString());
                    NodeUtils.checkVtnPortDesc(vdesc);
                }
            }
        }

        // Invalid node ID.
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String invalidMsg = "Invalid vtn-port-desc: ";
        String[] badNodeIds = {
            "", "a", "openflow", "openflow:1:2:3", "unknown:1",
        };
        for (String nodeId: badNodeIds) {
            String msg = "Invalid node ID: " + nodeId;
            String desc = nodeId + ",1,port-1";
            VtnPortDesc vdesc = new TestVtnPortDesc(desc);
            try {
                NodeUtils.checkVtnPortDesc(vdesc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(invalidMsg + desc + ": " + msg, e.getMessage());
            }
        }

        // Invalid port ID.
        String[] badPortIds = {
            "a", "bad port ID", "99999999999999999999999", "4294967041",
        };
        for (String portId: badPortIds) {
            String msg = "Invalid port ID: " + portId;
            String desc = "openflow:1," + portId + ",port-1";
            VtnPortDesc vdesc = new VtnPortDesc(desc);
            try {
                NodeUtils.checkVtnPortDesc(vdesc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(invalidMsg + desc + ": " + msg, e.getMessage());
            }
        }

        // Invalid format.
        VtnPortDesc[] invalidDescs = {
            new TestVtnPortDesc(""),
            new TestVtnPortDesc(","),
            new TestVtnPortDesc("openflow:1"),
            new TestVtnPortDesc("openflow:1,"),
            new TestVtnPortDesc("openflow:1,1"),
        };
        for (VtnPortDesc vd: invalidDescs) {
            try {
                NodeUtils.checkVtnPortDesc(vd);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("Invalid vtn-port-desc format: " + vd.getValue(),
                             e.getMessage());
            }
        }

        // vtn-port-desc is null.
        etag = RpcErrorTag.MISSING_ELEMENT;
        VtnPortDesc[] nullDescs = {
            null,
            new TestVtnPortDesc(),
        };
        for (VtnPortDesc vd: nullDescs) {
            try {
                NodeUtils.checkVtnPortDesc(vd);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("vtn-port-desc cannot be null", e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link NodeUtils#createPortDescArray(SalPort, VtnPort)}.
     */
    @Test
    public void testCreatePortDescArray() {
        for (long dpid = 1L; dpid <= 10L; dpid++) {
            for (long port = 1L; port <= 20L; port++) {
                SalPort sport = new SalPort(dpid, port);
                VtnPortBuilder vpbuilder = createVtnPortBuilder(sport);
                VtnPort vport = vpbuilder.build();
                String name = vport.getName();
                String base = "openflow:" + dpid + ",";
                VtnPortDesc[] expected = {
                    new VtnPortDesc(base + port + "," + name),
                    new VtnPortDesc(base + port + ","),
                    new VtnPortDesc(base + "," + name),
                    new VtnPortDesc(base + ","),
                };
                assertArrayEquals(expected,
                                  NodeUtils.createPortDescArray(sport, vport));

                // Port name is unavailable.
                vport = vpbuilder.setName(null).build();
                expected = new VtnPortDesc[] {
                    new VtnPortDesc(base + port + ","),
                    new VtnPortDesc(base + ","),
                };
                assertArrayEquals(expected,
                                  NodeUtils.createPortDescArray(sport, vport));
            }
        }
    }

    /**
     * Test case for {@link NodeUtils#createPortDesc(SalNode, String, String)}.
     */
    @Test
    public void testCreatePortDesc() {
        String[] names = {
            null, "port-1", "port-2",
        };
        String[] ids = {
            null, "1", "2",
        };

        for (long dpid = 1L; dpid <= 10L; dpid++) {
            SalNode snode = new SalNode(dpid);
            String base = snode.toString() + ",";
            for (String id: ids) {
                String i = (id == null) ? "" : id;
                for (String name: names) {
                    String n = (name == null) ? "" : name;
                    String desc = base + i + "," + n;
                    VtnPortDesc vdesc = NodeUtils.
                        createPortDesc(snode, id, name);
                    assertEquals(desc, vdesc.getValue());

                    // SalPort can be passed instead of SalNode.
                    for (long port = 1L; port <= 20L; port++) {
                        SalPort sport = new SalPort(dpid, port);
                        vdesc = NodeUtils.createPortDesc(sport, id, name);
                        assertEquals(desc, vdesc.getValue());
                    }
                }
            }
        }
    }

    /**
     * Test case for
     * {@link NodeUtils#equalsNodeConnectorId(SalNode, NodeConnectorId, NodeConnectorId)}.
     */
    @Test
    public void testEqualsNodeConnectorId() {
        BigInteger[] nodeIds = {
            BigInteger.ONE,
            BigInteger.valueOf(2L),
            BigInteger.valueOf(7777777L),
            BigInteger.valueOf(0x123456789L),
            new BigInteger("18446744073709551615"),
        };
        SalNode anotherNode = new SalNode(999L);

        long[] portIds = {
            1L, 2L, 345L, 6789L, 0xabcdefL, 0xffffff00L,
        };

        SalNode nullNode = null;
        NodeConnectorId nullNcId = null;
        NodeConnectorId empty1 = new TestNodeConnectorId();
        NodeConnectorId empty2 = new TestNodeConnectorId();

        NodeConnectorId[] badIds = {
            null, empty1,

            // Invalid node type.
            new NodeConnectorId(""),
            new NodeConnectorId("proto1:1:3"),
            new NodeConnectorId("of:2:1"),
            new NodeConnectorId("openflow1:1:4"),

            // Too large DPID.
            new NodeConnectorId("openflow:18446744073709551616:3"),
            new NodeConnectorId("openflow:18446744073709551617:2"),
            new NodeConnectorId("openflow:99999999999999999999999:2"),
            new NodeConnectorId("openflow:999999999999999999999999:LOCAL"),

            // Negative DPID.
            new NodeConnectorId("openflow:-1:1"),
            new NodeConnectorId("openflow:-12345678:2"),
            new NodeConnectorId("openflow:-3333333333333333333333333:2"),

            // Invalid DPID.
            new NodeConnectorId("openflow:0x12345678:1"),
            new NodeConnectorId("openflow:1234abcd:1"),
            new NodeConnectorId("openflow:Bad DPID:1"),

            // Invalid port number.
            new NodeConnectorId("4294967041"),
            new NodeConnectorId("4294967042"),
            new NodeConnectorId("4294967295"),
            new NodeConnectorId("9999999999999999999999999999"),
            new NodeConnectorId("-1"),
            new NodeConnectorId("-2"),
            new NodeConnectorId("-1000000"),
            new NodeConnectorId("abcde"),
            new NodeConnectorId("bad port ID"),
        };

        for (BigInteger nodeId: nodeIds) {
            SalNode snode = new SalNode(nodeId.longValue());
            SalNode[] snodes = {nullNode, snode, anotherNode};
            for (long portId1: portIds) {
                for (long portId2: portIds) {
                    // In case where both node-connector-ids are
                    // fully-qualified.
                    String str1 = String.format("openflow:%s:%d",
                                                nodeId, portId1);
                    String str2 = String.format("openflow:%s:%d",
                                                nodeId, portId2);
                    NodeConnectorId id1 = new NodeConnectorId(str1);
                    NodeConnectorId id2 = new NodeConnectorId(str2);
                    boolean match = (portId1 == portId2);
                    for (SalNode sn: snodes) {
                        assertEquals(match,
                                     equalsNodeConnectorId(sn, id1, id2));
                    }

                    // In case where only one node-connector-id is
                    // fully-qualified.
                    str2 = Long.toString(portId2);
                    id2 = new NodeConnectorId(str2);
                    for (SalNode sn: snodes) {
                        boolean expected = (match && snode.equals(sn));
                        assertEquals(expected,
                                     equalsNodeConnectorId(sn, id1, id2));
                        assertEquals(expected,
                                     equalsNodeConnectorId(sn, id2, id1));
                    }

                    // Should return if one node-connector-id is invalid.
                    NodeConnectorId[] ids = {id1, id2};
                    for (SalNode sn: snodes) {
                        for (NodeConnectorId id: ids) {
                            for (NodeConnectorId bad: badIds) {
                                assertFalse(equalsNodeConnectorId(sn, id, bad));
                                assertFalse(equalsNodeConnectorId(sn, bad, id));
                            }
                        }
                    }

                    // In case where both node-connector-ids are specified by
                    // port number.
                    str1 = Long.toString(portId1);
                    id1 = new NodeConnectorId(str1);
                    if (match) {
                        assertEquals(id2, id1);
                    } else {
                        assertNotEquals(id2, id1);
                    }
                    for (SalNode sn: snodes) {
                        assertEquals(match,
                                     equalsNodeConnectorId(sn, id1, id2));
                    }
                }
            }

            for (SalNode sn: snodes) {
                // Should return true if both node-connector-ids are null.
                assertTrue(equalsNodeConnectorId(sn, nullNcId, nullNcId));
                assertFalse(equalsNodeConnectorId(sn, empty1, nullNcId));
                assertFalse(equalsNodeConnectorId(sn, nullNcId, empty1));

                // Should return true if both node-connector-ids are empty.
                assertTrue(equalsNodeConnectorId(sn, empty1, empty2));
            }
        }
    }
}
