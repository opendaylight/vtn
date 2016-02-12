/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNMacMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNMacMapStatus;
import org.opendaylight.vtn.manager.it.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * Test case for {@link VtnMacMapService} that depends on inventory
 * information.
 */
public final class MacMapStatusTest extends TestMethodBase {
    /**
     * A list of MD-SAL node identifiers.
     */
    private final List<String>  nodeIds = new ArrayList<>();

    /**
     * MD-SAL node and port identifiers used for test.
     */
    private final Map<String, List<String>>  nodeMap = new HashMap<>();

    /**
     * Pairs of MD-SAL port identifiers and port names.
     */
    private final Map<String, String>  portNames = new HashMap<>();

    /**
     * A list of MAC addresses for test.
     */
    private final List<EtherAddress>  macAddresses;

    /**
     * A list of MAC addresses that are never mapped.
     */
    private final List<MacAddress>  unmappedMacAddresses;

    /**
     * A virtual node state waiter.
     */
    private final VNodeStateWaiter  vnodeWaiter;

    /**
     * A list of vBridge identifiers.
     */
    private final List<VBridgeIdentifier>  vbridgeIds = new ArrayList<>();

    /**
     * A list of MAC address table entry waiters.
     */
    private final List<MacEntryWaiter>  macWaiters = new ArrayList<>();

    /**
     * {@code MapArg} describes a set of arguments for MAC mapping test.
     */
    private static final class MapArg {
        /**
         * The vBridge index that indicates the invalid vBridge.
         */
        private static final int  INVALID = -1;

        /**
         * The index number associated with the vBridge to which the L2 host
         * should be mapped.
         */
        private final int  index;

        /**
         * The MD-SAL port identifier.
         */
        private String  portId;

        /**
         * IP address assigned to the L2 host.
         */
        private IpNetwork  ipAddress;

        /**
         * Construct a new instance.
         *
         * @param idx  The index associated with the vBridge to which the
         *             L2 host should be mapped.
         * @param pid  The MD-SAL port identifier to be associated with the
         *             L2 host.
         * @param ipn  The IP address assigned to the L2 host.
         */
        private MapArg(int idx, String pid, IpNetwork ipn) {
            index = idx;
            portId = pid;
            ipAddress = ipn;
        }

        /**
         * Return the index number that specifies the vBridge to which the L2
         * host should be mapped.
         *
         * @return  The index number for the vBridge.
         *          A negative value indicates the L2 host should not be
         *          mapped.
         */
        private int getIndex() {
            return index;
        }

        /**
         * Return the MD-SAL port identifier.
         *
         * @return  The MD-SAL port identifier.
         */
        private String getPortId() {
            return portId;
        }

        /**
         * Set the MD-SAL port identifier.
         *
         * @param pid  The MD-SAL port identifier.
         */
        private void setPortId(String pid) {
            portId = pid;
        }

        /**
         * Return the IP address assigned to the L2 host.
         *
         * @return  An {@link IpNetwork} instance.
         */
        private IpNetwork getIpAddress() {
            return ipAddress;
        }

        /**
         * Set the IP address assigned to the L2 host.
         *
         * @param ipn  An {@link IpNetwork} instance.
         */
        private void setIpAddress(IpNetwork ipn) {
            ipAddress = ipn;
        }
    }

    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public MacMapStatusTest(VTNManagerIT vit) {
        super(vit);

        // Create 8 MAC addresses for test.
        Random rand = new Random(0x987654321abcdeL);
        Set<EtherAddress> eaddrSet = new HashSet<>();
        do {
            eaddrSet.add(createEtherAddress(rand));
        } while (eaddrSet.size() < 8);
        macAddresses = new ArrayList<>(eaddrSet);

        // Create 4 MAC addresses that are never mapped.
        unmappedMacAddresses = new ArrayList<>();
        do {
            EtherAddress eaddr = createEtherAddress(rand);
            if (eaddrSet.add(eaddr)) {
                unmappedMacAddresses.add(eaddr.getMacAddress());
            }
        } while (unmappedMacAddresses.size() < 4);

        OfMockService ofmock = vit.getOfMockService();
        vnodeWaiter = new VNodeStateWaiter(ofmock);

        // Create vBridge identifiers and MAC address table entry waiters.
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        String bname1 = "vbr_1";
        String bname2 = "vbr_2";
        Collections.addAll(
            vbridgeIds,
            new VBridgeIdentifier(tname1, bname1),
            new VBridgeIdentifier(tname1, bname2),
            new VBridgeIdentifier(tname2, bname1));
        for (VBridgeIdentifier vbrId: vbridgeIds) {
            macWaiters.add(new MacEntryWaiter(ofmock, vbrId));
        }
    }

    /**
     * Create a new node for test.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param dpid    The datapath ID for the new node.
     * @return  The MD-SAL node identifier assigned to the new node.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private String addNode(OfMockService ofmock, BigInteger dpid)
        throws InterruptedException {
        String nid = ofmock.addNode(dpid);
        List<String> ports = new ArrayList<>();
        nodeMap.put(nid, ports);

        // Create 2 ports.
        for (long id = 1; id <= 2; id++) {
            ports.add(addPort(ofmock, nid, id));
        }

        return nid;
    }

    /**
     * Remove the specified node.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param nid     The MD-SAL node identifier to be removed.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private void removeNode(OfMockService ofmock, String nid)
        throws InterruptedException {
        ofmock.removeNode(nid);

        List<String> ports = nodeMap.get(nid);
        for (String pid: ports) {
            portNames.remove(pid);
        }
    }

    /**
     * Create a new switch port on the specified node.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param nid     The MD-SAL node identifier.
     * @param id      The identifier for a new port.
     * @return  The MD-SAL port identifier for the created port.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private String addPort(OfMockService ofmock, String nid, long id)
        throws InterruptedException {
        String pid = ofmock.addPort(nid, id, false);
        portNames.put(pid, ofmock.getPortName(pid));
        ofmock.setPortState(pid, true);
        return pid;
    }

    /**
     * Remove the specified switch port.
     *
     * <p>
     *   Note that this method never waits for completion of port deletion.
     * </p>
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param pid     The MD-SAL port identifier.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private void removePort(OfMockService ofmock, String pid)
        throws InterruptedException {
        ofmock.removePort(pid, false);
        portNames.remove(pid);
    }

    /**
     * Return the MD-SAL port identifier for the specified port.
     *
     * @param nidx  The index of the node.
     * @param pidx  The index of the port.
     * @return  The MD-SAL port identifier.
     */
    private String getPortId(int nidx, int pidx) {
        return nodeMap.get(nodeIds.get(nidx)).get(pidx);
    }

    /**
     * Learn L2 hosts by sending broadcast ICMP packet.
     *
     * @param mmapSrv   vtn-mac-map service.
     * @param vid       The VLAN ID.
     * @param mappings  A map that specifies active MAC mappings.
     * @throws Exception  An error occurred.
     */
    private void learnByIcmp(VtnMacMapService mmapSrv, int vid,
                             Map<MacVlan, MapArg> mappings) throws Exception {
        VirtualNetwork vnet = getVirtualNetwork();
        OfMockService ofmock = getTest().getOfMockService();
        Set<String> portSet = portNames.keySet();
        IpNetwork dstIp = new Ip4Network("192.168.99.254");

        for (int i = 0; i < macAddresses.size(); i++) {
            EtherAddress eaddr = macAddresses.get(i);
            MacVlan mv = new MacVlan(eaddr, vid);
            MapArg arg = mappings.get(mv);
            int idx = arg.getIndex();
            IpNetwork ipn = arg.getIpAddress();
            String pid = arg.getPortId();
            String pname = portNames.get(pid);
            TestHost th = new TestHost(eaddr, vid, pid, pname, ipn);

            VTNMacMapStatus vmst;
            if (idx < 0) {
                // Should be dropped.
                vmst = new VTNMacMapStatus();
            } else {
                VBridgeIdentifier vbrId = vbridgeIds.get(idx);
                MacEntryWaiter mwaiter = macWaiters.get(idx);
                vmst = vnet.getBridge(vbrId).getMacMap().getStatus();
                MacEntry ment = vmst.get(eaddr);
                if (ment == null) {
                    // In this case IP address should never be set in a
                    // MAC address table entry.
                    ment = new MacEntry(eaddr, vid, pid, pname, null);
                }
                vmst.add(ment);
                mwaiter.add(ment);
            }

            sendBroadcastIcmp(th, dstIp, vmst.getMappedVlans(portSet));
        }

        verify(mmapSrv);
    }

    /**
     * Learn L2 hosts by sending broadcast ARP packet.
     *
     * @param mmapSrv   vtn-mac-map service.
     * @param vid       The VLAN ID.
     * @param mappings  A map that specifies active MAC mappings.
     * @throws Exception  An error occurred.
     */
    private void learn(VtnMacMapService mmapSrv, int vid,
                       Map<MacVlan, MapArg> mappings) throws Exception {
        VirtualNetwork vnet = getVirtualNetwork();
        OfMockService ofmock = getTest().getOfMockService();
        Set<String> portSet = portNames.keySet();

        for (int i = 0; i < macAddresses.size(); i++) {
            EtherAddress eaddr = macAddresses.get(i);
            MacVlan mv = new MacVlan(eaddr, vid);
            MapArg arg = mappings.get(mv);
            int idx = arg.getIndex();
            IpNetwork ipn = arg.getIpAddress();
            String pid = arg.getPortId();
            String pname = portNames.get(pid);
            TestHost th = new TestHost(eaddr, vid, pid, pname, ipn);

            VTNMacMapStatus vmst;
            if (idx < 0) {
                // Should be dropped.
                vmst = new VTNMacMapStatus();
            } else {
                VBridgeIdentifier vbrId = vbridgeIds.get(idx);
                MacEntryWaiter mwaiter = macWaiters.get(idx);
                vmst = vnet.getBridge(vbrId).getMacMap().getStatus();
                MacEntry ment = vmst.get(eaddr);
                if (ment == null) {
                    ment = new MacEntry(eaddr, vid, pid, pname, ipn);
                } else {
                    ment = ment.addIpAddress(ipn);
                }
                vmst.add(ment);
                mwaiter.add(ment);
            }

            learnHost(ofmock, getTest(), vmst.getMappedVlans(portSet), th);
        }

        verify(mmapSrv);
    }

    /**
     * Move the specified host to the specified switch port.
     *
     * @param mmapSrv   vtn-mac-map service.
     * @param index     The index number that specifies the vBridge that maps
     *                  the specified host.
     * @param eaddr     The MAC address of the target host.
     * @param pid       The MD-SAL port identifier.
     * @param mappings  A map that specifies active MAC mappings.
     * @throws Exception  An error occurred.
     */
    private void moveHost(VtnMacMapService mmapSrv, int index,
                          EtherAddress eaddr, String pid,
                          Map<MacVlan, MapArg> mappings) throws Exception {
        VirtualNetwork vnet = getVirtualNetwork();
        VBridgeIdentifier vbrId = vbridgeIds.get(index);
        VTNMacMapStatus vmst = vnet.getBridge(vbrId).getMacMap().getStatus();
        MacEntry ment = vmst.get(eaddr);
        assertNotNull(ment);

        MacVlan mv = new MacVlan(eaddr, ment.getVlanId());
        MapArg arg = mappings.get(mv);
        assertNotNull(arg);
        arg.setPortId(pid);

        String pname = portNames.get(pid);
        MacEntry newEnt = ment.setPortIdentifier(pid, pname);
        restore(mmapSrv, index, Collections.singletonList(newEnt));
    }

    /**
     * Remove MAC mapped host information that were detected on the specified
     * switch.
     *
     * @param nid  The MD-SAL node identifier that specifies the switch to
     *             be removed.
     * @return  A map that keeps removed host information.
     */
    private Map<Integer, List<MacEntry>> removeByNode(String nid) {
        VirtualNetwork vnet = getVirtualNetwork();
        Map<Integer, List<MacEntry>> removedMap = new HashMap<>();
        for (int i = 0; i < vbridgeIds.size(); i++) {
            VBridgeIdentifier vbrId = vbridgeIds.get(i);
            VTNMacMapStatus vmst = vnet.getBridge(vbrId).
                getMacMap().getStatus();
            List<MacEntry> removed = vmst.removeByNode(nid);
            if (!removed.isEmpty()) {
                removedMap.put(i, removed);
                if (vmst.isEmpty()) {
                    vnodeWaiter.set(vbrId, VnodeState.DOWN);
                }
                macWaiters.get(i).filterOutByNode(nid);
            }
        }
        assertEquals(false, removedMap.isEmpty());

        return removedMap;
    }

    /**
     * Remove MAC mapped host information that were detected on the specified
     * switch port.
     *
     * @param pids  An array of MD-SAL port identifiers.
     * @return  A map that keeps removed host information.
     */
    private Map<Integer, List<MacEntry>> removeByPort(String ... pids) {
        VirtualNetwork vnet = getVirtualNetwork();
        Map<Integer, List<MacEntry>> removedMap = new HashMap<>();
        for (int i = 0; i < vbridgeIds.size(); i++) {
            VBridgeIdentifier vbrId = vbridgeIds.get(i);
            VTNMacMapStatus vmst = vnet.getBridge(vbrId).
                getMacMap().getStatus();
            List<MacEntry> removed = new ArrayList<>();
            for (String pid: pids) {
                removed.addAll(vmst.removeByPort(pid));
            }
            if (!removed.isEmpty()) {
                removedMap.put(i, removed);
                if (vmst.isEmpty()) {
                    vnodeWaiter.set(vbrId, VnodeState.DOWN);
                }

                Set<String> pidSet = new HashSet<>();
                for (String pid: pids) {
                    pidSet.add(pid);
                }
                macWaiters.get(i).filterOut(pidSet);
            }
        }
        assertEquals(false, removedMap.isEmpty());

        return removedMap;
    }

    /**
     * Restore MAC mappings.
     *
     * @param mmapSrv   vtn-mac-map service.
     * @param index     The index number for the target vBridge.
     * @param mentries  A list of L2 hosts to be mapped by MAC mapping.
     * @throws Exception  An error occurred.
     */
    private void restore(VtnMacMapService mmapSrv, int index,
                         List<MacEntry> mentries) throws Exception {
        sleep(SHORT_DELAY);
        flushTask();

        VTNManagerIT vit = getTest();
        OfMockService ofmock = vit.getOfMockService();
        Set<String> portSet = portNames.keySet();
        VBridgeIdentifier vbrId = vbridgeIds.get(index);
        MacEntryWaiter mwaiter = macWaiters.get(index);
        VirtualNetwork vnet = getVirtualNetwork();
        VTNMacMapStatus vmst = vnet.getBridge(vbrId).getMacMap().getStatus();

        for (MacEntry ment: mentries) {
            mwaiter.add(ment);
            vmst.add(ment);

            EtherAddress eaddr = ment.getMacAddress();
            int vid = ment.getVlanId();
            String pid = ment.getPortIdentifier();
            String pname = portNames.get(pid);
            boolean sent = false;
            Map<String, Set<Integer>> vlans = vmst.getMappedVlans(portSet);
            for (IpNetwork ipn: ment.getIpAddresses()) {
                TestHost th = new TestHost(eaddr, vid, pid, pname, ipn);
                learnHost(ofmock, vit, vlans, th);
                sent = true;
            }

            if (!sent) {
                IpNetwork ipn = new Ip4Network("11.22.33.44");
                IpNetwork dstIp = new Ip4Network("192.168.99.254");
                TestHost th = new TestHost(eaddr, vid, pid, pname, ipn);
                sendBroadcastIcmp(th, dstIp, vlans);
            }
        }

        vnodeWaiter.set(vbrId, VnodeState.UP);
        verify(mmapSrv);
    }

    /**
     * Restore MAC mappings.
     *
     * @param mmapSrv     vtn-mac-map service.
     * @param removedMap  A map that keeps hosts to be mapped by MAC mapping.
     * @throws Exception  An error occurred.
     */
    private void restore(VtnMacMapService mmapSrv,
                         Map<Integer, List<MacEntry>> removedMap)
        throws Exception {
        for (Entry<Integer, List<MacEntry>> entry: removedMap.entrySet()) {
            restore(mmapSrv, entry.getKey(), entry.getValue());
        }
        getVirtualNetwork().verify();
    }

    /**
     * Verify the MAC mapping status.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @throws Exception  An error occurred.
     */
    private void verify(VtnMacMapService mmapSrv) throws Exception {
        sleep(SHORT_DELAY);
        flushTask();

        vnodeWaiter.await();
        for (MacEntryWaiter mwaiter: macWaiters) {
            mwaiter.await();
        }

        VirtualNetwork vnet = getVirtualNetwork();
        for (VBridgeIdentifier vbrId: vbridgeIds) {
            VTNMacMapStatus vmst = vnet.getBridge(vbrId).
                getMacMap().getStatus();
            vmst.checkMappedHost(mmapSrv, vbrId);
        }
    }

    /**
     * Test case for get-mac-mapped-host RPC.
     *
     * @param mmapSrv   vtn-mac-map service.
     * @param index     The index number for the target vBridge.
     */
    private void testGetMacMappedHost(VtnMacMapService mmapSrv, int index) {
        VBridgeIdentifier vbrId = vbridgeIds.get(index);
        VTNMacMapStatus vmst = getVirtualNetwork().getBridge(vbrId).
            getMacMap().getStatus();
        Collection<MacEntry> entries = vmst.get();
        assertEquals(false, entries.isEmpty());

        // Specifying only one MAC address.
        Iterator<MacEntry> it = entries.iterator();
        EtherAddress mac = it.next().getMacAddress();
        vmst.checkMappedHost(mmapSrv, vbrId, mac);

        List<MacAddress> maddrs = new ArrayList<>();
        maddrs.add(mac.getMacAddress());
        if (it.hasNext()) {
            // Specifying 2 MAC addresses.
            maddrs.add(it.next().getMacAddress().getMacAddress());
            vmst.checkMappedHost(mmapSrv, vbrId, maddrs);
        }

        // Ensure that unmapped MAC addresses are ignored.
        maddrs.addAll(unmappedMacAddresses);
        vmst.checkMappedHost(mmapSrv, vbrId, maddrs);

        // Specifying all the mapped MAC addresses.
        maddrs.clear();
        for (MacEntry ment: entries) {
            EtherAddress eaddr = ment.getMacAddress();
            maddrs.add(eaddr.getMacAddress());
        }

        // Duplicate address should be ignored.
        for (MacEntry ment: entries) {
            EtherAddress eaddr = ment.getMacAddress();
            maddrs.add(eaddr.getMacAddress());
        }
        vmst.checkMappedHost(mmapSrv, vbrId, maddrs);

        // Specifying MAC addresses that are not mapped.
        vmst.checkMappedHost(mmapSrv, vbrId, unmappedMacAddresses);
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        VTNManagerIT vit = getTest();
        VtnMacMapService mmapSrv = vit.getMacMapService();
        OfMockService ofmock = vit.getOfMockService();

        // Create 2 nodes.
        BigInteger[] dpids = {
            BigInteger.ONE,
            new BigInteger("18446744073709551615"),
        };
        for (int i = 0; i < dpids.length; i++) {
            nodeIds.add(addNode(ofmock, dpids[i]));
        }

        // Create vBridges.
        VirtualNetwork vnet = getVirtualNetwork();
        for (VBridgeIdentifier vbrId: vbridgeIds) {
            vnet.addBridge(vbrId);
        }
        vnet.apply();

        // Map untagged frame except for macAddresses[7], and
        // macAddresses[5]@1 to vbrId0.
        VTNMacMapConfig mmap0 = new VTNMacMapConfig().
            addAllowed(new MacVlan(MacVlan.UNDEFINED, 0),
                       new MacVlan(macAddresses.get(5), 1)).
            addDenied(new MacVlan(macAddresses.get(7), 0));
        vnet.getBridge(vbridgeIds.get(0)).
            setMacMap(mmap0);

        // Map macAddresses[0]@1, macAddresses[1]@2, and macAddresses[2]@0 to
        // vbrId1.
        VTNMacMapConfig mmap1 = new VTNMacMapConfig().
            addAllowed(new MacVlan(macAddresses.get(0), 1),
                       new MacVlan(macAddresses.get(1), 2),
                       new MacVlan(macAddresses.get(2), 0));
        vnet.getBridge(vbridgeIds.get(1)).
            setMacMap(mmap1);

        // Map macAddresses[3]@1, macAddresses[4]@1, macAddresses[5]@2,
        // macAddresses[6]@2, and macAddresses[7]@4095 to vbrId2.
        VTNMacMapConfig mmap2 = new VTNMacMapConfig().
            addAllowed(new MacVlan(macAddresses.get(3), 1),
                       new MacVlan(macAddresses.get(4), 1),
                       new MacVlan(macAddresses.get(5), 2),
                       new MacVlan(macAddresses.get(6), 2),
                       new MacVlan(macAddresses.get(7), 4095));
        vnet.getBridge(vbridgeIds.get(2)).
            setMacMap(mmap2);
        vnet.apply();

        // All the MAC mappings should be inactivated.
        for (int i = 0; i < vbridgeIds.size(); i++) {
            VBridgeIdentifier vbrId = vbridgeIds.get(i);
            MacEntryWaiter mwaiter = macWaiters.get(i);
            VTNMacMapStatus vmst = vnet.getBridge(vbrId).getMacMap().
                getStatus();
            vmst.checkMappedHost(mmapSrv, vbrId, true);
            vmst.checkMappedHost(mmapSrv, vbrId, false);
            vnodeWaiter.set(vbrId, VnodeState.DOWN);
            mwaiter.clear().await();
        }
        vnodeWaiter.await();

        // Learn hosts on untagged network by sending ICMP packet.
        Map<MacVlan, MapArg> mappings = new HashMap<>();
        Ip4Network ibase = new Ip4Network("10.200.94.1");
        int vlanId = 0;
        Set<String> portSet = portNames.keySet();
        for (int i = 0; i < macAddresses.size(); i++) {
            EtherAddress mac = macAddresses.get(i);
            int idx;
            String pid;
            if (i == 7) {
                // Should be dropped.
                idx = MapArg.INVALID;
                pid = getPortId(0, 0);
            } else if (i == 2) {
                // Should be mapped to vbrId1.
                idx = 1;
                pid = getPortId(1, 1);
                vnodeWaiter.set(vbridgeIds.get(idx), VnodeState.UP);
            } else {
                // Should be mapped to vbrId0.
                idx = 0;
                pid = (i == 0) ? getPortId(1, 0) : getPortId(0, 0);
                vnodeWaiter.set(vbridgeIds.get(idx), VnodeState.UP);
            }

            IpNetwork ipn = new Ip4Network(ibase.getAddress() + i);
            MacVlan mv = new MacVlan(mac, vlanId);
            MapArg arg = new MapArg(idx, pid, ipn);
            assertEquals(null, mappings.put(mv, arg));
        }
        learnByIcmp(mmapSrv, vlanId, mappings);

        // Learn hosts on untagged network by sending ARP packet.
        // This will set IP address to MAC address table entry.
        learn(mmapSrv, vlanId, mappings);

        // Move macAddresses[0]@0 from port(1, 0) to port(0, 0).
        moveHost(mmapSrv, 0, macAddresses.get(0), getPortId(0, 0), mappings);

        // Move macAddresses[2]@0 from port(1, 1) to port(1, 0).
        moveHost(mmapSrv, 1, macAddresses.get(2), getPortId(1, 0), mappings);

        // Learn hosts on untagged network by sending ARP packet with changing
        // IP address. This will append IP address to MAC address table entry.
        ibase = new Ip4Network("192.168.33.1");
        for (int i = 0; i < macAddresses.size(); i++) {
            IpNetwork ipn = new Ip4Network(ibase.getAddress() + i);
            MacVlan mv = new MacVlan(macAddresses.get(i), vlanId);
            MapArg arg = mappings.get(mv);
            arg.setIpAddress(ipn);
        }
        learn(mmapSrv, vlanId, mappings);

        // Learn hosts on VLAN 1.
        ibase = new Ip4Network("10.20.30.1");
        vlanId = 1;
        for (int i = 0; i < macAddresses.size(); i++) {
            EtherAddress mac = macAddresses.get(i);
            int idx;
            String pid;
            if (i == 0) {
                // Should be mapped to vbrId1.
                idx = 1;
                pid = getPortId(0, 1);
            } else if (i == 3 || i == 4) {
                // Should be mapped to vbrId2.
                idx = 2;
                pid = getPortId(1, 1);
                vnodeWaiter.set(vbridgeIds.get(idx), VnodeState.UP);
            } else {
                // Should be dropped.
                // Although macAddresses[5]@1 is mapped to vbrId0, it should be
                // ignored because macAddresses[5]@0 is already mapped to
                // vbrId0.
                idx = MapArg.INVALID;
                pid = getPortId(0, 0);
            }

            IpNetwork ipn = new Ip4Network(ibase.getAddress() + i);
            MacVlan mv = new MacVlan(mac, vlanId);
            MapArg arg = new MapArg(idx, pid, ipn);
            assertEquals(null, mappings.put(mv, arg));
        }
        learn(mmapSrv, vlanId, mappings);

        // Learn hosts on VLAN 2.
        ibase = new Ip4Network("11.22.33.10");
        vlanId = 2;
        for (int i = 0; i < macAddresses.size(); i++) {
            EtherAddress mac = macAddresses.get(i);
            int idx;
            String pid;
            if (i == 1) {
                // Should be mapped to vbrId1.
                idx = 1;
                pid = getPortId(1, 1);
            } else if (i == 5 || i == 6) {
                // Should be mapped to vbrId2.
                idx = 2;
                pid = getPortId(1, 0);
            } else {
                // Should be dropped.
                idx = MapArg.INVALID;
                pid = getPortId(0, 0);
            }

            IpNetwork ipn = new Ip4Network(ibase.getAddress() + i);
            MacVlan mv = new MacVlan(mac, vlanId);
            MapArg arg = new MapArg(idx, pid, ipn);
            assertEquals(null, mappings.put(mv, arg));
        }
        learn(mmapSrv, vlanId, mappings);

        // Learn hosts on VLAN 4095.
        ibase = new Ip4Network("192.168.123.190");
        vlanId = 4095;
        for (int i = 0; i < macAddresses.size(); i++) {
            EtherAddress mac = macAddresses.get(i);
            int idx;
            String pid;
            if (i == 7) {
                // Should be mapped to vbrId2.
                idx = 2;
                pid = getPortId(0, 1);
            } else {
                // Should be dropped.
                idx = MapArg.INVALID;
                pid = getPortId(1, 1);
            }

            IpNetwork ipn = new Ip4Network(ibase.getAddress() + i);
            MacVlan mv = new MacVlan(mac, vlanId);
            MapArg arg = new MapArg(idx, pid, ipn);
            assertEquals(null, mappings.put(mv, arg));
        }
        learn(mmapSrv, vlanId, mappings);

        // Run tests for get-mac-mapped-host RPC.
        for (int i = 0; i < vbridgeIds.size(); i++) {
            testGetMacMappedHost(mmapSrv, i);
        }

        // Map untagged frame to vbrId2 using VLAN mapping.
        // This should never affect MAC mappings.
        VtnVlanMapService vmapSrv = vit.getVlanMapService();
        VTNVlanMapConfig vmap = new VTNVlanMapConfig();
        String vmapId = vmap.getMapId();
        VBridgeIdentifier vbrId2 = vbridgeIds.get(2);
        VBridgeConfig bconf2 = vnet.getBridge(vbrId2).
            addVlanMap(vmap);
        AddVlanMapInput vmIn = vmap.newInputBuilder(vbrId2).build();
        AddVlanMapOutput vmOut = getRpcOutput(vmapSrv.addVlanMap(vmIn));
        assertEquals(vmapId, vmOut.getMapId());
        vnet.verify();
        verify(mmapSrv);

        // Map VLAN 2 on port(1, 0) to vbrId0.
        // This will supersede MAC mappings in vbrId2.
        String iname = "if_1";
        VInterfaceConfig iconf = new VInterfaceConfig();
        VBridgeIdentifier vbrId0 = vbridgeIds.get(0);
        VBridgeIfIdentifier ifId = vbrId0.childInterface(iname);
        VBridgeConfig bconf0 = vnet.getBridge(vbrId0).
            addInterface(iname, iconf);
        VtnVinterfaceService ifSrv = vit.getVinterfaceService();
        assertEquals(VtnUpdateType.CREATED,
                     iconf.update(ifSrv, ifId, null, null));
        String portId10 = getPortId(1, 0);
        VTNPortMapConfig pmap = new VTNPortMapConfig(portId10, 2).
            setMappedPort(portId10);
        iconf.setPortMap(pmap);
        VtnPortMapService pmapSrv = vit.getPortMapService();
        assertEquals(VtnUpdateType.CREATED, pmap.update(pmapSrv, ifId));

        VTNMacMapStatus vmst2 = mmap2.getStatus();
        List<MacEntry> removed = vmst2.removeByPort(portId10);
        assertEquals(false, removed.isEmpty());
        macWaiters.get(2).filterOut(portId10);
        verify(mmapSrv);
        vnet.verify();

        // Remove VLAN and port mappings, and restore MAC mappings.
        assertEquals(VtnUpdateType.REMOVED, removeVlanMap(vbrId2, vmapId));
        bconf2.removeVlanMap(vmapId);
        removeVinterface(ifId);
        bconf0.removeInterface(iname);
        restore(mmapSrv, 2, removed);
        vnet.verify();

        // Add macAddresses[3]@0 to the denied host set for vbrId0.
        EtherAddress mac3 = macAddresses.get(3);
        MacVlan mvKey = new MacVlan(mac3, 0);
        mmap0.addDenied(mvKey);
        VTNMacMapStatus vmst0 = mmap0.getStatus();
        MacEntry rment = vmst0.get(mac3);
        assertNotNull(rment);
        MacEntryWaiter mwaiter0 = macWaiters.get(0);
        mwaiter0.remove(mac3);
        vmst0.remove(mac3);
        assertEquals(VtnUpdateType.CHANGED,
                     mmap0.update(mmapSrv, vbrId0, VtnAclType.DENY,
                                  VtnUpdateOperationType.SET));
        verify(mmapSrv);
        vnet.verify();

        // Restore MAC mappings.
        mmap0.removeDenied(mvKey);
        assertEquals(VtnUpdateType.CHANGED,
                     mmap0.update(mmapSrv, vbrId0, VtnAclType.DENY,
                                  VtnUpdateOperationType.SET));
        restore(mmapSrv, 0, Collections.singletonList(rment));
        vnet.verify();

        // Remove untagged frame mapping from vbrId0.
        mvKey = new MacVlan(MacVlan.UNDEFINED, 0);
        mmap0.removeAllowed(mvKey);
        removed = vmst0.removeByVlan(0);
        assertEquals(false, removed.isEmpty());
        mwaiter0.filterOut(0);
        vnodeWaiter.set(vbrId0, VnodeState.DOWN);
        assertEquals(VtnUpdateType.CHANGED,
                     mmap0.update(mmapSrv, vbrId0, VtnAclType.ALLOW,
                                  VtnUpdateOperationType.SET));
        verify(mmapSrv);
        vnet.verify();

        // Restore MAC mappings.
        mmap0.addAllowed(mvKey);
        assertEquals(VtnUpdateType.CHANGED,
                     mmap0.update(mmapSrv, vbrId0, VtnAclType.ALLOW,
                                  VtnUpdateOperationType.SET));
        restore(mmapSrv, 0, removed);
        vnet.verify();

        // Remove macAddresses[6]@2 from the allowed host set for vbrId2.
        EtherAddress mac6 = macAddresses.get(6);
        mvKey = new MacVlan(mac6, 2);
        mmap2.removeAllowed(mvKey);
        rment = vmst2.get(mac6);
        assertNotNull(rment);
        MacEntryWaiter mwaiter2 = macWaiters.get(2);
        mwaiter2.remove(mac6);
        vmst2.remove(mac6);
        VTNMacMapConfig tmpMap = new VTNMacMapConfig().
            addAllowed(mvKey);
        assertEquals(VtnUpdateType.CHANGED,
                     tmpMap.update(mmapSrv, vbrId2, VtnAclType.ALLOW,
                                   VtnUpdateOperationType.REMOVE));
        verify(mmapSrv);
        vnet.verify();

        // Restore MAC mappings.
        mmap2.addAllowed(mvKey);
        assertEquals(VtnUpdateType.CHANGED,
                     tmpMap.update(mmapSrv, vbrId2, VtnAclType.ALLOW,
                                   VtnUpdateOperationType.ADD));
        restore(mmapSrv, 2, Collections.singletonList(rment));
        vnet.verify();

        // Make port(0, 1) down.
        // This will inactivate MAC mappings on port(0, 1).
        String portId01 = getPortId(0, 1);
        ofmock.setPortState(portId01, false, false);
        Map<Integer, List<MacEntry>> removedMap = removeByPort(portId01);
        verify(mmapSrv);
        vnet.verify();

        // Restore port state and MAC mappings.
        ofmock.setPortState(portId01, true);
        restore(mmapSrv, removedMap);

        // Connect port(0, 0) with port(1, 1).
        // This will inactivates MAC mappings on port(0, 0) and port(1, 1).
        String portId00 = getPortId(0, 0);
        String portId11 = getPortId(1, 1);
        ofmock.setPeerIdentifier(portId00, portId11, false);
        ofmock.setPeerIdentifier(portId11, portId00, false);
        removedMap = removeByPort(portId00, portId11);
        verify(mmapSrv);
        vnet.verify();

        // Restore link state and MAC mappings.
        ofmock.setPeerIdentifier(portId00, null);
        ofmock.setPeerIdentifier(portId11, null);
        restore(mmapSrv, removedMap);

        // Remove port(0, 1).
        // This will inactivate MAC mappings on port(0, 1).
        removePort(ofmock, portId01);
        removedMap = removeByPort(portId01);
        verify(mmapSrv);
        vnet.verify();

        // Restore port state and MAC mappings.
        String nodeId0 = nodeIds.get(0);
        assertEquals(portId01, addPort(ofmock, nodeId0, 2L));
        nodeMap.get(nodeId0).add(1, portId01);
        restore(mmapSrv, removedMap);

        // Remove nodeId0 and nodeId1.
        for (String nid: nodeIds) {
            removeNode(ofmock, nid);
            removeByNode(nid);
            verify(mmapSrv);
            vnet.verify();
        }

        // Remove VTNs.
        Set<String> tenants = new HashSet<>(vnet.getTenants().keySet());
        for (String tname: tenants) {
            removeVtn(tname);
            vnet.removeTenant(tname).verify();
        }
    }
}
