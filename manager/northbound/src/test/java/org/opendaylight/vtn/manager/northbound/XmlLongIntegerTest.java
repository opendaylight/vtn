/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

/**
 * JUnit test for {@link XmlLongInteger}.
 */
public class XmlLongIntegerTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlLongInteger} class.
     */
    private static final String  XML_ROOT = "integer";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link XmlLongInteger} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlValueType(name, Long.class).add(parent));
        return dlist;
    }

    /**
     * Test case for all instance methods.
     */
    @Test
    public void testAll() {
        HashSet<Object> set = new HashSet<>();
        long[] longValues = {
            Long.MIN_VALUE, -0x7000000000000000L, -0x100000000L,
            -55555555, -444444, -3333, -2, -1, 0,
            1, 2, 3333333, 444444444, 0xffffffffL, 0x100000000L,
            0xfffffffffffffffL, Long.MAX_VALUE,
        };
        for (long lv: longValues) {
            Long v = Long.valueOf(lv);
            XmlLongInteger xl = new XmlLongInteger(lv);
            assertEquals(v, xl.getValue());
            testEquals(set, xl, new XmlLongInteger(lv));

            xl = new XmlLongInteger((int)lv);
            assertEquals(Long.valueOf((long)v.intValue()), xl.getValue());
        }

        assertEquals(longValues.length, set.size());
    }

    /**
     * Ensure that {@link XmlLongInteger} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        long[] longValues = {
            Long.MIN_VALUE, -0x7000000000000000L, -0x100000000L,
            -55555555, -444444, -3333, -2, -1, 0,
            1, 2, 3333333, 444444444, 0xffffffffL, 0x100000000L,
            0xfffffffffffffffL, Long.MAX_VALUE,
        };
        for (long lv: longValues) {
            XmlLongInteger xl = new XmlLongInteger(lv);
            jaxbTest(xl, XmlLongInteger.class, XML_ROOT);
            jsonTest(xl, XmlLongInteger.class);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(XmlLongInteger.class, getXmlDataTypes(XML_ROOT));
    }
}
