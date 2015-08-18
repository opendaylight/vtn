/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node;

/**
 * JUnit test for {@link NodeVlan}.
 */
public class NodeVlanTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (Node node: createNodes(10)) {
            for (short vlan = -10; vlan <= 10; vlan++) {
                NodeVlan nvlan = new NodeVlan(node, vlan);
                assertEquals(node, nvlan.getNode());
                assertEquals(vlan, nvlan.getVlan());
            }
        }
    }

    /**
     * Test case for {@link NodeVlan#equals(Object)} and
     * {@link NodeVlan#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Node> nodes = createNodes(10);
        short[] vlans = {-10, -1, 0, 1, 10, 100, 4095, 10000};
        for (Node node: nodes) {
            for (short vlan: vlans) {
                NodeVlan nk1 = new NodeVlan(node, vlan);
                NodeVlan nk2 = new NodeVlan(copy(node), vlan);
                testEquals(set, nk1, nk2);
            }
        }

        int required = nodes.size() * vlans.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link NodeVlan#toString()} and
     * {@link NodeVlan#appendContents(StringBuilder)}.
     */
    @Test
    public void testToString() {
        String prefix = "NodeVlan[";
        String suffix = "]";
        short[] vlans = {-10, -1, 0, 1, 10, 100, 4095, 10000};
        for (Node node: createNodes(10)) {
            for (short vlan: vlans) {
                NodeVlan nvlan = new NodeVlan(node, vlan);
                String n = (node == null) ? null : "node=" + node;
                String v = "vlan=" + vlan;

                StringBuilder builder = new StringBuilder();
                nvlan.appendContents(builder);
                String required = joinStrings(null, null, ",", n, v);
                assertEquals(required, builder.toString());

                required = joinStrings(prefix, suffix, ",", n, v);
                assertEquals(required, nvlan.toString());
            }
        }
    }

    /**
     * Ensure that {@link NodeVlan} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {-10, -1, 0, 1, 10, 100, 4095, 10000};
        for (Node node: createNodes(10)) {
            for (short vlan: vlans) {
                NodeVlan nvlan = new NodeVlan(node, vlan);
                serializeTest(nvlan);
            }
        }
    }
}
