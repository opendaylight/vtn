/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager;

import java.net.InetAddress;
import java.util.Set;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link MacAddressEntry}
 */
public class MacAddressEntryTest extends TestBase {

    // The Test class which implemented DataLinkAddress class.
    class TestDataLink extends DataLinkAddress {
        private static final long serialVersionUID = 5664547077536394233L;

        TestDataLink() {

        }

        TestDataLink(String name) {
            super(name);
        }

        public TestDataLink clone() {
            return new TestDataLink(this.getName());
        }
    }

    /**
     * Test case for {@link MacAddressEntry} and getter method
     */
    @Test
    public void testGetter() {
        short vlans[] = { -10, 0, 1, 100, 4095 };
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
        short vlans[] = { -10, 0, 1, 100, 4095 };
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
                        MacAddressEntry mae2 =
                            new MacAddressEntry(copy(ea), vlan, copy(nc),
                                                copy(ipset));

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
        String suffix = "]";
        short vlans[] = { -10, 0, 1, 100, 4095 };
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ipset);

                        String a = "address=" + ea;
                        String v = "vlan=" + vlan;
                        String c = "connector=" + nc;
                        String i = "ipaddr=" + toString(ipset);
                        String required =
                            joinStrings(prefix, suffix, ",", a, v, c, i);
                        assertEquals(required, mae.toString());
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
        short vlans[] = { -10, 0, 1, 100, 4095 };
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
     * Return a string representation of the given {@code InetAddress} set.
     *
     * @param ipaddrs  An set of {@code InetAddress}.
     * @return  A string representation of the given set.
     */
    private String toString(Set<InetAddress> ipaddrs) {
        StringBuilder builder = new StringBuilder("{");
        if (ipaddrs != null) {
            char sep = 0;
            for (InetAddress ip: ipaddrs) {
                if (sep != 0) {
                    builder.append(',');
                }
                builder.append(ip.getHostAddress());
                sep = ',';
            }
        }

        return builder.append('}').toString();
    }
}
