/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;

/**
 * JUnit test for {@link MacMap}.
 */
public class MacMapTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (Set<DataLinkHost> allow: createDataLinkHostSet(2)) {
            assertTrue(allow == null || !allow.isEmpty());
            for (Set<DataLinkHost> deny: createDataLinkHostSet(2)) {
                assertTrue(deny == null || !deny.isEmpty());
                for (List<MacAddressEntry> map: createMacAddressEntries()) {
                    assertTrue(map == null || !map.isEmpty());

                    MacMap macmap = new MacMap(allow, deny, map);
                    Set<DataLinkHost> a = macmap.getAllowedHosts();
                    Set<DataLinkHost> d = macmap.getDeniedHosts();
                    List<MacAddressEntry> m = macmap.getMappedHosts();

                    if (allow == null) {
                        assertTrue(a.isEmpty());
                    } else {
                        assertEquals(allow, a);
                        assertNotSame(allow, a);
                    }

                    if (deny == null) {
                        assertTrue(d.isEmpty());
                    } else {
                        assertEquals(deny, d);
                        assertNotSame(deny, d);
                    }

                    if (map == null) {
                        assertTrue(m.isEmpty());
                    } else {
                        assertEquals(map, m);
                        assertNotSame(map, m);
                    }
                }
            }
        }

        // Test that specifies empty set.
        Set<DataLinkHost> empty = new HashSet<DataLinkHost>();
        List<MacAddressEntry> emptyAddr = new ArrayList<MacAddressEntry>();
        MacMap macmap = new MacMap(empty, empty, emptyAddr);
        Set<DataLinkHost> a = macmap.getAllowedHosts();
        Set<DataLinkHost> d = macmap.getDeniedHosts();
        List<MacAddressEntry> m = macmap.getMappedHosts();
        assertTrue(a.isEmpty());
        assertNotSame(empty, a);
        assertTrue(d.isEmpty());
        assertNotSame(empty, d);
        assertNotSame(a, d);
        assertTrue(m.isEmpty());
        assertNotSame(emptyAddr, m);
    }

    /**
     * Test case for {@link MacMap#equals(Object)} and
     * {@link MacMap#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Set<DataLinkHost>> allowed = createDataLinkHostSet(1);
        List<Set<DataLinkHost>> denied = createDataLinkHostSet(1);
        List<List<MacAddressEntry>> mapped = createMacAddressEntries(2);
        for (Set<DataLinkHost> allow: allowed) {
            for (Set<DataLinkHost> deny: denied) {
                for (List<MacAddressEntry> map: mapped) {
                    MacMap m1 = new MacMap(allow, deny, map);
                    MacMap m2 = new MacMap(copy(allow, DataLinkHost.class),
                                           copy(deny, DataLinkHost.class),
                                           copy(map, MacAddressEntry.class));
                    testEquals(set, m1, m2);
                }
            }
        }

        assertEquals(allowed.size() * denied.size() * mapped.size(),
                     set.size());
    }

    /**
     * Test case for {@link MacMap#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "MacMap[";
        String suffix = "]";
        for (Set<DataLinkHost> allow: createDataLinkHostSet(2)) {
            for (Set<DataLinkHost> deny: createDataLinkHostSet(2)) {
                for (List<MacAddressEntry> map: createMacAddressEntries()) {
                    MacMap macmap = new MacMap(allow, deny, map);

                    // Data link host addresses are kept by unordered set.
                    // So a string representation of the object may be changed.
                    String str = macmap.toString();
                    HashSet<String> aset = new HashSet<String>();
                    HashSet<String> dset = new HashSet<String>();
                    ArrayList<String> mlist = new ArrayList<String>();
                    parseString(str, aset, dset, mlist);

                    Collection<String> aexp = (allow == null)
                        ? new HashSet<String>()
                        : toStringCollection(allow);
                    Collection<String> dexp = (deny == null)
                        ? new HashSet<String>()
                        : toStringCollection(deny);
                    Collection<String> mexp = (map == null)
                        ? new ArrayList<String>()
                        : toStringCollection(map);
                    assertEquals(aexp, aset);
                    assertEquals(dexp, dset);
                    assertEquals(mexp, mlist);
                }
            }
        }
    }

    /**
     * Ensure that {@link MacMap} is serializable.
     */
    @Test
    public void testSerialize() {
        for (Set<DataLinkHost> allow: createDataLinkHostSet(2)) {
            for (Set<DataLinkHost> deny: createDataLinkHostSet(2)) {
                for (List<MacAddressEntry> map: createMacAddressEntries()) {
                    MacMap macmap = new MacMap(allow, deny, map);
                    serializeTest(macmap);
                }
            }
        }
    }

    /**
     * Parse a string returned by {@link MacMap#toString()}.
     *
     * @param str    A string returned by {@link MacMap#toString()}.
     * @param allow  A set of strings which represents allowed hosts.
     * @param deny   A set of strings which represents denied hosts.
     * @param map    A list of strings which represents mapped hosts.
     */
    private void parseString(String str, Set<String> allow, Set<String> deny,
                             List<String> map) {
        String prefix = "MacMap[";
        assertEquals(0, str.indexOf(prefix));
        int start = prefix.length();

        String name = "EthernetHost";
        String label = "allow=";
        assertTrue(str.startsWith(label, start));
        start = parseString(name, str, start + label.length(), allow);

        label = ",deny=";
        assertTrue(str.startsWith(label, start));
        start = parseString(name, str, start + label.length(), deny);

        label = ",mapped=";
        assertTrue(str.startsWith(label, start));
        start = parseString("MacAddressEntry", str, start + label.length(),
                               map);

        assertEquals(']', str.charAt(start));
        start++;
        assertEquals(start, str.length());
    }

    /**
     * Create a list of lists of {@link MacAddressEntry} instances.
     *
     * @return  A list of lists of {@link MacAddressEntry} instances.
     */
    private List<List<MacAddressEntry>> createMacAddressEntries() {
        return createMacAddressEntries(Integer.MAX_VALUE);
    }

    /**
     * Create a list of lists of {@link MacAddressEntry} instances.
     *
     * @param max  The upper limit of the number of {@link MacAddressEntry}
     *             instances.
     * @return  A list of lists of {@link MacAddressEntry} instances.
     */
    private List<List<MacAddressEntry>> createMacAddressEntries(int max) {
        List<List<MacAddressEntry>> list =
            new ArrayList<List<MacAddressEntry>>();
        List<MacAddressEntry> mlist = new ArrayList<MacAddressEntry>();
        list.add(null);

        List<NodeConnector> ports = createNodeConnectors(4, false);
        List<Set<InetAddress>> ipaddrs = createInetAddresses();
        int portIdx = 0, ipIdx = 0;
        short vlan = 0;
        for (DataLinkAddress dladdr: createDataLinkAddresses(false)) {
            NodeConnector port = ports.get(portIdx);
            Set<InetAddress> ip = ipaddrs.get(ipIdx);
            MacAddressEntry ma = new MacAddressEntry(dladdr, vlan, port, ip);
            mlist.add(ma);
            list.add(new ArrayList<MacAddressEntry>(mlist));

            portIdx++;
            if (portIdx >= ports.size()) {
                portIdx = 0;
            }

            ipIdx++;
            if (ipIdx >= ipaddrs.size()) {
                ipIdx = 0;
            }

            vlan++;
            if ((int)vlan >= max) {
                break;
            }
        }

        return list;
    }
}
