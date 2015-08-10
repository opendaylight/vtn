/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * JUnit test for {@link PortMapInfo}.
 */
public class PortMapInfoTest extends TestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (NodeConnector nc: createNodeConnectors(3)) {
            for (Node node: createNodes(3)) {
                for (SwitchPort port: createSwitchPorts(5)) {
                    for (short vlan = -3; vlan <= 3; vlan++) {
                        String emsg = "(NodeConnector)"
                                + ((nc == null) ? "null" : nc.toString())
                                + "(Node)"
                                + ((node == null) ? "null" : node.toString())
                                + "(vlan)" + vlan;

                        PortMapConfig pmconf =
                            new PortMapConfig(node, port, vlan);
                        PortMapInfo pi = new PortMapInfo(pmconf, nc);
                        assertEquals(emsg, node, pi.getNode());
                        assertEquals(emsg, port, pi.getPort());
                        assertEquals(emsg, vlan, pi.getVlan());

                        SwitchPort sp = pi.getMappedPort();
                        if (nc == null) {
                            assertNull(emsg, sp);
                        } else {
                            assertNull(emsg, sp.getName());
                            assertEquals(emsg, nc.getType(), sp.getType());
                            assertEquals(emsg, nc.getNodeConnectorIDString(),
                                         sp.getId());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link PortMapInfo#equals(Object)} and
     * {@link PortMapInfo#hashCode()}.
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
                        PortMapInfo pi1 = new PortMapInfo(pc1, nc);
                        PortMapInfo pi2 = new PortMapInfo(pc2, copy(nc));
                        testEquals(set, pi1, pi2);
                    }
                }
            }
        }

        int required = connectors.size() * nodes.size() * ports.size() *
            vlans.length;
        assertEquals(required, set.size());
    }

    /**
     * Ensure that {@link PortMapInfo} is serializable.
     */
    @Test
    public void testSerialize() {
        String prefix = "PortMapInfo[";
        String suffix = "]";
        short[] vlans = {-10, 0, 4095};
        for (NodeConnector nc: createNodeConnectors(3)) {
            for (Node node: createNodes(3)) {
                for (SwitchPort port: createSwitchPorts(5)) {
                    for (short vlan: vlans) {
                        PortMapConfig pmconf =
                            new PortMapConfig(node, port, vlan);
                        PortMapInfo pi = new PortMapInfo(pmconf, nc);
                        serializeTest(pi);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link PortMapInfo} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        short[] vlans = {-10, 0, 4095};
        for (NodeConnector nc: createNodeConnectors(3)) {
            for (Node node: createNodes(3)) {
                for (SwitchPort port: createSwitchPorts(5)) {
                    for (short vlan: vlans) {
                        PortMapConfig pmconf =
                            new PortMapConfig(node, port, vlan);
                        PortMapInfo pi = new PortMapInfo(pmconf, nc);
                        jaxbTest(pi, PortMapInfo.class, "portmap");
                        jsonTest(pi, PortMapInfo.class);
                    }
                }
            }
        }
    }
}
