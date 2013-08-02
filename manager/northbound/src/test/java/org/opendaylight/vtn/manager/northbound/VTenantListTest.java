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

import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;

/**
 * JUnit test for {@link VTenantList}.
 */
public class VTenantListTest extends TestBase {
    /**
     * Test case for {@link VTenantList#equals(Object)} and
     * {@link VTenantList#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        // null list.
        VTenantList nullList = new VTenantList(null);
        testEquals(set, nullList, new VTenantList(null));

        // Empty list should be treated as null list.
        VTenantList emptyList = new VTenantList(new ArrayList<VTenant>());
        assertEquals(nullList, emptyList);
        assertEquals(nullList.hashCode(), emptyList.hashCode());
        assertFalse(set.add(emptyList));

        List<VTenant> list1 = new ArrayList<VTenant>();
        List<VTenant> list2 = new ArrayList<VTenant>();
        List<String> names = createStrings("name");
        List<String> descs = createStrings("description");
        for (String name: names) {
            for (String desc: descs) {
                VTenantConfig tconf = new VTenantConfig(desc);
                VTenant t1 = new VTenant(name, tconf);
                tconf = new VTenantConfig(copy(desc));
                VTenant t2 = new VTenant(copy(name), tconf);
                list1.add(t1);
                list2.add(t2);

                List<VTenant> l1 = new ArrayList<VTenant>(list1);
                List<VTenant> l2 = new ArrayList<VTenant>(list2);
                VTenantList vt1 = new VTenantList(l1);
                VTenantList vt2 = new VTenantList(l2);
                testEquals(set, vt1, vt2);
            }
        }

        assertEquals(names.size() * descs.size() + 1, set.size());
    }

    /**
     * Ensure that {@link VTenantList} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        // null list.
        VTenantList vtList = new VTenantList(null);
        String rootName = "vtns";
        jaxbTest(vtList, rootName);

        // Empty list.
        List<VTenant> list = new ArrayList<VTenant>();
        vtList = new VTenantList(list);
        jaxbTest(vtList, rootName);

        for (String name: createStrings("Tenant Name")) {
            for (String desc: createStrings("Description")) {
                VTenantConfig tconf = new VTenantConfig(desc);
                VTenant vtenant = new VTenant(name, tconf);

                // Single entry.
                List<VTenant> one = new ArrayList<VTenant>();
                one.add(vtenant);
                vtList = new VTenantList(one);
                jaxbTest(vtList, rootName);

                list.add(vtenant);
            }
        }

        vtList = new VTenantList(list);
        jaxbTest(vtList, rootName);
    }
}
