/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.net.InetAddress;
import java.util.Set;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link MacAddressEntry}
 */
public class MacAddressEntryTest extends TestBase {
    /**
     * Test case for {@link MacAddressEntry} and getter method
     */
    @Test
    public void testGetter() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        List<Set<InetAddress>> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ipset);

                        String emsg = mae.toString();
                        assertEquals(emsg, ea, mae.getAddress());
                        assertEquals(emsg, vlan, mae.getVlan());
                        assertEquals(emsg, nc, mae.getNodeConnector());

                        Set<InetAddress> addrs = mae.getInetAddresses();
                        assertNotNull(emsg, addrs);
                        if (ipset != null) {
                            // Ensure that MacAddressEntry copies the
                            // given InetAddress set.
                            Set<InetAddress> required =
                                new HashSet<InetAddress>(ipset);
                            ipset.clear();
                            assertEquals(emsg, required, addrs);
                        } else {
                            assertEquals(emsg, 0, addrs.size());
                        }

                        // Ensure that getInetAddresses() returns a clone.
                        Set<InetAddress> addrs1 = mae.getInetAddresses();
                        assertNotSame(emsg, addrs, addrs1);
                        assertEquals(emsg, addrs, addrs1);
                    }
                }
            }
        }

        // test for DataLinkAddress class except EthrenetAddress.
        TestDataLink tdl = new TestDataLink("TestDataLink");

        for (NodeConnector nc : createNodeConnectors(3, false)) {
            short vlan = 0;
            Set<InetAddress> ipset = ips.get(1);
            MacAddressEntry mae = new MacAddressEntry(tdl, vlan, nc, ipset);

            String emsg = mae.toString();
            assertEquals(emsg, tdl, mae.getAddress());
            assertEquals(emsg, vlan, mae.getVlan());
            assertEquals(emsg, nc, mae.getNodeConnector());
            assertEquals(emsg, ipset, mae.getInetAddresses());
        }
    }

    /**
     * Test case for {@link MacAddressEntry#equals(Object)} and
     * {@link MacAddressEntry#hashCode()}.
     */
    @Test
    public void testEquals() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        List<Set<InetAddress>> ips = createInetAddresses(false);
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        HashSet<Object> set = new HashSet<Object>();
        List<NodeConnector> connectors = createNodeConnectors(3, false);

        for (NodeConnector nc : connectors) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae1 =
                            new MacAddressEntry(ea, vlan, nc, ipset);
                        MacAddressEntry mae2 = new MacAddressEntry(
                            copy(ea), vlan, copy(nc),
                            copy(ipset, InetAddress.class));

                        testEquals(set, mae1, mae2);
                    }
                }
            }
        }

        int required = connectors.size() * ips.size() * ethaddrs.size() *
            vlans.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link MacAddressEntry#toString()}
     */
    @Test
    public void testToString() {
        List<Set<InetAddress>> ips = createInetAddresses();
        String prefix = "MacAddressEntry[";
        short[] vlans = {-10, 0, 1, 100, 4095};
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ipset);

                        String a = "address=" + ea;
                        String v = "vlan=" + vlan;
                        String c = "connector=" + nc;

                        // IP addresses are kept by unordered set.
                        // So string representation of the object may be
                        // changed by its order.
                        String required =
                            joinStrings(prefix, null, ",", a, v, c);
                        String str = mae.toString();
                        int idx = str.indexOf(",ipaddr={");
                        assertTrue(idx > 0);
                        assertEquals(required, str.substring(0, idx));

                        Set<InetAddress> addrSet = getInetAddressSet(str);
                        if (ipset == null) {
                            ipset = new HashSet<InetAddress>();
                        }
                        assertEquals(ipset, addrSet);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link MacAddressEntry} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (Set<InetAddress> ipset : createInetAddresses()) {
                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ipset);

                        serializeTest(mae);
                    }
                }
            }
        }
    }

    /**
     * Construct a set of {@link InetAddress} instances from a string returned
     * by {@link MacAddressEntry#toString()}.
     *
     * @param str  A string returned by {@link MacAddressEntry#toString()}.
     * @return  A set of {@link InetAddress} instances.
     */
    private Set<InetAddress> getInetAddressSet(String str) {
        HashSet<InetAddress> set = new HashSet<InetAddress>();
        String prefix = ",ipaddr={";
        int start = str.indexOf(prefix);
        if (start < 0) {
            return set;
        }

        start += prefix.length();
        int end = str.lastIndexOf("}]");
        if (end < start + 1) {
            return set;
        }

        String istr = str.substring(start, end);
        try {
            for (String s: istr.split(",")) {
                set.add(InetAddress.getByName(s));
            }
        } catch (Exception e) {
            unexpected(e);
        }

        return set;
    }
}
