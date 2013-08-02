/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.controller.sal.core.Node;

/**
 * JUnit test for {@link VlanMapConfig}.
 */
public class VlanMapConfigTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (Node node: createNodes(5)) {
            for (short vlan = -10; vlan <= 10; vlan++) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                assertEquals(node, vlconf.getNode());
                assertEquals(vlan, vlconf.getVlan());
            }
        }
    }

    /**
     * Test case for {@link VlanMapConfig#isOverlapped(VlanMapConfig)}.
     */
    @Test
    public void testIsOverlapped() {
        Node differentNode = null;
        try {
            differentNode = new Node(Node.NodeIDType.PRODUCTION,
                                     "Different Node");
        } catch (Exception e) {
            unexpected(e);
        }

        for (Node node: createNodes(5)) {
            for (short vlan = -10; vlan <= 10; vlan++) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                assertTrue(vlconf.isOverlapped(vlconf));

                VlanMapConfig vc = new VlanMapConfig(null, vlan);
                assertTrue(vlconf.isOverlapped(vc));

                for (short i = 1; i < 3; i++) {
                    vc = new VlanMapConfig(null, (short)(vlan + i));
                    assertFalse(vlconf.isOverlapped(vc));
                }

                vc = new VlanMapConfig(differentNode, vlan);
                if (node == null) {
                    assertTrue(vlconf.isOverlapped(vc));
                } else {
                    assertFalse(vlconf.isOverlapped(vc));
                }
            }
        }
    }

    /**
     * Test case for {@link VlanMapConfig#equals(Object)} and
     * {@link VlanMapConfig#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Node> nodes = createNodes(5);
        short minvlan = -10;
        short maxvlan = 10;
        for (Node node: nodes) {
            for (short vlan = minvlan; vlan <= maxvlan; vlan++) {
                VlanMapConfig vc1 = new VlanMapConfig(node, vlan);
                VlanMapConfig vc2 = new VlanMapConfig(copy(node), vlan);
                testEquals(set, vc1, vc2);
            }
        }

        int nvlan = (int)(maxvlan - minvlan) + 1;
        int required = nodes.size() * nvlan;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VlanMapConfig#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VlanMapConfig[";
        String suffix = "]";
        for (Node node: createNodes(5)) {
            for (short vlan = -10; vlan <= 10; vlan++) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                String n = (node != null) ? "node=" + node : null;
                String v = "vlan=" + vlan;
                String required = joinStrings(prefix, suffix, ",", n, v);
                assertEquals(required, vlconf.toString());
            }
        }
    }

    /**
     * Ensure that {@link VlanMapConfig} is serializable.
     */
    @Test
    public void testSerialize() {
        for (Node node: createNodes(5)) {
            for (short vlan = -10; vlan <= 10; vlan++) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                serializeTest(vlconf);
            }
        }
    }

    /**
     * Ensure that {@link VlanMapConfig} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (Node node: createNodes(5)) {
            for (short vlan = -10; vlan <= 10; vlan++) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                jaxbTest(vlconf, "vlanmapconf");
            }
        }
    }
}
