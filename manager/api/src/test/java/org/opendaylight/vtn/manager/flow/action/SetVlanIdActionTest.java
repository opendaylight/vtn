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
 * JUnit test for {@link SetVlanIdAction}.
 */
public class SetVlanIdActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetVlanIdAction} class.
     */
    private static final String  XML_ROOT = "setvlanid";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link SetVlanIdAction} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "vlan", short.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            assertEquals(vlan, act.getVlan());
        }
    }

    /**
     * Test case for {@link SetVlanIdAction#equals(Object)} and
     * {@link SetVlanIdAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act1 = new SetVlanIdAction(vlan);
            SetVlanIdAction act2 = new SetVlanIdAction(vlan);
            testEquals(set, act1, act2);
        }

        assertEquals(vlans.length, set.size());
    }

    /**
     * Test case for {@link SetVlanIdAction#toString()}.
     */
    @Test
    public void testToString() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            assertEquals("SetVlanIdAction[vlan=" + vlan + "]",
                         act.toString());
        }
    }

    /**
     * Ensure that {@link SetVlanIdAction} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetVlanIdAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            jaxbTest(act, SetVlanIdAction.class, XML_ROOT);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(SetVlanIdAction.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link SetVlanIdAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        short[] vlans = {Short.MIN_VALUE, 0, 1, 100, 4095, Short.MAX_VALUE};
        for (short vlan: vlans) {
            SetVlanIdAction act = new SetVlanIdAction(vlan);
            jsonTest(act, SetVlanIdAction.class);
        }
    }
}
