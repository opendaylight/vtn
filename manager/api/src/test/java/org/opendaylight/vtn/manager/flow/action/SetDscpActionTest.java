/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlDataType;
import org.opendaylight.vtn.manager.XmlAttributeType;

/**
 * JUnit test for {@link SetDscpAction}.
 */
public class SetDscpActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetDscpAction} class.
     */
    private static final String  XML_ROOT = "setdscp";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link SetDscpAction} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "dscp", byte.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            assertEquals(b, act.getDscp());
        }
    }

    /**
     * Test case for {@link SetDscpAction#equals(Object)} and
     * {@link SetDscpAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act1 = new SetDscpAction(b);
            SetDscpAction act2 = new SetDscpAction(b);
            testEquals(set, act1, act2);
        }

        assertEquals(bytes.length, set.size());
    }

    /**
     * Test case for {@link SetDscpAction#toString()}.
     */
    @Test
    public void testToString() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            assertEquals("SetDscpAction[dscp=" + b + "]", act.toString());
        }
    }

    /**
     * Ensure that {@link SetDscpAction} is serializable.
     */
    @Test
    public void testSerialize() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetDscpAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            jaxbTest(act, SetDscpAction.class, XML_ROOT);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(SetDscpAction.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link SetDscpAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        byte[] bytes = {Byte.MIN_VALUE, 0, 1, 30, 63, Byte.MAX_VALUE};
        for (byte b: bytes) {
            SetDscpAction act = new SetDscpAction(b);
            jsonTest(act, SetDscpAction.class);
        }
    }
}
