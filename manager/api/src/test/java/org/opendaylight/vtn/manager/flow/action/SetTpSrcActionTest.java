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
 * JUnit test for {@link SetTpSrcAction}.
 */
public class SetTpSrcActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetTpSrcAction} class.
     */
    private static final String  XML_ROOT = "settpsrc";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link SetTpSrcAction} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "port", int.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            assertEquals(port, act.getPort());
        }
    }

    /**
     * Test case for {@link SetTpSrcAction#equals(Object)} and
     * {@link SetTpSrcAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpSrcAction act1 = new SetTpSrcAction(port);
            SetTpSrcAction act2 = new SetTpSrcAction(port);
            testEquals(set, act1, act2);
        }

        assertEquals(ports.length, set.size());
    }

    /**
     * Test case for {@link SetTpSrcAction#toString()}.
     */
    @Test
    public void testToString() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            assertEquals("SetTpSrcAction[port=" + port + "]",
                         act.toString());
        }
    }

    /**
     * Ensure that {@link SetTpSrcAction} is serializable.
     */
    @Test
    public void testSerialize() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetTpSrcAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            jaxbTest(act, SetTpSrcAction.class, XML_ROOT);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(SetTpSrcAction.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link SetTpSrcAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            jsonTest(act, SetTpSrcAction.class);
        }
    }
}
