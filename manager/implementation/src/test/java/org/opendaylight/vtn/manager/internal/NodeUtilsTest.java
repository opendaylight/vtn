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

import static org.opendaylight.vtn.manager.internal.NodeUtils.checkNodeType;
import static
    org.opendaylight.vtn.manager.internal.NodeUtils.checkNodeConnectorType;

/**
 * JUnit test for {@link NodeUtils}.
 */
public class NodeUtilsTest extends TestBase {
    /**
     * Test case for {@link NodeUtils#checkNodeType(String)}.
     */
    @Test
    public void testCheckNodeType() {
        try {
            checkNodeType(NodeIDType.OPENFLOW);
        } catch (VTNException e) {
            unexpected(e);
        }

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
     */
    @Test
    public void testCheckNodeConnectorType() {
        try {
            checkNodeConnectorType(NodeConnectorIDType.OPENFLOW);
        } catch (VTNException e) {
            unexpected(e);
        }

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
     * Verify that {@link NodeUtils#checkNodeType(String)} throws
     * an {@link VTNException} if the given node type is invalid.
     *
     * @param type  A node type.
     */
    private void checkInvalidNodeType(String type) {
        try {
            checkNodeType(type);
            fail("Succeeded unexpectedly: type=" + type);
        } catch (VTNException e) {
            Status status = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, status.getCode());
            String msg = "Unsupported node: type=" + type;
            assertEquals(msg, status.getDescription());
        }
    }

    /**
     * Verify that {@link NodeUtils#checkNodeConnectorType(String)}
     * throws an {@link VTNException} if the given node type is invalid.
     *
     * @param type  A node connector type.
     */
    private void checkInvalidNodeConnectorType(String type) {
        try {
            checkNodeConnectorType(type);
            fail("Succeeded unexpectedly: type=" + type);
        } catch (VTNException e) {
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
