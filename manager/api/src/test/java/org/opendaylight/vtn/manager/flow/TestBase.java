/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.junit.Assert;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertNotNull;

public class TestBase extends Assert {

    /**
     * Ensure that the given two objects are identical.
     *
     * @param set  A set of tested objects.
     * @param o1   An object to be tested.
     * @param o2   An object to be tested.
     */
    protected static void toTestEquals(Set<Object> set, Object o1, Object o2) {
        assertEquals(o1, o2);
        assertEquals(o2, o1);
        assertEquals(o1, o1);
        assertEquals(o2, o2);
        assertEquals(o1.hashCode(), o2.hashCode());
        assertFalse(o1.equals(null));
        assertFalse(o1.equals(new Object()));
        assertFalse(o1.equals("string"));
        assertFalse(o1.equals(set));

        for (Object o: set) {
            assertFalse("o1=" + o1 + ", o=" + o, o1.equals(o));
            assertFalse(o.equals(o1));
        }

        assertTrue(set.add(o1));
        assertFalse(set.add(o1));
        assertFalse(set.add(o2));
    }
    /**
     * Throw an error which indicates an unexpected throwable is caught.
     *
     * @param t  A throwable.
     */
    protected static void unexpected(Throwable t) {
        throw new AssertionError("Unexpected throwable: " + t, t);
    }

    /**
     * Create a list of {@link NodeConnector}.
     *
     * @param num      The number of objects to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link NodeConnector}.
     */
    protected static List<NodeConnector> createNodeConnectors(
        int num, boolean setNull) {
        ArrayList<NodeConnector> list = new ArrayList<NodeConnector>();
        if (setNull) {
            list.add(null);
            num--;
        }

        String[] nodeTypes = {
            Node.NodeIDType.OPENFLOW,
            Node.NodeIDType.ONEPK,
            Node.NodeIDType.PRODUCTION,
        };
        String[] connTypes = {
            NodeConnector.NodeConnectorIDType.OPENFLOW,
            NodeConnector.NodeConnectorIDType.ONEPK,
            NodeConnector.NodeConnectorIDType.PRODUCTION,
        };

        for (int i = 0; i < num; i++) {
            int tidx = i % nodeTypes.length;
            String nodeType = nodeTypes[tidx];
            String connType = connTypes[tidx];
            Object nodeId, connId;
            if (nodeType.equals(Node.NodeIDType.OPENFLOW)) {
                nodeId = new Long((long)i);
                connId = new Short((short)(i + 10));
            } else {
                nodeId = "Node ID: " + i;
                connId = "Node Connector ID: " + i;
            }

            try {
                Node node = new Node(nodeType, nodeId);
                assertNotNull(node.getType());
                assertNotNull(node.getNodeIDString());
                NodeConnector nc = new NodeConnector(connType, connId, node);
                assertNotNull(nc.getType());
                assertNotNull(nc.getNodeConnectorIDString());
                list.add(nc);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

}
