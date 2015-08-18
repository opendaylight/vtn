/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlDataType;
import org.opendaylight.vtn.manager.XmlAttributeType;

/**
 * JUnit test for {@link SetIcmpCodeAction}.
 */
public class SetIcmpCodeActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetIcmpCodeAction} class.
     */
    private static final String  XML_ROOT = "seticmpcode";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link SetIcmpCodeAction} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "code", short.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] codes = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short code: codes) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            assertEquals(code, act.getCode());
        }
    }

    /**
     * Test case for {@link SetIcmpCodeAction#equals(Object)} and
     * {@link SetIcmpCodeAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] codes = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short code: codes) {
            SetIcmpCodeAction act1 = new SetIcmpCodeAction(code);
            SetIcmpCodeAction act2 = new SetIcmpCodeAction(code);
            testEquals(set, act1, act2);
        }

        assertEquals(codes.length, set.size());
    }

    /**
     * Test case for {@link SetIcmpCodeAction#toString()}.
     */
    @Test
    public void testToString() {
        short[] codes = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short code: codes) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            assertEquals("SetIcmpCodeAction[code=" + code + "]",
                         act.toString());
        }
    }

    /**
     * Ensure that {@link SetIcmpCodeAction} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] codes = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short code: codes) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetIcmpCodeAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        short[] codes = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short code: codes) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            jaxbTest(act, SetIcmpCodeAction.class, XML_ROOT);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(SetIcmpCodeAction.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link SetIcmpCodeAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        short[] codes = {Short.MIN_VALUE, 0, 1, 100, 255, Short.MAX_VALUE};
        for (short code: codes) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            jsonTest(act, SetIcmpCodeAction.class);
        }
    }
}
