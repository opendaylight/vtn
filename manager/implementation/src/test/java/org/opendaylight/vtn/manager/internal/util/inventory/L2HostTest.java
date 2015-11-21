/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link L2Host}.
 */
public class L2HostTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        int[] vlans = {0, 1, 1000, 4095};
        SalPort[] ports = {
            SalPort.create("openflow:1:1"),
            SalPort.create("openflow:1:2"),
            SalPort.create("openflow:1:3"),
            SalPort.create("openflow:2:1"),
            SalPort.create("openflow:2:2"),
            SalPort.create("openflow:2:3"),
            SalPort.create("openflow:123456789012:12345"),
        };

        for (SalPort sport: ports) {
            for (int vlan: vlans) {
                for (EtherAddress eaddr: createEtherAddresses()) {
                    long mac;
                    MacAddress maddr;
                    if (eaddr == null) {
                        mac = 0;
                        maddr = null;
                    } else {
                        mac = eaddr.getAddress();
                        maddr = eaddr.getMacAddress();
                    }

                    L2Host lh = new L2Host(mac, vlan, sport);
                    MacVlan mvlan = lh.getHost();
                    assertEquals(mac, mvlan.getAddress());
                    assertEquals(vlan, mvlan.getVlanId());
                    assertEquals(sport, lh.getPort());

                    lh = new L2Host(maddr, vlan, sport);
                    mvlan = lh.getHost();
                    assertEquals(mac, mvlan.getAddress());
                    assertEquals(vlan, mvlan.getVlanId());
                    assertEquals(sport, lh.getPort());

                    lh = new L2Host(mvlan, sport);
                    assertEquals(mvlan, lh.getHost());
                    assertEquals(sport, lh.getPort());
                }

                L2Host lh = new L2Host(vlan, sport);
                MacVlan mvlan = lh.getHost();
                assertEquals(MacVlan.UNDEFINED, mvlan.getAddress());
                assertEquals(vlan, mvlan.getVlanId());
                assertEquals(sport, lh.getPort());
            }
        }
    }

    /**
     * Test case for {@link L2Host#equals(Object)} and
     * {@link L2Host#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();
        int[] vlans = {0, 1, 10, 1000, 4095};
        List<EtherAddress> eaddrs = createEtherAddresses();
        SalPort[] ports = {
            SalPort.create("openflow:1:1"),
            SalPort.create("openflow:1:2"),
            SalPort.create("openflow:1:3"),
            SalPort.create("openflow:2:1"),
            SalPort.create("openflow:2:2"),
            SalPort.create("openflow:2:3"),
            SalPort.create("openflow:123456789012:12345"),
        };

        for (SalPort sport1: ports) {
            SalPort sport2 =
                new SalPort(sport1.getNodeNumber(), sport1.getPortNumber());
            for (int vlan: vlans) {
                for (EtherAddress eaddr: eaddrs) {
                    MacAddress mac1;
                    MacAddress mac2;
                    if (eaddr == null) {
                        mac1 = null;
                        mac2 = null;
                    } else {
                        mac1 = eaddr.getMacAddress();
                        mac2 = new MacAddress(mac1.getValue());
                    }

                    L2Host h1 = new L2Host(mac1, vlan, sport1);
                    L2Host h2 = new L2Host(mac2, vlan, sport2);
                    testEquals(set, h1, h2);
                }

                L2Host h = new L2Host(vlan, sport1);
                assertFalse(set.add(h));
            }
        }

        assertEquals(ports.length * vlans.length * eaddrs.size(),
                     set.size());
    }

    /**
     * Test method for {@link MacVlan#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "L2Host[";
        String suffix = "]";
        int[] vlans = {0, 1, 10, 1000, 4095};
        SalPort[] ports = {
            SalPort.create("openflow:1:1"),
            SalPort.create("openflow:1:2"),
            SalPort.create("openflow:1:3"),
            SalPort.create("openflow:2:1"),
            SalPort.create("openflow:2:2"),
            SalPort.create("openflow:2:3"),
            SalPort.create("openflow:123456789012:12345"),
        };

        for (EtherAddress eaddr: createEtherAddresses()) {
            for (int vlan: vlans) {
                for (SalPort sport: ports) {
                    MacAddress maddr = (eaddr == null)
                        ? null : eaddr.getMacAddress();
                    L2Host lh = new L2Host(maddr, vlan, sport);

                    StringBuilder builder = new StringBuilder("host=");
                    if (eaddr != null) {
                        builder.append(eaddr.getText());
                    }
                    builder.append('@').append(vlan);
                    String p = "port=" + sport;

                    String required = joinStrings(prefix, suffix, ",",
                                                  builder.toString(), p);
                    assertEquals(required, lh.toString());
                }
            }
        }
    }
}
