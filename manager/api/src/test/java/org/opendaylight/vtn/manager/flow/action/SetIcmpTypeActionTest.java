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
 * JUnit test for {@link SetIcmpTypeAction}.
 */
public class SetIcmpTypeActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetIcmpTypeAction} class.
     */
    private static final String  XML_ROOT = "seticmptype";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link SetIcmpTypeAction} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "type", short.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            assertEquals(type, act.getType());
        }
    }

    /**
     * Test case for {@link SetIcmpTypeAction#equals(Object)} and
     * {@link SetIcmpTypeAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act1 = new SetIcmpTypeAction(type);
            SetIcmpTypeAction act2 = new SetIcmpTypeAction(type);
            testEquals(set, act1, act2);
        }

        assertEquals(types.length, set.size());
    }

    /**
     * Test case for {@link SetIcmpTypeAction#toString()}.
     */
    @Test
    public void testToString() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            assertEquals("SetIcmpTypeAction[type=" + type + "]",
                         act.toString());
        }
    }

    /**
     * Ensure that {@link SetIcmpTypeAction} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetIcmpTypeAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            jaxbTest(act, SetIcmpTypeAction.class, XML_ROOT);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(SetIcmpTypeAction.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link SetIcmpTypeAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        short[] types = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short type: types) {
            SetIcmpTypeAction act = new SetIcmpTypeAction(type);
            jsonTest(act, SetIcmpTypeAction.class);
        }
    }
}
