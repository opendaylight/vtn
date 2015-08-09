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

/**
 * JUnit test for {@link PortMapConfig}.
 */
public class PortMapConfigTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                for (short vlan = -5; vlan <= 5; vlan++) {
                    PortMapConfig pmconf = new PortMapConfig(node, port, vlan);
                    String emsg = pmconf.toString();
                    assertEquals(emsg, port, pmconf.getPort());
                    assertEquals(emsg, node, pmconf.getNode());
                    assertEquals(emsg, vlan, pmconf.getVlan());
                }
            }
        }
    }

    /**
     * Test case for {@link PortMapConfig#equals(Object)} and
     * {@link PortMapConfig#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Node> nodes = createNodes(5);
        List<SwitchPort> ports = createSwitchPorts(10);
        short[] vlans = {-100, 0, 1, 100, 4095, 10000};
        for (Node node: nodes) {
            for (SwitchPort port: ports) {
                for (short vlan: vlans) {
                    PortMapConfig pc1 = new PortMapConfig(node, port, vlan);
                    PortMapConfig pc2 =
                        new PortMapConfig(copy(node), copy(port), vlan);
                    testEquals(set, pc1, pc2);
                }
            }
        }

        int required = nodes.size() * ports.size() * vlans.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link PortMapConfig#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "PortMapConfig[";
        String suffix = "]";
        short[] vlans = {-10, 0, 10, 100, 4095};
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                for (short vlan: vlans) {
                    PortMapConfig pmconf = new PortMapConfig(node, port, vlan);
                    String n = (node == null) ? null : "node=" + node;
                    String p = (port == null) ? null : "port=" + port;
                    String v = "vlan=" + vlan;
                    String required =
                        joinStrings(prefix, suffix, ",", n, p, v);
                    assertEquals(required, pmconf.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link PortMapConfig} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {-10, 0, 10, 100, 4095};
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                for (short vlan: vlans) {
                    PortMapConfig pmconf = new PortMapConfig(node, port, vlan);
                    serializeTest(pmconf);
                }
            }
        }
    }

    /**
     * Ensure that {@link PortMapConfig} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        short[] vlans = {-10, 0, 10, 100, 4095};
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                for (short vlan: vlans) {
                    PortMapConfig pmconf = new PortMapConfig(node, port, vlan);
                    jaxbTest(pmconf, PortMapConfig.class, "portmapconf");
                }
            }
        }
    }
}
