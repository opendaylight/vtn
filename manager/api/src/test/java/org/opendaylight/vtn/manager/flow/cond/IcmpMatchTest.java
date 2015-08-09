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
 * JUnit test for {@link IcmpMatch}.
 */
public class IcmpMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link IcmpMatch} class.
     */
    private static final String  XML_ROOT = "icmpmatch";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link IcmpMatch} instance.
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
            new XmlAttributeType(name, "type", Short.class).add(parent),
            new XmlAttributeType(name, "code", Short.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        Short[] types = {
            null, Short.valueOf((short)0), Short.valueOf((short)12),
            Short.valueOf((short)135), Short.valueOf((short)255),
        };
        Short[] codes = {
            null, Short.valueOf((short)0), Short.valueOf((short)29),
            Short.valueOf((short)143), Short.valueOf((short)255),
        };
        for (Short type: types) {
            for (Short code: codes) {
                IcmpMatch icm = new IcmpMatch(type, code);
                assertEquals(type, icm.getType());
                assertEquals(code, icm.getCode());
            }
        }
    }

    /**
     * Test case for {@link IcmpMatch#equals(Object)} and
     * {@link IcmpMatch#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        Short[] types = {
            null, Short.valueOf((short)0), Short.valueOf((short)12),
            Short.valueOf((short)135), Short.valueOf((short)255),
        };
        Short[] codes = {
            null, Short.valueOf((short)0), Short.valueOf((short)29),
            Short.valueOf((short)143), Short.valueOf((short)255),
        };

        for (Short type: types) {
            for (Short code: codes) {
                IcmpMatch icm1 = new IcmpMatch(type, code);
                IcmpMatch icm2 = new IcmpMatch(copy(type), copy(code));
                testEquals(set, icm1, icm2);
            }
        }

        int required = types.length * codes.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link IcmpMatch#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "IcmpMatch[";
        String suffix = "]";

        Short[] types = {
            null, Short.valueOf((short)0), Short.valueOf((short)12),
            Short.valueOf((short)135), Short.valueOf((short)255),
        };
        Short[] codes = {
            null, Short.valueOf((short)0), Short.valueOf((short)29),
            Short.valueOf((short)143), Short.valueOf((short)255),
        };

        for (Short type: types) {
            for (Short code: codes) {
                IcmpMatch icm = new IcmpMatch(type, code);
                String t = (type == null) ? null : "type=" + type;
                String c = (code == null) ? null : "code=" + code;
                String required = joinStrings(prefix, suffix, ",", t, c);
                assertEquals(required, icm.toString());
            }
        }
    }

    /**
     * Ensure that {@link IcmpMatch} is serializable.
     */
    @Test
    public void testSerialize() {
        Short[] types = {
            null, Short.valueOf((short)0), Short.valueOf((short)12),
            Short.valueOf((short)135), Short.valueOf((short)255),
        };
        Short[] codes = {
            null, Short.valueOf((short)0), Short.valueOf((short)29),
            Short.valueOf((short)143), Short.valueOf((short)255),
        };

        for (Short type: types) {
            for (Short code: codes) {
                IcmpMatch icm = new IcmpMatch(type, code);
                serializeTest(icm);
            }
        }
    }

    /**
     * Ensure that {@link IcmpMatch} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        Short[] types = {
            null, Short.valueOf((short)0), Short.valueOf((short)12),
            Short.valueOf((short)135), Short.valueOf((short)255),
        };
        Short[] codes = {
            null, Short.valueOf((short)0), Short.valueOf((short)29),
            Short.valueOf((short)143), Short.valueOf((short)255),
        };

        for (Short type: types) {
            for (Short code: codes) {
                IcmpMatch icm = new IcmpMatch(type, code);
                jaxbTest(icm, IcmpMatch.class, XML_ROOT);
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(IcmpMatch.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link IcmpMatch} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        Short[] types = {
            null, Short.valueOf((short)0), Short.valueOf((short)12),
            Short.valueOf((short)135), Short.valueOf((short)255),
        };
        Short[] codes = {
            null, Short.valueOf((short)0), Short.valueOf((short)29),
            Short.valueOf((short)143), Short.valueOf((short)255),
        };

        for (Short type: types) {
            for (Short code: codes) {
                IcmpMatch icm = new IcmpMatch(type, code);
                jsonTest(icm, IcmpMatch.class);
            }
        }
    }
}
