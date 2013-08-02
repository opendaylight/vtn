/**
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.SwitchPort;

/**
 * JUnit test for {@link MacEntry}
 */
public class MacEntryTest extends TestBase {

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
     * Test case for {@link MacEntry#MacEntry(MacAddressEntry)} and getter
     * method
     */
    @Test
    public void testGetter() {
        short vlans[] = { -10, 0, 1, 100, 4095 };
        List<InetAddress[]> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);

        // use a combination of NodeConnectors, InetAddresses,
        // EthernetAddresses, vlans as input parameter
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (InetAddress[] ip : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ip);
                        MacEntry me = new MacEntry(mae);

                        String type = nc.getType();
                        String id = nc.getNodeConnectorIDString();

                        assertEquals(ea.getMacAddress(), me.getAddress());
                        assertEquals(vlan, me.getVlan());
                        assertEquals(nc.getNode(), me.getNode());
                        assertEquals(new SwitchPort(type, id), me.getPort());

                        IpAddressArray iarray = me.getInetAddresses();
                        if (ip != null && ip.length != 0) {
                            assertNotNull(iarray);
                            assertEquals(ip.length, iarray.getLength());
                            for (int i = 0; i < ip.length; i++) {
                                IpAddress ipaddr = iarray.get(i);
                                assertEquals(ip[i].getHostAddress(),
                                             ipaddr.getAddress());
                            }
                        } else {
                            assertNull(iarray);
                        }
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
            MacEntry me = new MacEntry(mae);

            String type = nc.getType();
            String id = nc.getNodeConnectorIDString();
            SwitchPort port = new SwitchPort(type, id);

            assertEquals(tdl.toString(), me.getAddress());
            assertEquals(vlan, me.getVlan());
            assertEquals(nc.getNode(), me.getNode());
            assertEquals(port, me.getPort());

            IpAddressArray iarray = me.getInetAddresses();
            assertNotNull(iarray);
            assertEquals(ip.length, iarray.getLength());
            for (int i = 0; i < ip.length; i++) {
                IpAddress ipaddr = iarray.get(i);
                assertEquals(ip[i].getHostAddress(), ipaddr.getAddress());
            }
        }
    }

    /**
     * test case for {@link MacEntry#equals(java.lang.Object)} and
     * {@link MacEntry#hashCode()}
     */
    @Test
    public void testEquals() {
        short vlans[] = { -10, 0, 1, 100, 4095 };
        List<InetAddress[]> ips = createInetAddresses(false);
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        List<NodeConnector> connectors = createNodeConnectors(3, false);
        HashSet<Object> set = new HashSet<Object>();

        for (NodeConnector nc : connectors) {
            for (InetAddress[] ip : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae1 =
                            new MacAddressEntry(ea, vlan, nc, ip);
                        MacAddressEntry mae2 =
                            new MacAddressEntry(copy(ea), vlan, copy(nc),
                                                copy(ip));
                        MacEntry me1 = new MacEntry(mae1);
                        MacEntry me2 = new MacEntry(mae2);

                        testEquals(set, me1, me2);
                    }
                }
            }
        }

        assertEquals(connectors.size() * ips.size() * ethaddrs.size() *
                     vlans.length, set.size());
    }

    /**
     * Ensure that {@link MacEntry} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        short vlans[] = { -10, 0, 1, 100, 4095 };
        List<InetAddress[]> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        List<NodeConnector> connectors = createNodeConnectors(3, false);

        for (NodeConnector nc : connectors) {
            for (InetAddress[] ip : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ip);
                        MacEntry me = new MacEntry(mae);
                        jaxbTest(me, "macentry");
                    }
                }
            }
        }
    }
}
