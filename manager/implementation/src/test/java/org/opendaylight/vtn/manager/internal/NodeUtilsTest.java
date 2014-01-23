/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Random;
import java.util.UUID;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.sal.core.Bandwidth;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node.NodeIDType;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;

import static org.opendaylight.vtn.manager.internal.NodeUtils.isNodeSupported;
import static
    org.opendaylight.vtn.manager.internal.NodeUtils.isNodeConnectorSupported;
import static org.opendaylight.vtn.manager.internal.NodeUtils.checkNodeType;
import static
    org.opendaylight.vtn.manager.internal.NodeUtils.checkNodeConnectorType;
import static org.opendaylight.vtn.manager.internal.NodeUtils.isSpecial;

/**
 * JUnit test for {@link NodeUtils}.
 */
public class NodeUtilsTest extends TestBase {
    /**
     * Type name for MD-SAL node and node connector.
     */
    private static final String  MD_SAL_TYPE = "MD_SAL";

    /**
     * Prepare test environment.
     */
    @Before
    public void setUp() {
        assertTrue(NodeIDType.registerIDType(MD_SAL_TYPE, String.class));
        assertTrue(NodeConnectorIDType.
                   registerIDType(MD_SAL_TYPE, String.class, MD_SAL_TYPE));
    }

    /**
     * Clean up test environment.
     */
    @After
    public void cleanUp() {
        NodeIDType.unRegisterIDType(MD_SAL_TYPE);
        NodeConnectorIDType.unRegisterIDType(MD_SAL_TYPE);
    }

    /**
     * Test case for {@link NodeUtils#isNodeSupported(String,Object)}.
     */
    @Test
    public void testIsNodeSupported() {
        // AD-SAL node test should use only node type.
        assertTrue(isNodeSupported(NodeIDType.OPENFLOW, null));
        assertTrue(isNodeSupported(NodeIDType.OPENFLOW, Long.valueOf(1)));

        assertFalse(isNodeSupported(NodeIDType.PCEP, UUID.randomUUID()));
        assertFalse(isNodeSupported(NodeIDType.ONEPK, "node-1"));
        assertFalse(isNodeSupported(NodeIDType.PRODUCTION, "node-2"));

        // Test for MD-SAL openflow node.
        for (long i = 0; i <= 10; i++) {
            String id = "openflow:" + i;
            assertTrue(isNodeSupported(MD_SAL_TYPE, id));
        }

        Random rand = new Random();
        for (int loop = 0; loop < 100; loop++) {
            long dpid = rand.nextLong();
            String id = "openflow:" + dpid;
            assertTrue(isNodeSupported(MD_SAL_TYPE, id));
        }

        // Test for MD-SAL non-openflow node.
        assertFalse(isNodeSupported(MD_SAL_TYPE, null));
        assertFalse(isNodeSupported(MD_SAL_TYPE, new Object()));
        assertFalse(isNodeSupported(MD_SAL_TYPE, Integer.valueOf(1)));
        assertFalse(isNodeSupported(MD_SAL_TYPE, "openflow123:0"));
        assertFalse(isNodeSupported(MD_SAL_TYPE, "proto1:0"));
        assertFalse(isNodeSupported(MD_SAL_TYPE, "proto2:0"));
    }

    /**
     * Test case for {@link NodeUtils#isNodeConnectorSupported(String,Object)}.
     */
    @Test
    public void testIsNodeConnectorSupported() {
        // AD-SAL node connector test should use only node type.
        assertTrue(isNodeConnectorSupported(NodeConnectorIDType.OPENFLOW,
                                            null));
        assertTrue(isNodeConnectorSupported(NodeConnectorIDType.OPENFLOW,
                                            Short.valueOf((short)1)));

        assertFalse(isNodeConnectorSupported(NodeConnectorIDType.PCEP,
                                            Integer.valueOf(1)));
        assertFalse(isNodeConnectorSupported(NodeConnectorIDType.ONEPK,
                                             "node-2"));
        assertFalse(isNodeConnectorSupported(NodeConnectorIDType.OPENFLOW2PCEP,
                                             Short.valueOf((short)3)));
        assertFalse(isNodeConnectorSupported(NodeConnectorIDType.PCEP2OPENFLOW,
                                             Integer.valueOf(5)));
        assertFalse(isNodeConnectorSupported(NodeConnectorIDType.PCEP2ONEPK,
                                             Integer.valueOf(6)));
        assertFalse(isNodeConnectorSupported(NodeConnectorIDType.ONEPK2OPENFLOW,
                                             "node-7"));
        assertFalse(isNodeConnectorSupported(NodeConnectorIDType.ONEPK2PCEP,
                                             "node-8"));
        assertFalse(isNodeConnectorSupported(NodeConnectorIDType.PRODUCTION,
                                             "node-9"));

        // MD-SAL openflow node connector.
        for (long i = 0; i <= 5; i++) {
            for (int j = 0; i <= 5; i++) {
                StringBuilder builder = new StringBuilder("openflow:");
                builder.append(i).append(':').append(j);
                String id = builder.toString();
                assertTrue(isNodeConnectorSupported(MD_SAL_TYPE, id));
            }
        }

        Random rand = new Random();
        for (int loop = 0; loop < 100; loop++) {
            long dpid = rand.nextLong();
            int port = rand.nextInt(0x10000);
            StringBuilder builder = new StringBuilder("openflow:");
            builder.append(dpid).append(':').append(port);
            String id = builder.toString();
            assertTrue(isNodeConnectorSupported(MD_SAL_TYPE, id));
        }

        // Test for MD-SAL non-openflow node connector.
        assertFalse(isNodeConnectorSupported(MD_SAL_TYPE, null));
        assertFalse(isNodeConnectorSupported(MD_SAL_TYPE, new Object()));
        assertFalse(isNodeConnectorSupported(MD_SAL_TYPE, Integer.valueOf(1)));
        assertFalse(isNodeConnectorSupported(MD_SAL_TYPE, "openflow123:0:1"));
        assertFalse(isNodeConnectorSupported(MD_SAL_TYPE, "proto1:0"));
        assertFalse(isNodeConnectorSupported(MD_SAL_TYPE, "proto2:0"));
    }

    /**
     * Test case for {@link NodeUtils#checkNodeType(String,Object)}.
     */
    @Test
    public void testCheckNodeType() {
        // AD-SAL node test should use only node type.
        try {
            checkNodeType(NodeIDType.OPENFLOW, null);
            checkNodeType(NodeIDType.OPENFLOW, Long.valueOf(1));
        } catch (VTNException e) {
            unexpected(e);
        }

        checkInvalidNodeType(NodeIDType.PCEP, UUID.randomUUID());
        checkInvalidNodeType(NodeIDType.ONEPK, "node-1");
        checkInvalidNodeType(NodeIDType.PRODUCTION, "node-2");

        // Test for MD-SAL openflow node.
        for (long i = 0; i <= 10; i++) {
            String id = "openflow:" + i;
            try {
                checkNodeType(MD_SAL_TYPE, id);
            } catch (VTNException e) {
                unexpected(e);
            }
        }

        Random rand = new Random();
        for (int loop = 0; loop < 100; loop++) {
            long dpid = rand.nextLong();
            String id = "openflow:" + dpid;
            try {
                checkNodeType(MD_SAL_TYPE, id);
            } catch (VTNException e) {
                unexpected(e);
            }
        }

        // Test for MD-SAL non-openflow node.
        checkInvalidNodeType(MD_SAL_TYPE, null);
        checkInvalidNodeType(MD_SAL_TYPE, new Object());
        checkInvalidNodeType(MD_SAL_TYPE, Integer.valueOf(1));
        checkInvalidNodeType(MD_SAL_TYPE, "openflow123:0");
        checkInvalidNodeType(MD_SAL_TYPE, "proto1:0");
        checkInvalidNodeType(MD_SAL_TYPE, "proto2:0");
    }

    /**
     * Test case for {@link NodeUtils#checkNodeConnectorType(String,Object)}.
     */
    @Test
    public void testCheckNodeConnectorType() {
        // AD-SAL node connector test should use only node type.
        try {
            checkNodeConnectorType(NodeConnectorIDType.OPENFLOW, null);
            checkNodeConnectorType(NodeConnectorIDType.OPENFLOW,
                                   Short.valueOf((short)1));
        } catch (VTNException e) {
            unexpected(e);
        }

        checkInvalidNodeConnectorType(NodeConnectorIDType.PCEP,
                                      Integer.valueOf(1));
        checkInvalidNodeConnectorType(NodeConnectorIDType.ONEPK, "node-2");
        checkInvalidNodeConnectorType(NodeConnectorIDType.OPENFLOW2PCEP,
                                      Short.valueOf((short)3));
        checkInvalidNodeConnectorType(NodeConnectorIDType.PCEP2OPENFLOW,
                                      Integer.valueOf(5));
        checkInvalidNodeConnectorType(NodeConnectorIDType.PCEP2ONEPK,
                                      Integer.valueOf(6));
        checkInvalidNodeConnectorType(NodeConnectorIDType.ONEPK2OPENFLOW,
                                      "node-7");
        checkInvalidNodeConnectorType(NodeConnectorIDType.ONEPK2PCEP,
                                      "node-8");
        checkInvalidNodeConnectorType(NodeConnectorIDType.PRODUCTION,
                                      "node-9");

        // MD-SAL openflow node connector.
        for (long i = 0; i <= 5; i++) {
            for (int j = 0; i <= 5; i++) {
                StringBuilder builder = new StringBuilder("openflow:");
                builder.append(i).append(':').append(j);
                String id = builder.toString();
                try {
                    checkNodeConnectorType(MD_SAL_TYPE, id);
                } catch (VTNException e) {
                    unexpected(e);
                }
            }
        }

        Random rand = new Random();
        for (int loop = 0; loop < 100; loop++) {
            long dpid = rand.nextLong();
            int port = rand.nextInt(0x10000);
            StringBuilder builder = new StringBuilder("openflow:");
            builder.append(dpid).append(':').append(port);
            String id = builder.toString();
            try {
                checkNodeConnectorType(MD_SAL_TYPE, id);
            } catch (VTNException e) {
                unexpected(e);
            }
        }

        // Test for MD-SAL non-openflow node connector.
        checkInvalidNodeConnectorType(MD_SAL_TYPE, null);
        checkInvalidNodeType(MD_SAL_TYPE, new Object());
        checkInvalidNodeType(MD_SAL_TYPE, Integer.valueOf(1));
        checkInvalidNodeType(MD_SAL_TYPE, "openflow123:0:1");
        checkInvalidNodeType(MD_SAL_TYPE, "proto1:0");
        checkInvalidNodeType(MD_SAL_TYPE, "proto2:0");
    }

    /**
     * Test case for
     * {@link NodeUtils#isSpecial(ISwitchManager,NodeConnector,Map)}.
     */
    @Test
    public void testIsSpecial() {
        // Test for AD-SAL node.
        String[] adTypes = {
            NodeIDType.OPENFLOW,
            NodeIDType.PCEP,
            NodeIDType.ONEPK,
            NodeIDType.PRODUCTION,
        };
        String[] pseudoTypes = {
            NodeConnectorIDType.CONTROLLER,
            NodeConnectorIDType.ALL,
            NodeConnectorIDType.SWSTACK,
            NodeConnectorIDType.HWPATH,
        };

        TestSwitchManager swMgr = new TestSwitchManager();
        for (String type: adTypes) {
            for (int nodeId = 0; nodeId <= 10; nodeId++) {
                Node node = createNode(type, nodeId);
                for (int portId = 0; portId <= 10; portId++) {
                    for (String pseudo: pseudoTypes) {
                        NodeConnector nc =
                            createNodeConnector(node, pseudo, portId);
                        assertTrue(isSpecial(swMgr, nc, null));
                    }

                    NodeConnector nc = createNodeConnector(node, null, portId);
                    assertFalse(isSpecial(swMgr, nc, null));
                }
            }
        }

        // Test for MD-SAL node.
        Random rand = new Random();
        HashMap<NodeConnector, Map<String, Property>> propMap =
            new HashMap<NodeConnector, Map<String, Property>>();
        HashSet<NodeConnector> physical = new HashSet<NodeConnector>();

        for (int loop = 0; loop < 100; loop++) {
            long swId = rand.nextLong();
            int portId = rand.nextInt(100);
            String proto;
            if (rand.nextBoolean()) {
                proto = "openflow";
            } else {
                proto = "proto-" + rand.nextInt(10);
            }

            Node node = createMdNode(proto, swId);
            NodeConnector nc;
            do {
                nc = createMdNodeConnector(node, portId);
            } while (propMap.containsKey(nc));

            HashMap<String, Property> props = new HashMap<String, Property>();
            propMap.put(nc, props);

            ArrayList<Property> list = new ArrayList<Property>();
            StringBuilder builder = new StringBuilder("sw");
            builder.append(swId).append("-eth").append(portId);
            list.add(new Name(builder.toString()));
            list.add(new Config(Config.ADMIN_UP));
            list.add(new State(State.EDGE_UP));
            if (physical.isEmpty() || rand.nextBoolean()) {
                list.add(new Bandwidth(Bandwidth.BW10Gbps));
                assertTrue(physical.add(nc));
            }

            for (Property prop: list) {
                props.put(prop.getName(), prop);
            }
        }

        // Call isSpecial() without registering properties.
        // In this case isSpecial() with specifying null property should
        // always return true.
        for (Map.Entry<NodeConnector, Map<String, Property>> entry:
                 propMap.entrySet()) {
            NodeConnector nc = entry.getKey();
            Map<String, Property> props = entry.getValue();
            boolean special = !physical.contains(nc);
            assertEquals(special, isSpecial(swMgr, nc, props));
            assertTrue(isSpecial(swMgr, nc, null));
        }

        // Register properties to stub switch manager.
        for (Map.Entry<NodeConnector, Map<String, Property>> entry:
                 propMap.entrySet()) {
            NodeConnector nc = entry.getKey();
            Map<String, Property> props = entry.getValue();
            swMgr.setProperty(nc, props);
        }

        // Call isSpecial() again.
        for (Map.Entry<NodeConnector, Map<String, Property>> entry:
                 propMap.entrySet()) {
            NodeConnector nc = entry.getKey();
            Map<String, Property> props = entry.getValue();
            boolean special = !physical.contains(nc);
            assertEquals(special, isSpecial(swMgr, nc, props));
            assertEquals(special, isSpecial(swMgr, nc, null));
        }
    }

    /**
     * Verify that {@link NodeUtils#checkNodeType(String,Object)} throws
     * an {@link VTNException} if the given node type is invalid.
     */
    private void checkInvalidNodeType(String type, Object id) {
        try {
            checkNodeType(type, id);
            fail("Succeeded unexpectedly: type=" + type + ", id=" + id);
        } catch (VTNException e) {
            Status status = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, status.getCode());
            String msg = "Unsupported node: type=" + type + ", id=" + id;
            assertEquals(msg, status.getDescription());
        }
    }

    /**
     * Verify that {@link NodeUtils#checkNodeConnectorType(String,Object)}
     * throws an {@link VTNException} if the given node type is invalid.
     */
    private void checkInvalidNodeConnectorType(String type, Object id) {
        try {
            checkNodeConnectorType(type, id);
            fail("Succeeded unexpectedly: type=" + type + ", id=" + id);
        } catch (VTNException e) {
            Status status = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, status.getCode());
            String msg = "Unsupported node connector: type=" + type +
                ", id=" + id;
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
     * Create a MD-SAL node for test.
     *
     * @param proto  Name of protocol plugin.
     * @param id     Identifier of a new node.
     * @return       A {@link Node} object for test.
     */
    private Node createMdNode(String proto, long id) {
        StringBuilder builder = new StringBuilder(proto);
        builder.append(':').append(id);

        try {
            return new Node(MD_SAL_TYPE, builder.toString());
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

    /**
     * Create a MD-SAL node connector for test.
     *
     * @param node  A {@link Node} object.
     * @param id    Identifier of a new node connector.
     * @return      A {@link NodeConnector} object for test.
     */
    private NodeConnector createMdNodeConnector(Node node, int id) {
        StringBuilder builder = new StringBuilder(node.getID().toString());
        builder.append(':').append(id);

        try {
            return new NodeConnector(MD_SAL_TYPE, builder.toString(), node);
        } catch (Exception e) {
            unexpected(e);
        }

        return null;
    }

    /**
     * Implementation of {@link ISwitchManager} for test.
     */
    private class TestSwitchManager extends TestStub {
        /**
         * A set of properties for node connectors.
         */
        private Map<NodeConnector, Map<String, Property>> nodeConnectorProps =
            new HashMap<NodeConnector, Map<String, Property>>();

        /**
         * Construct a switch manager service.
         */
        private TestSwitchManager() {
            super(1);
        }

        /**
         * Return a specific property of a node connector given the property
         * name.
         *
         * @param nc    A {@link NodeConnector} object.
         * @param name  Property name.
         * @return      A {@link Property} object if found.
         *              {@code null} if not found.
         */
        @Override
        public Property getNodeConnectorProp(NodeConnector nc, String name) {
            Map<String, Property> map = nodeConnectorProps.get(nc);
            return (map == null) ? null : map.get(name);
        }

        /**
         * Set property map for the given node connector.
         *
         * @param nc     A {@link NodeConnector} object.
         * @param props  Map which contains properties for {@code nc}.
         */
        private void setProperty(NodeConnector nc,
                                 Map<String, Property> props) {
            nodeConnectorProps.put(nc, props);
        }
    }
}
