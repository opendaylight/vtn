/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;

/**
 * JUnit test for {@link VInterfaceList}.
 */
public class VInterfaceListTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        List<VInterface> list = new ArrayList<VInterface>();
        List<String> names = createStrings("nm");
        List<VInterfaceConfig> configs = createConfigs();
        VNodeState[] states = VNodeState.values();

        // null list.
        VInterfaceList nullList = new VInterfaceList(null);
        assertNull(nullList.getList());

        // Empty list should be treated as null list.
        VInterfaceList emptyList =
            new VInterfaceList(new ArrayList<VInterface>());
        assertEquals(list, emptyList.getList());

        for (String name: names) {
            for (VInterfaceConfig iconf: configs) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
                        VInterface i =
                            new VInterface(name, state, estate, iconf);

                        list.add(i);

                        // Single entry.
                        List<VInterface> l = new ArrayList<VInterface>();
                        l.add(i);
                        VInterfaceList vi = new VInterfaceList(l);
                        assertEquals(l, vi.getList());
                        // Multiple entries.
                        VInterfaceList vil = new VInterfaceList(list);
                        assertEquals(list, vil.getList());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VInterfaceList#equals(Object)} and
     * {@link VInterfaceList#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        // null list.
        VInterfaceList nullList = new VInterfaceList(null);
        testEquals(set, nullList, new VInterfaceList(null));

        // Empty list should be treated as null list.
        VInterfaceList emptyList =
            new VInterfaceList(new ArrayList<VInterface>());
        assertEquals(nullList, emptyList);
        assertEquals(nullList.hashCode(), emptyList.hashCode());
        assertFalse(set.add(emptyList));

        List<VInterface> list1 = new ArrayList<VInterface>();
        List<VInterface> list2 = new ArrayList<VInterface>();
        List<String> names = createStrings("nm");
        List<VInterfaceConfig> configs = createConfigs();
        VNodeState[] states = VNodeState.values();
        for (String name: names) {
            for (VInterfaceConfig iconf: configs) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
                        VInterface i1 =
                            new VInterface(name, state, estate, iconf);
                        VInterface i2 =
                            new VInterface(copy(name), state, estate,
                                           copy(iconf));

                        list1.add(i1);
                        list2.add(i2);

                        // Single entry.
                        List<VInterface> l1 = new ArrayList<VInterface>();
                        List<VInterface> l2 = new ArrayList<VInterface>();
                        l1.add(i1);
                        l2.add(i2);
                        VInterfaceList vi1 = new VInterfaceList(l1);
                        VInterfaceList vi2 = new VInterfaceList(l2);
                        testEquals(set, vi1, vi2);
                    }
                }
            }
        }

        // Multiple entries.
        VInterfaceList vi1 = new VInterfaceList(list1);
        VInterfaceList vi2 = new VInterfaceList(list2);
        testEquals(set, vi1, vi2);
        list2.remove(0);
        vi2 = new VInterfaceList(list2);
        assertFalse(vi1.equals(vi2));

        int required = names.size() * configs.size() * states.length *
            states.length + 2;
        assertEquals(required, set.size());
    }

    /**
     * Ensure that {@link VInterfaceList} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        // null list.
        VInterfaceList ifList = new VInterfaceList(null);
        String rootName = "interfaces";
        jaxbTest(ifList, rootName);

        // Empty list.
        List<VInterface> list = new ArrayList<VInterface>();
        ifList = new VInterfaceList(list);
        jaxbTest(ifList, rootName);

        VNodeState[] states = VNodeState.values();
        for (String name: createStrings("nm")) {
            for (VInterfaceConfig iconf: createConfigs()) {
                for (VNodeState state: states) {
                    for (VNodeState estate: states) {
                        VInterface viface =
                            new VInterface(name, state, estate, iconf);

                        // Single entry.
                        List<VInterface> one = new ArrayList<VInterface>();
                        one.add(viface);
                        ifList = new VInterfaceList(one);
                        jaxbTest(ifList, rootName);

                        list.add(viface);
                    }
                }
            }
        }

        ifList = new VInterfaceList(list);
        jaxbTest(ifList, rootName);
    }

    /**
     * Create a list of test interface configurations.
     *
     * @return  A list of {@link VInterfaceConfig} objects.
     */
    private List<VInterfaceConfig> createConfigs() {
        ArrayList<VInterfaceConfig> list = new ArrayList<VInterfaceConfig>();

        for (String desc: createStrings("ds")) {
            for (Boolean enabled: createBooleans()) {
                VInterfaceConfig iconf = new VInterfaceConfig(desc, enabled);
                list.add(iconf);
            }
        }

        return list;
    }
}
