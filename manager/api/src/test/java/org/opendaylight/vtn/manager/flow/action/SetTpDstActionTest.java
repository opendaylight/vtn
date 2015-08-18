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
 * JUnit test for {@link SetTpDstAction}.
 */
public class SetTpDstActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetTpDstAction} class.
     */
    private static final String  XML_ROOT = "settpdst";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link SetTpDstAction} instance.
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
            SetTpDstAction act = new SetTpDstAction(port);
            assertEquals(port, act.getPort());
        }
    }

    /**
     * Test case for {@link SetTpDstAction#equals(Object)} and
     * {@link SetTpDstAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpDstAction act1 = new SetTpDstAction(port);
            SetTpDstAction act2 = new SetTpDstAction(port);
            testEquals(set, act1, act2);
        }

        assertEquals(ports.length, set.size());
    }

    /**
     * Test case for {@link SetTpDstAction#toString()}.
     */
    @Test
    public void testToString() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpDstAction act = new SetTpDstAction(port);
            assertEquals("SetTpDstAction[port=" + port + "]",
                         act.toString());
        }
    }

    /**
     * Ensure that {@link SetTpDstAction} is serializable.
     */
    @Test
    public void testSerialize() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpDstAction act = new SetTpDstAction(port);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetTpDstAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpDstAction act = new SetTpDstAction(port);
            jaxbTest(act, SetTpDstAction.class, XML_ROOT);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(SetTpSrcAction.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link SetTpDstAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        int[] ports = {Integer.MIN_VALUE, 0, 1, 100, 65535, Integer.MAX_VALUE};
        for (int port: ports) {
            SetTpDstAction act = new SetTpDstAction(port);
            jsonTest(act, SetTpDstAction.class);
        }
    }
}
