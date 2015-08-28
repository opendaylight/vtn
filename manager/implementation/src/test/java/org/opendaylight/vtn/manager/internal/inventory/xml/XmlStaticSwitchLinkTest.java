/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory.xml;

import java.util.HashSet;
import java.util.Set;

import javax.xml.bind.JAXBException;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link XmlStaticSwitchLink}.
 */
public class XmlStaticSwitchLinkTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlStaticSwitchLink} class.
     */
    private static final String  XML_ROOT = "static-switch-link";

    /**
     * Create a {@link XmlNode} instance which represents an instance of
     * {@link XmlStaticSwitchLink}.
     *
     * @param src  The source switch port of the link.
     * @param dst  The destination switch port of the link.
     * @return  A {@link XmlNode} instance.
     */
    public static XmlNode createXmlNode(String src, String dst) {
        return createXmlNode(XML_ROOT, src, dst);
    }

    /**
     * Create a {@link XmlNode} instance which represents an instance of
     * {@link XmlStaticSwitchLink}.
     *
     * @param name  The name of the root node.
     * @param src   The source switch port of the link.
     * @param dst   The destination switch port of the link.
     * @return  A {@link XmlNode} instance.
     */
    public static XmlNode createXmlNode(String name, String src, String dst) {
        XmlNode root = new XmlNode(name);
        if (src != null) {
            root.add(new XmlNode("source", src));
        }
        if (dst != null) {
            root.add(new XmlNode("destination", dst));
        }

        return root;
    }

    /**
     * Construct an instance of {@link XmlStaticSwitchLink} using JAXB.
     *
     * @param src  The source switch port of the link.
     * @param dst  The destination switch port of the link.
     * @return  A {@link XmlStaticSwitchLink} instance.
     * @throws JAXBException
     *    Failed to unmarshal XML.
     */
    public static XmlStaticSwitchLink newXmlStaticSwitchLink(
        String src, String dst) throws JAXBException {
        XmlNode root = createXmlNode(src, dst);
        Class<XmlStaticSwitchLink> type = XmlStaticSwitchLink.class;
        Unmarshaller um = createUnmarshaller(type);
        XmlStaticSwitchLink xlink = unmarshal(um, root.toString(), type);
        assertEquals(src, xlink.getSource());
        assertEquals(dst, xlink.getDestination());

        return xlink;
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>
     *     {@link XmlStaticSwitchLink#XmlStaticSwitchLink(StaticSwitchLink)}
     *   </li>
     *   <li>{@link XmlStaticSwitchLink#getSource()}</li>
     *   <li>{@link XmlStaticSwitchLink#getDestination()}</li>
     *   <li>{@link XmlStaticSwitchLink#toStaticSwitchLink()}</li>
     * </ul>
     */
    @Test
    public void testConstructor() {
        String[] sources = {
            "openflow:1:2", "openflow:3:4", "unknown:1:2", "unknown:3:4",
        };
        String[] destinations = {
            "openflow:10:20", "openflow:5:6", "unknown:5:6", "unknown:7:8",
        };

        for (String src: sources) {
            for (String dst: destinations) {
                StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
                    setSource(new NodeConnectorId(src)).
                    setDestination(new NodeConnectorId(dst)).
                    build();
                XmlStaticSwitchLink xlink = new XmlStaticSwitchLink(swlink);
                assertEquals(src, xlink.getSource());
                assertEquals(dst, xlink.getDestination());
                assertEquals(swlink, xlink.toStaticSwitchLink());
            }
        }

        // Specifying null node-connector-id.
        NodeConnectorId ncId = new NodeConnectorId("openflow:1:2");
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setDestination(ncId).build();
        try {
            new XmlStaticSwitchLink(swlink);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Source port cannot be null.", e.getMessage());
        }

        swlink = new StaticSwitchLinkBuilder().setSource(ncId).build();
        try {
            new XmlStaticSwitchLink(swlink);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Destination port cannot be null.", e.getMessage());
        }

        // Specifying empty port ID.
        NodeConnectorId empty = new NodeConnectorId("");
        swlink = new StaticSwitchLinkBuilder().
            setSource(empty).setDestination(ncId).build();
        try {
            new XmlStaticSwitchLink(swlink);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Source port is invalid: ", e.getMessage());
        }

        // Specifying empty port ID.
        swlink = new StaticSwitchLinkBuilder().
            setSource(ncId).setDestination(empty).build();
        try {
            new XmlStaticSwitchLink(swlink);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Destination port is invalid: ", e.getMessage());
        }

        // Specifying the same destination as source.
        for (String src: sources) {
            swlink = new StaticSwitchLinkBuilder().
                setSource(new NodeConnectorId(src)).
                setDestination(new NodeConnectorId(src)).
                build();
            try {
                new XmlStaticSwitchLink(swlink);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Destination must not be the same as source.",
                             e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link XmlStaticSwitchLink#isValidPortId(String)}.
     */
    @Test
    public void testIsValidPortId() {
        String[] ports = {
            "openflow:1:2", "openflow:1:3", "openflow:2:1", "openflow:2:2",
            "unknown:1", "unknown-1:1",
        };
        for (String port: ports) {
            assertEquals(true, XmlStaticSwitchLink.isValidPortId(port));
        }

        assertEquals(false, XmlStaticSwitchLink.isValidPortId(null));
        assertEquals(false, XmlStaticSwitchLink.isValidPortId(""));
    }

    /**
     * Test case for {@link XmlStaticSwitchLink#equals(Object)} and
     * {@link XmlStaticSwitchLink#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        String[] sources = {
            null, "openflow:1:2", "openflow:3:4", "unknown:1:2", "unknown:3:4",
        };
        String[] destinations = {
            null, "openflow:4:5", "openflow:5:6", "unknown:5:6", "unknown:7:8",
        };

        for (String src: sources) {
            for (String dst: destinations) {
                XmlStaticSwitchLink xlink1 = newXmlStaticSwitchLink(src, dst);
                XmlStaticSwitchLink xlink2 = newXmlStaticSwitchLink(src, dst);
                testEquals(set, xlink1, xlink2);
            }
        }

        assertEquals(sources.length * destinations.length, set.size());
    }

    /**
     * Test case for {@link XmlStaticSwitchLink#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        String[] sources = {
            null, "openflow:1:2", "openflow:3:4", "unknown:1:2", "unknown:3:4",
        };
        String[] destinations = {
            null, "openflow:4:5", "openflow:5:6", "unknown:5:6", "unknown:7:8",
        };

        for (String src: sources) {
            for (String dst: destinations) {
                XmlStaticSwitchLink xlink = newXmlStaticSwitchLink(src, dst);
                String expected = "static-switch-link[src=" + src +
                    ", dst=" + dst + "]";
                assertEquals(expected, xlink.toString());
            }
        }
    }

    /**
     * Test case for {@link XmlStaticSwitchLink#isValid()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testIsValid() throws Exception {
        String[] sources = {
            "openflow:1:2", "openflow:3:4", "unknown:1:2", "unknown:3:4",
        };
        String[] destinations = {
            "openflow:10:20", "openflow:5:6", "unknown:5:6", "unknown:7:8",
        };

        for (String src: sources) {
            for (String dst: destinations) {
                StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
                    setSource(new NodeConnectorId(src)).
                    setDestination(new NodeConnectorId(dst)).
                    build();
                XmlStaticSwitchLink xlink = new XmlStaticSwitchLink(swlink);
                assertEquals(true, xlink.isValid());
            }
        }

        // Empty switch link.
        XmlStaticSwitchLink xlink = newXmlStaticSwitchLink(null, null);
        assertEquals(false, xlink.isValid());

        // Source is null.
        String portId = "openflow:1:2";
        xlink = newXmlStaticSwitchLink(null, portId);
        assertEquals(false, xlink.isValid());

        // Source is empty.
        String empty = "";
        xlink = newXmlStaticSwitchLink(empty, portId);
        assertEquals(false, xlink.isValid());

        // Destination is null.
        xlink = newXmlStaticSwitchLink(portId, null);
        assertEquals(false, xlink.isValid());

        // Destination is empty.
        xlink = newXmlStaticSwitchLink(portId, empty);
        assertEquals(false, xlink.isValid());

        // Destination is the same as source.
        for (String src: sources) {
            xlink = newXmlStaticSwitchLink(src, src);
            assertEquals(false, xlink.isValid());
        }
        for (String dst: destinations) {
            xlink = newXmlStaticSwitchLink(dst, dst);
            assertEquals(false, xlink.isValid());
        }
    }

    /**
     * Test case for JAXB mapping.
     */
    @Test
    public void testJAXB() {
        String[] sources = {
            "openflow:1:2", "openflow:3:4", "unknown:1:2", "unknown:3:4",
        };
        String[] destinations = {
            "openflow:10:20", "openflow:5:6", "unknown:5:6", "unknown:7:8",
        };

        for (String src: sources) {
            for (String dst: destinations) {
                StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
                    setSource(new NodeConnectorId(src)).
                    setDestination(new NodeConnectorId(dst)).
                    build();
                XmlStaticSwitchLink xlink = new XmlStaticSwitchLink(swlink);
                jaxbTest(xlink, XmlStaticSwitchLink.class, XML_ROOT);
            }
        }
    }
}
