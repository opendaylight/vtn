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
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * JUnit test for {@link VBridgeList}.
 */
public class VBridgeListTest extends TestBase {
    /**
     * Root XML element name associated with {@link VBridgeList} class.
     */
    private static final String  XML_ROOT = "vbridges";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        List<VBridge> list = new ArrayList<VBridge>();
        List<String> names = createStrings("name");
        List<String> descs = createStrings("desc");
        int nflts = 2;

        // null list.
        VBridgeList nullList = new VBridgeList(null);
        assertNull(nullList.getList());

        // Empty list should be treated as null list.
        VBridgeList emptyList = new VBridgeList(new ArrayList<VBridge>());
        assertEquals(list, emptyList.getList());

        VnodeState[] states = VnodeState.values();
        for (String name: names) {
            for (String desc: descs) {
                for (int flt = 0; flt < nflts; flt++) {
                    for (VnodeState state: states) {
                        VBridgeConfig bconf = new VBridgeConfig(desc);
                        VBridge b = new VBridge(name, state, flt, bconf);

                        list.add(b);

                        List<VBridge> l = new ArrayList<VBridge>(list);
                        VBridgeList vb = new VBridgeList(l);
                        assertEquals(list, vb.getList());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeList#equals(Object)} and
     * {@link VBridgeList#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        // null list.
        VBridgeList nullList = new VBridgeList(null);
        testEquals(set, nullList, new VBridgeList(null));

        // Empty list should be treated as null list.
        VBridgeList emptyList = new VBridgeList(new ArrayList<VBridge>());
        assertEquals(nullList, emptyList);
        assertEquals(nullList.hashCode(), emptyList.hashCode());
        assertFalse(set.add(emptyList));

        List<VBridge> list1 = new ArrayList<VBridge>();
        List<VBridge> list2 = new ArrayList<VBridge>();
        List<String> names = createStrings("name");
        List<String> descs = createStrings("desc");
        int nflts = 2;
        VnodeState[] states = VnodeState.values();
        for (String name: names) {
            for (String desc: descs) {
                for (int flt = 0; flt < nflts; flt++) {
                    for (VnodeState state: states) {
                        VBridgeConfig bconf = new VBridgeConfig(desc);
                        VBridge b1 = new VBridge(name, state, flt, bconf);
                        bconf = new VBridgeConfig(copy(desc));
                        VBridge b2 = new VBridge(copy(name), state, flt, bconf);
                        list1.add(b1);
                        list2.add(b2);

                        List<VBridge> l1 = new ArrayList<VBridge>(list1);
                        List<VBridge> l2 = new ArrayList<VBridge>(list2);
                        VBridgeList vb1 = new VBridgeList(l1);
                        VBridgeList vb2 = new VBridgeList(l2);
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
     * Ensure that {@link VBridgeList} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        // null list.
        VBridgeList vbList = new VBridgeList(null);
        jaxbTest(vbList, VBridgeList.class, XML_ROOT);
        jsonTest(vbList, VBridgeList.class);

        // Empty list.
        List<VBridge> list = new ArrayList<VBridge>();
        vbList = new VBridgeList(list);
        jaxbTest(vbList, VBridgeList.class, XML_ROOT);
        jsonTest(vbList, VBridgeList.class);

        VnodeState[] states = VnodeState.values();
        for (String name: createStrings("vBridge")) {
            for (String desc: createStrings("Description")) {
                for (int flt = 0; flt < 2; flt++) {
                    for (VnodeState state: states) {
                        VBridgeConfig bconf = new VBridgeConfig(desc);
                        VBridge vbridge = new VBridge(name, state, flt, bconf);

                        // Single entry.
                        List<VBridge> one = new ArrayList<VBridge>();
                        one.add(vbridge);
                        vbList = new VBridgeList(one);
                        jaxbTest(vbList, VBridgeList.class, XML_ROOT);
                        jsonTest(vbList, VBridgeList.class);

                        list.add(vbridge);
                    }
                }
            }
        }

        vbList = new VBridgeList(list);
        jaxbTest(vbList, VBridgeList.class, XML_ROOT);
        jsonTest(vbList, VBridgeList.class);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(VBridgeList.class,
                      new XmlAttributeType("vbridge", "faults", int.class).
                      add(XML_ROOT),
                      new XmlAttributeType("vbridge", "state", int.class).
                      add(XML_ROOT),
                      new XmlAttributeType("vbridge", "ageInterval",
                                           Integer.class).add(XML_ROOT));
    }
}
