/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.Set;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;
import java.util.ArrayList;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.MacMapActivation;
import org.opendaylight.vtn.manager.internal.MacMapDuplicateException;
import org.opendaylight.vtn.manager.internal.NodePortFilter;
import org.opendaylight.vtn.manager.internal.PortFilter;
import org.opendaylight.vtn.manager.internal.SpecificPortFilter;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;

/**
 * JUnit test for {@link MacMapState}.
 */
public class MacMapStateTest extends TestBase {
    /**
     * A list of {@link NodeConnector} instances.
     */
    private final List<NodeConnector>  portList =
        new ArrayList<NodeConnector>();

    /**
     * Cursor into {@link #portList} array.
     */
    private int  portListIndex;

    /**
     * A list of {@link NodeConnector} instances which are never used for test.
     */
    private final List<NodeConnector> unusedPortList =
        new ArrayList<NodeConnector>();

    /**
     * A class to keep networks mapped to a {@link MacMapState} instance.
     */
    private static class MappedNetwork {
        /**
         * A map to pairs of {@link PortVlan} instance and its reference
         * counter.
         */
        private final Map<PortVlan, Integer>  networkMap =
            new HashMap<PortVlan, Integer>();

        /**
         * Construct a new instance.
         */
        private MappedNetwork() {
        }

        /**
         * Construct a new instance which keeps the same contents as the
         * specified instance.
         *
         * @param nw  A {@link MappedNetwork} instance to be copied.
         */
        private MappedNetwork(MappedNetwork nw) {
            networkMap.putAll(nw.networkMap);
        }

        /**
         * Append the specified network.
         *
         * @param pvlan  A {@link PortVlan} instance to be added.
         */
        private void add(PortVlan pvlan) {
            Integer count = networkMap.get(pvlan);
            int cnt = (count == null) ? 1 : count.intValue() + 1;
            networkMap.put(pvlan, Integer.valueOf(cnt));
        }

        /**
         * Remove the specified network.
         *
         * @param pvlan  A {@link PortVlan} instance to be removed.
         * @return  {@code true} is returned if the specified {@link PortVlan}
         *          was removed from this instance.
         *          {@code false} is returned if it is still kept by this
         *          instance.
         */
        private boolean remove(PortVlan pvlan) {
            Integer count = networkMap.remove(pvlan);
            assertNotNull(count);

            int cnt = count.intValue();
            boolean removed;
            if (cnt > 1) {
                networkMap.put(pvlan, Integer.valueOf(cnt - 1));
                removed = false;
            } else {
                removed = true;
            }

            return removed;
        }

        /**
         * Force to remove the specified network.
         *
         * @param pvlan  A {@link PortVlan} instance to be removed.
         */
        private void removeForce(PortVlan pvlan) {
            networkMap.remove(pvlan);
        }

        /**
         * Force to remove all networks on the specified port.
         *
         * @param filter  A {@link PortFilter} instance which determines
         *                ports to be removed.
         * @return  A set of {@link PortVlan} instances which were actually
         *          removed is returned.
         */
        private Set<PortVlan> removeForce(PortFilter filter) {
            Set<PortVlan> removed = new HashSet<PortVlan>();
            for (Iterator<PortVlan> it = networkMap.keySet().iterator();
                 it.hasNext();) {
                PortVlan pvlan = it.next();
                NodeConnector port = pvlan.getNodeConnector();
                if (filter.accept(port, null)) {
                    removed.add(pvlan);
                    it.remove();
                }
            }

            return removed;
        }

        /**
         * Return a set of {@link PortVlan} instances.
         *
         * @return  A set of {@link PortVlan} instances.
         */
        private Set<PortVlan> getNetworks() {
            return networkMap.keySet();
        }
    }

    /**
     * Construct a new instance.
     */
    public MacMapStateTest() {
        // Create node connectors for test.
        for (long dpid = 0; dpid < 2; dpid++) {
            Node node = NodeCreator.createOFNode(Long.valueOf(dpid));
            for (short id = 1; id <= 5; id++) {
                NodeConnector port = NodeConnectorCreator.
                    createOFNodeConnector(Short.valueOf(id), node);
                assertNotNull(port);
                portList.add(port);
            }

            for (short id = 100; id <= 102; id++) {
                NodeConnector port = NodeConnectorCreator.
                    createOFNodeConnector(Short.valueOf(id), node);
                assertNotNull(port);
                unusedPortList.add(port);
            }
        }
    }

    /**
     * Ensure that {@link MacMapState} maintains mapped host information
     * correctly.
     *
     * <ul>
     *   <li>{@link MacMapState#activate(MapReference, MacVlan, NodeConnector)}</li>
     *   <li>{@link MacMapState#getPort(MacVlan)}</li>
     *   <li>{@link MacMapState#getPortVlan(long)}</li>
     *   <li>{@link MacMapState#hasMapping(PortVlan)}</li>
     *   <li>{@link MacMapState#hasMapping()}</li>
     *   <li>{@link MacMapState#getNetworks()}</li>
     *   <li>{@link MacMapState#getActiveHosts()}</li>
     *   <li>{@link MacMapState#getDuplicate(MacVlan)}</li>
     *   <li>{@link MacMapState#isDirty()}</li>
     *   <li>{@link MacMapState#inactivate(MacVlan, Set)}</li>
     *   <li>{@link MacMapState#inactivate(PortVlan)}</li>
     *   <li>{@link MacMapState#inactivate(PortFilter, Set)}</li>
     *   <li>{@link MacMapState#inactivate(Map, MapReference, Set, Set)}</li>
     * </ul>
     */
    @Test
    public void testState() {
        // Test case for an empty instance.
        MacMapState mst = new MacMapState();
        checkEmtpy(mst);
        assertFalse(mst.isDirty());

        MappedNetwork nwMap = new MappedNetwork();
        Map<MacVlan, NodeConnector> hostMap =
            new HashMap<MacVlan, NodeConnector>();

        // Activate MAC mappings with specifying MAC addresses in which the
        // MSB is not set.
        VBridgePath bpath = new VBridgePath("tenant", "bridge");
        MacMapPath mpath = new MacMapPath(bpath);
        MapReference ref = new MapReference(MapType.MAC, "default", mpath);
        short[] vlans = {0, 1, 4094, 4095};
        long mac = 0x1000L;
        for (int i = 0; i < 10; i++) {
            for (short vlan: vlans) {
                checkActivate(mst, ref, mac, vlan, null, nwMap, hostMap);
                mac++;
            }
        }

        // Activate MAC mappings with specifying MAC addresses in which the
        // MSB is set.
        mac = 0xfeffffffff00L;
        for (int i = 0; i < 10; i++) {
            for (short vlan: vlans) {
                checkActivate(mst, ref, mac, vlan, null, nwMap, hostMap);
                mac++;
            }
        }

        // Ensure that duplicate MAC addresses are rejected.
        for (Map.Entry<MacVlan, NodeConnector> entry: hostMap.entrySet()) {
            MacVlan mapped = entry.getKey();
            NodeConnector port = entry.getValue();
            mac = mapped.getMacAddress();
            MacVlan mvlan = new MacVlan(mac, (short)2000);

            assertEquals(mapped, mst.getDuplicate(mvlan));
            try {
                mst.activate(ref, mvlan, port);
                fail("An exception must be thrown.");
            } catch (MacMapDuplicateException e) {
                assertEquals(mvlan, e.getHost());
                assertEquals(ref, e.getMapReference());
                assertEquals(mapped, e.getDuplicate());
                assertFalse(mst.isDirty());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        Set<PortVlan> nw = mst.getNetworks();
        Map<MacVlan, NodeConnector> hosts = mst.getActiveHosts();
        assertEquals(nwMap.getNetworks(), nw);
        assertEquals(hostMap, hosts);

        // Ensure that the MacMapState is never changed even if returned
        // Set/Map are cleared.
        assertFalse(nw.isEmpty());
        assertFalse(hosts.isEmpty());
        nw.clear();
        hosts.clear();
        assertTrue(nw.isEmpty());
        assertTrue(hosts.isEmpty());
        assertEquals(nwMap.getNetworks(), mst.getNetworks());
        assertEquals(hostMap, mst.getActiveHosts());

        // Move all hosts to one port.
        NodeConnector newPort = unusedPortList.get(0);
        Map<MacVlan, NodeConnector> hmap =
            new HashMap<MacVlan, NodeConnector>(hostMap);
        for (Map.Entry<MacVlan, NodeConnector> entry: hmap.entrySet()) {
            MacVlan mvlan = entry.getKey();
            mac = mvlan.getMacAddress();
            short vlan = mvlan.getVlan();
            NodeConnector port = entry.getValue();
            PortVlan pvlan = new PortVlan(port, vlan);
            assertNull(mst.getDuplicate(mvlan));
            assertEquals(pvlan, mst.getPortVlan(mac));
            assertEquals(port, mst.getPort(mvlan));
            assertFalse(mst.isDirty());
            try {
                MacMapActivation result = mst.activate(ref, mvlan, newPort);
                assertTrue(mst.isDirty());
                assertFalse(mst.isDirty());
                assertFalse(result.isActivated());
                assertEquals(port, result.getOldPort());

                assertNull(mst.activate(ref, mvlan, newPort));
                assertFalse(mst.isDirty());
            } catch (Exception e) {
                unexpected(e);
            }

            nwMap.remove(pvlan);
            pvlan = new PortVlan(newPort, mvlan.getVlan());
            assertNull(mst.getDuplicate(mvlan));
            assertEquals(pvlan, mst.getPortVlan(mac));
            assertEquals(newPort, mst.getPort(mvlan));

            nwMap.add(pvlan);
            hostMap.put(mvlan, newPort);
            assertEquals(nwMap.getNetworks(), mst.getNetworks());
            assertEquals(hostMap, mst.getActiveHosts());
            assertTrue(mst.hasMapping());
        }

        // Restore mappings.
        checkActivate(mst, ref, hmap, nwMap, hostMap);
        assertEquals(hmap, hostMap);

        // Inactivate all hosts by specifying MacVlan instance.
        for (Map.Entry<MacVlan, NodeConnector> entry: hmap.entrySet()) {
            assertTrue(mst.hasMapping());
            MacVlan mvlan = entry.getKey();
            mac = mvlan.getMacAddress();
            NodeConnector port = entry.getValue();
            PortVlan pvlan = new PortVlan(port, mvlan.getVlan());
            assertTrue(mst.hasMapping(pvlan));

            Set<PortVlan> released = new HashSet<PortVlan>();
            assertEquals(port, mst.inactivate(mvlan, released));
            assertTrue(mst.isDirty());
            assertFalse(mst.isDirty());

            boolean removed = nwMap.remove(pvlan);
            if (removed) {
                assertEquals(1, released.size());
                assertTrue(released.contains(pvlan));
            } else {
                assertTrue(released.isEmpty());
            }
            assertEquals(!removed, mst.hasMapping(pvlan));
            assertEquals(port, hostMap.remove(mvlan));

            assertNull(mst.getDuplicate(mvlan));
            assertNull(mst.getPortVlan(mac));
            assertNull(mst.getPort(mvlan));

            // Try to inactivate the same host.
            released.clear();
            assertNull(mst.inactivate(mvlan, released));
            assertFalse(mst.isDirty());
            assertTrue(released.isEmpty());
        }
        assertFalse(mst.hasMapping());

        // Restore mappings again.
        checkActivate(mst, ref, hmap, nwMap, hostMap);
        assertEquals(hmap, hostMap);

        // Inactivate all hosts by specifying PortVlan instance.
        Set<PortVlan> nwSet = new HashSet<PortVlan>(nwMap.getNetworks());
        for (PortVlan pvlan: nwSet) {
            assertTrue(mst.hasMapping());
            assertTrue(mst.hasMapping(pvlan));
            Set<MacVlan> removed = mst.inactivate(pvlan);
            assertTrue(mst.isDirty());
            assertFalse(mst.isDirty());
            assertEquals(removeHosts(hostMap, pvlan).keySet(), removed);
            nwMap.removeForce(pvlan);

            assertFalse(mst.hasMapping(pvlan));
            Map<MacVlan, NodeConnector> hmap1 = hostMap;
            Set<PortVlan> nw1 = nwMap.getNetworks();
            if (nw1.isEmpty()) {
                nw1 = null;
                hmap1 = null;
            }
            assertEquals(nw1, mst.getNetworks());
            assertEquals(hmap1, mst.getActiveHosts());

            for (MacVlan mvlan: removed) {
                assertNull(mst.getDuplicate(mvlan));
                assertNull(mst.getPortVlan(mvlan.getMacAddress()));
                assertNull(mst.getPort(mvlan));
            }

            // Try to inactivate the same network.
            assertNull(mst.inactivate(pvlan));
            assertFalse(mst.isDirty());
        }
        assertFalse(mst.hasMapping());

        // Restore mappings again.
        checkActivate(mst, ref, hmap, nwMap, hostMap);
        assertEquals(hmap, hostMap);

        // Inactivate all hosts on the specified switch.
        Node removedNode = portList.get(0).getNode();
        PortFilter filter = new NodePortFilter(removedNode);
        Set<PortVlan> released = new HashSet<PortVlan>();
        Map<MacVlan, NodeConnector> removedMap =
            mst.inactivate(filter, released);
        assertTrue(mst.isDirty());
        assertFalse(mst.isDirty());
        assertFalse(removedMap.isEmpty());
        assertFalse(released.isEmpty());
        assertEquals(removedMap, removeHosts(hostMap, filter));
        assertEquals(released, nwMap.removeForce(filter));
        assertEquals(nwMap.getNetworks(), mst.getNetworks());
        assertEquals(hostMap, mst.getActiveHosts());
        for (Map.Entry<MacVlan, NodeConnector> entry: removedMap.entrySet()) {
            MacVlan mvlan = entry.getKey();
            NodeConnector port = entry.getValue();
            assertEquals(removedNode, port.getNode());
            assertNull(mst.getDuplicate(mvlan));
            assertNull(mst.getPortVlan(mvlan.getMacAddress()));
            assertNull(mst.getPort(mvlan));
            PortVlan pvlan = new PortVlan(port, mvlan.getVlan());
            assertFalse(mst.hasMapping(pvlan));
        }
        assertTrue(mst.hasMapping());

        // Try to inactivate again.
        released.clear();
        assertTrue(mst.inactivate(filter, released).isEmpty());
        assertFalse(mst.isDirty());
        assertTrue(released.isEmpty());

        // Inactivate all hosts by specifying switch port.
        for (NodeConnector port: portList) {
            Node node = port.getNode();
            boolean inactivated = node.equals(removedNode);
            filter = new SpecificPortFilter(port);
            released.clear();
            removedMap = mst.inactivate(filter, released);
            assertEquals(!inactivated, mst.isDirty());
            assertFalse(mst.isDirty());
            assertEquals(inactivated, removedMap.isEmpty());
            assertEquals(inactivated, released.isEmpty());
            assertEquals(removedMap, removeHosts(hostMap, filter));
            assertEquals(released, nwMap.removeForce(filter));
            Map<MacVlan, NodeConnector> hmap1 = hostMap;
            Set<PortVlan> nw1 = nwMap.getNetworks();
            if (nw1.isEmpty()) {
                nw1 = null;
                hmap1 = null;
            }
            assertEquals(nw1, mst.getNetworks());
            assertEquals(hmap1, mst.getActiveHosts());

            for (Map.Entry<MacVlan, NodeConnector> entry:
                     removedMap.entrySet()) {
                MacVlan mvlan = entry.getKey();
                assertEquals(port, entry.getValue());
                assertFalse(removedNode.equals(port.getNode()));
                assertNull(mst.getDuplicate(mvlan));
                assertNull(mst.getPortVlan(mvlan.getMacAddress()));
                assertNull(mst.getPort(mvlan));
                PortVlan pvlan = new PortVlan(port, mvlan.getVlan());
                assertFalse(mst.hasMapping(pvlan));
            }

            // Try to inactivate again.
            released.clear();
            assertTrue(mst.inactivate(filter, released).isEmpty());
            assertFalse(mst.isDirty());
            assertTrue(released.isEmpty());
        }
        assertFalse(mst.hasMapping());

        // Restore mappings again.
        checkActivate(mst, ref, hmap, nwMap, hostMap);
        assertEquals(hmap, hostMap);

        // Test case for inactivate(Map, MapReference, Set, Set).
        // In order to simplify the test code, hosts to be inactivated are
        // determined by PortVlan instances.
        // At first, choose 2 arbitrary VLAN networks.
        Set<PortVlan> expectedNw = new HashSet<PortVlan>();
        Map<MacVlan, NodeConnector> expectedHosts =
            new HashMap<MacVlan, NodeConnector>();
        Set<Short> unmappedVlans = new HashSet<Short>();
        for (int i = 1; i <= 2; i++) {
            int idx = (i * i) + 2;
            short vlan = vlans[i];
            NodeConnector port = portList.get(idx);
            PortVlan pvlan = new PortVlan(port, vlan);
            expectedNw.add(pvlan);
            nwMap.removeForce(pvlan);
            Map<MacVlan, NodeConnector> m = removeHosts(hostMap, pvlan);
            assertFalse(m.isEmpty());
            expectedHosts.putAll(m);

            // This test case assumes that this host is mapped by MAC mapping
            // with wildcard MAC address. So this VLAN ID needs to be added
            // to unmappedVlans.
            unmappedVlans.add(vlan);
        }
        assertFalse(expectedNw.isEmpty());
        assertFalse(expectedHosts.isEmpty());
        assertFalse(unmappedVlans.isEmpty());

        // Construct an allowed host set.
        Map<MacVlan, MapReference> allowed =
            new HashMap<MacVlan, MapReference>();
        for (MacVlan mvlan: hmap.keySet()) {
            short vlan = mvlan.getVlan();
            if (unmappedVlans.contains(vlan) &&
                !expectedHosts.containsKey(mvlan)) {
                // This host should be retained.
                allowed.put(mvlan, ref);
            }
        }
        assertFalse(allowed.isEmpty());

        // Inactivate all hosts on the specified VLAN except for hosts
        // which are mapped explicitly.
        released.clear();
        removedMap = mst.inactivate(allowed, ref, unmappedVlans, released);
        assertTrue(mst.isDirty());
        assertFalse(mst.isDirty());
        assertFalse(removedMap.isEmpty());
        assertFalse(released.isEmpty());
        assertEquals(removedMap, expectedHosts);
        assertEquals(released, expectedNw);
        assertEquals(nwMap.getNetworks(), mst.getNetworks());
        assertEquals(hostMap, mst.getActiveHosts());
        for (Map.Entry<MacVlan, NodeConnector> entry: removedMap.entrySet()) {
            MacVlan mvlan = entry.getKey();
            NodeConnector port = entry.getValue();
            assertNull(mst.getDuplicate(mvlan));
            assertNull(mst.getPortVlan(mvlan.getMacAddress()));
            assertNull(mst.getPort(mvlan));
            PortVlan pvlan = new PortVlan(port, mvlan.getVlan());
            assertFalse(mst.hasMapping(pvlan));
        }
        assertTrue(mst.hasMapping());

        // Try to inactivate again.
        released.clear();
        assertTrue(mst.inactivate(allowed, ref, unmappedVlans, released).
                   isEmpty());
        assertFalse(mst.isDirty());
        assertTrue(released.isEmpty());
        assertTrue(mst.hasMapping());

        // No host should be inactivated if unmappedVlans is empty.
        allowed.clear();
        unmappedVlans.clear();
        assertTrue(mst.inactivate(allowed, ref, unmappedVlans, released).
                   isEmpty());
        assertFalse(mst.isDirty());
        assertTrue(released.isEmpty());
        assertTrue(mst.hasMapping());

        // No host should be inactivated if all hosts are explicitly mapped.
        for (short vlan: vlans) {
            unmappedVlans.add(vlan);
        }
        allowed.clear();
        for (MacVlan mvlan: hostMap.keySet()) {
            allowed.put(mvlan, ref);
        }
        assertTrue(mst.inactivate(allowed, ref, unmappedVlans, released).
                   isEmpty());
        assertFalse(mst.isDirty());
        assertTrue(released.isEmpty());
        assertTrue(mst.hasMapping());

        // All hosts should be inactivated if no host is explicitly mapped
        // and all VLANs are unmapped.
        allowed.clear();
        assertEquals(hostMap,
                     mst.inactivate(allowed, ref, unmappedVlans, released));
        assertTrue(mst.isDirty());
        assertFalse(mst.isDirty());
        assertEquals(nwMap.getNetworks(), released);
        assertFalse(mst.hasMapping());
        assertNull(mst.getNetworks());
        assertNull(mst.getActiveHosts());
        for (Map.Entry<MacVlan, NodeConnector> entry: hostMap.entrySet()) {
            MacVlan mvlan = entry.getKey();
            NodeConnector port = entry.getValue();
            assertNull(mst.getDuplicate(mvlan));
            assertNull(mst.getPortVlan(mvlan.getMacAddress()));
            assertNull(mst.getPort(mvlan));
            PortVlan pvlan = new PortVlan(port, mvlan.getVlan());
            assertFalse(mst.hasMapping(pvlan));
        }

        // Try to inactivate again.
        released.clear();
        assertTrue(mst.inactivate(allowed, ref, unmappedVlans, released).
                   isEmpty());
        assertFalse(mst.isDirty());
        assertTrue(released.isEmpty());
        assertFalse(mst.hasMapping());
    }

    /**
     * Test case for {@link MacMapState#equals(Object)} and
     * {@link MacMapState#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] vlans = {0, 1, 10, 100, 4094, 4095};
        Set<MacVlan> hosts = new HashSet<MacVlan>();
        long mac = 1L;
        for (int i = 0; i < 5; i++) {
            for (short vlan: vlans) {
                hosts.add(new MacVlan(mac, vlan));
                mac++;
            }
        }
        mac = 0x7fffffffL;
        for (int i = 0; i < 5; i++) {
            for (short vlan: vlans) {
                hosts.add(new MacVlan(mac, vlan));
                mac++;
            }
        }
        mac = 0xf0ffffffff00L;
        for (int i = 0; i < 5; i++) {
            for (short vlan: vlans) {
                hosts.add(new MacVlan(mac, vlan));
                mac++;
            }
        }

        VBridgePath bpath = new VBridgePath("tenant", "bridge");
        MacMapPath mpath = new MacMapPath(bpath);
        MapReference ref = new MapReference(MapType.MAC, "default", mpath);
        MacMapState mst1 = new MacMapState();
        MacMapState mst2 = new MacMapState();
        testEquals(set, mst1.clone(), mst2.clone());

        try {
            for (MacVlan mvlan: hosts) {
                NodeConnector port = getPort();
                MacMapActivation result = mst1.activate(ref, mvlan, port);
                assertNull(result.getOldPort());
                result = mst2.activate(ref,
                                       new MacVlan(mvlan.getEncodedValue()),
                                       copy(port));
                assertNull(result.getOldPort());
                assertEquals(mst2, mst1);
                testEquals(set, mst1.clone(), mst2.clone());

                // Dirty flag should not affect object identity.
                assertTrue(mst1.isDirty());
                assertFalse(mst1.isDirty());
                assertEquals(mst2, mst1);
                assertTrue(mst2.isDirty());
                assertFalse(mst2.isDirty());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Ensure that {@link MacMapPath} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {0, 1, 10, 100, 4094, 4095};
        Set<MacVlan> hosts = new HashSet<MacVlan>();
        long mac = 1L;
        for (int i = 0; i < 5; i++) {
            for (short vlan: vlans) {
                hosts.add(new MacVlan(mac, vlan));
                mac++;
            }
        }
        mac = 0x7fffffffL;
        for (int i = 0; i < 5; i++) {
            for (short vlan: vlans) {
                hosts.add(new MacVlan(mac, vlan));
                mac++;
            }
        }
        mac = 0xf0ffffffff00L;
        for (int i = 0; i < 5; i++) {
            for (short vlan: vlans) {
                hosts.add(new MacVlan(mac, vlan));
                mac++;
            }
        }

        VBridgePath bpath = new VBridgePath("tenant", "bridge");
        MacMapPath mpath = new MacMapPath(bpath);
        MapReference ref = new MapReference(MapType.MAC, "default", mpath);
        MacMapState mst = new MacMapState();
        MacMapState dmst = (MacMapState)serializeTest(mst);
        MacMapState[] array = {mst, dmst};
        for (MacMapState m: array) {
            assertFalse(m.isDirty());
            assertNull(m.getNetworks());
            assertNull(m.getActiveHosts());
        }

        MappedNetwork nwMap = new MappedNetwork();
        Map<MacVlan, NodeConnector> hostMap =
            new HashMap<MacVlan, NodeConnector>();

        try {
            for (MacVlan mvlan: hosts) {
                NodeConnector port = getPort();
                nwMap.add(new PortVlan(port, mvlan.getVlan()));
                hostMap.put(mvlan, port);

                MacMapActivation result = mst.activate(ref, mvlan, port);
                assertNull(result.getOldPort());
                dmst = (MacMapState)serializeTest(mst);
                array[1] = dmst;

                // Dirty flag in deserialized object should be cleared.
                assertTrue(mst.isDirty());

                for (MacMapState m: array) {
                    assertFalse(m.isDirty());
                    assertEquals(nwMap.getNetworks(), m.getNetworks());
                    assertEquals(hostMap, m.getActiveHosts());
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }

        // Ensure that all hosts in the deserialized object can be inactivated.
        inactivateAll(mst);
        inactivateAll(dmst);
    }

    /**
     * Test case for {@link MacMapState#clone()}.
     */
    @Test
    public void testClone() {
        // Create a clone of empty instance.
        MacMapState mst = new MacMapState();
        checkEmtpy(mst);
        MacMapState cmst = mst.clone();
        assertEquals(mst, cmst);
        checkEmtpy(cmst);

        // Activate some hosts.
        MappedNetwork nwMap = new MappedNetwork();
        Map<MacVlan, NodeConnector> hostMap =
            new HashMap<MacVlan, NodeConnector>();
        VBridgePath bpath = new VBridgePath("tenant", "bridge");
        MacMapPath mpath = new MacMapPath(bpath);
        MapReference ref = new MapReference(MapType.MAC, "default", mpath);

        short[] vlans = {0, 1, 4094, 4095};
        long mac = 0xfff000L;
        for (int i = 0; i < 10; i++) {
            for (short vlan: vlans) {
                checkActivate(mst, ref, mac, vlan, null, nwMap, hostMap);
                mac++;
            }
        }

        // Create a clone.
        cmst = mst.clone();
        assertEquals(mst, cmst);
        assertEquals(nwMap.getNetworks(), cmst.getNetworks());
        assertEquals(hostMap, cmst.getActiveHosts());

        // Activate more hosts to the clone.
        mac = 0xa00000000000L;
        MappedNetwork newNwMap = new MappedNetwork(nwMap);
        Map<MacVlan, NodeConnector> newHostMap =
            new HashMap<MacVlan, NodeConnector>(hostMap);
        for (int i = 0; i < 10; i++) {
            for (short vlan: vlans) {
                checkActivate(cmst, ref, mac, vlan, null, newNwMap,
                              newHostMap);
                mac++;
            }
        }

        // Changes to cloned object should not affect original.
        assertFalse(mst.equals(cmst));
        assertEquals(nwMap.getNetworks(), mst.getNetworks());
        assertEquals(hostMap, mst.getActiveHosts());
        assertEquals(newNwMap.getNetworks(), cmst.getNetworks());
        assertEquals(newHostMap, cmst.getActiveHosts());

        // Activate one more host.
        NodeConnector port = getPort();
        MacVlan mvlan = new MacVlan(0xe00000000000L, (short)300);
        newNwMap.add(new PortVlan(port, mvlan.getVlan()));
        newHostMap.put(mvlan, port);
        try {
            MacMapActivation result = cmst.activate(ref, mvlan, port);
            assertNull(result.getOldPort());
        } catch (Exception e) {
            unexpected(e);
        }

        // Dirty flag should be copied.
        MacMapState cmst1 = cmst.clone();
        assertTrue(cmst.isDirty());
        assertFalse(cmst.isDirty());
        assertEquals(cmst, cmst1);
        assertTrue(cmst1.isDirty());
        assertFalse(cmst1.isDirty());

        assertEquals(newNwMap.getNetworks(), cmst.getNetworks());
        assertEquals(newHostMap, cmst.getActiveHosts());
        assertEquals(newNwMap.getNetworks(), cmst1.getNetworks());
        assertEquals(newHostMap, cmst1.getActiveHosts());

        // Ensure that all hosts in a cloned object can be inactivated.
        inactivateAll(cmst);
        inactivateAll(cmst1);
    }

    /**
     * Ensure that the specified {@link MacMapState} instance is empty.
     *
     * @param mst    A {@link MacMapState} instance to be tested.
     */
    private void checkEmtpy(MacMapState mst) {
        assertFalse(mst.hasMapping());
        assertNull(mst.getNetworks());
        assertNull(mst.getActiveHosts());

        for (long mac = 1L; mac <= 10; mac++) {
            assertNull(mst.getPortVlan(mac));
            for (short vlan = 0; vlan < 10; vlan++) {
                MacVlan mvlan = new MacVlan(mac, vlan);
                assertNull(mst.getPort(mvlan));
                assertNull(mst.getDuplicate(mvlan));
            }
        }

        for (NodeConnector port: portList) {
            for (short vlan = 0; vlan < 10; vlan++) {
                PortVlan pvlan = new PortVlan(port, vlan);
                assertFalse(mst.hasMapping(pvlan));
            }
        }
    }

    /**
     * Return a {@link NodeConnector} for test.
     *
     * @return  A {@link NodeConnector} instance.
     */
    private NodeConnector getPort() {
        int idx = portListIndex;
        NodeConnector port = portList.get(idx);
        idx++;
        portListIndex = (idx >= portList.size()) ? 0 : idx;

        return port;
    }

    /**
     * Activate hosts specified by the map.
     *
     * @param mst      A {@link MacMapState} instance to be tested.
     * @param ref      A reference to the MAC mapping.
     * @param hosts    A map which contains hosts to be activated.
     * @param nwMap    A {@link MappedNetwork} instance to store mapped
     *                 networks.
     * @param hostMap  A map to store activated hosts.
     */
    private void checkActivate(MacMapState mst, MapReference ref,
                               Map<MacVlan, NodeConnector> hosts,
                               MappedNetwork nwMap,
                               Map<MacVlan, NodeConnector> hostMap) {
        for (Map.Entry<MacVlan, NodeConnector> entry: hosts.entrySet()) {
            MacVlan mvlan = entry.getKey();
            NodeConnector port = entry.getValue();
            long mac = mvlan.getMacAddress();
            short vlan = mvlan.getVlan();
            checkActivate(mst, ref, mac, vlan, port, nwMap, hostMap);
        }
    }

    /**
     * Activate the given host and verify results.
     *
     * @param mst      A {@link MacMapState} instance to be tested.
     * @param ref      A reference to the MAC mapping.
     * @param mac      A long value which represents the MAC address.
     * @param vlan     A VLAN ID.
     * @param port     A {@link NodeConnector} instance.
     *                 An arbitrary port is chosen if {@code null} is
     *                 specified.
     * @param nwMap    A {@link MappedNetwork} instance to store mapped
     *                 networks.
     * @param hostMap  A map to store activated hosts.
     */
    private void checkActivate(MacMapState mst, MapReference ref,
                               long mac, short vlan, NodeConnector port,
                               MappedNetwork nwMap,
                               Map<MacVlan, NodeConnector> hostMap) {
        boolean active = !mst.hasMapping();
        MacVlan mvlan = new MacVlan(mac, vlan);
        if (port == null) {
            port = getPort();
        }
        PortVlan pvlan = new PortVlan(port, vlan);
        assertEquals(nwMap.getNetworks().contains(pvlan),
                     mst.hasMapping(pvlan));
        assertNull(mst.getDuplicate(mvlan));

        NodeConnector oldPort = hostMap.get(mvlan);
        PortVlan oldPv;
        if (oldPort == null) {
            oldPv = null;
            assertNull(mst.getPortVlan(mac));
            assertNull(mst.getPort(mvlan));
        } else {
            oldPv = new PortVlan(oldPort, vlan);
            assertEquals(oldPv, mst.getPortVlan(mac));
            assertEquals(oldPort, mst.getPort(mvlan));
        }

        try {
            MacMapActivation result = mst.activate(ref, mvlan, port);
            assertTrue(mst.isDirty());
            assertFalse(mst.isDirty());
            assertEquals(active, result.isActivated());
            assertEquals(oldPort, result.getOldPort());

            // null should be returned if the host is already
            // activated.
            assertNull(mst.activate(ref, mvlan, port));
            assertFalse(mst.isDirty());
        } catch (Exception e) {
            unexpected(e);
        }

        assertTrue(mst.hasMapping(pvlan));
        assertNull(mst.getDuplicate(mvlan));
        assertEquals(pvlan, mst.getPortVlan(mac));
        assertEquals(port, mst.getPort(mvlan));

        if (oldPv != null) {
            nwMap.remove(oldPv);
        }
        nwMap.add(pvlan);
        hostMap.put(mvlan, port);

        assertEquals(nwMap.getNetworks(), mst.getNetworks());
        assertEquals(hostMap, mst.getActiveHosts());
        assertTrue(mst.hasMapping());
    }

    /**
     * Remove all hosts on the specified network from the host map.
     *
     * @param hostMap  A map to store activated hosts.
     * @param pvlan    A {@link PortVlan} instance.
     * @return  A map which contains removed entries is returned.
     */
    private Map<MacVlan, NodeConnector> removeHosts(
        Map<MacVlan, NodeConnector> hostMap, PortVlan pvlan) {

        Map<MacVlan, NodeConnector> removed =
            new HashMap<MacVlan, NodeConnector>();
        NodeConnector targetPort = pvlan.getNodeConnector();
        short targetVlan = pvlan.getVlan();
        for (Iterator<Map.Entry<MacVlan, NodeConnector>>
                 it = hostMap.entrySet().iterator(); it.hasNext();) {
            Map.Entry<MacVlan, NodeConnector> entry = it.next();
            MacVlan mvlan = entry.getKey();
            NodeConnector port = entry.getValue();
            if (mvlan.getVlan() == targetVlan && port.equals(targetPort)) {
                removed.put(mvlan, port);
                it.remove();
            }
        }

        return removed;
    }

    /**
     * Remove all hosts on switch ports selected by the specified filter.
     *
     * @param hostMap  A map to store activated hosts.
     * @param filter   A {@link PortFilter} instance which determines switch
     *                 ports.
     * @return  A map which contains removed entries is returned.
     */
    private Map<MacVlan, NodeConnector> removeHosts(
        Map<MacVlan, NodeConnector> hostMap, PortFilter filter) {

        Map<MacVlan, NodeConnector> removed =
            new HashMap<MacVlan, NodeConnector>();
        for (Iterator<Map.Entry<MacVlan, NodeConnector>>
                 it = hostMap.entrySet().iterator(); it.hasNext();) {
            Map.Entry<MacVlan, NodeConnector> entry = it.next();
            MacVlan mvlan = entry.getKey();
            NodeConnector port = entry.getValue();
            if (filter.accept(port, null)) {
                removed.put(mvlan, port);
                it.remove();
            }
        }

        return removed;
    }

    /**
     * Inactivate all hosts in the specified {@link MacMapState} instance.
     *
     * @param mst  A {@link MacMapState} instance.
     */
    private void inactivateAll(MacMapState mst) {
        Set<PortVlan> nw = mst.getNetworks();
        Map<MacVlan, NodeConnector> hosts = mst.getActiveHosts();
        if (nw == null) {
            assertNull(hosts);
            nw = new HashSet<PortVlan>();
            hosts = new HashMap<MacVlan, NodeConnector>();
        }

        // Inactivate all hosts by using PortFilter instance which accepts
        // all switch ports.
        PortFilter filter = new PortFilter() {
            @Override
            public boolean accept(NodeConnector port, VtnPort vport) {
                return true;
            }
        };

        Set<PortVlan> released = new HashSet<PortVlan>();
        assertEquals(hosts, mst.inactivate(filter, released));
        assertEquals(nw, released);
        assertFalse(mst.hasMapping());
        assertNull(mst.getNetworks());
        assertNull(mst.getActiveHosts());
    }
}
