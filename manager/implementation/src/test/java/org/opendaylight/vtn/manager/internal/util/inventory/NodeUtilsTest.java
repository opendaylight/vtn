/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

import org.junit.Test;

import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node.NodeIDType;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import static org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils.checkNodeType;
import static org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils.checkNodeConnectorType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalIngressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataIngressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortLocation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * JUnit test for {@link NodeUtils}.
 */
public class NodeUtilsTest extends TestBase {
    /**
     * Test case for {@link NodeUtils#checkNodeType(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckNodeType() throws Exception {
        checkNodeType(NodeIDType.OPENFLOW);

        String[] invalidTypes = {
            NodeIDType.PCEP,
            NodeIDType.ONEPK,
            NodeIDType.PRODUCTION
        };
        for (String type: invalidTypes) {
            checkInvalidNodeType(type);
        }
    }

    /**
     * Test case for {@link NodeUtils#checkNodeConnectorType(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckNodeConnectorType() throws Exception {
        checkNodeConnectorType(NodeConnectorIDType.OPENFLOW);

        String[] invalidTypes = {
            NodeConnectorIDType.PCEP,
            NodeConnectorIDType.ONEPK,
            NodeConnectorIDType.OPENFLOW2PCEP,
            NodeConnectorIDType.PCEP2OPENFLOW,
            NodeConnectorIDType.PCEP2ONEPK,
            NodeConnectorIDType.ONEPK2OPENFLOW,
            NodeConnectorIDType.ONEPK2PCEP,
            NodeConnectorIDType.PRODUCTION,
        };
        for (String type: invalidTypes) {
            checkInvalidNodeConnectorType(type);
        }
    }

    /**
     * Test case for {@link NodeUtils#checkPortLocation(PortLocation)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckPortLocation() throws Exception {
        try {
            NodeUtils.checkPortLocation(null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Port location cannot be null", st.getDescription());
        }

        Node[] invalidNodes = {
            null,
            createNode(NodeIDType.ONEPK, 1L),
            createNode(NodeIDType.PRODUCTION, 2L),
            createNode(NodeIDType.PCEP, 3L),
        };
        for (Node node: invalidNodes) {
            PortLocation ploc = new PortLocation(node, null);
            try {
                NodeUtils.checkPortLocation(ploc);
                unexpected();
            } catch (RpcException e) {
                RpcErrorTag etag = (node == null)
                    ? RpcErrorTag.MISSING_ELEMENT
                    : RpcErrorTag.BAD_ELEMENT;
                assertEquals(etag, e.getErrorTag());
                assertEquals(null, e.getCause());
                Status status = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, status.getCode());
                String msg = (node == null)
                    ? "Node cannot be null"
                    : "Unsupported node: type=" + node.getType();
                assertEquals(msg, status.getDescription());
            }
        }

        Node node = createNode(NodeIDType.OPENFLOW, 1L);
        Map<SwitchPort, String> invalidSwPorts = new HashMap<>();
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.OPENFLOW, null),
                           "Port type must be specified with port ID");
        invalidSwPorts.put(new SwitchPort((String)null, "1"),
                           "Port ID must be specified with port type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.ONEPK, "port-1"),
                           "Unsupported node connector type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.PCEP, "port-1"),
                           "Unsupported node connector type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.PRODUCTION,
                                          "port-2"),
                           "Unsupported node connector type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                          "port-3"),
                           "Broken node connector is specified");
        invalidSwPorts.put(new SwitchPort(null),
                           "Switch port cannot be empty");
        invalidSwPorts.put(new SwitchPort(""), "Port name cannot be empty");
        for (Map.Entry<SwitchPort, String> entry: invalidSwPorts.entrySet()) {
            SwitchPort swport = entry.getKey();
            String msg = entry.getValue();
            PortLocation ploc = new PortLocation(node, swport);
            try {
                NodeUtils.checkPortLocation(ploc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals(msg, st.getDescription());
            }
        }

        PortLocation[] plocs = {
            new PortLocation(node, null),
            new PortLocation(node, new SwitchPort("port-1")),
            new PortLocation(node, new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                                  "10")),
            new PortLocation(node, new SwitchPort("port-20",
                                                  NodeConnectorIDType.OPENFLOW,
                                                  "20")),
        };

        for (PortLocation ploc: plocs) {
            NodeUtils.checkPortLocation(ploc);
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
     * Test case for {@link NodeUtils#toVtnPortDesc(PortLocation)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToVtnPortDesc() throws Exception {
        try {
            NodeUtils.toVtnPortDesc(null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Port location cannot be null", st.getDescription());
        }

        Node[] invalidNodes = {
            null,
            createNode(NodeIDType.ONEPK, 1L),
            createNode(NodeIDType.PRODUCTION, 2L),
            createNode(NodeIDType.PCEP, 3L),
        };

        for (Node node: invalidNodes) {
            PortLocation ploc = new PortLocation(node, null);
            try {
                NodeUtils.toVtnPortDesc(ploc);
                unexpected();
            } catch (RpcException e) {
                RpcErrorTag etag = (node == null)
                    ? RpcErrorTag.MISSING_ELEMENT
                    : RpcErrorTag.BAD_ELEMENT;
                assertEquals(etag, e.getErrorTag());
                assertEquals(null, e.getCause());
                Status status = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, status.getCode());
                String msg = (node == null)
                    ? "Node cannot be null"
                    : "Unsupported node: type=" + node.getType();
                assertEquals(msg, status.getDescription());
            }
        }

        SalNode snode = new SalNode(1L);
        Node node = snode.getAdNode();
        Map<SwitchPort, String> invalidSwPorts = new HashMap<>();
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.OPENFLOW, null),
                           "Port type must be specified with port ID");
        invalidSwPorts.put(new SwitchPort((String)null, "1"),
                           "Port ID must be specified with port type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.ONEPK, "port-1"),
                           "Unsupported node connector type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.PRODUCTION,
                                          "port-2"),
                           "Unsupported node connector type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                          "port-3"),
                           "Broken node connector is specified");
        invalidSwPorts.put(new SwitchPort(null),
                           "Switch port cannot be empty");
        invalidSwPorts.put(new SwitchPort(""), "Port name cannot be empty");
        for (Map.Entry<SwitchPort, String> entry: invalidSwPorts.entrySet()) {
            SwitchPort swport = entry.getKey();
            String msg = entry.getValue();
            PortLocation ploc = new PortLocation(node, swport);
            try {
                NodeUtils.toVtnPortDesc(ploc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals(msg, st.getDescription());
            }
        }

        String[] names = {
            "port-1", "port-2",
        };
        String[] ids = {
            "1", "2",
        };
        String type = NodeConnectorIDType.OPENFLOW;

        for (long dpid = 1L; dpid <= 10L; dpid++) {
            snode = new SalNode(dpid);
            node = snode.getAdNode();
            PortLocation ploc = new PortLocation(node, null);
            String desc = "openflow:" + dpid + ",,";
            VtnPortDesc vdesc = NodeUtils.toVtnPortDesc(ploc);
            assertEquals(desc, vdesc.getValue());

            for (String id: ids) {
                SwitchPort swport = new SwitchPort(type, id);
                ploc = new PortLocation(node, swport);
                desc = "openflow:" + dpid + "," + id + ",";
                vdesc = NodeUtils.toVtnPortDesc(ploc);
                assertEquals(desc, vdesc.getValue());
            }

            for (String name: names) {
                SwitchPort swport = new SwitchPort(name);
                ploc = new PortLocation(node, swport);
                desc = "openflow:" + dpid + ",," + name;
                vdesc = NodeUtils.toVtnPortDesc(ploc);
                assertEquals(desc, vdesc.getValue());

                for (String id: ids) {
                    swport = new SwitchPort(name, type, id);
                    ploc = new PortLocation(node, swport);
                    desc = "openflow:" + dpid + "," + id + "," + name;
                    vdesc = NodeUtils.toVtnPortDesc(ploc);
                    assertEquals(desc, vdesc.getValue());
                }
            }
        }
    }

    /**
     * Test case for {@link NodeUtils#toPortLocation(VtnPortDesc)}.
     */
    @Test
    public void testToPortLocation1() {
        assertEquals(null, NodeUtils.toPortLocation((VtnPortDesc)null));

        String[] invalid = {
            "unknown,,",
            "unknown,1,",
            "unknown,2,port-2",
            "openflow1:,,",
            "openflow1:,1,",
            "openflow1:,1,port-1",
        };

        for (String desc: invalid) {
            VtnPortDesc vdesc = new VtnPortDesc(desc);
            assertEquals(null, NodeUtils.toPortLocation(vdesc));
        }

        String[] names = {
            null, "port-1", "port-2",
        };
        String[] ids = {
            null, "1", "2",
        };
        String ofType = NodeConnectorIDType.OPENFLOW;

        for (long dpid = 1L; dpid <= 10L; dpid++) {
            SalNode snode = new SalNode(dpid);
            Node node = snode.getAdNode();
            for (String id: ids) {
                String i = (id == null) ? "" : id;
                for (String name: names) {
                    String n = (name == null) ? "" : name;
                    VtnPortDesc vdesc = new VtnPortDesc(
                        "openflow:" + dpid + "," + i + "," + n);
                    String type = (id == null) ? null : ofType;
                    SwitchPort swport = (id == null && name == null)
                        ? null
                        : new SwitchPort(name, type, id);
                    PortLocation expected = new PortLocation(node, swport);
                    assertEquals(expected, NodeUtils.toPortLocation(vdesc));
                }
            }
        }
    }

    /**
     * Test case for {@link NodeUtils#toPortLocation(VtnPortLocation)}.
     */
    @Test
    public void testToPortLocation2() {
        assertEquals(null, NodeUtils.toPortLocation((VtnPortLocation)null));

        String[] names = {
            null, "port-1", "port-2",
        };
        String[] ids = {
            null, "1", "2",
        };

        for (long dpid = 1L; dpid <= 10L; dpid++) {
            SalNode snode = new SalNode(dpid);
            Node node = snode.getAdNode();
            NodeId nodeId = snode.getNodeId();
            for (String id: ids) {
                String type = (id == null)
                    ? null : NodeConnectorIDType.OPENFLOW;
                for (String name: names) {
                    VtnPortLocation vploc = new DataIngressPortBuilder().
                        setNode(nodeId).setPortId(id).setPortName(name).
                        build();
                    SwitchPort swport = (id == null && name == null)
                        ? null : new SwitchPort(name, type, id);
                    PortLocation expected = new PortLocation(node, swport);
                    assertEquals(expected, NodeUtils.toPortLocation(vploc));
                }
            }
        }
    }

    /**
     * Test case for {@link NodeUtils#toSwitchPort(VtnSwitchPort)}.
     */
    @Test
    public void testToSwitchPort() {
        assertEquals(null, NodeUtils.toSwitchPort((VtnSwitchPort)null));

        String[] names = {
            null, "port-1", "port-2",
        };
        String[] ids = {
            null, "1", "2",
        };

        for (String id: ids) {
            String type = (id == null) ? null : NodeConnectorIDType.OPENFLOW;
            for (String name: names) {
                VtnSwitchPort vswp = new PhysicalIngressPortBuilder().
                    setPortId(id).setPortName(name).build();
                SwitchPort expected = (id == null && name == null)
                    ? null : new SwitchPort(name, type, id);
                assertEquals(expected, NodeUtils.toSwitchPort(vswp));
            }
        }
    }

    /**
     * Verify that {@link NodeUtils#checkNodeType(String)} throws
     * an {@link RpcException} if the given node type is invalid.
     *
     * @param type  A node type.
     */
    private void checkInvalidNodeType(String type) {
        try {
            checkNodeType(type);
            fail("Succeeded unexpectedly: type=" + type);
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            Status status = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, status.getCode());
            String msg = "Unsupported node: type=" + type;
            assertEquals(msg, status.getDescription());
        }
    }

    /**
     * Verify that {@link NodeUtils#checkNodeConnectorType(String)}
     * throws an {@link RpcException} if the given node type is invalid.
     *
     * @param type  A node connector type.
     */
    private void checkInvalidNodeConnectorType(String type) {
        try {
            checkNodeConnectorType(type);
            fail("Succeeded unexpectedly: type=" + type);
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            Status status = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, status.getCode());
            String msg = "Unsupported node connector: type=" + type;
            assertEquals(msg, status.getDescription());
        }
    }

    /**
     * Create a node for test.
     *
     * @param type   Node type.
     * @param id     Identifier of a new node.
     * @return       A {@link Node} object for test.
     */
    private Node createNode(String type, long id) {
        Object nodeId = null;
        if (NodeIDType.OPENFLOW.equals(type)) {
            nodeId = Long.valueOf(id);
        } else if (NodeIDType.PCEP.equals(type)) {
            long high = id >>> 48;
            long low = id & 0xffffffffffffL;
            StringBuilder builder = new StringBuilder("00000000-0000-0000-");
            builder.append(String.format("%04x", high)).append('-').
                append(String.format("%012x", low));
            nodeId = UUID.fromString(builder.toString());
        } else if (NodeIDType.ONEPK.equals(type) ||
                   NodeIDType.PRODUCTION.equals(type)) {
            nodeId = "node-" + id;
        } else {
            fail("Unexpected node type: " + type);
        }

        try {
            return new Node(type, nodeId);
        } catch (Exception e) {
            unexpected(e);
        }

        return null;
    }

    /**
     * Create a node connector for test.
     *
     * @param node  A {@link Node} object.
     * @param type  Node connector type.
     *              If {@code null} is specified, this method creates a node
     *              connector which represents a physical port.
     *              If a non-{@code null} value is specified, this method
     *              creates a node connector which represents a pseudo port.
     * @param id    Identifier of a new node connector.
     * @return      A {@link NodeConnector} object for test.
     */
    private NodeConnector createNodeConnector(Node node, String type, int id) {
        String nodeType = node.getType();
        String portType = null;
        Object portId = null;
        if (type == null) {
            // Create a node connector corresponding to a physical port.
            if (NodeIDType.OPENFLOW.equals(nodeType)) {
                portType = NodeConnectorIDType.OPENFLOW;
                portId = Short.valueOf((short)id);
            } else if (NodeIDType.PCEP.equals(nodeType)) {
                portType = NodeConnectorIDType.PCEP;
                portId = Integer.valueOf(id);
            } else if (NodeIDType.ONEPK.equals(nodeType)) {
                portType = NodeConnectorIDType.ONEPK;
                portId = "port-" + id;
            } else if (NodeIDType.PRODUCTION.equals(nodeType)) {
                portType = NodeConnectorIDType.PRODUCTION;
                portId = "port-" + id;
            } else {
                fail("Unexpected node type: " + nodeType);
            }
        } else {
            // Create a node connector corresponding to a pseudo port.
            portType = type;
            portId = Short.valueOf((short)id);
        }

        try {
            return new NodeConnector(portType, portId, node);
        } catch (Exception e) {
            unexpected(e);
        }

        return null;
    }
}
