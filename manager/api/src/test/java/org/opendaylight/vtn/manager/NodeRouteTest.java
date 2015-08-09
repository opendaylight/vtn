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
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

/**
 * JUnit test for {@link NodeRoute}.
 */
public class NodeRouteTest extends TestBase {
    /**
     * Root XML element name associated with {@link NodeRoute} class.
     */
    private static final String  XML_ROOT = "noderoute";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (Node node: createNodes(5)) {
            for (SwitchPort input: createSwitchPorts(5)) {
                for (SwitchPort output: createSwitchPorts(5)) {
                    NodeRoute nr = new NodeRoute(node, input, output);
                    assertEquals(node, nr.getNode());
                    assertEquals(input, nr.getInputPort());
                    assertEquals(output, nr.getOutputPort());
                }
            }
        }

        // Construct NodeRoute from NodeConnector.
        Node node = NodeCreator.createOFNode(Long.valueOf(12345L));
        NodeConnector inport = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)1), node);
        NodeConnector outport = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)4), node);
        assertNotNull(node);
        assertNotNull(inport);
        assertNotNull(outport);

        String iname = "port-1";
        String oname = "port-4";
        NodeRoute nr = new NodeRoute(inport, iname, outport, oname);
        SwitchPort input = new SwitchPort(inport, iname);
        SwitchPort output = new SwitchPort(outport, oname);
        assertEquals(node, nr.getNode());
        assertEquals(input, nr.getInputPort());
        assertEquals(output, nr.getOutputPort());

        // Specified ports should belong to the same switch.
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));
        outport = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)4), node1);
        assertNotNull(outport);
        try {
            new NodeRoute(inport, iname, outport, oname);
            unexpected();
        } catch (IllegalArgumentException e) {
        }
    }

    /**
     * Test case for {@link NodeRoute#equals(Object)} and
     * {@link NodeRoute#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Node> nodes = createNodes(5);
        List<SwitchPort> inputs = createSwitchPorts(5);
        List<SwitchPort> outputs = createSwitchPorts(5);
        for (Node node: nodes) {
            for (SwitchPort input: inputs) {
                for (SwitchPort output: outputs) {
                    NodeRoute nr1 = new NodeRoute(node, input, output);
                    NodeRoute nr2 = new NodeRoute(copy(node), copy(input),
                                                  copy(output));
                    testEquals(set, nr1, nr2);
                }
            }
        }

        int required = nodes.size() * inputs.size() * outputs.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link NodeRoute#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "NodeRoute[";
        String suffix = "]";
        for (Node node: createNodes(5)) {
            String n = (node == null) ? null : "node=" + node;
            for (SwitchPort input: createSwitchPorts(5)) {
                String i = (input == null) ? null : "input=" + input;
                for (SwitchPort output: createSwitchPorts(5)) {
                    String o = (output == null) ? null : "output=" + output;

                    NodeRoute nr = new NodeRoute(node, input, output);
                    String required = joinStrings(prefix, suffix, ",",
                                                  n, i, o);
                    assertEquals(required, nr.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link NodeRoute} is serializable.
     */
    @Test
    public void testSerialize() {
        for (Node node: createNodes(5)) {
            for (SwitchPort input: createSwitchPorts(5)) {
                for (SwitchPort output: createSwitchPorts(5)) {
                    NodeRoute nr = new NodeRoute(node, input, output);
                    serializeTest(nr);
                }
            }
        }
    }

    /**
     * Ensure that {@link NodeRoute} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (Node node: createNodes(5)) {
            for (SwitchPort input: createSwitchPorts(5)) {
                for (SwitchPort output: createSwitchPorts(5)) {
                    NodeRoute nr = new NodeRoute(node, input, output);
                    jaxbTest(nr, NodeRoute.class, XML_ROOT);
                }
            }
        }
    }

    /**
     * Ensure that {@link PathCost} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (Node node: createNodes(5)) {
            for (SwitchPort input: createSwitchPorts(5)) {
                for (SwitchPort output: createSwitchPorts(5)) {
                    NodeRoute nr = new NodeRoute(node, input, output);
                    jsonTest(nr, NodeRoute.class);
                }
            }
        }
    }
}
