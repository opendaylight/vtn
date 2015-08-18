/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.vtn.manager.MacAddressEntry;

/**
 * JUnit test for {@link MacEntryList}
 */
public class MacEntryListTest extends TestBase {
    /**
     * Root XML element name associated with {@link MacEntryList} class.
     */
    private static final String  XML_ROOT = "macentries";

    /**
     * test case for {@link MacEntryList#MacEntryList(java.util.List)} and
     * {@link MacEntryList#getList()}
     */
    @Test
    public void testGetter() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        List<Set<InetAddress>> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        List<NodeConnector> connectors = createNodeConnectors(3, false);

        // null list.
        MacEntryList nullList = new MacEntryList(null);
        assertNull(nullList.getList());

        // Empty list should be treated as null list.
        MacEntryList emptyList =
            new MacEntryList(new ArrayList<MacAddressEntry>());
        assertNotNull(emptyList.getList());
        assertEquals(new ArrayList<MacAddressEntry>(), emptyList.getList());

        // use a combination of NodeConnectors, InetAddresses,
        // EthernetAddresses, vlans as input parameter
        List<MacAddressEntry> list = new ArrayList<MacAddressEntry>();
        List<MacEntry> elist = new ArrayList<MacEntry>();
        for (NodeConnector nc : connectors) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ipset);
                        list.add(mae);
                        elist.add(new MacEntry(mae));
                        MacEntryList mel = new MacEntryList(list);
                        assertEquals(elist, mel.getList());
                    }
                }
            }
        }
    }

    /**
     * test case for {@link MacEntryList#equals(java.lang.Object)} and
     * {@link MacEntryList#hashCode()}
     */
    @Test
    public void testEquals() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        List<Set<InetAddress>> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        List<NodeConnector> connectors = createNodeConnectors(3, false);
        HashSet<Object> set = new HashSet<Object>();

        // null list.
        MacEntryList nullList = new MacEntryList(null);
        testEquals(set, nullList, new MacEntryList(null));

        // Empty list should be treated as null list.
        MacEntryList emptyList = new MacEntryList(new ArrayList<MacAddressEntry>());
        assertEquals(nullList, emptyList);
        assertEquals(emptyList, nullList);
        assertEquals(nullList.hashCode(), emptyList.hashCode());
        assertFalse(set.add(emptyList));

        // use a combination of NodeConnectors, InetAddresses,
        // EthernetAddresses, vlans as input parameter
        List<MacAddressEntry> list1 = new ArrayList<MacAddressEntry>();
        List<MacAddressEntry> list2 = new ArrayList<MacAddressEntry>();
        for (NodeConnector nc : connectors) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae1 =
                            new MacAddressEntry(ea, vlan, nc, ipset);
                        MacAddressEntry mae2 = new MacAddressEntry(
                            copy(ea), vlan, copy(nc),
                            copy(ipset, InetAddress.class));
                        list1.add(mae1);
                        list2.add(mae2);

                        List<MacAddressEntry> l1 =
                            new ArrayList<MacAddressEntry>(list1);
                        List<MacAddressEntry> l2 =
                            new ArrayList<MacAddressEntry>(list2);
                        MacEntryList mel1 = new MacEntryList(l1);
                        MacEntryList mel2 = new MacEntryList(l2);
                        testEquals(set, mel1, mel2);
                    }
                }
            }
        }

        assertEquals(connectors.size() * ips.size() * ethaddrs.size() *
                     vlans.length + 1, set.size());
    }

    /**
     * Ensure that {@link MacEntryList} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        List<Set<InetAddress>> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        List<NodeConnector> connectors = createNodeConnectors(3, false);
        List<MacAddressEntry> list = new ArrayList<MacAddressEntry>();
        MacEntryList meList = null;

        // null list.
        meList = new MacEntryList(null);
        jaxbTest(meList, MacEntryList.class, XML_ROOT);
        jsonTest(meList, MacEntryList.class);

        // Empty list should be treated as null list.
        meList = new MacEntryList(list);
        jaxbTest(meList, MacEntryList.class, XML_ROOT);
        jsonTest(meList, MacEntryList.class);

        // use a combination of NodeConnectors, InetAddresses,
        // EthernetAddresses, vlans as input parameter
        for (NodeConnector nc : connectors) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ipset);
                        List<MacAddressEntry> one =
                            new ArrayList<MacAddressEntry>();
                        one.add(mae);
                        meList = new MacEntryList(one);
                        jaxbTest(meList, MacEntryList.class, XML_ROOT);
                        jsonTest(meList, MacEntryList.class);

                        list.add(mae);
                    }
                }
            }
        }

        meList = new MacEntryList(list);
        jaxbTest(meList, MacEntryList.class, XML_ROOT);
        jsonTest(meList, MacEntryList.class);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(MacEntryList.class,
                      MacEntryTest.getXmlDataTypes("macentry", XML_ROOT));
    }
}
