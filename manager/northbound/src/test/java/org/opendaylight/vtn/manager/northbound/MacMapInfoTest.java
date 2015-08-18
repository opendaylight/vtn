/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
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

import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;

/**
 * JUnit test for {@link MacMapInfo}.
 */
public class MacMapInfoTest extends TestBase {
    /**
     * Test case for {@link MacMapInfo#MacMapInfo(MacMap)}
     * and getter methods.
     */
    @Test
    public void testGetter() {
        for (Set<EthernetHost> allow: createEthernetHostSet(3)) {
            for (Set<EthernetHost> deny: createEthernetHostSet(3)) {
                for (List<MacAddressEntry> map:
                         createMacAddressEntryLists(10)) {
                    MacMap mcmap = new MacMap(allow, deny, map);
                    MacMapInfo mi = new MacMapInfo(mcmap);
                    MacHostSet a = mi.getAllowedHosts();
                    MacHostSet d = mi.getDeniedHosts();
                    MacEntryList m = mi.getMappedHosts();
                    if (allow == null) {
                        assertNull(a);
                    } else {
                        assertEquals(new MacHostSet(allow), a);
                    }

                    if (deny == null) {
                        assertNull(d);
                    } else {
                        assertEquals(new MacHostSet(deny), d);
                    }

                    if (map == null) {
                        assertNull(m);
                    } else {
                        assertEquals(new MacEntryList(map), m);
                    }
                }
            }
        }
    }

    /**
     * test case for {@link MacMapInfo#equals(Object)} and
     * {@link MacMapInfo#hashCode()}
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Set<EthernetHost>> allowed = createEthernetHostSet(3);
        List<Set<EthernetHost>> denied = createEthernetHostSet(3);
        List<List<MacAddressEntry>> mapped = createMacAddressEntryLists(10);
        for (Set<EthernetHost> allow: allowed) {
            for (Set<EthernetHost> deny: denied) {
                for (List<MacAddressEntry> map: mapped) {
                    MacMap m1 = new MacMap(allow, deny, map);
                    MacMap m2 = new MacMap(
                        copy(allow, EthernetHost.class),
                        copy(deny, EthernetHost.class),
                        copy(map, MacAddressEntry.class));
                    MacMapInfo mi1 = new MacMapInfo(m1);
                    MacMapInfo mi2 = new MacMapInfo(m2);
                    testEquals(set, mi1, mi2);
                }
            }
        }

        assertEquals(allowed.size() * denied.size() * mapped.size(),
                     set.size());
    }

    /**
     * Ensure that {@link MacMapInfo} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        for (Set<EthernetHost> allow: createEthernetHostSet(3)) {
            for (Set<EthernetHost> deny: createEthernetHostSet(3)) {
                for (List<MacAddressEntry> map:
                         createMacAddressEntryLists(10)) {
                    MacMap mcmap = new MacMap(allow, deny, map);
                    MacMapInfo mi = new MacMapInfo(mcmap);
                    jaxbTest(mi, MacMapInfo.class, "macmap");
                    jsonTest(mi, MacMapInfo.class);
                }
            }
        }
    }

    /**
     * Create a list of lists of {@link MacAddressEntry} instances.
     *
     * @param limit  The upper limit of the number of instances.
     * @return  A list of lists of {@link MacAddressEntry} instances.
     */
    private List<List<MacAddressEntry>> createMacAddressEntryLists(int limit) {
        List<List<MacAddressEntry>> list =
            new ArrayList<List<MacAddressEntry>>();
        List<MacAddressEntry> mlist = new ArrayList<MacAddressEntry>();
        list.add(null);

        short[] vlans = {0, 4095};
        for (NodeConnector nc: createNodeConnectors(2, false)) {
            for (Set<InetAddress> ipset: createInetAddresses()) {
                for (short vlan : vlans) {
                    for (DataLinkAddress dladdr:
                             createDataLinkAddresses(false)) {
                        mlist.add(new MacAddressEntry(dladdr, vlan, nc,
                                                      ipset));
                        list.add(new ArrayList<MacAddressEntry>(mlist));
                        if (list.size() >= limit) {
                            break;
                        }
                    }
                }
            }
        }

        for (int i = list.size() - 1; i >= limit; i--) {
            list.remove(i);
        }

        return list;
    }
}
