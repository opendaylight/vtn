/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

/**
 * JUnit test for {@link XmlLongIntegerList}.
 */
public class XmlLongIntegerListTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlLongIntegerList} class.
     */
    private static final String  XML_ROOT = "integers";

    /**
     * Test case for all instance methods.
     */
    @Test
    public void testAll() {
        XmlLongIntegerList nil = new XmlLongIntegerList(null);
        assertEquals(null, nil.getIntegers());
        XmlLongIntegerList empty =
            new XmlLongIntegerList(Collections.<Integer>emptyList());
        assertEquals(0, empty.getIntegers().size());

        HashSet<Object> set = new HashSet<>();
        testEquals(set, nil, new XmlLongIntegerList(null));
        assertEquals(false, set.add(empty));
        int count = 1;

        List<Number> list = new ArrayList<>();
        List<XmlLongInteger> expected = new ArrayList<>();
        list.add((byte)1);
        expected.add(new XmlLongInteger(1));
        XmlLongIntegerList xll = new XmlLongIntegerList(list);
        assertEquals(expected, xll.getIntegers());
        testEquals(set, xll, new XmlLongIntegerList(list));
        count++;

        list.add(Short.MIN_VALUE);
        expected.add(new XmlLongInteger((long)Short.MIN_VALUE));
        xll = new XmlLongIntegerList(list);
        assertEquals(expected, xll.getIntegers());
        testEquals(set, xll, new XmlLongIntegerList(list));
        count++;

        list.add(Long.MAX_VALUE);
        expected.add(new XmlLongInteger(Long.MAX_VALUE));
        xll = new XmlLongIntegerList(list);
        assertEquals(expected, xll.getIntegers());
        testEquals(set, xll, new XmlLongIntegerList(list));
        count++;

        assertEquals(count, set.size());
    }

    /**
     * Ensure that {@link XmlLongIntegerList} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        XmlLongIntegerList xll = new XmlLongIntegerList(null);
        jaxbTest(xll, XmlLongIntegerList.class, XML_ROOT);
        jsonTest(xll, XmlLongIntegerList.class);
        xll = new XmlLongIntegerList(Collections.<Long>emptyList());
        jaxbTest(xll, XmlLongIntegerList.class, XML_ROOT);
        jsonTest(xll, XmlLongIntegerList.class);

        List<Long> list = new ArrayList<>();
        long[] longValues = {
            Long.MIN_VALUE, -0x7000000000000000L, -0x100000000L,
            -55555555, -444444, -3333, -2, -1, 0,
            1, 2, 3333333, 444444444, 0xffffffffL, 0x100000000L,
            0xfffffffffffffffL, Long.MAX_VALUE,
        };
        for (long lv: longValues) {
            list.add(lv);
            xll = new XmlLongIntegerList(Collections.<Long>emptyList());
            jaxbTest(xll, XmlLongIntegerList.class, XML_ROOT);
            jsonTest(xll, XmlLongIntegerList.class);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(XmlLongIntegerList.class,
                      XmlLongIntegerTest.getXmlDataTypes("integer", XML_ROOT));
    }
}
