/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.controller.sal.core.Node;

/**
 * JUnit test for {@link PathCost}.
 */
public class PathCostTest extends TestBase {
    /**
     * Root XML element name associated with {@link PathCost} class.
     */
    private static final String  XML_ROOT = "pathcost";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link PathCost} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "cost", Long.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        PathCost pc = createUndefiedCost(null);
        assertEquals(null, pc.getLocation());
        assertEquals(null, pc.getCost());

        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(5)) {
                PortLocation ploc = new PortLocation(node, port);
                pc = createUndefiedCost(ploc);
                assertEquals(ploc, pc.getLocation());
                assertEquals(null, pc.getCost());
                for (long c = 0; c <= 5; c++) {
                    pc = new PathCost(ploc, c);
                    assertEquals(ploc, pc.getLocation());
                    assertEquals(Long.valueOf(c), pc.getCost());
                }
            }
        }

        for (long c = 0; c <= 5; c++) {
            pc = new PathCost(null, c);
            assertEquals(null, pc.getLocation());
            assertEquals(Long.valueOf(c), pc.getCost());
        }
    }

    /**
     * Test case for {@link PathCost#equals(Object)} and
     * {@link PathCost#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        PathCost pc1 = createUndefiedCost(null);
        PathCost pc2 = createUndefiedCost(null);
        testEquals(set, pc1, pc2);

        List<Node> nodes = createNodes(3);
        List<SwitchPort> ports = createSwitchPorts(5);
        final long ncosts = 3;
        for (Node node: nodes) {
            for (SwitchPort port: ports) {
                PortLocation ploc1 = new PortLocation(node, port);
                PortLocation ploc2 = new PortLocation(copy(node), copy(port));
                pc1 = createUndefiedCost(ploc1);
                pc2 = createUndefiedCost(ploc2);
                testEquals(set, pc1, pc2);
                for (long c = 0; c < ncosts; c++) {
                    pc1 = new PathCost(ploc1, c);
                    pc2 = new PathCost(ploc2, c);
                    testEquals(set, pc1, pc2);
                }
            }
        }

        for (long c = 0; c < ncosts; c++) {
            pc1 = new PathCost(null, c);
            pc2 = new PathCost(null, c);
            testEquals(set, pc1, pc2);
        }

        int required = (nodes.size() * ports.size() * ((int)ncosts + 1)) +
            (int)ncosts + 1;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link PathCost#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "PathCost[";
        String suffix = "]";
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(5)) {
                PortLocation ploc = new PortLocation(node, port);
                String l = "location=" + ploc;
                PathCost pc = createUndefiedCost(ploc);
                String required = joinStrings(prefix, suffix, ",", l, null);
                assertEquals(required, pc.toString());

                for (long c = 0; c <= 5; c++) {
                    String sc = "cost=" + c;
                    pc = new PathCost(ploc, c);
                    required = joinStrings(prefix, suffix, ",", l, sc);
                    assertEquals(required, pc.toString());
                }
            }
        }

        for (long c = 0; c <= 5; c++) {
            PathCost pc = new PathCost(null, c);
            String sc = "cost=" + c;
            String required = joinStrings(prefix, suffix, ",", null, sc);
            assertEquals(required, pc.toString());
        }

        PathCost pc = createUndefiedCost(null);
        assertEquals("PathCost[]", pc.toString());
    }

    /**
     * Ensure that {@link PathCost} is serializable.
     */
    @Test
    public void testSerialize() {
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(5)) {
                PortLocation ploc = new PortLocation(node, port);
                PathCost pc = createUndefiedCost(ploc);
                serializeTest(pc);

                for (long c = 0; c <= 5; c++) {
                    pc = new PathCost(ploc, c);
                    serializeTest(pc);
                }
            }
        }

        for (long c = 0; c <= 5; c++) {
            PathCost pc = new PathCost(null, c);
            serializeTest(pc);
        }

        PathCost pc = createUndefiedCost(null);
        serializeTest(pc);
    }

    /**
     * Ensure that {@link PathCost} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(5)) {
                PortLocation ploc = new PortLocation(node, port);
                PathCost pc = createUndefiedCost(ploc);
                jaxbTest(pc, PathCost.class, XML_ROOT);

                for (long c = 0; c <= 5; c++) {
                    pc = new PathCost(ploc, c);
                    jaxbTest(pc, PathCost.class, XML_ROOT);
                }
            }
        }

        for (long c = 0; c <= 5; c++) {
            PathCost pc = new PathCost(null, c);
            jaxbTest(pc, PathCost.class, XML_ROOT);
        }

        PathCost pc = createUndefiedCost(null);
        jaxbTest(pc, PathCost.class, XML_ROOT);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(PathCost.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link PathCost} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (Node node: createNodes(5)) {
            for (SwitchPort port: createSwitchPorts(5)) {
                PortLocation ploc = new PortLocation(node, port);
                PathCost pc = createUndefiedCost(ploc);
                jsonTest(pc, PathCost.class);

                for (long c = 0; c <= 5; c++) {
                    pc = new PathCost(ploc, c);
                    jsonTest(pc, PathCost.class);
                }
            }
        }

        for (long c = 0; c <= 5; c++) {
            PathCost pc = new PathCost(null, c);
            jsonTest(pc, PathCost.class);
        }

        PathCost pc = createUndefiedCost(null);
        jsonTest(pc, PathCost.class);
    }

    /**
     * Create a {@link PathCost} instance that has no cost information.
     *
     * @param ploc  A {@link PortLocation} instance.
     * @return  A {@link PathCost} instance.
     */
    private PathCost createUndefiedCost(PortLocation ploc) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (ploc == null) {
            builder.append(" />");
        } else {
            builder.append("><location>");
            Node node = ploc.getNode();
            if (node != null) {
                builder.append("<node>");
                String type = node.getType();
                if (type != null) {
                    builder.append("<type>").append(type).append("</type>");
                }
                String id = node.getNodeIDString();
                if (id != null) {
                    builder.append("<id>").append(id).append("</id>");
                }
                builder.append("</node>");
            }

            SwitchPort port = ploc.getPort();
            if (port != null) {
                builder.append("<port");
                String name = port.getName();
                if (name != null) {
                    builder.append(" name=\"").append(name).append("\"");
                }
                String type = port.getType();
                if (type != null) {
                    builder.append(" type=\"").append(type).append("\"");
                }
                String id = port.getId();
                if (id != null) {
                    builder.append(" id=\"").append(id).append("\"");
                }
                builder.append(" />");
            }
            builder.append("</location></pathcost>");
        }

        PathCost pc = null;
        Unmarshaller um = createUnmarshaller(PathCost.class);
        String xml = builder.toString();
        try {
            pc = unmarshal(um, xml, PathCost.class);
        } catch (Exception e) {
            unexpected(e);
        }

        return pc;
    }
}
