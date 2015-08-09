/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

/**
 * JUnit test for {@link PortMap}.
 */
public class PortMapTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        PortMap empty = new PortMap(null, null);
        assertNull(empty.getConfig());
        assertNull(empty.getNodeConnector());

        for (NodeConnector nc: createNodeConnectors(3)) {
            for (Node node: createNodes(3)) {
                for (SwitchPort port: createSwitchPorts(5)) {
                    for (short vlan = -3; vlan <= 3; vlan++) {
                        PortMapConfig pmconf =
                            new PortMapConfig(node, port, vlan);
                        PortMap pmap = new PortMap(pmconf, nc);
                        assertEquals(pmconf, pmap.getConfig());
                        assertEquals(nc, pmap.getNodeConnector());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link PortMap#equals(Object)} and
     * {@link PortMap#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<NodeConnector> connectors = createNodeConnectors(3);
        List<Node> nodes = createNodes(3);
        List<SwitchPort> ports = createSwitchPorts(5);
        short[] vlans = {-10, 0, 1, 100, 4095};
        for (NodeConnector nc: connectors) {
            for (Node node: nodes) {
                for (SwitchPort port: ports) {
                    for (short vlan: vlans) {
                        PortMapConfig pc1 =
                            new PortMapConfig(node, port, vlan);
                        PortMapConfig pc2 =
                            new PortMapConfig(copy(node), copy(port), vlan);
                        PortMap pm1 = new PortMap(pc1, nc);
                        PortMap pm2 = new PortMap(pc2, copy(nc));
                        testEquals(set, pm1, pm2);
                    }
                }
            }
        }

        testEquals(set, new PortMap(null, null), new PortMap(null, null));

        int required = connectors.size() * nodes.size() * ports.size() *
            vlans.length + 1;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link PortMap#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "PortMap[";
        String suffix = "]";
        short[] vlans = {-10, 0, 4095};
        for (NodeConnector nc: createNodeConnectors(3)) {
            for (Node node: createNodes(3)) {
                for (SwitchPort port: createSwitchPorts(5)) {
                    for (short vlan: vlans) {
                        PortMapConfig pmconf =
                            new PortMapConfig(node, port, vlan);
                        PortMap pmap = new PortMap(pmconf, nc);
                        String c = (nc == null) ? null : "connector=" + nc;
                        String cf = "config=" + pmconf;
                        String required =
                            joinStrings(prefix, suffix, ",", cf, c);
                        assertEquals(required, pmap.toString());
                    }
                }
            }
        }

        PortMap empty = new PortMap(null, null);
        assertEquals("PortMap[]", empty.toString());

        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc = NodeConnectorCreator.createOFNodeConnector((short)0, node);
        empty = new PortMap(null, nc);
        assertEquals("PortMap[connector=" + nc.toString() + "]", empty.toString());

    }

    /**
     * Ensure that {@link PortMap} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {-10, 0, 4095};
        for (NodeConnector nc: createNodeConnectors(3)) {
            for (Node node: createNodes(3)) {
                for (SwitchPort port: createSwitchPorts(5)) {
                    for (short vlan: vlans) {
                        PortMapConfig pmconf =
                            new PortMapConfig(node, port, vlan);
                        PortMap pmap = new PortMap(pmconf, nc);
                        serializeTest(pmap);
                    }
                }
            }
        }

        PortMap empty = new PortMap(null, null);
        serializeTest(empty);
    }
}
