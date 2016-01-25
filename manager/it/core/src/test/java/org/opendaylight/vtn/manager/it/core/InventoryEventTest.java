/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_OPENFLOW;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
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
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * Ensure that the state of virtual bridge and virtual interface are changed
 * according to inventory events.
 */
public final class InventoryEventTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public InventoryEventTest(VTNManagerIT vit) {
        super(vit);
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
        OfMockService ofmock = vit.getOfMockService();

        // Create a vBridge that does not map any network.
        String tname = "vtn";
        String iname = "if";
        String nomap = "nomap";
        VBridgeIdentifier bpathNomap = new VBridgeIdentifier(tname, nomap);
        VBridgeIfIdentifier bipathNomap = bpathNomap.childInterface(iname);

        // Create a vBridge that maps all untagged network.
        VBridgeIdentifier bpathVlan0 = new VBridgeIdentifier(tname, "vlan0");

        // Create a vBridge that maps VLAN 1 on node 1.
        VBridgeIdentifier bpathNode1 = new VBridgeIdentifier(tname, "node1");

        // Create a vBridge that maps port 1 on node 2.
        VBridgeIdentifier bpathNode2Port1 =
            new VBridgeIdentifier(tname, "node2_port1");
        VBridgeIfIdentifier bipathNode2Port1 =
            bpathNode2Port1.childInterface(iname);

        // Create a vBridge that maps a host at port 2 on node 2 using
        // MAC mapping.
        VBridgeIdentifier bpathMac = new VBridgeIdentifier(tname, "mac");

        // Create a vTerminal that does not map any network.
        VTerminalIdentifier vtpathNomap =
            new VTerminalIdentifier(tname, nomap);
        VTerminalIfIdentifier vtipathNomap = vtpathNomap.childInterface(iname);

        // Create a vTerminal that maps port 3 on node 1.
        VTerminalIdentifier vtpathNode1Port3 =
            new VTerminalIdentifier(tname, "node1_port3");
        VTerminalIfIdentifier vtipathNode1Port3 =
            vtpathNode1Port3.childInterface(iname);

        VirtualNetwork vnet = getVirtualNetwork().
            addBridge(bpathVlan0, bpathNode1, bpathMac).
            addInterface(bipathNomap, bipathNode2Port1,
                         vtipathNomap, vtipathNode1Port3).
            apply().
            verify();

        VNodeStateWaiter waiter = new VNodeStateWaiter(ofmock).
            set(bpathNomap, VnodeState.UNKNOWN).
            set(bipathNomap, VnodeState.UNKNOWN, VnodeState.UNKNOWN).
            set(bpathVlan0, VnodeState.UNKNOWN).
            set(bpathNode1, VnodeState.UNKNOWN).
            set(bpathNode2Port1, VnodeState.UNKNOWN).
            set(bipathNode2Port1, VnodeState.UNKNOWN, VnodeState.UNKNOWN).
            set(bpathMac, VnodeState.UNKNOWN).
            set(vtpathNomap, VnodeState.UNKNOWN).
            set(vtipathNomap, VnodeState.UNKNOWN, VnodeState.UNKNOWN).
            set(vtpathNode1Port3, VnodeState.UNKNOWN).
            set(vtipathNode1Port3, VnodeState.UNKNOWN, VnodeState.UNKNOWN).
            await();

        // Map VLAN 0 to bpathVlan0.
        VTNVlanMapConfig vmap0 = new VTNVlanMapConfig(0);
        VBridgeConfig bconfVlan0 = vnet.getBridge(bpathVlan0).
            addVlanMap(vmap0);
        vnet.apply().verify();
        waiter.set(bpathVlan0, VnodeState.UP).await();

        // Map VLAN 1 on node1 to bpathNode1.
        BigInteger dpid1 = BigInteger.ONE;
        String nid1 = ID_OPENFLOW + dpid1;
        VTNVlanMapConfig vmap1 = new VTNVlanMapConfig(nid1, 1).
            setActive(false);
        VBridgeConfig bconfNode1 = vnet.getBridge(bpathNode1).
            addVlanMap(vmap1);
        vnet.apply().verify();
        waiter.set(bpathNode1, VnodeState.DOWN).await();

        // Map VLAN 2 on port1 on node 2 to bpathNode2Port1.
        BigInteger dpid2 = BigInteger.valueOf(2L);
        String nid2 = ID_OPENFLOW + dpid2;
        VTNPortMapConfig pmap = new VTNPortMapConfig(nid2, "1", null, 2);
        VInterfaceConfig biconfNode2Port1 =
            vnet.getInterface(bipathNode2Port1).
            setPortMap(pmap).
            setState(VnodeState.DOWN).
            setEntityState(VnodeState.UNKNOWN);
        vnet.apply().verify();
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UNKNOWN).
            await();

        // Map a host at port 2 on node 2 to bpathMac.
        String pid22 = OfMockUtils.getPortIdentifier(nid2, 2L);
        TestHost host = new TestHost(1, pid22, "eth2", 4095);
        int hostIdx = 2;
        VTNMacMapConfig macMap = new VTNMacMapConfig().
            addAllowed(new MacVlan(host.getEtherAddress(), host.getVlanId()));
        VBridgeConfig bconfMac = vnet.getBridge(bpathMac).setMacMap(macMap);
        vnet.apply().verify();
        waiter.set(bpathMac, VnodeState.DOWN).await();

        // Map VLAN 10 on port 3 on node 1 to vtpathNode1Port3.
        pmap = new VTNPortMapConfig(nid1, "3", null, 10);
        VInterfaceConfig ticonfNode1Port3 = vnet.
            getInterface(vtipathNode1Port3).
            setPortMap(pmap).
            setState(VnodeState.DOWN).
            setEntityState(VnodeState.UNKNOWN);
        vnet.apply().verify();
        waiter.set(vtpathNode1Port3, VnodeState.DOWN).
            set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.UNKNOWN).
            await();

        List<String> nodeIds = new ArrayList<>();
        final long nports = 4L;
        final int npids = (int)nports + 1;
        String[][] pids = {
            null,
            new String[npids],
            new String[npids],
            new String[npids],
        };

        // Create a non-OpenFlow node.
        // This should never affect virtual node state.
        String badProto = "unknown:";
        String badNid = badProto + dpid1;
        nodeIds.add(badNid);
        ofmock.addNode(badProto, dpid1, false);
        String[] badPorts = new String[npids];
        for (long idx = 1; idx <= nports; idx++) {
            badPorts[(int)idx] = ofmock.addPort(badNid, idx, false);
        }
        waiter.await();
        vnet.verify();

        // Create node 3 that will not be mapped to virtual nodes except
        // bpathVlan0. This should never affect virtual node state because
        // no edge port is available yet.
        BigInteger dpid3 = BigInteger.valueOf(3L);
        String nid3 = ID_OPENFLOW + dpid3;
        ofmock.addNode(dpid3);
        for (long idx = 1; idx <= nports; idx++) {
            pids[3][(int)idx] = ofmock.addPort(nid3, idx, false);
        }
        for (int i = 1; i < npids; i++) {
            ofmock.awaitPortCreated(pids[3][i]);
        }
        waiter.await();
        vnet.verify();

        // Up port 2 on node 3.
        ofmock.setPortState(pids[3][2], true, false);
        waiter.await();
        vnet.verify();

        // Create node 1.
        // This should never affect virtual node state because no edge port
        // is available yet.
        ofmock.addNode(dpid1);
        waiter.await();
        vnet.verify();

        // Add ports to node 1 except port 3.
        // This should never affect virtual node state because:
        //   - Port 3, which will be mapped to vtipathNode1Port3, is not yet
        //     added.
        //   - All edge ports are down.
        for (long idx = 1; idx <= nports; idx++) {
            if (idx != 3) {
                pids[1][(int)idx] = ofmock.addPort(nid1, idx, false);
            }
        }
        for (int i = 1; i < npids; i++) {
            if (i != 3) {
                ofmock.awaitPortCreated(pids[1][i]);
            }
        }
        waiter.await();
        vnet.verify();

        // Enable port 4 on node 1.
        // This will activate bpathNode1.
        ofmock.setPortState(pids[1][4], true, false);
        vmap1.setActive(true);
        waiter.set(bpathNode1, VnodeState.UP).await();
        vnet.verify();

        // Add port 3 on node 1.
        // This will establish port mapping on vtipathNode1Port3.
        pids[1][3] = ofmock.addPort(nid1, 3L, false);
        waiter.set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.DOWN).
            await();
        ticonfNode1Port3.
            setEntityState(VnodeState.DOWN).
            getPortMap().
            setMappedPort(pids[1][3]);
        vnet.verify();

        // Enable port 3 on node 1.
        // This will activate vtpathNode1Port3.
        ofmock.setPortState(pids[1][3], true, false);
        waiter.set(vtpathNode1Port3, VnodeState.UP).
            set(vtipathNode1Port3, VnodeState.UP, VnodeState.UP).
            await();
        ticonfNode1Port3.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        vnet.verify();

        // Create node 2.
        // This should never affect virtual node state because no edge port
        // is available yet.
        ofmock.addNode(dpid2);
        waiter.await();

        // Add ports to node 2 except port 1.
        // This should never affect virtual node state because:
        //   - Port 1, which will be mapped to bipathNode2Port1, is not yet
        //     added.
        //   - Although port 2 is created, no host is detected yet on port 2.
        for (long idx = 1; idx <= nports; idx++) {
            if (idx != 1) {
                String pid = ofmock.addPort(nid2, idx, false);
                pids[2][(int)idx] = pid;
                ofmock.setPortState(pid, true, false);
            }
        }
        for (int i = 1; i < npids; i++) {
            String pid = pids[2][i];
            if (pid != null) {
                ofmock.awaitLinkState(pid, true);
            }
        }
        waiter.await();

        // Create port 1.
        // This will establish port mapping on bipathNode2Port1.
        pids[2][1] = ofmock.addPort(nid2, 1L);
        waiter.set(bipathNode2Port1, VnodeState.DOWN, VnodeState.DOWN).await();
        biconfNode2Port1.setEntityState(VnodeState.DOWN).
            getPortMap().
            setMappedPort(pids[2][1]);
        vnet.verify();

        // Enable port 1.
        ofmock.setPortState(pids[2][1], true, false);
        waiter.set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).
            await();
        biconfNode2Port1.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        vnet.verify();

        // Enable all ports, and let the vBridge at bpathVlan0 learn some
        // MAC addresses.
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                ofmock.setPortState(ports[j], true, false);
            }
        }

        // Need to synchronize background tasks triggered by port state events.
        sleep(BGTASK_DELAY);

        MacEntryWaiter bpathVlan0Hosts =
            new MacEntryWaiter(ofmock, bpathVlan0);
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                String pid = ports[j];
                String pname = ofmock.getPortName(pid);
                ofmock.awaitLinkState(pid, true);
                TestHost th = new TestHost(hostIdx, pid, pname, 0);
                hostIdx++;
                sendBroadcast(ofmock, th);
                bpathVlan0Hosts.add(th.getMacEntry());
            }
        }

        // Let all vBridges learn MAC addresses.
        MacEntryWaiter bpathNode1Hosts =
            new MacEntryWaiter(ofmock, bpathNode1);
        for (int i = 1; i < npids; i++) {
            String pid = pids[1][i];
            String pname = ofmock.getPortName(pid);
            TestHost th = new TestHost(hostIdx, pid, pname, 1);
            hostIdx++;
            sendBroadcast(ofmock, th);
            bpathNode1Hosts.add(th.getMacEntry());
        }

        MacEntryWaiter bpathNode2Port1Hosts =
            new MacEntryWaiter(ofmock, bpathNode2Port1);
        MacEntryWaiter bpathMacHosts =
            new MacEntryWaiter(ofmock, bpathMac);
        final int nhosts = 4;
        for (int i = 0; i < nhosts; i++) {
            String pid = pids[2][1];
            String pname = ofmock.getPortName(pid);
            TestHost th = new TestHost(hostIdx, pid, pname, 2);
            hostIdx++;
            sendBroadcast(ofmock, th);
            bpathNode2Port1Hosts.add(th.getMacEntry());

            pid = pids[2][2];
            pname = ofmock.getPortName(pid);
            th = new TestHost(hostIdx, pid, pname, host.getVlanId());
            hostIdx++;
            sendBroadcast(ofmock, th);
        }
        waiter.await();
        bpathMacHosts.set(Collections.<MacEntry>emptySet()).await();

        // Send a broadcast packet from host.
        // This will activate bpathMac.
        sendBroadcast(ofmock, host);
        bpathMacHosts.add(host.getMacEntry()).await();
        waiter.set(bpathMac, VnodeState.UP).await();
        VTNMacMapStatus vmst = macMap.getStatus();
        vmst.add(host.getMacEntry());
        vnet.verify();
        bpathVlan0Hosts.await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable all ports on a non-OpenFlow node.
        // This should never affect virtual node state.
        for (int i = 1; i < npids; i++) {
            ofmock.setPortState(badPorts[i], false, false);
        }
        waiter.await();
        vnet.verify();

        // Remove a non-OpenFlow node.
        // This should never affect virtual node state.
        ofmock.removeNode(badNid);
        waiter.await();
        vnet.verify();

        // Disable port 1 on node 2.
        ofmock.setPortState(pids[2][1], false, false);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.DOWN).
            await();
        biconfNode2Port1.setState(VnodeState.DOWN).
            setEntityState(VnodeState.DOWN);
        vnet.verify();

        Set<MacEntry> bpathVlan0Entries =
            new HashSet<>(bpathVlan0Hosts.getMacEntries());
        Set<MacEntry> bpathNode1Entries =
            new HashSet<>(bpathNode1Hosts.getMacEntries());
        Set<MacEntry> bpathNode2Port1Entries =
            new HashSet<>(bpathNode2Port1Hosts.getMacEntries());
        Set<MacEntry> bpathMacEntries =
            new HashSet<>(bpathMacHosts.getMacEntries());
        bpathVlan0Hosts.filterOut(pids[2][1]).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.clear().await();
        bpathMacHosts.await();

        // Disable port 2 on node 2.
        ofmock.setPortState(pids[2][2], false, false);
        waiter.set(bpathMac, VnodeState.DOWN).await();
        vmst.remove(host.getEtherAddress());
        vnet.verify();

        bpathVlan0Hosts.filterOut(pids[2][2]).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.clear().await();

        // Disable all ports on node 1 but port 4.
        Set<String> pidSet = new HashSet<>();
        for (int i = 1; i < npids; i++) {
            if (i != 4) {
                String pid = pids[1][i];
                ofmock.setPortState(pid, false, false);
                pidSet.add(pid);
            }
        }
        bpathVlan0Hosts.filterOut(pidSet);
        bpathNode1Hosts.filterOut(pidSet);

        for (int i = 1; i < npids; i++) {
            if (i != 4) {
                ofmock.awaitLinkState(pids[1][i], false);
            }
        }

        // vtipathNode1Port3 should be changed to DOWN because it maps
        // pids[1][3].
        waiter.set(vtpathNode1Port3, VnodeState.DOWN).
            set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.DOWN).
            await();
        ticonfNode1Port3.setState(VnodeState.DOWN).
            setEntityState(VnodeState.DOWN);
        vnet.verify();

        bpathVlan0Hosts.await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable port 4 on node 1.
        ofmock.setPortState(pids[1][4], false);
        waiter.set(bpathNode1, VnodeState.DOWN).await();
        vmap1.setActive(false);
        vnet.verify();

        bpathVlan0Hosts.filterOut(pids[1][4]).await();
        bpathNode1Hosts.clear().await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable all ports on node 3.
        pidSet.clear();
        for (int i = 1; i < npids; i++) {
            String pid = pids[3][i];
            ofmock.setPortState(pid, false, false);
            pidSet.add(pid);
        }

        for (int i = 1; i < npids; i++) {
            ofmock.awaitLinkState(pids[3][i], false);
        }
        waiter.await();
        vnet.verify();
        bpathVlan0Hosts.filterOut(pidSet).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable port 3 on node 2.
        ofmock.setPortState(pids[2][3], false);
        waiter.await();
        vnet.verify();
        bpathVlan0Hosts.filterOut(pids[2][3]).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable port 4 on node 2.
        ofmock.setPortState(pids[2][4], false);
        waiter.await();
        vnet.verify();
        bpathVlan0Hosts.clear().await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Up all ports, and restore MAC address table entries.
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                ofmock.setPortState(ports[j], true, false);
            }
        }
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                ofmock.awaitLinkState(ports[j], true);
            }
        }

        // Need to synchronize background tasks triggered by port state events.
        sleep(BGTASK_DELAY);

        for (MacEntry ment: bpathVlan0Entries) {
            sendBroadcast(ofmock, ment);
        }
        for (MacEntry ment: bpathNode1Entries) {
            sendBroadcast(ofmock, ment);
        }
        for (MacEntry ment: bpathNode2Port1Entries) {
            sendBroadcast(ofmock, ment);
        }
        for (MacEntry ment: bpathMacEntries) {
            sendBroadcast(ofmock, ment);
        }

        waiter.set(bpathNode1, VnodeState.UP).
            set(vtpathNode1Port3, VnodeState.UP).
            set(vtipathNode1Port3, VnodeState.UP, VnodeState.UP).
            set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).
            set(bpathMac, VnodeState.UP).
            await();
        ticonfNode1Port3.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        biconfNode2Port1.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        vmap1.setActive(true);
        vmst.add(host.getMacEntry());
        vnet.verify();
        bpathVlan0Hosts.set(bpathVlan0Entries).await();
        bpathNode1Hosts.set(bpathNode1Entries).await();
        bpathNode2Port1Hosts.set(bpathNode2Port1Entries).await();
        bpathMacHosts.set(bpathMacEntries).await();

        // Connect port2 on node2 to port1 on node2.
        String src = pids[2][2];
        String dst = pids[2][1];
        ofmock.setPeerIdentifier(src, dst);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UP).
            set(bpathMac, VnodeState.DOWN).
            await();
        biconfNode2Port1.setState(VnodeState.DOWN);
        vmst.remove(host.getEtherAddress());
        vnet.verify();

        pidSet.clear();
        Collections.addAll(pidSet, src, dst);
        bpathVlan0Hosts.filterOut(pidSet).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.clear().await();
        bpathMacHosts.clear().await();

        ofmock.setPeerIdentifier(src, null);
        waiter.set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).
            await();
        biconfNode2Port1.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        vnet.verify();

        for (MacEntry ment: bpathNode2Port1Entries) {
            sendBroadcast(ofmock, ment);
        }
        for (MacEntry ment: bpathMacEntries) {
            sendBroadcast(ofmock, ment);
        }
        waiter.set(bpathMac, VnodeState.UP).await();
        vmst.add(host.getMacEntry());
        vnet.verify();

        bpathVlan0Hosts.await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.set(bpathNode2Port1Entries).await();
        bpathMacHosts.set(bpathMacEntries).await();

        // Remove node 3.
        ofmock.removeNode(nid3);
        waiter.await();
        vnet.verify();
        pidSet.clear();
        for (int i = 1; i < npids; i++) {
            pidSet.add(pids[3][i]);
        }
        bpathVlan0Hosts.filterOut(pidSet).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Remove node 2.
        ofmock.removeNode(nid2);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UNKNOWN).
            set(bpathMac, VnodeState.DOWN).
            await();
        biconfNode2Port1.setState(VnodeState.DOWN).
            setEntityState(VnodeState.UNKNOWN).
            getPortMap().
            setMappedPort(null);
        vmst.remove(host.getEtherAddress());
        vnet.verify();
        pidSet.clear();
        for (int i = 1; i < npids; i++) {
            pidSet.add(pids[2][i]);
        }
        bpathVlan0Hosts.filterOut(pidSet).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.clear().await();
        bpathMacHosts.clear().await();

        // Remove node 1.
        ofmock.removeNode(nid1);
        waiter.set(bpathNode1, VnodeState.DOWN).
            set(vtpathNode1Port3, VnodeState.DOWN).
            set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.UNKNOWN).
            await();
        vmap1.setActive(false);
        ticonfNode1Port3.setState(VnodeState.DOWN).
            setEntityState(VnodeState.UNKNOWN).
            getPortMap().
            setMappedPort(null);
        vnet.verify();
        bpathVlan0Hosts.clear().await();
        bpathNode1Hosts.clear().await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Clean up.
        removeVtn(tname);
        bpathVlan0Hosts.set(null).await();
        bpathNode1Hosts.set(null).await();
        bpathNode2Port1Hosts.set(null).await();
        bpathMacHosts.set(null).await();
        vnet.removeTenant(tname).verify();
    }
}
