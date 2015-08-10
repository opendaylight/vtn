/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.core.NodeConnector;

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
        short[] vlans = new short[] {0, 1, 1000, 4095};
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
            NodeConnector nc = sport.getAdNodeConnector();
            for (short vlan : vlans) {
                for (EthernetAddress ea : createEthernetAddresses()) {
                    byte[] addr;
                    long mac;
                    MacAddress maddr;
                    if (ea == null) {
                        addr = null;
                        mac = 0;
                        maddr = null;
                    } else {
                        addr = ea.getValue();
                        EtherAddress eaddr = EtherAddress.create(addr);
                        mac = eaddr.getAddress();
                        maddr = eaddr.getMacAddress();
                    }
                    L2Host lh = new L2Host(addr, vlan, nc);
                    MacVlan mvlan = lh.getHost();
                    assertEquals(mac, mvlan.getMacAddress());
                    assertEquals(vlan, mvlan.getVlan());
                    assertEquals(sport, lh.getPort());

                    L2Host lh2 = new L2Host(mac, vlan, nc);
                    mvlan = lh2.getHost();
                    assertEquals(mac, mvlan.getMacAddress());
                    assertEquals(vlan, mvlan.getVlan());
                    assertEquals(sport, lh2.getPort());

                    L2Host lh3 = new L2Host(maddr, (int)vlan, sport);
                    mvlan = lh3.getHost();
                    assertEquals(mac, mvlan.getMacAddress());
                    assertEquals(vlan, mvlan.getVlan());
                    assertEquals(sport, lh3.getPort());

                    L2Host lh4 = new L2Host(mvlan, sport);
                    assertEquals(mvlan, lh4.getHost());
                    assertEquals(sport, lh4.getPort());
                }

                L2Host lh = new L2Host(vlan, nc);
                MacVlan mvlan = lh.getHost();
                assertEquals(MacVlan.UNDEFINED, mvlan.getMacAddress());
                assertEquals(vlan, mvlan.getVlan());
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
        HashSet<Object> set = new HashSet<Object>();
        short[] vlans = {0, 1, 10, 1000, 4095};
        List<EthernetAddress> ethers = createEthernetAddresses();
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
            NodeConnector nc = sport.getAdNodeConnector();
            for (short vlan: vlans) {
                for (EthernetAddress ea : ethers) {
                    byte[] addr = (ea == null) ? null : ea.getValue();
                    L2Host h1 = new L2Host(addr, vlan, nc);

                    byte[] addr2;
                    if (addr == null) {
                        addr2 = null;
                    } else {
                        addr2 = new byte[addr.length];
                        System.arraycopy(addr, 0, addr2, 0, addr.length);
                    }
                    L2Host h2 = new L2Host(addr2, vlan, copy(nc));
                    testEquals(set, h1, h2);
                }

                L2Host h = new L2Host(vlan, nc);
                assertFalse(set.add(h));
            }
        }

        assertEquals(ports.length * vlans.length * ethers.size(),
                     set.size());
    }

    /**
     * Test method for {@link MacVlan#toString()} and
     * {@link MacVlan#appendContents(StringBuilder)}.
     */
    @Test
    public void testToString() {
        String prefix = "L2Host[";
        String suffix = "]";
        short[] vlans = {0, 1, 10, 1000, 4095};
        SalPort[] ports = {
            SalPort.create("openflow:1:1"),
            SalPort.create("openflow:1:2"),
            SalPort.create("openflow:1:3"),
            SalPort.create("openflow:2:1"),
            SalPort.create("openflow:2:2"),
            SalPort.create("openflow:2:3"),
            SalPort.create("openflow:123456789012:12345"),
        };

        for (EthernetAddress ea : createEthernetAddresses()) {
            for (short vlan: vlans) {
                for (SalPort sport: ports) {
                    NodeConnector nc = sport.getAdNodeConnector();
                    byte[] addr;
                    long mac;
                    if (ea == null) {
                        addr = null;
                        mac = 0;
                    } else {
                        addr = ea.getValue();
                        mac = EtherAddress.toLong(addr);
                    }
                    L2Host lh = new L2Host(addr, vlan, nc);

                    StringBuilder builder = new StringBuilder("host={");
                    if (mac != MacVlan.UNDEFINED) {
                        builder.append("addr=").
                            append(ByteUtils.toHexString(addr)).
                            append(',');
                    }
                    builder.append("vlan=").append((int)vlan).append('}');
                    String p = "port=" + sport;

                    String required = joinStrings(prefix, suffix, ",",
                                                  builder.toString(), p);
                    assertEquals(required, lh.toString());
                }
            }
        }
    }
}
