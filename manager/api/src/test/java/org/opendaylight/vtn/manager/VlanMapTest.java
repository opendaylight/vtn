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
 * JUnit test for {@link VlanMap}.
 */
public class VlanMapTest extends TestBase {
    /**
     * Root XML element name associated with {@link VlanMap} class.
     */
    private static final String  XML_ROOT = "vlanmap";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = {-5, 0, 3, 4095};
        for (String id: createStrings("map")) {
            for (Node node: createNodes(3)) {
                for (short vlan: vlans) {
                    VlanMap vlmap = new VlanMap(id, node, vlan);
                    assertEquals(id, vlmap.getId());
                    assertEquals(node, vlmap.getNode());
                    assertEquals(vlan, vlmap.getVlan());
                }
            }
        }
    }

    /**
     * Test case for {@link VlanMap#equals(Object)} and
     * {@link VlanMap#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> ids = createStrings("map");
        List<Node> nodes = createNodes(3);
        short[] vlans = {-5, 0, 3, 4095};
        for (String id: ids) {
            for (Node node: nodes) {
                for (short vlan: vlans) {
                    VlanMap v1 = new VlanMap(id, node, vlan);
                    VlanMap v2 = new VlanMap(copy(id), copy(node), vlan);
                    testEquals(set, v1, v2);
                }
            }
        }

        int required = ids.size() * nodes.size() * vlans.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VlanMap#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VlanMap[";
        String suffix = "]";
        short[] vlans = {-5, 0, 3, 4095};
        for (String id: createStrings("map")) {
            for (Node node: createNodes(3)) {
                for (short vlan : vlans) {
                    VlanMap vlmap = new VlanMap(id, node, vlan);
                    if (id != null) {
                        id = "id=" + id;
                    }
                    String n = (node != null) ? "node=" + node : null;
                    String v = "vlan=" + vlan;
                    String required =
                        joinStrings(prefix, suffix, ",", id, n, v);
                    assertEquals(required, vlmap.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link VlanMap} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {-5, 0, 3, 4095};
        for (String id: createStrings("map")) {
            for (Node node: createNodes(3)) {
                for (short vlan: vlans) {
                    VlanMap vlmap = new VlanMap(id, node, vlan);
                    serializeTest(vlmap);
                }
            }
        }
    }

    /**
     * Ensure that {@link VlanMap} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        short[] vlans = {-5, 0, 3, 4095};
        for (String id: createStrings("map")) {
            for (Node node: createNodes(3)) {
                for (short vlan: vlans) {
                    VlanMap vlmap = new VlanMap(id, node, vlan);
                    jaxbTest(vlmap, VlanMap.class, XML_ROOT);
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(VlanMap.class,
                      VlanMapConfigTest.getXmlDataTypes(XML_ROOT));
    }
}
