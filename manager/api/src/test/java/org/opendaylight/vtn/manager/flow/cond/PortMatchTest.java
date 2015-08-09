/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlAttributeType;
import org.opendaylight.vtn.manager.XmlDataType;

/**
 * JUnit test for {@link PortMatch}.
 */
public class PortMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link PortMatch} class.
     */
    private static final String  XML_ROOT = "portmatch";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link PortMatch} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlAttributeType(name, "from", Integer.class).add(parent),
            new XmlAttributeType(name, "to", Integer.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        Integer[] fromPorts = {
            null, Integer.valueOf(0), Integer.valueOf(10),
            Integer.valueOf(30000), Integer.valueOf(65535),
        };
        Integer[] toPorts = {
            null, Integer.valueOf(1), Integer.valueOf(1230),
            Integer.valueOf(7938), Integer.valueOf(65534),
        };

        for (Integer from: fromPorts) {
            for (Integer to: toPorts) {
                PortMatch pm = new PortMatch(from, to);
                assertEquals(from, pm.getPortFrom());
                if (to == null) {
                    assertEquals(from, pm.getPortTo());
                } else {
                    assertEquals(to, pm.getPortTo());
                }

                pm = new PortMatch(from);
                assertEquals(from, pm.getPortFrom());
                assertEquals(from, pm.getPortTo());

                Short sf = (from == null)
                    ? null : Short.valueOf(from.shortValue());
                Short st = (to == null)
                    ? null : Short.valueOf(to.shortValue());

                try {
                    pm = new PortMatch(sf, st);
                    assertTrue(sf != null);
                    assertEquals(from, pm.getPortFrom());
                    if (st == null) {
                        assertEquals(from, pm.getPortTo());
                    } else {
                        assertEquals(to, pm.getPortTo());
                    }
                } catch (NullPointerException e) {
                    assertEquals(null, sf);
                }

                try {
                    pm = new PortMatch(sf);
                    assertTrue(sf != null);
                    assertEquals(from, pm.getPortFrom());
                    assertEquals(from, pm.getPortTo());
                } catch (NullPointerException e) {
                    assertEquals(null, sf);
                }
            }
        }
    }

    /**
     * Test case for {@link PortMatch#equals(Object)} and
     * {@link PortMatch#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        Integer[] fromPorts = {
            null, Integer.valueOf(0), Integer.valueOf(10),
            Integer.valueOf(30000), Integer.valueOf(65535),
        };
        Integer[] toPorts = {
            null, Integer.valueOf(1), Integer.valueOf(1230),
            Integer.valueOf(7938), Integer.valueOf(65534),
        };

        for (Integer from: fromPorts) {
            for (Integer to: toPorts) {
                PortMatch pm1 = new PortMatch(from, to);
                PortMatch pm2 = new PortMatch(copy(from), copy(to));
                testEquals(set, pm1, pm2);
            }
        }

        int required = fromPorts.length * toPorts.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link PortMatch#toString()}.
     */
    @Test
    public void testToString() {
        Integer[] fromPorts = {
            null, Integer.valueOf(0), Integer.valueOf(10),
            Integer.valueOf(30000), Integer.valueOf(65535),
        };
        Integer[] toPorts = {
            null, Integer.valueOf(1), Integer.valueOf(1230),
            Integer.valueOf(7938), Integer.valueOf(65534),
        };

        String prefix = "PortMatch[";
        String suffix = "]";
        for (Integer from: fromPorts) {
            for (Integer to: toPorts) {
                PortMatch pm = new PortMatch(from, to);
                String f = (from == null) ? null : "from=" + from;
                String t;
                if (to == null) {
                    t = (from == null) ? null : "to=" + from;
                } else {
                    t = "to=" + to;
                }
                String required = joinStrings(prefix, suffix, ",", f, t);
                assertEquals(required, pm.toString());
            }
        }
    }

    /**
     * Ensure that {@link PortMatch} is serializable.
     */
    @Test
    public void testSerialize() {
        Integer[] fromPorts = {
            null, Integer.valueOf(0), Integer.valueOf(10),
            Integer.valueOf(30000), Integer.valueOf(65535),
        };
        Integer[] toPorts = {
            null, Integer.valueOf(1), Integer.valueOf(1230),
            Integer.valueOf(7938), Integer.valueOf(65534),
        };

        for (Integer from: fromPorts) {
            for (Integer to: toPorts) {
                PortMatch pm = new PortMatch(from, to);
                serializeTest(pm);
            }
        }
    }

    /**
     * Ensure that {@link PortMatch} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        Integer[] fromPorts = {
            null, Integer.valueOf(0), Integer.valueOf(10),
            Integer.valueOf(30000), Integer.valueOf(65535),
        };
        Integer[] toPorts = {
            null, Integer.valueOf(1), Integer.valueOf(1230),
            Integer.valueOf(7938), Integer.valueOf(65534),
        };

        for (Integer from: fromPorts) {
            for (Integer to: toPorts) {
                PortMatch pm = new PortMatch(from, to);
                jaxbTest(pm, PortMatch.class, XML_ROOT);
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(PortMatch.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link PortMatch} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        Integer[] fromPorts = {
            null, Integer.valueOf(0), Integer.valueOf(10),
            Integer.valueOf(30000), Integer.valueOf(65535),
        };
        Integer[] toPorts = {
            null, Integer.valueOf(1), Integer.valueOf(1230),
            Integer.valueOf(7938), Integer.valueOf(65534),
        };

        for (Integer from: fromPorts) {
            for (Integer to: toPorts) {
                PortMatch pm = new PortMatch(from, to);
                jsonTest(pm, PortMatch.class);
            }
        }
    }
}
