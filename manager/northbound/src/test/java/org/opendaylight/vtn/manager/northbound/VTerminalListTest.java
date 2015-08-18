/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
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
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * JUnit test for {@link VTerminalList}.
 */
public class VTerminalListTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTerminalList} class.
     */
    private static final String  XML_ROOT = "vterminals";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        List<VTerminal> list = new ArrayList<VTerminal>();
        List<String> names = createStrings("name");
        List<String> descs = createStrings("desc");
        int nflts = 2;

        // null list.
        VTerminalList nullList = new VTerminalList(null);
        assertNull(nullList.getList());

        // Empty list should be treated as null list.
        VTerminalList emptyList = new VTerminalList(new ArrayList<VTerminal>());
        assertEquals(list, emptyList.getList());

        VnodeState[] states = VnodeState.values();
        for (String name: names) {
            for (String desc: descs) {
                for (int flt = 0; flt < nflts; flt++) {
                    for (VnodeState state: states) {
                        VTerminalConfig bconf = new VTerminalConfig(desc);
                        VTerminal b = new VTerminal(name, state, flt, bconf);

                        list.add(b);

                        List<VTerminal> l = new ArrayList<VTerminal>(list);
                        VTerminalList vb = new VTerminalList(l);
                        assertEquals(list, vb.getList());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalList#equals(Object)} and
     * {@link VTerminalList#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        // null list.
        VTerminalList nullList = new VTerminalList(null);
        testEquals(set, nullList, new VTerminalList(null));

        // Empty list should be treated as null list.
        VTerminalList emptyList = new VTerminalList(new ArrayList<VTerminal>());
        assertEquals(nullList, emptyList);
        assertEquals(nullList.hashCode(), emptyList.hashCode());
        assertFalse(set.add(emptyList));

        List<VTerminal> list1 = new ArrayList<VTerminal>();
        List<VTerminal> list2 = new ArrayList<VTerminal>();
        List<String> names = createStrings("name");
        List<String> descs = createStrings("desc");
        int nflts = 2;
        VnodeState[] states = VnodeState.values();
        for (String name: names) {
            for (String desc: descs) {
                for (int flt = 0; flt < nflts; flt++) {
                    for (VnodeState state: states) {
                        VTerminalConfig vtconf = new VTerminalConfig(desc);
                        VTerminal b1 = new VTerminal(name, state, flt, vtconf);
                        vtconf = new VTerminalConfig(copy(desc));
                        VTerminal b2 =
                            new VTerminal(copy(name), state, flt, vtconf);
                        list1.add(b1);
                        list2.add(b2);

                        List<VTerminal> l1 = new ArrayList<VTerminal>(list1);
                        List<VTerminal> l2 = new ArrayList<VTerminal>(list2);
                        VTerminalList vb1 = new VTerminalList(l1);
                        VTerminalList vb2 = new VTerminalList(l2);
                        testEquals(set, vb1, vb2);
                    }
                }
            }
        }

        int required = names.size() * descs.size() * nflts *
            states.length + 1;
        assertEquals(required, set.size());
    }

    /**
     * Ensure that {@link VTerminalList} is mapped to both XML root element
     * and JSON object.
     */
    @Test
    public void testJAXB() {
        // null list.
        VTerminalList vtList = new VTerminalList(null);
        jaxbTest(vtList, VTerminalList.class, XML_ROOT);
        jsonTest(vtList, VTerminalList.class);

        // Empty list.
        List<VTerminal> list = new ArrayList<VTerminal>();
        vtList = new VTerminalList(list);
        jaxbTest(vtList, VTerminalList.class, XML_ROOT);
        jsonTest(vtList, VTerminalList.class);

        VnodeState[] states = VnodeState.values();
        for (String name: createStrings("vTerminal")) {
            for (String desc: createStrings("Description")) {
                for (int flt = 0; flt < 2; flt++) {
                    for (VnodeState state: states) {
                        VTerminalConfig vtconf = new VTerminalConfig(desc);
                        VTerminal vterm =
                            new VTerminal(name, state, flt, vtconf);

                        // Single entry.
                        List<VTerminal> one = new ArrayList<VTerminal>();
                        one.add(vterm);
                        vtList = new VTerminalList(one);
                        jaxbTest(vtList, VTerminalList.class, XML_ROOT);
                        jsonTest(vtList, VTerminalList.class);

                        list.add(vterm);
                    }
                }
            }
        }

        vtList = new VTerminalList(list);
        jaxbTest(vtList, VTerminalList.class, XML_ROOT);
        jsonTest(vtList, VTerminalList.class);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(VTerminalList.class,
                      new XmlAttributeType("vterminal", "faults", int.class).
                      add(XML_ROOT),
                      new XmlAttributeType("vterminal", "state", int.class).
                      add(XML_ROOT));
    }
}
