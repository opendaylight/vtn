/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link EthernetHost}.
 */
public class EthernetHostTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = {0, 1, 100, 1000, 4095};
        for (short vlan: vlans) {
            for (EthernetAddress eaddr: createEthernetAddresses()) {
                EthernetHost ehost = new EthernetHost(eaddr, vlan);
                assertEquals(eaddr, ehost.getAddress());
                assertEquals(vlan, ehost.getVlan());
            }

            EthernetHost ehost = new EthernetHost(vlan);
            assertNull(ehost.getAddress());
            assertEquals(vlan, ehost.getVlan());
        }
    }

    /**
     * Test case for {@link EthernetHost#equals(Object)} and
     * {@link EthernetHost#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] vlans = {0, 1, 100, 1000, 4095};
        List<EthernetAddress> eaddrs = createEthernetAddresses();
        for (EthernetAddress eaddr: eaddrs) {
            for (short vlan: vlans) {
                EthernetHost eh1 = new EthernetHost(eaddr, vlan);
                EthernetHost eh2 =
                    new EthernetHost((EthernetAddress)copy(eaddr), vlan);
                testEquals(set, eh1, eh2);
            }
        }

        // An instance of different class should be treated as different
        // instance.
        List<DataLinkAddress> dladdrs = createDataLinkAddresses();
        for (DataLinkAddress dladdr1: dladdrs) {
            DataLinkAddress dladdr2 = copy(dladdr1);
            DataLinkHost dh1 = new TestDataLinkHost(dladdr1);
            DataLinkHost dh2 = new TestDataLinkHost(dladdr2);
            testEquals(set, dh1, dh2);
        }

        int required = eaddrs.size() * vlans.length + dladdrs.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link EthernetHost#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "EthernetHost[";
        String suffix = "]";
        short[] vlans = {0, 1, 100, 1000, 4095};
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            for (short vlan: vlans) {
                EthernetHost ehost = new EthernetHost(eaddr, vlan);
                String a = (eaddr == null)
                    ? null : "address=" + eaddr.getMacAddress();
                String v = "vlan=" + vlan;
                String required = joinStrings(prefix, suffix, ",", a, v);
                assertEquals(required, ehost.toString());
            }
        }
    }

    /**
     * Ensure that {@link EthernetHost} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {0, 1, 100, 1000, 4095};
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            for (short vlan: vlans) {
                EthernetHost ehost = new EthernetHost(eaddr, vlan);
                serializeTest(ehost);
            }
        }
    }
}
