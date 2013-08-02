/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;

import org.junit.Test;

import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VNodeState;

import org.opendaylight.controller.sal.core.Node;

/**
 * JUnit test for {@link VlanMapList}.
 */
public class VlanMapListTest extends TestBase {
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
        short vlans[] = {-5, 0, 3, 4095};
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
     * Ensure that {@link VlanMapList} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        // null list.
        VlanMapList vmList = new VlanMapList(null);
        String rootName = "vlanmaps";
        jaxbTest(vmList, rootName);

        // Empty list.
        List<VlanMap> list = new ArrayList<VlanMap>();
        vmList = new VlanMapList(list);
        jaxbTest(vmList, rootName);

        short vlans[] = {-5, 0, 3, 4095};
        for (String id: createStrings("map")) {
            for (Node node: createNodes(3)) {
                for (short vlan: vlans) {
                    VlanMap vlmap = new VlanMap(id, node, vlan);

                    // Single entry.
                    List<VlanMap> one = new ArrayList<VlanMap>();
                    one.add(vlmap);
                    vmList = new VlanMapList(one);
                    jaxbTest(vmList, rootName);

                    list.add(vlmap);
                }
            }
        }

        vmList = new VlanMapList(list);
        jaxbTest(vmList, rootName);
    }
}
