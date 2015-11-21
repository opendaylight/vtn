/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import org.apache.commons.lang3.tuple.Triple;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.inventory.port.NodePortFilter;
import org.opendaylight.vtn.manager.internal.util.inventory.port.PortFilter;
import org.opendaylight.vtn.manager.internal.util.inventory.port.SpecificPortFilter;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * JUnit test for {@link VTNMacMapStatus}.
 */
public class VTNMacMapStatusTest extends TestBase {
    /**
     * A list of {@link SalPort} instances.
     */
    private final List<SalPort>  portList = new ArrayList<>();

    /**
     * Cursor into {@link #portList} array.
     */
    private int  portListIndex;

    /**
     * A list of {@link SalPort} instances which are never used for test.
     */
    private final List<SalPort> unusedPortList = new ArrayList<>();

    /**
     * A class to keep hosts mapped by MAC mapping.
     */
    private static final class HostMap {
        /**
         * A map to keep hosts mapped by MAC mapping.
         */
        private final Map<MacVlan, SalPort>  hostMap;

        /**
         * Construct an empty instance.
         */
        private HostMap() {
            hostMap = new HashMap<>();
        }

        /**
         * Copy constructor.
         *
         * @param map  A {@link HostMap} instance to be copied.
         */
        private HostMap(HostMap map) {
            hostMap = new HashMap<>(map.hostMap);
        }

        /**
         * Return the switch port associated with the given host.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  A {@link SalPort} instance if found.
         *          {@code null} if not found.
         */
        private SalPort get(MacVlan mvlan) {
            return hostMap.get(mvlan);
        }

        /**
         * Add the specified host.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @param sport  A {@link SalPort} instance.
         */
        private void put(MacVlan mvlan, SalPort sport) {
            hostMap.put(mvlan, sport);
        }

        /**
         * Remove the specified host.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  A {@link SalPort} instance previously associated with
         *          {@code mvlan}.
         */
        private SalPort remove(MacVlan mvlan) {
            return hostMap.remove(mvlan);
        }

        /**
         * Return a set of hosts.
         *
         * @return  A set of host.
         */
        private Set<MacVlan> keySet() {
            return hostMap.keySet();
        }

        /**
         * Return a set of host entries.
         *
         * @return  A set of host entries.
         */
        private Set<Entry<MacVlan, SalPort>> entrySet() {
            return hostMap.entrySet();
        }

        /**
         * Remove all the host entries in this map.
         */
        private void clear() {
            hostMap.clear();
        }

        /**
         * Ensure that the given map keeps the same hosts as this instance.
         *
         * @param map  A map that keeps mapped hosts.
         */
        private void verify(Map<MacVlan, SalPort> map) {
            assertEquals(hostMap, map);
        }

        /**
         * Ensure that the given {@link MacMapStatus} instance keeps the same
         * hosts as this instance.
         *
         * @param mst  A {@link MacMapStatus} instance.
         */
        private void verify(MacMapStatus mst) {
            List<MappedHost> hosts = mst.getMappedHost();
            if (hostMap.isEmpty()) {
                assertEquals(null, hosts);
                return;
            }

            assertNotNull(hosts);
            assertEquals(hostMap.size(), hosts.size());

            for (MappedHost mhost: hosts) {
                EtherAddress eaddr = new EtherAddress(mhost.getMacAddress());
                int vid = mhost.getVlanId().getValue().intValue();
                MacVlan mv = new MacVlan(eaddr.getAddress(), vid);
                SalPort sport = SalPort.create(mhost.getPortId());
                assertEquals(hostMap.get(mv), sport);
            }
        }

        // Object

        /**
         * Determine whether the given object is identical to this object.
         *
         * @param o  An object to be compared.
         * @return   {@code true} if identical. Otherwise {@code false}.
         */
        @Override
        public boolean equals(Object o) {
            boolean ret = (o == this);
            if (!ret && o != null && getClass().equals(o.getClass())) {
                HostMap hmap = (HostMap)o;
                ret = hostMap.equals(hmap.hostMap);
            }

            return ret;
        }

        /**
         * Return the hash code of this object.
         *
         * @return  The hash code.
         */
        @Override
        public int hashCode() {
            return Objects.hash(getClass(), hostMap);
        }
    }

    /**
     * A class to keep networks mapped by MAC mapping.
     */
    private static final class NetworkMap {
        /**
         * Pairs of {@link PortVlan} instance and its reference counter.
         */
        private final Map<PortVlan, Integer>  networkMap = new HashMap<>();

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
         * @throws Exception  An error occurred.
         */
        private Set<PortVlan> removeForce(PortFilter filter) throws Exception {
            Set<PortVlan> removed = new HashSet<PortVlan>();
            for (Iterator<PortVlan> it = networkMap.keySet().iterator();
                 it.hasNext();) {
                PortVlan pvlan = it.next();
                SalPort sport = pvlan.getPort();
                if (filter.accept(sport, null)) {
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
    public VTNMacMapStatusTest() {
        // Create node connectors for test.
        for (long dpid = 0; dpid < 2L; dpid++) {
            for (long id = 1L; id <= 5L; id++) {
                portList.add(new SalPort(dpid, id));
            }

            for (long id = 100L; id <= 102L; id++) {
                unusedPortList.add(new SalPort(dpid, id));
            }
        }
    }

    /**
     * Test case for {@link VTNMacMapStatus#VTNMacMapStatus()} and
     * {@link VTNMacMapStatus#VTNMacMapStatus(MacMapStatus)}.
     */
    @Test
    public void testConstructor() {
        // Test case for an empty instance.
        VTNMacMapStatus vmst = new VTNMacMapStatus();
        MacMapStatus empty = new MacMapStatusBuilder().build();
        assertEquals(empty, vmst.toMacMapStatus());
        assertFalse(vmst.isDirty());

        vmst = new VTNMacMapStatus(null);
        assertEquals(empty, vmst.toMacMapStatus());
        assertFalse(vmst.isDirty());

        vmst = new VTNMacMapStatus(empty);
        assertEquals(empty, vmst.toMacMapStatus());
        assertFalse(vmst.isDirty());

        HostMap hostMap = new HostMap();
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, 0),
            new MacVlan(0xfeffabcdef00L, 0),
            new MacVlan(0xfeffabcdef01L, 1),
            new MacVlan(0xfeffabcdef02L, 100),
            new MacVlan(0xfeffabcdefffL, 123),
            new MacVlan(0x000000000001L, 4094),
            new MacVlan(0x000000000002L, 4094),
            new MacVlan(0xfc927ace41d7L, 4095),
        };

        List<MappedHost> mhosts = new ArrayList<>();
        for (MacVlan mv: hosts) {
            SalPort sport = getPort();
            hostMap.put(mv, sport);

            MappedHost mhost = new MappedHostBuilder().
                setMacAddress(mv.getMacAddress()).
                setPortId(sport.getNodeConnectorId()).
                setVlanId(new VlanId(mv.getVlanId())).
                build();
            mhosts.add(mhost);
            MacMapStatus mst = new MacMapStatusBuilder().
                setMappedHost(mhosts).build();
            vmst = new VTNMacMapStatus(mst);
            hostMap.verify(vmst.toMacMapStatus());
        }
    }

    /**
     * Test case for
     * {@link VTNMacMapStatus#submit(ReadWriteTransaction, MacMapIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSubmit() throws Exception {
        // Empty instance.
        VTNMacMapStatus vmst = new VTNMacMapStatus();
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        VnodeName vtnName = new VnodeName("tenant");
        VnodeName vbrName = new VnodeName("bridge");
        MacMapIdentifier mapId = new MacMapIdentifier(vtnName, vbrName);
        vmst.submit(tx, mapId);
        verifyZeroInteractions(tx);

        // Activate a mapping.
        MacVlan mv = new MacVlan(0x001122334455L, 1234);
        SalPort sport = new SalPort(1234L, 5678L);
        MacAddress mac = mv.getMacAddress();
        MappedHost mhost = new MappedHostBuilder().
            setMacAddress(mac).
            setPortId(sport.getNodeConnectorId()).
            setVlanId(new VlanId(mv.getVlanId())).
            build();
        MacMapStatus mst = new MacMapStatusBuilder().
            setMappedHost(Collections.singletonList(mhost)).
            build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<MacMapStatus> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(vtnName)).
            child(Vbridge.class, new VbridgeKey(vbrName)).
            child(MacMap.class).
            child(MacMapStatus.class).
            build();

        vmst.activate(mapId, mv, sport);
        vmst.submit(tx, mapId);
        verify(tx).put(oper, path, mst, false);
        verifyNoMoreInteractions(tx);

        // Submit again.
        tx = mock(ReadWriteTransaction.class);
        vmst.submit(tx, mapId);
        verifyZeroInteractions(tx);

        // Try to activate the same host.
        vmst.activate(mapId, mv, sport);
        vmst.submit(tx, mapId);
        verifyZeroInteractions(tx);
    }

    /**
     * Ensure that {@link VTNMacMapStatus} maintains mapped host information
     * correctly.
     *
     * <ul>
     *   <li>{@link VTNMacMapStatus#activate(MacMapIdentifier, MacVlan, SalPort)}</li>
     *   <li>{@link VTNMacMapStatus#getPort(MacVlan)}</li>
     *   <li>{@link VTNMacMapStatus#getPortVlan(long)}</li>
     *   <li>{@link VTNMacMapStatus#hasMapping(PortVlan)}</li>
     *   <li>{@link VTNMacMapStatus#hasMapping()}</li>
     *   <li>{@link VTNMacMapStatus#getNetworks()}</li>
     *   <li>{@link VTNMacMapStatus#getDuplicate(MacVlan)}</li>
     *   <li>{@link VTNMacMapStatus#isDirty()}</li>
     *   <li>{@link VTNMacMapStatus#inactivate(MacVlan, Set)}</li>
     *   <li>{@link VTNMacMapStatus#inactivate(PortVlan)}</li>
     *   <li>{@link VTNMacMapStatus#inactivate(PortFilter, Set)}</li>
     *   <li>{@link VTNMacMapStatus#inactivate(MacMapIdentifier, Set, Set, Set)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testStatus() throws Exception {
        // Test case for an empty instance.
        VTNMacMapStatus vmst = new VTNMacMapStatus();
        assertFalse(vmst.hasMapping());
        assertNull(vmst.getNetworks());
        assertFalse(vmst.isDirty());

        NetworkMap nwMap = new NetworkMap();
        HostMap hostMap = new HostMap();

        // Activate MAC mappings with specifying MAC addresses in which the
        // MSB is not set.
        VnodeName vtnName = new VnodeName("tenant");
        VnodeName vbrName = new VnodeName("bridge");
        MacMapIdentifier mapId = new MacMapIdentifier(vtnName, vbrName);
        int[] vlans = {0, 1, 4094, 4095};
        long mac = 0x1000L;
        for (int i = 0; i < 10; i++) {
            for (int vid: vlans) {
                checkActivate(vmst, mapId, mac, vid, null, nwMap, hostMap);
                mac++;
            }
        }

        // Activate MAC mappings with specifying MAC addresses in which the
        // MSB is set.
        mac = 0xfeffffffff00L;
        for (int i = 0; i < 10; i++) {
            for (int vid: vlans) {
                checkActivate(vmst, mapId, mac, vid, null, nwMap, hostMap);
                mac++;
            }
        }

        // Ensure that duplicate MAC addresses are rejected.
        for (Entry<MacVlan, SalPort> entry: hostMap.entrySet()) {
            MacVlan mapped = entry.getKey();
            SalPort sport = entry.getValue();
            mac = mapped.getAddress();
            MacVlan mvlan = new MacVlan(mac, 2000);

            assertEquals(mapped, vmst.getDuplicate(mvlan));
            try {
                vmst.activate(mapId, mvlan, sport);
                fail("An exception must be thrown.");
            } catch (MacMapDuplicateException e) {
                assertEquals(mvlan, e.getHost());
                assertEquals(mapId, e.getIdentifier());
                assertEquals(mapped, e.getDuplicate());
            }

            assertFalse(vmst.isDirty());
            hostMap.verify(vmst.toMacMapStatus());
        }

        Set<PortVlan> nw = vmst.getNetworks();
        assertEquals(nwMap.getNetworks(), nw);

        // Move all hosts to one port.
        SalPort newPort = unusedPortList.get(0);
        HostMap hmap = new HostMap(hostMap);
        for (Entry<MacVlan, SalPort> entry: hmap.entrySet()) {
            MacVlan mvlan = entry.getKey();
            mac = mvlan.getAddress();
            int vid = mvlan.getVlanId();
            SalPort sport = entry.getValue();
            PortVlan pvlan = new PortVlan(sport, vid);
            assertNull(vmst.getDuplicate(mvlan));
            assertEquals(pvlan, vmst.getPortVlan(mac));
            assertEquals(sport, vmst.getPort(mvlan));
            assertFalse(vmst.isDirty());

            Triple<Boolean, SalPort, PortVlan> result =
                vmst.activate(mapId, mvlan, newPort);
            assertTrue(vmst.isDirty());
            assertFalse(vmst.isDirty());
            assertFalse(result.getLeft().booleanValue());
            assertEquals(sport, result.getMiddle());
            PortVlan released = (nwMap.remove(pvlan)) ? pvlan : null;
            assertEquals(released, result.getRight());
            hostMap.put(mvlan, newPort);
            hostMap.verify(vmst.toMacMapStatus());

            assertNull(vmst.activate(mapId, mvlan, newPort));
            assertFalse(vmst.isDirty());
            hostMap.verify(vmst.toMacMapStatus());

            pvlan = new PortVlan(newPort, mvlan.getVlanId());
            assertNull(vmst.getDuplicate(mvlan));
            assertEquals(pvlan, vmst.getPortVlan(mac));
            assertEquals(newPort, vmst.getPort(mvlan));

            nwMap.add(pvlan);
            assertEquals(nwMap.getNetworks(), vmst.getNetworks());
            assertTrue(vmst.hasMapping());
        }

        // Restore mappings.
        checkActivate(vmst, mapId, hmap, nwMap, hostMap);
        assertEquals(hmap, hostMap);

        // Inactivate all hosts by specifying MacVlan instance.
        for (Entry<MacVlan, SalPort> entry: hmap.entrySet()) {
            assertTrue(vmst.hasMapping());
            MacVlan mvlan = entry.getKey();
            mac = mvlan.getAddress();
            SalPort sport = entry.getValue();
            PortVlan pvlan = new PortVlan(sport, mvlan.getVlanId());
            assertTrue(vmst.hasMapping(pvlan));

            Set<PortVlan> released = new HashSet<PortVlan>();
            assertEquals(sport, vmst.inactivate(mvlan, released));
            assertTrue(vmst.isDirty());
            assertFalse(vmst.isDirty());
            assertEquals(sport, hostMap.remove(mvlan));
            hostMap.verify(vmst.toMacMapStatus());

            boolean removed = nwMap.remove(pvlan);
            if (removed) {
                assertEquals(1, released.size());
                assertTrue(released.contains(pvlan));
            } else {
                assertTrue(released.isEmpty());
            }
            assertEquals(!removed, vmst.hasMapping(pvlan));

            assertNull(vmst.getDuplicate(mvlan));
            assertNull(vmst.getPortVlan(mac));
            assertNull(vmst.getPort(mvlan));

            // Try to inactivate the same host.
            released.clear();
            assertNull(vmst.inactivate(mvlan, released));
            assertFalse(vmst.isDirty());
            assertTrue(released.isEmpty());
            hostMap.verify(vmst.toMacMapStatus());
        }
        assertFalse(vmst.hasMapping());

        // Restore mappings again.
        checkActivate(vmst, mapId, hmap, nwMap, hostMap);
        assertEquals(hmap, hostMap);

        // Inactivate all hosts by specifying PortVlan instance.
        Set<PortVlan> nwSet = new HashSet<>(nwMap.getNetworks());
        for (PortVlan pvlan: nwSet) {
            assertTrue(vmst.hasMapping());
            assertTrue(vmst.hasMapping(pvlan));
            Set<MacVlan> removed = vmst.inactivate(pvlan);
            assertTrue(vmst.isDirty());
            assertFalse(vmst.isDirty());
            assertEquals(removeHosts(hostMap, pvlan).keySet(), removed);
            nwMap.removeForce(pvlan);
            hostMap.verify(vmst.toMacMapStatus());

            assertFalse(vmst.hasMapping(pvlan));
            Set<PortVlan> nw1 = nwMap.getNetworks();
            if (nw1.isEmpty()) {
                nw1 = null;
            }
            assertEquals(nw1, vmst.getNetworks());

            for (MacVlan mvlan: removed) {
                assertNull(vmst.getDuplicate(mvlan));
                assertNull(vmst.getPortVlan(mvlan.getAddress()));
                assertNull(vmst.getPort(mvlan));
            }

            // Try to inactivate the same network.
            assertNull(vmst.inactivate(pvlan));
            assertFalse(vmst.isDirty());
            hostMap.verify(vmst.toMacMapStatus());
        }
        assertFalse(vmst.hasMapping());

        // Restore mappings again.
        checkActivate(vmst, mapId, hmap, nwMap, hostMap);
        assertEquals(hmap, hostMap);

        // Inactivate all hosts on the specified switch.
        SalNode removedNode = portList.get(0).getSalNode();
        PortFilter filter = new NodePortFilter(removedNode);
        Set<PortVlan> released = new HashSet<>();
        Map<MacVlan, SalPort> removedMap = vmst.inactivate(filter, released);
        assertTrue(vmst.isDirty());
        assertFalse(vmst.isDirty());
        assertFalse(removedMap.isEmpty());
        assertFalse(released.isEmpty());
        assertEquals(removedMap, removeHosts(hostMap, filter));
        assertEquals(released, nwMap.removeForce(filter));
        assertEquals(nwMap.getNetworks(), vmst.getNetworks());
        hostMap.verify(vmst.toMacMapStatus());

        for (Entry<MacVlan, SalPort> entry: removedMap.entrySet()) {
            MacVlan mvlan = entry.getKey();
            SalPort sport = entry.getValue();
            assertEquals(removedNode, sport.getSalNode());
            assertNull(vmst.getDuplicate(mvlan));
            assertNull(vmst.getPortVlan(mvlan.getAddress()));
            assertNull(vmst.getPort(mvlan));
            PortVlan pvlan = new PortVlan(sport, mvlan.getVlanId());
            assertFalse(vmst.hasMapping(pvlan));
        }
        assertTrue(vmst.hasMapping());

        // Try to inactivate again.
        released.clear();
        assertTrue(vmst.inactivate(filter, released).isEmpty());
        assertFalse(vmst.isDirty());
        assertTrue(released.isEmpty());
        hostMap.verify(vmst.toMacMapStatus());

        // Inactivate all hosts by specifying switch port.
        for (SalPort sport: portList) {
            SalNode snode = sport.getSalNode();
            boolean inactivated = snode.equals(removedNode);
            filter = new SpecificPortFilter(sport);
            released.clear();
            removedMap = vmst.inactivate(filter, released);
            assertEquals(!inactivated, vmst.isDirty());
            assertFalse(vmst.isDirty());
            assertEquals(inactivated, removedMap.isEmpty());
            assertEquals(inactivated, released.isEmpty());
            assertEquals(removedMap, removeHosts(hostMap, filter));
            assertEquals(released, nwMap.removeForce(filter));
            hostMap.verify(vmst.toMacMapStatus());

            Set<PortVlan> nw1 = nwMap.getNetworks();
            if (nw1.isEmpty()) {
                nw1 = null;
            }
            assertEquals(nw1, vmst.getNetworks());

            for (Entry<MacVlan, SalPort> entry: removedMap.entrySet()) {
                MacVlan mvlan = entry.getKey();
                assertEquals(sport, entry.getValue());
                assertFalse(removedNode.equals(sport.getSalNode()));
                assertNull(vmst.getDuplicate(mvlan));
                assertNull(vmst.getPortVlan(mvlan.getAddress()));
                assertNull(vmst.getPort(mvlan));
                PortVlan pvlan = new PortVlan(sport, mvlan.getVlanId());
                assertFalse(vmst.hasMapping(pvlan));
            }

            // Try to inactivate again.
            released.clear();
            assertTrue(vmst.inactivate(filter, released).isEmpty());
            assertFalse(vmst.isDirty());
            assertTrue(released.isEmpty());
            hostMap.verify(vmst.toMacMapStatus());
        }
        assertFalse(vmst.hasMapping());

        // Restore mappings again.
        checkActivate(vmst, mapId, hmap, nwMap, hostMap);
        assertEquals(hmap, hostMap);

        // Test case for inactivate(MacMapIdentifier, Set, Set, Set).
        // In order to simplify the test code, hosts to be inactivated are
        // determined by PortVlan instances.
        // At first, choose 2 arbitrary VLAN networks.
        Set<PortVlan> expectedNw = new HashSet<>();
        Map<MacVlan, SalPort> expectedHosts = new HashMap<>();
        Set<Integer> unmappedVlans = new HashSet<>();
        for (int i = 1; i <= 2; i++) {
            int idx = (i * i) + 2;
            int vid = vlans[i];
            SalPort sport = portList.get(idx);
            PortVlan pvlan = new PortVlan(sport, vid);
            expectedNw.add(pvlan);
            nwMap.removeForce(pvlan);
            Map<MacVlan, SalPort> m = removeHosts(hostMap, pvlan);
            assertFalse(m.isEmpty());
            expectedHosts.putAll(m);

            // This test case assumes that this host is mapped by MAC mapping
            // with wildcard MAC address. So this VLAN ID needs to be added
            // to unmappedVlans.
            unmappedVlans.add(vid);
        }
        assertFalse(expectedNw.isEmpty());
        assertFalse(expectedHosts.isEmpty());
        assertFalse(unmappedVlans.isEmpty());

        // Construct an allowed host set.
        Set<MacVlan> allowed = new HashSet<>();
        for (MacVlan mvlan: hmap.keySet()) {
            int vid = mvlan.getVlanId();
            if (unmappedVlans.contains(vid) &&
                !expectedHosts.containsKey(mvlan)) {
                // This host should be retained.
                allowed.add(mvlan);
            }
        }
        assertFalse(allowed.isEmpty());

        // Inactivate all hosts on the specified VLAN except for hosts
        // which are mapped explicitly.
        released.clear();
        removedMap = vmst.inactivate(mapId, allowed, unmappedVlans, released);
        assertTrue(vmst.isDirty());
        assertFalse(vmst.isDirty());
        assertFalse(removedMap.isEmpty());
        assertFalse(released.isEmpty());
        assertEquals(removedMap, expectedHosts);
        assertEquals(released, expectedNw);
        assertEquals(nwMap.getNetworks(), vmst.getNetworks());
        hostMap.verify(vmst.toMacMapStatus());

        for (Entry<MacVlan, SalPort> entry: removedMap.entrySet()) {
            MacVlan mvlan = entry.getKey();
            SalPort sport = entry.getValue();
            assertNull(vmst.getDuplicate(mvlan));
            assertNull(vmst.getPortVlan(mvlan.getAddress()));
            assertNull(vmst.getPort(mvlan));
            PortVlan pvlan = new PortVlan(sport, mvlan.getVlanId());
            assertFalse(vmst.hasMapping(pvlan));
        }
        assertTrue(vmst.hasMapping());

        // Try to inactivate again.
        released.clear();
        assertTrue(vmst.inactivate(mapId, allowed, unmappedVlans, released).
                   isEmpty());
        assertFalse(vmst.isDirty());
        assertTrue(released.isEmpty());
        assertTrue(vmst.hasMapping());
        hostMap.verify(vmst.toMacMapStatus());

        // No host should be inactivated if unmappedVlans is empty.
        allowed.clear();
        unmappedVlans.clear();
        assertTrue(vmst.inactivate(mapId, allowed, unmappedVlans, released).
                   isEmpty());
        assertFalse(vmst.isDirty());
        assertTrue(released.isEmpty());
        assertTrue(vmst.hasMapping());
        hostMap.verify(vmst.toMacMapStatus());

        // No host should be inactivated if all hosts are explicitly mapped.
        for (int vid: vlans) {
            unmappedVlans.add(vid);
        }
        allowed.clear();
        allowed.addAll(hostMap.keySet());
        assertTrue(vmst.inactivate(mapId, allowed, unmappedVlans, released).
                   isEmpty());
        assertFalse(vmst.isDirty());
        assertTrue(released.isEmpty());
        assertTrue(vmst.hasMapping());
        hostMap.verify(vmst.toMacMapStatus());

        // All hosts should be inactivated if no host is explicitly mapped
        // and all VLANs are unmapped.
        allowed.clear();
        hostMap.verify(vmst.inactivate(mapId, allowed, unmappedVlans,
                                       released));
        assertTrue(vmst.isDirty());
        assertFalse(vmst.isDirty());
        assertEquals(nwMap.getNetworks(), released);
        assertFalse(vmst.hasMapping());
        assertNull(vmst.getNetworks());

        for (Entry<MacVlan, SalPort> entry: hostMap.entrySet()) {
            MacVlan mvlan = entry.getKey();
            SalPort sport = entry.getValue();
            assertNull(vmst.getDuplicate(mvlan));
            assertNull(vmst.getPortVlan(mvlan.getAddress()));
            assertNull(vmst.getPort(mvlan));
            PortVlan pvlan = new PortVlan(sport, mvlan.getVlanId());
            assertFalse(vmst.hasMapping(pvlan));
        }

        hostMap.clear();
        hostMap.verify(vmst.toMacMapStatus());

        // Try to inactivate again.
        released.clear();
        assertTrue(vmst.inactivate(mapId, allowed, unmappedVlans, released).
                   isEmpty());
        assertFalse(vmst.isDirty());
        assertTrue(released.isEmpty());
        assertFalse(vmst.hasMapping());
        hostMap.verify(vmst.toMacMapStatus());
    }

    /**
     * Return a {@link SalPort} for test.
     *
     * @return  A {@link SalPort} instance.
     */
    private SalPort getPort() {
        int idx = portListIndex;
        SalPort sport = portList.get(idx);
        idx++;
        portListIndex = (idx >= portList.size()) ? 0 : idx;

        return sport;
    }

    /**
     * Activate hosts specified by the map.
     *
     * @param vmst     A {@link VTNMacMapStatus} instance to be tested.
     * @param mapId    A {@link MacMapIdentifier} instance.
     * @param hosts    A map which contains hosts to be activated.
     * @param nwMap    A {@link NetworkMap} instance to store mapped
     *                 networks.
     * @param hostMap  A {@link HostMap} instance to store mapped hosts.
     * @throws Exception  An error occurred.
     */
    private void checkActivate(VTNMacMapStatus vmst, MacMapIdentifier mapId,
                               HostMap hosts, NetworkMap nwMap,
                               HostMap hostMap) throws Exception {
        for (Entry<MacVlan, SalPort> entry: hosts.entrySet()) {
            MacVlan mvlan = entry.getKey();
            SalPort sport = entry.getValue();
            long mac = mvlan.getAddress();
            int vid = mvlan.getVlanId();
            checkActivate(vmst, mapId, mac, vid, sport, nwMap, hostMap);
        }
    }

    /**
     * Activate the given host and verify results.
     *
     * @param vmst     A {@link VTNMacMapStatus} instance to be tested.
     * @param mapId    A {@link MacMapIdentifier} instance.
     * @param mac      A long value which represents the MAC address.
     * @param vid      A VLAN ID.
     * @param sport    A {@link SalPort} instance.
     *                 An arbitrary port is chosen if {@code null} is
     *                 specified.
     * @param nwMap    A {@link NetworkMap} instance to store mapped
     *                 networks.
     * @param hostMap  A map to store activated hosts.
     * @throws Exception  An error occurred.
     */
    private void checkActivate(VTNMacMapStatus vmst, MacMapIdentifier mapId,
                               long mac, int vid, SalPort sport,
                               NetworkMap nwMap, HostMap hostMap)
        throws Exception {
        boolean active = !vmst.hasMapping();
        MacVlan mvlan = new MacVlan(mac, vid);
        SalPort port = sport;
        if (port == null) {
            port = getPort();
        }
        PortVlan pvlan = new PortVlan(port, vid);
        assertEquals(nwMap.getNetworks().contains(pvlan),
                     vmst.hasMapping(pvlan));
        assertNull(vmst.getDuplicate(mvlan));

        SalPort oldPort = hostMap.get(mvlan);
        PortVlan oldPv;
        if (oldPort == null) {
            oldPv = null;
            assertNull(vmst.getPortVlan(mac));
            assertNull(vmst.getPort(mvlan));
        } else {
            oldPv = new PortVlan(oldPort, vid);
            assertEquals(oldPv, vmst.getPortVlan(mac));
            assertEquals(oldPort, vmst.getPort(mvlan));
        }

        Triple<Boolean, SalPort, PortVlan> result =
            vmst.activate(mapId, mvlan, port);
        assertTrue(vmst.isDirty());
        assertFalse(vmst.isDirty());
        assertEquals(active, result.getLeft().booleanValue());
        assertEquals(oldPort, result.getMiddle());
        PortVlan released;
        if (oldPv == null) {
            released = null;
        } else {
            released = (nwMap.remove(oldPv)) ? oldPv : null;
        }
        assertEquals(released, result.getRight());
        hostMap.put(mvlan, port);
        hostMap.verify(vmst.toMacMapStatus());

        // null should be returned if the host is already
        // activated.
        assertNull(vmst.activate(mapId, mvlan, port));
        assertFalse(vmst.isDirty());
        hostMap.verify(vmst.toMacMapStatus());

        assertTrue(vmst.hasMapping(pvlan));
        assertNull(vmst.getDuplicate(mvlan));
        assertEquals(pvlan, vmst.getPortVlan(mac));
        assertEquals(port, vmst.getPort(mvlan));

        nwMap.add(pvlan);

        assertEquals(nwMap.getNetworks(), vmst.getNetworks());
        assertTrue(vmst.hasMapping());
    }

    /**
     * Remove all hosts on the specified network from the host map.
     *
     * @param hostMap  A map to store activated hosts.
     * @param pvlan    A {@link PortVlan} instance.
     * @return  A map which contains removed entries is returned.
     */
    private Map<MacVlan, SalPort> removeHosts(HostMap hostMap, PortVlan pvlan) {
        Map<MacVlan, SalPort> removed = new HashMap<>();
        SalPort targetPort = pvlan.getPort();
        int targetVlan = pvlan.getVlanId();
        for (Iterator<Entry<MacVlan, SalPort>> it =
                 hostMap.entrySet().iterator(); it.hasNext();) {
            Entry<MacVlan, SalPort> entry = it.next();
            MacVlan mvlan = entry.getKey();
            SalPort sport = entry.getValue();
            if (mvlan.getVlanId() == targetVlan && sport.equals(targetPort)) {
                removed.put(mvlan, sport);
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
     * @throws Exception  An error occurred.
     */
    private Map<MacVlan, SalPort> removeHosts(
        HostMap hostMap, PortFilter filter) throws Exception {
        Map<MacVlan, SalPort> removed = new HashMap<>();
        for (Iterator<Entry<MacVlan, SalPort>> it =
                 hostMap.entrySet().iterator(); it.hasNext();) {
            Entry<MacVlan, SalPort> entry = it.next();
            MacVlan mvlan = entry.getKey();
            SalPort sport = entry.getValue();
            if (filter.accept(sport, null)) {
                removed.put(mvlan, sport);
                it.remove();
            }
        }

        return removed;
    }
}
