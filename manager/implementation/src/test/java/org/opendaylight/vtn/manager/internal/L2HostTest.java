/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.cluster.MacVlan;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;

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
        for (NodeConnector port: createNodeConnectors(10, false)) {
            for (short vlan : vlans) {
                for (EthernetAddress ea : createEthernetAddresses()) {
                    byte[] addr = (ea == null) ? null : ea.getValue();
                    L2Host lh = new L2Host(addr, vlan, port);
                    long mac = NetUtils.byteArray6ToLong(addr);
                    MacVlan mvlan = lh.getHost();
                    assertEquals(mac, mvlan.getMacAddress());
                    assertEquals(vlan, mvlan.getVlan());
                    assertEquals(port, lh.getPort());

                    L2Host lh2 = new L2Host(mac, vlan, port);
                    mvlan = lh2.getHost();
                    assertEquals(mac, mvlan.getMacAddress());
                    assertEquals(vlan, mvlan.getVlan());
                    assertEquals(port, lh2.getPort());
                }

                L2Host lh = new L2Host(vlan, port);
                MacVlan mvlan = lh.getHost();
                assertEquals(MacVlan.UNDEFINED, mvlan.getMacAddress());
                assertEquals(vlan, mvlan.getVlan());
                assertEquals(port, lh.getPort());
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
        List<NodeConnector> ports = createNodeConnectors(10, false);

        for (NodeConnector port: ports) {
            for (short vlan: vlans) {
                for (EthernetAddress ea : ethers) {
                    byte[] addr = (ea == null) ? null : ea.getValue();
                    L2Host h1 = new L2Host(addr, vlan, port);

                    byte[] addr2;
                    if (addr == null) {
                        addr2 = null;
                    } else {
                        addr2 = new byte[addr.length];
                        System.arraycopy(addr, 0, addr2, 0, addr.length);
                    }
                    L2Host h2 = new L2Host(addr2, vlan, copy(port));
                    testEquals(set, h1, h2);
                }

                L2Host h = new L2Host(vlan, port);
                assertFalse(set.add(h));
            }
        }

        assertEquals(ports.size() * vlans.length * ethers.size(),
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

        for (EthernetAddress ea : createEthernetAddresses()) {
            for (short vlan: vlans) {
                for (NodeConnector port: createNodeConnectors(10, false)) {
                    byte[] addr = (ea == null) ? null : ea.getValue();
                    L2Host lh = new L2Host(addr, vlan, port);

                    long mac = NetUtils.byteArray6ToLong(addr);
                    StringBuilder builder = new StringBuilder("host={");
                    if (mac != MacVlan.UNDEFINED) {
                        builder.append("addr=").
                            append(HexEncode.bytesToHexStringFormat(addr)).
                            append(',');
                    }
                    builder.append("vlan=").append((int)vlan).append('}');
                    String p = "port=" + port;

                    String required = joinStrings(prefix, suffix, ",",
                                                  builder.toString(), p);
                    assertEquals(required, lh.toString());
                }
            }
        }
    }
}
