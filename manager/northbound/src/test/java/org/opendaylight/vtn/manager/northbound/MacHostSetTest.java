/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link MacHostSet}
 */
public class MacHostSetTest extends TestBase {
    /**
     * test case for {@link MacHostSet#MacHostSet(java.util.Collection)} and
     * {@link MacHostSet#getHosts()}
     */
    @Test
    public void testGetter() {
        MacHostSet mhset = new MacHostSet(null);
        assertTrue(mhset.getHosts().isEmpty());

        mhset = new MacHostSet(new HashSet<DataLinkHost>());
        assertTrue(mhset.getHosts().isEmpty());

        List<DataLinkHost> list = new ArrayList<DataLinkHost>();
        Set<MacHost> expected = new HashSet<MacHost>();

        short vlans[] = {0, 1, 100, 1000, 4095};
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            for (short vlan: vlans) {
                DataLinkHost dlhost = new EthernetHost(eaddr, vlan);
                list.add(dlhost);
                expected.add(new MacHost(dlhost));

                // Duplicate elements should not be added.
                List<DataLinkHost> l = new ArrayList<DataLinkHost>(list);
                list.add(dlhost);

                mhset = new MacHostSet(list);
                assertEquals(expected, mhset.getHosts());
            }
        }
    }

    /**
     * test case for {@link MacHostSet#equals(Object)} and
     * {@link MacHostSet#hashCode()}
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        assertTrue(set.add(new MacHostSet(null)));
        assertFalse(set.add(new MacHostSet(new HashSet<DataLinkHost>())));

        List<DataLinkHost> list1 = new ArrayList<DataLinkHost>();
        List<DataLinkHost> list2 = new ArrayList<DataLinkHost>();
        List<EthernetAddress> addresses = createEthernetAddresses();
        short vlans[] = {0, 1, 100, 1000, 4095};
        for (EthernetAddress eaddr: addresses) {
            for (short vlan : vlans) {
                DataLinkHost dh1 = new EthernetHost(eaddr, vlan);
                DataLinkHost dh2 =
                    new EthernetHost((EthernetAddress)copy(eaddr), vlan);
                list1.add(dh1);
                list2.add(dh2);

                MacHostSet l1 = new MacHostSet(list1);
                MacHostSet l2 = new MacHostSet(list2);
                testEquals(set, l1, l2);
            }
        }

        assertEquals(addresses.size() * vlans.length + 1, set.size());
    }

    /**
     * Ensure that {@link MacHostSet} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        String rootName = "machosts";
        MacHostSet mhset = new MacHostSet(null);
        jaxbTest(mhset, rootName);

        mhset = new MacHostSet(new HashSet<DataLinkHost>());
        jaxbTest(mhset, rootName);

        List<DataLinkHost> list = new ArrayList<DataLinkHost>();
        short vlans[] = {0, 1, 100, 1000, 4095};
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            for (short vlan: vlans) {
                DataLinkHost dlhost = new EthernetHost(eaddr, vlan);
                list.add(dlhost);
                mhset = new MacHostSet(list);
                jaxbTest(mhset, rootName);
            }
        }
    }
}
