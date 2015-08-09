/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlDataType;

/**
 * JUnit test for {@link UdpMatch}.
 */
public class UdpMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link UdpMatch} class.
     */
    private static final String  XML_ROOT = "udpmatch";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link UdpMatch} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        return TcpMatchTest.getXmlDataTypes(name, parent);
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                UdpMatch um = new UdpMatch(src, dst);
                assertEquals(src, um.getSourcePort());
                assertEquals(dst, um.getDestinationPort());
            }
        }

        Integer[] srcs = {
            null, Integer.valueOf(0), Integer.valueOf(12345),
            Integer.valueOf(65355),
        };
        Integer[] dsts = {
            null, Integer.valueOf(0), Integer.valueOf(9988),
            Integer.valueOf(65354),
        };
        for (Integer src: srcs) {
            for (Integer dst: dsts) {
                UdpMatch um = new UdpMatch(src, dst);
                assertEquals((src == null) ? null : new PortMatch(src),
                             um.getSourcePort());
                assertEquals((dst == null) ? null : new PortMatch(dst),
                             um.getDestinationPort());

                Short ss = (src == null)
                    ? null : Short.valueOf(src.shortValue());
                Short sd = (dst == null)
                    ? null : Short.valueOf(dst.shortValue());
                um = new UdpMatch(ss, sd);
                assertEquals((ss == null) ? null : new PortMatch(src),
                             um.getSourcePort());
                assertEquals((sd == null) ? null : new PortMatch(dst),
                             um.getDestinationPort());
            }
        }
    }

    /**
     * Test case for {@link UdpMatch#equals(Object)} and
     * {@link UdpMatch#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<PortMatch> srcs = createPortMatches();
        List<PortMatch> dsts = createPortMatches();
        for (PortMatch src: srcs) {
            for (PortMatch dst: dsts) {
                UdpMatch um1 = new UdpMatch(src, dst);
                UdpMatch um2 = new UdpMatch(copy(src), copy(dst));
                testEquals(set, um1, um2);

                TcpMatch tm = new TcpMatch(src, dst);
                assertFalse(um1.equals(tm));
            }
        }

        int required = srcs.size() * dsts.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link UdpMatch#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "UdpMatch[";
        String suffix = "]";
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                UdpMatch um = new UdpMatch(src, dst);
                String s = (src == null) ? null : "src=" + src;
                String d = (dst == null) ? null : "dst=" + dst;
                String required = joinStrings(prefix, suffix, ",", s, d);
                assertEquals(required, um.toString());
            }
        }
    }

    /**
     * Ensure that {@link UdpMatch} is serializable.
     */
    @Test
    public void testSerialize() {
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                UdpMatch um = new UdpMatch(src, dst);
                serializeTest(um);
            }
        }
    }

    /**
     * Ensure that {@link UdpMatch} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                UdpMatch um = new UdpMatch(src, dst);
                jaxbTest(um, UdpMatch.class, XML_ROOT);
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(UdpMatch.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link UdpMatch} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                UdpMatch um = new UdpMatch(src, dst);
                jsonTest(um, UdpMatch.class);
            }
        }
    }
}
