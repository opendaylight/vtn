/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
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

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * JUnit test for {@link PortVlan}.
 */
public class PortVlanTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (NodeConnector nc: createNodeConnectors(10, false)) {
            for (short vlan = -10; vlan <= 10; vlan++) {
                PortVlan pvlan = new PortVlan(nc, vlan);
                assertEquals(nc, pvlan.getNodeConnector());
                assertEquals(vlan, pvlan.getVlan());
            }
        }
    }

    /**
     * Test case for {@link PortVlan#equals(Object)} and
     * {@link PortVlan#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<NodeConnector> connectors = createNodeConnectors(10, false);
        short[] vlans = {-10, -1, 0, 1, 10, 100, 4095, 10000};
        for (NodeConnector nc: connectors) {
            for (short vlan: vlans) {
                PortVlan pk1 = new PortVlan(nc, vlan);
                PortVlan pk2 = new PortVlan(copy(nc), vlan);
                testEquals(set, pk1, pk2);
            }
        }

        int required = connectors.size() * vlans.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link PortVlan#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "PortVlan[";
        String suffix = "]";
        short[] vlans = {-10, -1, 0, 1, 10, 100, 4095, 10000};
        for (NodeConnector nc: createNodeConnectors(10, false)) {
            for (short vlan: vlans) {
                PortVlan pvlan = new PortVlan(nc, vlan);
                String c = "connector=" + nc;
                String v = "vlan=" + vlan;
                String required = joinStrings(prefix, suffix, ",", c, v);
                assertEquals(required, pvlan.toString());
            }
        }
    }

    /**
     * Ensure that {@link PortVlan} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {-10, -1, 0, 1, 10, 100, 4095, 10000};
        for (NodeConnector nc: createNodeConnectors(10, false)) {
            for (short vlan: vlans) {
                PortVlan pvlan = new PortVlan(nc, vlan);
                serializeTest(pvlan);
            }
        }
    }
}
