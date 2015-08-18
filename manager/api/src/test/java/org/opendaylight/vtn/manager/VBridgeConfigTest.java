/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
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

/**
 * JUnit test for {@link VBridgeConfig}.
 */
public class VBridgeConfigTest extends TestBase {
    /**
     * Root XML element name associated with {@link VBridgeConfig} class.
     */
    private static final String  XML_ROOT = "vbridgeconf";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VBridgeConfig} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "ageInterval",
                                       Integer.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String desc: createStrings("description")) {
            for (Integer ival: createIntegers(-10, 20)) {
                VBridgeConfig[] configs = {
                    createVBridgeConfig(desc, ival),
                    createVBridgeConfigJAXB(desc, ival),
                };
                String emsg = "(input interval value)"
                    + ((ival == null) ? "null" : ival.intValue());
                for (VBridgeConfig bconf: configs) {
                    assertEquals(desc, bconf.getDescription());

                    if (ival == null || ival.intValue() < 0) {
                        assertEquals(emsg, -1, bconf.getAgeInterval());
                        assertNull(emsg, bconf.getAgeIntervalValue());
                    } else {
                        assertEquals(emsg, ival.intValue(),
                                     bconf.getAgeInterval());
                        assertEquals(emsg, ival, bconf.getAgeIntervalValue());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeConfig#equals(Object)} and
     * {@link VBridgeConfig#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> descs = createStrings("description");
        List<Integer> intervals = createIntegers(0, 20);
        for (String desc: descs) {
            for (Integer ival: intervals) {
                VBridgeConfig bc1 = createVBridgeConfig(desc, ival);
                VBridgeConfig bc2 = createVBridgeConfig(copy(desc), ival);
                testEquals(set, bc1, bc2);
            }
        }

        assertEquals(descs.size() * intervals.size(), set.size());
    }

    /**
     * Test case for {@link VBridgeConfig#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VBridgeConfig[";
        String suffix = "]";
        for (String desc: createStrings("description")) {
            for (Integer ival: createIntegers(-1, 20)) {
                VBridgeConfig bconf = createVBridgeConfig(desc, ival);
                String s = (desc == null) ? null : "desc=" + desc;
                String i = (ival == null || ival < 0) ? null : "ageInterval=" + ival;
                String required = joinStrings(prefix, suffix, ",", s, i);
                assertEquals(required, bconf.toString());
            }
        }
    }

    /**
     * Ensure that {@link VBridgeConfig} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String desc: createStrings("description")) {
            for (Integer ival: createIntegers(-1, 20)) {
                VBridgeConfig bconf = createVBridgeConfig(desc, ival);
                serializeTest(bconf);
            }
        }
    }

    /**
     * Ensure that {@link VBridgeConfig} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (String desc: createStrings("description")) {
            for (Integer ival: createIntegers(-1, 20)) {
                VBridgeConfig bconf = createVBridgeConfig(desc, ival);
                jaxbTest(bconf, VBridgeConfig.class, "vbridgeconf");
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(VBridgeConfig.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Construct a {@link VBridgeConfig} instance using JAXB.
     *
     * @param desc  A brief description about the vBridge.
     * @param age   MAC address aging interval.
     * @return  A {@link VBridgeConfig} instance.
     */
    private VBridgeConfig createVBridgeConfigJAXB(String desc, Integer age) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (desc != null) {
            builder.append(" description=\"").append(desc).append("\"");
        }
        if (age != null) {
            builder.append(" ageInterval=\"").append(age).append("\"");
        }

        String xml = builder.append(" />").toString();
        Unmarshaller um = createUnmarshaller(VBridgeConfig.class);
        VBridgeConfig bconf = null;
        try {
            bconf = unmarshal(um, xml, VBridgeConfig.class);
        } catch (Exception e) {
            unexpected(e);
        }

        return bconf;
    }
}
