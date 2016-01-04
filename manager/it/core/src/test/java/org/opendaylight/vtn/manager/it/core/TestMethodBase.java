/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.DEFAULT_TABLE;
import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_SEPARATOR;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.ofmock.OfMockFlow;
import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.util.BridgeNetwork;
import org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowCondSet;
import org.opendaylight.vtn.manager.it.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.it.util.packet.ArpFactory;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.Icmp4Factory;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;
import org.opendaylight.vtn.manager.it.util.pathmap.PathMapSet;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathCost;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathPolicySet;
import org.opendaylight.vtn.manager.it.util.unicast.ArpFlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlow;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlowFactory;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * Abstract base class for classes that implement integration test method.
 */
public abstract class TestMethodBase extends ModelDrivenTestBase {
    /**
     * The bit that indicates the port ID should be specified in the
     * vtn-port-desc.
     */
    protected static final int  PATH_COST_MODE_ID = 0x1;

    /**
     * The bit that indicates the port name should be specified in the
     * vtn-port-desc.
     */
    protected static final int  PATH_COST_MODE_NAME = 0x2;

    /**
     * The test instance.
     */
    private final VTNManagerIT  theTest;

    /**
     * The configuration of the virtual network.
     */
    private VirtualNetwork  virtualNet;

    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    protected TestMethodBase(VTNManagerIT vit) {
        theTest = vit;
    }

    /**
     * Return the IT instance.
     *
     * @return  A {@link VTNManagerIT} instance.
     */
    protected VTNManagerIT getTest() {
        return theTest;
    }

    /**
     * Return the configuration of the virtual network.
     *
     * @return  A {@link VirtualNetwork} instance.
     */
    protected final VirtualNetwork getVirtualNetwork() {
        VirtualNetwork vnet = virtualNet;
        if (vnet == null) {
            vnet = new VirtualNetwork(theTest);
            virtualNet = vnet;
        }

        return vnet;
    }

    /**
     * Remove the specified VTN.
     *
     * @param tname  The name of the VTN.
     */
    protected final void removeVtn(String tname) {
        VTenantConfig.removeVtn(theTest.getVtnService(), tname);
    }

    /**
     * Remove the specified vBridge.
     *
     * @param ident  The identifier for the vBridge.
     */
    protected final void removeVbridge(VBridgeIdentifier ident) {
        VBridgeConfig.removeVbridge(theTest.getVbridgeService(), ident);
    }

    /**
     * Remove the specified vTerminal.
     *
     * @param ident  The identifier for the vTerminal.
     */
    protected final void removeVterminal(VTerminalIdentifier ident) {
        VTerminalConfig.removeVterminal(theTest.getVterminalService(), ident);
    }

    /**
     * Remove the specified virtual interface.
     *
     * @param ident  The identifier for the virtual interface.
     */
    protected final void removeVinterface(VInterfaceIdentifier<?> ident) {
        VInterfaceConfig.removeVinterface(
            theTest.getVinterfaceService(), ident);
    }

    /**
     * Remove all the VLAN mappings configured in the specified vBridge.
     *
     * @param ident  The identifier for the target vBridge.
     * @return  A map that specifies the removed VLAN mappings.
     */
    protected final Map<String, VtnUpdateType> removeVlanMap(
        VBridgeIdentifier ident) {
        return removeVlanMap(ident, null);
    }

    /**
     * Remove the specified VLAN mapping.
     *
     * @param ident   The identifier for the target vBridge.
     * @param mapIds  A list of VLAN mappings IDs to be removed.
     * @return  A map that specifies the removed VLAN mappings.
     */
    protected final Map<String, VtnUpdateType> removeVlanMap(
        VBridgeIdentifier ident, List<String> mapIds) {
        return VTNVlanMapConfig.removeVlanMap(
            theTest.getVlanMapService(), ident, mapIds);
    }

    /**
     * Remove the port mapping in the specified virtual interface.
     *
     * @param ident  The identifier for the target virtual interface.
     * @return  A {@link VtnUpdateType} instance.
     */
    protected final VtnUpdateType removePortMap(
        VInterfaceIdentifier<?> ident) {
        return VTNPortMapConfig.removePortMap(
            theTest.getPortMapService(), ident);
    }

    /**
     * Remove the specified path map in the specified path map list.
     *
     * @param tname  The name of the VTN.
     *               {@code null} implies the global path map list.
     * @param index  The index of the path map to be removed.
     * @return  A {@link VtnUpdateType} instance that indicates the result of
     *          RPC.
     */
    protected final VtnUpdateType removePathMap(String tname, Integer index) {
        return PathMapSet.removePathMap(
            theTest.getPathMapService(), tname, index);
    }

    /**
     * Remove the specified path maps in the specified path map list.
     *
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map list.
     * @param indices  A list of path map indices to be removed.
     * @return  A map that specifies the removed path maps.
     */
    protected final Map<Integer, VtnUpdateType> removePathMap(
        String tname, List<Integer> indices) {
        return PathMapSet.removePathMap(
            theTest.getPathMapService(), tname, indices);
    }

    /**
     * Remove all the path maps in the specified path map list.
     *
     * @param tname  The name of the VTN.
     *               {@code null} implies the global path map list.
     * @return  A {@link VtnUpdateType} instance.
     */
    protected final VtnUpdateType clearPathMap(String tname) {
        return PathMapSet.clearPathMap(theTest.getPathMapService(), tname);
    }

    /**
     * Remove the specified path policy.
     *
     * @param id  The identifier for the path policy to be removed.
     */
    protected final void removePathPolicy(Integer id) {
        PathPolicySet.removePathPolicy(theTest.getPathPolicyService(), id);
    }

    /**
     * Remove the specified flow condition.
     *
     * @param name  The name of the flow condition to be removed.
     */
    protected final void removeFlowCondition(String name) {
        FlowCondSet.removeFlowCondition(
            theTest.getFlowConditionService(), name);
    }

    /**
     * Remove all the flow filters in the specified flow filter list.
     *
     * @param ident  The identifier for the virtual node.
     * @param out    A boolean value that determines the packet direction.
     * @return  A map that specifies the removed flow filters.
     */
    protected final Map<Integer, VtnUpdateType> removeFlowFilter(
        VNodeIdentifier<?> ident, boolean out) {
        return FlowFilterList.removeFlowFilter(
            theTest.getFlowFilterService(), ident, out, (List<Integer>)null);
    }

    /**
     * Remove the specified flow filters in the specified flow filter list.
     *
     * @param ident    The identifier for the virtual node.
     * @param out      A boolean value that determines the packet direction.
     * @param indices  A list of flow filter indices to be removed.
     * @return  A map that specifies the removed flow filters.
     */
    protected final Map<Integer, VtnUpdateType> removeFlowFilter(
        VNodeIdentifier<?> ident, boolean out, List<Integer> indices) {
        return FlowFilterList.removeFlowFilter(
            theTest.getFlowFilterService(), ident, out, indices);
    }

    /**
     * Remove the specified flow filter in the specified flow filter list.
     *
     * @param ident  The identifier for the virtual node.
     * @param out    A boolean value that determines the packet direction.
     * @param index  A flow filter index to be removed.
     * @return  A {@link VtnUpdateType} instance.
     */
    protected final VtnUpdateType removeFlowFilter(
        VNodeIdentifier<?> ident, boolean out, Integer index) {
        return FlowFilterList.removeFlowFilter(
            theTest.getFlowFilterService(), ident, out, index);
    }

    /**
     * Send an ICMPv4 broadcast packet.
     *
     * @param host      Source host.
     * @param dstIp     Destination IP address.
     * @param allPorts  A map that contains all switch ports as map keys.
     *                  A set of VLAN IDs mapped to the given vBridge must be
     *                  associated with the key.
     */
    protected final void sendBroadcastIcmp(TestHost host, IpNetwork dstIp,
                                           Map<String, Set<Integer>> allPorts)
        throws Exception {
        String pid = host.getPortIdentifier();
        EtherAddress src = host.getEtherAddress();
        IpNetwork srcIp = host.getInetAddress();
        int vid = host.getVlanId();
        byte type = 8;
        byte code = 0;

        EthernetFactory efc = new EthernetFactory(src, EtherAddress.BROADCAST).
            setVlanId(vid);
        Inet4Factory i4fc = Inet4Factory.newInstance(efc, srcIp, dstIp);
        Icmp4Factory ic4fc = Icmp4Factory.newInstance(i4fc, type, code);
        byte[] raw = {
            (byte)0x00, (byte)0x11, (byte)0x22, (byte)0x33,
            (byte)0x44, (byte)0x55, (byte)0x66, (byte)0x77,
            (byte)0x88, (byte)0x99, (byte)0xaa, (byte)0xbb,
            (byte)0xcc, (byte)0xdd, (byte)0xee, (byte)0xff,
            (byte)0x12,
        };
        ic4fc.setRawPayload(raw);
        byte[] payload = efc.create();

        OfMockService ofmock = theTest.getOfMockService();
        ofmock.sendPacketIn(pid, payload);

        for (Entry<String, Set<Integer>> entry: allPorts.entrySet()) {
            Set<Integer> vids = entry.getValue();
            if (vids != null) {
                String portId = entry.getKey();
                if (portId.equals(pid)) {
                    // VTN Manager will send an ARP request to probe
                    // IP address.
                    efc.setProbe(srcIp, vid);
                } else {
                    efc.setProbe(null, -1);
                }

                int count = vids.size();
                for (int i = 0; i < count; i++) {
                    byte[] transmitted =
                        ofmock.awaitTransmittedPacket(portId);
                    vids = efc.verify(ofmock, transmitted, vids);
                }
                assertTrue(vids.isEmpty());
            }
        }

        for (String p: allPorts.keySet()) {
            assertNull(ofmock.getTransmittedPacket(p));
        }
    }

    /**
     * Learn all hosts mapped to the given bridge network.
     *
     * @param bridge  A {@link BridgeNetwork} instance.
     * @throws Exception  An error occurred.
     */
    protected final void learnHosts(BridgeNetwork bridge) throws Exception {
        VBridgeIdentifier vbrId = bridge.getBridgeId();
        Map<String, List<TestHost>> hostMap = bridge.getTestHosts();
        Map<String, Set<Integer>> allPorts = bridge.getMappedVlans();

        for (List<TestHost> hosts: hostMap.values()) {
            for (TestHost host: hosts) {
                learnHost(vbrId, allPorts, host);
            }
        }
    }

    /**
     * Learn the given host mapped to the given bridge network.
     *
     * @param vbrId     The identifier for the target vBridge.
     * @param allPorts  A map that contains all switch ports as map keys.
     *                  A set of VLAN IDs mapped to the given vBridge must
     *                  be associated with the key.
     * @param host      A test host to learn.
     * @throws Exception  An error occurred.
     */
    protected final void learnHost(VBridgeIdentifier vbrId,
                                   Map<String, Set<Integer>> allPorts,
                                   TestHost host) throws Exception {
        learnHost(theTest.getOfMockService(), allPorts, host);

        MacAddress mac = host.getEtherAddress().getMacAddress();
        try (ReadOnlyTransaction rtx = theTest.newReadOnlyTransaction()) {
            InstanceIdentifier<MacTableEntry> path =
                VBridgeIdentifier.getMacEntryPath(vbrId, mac);
            Optional<MacTableEntry> opt = DataStoreUtils.read(rtx, path);
            assertEquals(true, opt.isPresent());
            assertEquals(host.getMacEntry(), new MacEntry(opt.get()));
        }
    }

    /**
     * Do the ARP unicast packet forwarding test.
     *
     * @param bridge    A {@link BridgeNetwork} instance.
     * @param islPorts  A set of ISL ports.
     * @param up        {@code true} means that all ISL ports should be up.
     *                  {@code false} means that all ISL ports should be down.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    protected final List<UnicastFlow> unicastTest(
        BridgeNetwork bridge, Set<String> islPorts, boolean up)
        throws Exception {
        ArpFlowFactory factory =
            new ArpFlowFactory(theTest.getOfMockService());
        return unicastTest(factory, bridge, islPorts, up);
    }

    /**
     * Do the ARP unicast packet forwarding test.
     *
     * @param bridge       A {@link BridgeNetwork} instance.
     * @param islPorts     A set of ISL ports.
     * @param up           {@code true} means that all ISL ports should be up.
     *                     {@code false} means that all ISL ports should be
     *                     down.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    protected final List<UnicastFlow> unicastTest(
        BridgeNetwork bridge, Set<String> islPorts, boolean up,
        VnodeState bridgeState) throws Exception {
        ArpFlowFactory factory =
            new ArpFlowFactory(theTest.getOfMockService());
        return unicastTest(factory, bridge, islPorts, up, bridgeState);
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory   A {@link UnicastFlowFactory} instance used to
     *                  create unicast flows.
     * @param bridge    A {@link BridgeNetwork} instance.
     * @param islPorts  A set of ISL ports.
     * @param up        {@code true} means that all ISL ports should be up.
     *                  {@code false} means that all ISL ports should be down.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    protected final List<UnicastFlow> unicastTest(
        UnicastFlowFactory factory, BridgeNetwork bridge, Set<String> islPorts,
        boolean up) throws Exception {
        return unicastTest(factory, bridge, islPorts, up, VnodeState.UP);
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory      A {@link UnicastFlowFactory} instance used to
     *                     create unicast flows.
     * @param bridge       A {@link BridgeNetwork} instance.
     * @param islPorts     A set of ISL ports.
     * @param up           {@code true} means that all ISL ports should be up.
     *                     {@code false} means that all ISL ports should be
     *                     down.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    protected final List<UnicastFlow> unicastTest(
        UnicastFlowFactory factory, BridgeNetwork bridge, Set<String> islPorts,
        boolean up, VnodeState bridgeState) throws Exception {
        VBridgeIdentifier vbrId = bridge.getBridgeId();
        Map<String, List<TestHost>> hostMap = bridge.getTestHosts();
        factory.addPortSet(bridge.getMappedVlans().keySet());

        OfMockService ofmock = theTest.getOfMockService();
        boolean changed = false;
        for (String pid: islPorts) {
            if (ofmock.setPortState(pid, up)) {
                changed = true;
            }
        }
        for (String pid: islPorts) {
            ofmock.awaitLink(pid, null, up);
        }

        if (changed) {
            // remove-flow operation is triggered when port state is changed.
            // So we need to wait for its completion.
            sleep(BGTASK_DELAY);
        }

        List<UnicastFlow> flows = new ArrayList<>();
        Set<NodePath> fpaths = new HashSet<>();
        for (Entry<String, List<TestHost>> srcEnt: hostMap.entrySet()) {
            String srcNid = srcEnt.getKey();
            List<TestHost> srcHosts = srcEnt.getValue();
            for (Entry<String, List<TestHost>> dstEnt: hostMap.entrySet()) {
                String dstNid = dstEnt.getKey();
                if (srcNid.equals(dstNid)) {
                    continue;
                }

                List<TestHost> dstHosts = dstEnt.getValue();
                List<OfMockLink> route =
                    ofmock.getRoute(srcNid, dstNid);
                if (up) {
                    assertNotNull(route);
                } else {
                    fpaths.add(new NodePath(srcNid, dstNid));
                    assertEquals(null, route);
                }

                flows.addAll(unicastTest(factory, vbrId, bridgeState, srcHosts,
                                         dstHosts, fpaths, route));
            }
        }

        return flows;
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory      A {@link UnicastFlowFactory} instance used to
     *                     create unicast flows.
     * @param vbrId        The identifier for the the test vBridge.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @param srcList      A list of source hosts.
     * @param dstList      A list of destination hosts.
     * @param route        A list of {@link OfMockLink} which represents the
     *                     packet route.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    protected final List<UnicastFlow> unicastTest(
        UnicastFlowFactory factory, VBridgeIdentifier vbrId,
        VnodeState bridgeState, List<TestHost> srcList, List<TestHost> dstList,
        List<OfMockLink> route) throws Exception {
        return unicastTest(factory, vbrId, bridgeState, srcList, dstList,
                           Collections.<NodePath>emptySet(), route);
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory      A {@link UnicastFlowFactory} instance used to
     *                     create unicast flows.
     * @param vbrId        The identifier for the the test vBridge.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @param srcList      A list of source hosts.
     * @param dstList      A list of destination hosts.
     * @param fpaths       A set of expected faulted paths.
     * @param route        A list of {@link OfMockLink} which represents the
     *                     packet route.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    protected final List<UnicastFlow> unicastTest(
        UnicastFlowFactory factory, VBridgeIdentifier vbrId,
        VnodeState bridgeState, List<TestHost> srcList, List<TestHost> dstList,
        Set<NodePath> fpaths, List<OfMockLink> route) throws Exception {
        List<UnicastFlow> flows = new ArrayList<>();
        boolean reachable = fpaths.isEmpty();
        VnodeState bstate;
        if (reachable) {
            bstate = bridgeState;
            assertNotNull(route);
        } else {
            bstate = VnodeState.DOWN;
            assertEquals(null, route);
        }

        OfMockService ofmock = theTest.getOfMockService();
        for (TestHost src: srcList) {
            for (TestHost dst: dstList) {
                // Run tests.
                VNodeStateWaiter waiter = new VNodeStateWaiter(ofmock).
                    set(vbrId, bstate, fpaths);
                UnicastFlow unicast = factory.create(src, dst, route);
                unicast.runTest(waiter);

                // Verify results.
                int count = unicast.getFlowCount();
                if (reachable && !factory.isAlreadyMapped()) {
                    assertTrue(count > 0);
                    flows.add(unicast);
                } else {
                    assertEquals(0, count);
                }
            }
        }

        return flows;
    }

    /**
     * Ensure that an unicast packet sent by the given host is dropped.
     *
     * @param host      The source host.
     * @param allPorts  A set of all port identifiers.
     * @return  A flow entry that drops incoming packet from the given host.
     */
    protected final OfMockFlow dropTest(TestHost host, Set<String> allPorts)
        throws Exception {
        String ingress = host.getPortIdentifier();
        EtherAddress eaddr = host.getEtherAddress();
        byte[] sha = eaddr.getBytes();
        byte[] spa = host.getRawInetAddress();
        int vid = host.getVlanId();
        EthernetFactory efc = new EthernetFactory(eaddr, MAC_DUMMY).
            setVlanId(vid);
        ArpFactory afc = ArpFactory.newInstance(efc).
            setSenderHardwareAddress(sha).
            setTargetHardwareAddress(MAC_DUMMY.getBytes()).
            setSenderProtocolAddress(spa).
            setTargetProtocolAddress(IPV4_DUMMY.getBytes());
        byte[] payload = efc.create();
        OfMockService ofmock = theTest.getOfMockService();
        ofmock.sendPacketIn(ingress, payload);

        // Ensure that a flow entry that drops the packet was installed.
        String nid = OfMockUtils.getNodeIdentifier(ingress);
        int pri = ofmock.getL2FlowPriority();
        Match match = createMatch(ingress, vid).build();
        OfMockFlow flow = ofmock.
            awaitFlow(nid, DEFAULT_TABLE, match, pri, true);
        verifyDropFlow(flow.getInstructions());

        // Ensure that no packet was forwarded.
        for (String pid: allPorts) {
            assertNull(ofmock.getTransmittedPacket(pid));
        }

        return flow;
    }

    /**
     * Ensure that all flow entries related to the given network have been
     * uninstalled.
     *
     * @param pid    The port identifier of the unmapped network.
     * @param vid    The VLAN ID of the unmapped network.
     * @param flows  A list of {@link UnicastFlow} instances.
     */
    protected final void verifyFlowUninstalled(String pid, int vid,
                                               List<UnicastFlow> flows)
        throws Exception {
        // Determine flow entries to be uninstalled.
        final int tableId = DEFAULT_TABLE;
        List<UnicastFlow> uninstalled = new ArrayList<>();
        for (Iterator<UnicastFlow> it = flows.iterator(); it.hasNext();) {
            // Check ingress flow.
            UnicastFlow unicast = it.next();
            List<OfMockFlow> flowList = unicast.getFlowList();
            OfMockFlow ingress = flowList.get(0);
            Match match = ingress.getMatch();
            int inVid = getVlanMatch(match);
            if (inVid == vid && pid.equals(getInPortMatch(match))) {
                uninstalled.add(unicast);
                it.remove();
                continue;
            }

            // Check egress flow.
            OfMockFlow egress = flowList.get(flowList.size() - 1);
            if (hasOutput(egress.getInstructions(), pid, vid, inVid)) {
                uninstalled.add(unicast);
                it.remove();
                continue;
            }
        }

        UnicastFlow.verifyFlows(uninstalled, false, true);

        // Rest of flow entries should be still installed.
        UnicastFlow.verifyFlows(flows, true, false);
    }

    /**
     * Create a new path cost configuration.
     *
     * @param pid   The MD-SAL switch port identifier.
     * @param cost  The cost of using the specified port.
     * @return  A {@link PathCost} instance.
     */
    protected final PathCost newPathCost(String pid, Long cost) {
        return newPathCost(pid, cost, PATH_COST_MODE_ID);
    }

    /**
     * Create a new path cost configuration.
     *
     * @param pid   The MD-SAL switch port identifier.
     * @param cost  The cost of using the specified port.
     * @param mode  An integer that specifies how to specify the specified
     *              switch port.
     * @return  A {@link PathCost} instance.
     */
    protected final PathCost newPathCost(String pid, Long cost, int mode) {
        String node = OfMockUtils.getNodeIdentifier(pid);
        String id = ((mode & PATH_COST_MODE_ID) != 0)
            ? OfMockUtils.getPortId(pid)
            : null;
        String name = ((mode & PATH_COST_MODE_NAME) != 0)
            ? theTest.getOfMockService().getPortName(pid)
            : null;

        return new PathCost(node, id, name, cost);
    }

    /**
     * Determine the inter-switch link between the given nodes.
     *
     * @param src  The source node identifier.
     * @param dst  The destination node identifier.
     * @return  An {@link OfMockLink} instance.
     *          {@code null} if not found.
     */
    protected final OfMockLink getInterSwitchLink(String src, String dst) {
        String prefix = dst + ID_SEPARATOR;
        OfMockService ofmock = theTest.getOfMockService();
        for (String pid: ofmock.getPorts(src, false)) {
            String peer = ofmock.getPeerIdentifier(pid);
            assertNotNull(peer);
            if (peer.startsWith(prefix)) {
                return new OfMockLink(pid, peer);
            }
        }

        return null;
    }

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    protected abstract void runTest() throws Exception;
}
