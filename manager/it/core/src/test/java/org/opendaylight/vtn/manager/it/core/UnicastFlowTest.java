/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.DEFAULT_TABLE;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.OfMockFlow;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.util.BridgeNetwork;
import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.TestPort;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.it.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlow;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNMacMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNMacMapStatus;
import org.opendaylight.vtn.manager.it.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;

/**
 * Ensure that the VTN Manager can establish unicast flow.
 */
public final class UnicastFlowTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public UnicastFlowTest(VTNManagerIT vit) {
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
        VtnPortMapService pmapSrv = vit.getPortMapService();
        VtnMacMapService mmapSrv = vit.getMacMapService();

        // Create one VTN, 2 vBridges, and 1 vTerminal.
        String tname = "vtn";
        String iname1 = "if1";
        VBridgeIdentifier bpath1 = new VBridgeIdentifier(tname, "vbr1");
        VBridgeIdentifier bpath2 = new VBridgeIdentifier(tname, "vbr2");
        VBridgeIfIdentifier bipath11 = bpath1.childInterface(iname1);
        VBridgeIfIdentifier bipath21 = bpath2.childInterface(iname1);
        VTerminalIfIdentifier vtipath =
            new VTerminalIfIdentifier(tname, "vterm", iname1);
        VirtualNetwork vnet = getVirtualNetwork().
            addInterface(bipath11, bipath21, vtipath).
            apply().
            verify();

        // Collect edge ports per node, and all ISL ports.
        Map<String, List<String>> edgePorts = new HashMap<>();
        List<String> edgeNodes = new ArrayList<>();
        Set<String> islPorts = new HashSet<>();
        Set<String> allPorts = new HashSet<>();
        BridgeNetwork bridge1 = new BridgeNetwork(ofmock, bpath1);
        BridgeNetwork bridge2 = new BridgeNetwork(ofmock, bpath2);
        Map<VBridgeIdentifier, BridgeNetwork> bridges = new HashMap<>();
        Map<String, String> portNames = new HashMap<>();
        bridges.put(bpath1, bridge1);
        bridges.put(bpath2, bridge2);
        for (String nid: ofmock.getNodes()) {
            List<String> ports = ofmock.getPorts(nid, true);
            if (!ports.isEmpty()) {
                edgeNodes.add(nid);
                allPorts.addAll(ports);
                assertEquals(null, edgePorts.put(nid, ports));
            }
            for (String pid: ports) {
                String pname = ofmock.getPortName(pid);
                assertEquals(null, portNames.put(pid, pname));
            }

            for (String pid: ofmock.getPorts(nid, false)) {
                String pname = ofmock.getPortName(pid);
                assertEquals(null, portNames.put(pid, pname));
                assertTrue(islPorts.add(pid));
                allPorts.add(pid);
                bridge1.setUnmappedPort(pid);
                bridge2.setUnmappedPort(pid);
            }
        }
        assertFalse(edgeNodes.isEmpty());
        assertFalse(edgePorts.isEmpty());
        assertFalse(islPorts.isEmpty());
        assertFalse(allPorts.isEmpty());

        // Map untagged network to vbr1 using VLAN mapping.
        VTNVlanMapConfig vmap0 = new VTNVlanMapConfig(0);
        VBridgeConfig bconf1 = vnet.getBridge(bpath1).
            addVlanMap(vmap0);

        // But untagged network on edgeNodes[0].ports[0] is mapped to vbr2
        // using port mapping.
        Set<TestPort> portMapped = new HashSet<>();
        String nid0 = edgeNodes.get(0);
        String targetPort = edgePorts.get(nid0).get(0);
        VTNPortMapConfig pmap0 = new VTNPortMapConfig(targetPort, 0).
            setMappedPort(targetPort);
        VInterfaceConfig iconf21 = vnet.getInterface(bipath21).
            setPortMap(pmap0);
        assertTrue(portMapped.add(new TestPort(targetPort, 0)));

        for (List<String> ports: edgePorts.values()) {
            for (String pid: ports) {
                BridgeNetwork bridge = (pid.equals(targetPort))
                    ? bridge2 : bridge1;
                bridge.addMappedVlan(pid, 0);
            }
        }

        // Map VLAN 1 on edgeNodes[0] to vbr1.
        VTNVlanMapConfig vmap1 = new VTNVlanMapConfig(nid0, 1);
        bconf1.addVlanMap(vmap1);

        // Map VLAN 1 on edgeNodes[1] to vbr2.
        String nid1 = edgeNodes.get(1);
        VTNVlanMapConfig vmap2 = new VTNVlanMapConfig(nid1, 1);
        VBridgeConfig bconf2 = vnet.getBridge(bpath2).
            addVlanMap(vmap2);

        // But one port in edgeNodes[1] is mapped to vbr1 using port mapping.
        targetPort = edgePorts.get(nid1).get(1);
        VTNPortMapConfig pmap1 = new VTNPortMapConfig(targetPort, 1).
            setMappedPort(targetPort);
        VInterfaceConfig iconf11 = vnet.getInterface(bipath11).
            setPortMap(pmap1);
        assertTrue(portMapped.add(new TestPort(targetPort, 1)));

        // Apply configuration.
        vnet.apply().verify();

        String mmapPort = null;
        for (String pid: edgePorts.get(nid0)) {
            if (mmapPort == null) {
                // Override this port using MAC mapping.
                mmapPort = pid;
            }
            bridge1.addMappedVlan(pid, 1);
        }
        assertNotNull(mmapPort);
        String mmapNode = OfMockUtils.getNodeIdentifier(mmapPort);

        for (String pid: edgePorts.get(nid1)) {
            BridgeNetwork bridge = (pid.equals(targetPort))
                ? bridge1 : bridge2;
            bridge.addMappedVlan(pid, 1);
        }

        // Create test hosts.
        final int nhosts = 4;
        int hostIdx = bridge1.addTestHosts(1, nhosts);
        hostIdx = bridge2.addTestHosts(hostIdx, nhosts);

        // Map hosts on mmapPort (VLAN 1) to vbr2 using MAC mapping.
        Map<VBridgeIdentifier, Set<TestHost>> mmapAllowed = new HashMap<>();
        assertEquals(null, mmapAllowed.put(bpath2, new HashSet<TestHost>()));
        VTNMacMapConfig macMap1 = new VTNMacMapConfig();
        bconf2.setMacMap(macMap1);
        for (int i = 0; i < nhosts; i++) {
            String pname = portNames.get(mmapPort);
            TestHost th = new TestHost(hostIdx, mmapPort, pname, 1);
            hostIdx++;
            MacVlan mv = new MacVlan(th.getEtherAddress(), th.getVlanId());
            assertTrue(mmapAllowed.get(bpath2).add(th));
            macMap1.addAllowed(mv);
        }
        assertEquals(VtnUpdateType.CREATED,
                     macMap1.update(mmapSrv, bpath2, VtnAclType.ALLOW,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        // Configure MAC mappings for VLAN 4095.
        Deque<VBridgeIdentifier> bpathQueue = new LinkedList<>();
        Collections.addAll(bpathQueue, bpath1, bpath2);
        for (Entry<String, List<String>> entry: edgePorts.entrySet()) {
            String nid = entry.getKey();
            List<String> ports = entry.getValue();
            for (String pid: ports) {
                VBridgeIdentifier bpath = bpathQueue.removeFirst();
                bpathQueue.addLast(bpath);

                Set<TestHost> allowed = mmapAllowed.get(bpath);
                VtnUpdateType expected;
                if (allowed == null) {
                    expected = VtnUpdateType.CREATED;
                    allowed = new HashSet<TestHost>();
                    assertEquals(null, mmapAllowed.put(bpath, allowed));
                } else {
                    expected = VtnUpdateType.CHANGED;
                }

                VBridgeConfig bconf = vnet.getBridge(bpath);
                VTNMacMapConfig macMap = bconf.getMacMap();
                if (macMap == null) {
                    macMap = new VTNMacMapConfig();
                    bconf.setMacMap(macMap);
                }

                for (int i = 0; i < nhosts; i++) {
                    String pname = portNames.get(pid);
                    TestHost th = new TestHost(hostIdx, pid, pname, 4095);
                    hostIdx++;
                    assertTrue(allowed.add(th));
                    macMap.addAllowed(th.getMacVlan());
                }

                VtnUpdateType result = macMap.update(
                    mmapSrv, bpath, VtnAclType.ALLOW,
                    VtnUpdateOperationType.ADD);
                assertEquals(expected, result);
                vnet.verify();
            }
        }

        bridge1.verify();
        bridge2.verify();

        // Ensure that all virtual interfaces are ready.
        // Note that vBridge state should be DOWN because all MAC mappings are
        // still inactivated.
        VNodeStateWaiter waiter = new VNodeStateWaiter(ofmock).
            set(bpath1, VnodeState.DOWN).set(bpath2, VnodeState.DOWN).
            set(bipath11, VnodeState.UP, VnodeState.UP).
            set(bipath21, VnodeState.UP, VnodeState.UP);
        waiter.await();
        vnet.verify();

        // Ensure any incoming packet from internal port is ignored.
        for (String pid: islPorts) {
            String pname = portNames.get(pid);
            TestHost th = new TestHost(hostIdx, pid, pname, 0);
            hostIdx++;
            sendBroadcast(ofmock, th);
        }
        sleep(SHORT_DELAY);
        for (List<String> ports: edgePorts.values()) {
            for (String pid: ports) {
                assertNull(ofmock.getTransmittedPacket(pid));
            }
        }
        for (String pid: islPorts) {
            assertNull(ofmock.getTransmittedPacket(pid));
        }

        // Let test vBridges learn MAC addresses except MAC mapped hosts.
        learnHosts(bridge1);
        learnHosts(bridge2);
        MacEntryWaiter macWaiter1 = new MacEntryWaiter(ofmock, bpath1).
            set(bridge1.getMacEntries()).
            await();
        MacEntryWaiter macWaiter2 = new MacEntryWaiter(ofmock, bpath2).
            set(bridge2.getMacEntries()).
            await();
        vnet.verify();

        // Send unicast packets.
        List<UnicastFlow> flows1 =
            unicastTest(bridge1, islPorts, true, VnodeState.DOWN);
        List<UnicastFlow> flows2 =
            unicastTest(bridge2, islPorts, true, VnodeState.DOWN);
        FlowCounter counter = new FlowCounter(ofmock).
            add(flows1).add(flows2).verify();

        // Activate MAC mappings.
        Set<PortVlan> remappedVlans = new HashSet<>();
        for (Entry<VBridgeIdentifier, BridgeNetwork> entry:
                 bridges.entrySet()) {
            VBridgeIdentifier bpath = entry.getKey();
            BridgeNetwork bridge = entry.getValue();
            VBridgeConfig bconf = vnet.getBridge(bpath);
            VTNMacMapConfig mmap = bconf.getMacMap();
            VTNMacMapStatus vmst = mmap.getStatus();
            Set<TestHost> allowed = mmapAllowed.get(bpath);
            for (TestHost th: allowed) {
                String pid = th.getPortIdentifier();
                int vid = th.getVlanId();
                PortVlan pv = new PortVlan(pid, vid);
                boolean remapped = remappedVlans.add(pv);
                if (remapped) {
                    bridge1.removeMappedVlan(pid, vid);
                    bridge2.removeMappedVlan(pid, vid);
                }
                bridge.addHost(th);
                learnHost(bpath, bridge.getMappedVlans(), th);
                mmap.addAllowed(th.getMacVlan());
                vmst.add(th.getMacEntry());

                if (remapped) {
                    // Ensure that flow entries for unmapped hosts have been
                    // uninstalled.
                    verifyFlowUninstalled(pid, vid, flows1);
                    verifyFlowUninstalled(pid, vid, flows2);
                }
            }
        }

        // Ensure that unexpected flow enty is not installed.
        counter.clear().add(flows1).add(flows2).verify();

        // Ensure MAC address entries for unmapped hosts have been removed.
        macWaiter1.set(bridge1.getMacEntries()).await();
        macWaiter2.set(bridge2.getMacEntries()).await();

        // Ensure that all virtual nodes are ready.
        waiter.set(bpath1, VnodeState.UP).set(bpath2, VnodeState.UP).await();
        vnet.verify();

        // Send unicast packets again.
        flows1 = unicastTest(bridge1, islPorts, true);
        flows2 = unicastTest(bridge2, islPorts, true);
        counter.clear().add(flows1).add(flows2).verify();

        // Ensure that flow entries are not changed when port mapping fails.
        Set<String> skip = new HashSet<>();
        for (TestPort tp: portMapped) {
            String pid = tp.getPortIdentifier();
            skip.add(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(pid, tp.getVlanId());
            SetPortMapInput input = pmap.newInputBuilder(vtipath).build();
            checkRpcError(pmapSrv.setPortMap(input),
                          RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
            vnet.verify();
            counter.verify();
            macWaiter1.await();
            macWaiter2.await();
        }

        // Determine port for vTerminal test.
        targetPort = null;
        for (String pid: allPorts) {
            if (!skip.contains(pid) && !islPorts.contains(pid)) {
                targetPort = pid;
                break;
            }
        }
        assertNotNull(targetPort);

        // Try to map edge port to vTerminal.
        OfMockFlow dropFlow = null;
        VInterfaceConfig ticonf = vnet.getInterface(vtipath);
        String portName = portNames.get(targetPort);
        for (int vid: new int[]{0, 1, 4095}) {
            VTNPortMapConfig old = ticonf.getPortMap();
            VtnUpdateType expected = (old == null)
                ? VtnUpdateType.CREATED
                : VtnUpdateType.CHANGED;
            VTNPortMapConfig pmap = new VTNPortMapConfig(targetPort, vid).
                setMappedPort(targetPort);
            ticonf.setPortMap(pmap);
            assertEquals(expected, pmap.update(pmapSrv, vtipath));

            verifyFlowUninstalled(targetPort, vid, flows1);
            verifyFlowUninstalled(targetPort, vid, flows2);
            counter.clear().add(flows1).add(flows2).verify();
            macWaiter1.remove(bridge1.removeMappedVlan(targetPort, vid)).
                await();
            macWaiter2.remove(bridge2.removeMappedVlan(targetPort, vid)).
                await();

            if (dropFlow != null) {
                String nid = dropFlow.getNodeIdentifier();
                int pri = dropFlow.getPriority();
                Match match = dropFlow.getMatch();
                OfMockFlow f =
                    ofmock.awaitFlow(nid, DEFAULT_TABLE, match, pri, false);
                assertEquals(null, f);
            }

            // Ensure that vTerminal drops incoming packet.
            TestHost host = new TestHost(hostIdx, targetPort, portName, vid);
            hostIdx++;
            dropFlow = dropTest(host, allPorts);
            counter.add(dropFlow).verify();
            macWaiter1.await();
            macWaiter2.await();
        }

        // Clean up.
        removeVtn(tname);
        vnet.removeTenant(tname).verify();
        macWaiter1.set(null).await();
        macWaiter2.set(null).await();
    }
}
