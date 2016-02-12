/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.util.NumberUtils.getUnsigned;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNInet4Match;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNTcpMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNUdpMatch;
import org.opendaylight.vtn.manager.it.util.pathmap.PathMap;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathCost;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathPolicy;
import org.opendaylight.vtn.manager.it.util.unicast.ArpFlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.Tcp4FlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.Udp4FlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlow;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Ensure that packets are routed according to the path map configuration.
 */
public final class PathMapTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public PathMapTest(VTNManagerIT vit) {
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
        VtnPathPolicyService ppSrv = vit.getPathPolicyService();
        VtnPathMapService pmSrv = vit.getPathMapService();
        VtnFlowConditionService fcSrv = vit.getFlowConditionService();

        // Collect existing switch ports.
        Map<String, Set<Integer>> allPorts = new HashMap<>();
        for (String nid: ofmock.getNodes()) {
            for (boolean b: new boolean[]{true, false}) {
                for (String pid: ofmock.getPorts(nid, b)) {
                    assertEquals(null, allPorts.put(pid, null));
                }
            }
        }

        // Create 4 nodes.
        final int numNodes = 4;
        String[] nodeIds = new String[numNodes];
        for (int i = 0; i < numNodes; i++) {
            BigInteger dpid = getUnsigned((long)(i + 1));
            String nid = ofmock.addNode(dpid, false);
            nodeIds[i] = nid;

            for (int j = 1; j <= numNodes; j++) {
                String pid = ofmock.addPort(nid, (long)j, false);
                assertEquals(null, allPorts.put(pid, null));
                ofmock.setPortState(pid, true);
            }
        }

        // Construct full-mesh network.
        Map<String, String> allLinks = new HashMap<>();
        for (int i = 0; i < numNodes; i++) {
            String srcNid = nodeIds[i];
            Iterator<String> srcIt = ofmock.getPorts(srcNid, true).
                iterator();
            for (int j = i + 1; j < numNodes; j++) {
                String dstNid = nodeIds[j];
                String src = srcIt.next();
                String dst = ofmock.getPorts(dstNid, true).get(0);
                assertEquals(null, allLinks.put(src, dst));
                assertEquals(null, allLinks.put(dst, src));
                ofmock.setPeerIdentifier(src, dst, false);
                ofmock.setPeerIdentifier(dst, src, false);
            }
        }
        assertEquals(numNodes * (numNodes - 1), allLinks.size());

        // Wait for network topology to be established.
        for (Map.Entry<String, String> entry: allLinks.entrySet()) {
            String src = entry.getKey();
            String dst = entry.getValue();
            ofmock.awaitLink(src, dst, true);
        }

        // Create a VTN and a vBridge for test.
        String tname = "vtn";
        VBridgeIdentifier bpath = new VBridgeIdentifier(tname, "vbr");
        VBridgeIfIdentifier ipath0 = bpath.childInterface("if0");
        VBridgeIfIdentifier ipath2 = bpath.childInterface("if2");
        VirtualNetwork vnet = getVirtualNetwork().
            addInterface(ipath0, ipath2);

        // Map an edge port on nodeIds[0] to ipath0.
        int vid0 = 0;
        String edge0 = ofmock.getPorts(nodeIds[0], true).get(0);
        VInterfaceConfig iconf0 = vnet.getInterface(ipath0);
        VTNPortMapConfig pmap0 = new VTNPortMapConfig(edge0, vid0).
            setMappedPort(edge0);
        iconf0.setPortMap(pmap0);
        allPorts.put(edge0, Collections.singleton(vid0));

        // Map an edge port on nodeIds[2] to ipath2.
        int vid2 = 2;
        String edge2 = ofmock.getPorts(nodeIds[2], true).get(0);
        VInterfaceConfig iconf2 = vnet.getInterface(ipath2);
        VTNPortMapConfig pmap2 = new VTNPortMapConfig(edge2, vid2).
            setMappedPort(edge2);
        iconf2.setPortMap(pmap2);
        allPorts.put(edge2, Collections.singleton(vid2));

        // Apply configuration, and wait for the test vBridge to change its
        // state to UP.
        vnet.apply();
        VNodeStateWaiter waiter = new VNodeStateWaiter(ofmock).
            set(bpath, VnodeState.UP).
            await();
        vnet.verify();

        // Create 2 hosts connected to edge0.
        int hostIdx = 1;
        final int numHosts = 2;
        List<TestHost> hosts0 = new ArrayList<>();
        String pname0 = ofmock.getPortName(edge0);
        for (int i = 0; i < numHosts; i++) {
            hosts0.add(new TestHost(hostIdx, edge0, pname0, vid0));
            hostIdx++;
        }

        // Create 2 hosts connected to edge2.
        List<TestHost> hosts2 = new ArrayList<>();
        String pname2 = ofmock.getPortName(edge2);
        for (int i = 0; i < numHosts; i++) {
            hosts2.add(new TestHost(hostIdx, edge2, pname2, vid2));
            hostIdx++;
        }

        // Determine shortest path from nodeIds[0] to nodeIds[2].
        OfMockLink link = getInterSwitchLink(nodeIds[0], nodeIds[2]);
        assertNotNull(link);
        List<OfMockLink> route0 = Collections.singletonList(link);

        // Configure path policy 1.
        //   Route from nodeIds[0] to nodeIds[2]:
        //     0 -> 1 -> 2
        List<OfMockLink> route1 = new ArrayList<>();
        link = getInterSwitchLink(nodeIds[0], nodeIds[1]);
        assertNotNull(link);
        route1.add(link);
        link = getInterSwitchLink(nodeIds[1], nodeIds[2]);
        assertNotNull(link);
        route1.add(link);

        long defCost1 = 10000L;
        PathPolicy policy1 = new PathPolicy(1, defCost1);
        int pcMode = PATH_COST_MODE_NAME;
        for (OfMockLink l: route1) {
            policy1.add(newPathCost(l.getSourcePort(), null, pcMode));
        }

        assertEquals(VtnUpdateType.CREATED,
                     policy1.update(ppSrv, VtnUpdateOperationType.ADD, false));
        vnet.getPathPolicies().add(policy1);
        vnet.verify();

        // Configure path policy 2.
        //   Route from nodeIds[0] to nodeIds[2]:
        //     0 -> 3 -> 2
        List<OfMockLink> route2 = new ArrayList<>();
        link = getInterSwitchLink(nodeIds[0], nodeIds[3]);
        assertNotNull(link);
        route2.add(link);
        link = getInterSwitchLink(nodeIds[3], nodeIds[2]);
        assertNotNull(link);
        route2.add(link);

        long defCost2 = 20000L;
        PathPolicy policy2 = new PathPolicy(2, defCost2);
        assertEquals(VtnUpdateType.CREATED,
                     policy2.update(ppSrv, VtnUpdateOperationType.SET, null));
        vnet.getPathPolicies().add(policy2);
        vnet.verify();

        Map<VtnPortDesc, VtnUpdateType> pdResult = new HashMap<>();
        List<PathCost> costs = new ArrayList<>();
        for (OfMockLink l: route2) {
            PathCost pc = newPathCost(l.getSourcePort(), 2L);
            costs.add(pc);
            VtnPortDesc pdesc = pc.getPortDesc();
            assertEquals(null, pdResult.put(pdesc, VtnUpdateType.CREATED));
        }

        assertEquals(pdResult, policy2.addCosts(ppSrv, costs));
        vnet.verify();

        // Increase cost for the links from nodeIds[0] to nodeIds[1]/nodeIds[2]
        // for later test.
        pdResult.clear();
        costs.clear();
        pcMode = PATH_COST_MODE_NAME | PATH_COST_MODE_ID;
        for (String dst: new String[]{nodeIds[1], nodeIds[2]}) {
            link = getInterSwitchLink(nodeIds[0], dst);
            assertNotNull(link);
            long cost = 100000000000L;
            PathCost pc = newPathCost(link.getSourcePort(), cost, pcMode);
            costs.add(pc);
            VtnPortDesc pdesc = pc.getPortDesc();
            assertEquals(null, pdResult.put(pdesc, VtnUpdateType.CREATED));
        }
        assertEquals(pdResult, policy2.addCosts(ppSrv, costs));
        vnet.verify();

        // Configure path policy 3.
        //   Route from nodeIds[0] to nodeIds[2]:
        //     0 -> 1 -> 3 -> 2
        List<OfMockLink> route3 = new ArrayList<>();
        link = getInterSwitchLink(nodeIds[0], nodeIds[1]);
        assertNotNull(link);
        route3.add(link);
        link = getInterSwitchLink(nodeIds[1], nodeIds[3]);
        assertNotNull(link);
        route3.add(link);
        link = getInterSwitchLink(nodeIds[3], nodeIds[2]);
        assertNotNull(link);
        route3.add(link);

        long defCost3 = 30000L;
        PathPolicy policy3 = new PathPolicy(3, defCost3);
        assertEquals(VtnUpdateType.CREATED,
                     policy3.update(ppSrv, null, false));
        vnet.getPathPolicies().add(policy3);
        vnet.verify();

        PathCost pc3 = null;
        long cost3 = 10L;
        for (OfMockLink l: route3) {
            String pid = l.getSourcePort();
            PathCost pc = newPathCost(pid, cost3);
            if (OfMockUtils.getNodeIdentifier(pid).equals(nodeIds[3])) {
                assertEquals(null, pc3);
                pc3 = pc;
            }
            policy3.add(pc);
        }
        assertNotNull(pc3);
        assertEquals(VtnUpdateType.CHANGED,
                     policy3.update(ppSrv, VtnUpdateOperationType.SET, false));

        // Increase cost for the link from nodeIds[0] to nodeIds[2]/nodeIds[3]
        // for later test.
        pdResult.clear();
        costs.clear();
        for (String dst: new String[]{nodeIds[2], nodeIds[3]}) {
            link = getInterSwitchLink(nodeIds[0], dst);
            assertNotNull(link);
            long cost = defCost3 * 2L;
            PathCost pc = newPathCost(link.getSourcePort(), cost);
            costs.add(pc);
            VtnPortDesc pdesc = pc.getPortDesc();
            assertEquals(null, pdResult.put(pdesc, VtnUpdateType.CREATED));
        }
        assertEquals(pdResult, policy3.addCosts(ppSrv, costs));
        vnet.verify();

        // Configure flow conditions.

        // udp_src_50000: Match UDP packet from port 50000.
        String cnameUdp = "udp_src_50000";
        FlowCondition condUdp = new FlowCondition(cnameUdp);
        vnet.getFlowConditions().add(condUdp);
        assertEquals(VtnUpdateType.CREATED, condUdp.update(fcSrv, null, null));
        assertEquals(null, condUdp.update(fcSrv, null, null));
        vnet.verify();

        final int udpSrcPort = 50000;
        VTNUdpMatch udm = new VTNUdpMatch(udpSrcPort, null);
        FlowMatch fm = new FlowMatch(1, null, null, udm);
        assertEquals(VtnUpdateType.CREATED, condUdp.addMatch(fcSrv, fm));
        assertEquals(null, condUdp.addMatch(fcSrv, fm));
        vnet.verify();

        // tcp_dst_23: Match TCP packet destinated to port 23.
        String cnameTcp = "tcp_dst_23";
        FlowCondition condTcp = new FlowCondition(cnameTcp);
        vnet.getFlowConditions().add(condTcp);
        assertEquals(VtnUpdateType.CREATED,
                     condTcp.update(fcSrv, VtnUpdateOperationType.ADD, null));
        vnet.verify();

        final int tcpDstPort = 23;
        VTNTcpMatch tcm = new VTNTcpMatch(null, tcpDstPort);
        fm = new FlowMatch(2, null, null, tcm);
        assertEquals(VtnUpdateType.CREATED, condTcp.addMatch(fcSrv, fm));
        assertEquals(null, condTcp.addMatch(fcSrv, fm));
        vnet.verify();

        // dscp_10: Match IPv4 packet assigned 10 as DSCP.
        final short dscpValue = 10;
        String cnameDscp = "dscp_10";
        VTNInet4Match im = new VTNInet4Match().setDscp(dscpValue);
        FlowCondition condDscp = new FlowCondition(cnameDscp).
            add(new FlowMatch(3, null, im, null));
        vnet.getFlowConditions().add(condDscp);
        assertEquals(VtnUpdateType.CREATED,
                     condDscp.update(fcSrv, VtnUpdateOperationType.SET, false));
        vnet.verify();

        // Configure path maps.

        // Map udp_src_50000 packets to path policy 1 using VTN-local path map.
        PathMap vpmap1 = new PathMap(1, cnameUdp, 1);
        assertEquals(VtnUpdateType.CREATED, vpmap1.update(pmSrv, tname));
        assertEquals(null, vpmap1.update(pmSrv, tname));
        VTenantConfig tconf = vnet.getTenant(tname);
        tconf.getPathMaps().add(vpmap1);
        vnet.verify();

        // Map dscp_10 packets to path policy 2 using VTN-local path map.
        int idle2 = 10000;
        int hard2 = 30000;
        PathMap vpmap2 = new PathMap(2, cnameDscp, 2, idle2, hard2);
        assertEquals(VtnUpdateType.CREATED, vpmap2.update(pmSrv, tname));
        assertEquals(null, vpmap2.update(pmSrv, tname));
        tconf.getPathMaps().add(vpmap2);
        vnet.verify();

        // Map tcp_dst_23 packets to path policy 3 using global path map.
        int idle3 = 11111;
        int hard3 = 32000;
        PathMap gpmap = new PathMap(1, cnameTcp, 3, idle3, hard3);
        assertEquals(VtnUpdateType.CREATED, gpmap.update(pmSrv));
        assertEquals(null, gpmap.update(pmSrv));
        vnet.getPathMaps().add(gpmap);
        vnet.verify();

        // Ensure that network topology is established.
        for (Map.Entry<String, String> entry: allLinks.entrySet()) {
            String src = entry.getKey();
            String dst = entry.getValue();
            ofmock.awaitLink(src, dst, true);
        }

        // Let the test vBridge learn MAC addresses.
        for (TestHost host: hosts0) {
            learnHost(bpath, allPorts, host);
        }
        for (TestHost host: hosts2) {
            learnHost(bpath, allPorts, host);
        }

        // Ensure no flow entry is installed.
        FlowCounter counter = new FlowCounter(ofmock).verify();

        // Send ARP unicast packets that should be mapped by the default
        // route resolver. Ethernet type will be configured in flow match.
        ArpFlowFactory arpfc = new ArpFlowFactory(ofmock, vit);
        arpfc.addMatchType(FlowMatchType.ETH_TYPE);
        List<UnicastFlow> flows00 =
            unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2,
                        route0);

        // Send UDP packet that should be mapped by the path policy 1.
        // Ethernet type, IPv4 protocol, and UDP source port will be
        // configured in flow match. Although UDP packets will have DSCP
        // value 10, DSCP should not be configured in flow match because
        // udp_src_50000 will be evaluated before dscp_10.
        Udp4FlowFactory udpfc =
            new Udp4FlowFactory(ofmock, vit, udpSrcPort, (short)53);
        udpfc.setDscp(dscpValue).
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.UDP_SRC);
        List<UnicastFlow> flows1 =
            unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2,
                        route1);

        // Send TCP packet that should be mapped by the path policy 2.
        // Ethernet type, IPv4 protocol, and IP DSCP will be configured in
        // flow match. Although the TCP destination port is 23, packets
        // should be mapped by dscp_10 because it will be evaluated before
        // tcp_dst_23.
        Tcp4FlowFactory tcpfc1 =
            new Tcp4FlowFactory(ofmock, vit, (short)12345, tcpDstPort);
        tcpfc1.setDscp(dscpValue).
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.IP_DSCP).
            setIdleTimeout(idle2).setHardTimeout(hard2);
        List<UnicastFlow> flows2 =
            unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                        route2);

        // Send TCP packet that should be mapped by the path policy 3.
        // Ethernet type, IPv4 protocol, IP DSCP, and TCP destination port
        // will be configured in flow match.
        Tcp4FlowFactory tcpfc2 =
            new Tcp4FlowFactory(ofmock, vit, (short)12345, tcpDstPort);
        tcpfc2.addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.IP_DSCP).
            addMatchType(FlowMatchType.TCP_DST).
            setIdleTimeout(idle3).setHardTimeout(hard3);
        List<UnicastFlow> flows3 =
            unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2,
                        route3);

        // Send TCP packet that should not be matched by any flow condition.
        Tcp4FlowFactory tcpfc3 =
            new Tcp4FlowFactory(ofmock, vit, (short)333, (short)444);
        tcpfc3.addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.IP_DSCP).
            addMatchType(FlowMatchType.TCP_DST);
        List<UnicastFlow> flows01 =
            unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2,
                        route0);

        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Remove path policy 3.
        // This should remove unicast flows routed by the path policy 3.
        Integer ppId3 = policy3.getId();
        removePathPolicy(ppId3);
        vnet.getPathPolicies().remove(ppId3);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, false, true);

        // Restore path policy 3.
        // This should remove all flow entries.
        assertEquals(VtnUpdateType.CREATED, policy3.update(ppSrv, null, null));
        vnet.getPathPolicies().add(policy3);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, false, true);

        // Ensure that no flow entry is installed.
        counter.clear().verify();

        // Restore unicast flows in reverse order.
        flows01 = unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2,
                              route0);
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2,
                             route3);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                             route2);
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2,
                             route1);
        flows00 = unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2,
                              route0);

        // Set 1 to the default cost for the path policy 1.
        // This should remove unicast flows routed by the path policy 1.
        defCost1 = 1L;
        PathPolicy policy = new PathPolicy(1, defCost1);
        assertEquals(VtnUpdateType.CHANGED, policy.update(ppSrv, null, true));
        assertEquals(null, policy.update(ppSrv, null, true));
        policy1.setDefaultCost(defCost1);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Now the path policy 1 will choose the shortest path for packets
        // from nodeIds[0] to nodeIds[2].
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2,
                             route0);

        // Increase the cost for transmitting packet from nodeIds[3] to
        // nodeIds[2] in the path policy 2.
        // This should remove unicast flows routed by the path policy 2.
        link = getInterSwitchLink(nodeIds[3], nodeIds[2]);
        assertNotNull(link);
        long cost2 = 200000000L;
        pcMode = PATH_COST_MODE_NAME | PATH_COST_MODE_ID;
        PathCost pc2 = newPathCost(link.getSourcePort(), cost2, pcMode);
        assertEquals(VtnUpdateType.CREATED, policy2.addCost(ppSrv, pc2));
        assertEquals(null, policy2.addCost(ppSrv, pc2));
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Now path policy 2 should route packet from nodeIds[0] to nodeIds[2]
        // as follows:
        //    0 -> 3 -> 1 -> 2
        route2.clear();
        link = getInterSwitchLink(nodeIds[0], nodeIds[3]);
        assertNotNull(link);
        route2.add(link);
        link = getInterSwitchLink(nodeIds[3], nodeIds[1]);
        assertNotNull(link);
        route2.add(link);
        link = getInterSwitchLink(nodeIds[1], nodeIds[2]);
        assertNotNull(link);
        route2.add(link);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                             route2);
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Remove the cost for transmitting packet from nodeIds[3] to
        // nodeIds[2] in the path policy 3.
        // This should remove unicast flows routed by the path policy 3.
        assertEquals(VtnUpdateType.REMOVED, policy3.removeCost(ppSrv, pc3));
        assertEquals(null, policy3.removeCost(ppSrv, pc3));
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, false, true);

        // Now path policy 3 should route packet from nodeIds[0] to nodeIds[2]
        // as follows:
        //    0 -> 1 -> 2
        route3.clear();
        link = getInterSwitchLink(nodeIds[0], nodeIds[1]);
        assertNotNull(link);
        route3.add(link);
        link = getInterSwitchLink(nodeIds[1], nodeIds[2]);
        assertNotNull(link);
        route3.add(link);
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2,
                             route3);

        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);
        counter.clear().add(flows00).add(flows01).add(flows1).
            add(flows2).add(flows3).verify();

        // Remove path policy 2.
        // This will remove unicast flows routed by the path policy 2.
        Integer ppId2 = policy2.getId();
        removePathPolicy(ppId2);
        vnet.getPathPolicies().remove(ppId2);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, true, false);
        counter.clear().add(flows00).add(flows01).add(flows1).add(flows3).
            verify();

        // The VTN-local path map 2 should be ignored because the path policy
        // 2 is not present. So flows2 should be mapped by the global path 1.
        tcpfc1.clearMatchType().
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.TCP_DST).
            setIdleTimeout(idle3).setHardTimeout(hard3);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                             route3);
        counter.clear().add(flows00).add(flows01).add(flows1).add(flows2).
            add(flows3).verify();

        // Remove flow condition tcp_dst_23 used by the global path map 1.
        // This will remove all flow entries.
        removeFlowCondition(cnameTcp);
        vnet.getFlowConditions().remove(cnameTcp);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, false, true);
        counter.clear().verify();

        // Configure flow timeout for the VTN-local path map 1.
        int idle1 = 12345;
        int hard1 = 23456;
        vpmap1.setIdleTimeout(idle1).setHardTimeout(hard1);
        assertEquals(VtnUpdateType.CHANGED, vpmap1.update(pmSrv, tname));
        assertEquals(null, vpmap1.update(pmSrv, tname));
        vnet.verify();
        udpfc.setIdleTimeout(idle1).setHardTimeout(hard1);

        // The global path map 2 should be ignored because the flow condition
        // tcp_dst_23 is not present.
        tcpfc2.clearMatchType().
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            resetTimeout();
        tcpfc3.clearMatchType().
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO);
        flows00 = unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2,
                              route0);
        flows01 = unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2,
                              route0);
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2,
                             route0);

        // Unicast flows that will be created by tcpfc1 and tcpfc2 should be
        // forwarded by flow entries craeted by tcpfc3.
        tcpfc1.setAlreadyMapped(true);
        tcpfc2.setAlreadyMapped(true);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                             route0);
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2,
                             route0);
        counter.clear().add(flows00).add(flows01).add(flows1).verify();

        // Remove global path map.
        // This will remove all flow entries.
        Integer gpmIdx = gpmap.getIndex();
        assertEquals(VtnUpdateType.REMOVED, removePathMap(null, gpmIdx));
        assertEquals(null, removePathMap(null, gpmIdx));
        vnet.getPathMaps().remove(gpmIdx);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        counter.clear().verify();

        // Clean up.
        assertEquals(VtnUpdateType.REMOVED,
                     getRpcResult(fcSrv.clearFlowCondition()));
        assertEquals(null, getRpcResult(fcSrv.clearFlowCondition()));
        vnet.getFlowConditions().clear();
        vnet.verify();

        assertEquals(VtnUpdateType.REMOVED,
                     getRpcResult(ppSrv.clearPathPolicy()));
        assertEquals(null, getRpcResult(ppSrv.clearPathPolicy()));
        vnet.getPathPolicies().clear();
        vnet.verify();

        assertEquals(VtnUpdateType.REMOVED, clearPathMap(tname));
        assertEquals(null, clearPathMap(tname));
        tconf.getPathMaps().clear();
        vnet.verify();

        removeVtn(tname);
        vnet.removeTenant(tname).verify();
    }
}
