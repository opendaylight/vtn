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
import java.util.Arrays;
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
        List<InetAddress[]> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (InetAddress[] ip : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ip);

                        assertEquals(ea, mae.getAddress());
                        assertEquals(vlan, mae.getVlan());
                        assertEquals(nc, mae.getNodeConnector());

                        InetAddress[] addrs = mae.getInetAddresses();
                        assertNotNull(addrs);
                        if (ip != null) {
                            // Ensure that MacAddressEntry copies the
                            // given InetAddress array.
                            InetAddress[] required = ip.clone();
                            for (int i = 0; i < ip.length; i++) {
                                ip[i] = null;
                            }
                            assertTrue(Arrays.equals(required, addrs));
                        } else {
                            assertEquals(0, addrs.length);
                        }

                        // Ensure that getInetAddresses() returns a clone.
                        InetAddress[] addrs1 = mae.getInetAddresses();
                        assertNotSame(addrs, addrs1);
                        assertTrue(Arrays.equals(addrs, addrs1));
                    }
                }
            }
        }

        // test for DataLinkAddress class except EthrenetAddress.
        TestDataLink tdl = new TestDataLink("TestDataLink");

        for (NodeConnector nc : createNodeConnectors(3, false)) {
            short vlan = 0;
            InetAddress[] ip = ips.get(1);
            MacAddressEntry mae = new MacAddressEntry(tdl, vlan, nc, ip);

            assertEquals(tdl, mae.getAddress());
            assertEquals(vlan, mae.getVlan());
            assertEquals(nc, mae.getNodeConnector());
            assertTrue(Arrays.equals(ip, mae.getInetAddresses()));
        }
    }

    /**
     * Test case for {@link MacAddressEntry#equals(Object)} and
     * {@link MacAddressEntry#hashCode()}.
     */
    @Test
    public void testEquals() {
        short vlans[] = { -10, 0, 1, 100, 4095 };
        List<InetAddress[]> ips = createInetAddresses(false);
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        HashSet<Object> set = new HashSet<Object>();
        List<NodeConnector> connectors = createNodeConnectors(3, false);

        for (NodeConnector nc : connectors) {
            for (InetAddress[] ip : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae1 =
                            new MacAddressEntry(ea, vlan, nc, ip);
                        MacAddressEntry mae2 =
                            new MacAddressEntry(copy(ea), vlan, copy(nc),
                                                copy(ip));

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
        List<InetAddress[]> ips = createInetAddresses();
        String prefix = "MacAddressEntry[";
        String suffix = "]";
        short vlans[] = { -10, 0, 1, 100, 4095 };
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (InetAddress[] ip : ips) {
                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ip);

                        String a = "address=" + ea;
                        String v = "vlan=" + vlan;
                        String c = "connector=" + nc;
                        String i = "ipaddr=" + toString(ip);
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
            for (InetAddress[] ip : createInetAddresses()) {
                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ip);

                        serializeTest(mae);
                    }
                }
            }
        }
    }

    /**
     * Return a string representation of the given {@code InetAddress} array.
     *
     * @param ipaddrs  An array of {@code InetAddress}.
     * @return  A string representation of the given array.
     */
    private String toString(InetAddress[] ipaddrs) {
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
