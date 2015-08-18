/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.SlowTest;

import org.opendaylight.controller.sal.core.Node.NodeIDType;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link MacTableEntry}
 */
@Category(SlowTest.class)
public class MacTableEntryTest extends TestBase {
    /**
     * A list of {@link VBridgePath} instances for test.
     */
    private final List<VBridgePath>  bridgePaths;

    /**
     * The maximum number of IP address probe request.
     */
    private static final int  MAX_IP_PROBE = 10;

    /**
     * Construct a new instance.
     */
    public MacTableEntryTest() {
        bridgePaths = createVBridgePaths(true);
    }

    /**
     * Test case for getter and setter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = {-10, 0, 1, 100, 4095};

        for (VBridgePath path: bridgePaths) {
            VBridgePath bpath = new VBridgePath(path.getTenantName(),
                                                path.getBridgeName());
            for (EthernetAddress ea: createEthernetAddresses(false)) {
                long mac = EtherAddress.toLong(ea.getValue());
                for (NodeConnector nc: createNodeConnectors(3, false)) {
                    for (short vlan: vlans) {
                        for (Set<InetAddress> ips: createInetAddresses(false)) {
                            MacTableEntry me = null;
                            if (ips.size() == 0) {
                                me = new MacTableEntry(path, mac, copy(nc),
                                                       vlan, null);
                                checkProbeNeeded(me);
                            } else {
                                for (InetAddress ip: ips) {
                                    if (me == null) {
                                        me = new MacTableEntry(path, mac,
                                                               copy(nc), vlan,
                                                               ip);
                                    } else {
                                        assertTrue(me.addInetAddress(ip));
                                    }
                                    checkProbeNeeded(me);
                                }
                            }

                            checkMacTableEntry(me, ips, mac, nc, vlan);

                            InetAddress ia =
                                createInetAddress(new byte[] {10, 1, 1, 100});
                            me = new MacTableEntry(path, mac, copy(nc),
                                                   vlan, ia);
                            me.setInetAddresses(ips);
                            checkMacTableEntry(me, ips, mac, nc, vlan);

                            MacTableEntryId id = me.getEntryId();
                            assertEquals(path, id.getMapPath());
                            assertEquals(bpath, id.getBridgePath());
                            assertEquals(mac, id.getMacAddress());

                            // test for getEntry()
                            try {
                                MacAddressEntry mae1 = me.getEntry();
                                MacAddressEntry mae2 =
                                    new MacAddressEntry(copy(ea), vlan,
                                                        copy(nc), copy(ips));
                                assertEquals(mae2, mae1);
                            } catch (VTNException e) {
                                unexpected(e);
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Check fields of {@link MacTableEntry}.
     *
     * @param me    A checked {@link MacTableEntry}.
     * @param ips   A set of IP Addresses which expected to set.
     * @param mac   A expected MAC address.
     * @param nc    A expected {@link NodeConnector}.
     * @param vlan  A expected VLAN ID.
     */
    private void checkMacTableEntry(MacTableEntry me, Set<InetAddress> ips,
            long mac, NodeConnector nc, short vlan) {
        assertEquals(ips, me.getInetAddresses());
        assertEquals(mac, me.getMacAddress());
        assertEquals(nc, me.getPort());
        assertEquals(vlan, me.getVlan());
        assertTrue(me.getUsed());
    }

    /**
     * Test case for {@link MacTableEntry#setUsed()} and
     * {@link MacTableEntry#clearUsed()}
     */
    @Test
    public void testUsed() {
        short vlan = 0;
        List<NodeConnector> connectors = createNodeConnectors(1, false);
        NodeConnector nc = connectors.get(0);
        List<Set<InetAddress>> ips = createInetAddresses(false);

        Iterator<InetAddress> ip = ips.get(0).iterator();
        VBridgePath path = new VBridgePath("tenant", "bridge");
        long mac = 12345L;
        MacTableEntry me = new MacTableEntry(path, mac, nc, vlan, ip.next());

        assertTrue(me.clearUsed());
        me.setUsed();
        assertTrue(me.clearUsed());
        assertFalse(me.clearUsed());
    }

    /**
     * Test case for {@link MacTableEntry#reassignEntryId()}.
     */
    @Test
    public void testReassignId() {
        HashSet<MacTableEntryId> idSet = new HashSet<MacTableEntryId>();
        List<MacTableEntry> entries = createMacTableEntries();
        int loop = 3;
        for (MacTableEntry tent: entries) {
            MacTableEntryId id =  tent.getEntryId();
            assertTrue(tent.toString(), idSet.add(id));

            for (int i = 0; i < loop; i++) {
                MacTableEntryId newId = tent.reassignEntryId();
                assertEquals(newId, tent.getEntryId());
                assertTrue(idSet.add(newId));
            }
            assertEquals((loop + 1), idSet.size());
            idSet.clear();
        }
    }

    /**
     * Test case for
     * {@link MacTableEntry#hasMoved(NodeConnector, short, VBridgePath)}.
     */
    @Test
    public void testHasMoved() {
        String unknown = "unknown";
        NodeConnector unknownPort = null;
        try {
            Node node = new Node(NodeIDType.PRODUCTION, unknown);
            unknownPort = new NodeConnector(NodeConnectorIDType.PRODUCTION,
                                            unknown, node);
        } catch (Exception e) {
            unexpected(e);
        }

        VBridgeIfPath unknownPath =
            new VBridgeIfPath(unknown, unknown, unknown);

        for (MacTableEntry tent: createMacTableEntries()) {
            MacTableEntryId id = tent.getEntryId();
            VBridgePath mapPath = id.getMapPath();
            short vlan = tent.getVlan();
            NodeConnector port = copy(tent.getPort());
            assertFalse(tent.hasMoved(port, vlan, mapPath));

            short otherVlan = (vlan == 0) ? (short)1 : (short)(vlan - 1);
            assertTrue(tent.hasMoved(unknownPort, vlan, mapPath));
            assertTrue(tent.hasMoved(port, otherVlan, mapPath));
            assertTrue(tent.hasMoved(port, vlan, unknownPath));
            assertTrue(tent.hasMoved(unknownPort, otherVlan, mapPath));
            assertTrue(tent.hasMoved(unknownPort, vlan, unknownPath));
            assertTrue(tent.hasMoved(port, otherVlan, unknownPath));
        }
    }

    /**
     * Test case for {@link MacTableEntry#equals(Object)} and
     * {@link MacTableEntry#hashCode()}.
     */
    @Test
    public void testEquals() {
        List<MacTableEntry> entries = createMacTableEntries();
        HashSet<Object> set = new HashSet<Object>(entries.size());
        for (MacTableEntry tent1: entries) {
            MacTableEntryId id = tent1.getEntryId();
            MacTableEntryId newId =
                new MacTableEntryId(id.getControllerAddress(), id.getEventId(),
                                    id.getMapPath(), id.getMacAddress());
            NodeConnector port = copy(tent1.getPort());
            short vlan = tent1.getVlan();
            MacTableEntry tent2 = new MacTableEntry(newId, port, vlan, null);
            for (InetAddress addr: tent1.getInetAddresses()) {
                tent2.addInetAddress(copy(addr));
            }
            testEquals(set, tent1, tent2);

            // Ensure that "used" and "probeCount" never affect object
            // identity.
            tent1.clearUsed();
            assertEquals(tent2, tent1);
            assertTrue(set.contains(tent1));
            assertTrue(set.contains(tent2));

            tent1.isProbeNeeded();
            assertEquals(tent2, tent1);
            assertTrue(set.contains(tent1));
            assertTrue(set.contains(tent2));
        }

        assertEquals(entries.size(), set.size());
    }

    /**
     * Test case for {@link MacTableEntry#toString()}
     */
    @Test
    public void testToString() {
        String prefix = "MacTableEntry[";
        String suffix = "]";
        short[] vlans = {-10, 0, 1, 100, 4095};

        for (VBridgePath path: bridgePaths) {
            for (EthernetAddress ea: createEthernetAddresses(false)) {
                long mac = EtherAddress.toLong(ea.getValue());
                for (NodeConnector nc: createNodeConnectors(3, false)) {
                    for (short vlan: vlans) {
                        for (Set<InetAddress> ips: createInetAddresses(false)) {
                            MacTableEntry me = null;
                            if (ips.size() == 0) {
                                me = new MacTableEntry(path, mac, nc, vlan,
                                                       null);
                            } else {
                                for (InetAddress ip: ips) {
                                    if (me == null) {
                                        me = new MacTableEntry(path, mac, nc,
                                                               vlan, ip);
                                    } else {
                                        assertTrue(me.addInetAddress(ip));
                                    }
                                }
                            }

                            String id = "id=" + me.getEntryId();
                            String p = "port=" + nc.toString();
                            String v = "vlan=" + vlan;
                            String i = "ipaddr=" + toString(ips);
                            String required = joinStrings(prefix, suffix, ",",
                                                          id, p, v, i);
                            assertEquals(required, me.toString());
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link PortVlan} is serializable.
     */
    @Test
    public void testSerialize() {
        for (MacTableEntry tent1: createMacTableEntries()) {
            MacTableEntry tent2 = (MacTableEntry)serializeTest(tent1);
            assertTrue(tent2.getUsed());
            assertFalse(tent2.isProbeNeeded());
        }
    }

    /**
     * Create a list of {@link MacTableEntry} objects.
     *
     * @return  A list of {@link MacTableEntry} objects.
     */
    private List<MacTableEntry> createMacTableEntries() {
        List<MacTableEntry> list = new ArrayList<MacTableEntry>();
        short[] vlans = {0, 1, 4095};
        long[] macAddrs = {
            0x000000000001L,
            0xf0123456789aL,
        };

        for (VBridgePath path: bridgePaths) {
            for (long mac: macAddrs) {
                MacTableEntryId id = new MacTableEntryId(path, mac);
                for (NodeConnector nc: createNodeConnectors(2, false)) {
                    for (short vlan: vlans) {
                        for (Set<InetAddress> ips: createInetAddresses(false)) {
                            MacTableEntry me = null;
                            if (ips.size() == 0) {
                                me = new MacTableEntry(id, copy(nc), vlan, null);
                            } else {
                                for (InetAddress ip: ips) {
                                    if (me == null) {
                                        me = new MacTableEntry(id, copy(nc),
                                                               vlan, ip);
                                    } else {
                                        assertTrue(me.addInetAddress(ip));
                                    }
                                }
                            }
                            list.add(me);
                        }
                    }
                }
            }
        }

        return list;
    }

    /**
     * Return a string representation of a set of IP addresses.
     *
     * @param addrs  A set of IP addresses.
     * @return  A string representation of the given set of IP addresses.
     */
    private String toString(Set<InetAddress> addrs) {
        char sep = 0;
        StringBuilder builder = new StringBuilder("{");
        for (InetAddress addr: addrs) {
            if (sep == 0) {
                sep = ',';
            } else {
                builder.append(sep);
            }
            builder.append(addr.getHostAddress());
        }

        return builder.append('}').toString();
    }

    /**
     * Ensure that {@link MacTableEntry#isProbeNeeded()} works.
     *
     * @param tent  A {@link MacTableEntry} instance to be tested.
     */
    private void checkProbeNeeded(MacTableEntry tent) {
        if (tent.getInetAddresses().isEmpty()) {
            for (int i = 0; i < MAX_IP_PROBE; i++) {
                assertTrue(tent.isProbeNeeded());
            }
        }
        for (int i = 0; i < 10; i++) {
            assertFalse(tent.isProbeNeeded());
        }
    }
}
