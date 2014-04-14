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
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.topology.TopoEdgeUpdate;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;

/**
 * JUnit test for {@link EdgeUpdateState}.
 */
public class EdgeUpdateStateTest extends TestUseVTNManagerBase {
    /**
     * Construct a new instance.
     */
    public EdgeUpdateStateTest() {
        super(2);
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        Node[] nodes = new Node[4];
        HashMap<Node, List<NodeConnector>> ports =
            new HashMap<Node, List<NodeConnector>>();
        HashMap<NodeConnector, Boolean> islMap =
            new HashMap<NodeConnector, Boolean>();
        HashSet<NodeConnector> islPorts = new HashSet<NodeConnector>();
        HashSet<NodeConnector> edgePorts = new HashSet<NodeConnector>();
        HashSet<NodeConnector> unchanged = new HashSet<NodeConnector>();

        for (int i = 0; i < nodes.length; i++) {
            Node node = NodeCreator.createOFNode(Long.valueOf(i));
            assertNotNull(node);
            nodes[i] = node;

            ArrayList<NodeConnector> plist =
                new ArrayList<NodeConnector>(nodes.length);
            for (int j = 0; j < nodes.length; j++) {
                NodeConnector nc = NodeConnectorCreator.
                    createOFNodeConnector(Short.valueOf((short)(j + 1)), node);
                assertNotNull(nc);
                plist.add(nc);

                if (j < i) {
                    islPorts.add(nc);
                    islMap.put(nc, Boolean.TRUE);
                } else if (j > i) {
                    edgePorts.add(nc);
                    islMap.put(nc, Boolean.FALSE);
                } else {
                    unchanged.add(nc);
                }
            }

            ports.put(node, plist);
        }

        assertFalse(islPorts.isEmpty());
        assertFalse(edgePorts.isEmpty());
        assertFalse(unchanged.isEmpty());

        EdgeUpdateState estate = new EdgeUpdateState(islMap);
        for (NodeConnector nc: islPorts) {
            assertEquals(Boolean.TRUE, estate.getPortState(nc));
            assertTrue(estate.accept(nc, null));
        }

        for (NodeConnector nc: edgePorts) {
            assertEquals(Boolean.FALSE, estate.getPortState(nc));
            assertFalse(estate.accept(nc, null));
        }

        for (NodeConnector nc: unchanged) {
            assertNull(estate.getPortState(nc));
            assertFalse(estate.accept(nc, null));
        }

        for (Node node: nodes) {
            assertTrue(estate.contains(node));
        }

        for (int i = 0; i < 10; i++) {
            long id = nodes.length + i;
            Node node = NodeCreator.createOFNode(Long.valueOf(id));
            assertNotNull(node);
            assertFalse(estate.contains(node));
        }
    }

    /**
     * Test case for {@link EdgeUpdateState#hasEdgePort(VTNManagerImpl, Node)}.
     */
    @Test
    public void testHasEdgePort() {
        VTNManagerImpl mgr = vtnMgr;
        ISwitchManager swMgr = mgr.getSwitchManager();
        Set<Node> nodeSet = swMgr.getNodes();
        ITopologyManager tpMgr = mgr.getTopologyManager();

        List<NodeConnector> edgePortList = new ArrayList<NodeConnector>();
        HashMap<Node, Boolean> edgeMap = new HashMap<Node, Boolean>();
        boolean hasEdge = false;
        for (Node node: nodeSet) {
            Boolean edge = Boolean.FALSE;
            for (NodeConnector nc: swMgr.getNodeConnectors(node)) {
                if (!swMgr.isSpecial(nc) && !tpMgr.isInternal(nc)) {
                    edge = Boolean.TRUE;
                    edgePortList.add(nc);
                    hasEdge = true;
                }
            }

            edgeMap.put(node, edge);
            assertEquals(edge.booleanValue(), mgr.hasEdgePort(node));
        }

        assertTrue(hasEdge);

        // hasEdgeState() does not use internal ISL map.
        EdgeUpdateState estate = new EdgeUpdateState(null);
        for (Map.Entry<Node, Boolean> entry: edgeMap.entrySet()) {
            Node node = entry.getKey();
            Boolean expected = entry.getValue();
            assertEquals(expected.booleanValue(),
                         estate.hasEdgePort(mgr, node));
        }

        // Link all ports in one node.
        Node[] nodes = nodeSet.toArray(new Node[nodeSet.size()]);
        Node node0 = nodes[0];
        List<TopoEdgeUpdate> topoList = new ArrayList<TopoEdgeUpdate>();

        for (NodeConnector nc: swMgr.getNodeConnectors(node0)) {
            if (swMgr.isSpecial(nc) || tpMgr.isInternal(nc)) {
                continue;
            }

            while (true) {
                NodeConnector other = edgePortList.remove(0);
                assertNotNull(other);
                if (!node0.equals(other.getNode())) {
                    addEdge(nc, other, topoList);
                    addEdge(other, nc, topoList);
                    break;
                }
            }
        }

        // Result of hasEdgeState() must be cached in EdgeUpdateState instance.
        mgr.edgeUpdate(topoList);
        assertFalse(mgr.hasEdgePort(node0));

        for (Map.Entry<Node, Boolean> entry: edgeMap.entrySet()) {
            Node node = entry.getKey();
            Boolean expected = entry.getValue();
            assertEquals(expected.booleanValue(),
                         estate.hasEdgePort(mgr, node));
        }
    }

    /**
     *  Add the specified edge.
     *
     * @param tail      A tail node output connector.
     * @param head      A head node input connector.
     * @param topoList  A list of {@link TopoEdgeUpdate} to store events.
     */
    private void addEdge(NodeConnector tail, NodeConnector head,
                         List<TopoEdgeUpdate> topoList) {
        try {
            Edge edge = new Edge(tail, head);
            Set<Property> propSet = new HashSet<Property>();
            TopoEdgeUpdate update =
                new TopoEdgeUpdate(edge, propSet, UpdateType.ADDED);
            topoList.add(update);
            stubObj.addEdge(edge);
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
