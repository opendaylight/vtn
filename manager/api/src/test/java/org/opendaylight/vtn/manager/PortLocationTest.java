/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
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

/**
 * JUnit test for {@link PortLocation}.
 */
public class PortLocationTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                PortLocation ploc = new PortLocation(node, port);
                assertEquals(node, ploc.getNode());
                assertEquals(port, ploc.getPort());
            }
        }

        for (NodeConnector nc: createNodeConnectors(10, false)) {
            for (String name: createStrings("portname")) {
                PortLocation ploc = new PortLocation(nc, name);
                Node node = nc.getNode();
                SwitchPort port = new SwitchPort(nc, name);
                assertEquals(node, ploc.getNode());
                assertEquals(port, ploc.getPort());
            }
        }
    }

    /**
     * Test case for {@link PortLocation#equals(Object)} and
     * {@link PortLocation#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Node> nodes = createNodes(5);
        List<SwitchPort> ports = createSwitchPorts(10);
        for (Node node: nodes) {
            for (SwitchPort port: ports) {
                PortLocation ploc1 = new PortLocation(node, port);
                PortLocation ploc2 = new PortLocation(copy(node), copy(port));
                testEquals(set, ploc1, ploc2);
            }
        }

        assertEquals(nodes.size() * ports.size(), set.size());
    }

    /**
     * Test case for {@link PortLocation#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "PortLocation[";
        String suffix = "]";
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                PortLocation ploc = new PortLocation(node, port);

                String n = (node == null) ? null : "node=" + node;
                String p = (port == null) ? null : "port=" + port;
                String required = joinStrings(prefix, suffix, ",", n, p);
                assertEquals(required, ploc.toString());
            }
        }
    }

    /**
     * Ensure that {@link PortLocation} is serializable.
     */
    @Test
    public void testSerialize() {
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                PortLocation ploc = new PortLocation(node, port);
                serializeTest(ploc);
            }
        }
    }

    /**
     * Ensure that {@link PortLocation} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                PortLocation ploc = new PortLocation(node, port);
                jaxbTest(ploc, PortLocation.class, "portlocation");
            }
        }
    }

    /**
     * Ensure that {@link PortLocation} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(10)) {
                PortLocation ploc = new PortLocation(node, port);
                jsonTest(ploc, PortLocation.class);
            }
        }
    }
}
