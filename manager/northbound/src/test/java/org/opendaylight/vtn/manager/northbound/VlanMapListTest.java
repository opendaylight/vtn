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
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.vtn.manager.VlanMap;

/**
 * JUnit test for {@link VlanMapList}.
 */
public class VlanMapListTest extends TestBase {
    /**
     * Root XML element name associated with {@link VlanMapList} class.
     */
    private static final String  XML_ROOT = "vlanmaps";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        List<VlanMap> list = new ArrayList<VlanMap>();
        List<String> ids = createStrings("map");
        List<Node> nodes = createNodes(3);
        short[] vlans = {-5, 0, 3, 4095};

        // null list.
        VlanMapList nullList = new VlanMapList(null);
        assertNull(nullList.getList());

        // Empty list should be treated as null list.
        VlanMapList emptyList = new VlanMapList(new ArrayList<VlanMap>());
        assertEquals(list, emptyList.getList());

        for (String id: ids) {
            for (Node node: nodes) {
                for (short vlan: vlans) {
                    VlanMap v = new VlanMap(id, node, vlan);

                    list.add(v);
                    List<VlanMap> l = new ArrayList<VlanMap>(list);

                    VlanMapList vl = new VlanMapList(l);
                    assertEquals(list, vl.getList());
                }
            }
        }
    }

    /**
     * Test case for {@link VlanMapList#equals(Object)} and
     * {@link VlanMapList#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        // null list.
        VlanMapList nullList = new VlanMapList(null);
        testEquals(set, nullList, new VlanMapList(null));

        // Empty list should be treated as null list.
        VlanMapList emptyList = new VlanMapList(new ArrayList<VlanMap>());
        assertEquals(nullList, emptyList);
        assertEquals(nullList.hashCode(), emptyList.hashCode());
        assertFalse(set.add(emptyList));

        List<VlanMap> list1 = new ArrayList<VlanMap>();
        List<VlanMap> list2 = new ArrayList<VlanMap>();
        List<String> ids = createStrings("map");
        List<Node> nodes = createNodes(3);
        short[] vlans = {-5, 0, 3, 4095};
        for (String id: ids) {
            for (Node node: nodes) {
                for (short vlan: vlans) {
                    VlanMap v1 = new VlanMap(id, node, vlan);
                    VlanMap v2 = new VlanMap(copy(id), copy(node), vlan);

                    list1.add(v1);
                    list2.add(v2);

                    List<VlanMap> l1 = new ArrayList<VlanMap>(list1);
                    List<VlanMap> l2 = new ArrayList<VlanMap>(list2);
                    VlanMapList vl1 = new VlanMapList(l1);
                    VlanMapList vl2 = new VlanMapList(l2);
                    testEquals(set, vl1, vl2);
                }
            }
        }

        int required = ids.size() * nodes.size() * vlans.length + 1;
        assertEquals(required, set.size());
    }

    /**
     * Ensure that {@link VlanMapList} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        // null list.
        VlanMapList vmList = new VlanMapList(null);
        jaxbTest(vmList, VlanMapList.class, XML_ROOT);
        jsonTest(vmList, VlanMapList.class);

        // Empty list.
        List<VlanMap> list = new ArrayList<VlanMap>();
        vmList = new VlanMapList(list);
        jaxbTest(vmList, VlanMapList.class, XML_ROOT);
        jsonTest(vmList, VlanMapList.class);

        short[] vlans = {-5, 0, 3, 4095};
        for (String id: createStrings("map")) {
            for (Node node: createNodes(3)) {
                for (short vlan: vlans) {
                    VlanMap vlmap = new VlanMap(id, node, vlan);

                    // Single entry.
                    List<VlanMap> one = new ArrayList<VlanMap>();
                    one.add(vlmap);
                    vmList = new VlanMapList(one);
                    jaxbTest(vmList, VlanMapList.class, XML_ROOT);
                    jsonTest(vmList, VlanMapList.class);

                    list.add(vlmap);
                }
            }
        }

        vmList = new VlanMapList(list);
        jaxbTest(vmList, VlanMapList.class, XML_ROOT);
        jsonTest(vmList, VlanMapList.class);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(VlanMapList.class,
                      new XmlAttributeType("vlanmap", "vlan", short.class).
                      add(XML_ROOT));
    }
}
