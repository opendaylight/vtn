/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.ActionList;
import org.opendaylight.vtn.manager.internal.FlowModTaskTestBase;
import org.opendaylight.vtn.manager.internal.L2Host;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

/**
 * JUnit Test for {@link VTNFlow}
 */
public class VTNFlowTest extends FlowModTaskTestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = new short[] {-1, 0, 1, 4095};
        int[] priorities = new int[] {1, 10, 0xefff};
        for (String tname : createStrings("name")) {
            FlowGroupId gid = new FlowGroupId(tname);
            VTNFlow vflow = new VTNFlow(gid);
            assertEquals(gid, vflow.getGroupId());

            List<FlowEntry> entries = new ArrayList<FlowEntry>();
            Set<Node> flowNodes = new HashSet<Node>();
            Set<NodeConnector> flowPorts = new HashSet<NodeConnector>();
            assertEquals(entries, vflow.getFlowEntries());
            assertEquals(flowNodes, vflow.getFlowNodes());
            assertEquals(flowPorts, vflow.getFlowPorts());

            for (NodeConnector port : createNodeConnectors(3, false)) {
                for (MatchType mtype : MatchType.values()) {
                    for (short vlan : vlans) {
                        for (int pri : priorities) {
                            String emsg = "(tenant name)" + tname
                                    + ",(port)" + port.toString()
                                    + ",(vlan)" + vlan
                                    + ",(priority)" + pri;
                            Match match = new Match();
                            match.setField(MatchType.IN_PORT, port);

                            if (mtype == MatchType.DL_OUTER_VLAN ||
                                    mtype == MatchType.DL_VLAN) {
                                match = setMatchField(match, mtype, vlan);
                            } else {
                                match = setMatchField(match, mtype);
                            }

                            ActionList actions = new ActionList(port.getNode());
                            actions.addOutput(port);

                            if (vlan >= 0) {
                                actions.addVlanId(vlan);
                            }

                            vflow.addFlow(vtnMgr, match, actions, pri);

                            String gname = vflow.getGroupId().toString();
                            int index = entries.size();
                            String fname = gname + ClusterEventId.SEPARATOR + index;
                            Flow flow = new Flow(match, actions.get());
                            flow.setPriority((short) pri);
                            FlowEntry fent = new FlowEntry(gname, fname, flow,
                                                           port.getNode());
                            entries.add(fent);
                            flowNodes.add(port.getNode());
                            flowPorts.add(port);

                            // check VTNFlow.
                            // entries.
                            assertTrue(emsg,
                                       vflow.getFlowEntries().contains(fent));
                            assertEquals(emsg, flowNodes, vflow.getFlowNodes());
                            assertEquals(emsg, flowPorts, vflow.getFlowPorts());
                        }
                    }
                }
            }

            for (Integer idle : createIntegers(-1, 3, false)) {
                for (Integer hard : createIntegers(-1, 3, false)) {
                    String emsg = "(tenant name)" + tname
                            + ",(idle)" + idle + ",(hard)" + hard;
                    vflow.setTimeout(idle.intValue(), hard.intValue());

                    Flow flow = vflow.getFlowEntries().get(0).getFlow();
                    assertEquals(emsg, idle.shortValue(), flow.getIdleTimeout());
                    assertEquals(emsg, hard.shortValue(), flow.getHardTimeout());
                }
            }
        }

        // in case actions == null.
        FlowGroupId gid = new FlowGroupId("test");
        VTNFlow vflow = new VTNFlow(gid);
        Match match = new Match();
        Node node0 = NodeCreator.createOFNode(Long.valueOf("0"));
        NodeConnector port
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node0);
        match.setField(MatchType.IN_PORT, port);
        Flow flow = new Flow(match, null);
        vflow.addFlow(vtnMgr, flow, node0);

        assertEquals(vflow.getGroupId(), vflow.getGroupId());
        assertEquals(flow, vflow.getFlowEntries().get(0).getFlow());
        assertEquals(1, vflow.getFlowNodes().size());
        assertEquals(1, vflow.getFlowPorts().size());
        assertEquals(node0, vflow.getFlowNodes().iterator().next());
        assertEquals(port, vflow.getFlowPorts().iterator().next());

        // in case port is special port.
        gid = new FlowGroupId("test");
        vflow = new VTNFlow(gid);
        match = new Match();
        NodeConnector specialNc = null;
        try {
            specialNc = new NodeConnector(
                    NodeConnector.NodeConnectorIDType.CONTROLLER,
                    NodeConnector.SPECIALNODECONNECTORID, node0);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        match.setField(MatchType.IN_PORT, port);
        ActionList actions = new ActionList(specialNc.getNode());
        actions.addOutput(specialNc);
        flow = new Flow(match, actions.get());

        vflow.addFlow(vtnMgr, match, actions, 0);
        assertEquals(vflow.getGroupId(), vflow.getGroupId());
        assertEquals(flow, vflow.getFlowEntries().get(0).getFlow());
        assertEquals(1, vflow.getFlowNodes().size());
        assertEquals(1, vflow.getFlowPorts().size());
        assertEquals(node0, vflow.getFlowNodes().iterator().next());

        // invoke setTimeout() for VTNFlow which have no flow entries.
        vflow = new VTNFlow(gid);
        vflow.setTimeout(100, 100);
        assertEquals(0, vflow.getFlowEntries().size());
    }

    /**
     * Test case for
     * {@link VTNFlow#addDependency(Set)},
     * {@link VTNFlow#dependsOn(VTenantPath)}.
     */
    @Test
    public void testDependencyTenantPath() {
        VTenantPath tpathNotMatch = new VTenantPath("not_match_tenant");

        VTenantPath stpath = new VTenantPath("tenant_src");
        VBridgePath sbpath = new VBridgePath(stpath, "bridge");
        VBridgeIfPath srcpath = new VBridgeIfPath(sbpath, "id");
        Set<VTenantPath> srcPaths = new HashSet<VTenantPath>();
        assertTrue(srcPaths.add(stpath));
        assertTrue(srcPaths.add(sbpath));
        assertTrue(srcPaths.add(srcpath));

        VTenantPath dtpath = new VTenantPath("tenant_dst");
        VBridgePath dbpath = new VBridgePath(dtpath, "bridge");
        VlanMapPath dstpath = new VlanMapPath(dbpath, "id");
        Set<VTenantPath> dstPaths = new HashSet<VTenantPath>();
        assertTrue(dstPaths.add(dtpath));
        assertTrue(dstPaths.add(dbpath));
        assertTrue(dstPaths.add(dstpath));

        Set<VTenantPath> srcDstPaths = new HashSet<VTenantPath>(srcPaths);
        srcDstPaths.addAll(dstPaths);

        Set<VTenantPath> independPaths = new HashSet<VTenantPath>();
        independPaths.add(tpathNotMatch);
        independPaths.add(new VBridgePath(stpath.getTenantName(), "vbr10"));
        independPaths.add(new VBridgeIfPath(sbpath, "if_10"));
        independPaths.add(new VlanMapPath(sbpath, "id"));
        independPaths.add(new VBridgePath(dtpath.getTenantName(), "vbr10"));
        independPaths.add(new VBridgeIfPath(dbpath, "id"));
        independPaths.add(new VlanMapPath(dbpath, "ANY.0"));

        FlowGroupId gid = new FlowGroupId("test");
        VTNFlow vflow = new VTNFlow(gid);
        for (VTenantPath path: srcDstPaths) {
            assertFalse(vflow.dependsOn(path));
        }

        // Set node path associated with the ingress flow.
        assertNull(vflow.getIngressPath());
        vflow.setIngressPath(srcpath);
        assertEquals(srcpath, vflow.getIngressPath());
        for (VTenantPath path: srcPaths) {
            assertTrue(vflow.dependsOn(path));
        }
        for (VTenantPath path: dstPaths) {
            assertFalse(vflow.dependsOn(path));
        }

        // Set node path associated with the egress flow.
        assertNull(vflow.getEgressPath());
        vflow.setEgressPath(dstpath);
        assertEquals(dstpath, vflow.getEgressPath());
        for (VTenantPath path: srcDstPaths) {
            assertTrue(vflow.dependsOn(path));
        }
        for (VTenantPath path: independPaths) {
            assertFalse(vflow.dependsOn(path));
        }

        Set<VTenantPath> pathSet = new HashSet<VTenantPath>(srcDstPaths);
        for (String tname : createStrings("vtn", false)) {
            VBridgePath bpath1 = new VBridgePath(tname, "bridge_1");
            VBridgePath bpath2 = new VBridgePath(tname, "bridge_2");
            Set<VTenantPath> set = new HashSet<VTenantPath>();
            assertTrue(set.add(bpath1));
            assertTrue(set.add(bpath2));
            pathSet.add(bpath1);
            pathSet.add(bpath2);
            vflow.addDependency(set);

            for (VTenantPath path: pathSet) {
                assertTrue(vflow.dependsOn(path));
            }

            independPaths.add(new VTenantPath(tname));
            independPaths.add(new VBridgePath(tname, "bridge_3"));
            for (VTenantPath path: set) {
                VBridgePath bp = (VBridgePath)path;
                independPaths.add(new VBridgeIfPath(bp, "id"));
                independPaths.add(new VlanMapPath(bp, "id"));
            }
        }

        for (VTenantPath path: independPaths) {
            assertFalse(vflow.dependsOn(path));
        }
        assertEquals(gid, vflow.getGroupId());
    }

    /**
     * Test case for {@link VTNFlow#dependsOn(MacVlan)} and
     * {@link VTNFlow#getEdgeHosts()}.
     */
    @Test
    public void testDependencyMacVlan() {
        FlowGroupId gid = new FlowGroupId("test");
        VTNFlow vflow = new VTNFlow(gid);
        byte[] addrEE = {
            (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x00, (byte)0x00, (byte)0xee,
        };
        byte[] addrFF = {
            (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x00, (byte)0x00, (byte)0xff,
        };
        MacVlan hostEE = new MacVlan(addrEE, (short)9);
        MacVlan hostFF = new MacVlan(addrFF, (short)9);
        assertFalse(vflow.dependsOn(hostEE));
        assertFalse(vflow.dependsOn(hostFF));
        assertNull(vflow.getEdgeHosts());

        byte[][] srcAddrs = {
            new byte[]{
                (byte)0x00, (byte)0x11, (byte)0x22,
                (byte)0x33, (byte)0x44, (byte)0x55,
            },
            new byte[]{
                (byte)0xf0, (byte)0xfa, (byte)0xfb,
                (byte)0xfc, (byte)0xfc, (byte)0xfe,
            },
        };
        byte[][] dstAddrs = {
            new byte[]{
                (byte)0x00, (byte)0xaa, (byte)0xbb,
                (byte)0xcc, (byte)0xdd, (byte)0xee,
            },
            new byte[]{
                (byte)0xa0, (byte)0xa1, (byte)0xa2,
                (byte)0xa3, (byte)0xa4, (byte)0xa5,
            },
        };
        short[] srcVlans = {0, 10, 4095};
        short[] dstVlans = {0, 100, 200};
        short ivlan = 999;

        Node node = NodeCreator.createOFNode(Long.valueOf(0));
        NodeConnector sport = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)1), node);
        NodeConnector dport = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)2), node);
        NodeConnector iport = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)3), node);

        MacVlan imvlan = new MacVlan(addrFF, ivlan);
        L2Host ihost = new L2Host(addrFF, ivlan, iport);

        Set<MacVlan> mvSet = new HashSet<MacVlan>();
        for (byte[] src: srcAddrs) {
            for (short svlan: srcVlans) {
                MacVlan srcHost = new MacVlan(src, svlan);
                L2Host sh = new L2Host(src, svlan, sport);
                Match match = new Match();
                match.setField(MatchType.DL_SRC, src);
                match.setField(MatchType.DL_VLAN, svlan);
                match.setField(MatchType.IN_PORT, sport);

                for (byte[] dst: dstAddrs) {
                    match.setField(MatchType.DL_DST, dst);

                    for (short dvlan: dstVlans) {
                        boolean sameVlan = (svlan == dvlan);
                        MacVlan dstHost = new MacVlan(dst, dvlan);
                        L2Host dh = new L2Host(dst, dvlan, dport);
                        MacVlan[] hosts = {
                            new MacVlan(src, dvlan),
                            new MacVlan(dst, svlan),
                        };

                        // Test for VTN flow that has no action.
                        vflow = new VTNFlow(gid);
                        Flow f = new Flow(match, null);
                        vflow.addFlow(vtnMgr, f, node);

                        ObjectPair<L2Host, L2Host> edges =
                            vflow.getEdgeHosts();
                        assertEquals(sh, edges.getLeft());
                        assertNull(edges.getRight());

                        assertTrue(vflow.dependsOn(srcHost));
                        assertFalse(vflow.dependsOn(dstHost));
                        assertFalse(vflow.dependsOn(hostEE));
                        assertFalse(vflow.dependsOn(hostFF));
                        assertFalse(vflow.dependsOn(imvlan));

                        // Test for VTN flow that has only one flow.
                        vflow = new VTNFlow(gid);
                        ActionList actions = new ActionList(node);
                        actions.addOutput(dport).addVlanId(dvlan);
                        vflow.addFlow(vtnMgr, match, actions, 10);

                        edges = vflow.getEdgeHosts();
                        assertEquals(sh, edges.getLeft());
                        assertEquals(dh, edges.getRight());

                        assertTrue(vflow.dependsOn(srcHost));
                        assertTrue(vflow.dependsOn(dstHost));
                        assertFalse(vflow.dependsOn(hostEE));
                        assertFalse(vflow.dependsOn(hostFF));
                        assertFalse(vflow.dependsOn(imvlan));
                        for (MacVlan mv: hosts) {
                            assertEquals(sameVlan, vflow.dependsOn(mv));
                        }

                        for (int nflows = 0; nflows < 5; nflows++) {
                            // Add intermediate flow.
                            for (int i = 0; i < nflows; i++) {
                                actions = new ActionList(node);
                                actions.addOutput(iport).addVlanId(ivlan);
                                List<Action> alist = actions.get();
                                alist.add(new SetDlSrc(addrEE));
                                alist.add(new SetDlDst(addrFF));
                                f = new Flow(match, alist);
                                vflow.addFlow(vtnMgr, f, node);
                            }

                            L2Host ih;
                            NodeConnector inc;
                            boolean hasIflow;
                            if (nflows == 0) {
                                ih = dh;
                                hasIflow = false;
                            } else {
                                ih = ihost;
                                hasIflow = true;
                            }
                            edges = vflow.getEdgeHosts();
                            assertEquals(sh, edges.getLeft());
                            assertEquals(ih, edges.getRight());

                            assertTrue(vflow.dependsOn(srcHost));
                            assertEquals(!hasIflow, vflow.dependsOn(dstHost));
                            assertFalse(vflow.dependsOn(hostEE));
                            assertFalse(vflow.dependsOn(hostFF));
                            assertEquals(hasIflow, vflow.dependsOn(imvlan));
                            assertEquals(sameVlan && !hasIflow,
                                         vflow.dependsOn
                                         (new MacVlan(dst, svlan)));

                            // Add egress flow.
                            actions = new ActionList(node);
                            actions.addOutput(dport).addVlanId(dvlan);
                            vflow.addFlow(vtnMgr, match, actions, 10);

                            edges = vflow.getEdgeHosts();
                            assertEquals(sh, edges.getLeft());
                            assertEquals(dh, edges.getRight());

                            assertTrue(vflow.dependsOn(srcHost));
                            assertTrue(vflow.dependsOn(dstHost));
                            assertFalse(vflow.dependsOn(hostEE));
                            assertFalse(vflow.dependsOn(hostFF));
                            assertFalse(vflow.dependsOn(imvlan));
                            for (MacVlan mv: hosts) {
                                assertEquals(sameVlan, vflow.dependsOn(mv));
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for
     * {@link VTNFlow#equals(Object)} and
     * {@link VTNFlow#hashCode()}.
     */
    @Test
    public void testEquals() {
        short[] vlans = new short[] {-1, 0, 1, 4095};
        int[] priorities = new int[] {1, 10, 0xefff};
        Set<String> tnames = new HashSet<String>();
        tnames.add("d");
        tnames.add("default");

        VBridgePath bpath = new VBridgePath("tenant", "bridge");
        VlanMapPath ipath = new VlanMapPath(bpath, "OF|100");
        VBridgeIfPath epath = new VBridgeIfPath("t1", "b1", "i1");

        Set<Object> set = new HashSet<Object>();
        Set<Object> setMulti = new HashSet<Object>();
        List<NodeConnector> connectors = createNodeConnectors(3, false);
        for (String tname : tnames) {
            FlowGroupId gidMulti = new FlowGroupId(tname);
            VTNFlow vflowMul1 = new VTNFlow(gidMulti);
            VTNFlow vflowMul2 = new VTNFlow(gidMulti);

            for (NodeConnector port : connectors) {
                for (MatchType mtype : MatchType.values()) {
                    for (short vlan : vlans) {
                        for (int pri : priorities) {
                            FlowGroupId gid = new FlowGroupId(tname);
                            VTNFlow vflow1 = new VTNFlow(gid);
                            VTNFlow vflow2 = new VTNFlow(gid);

                            // IN_PORT must be included.
                            Match match = new Match();
                            match.setField(MatchType.IN_PORT, port);

                            if (mtype == MatchType.DL_OUTER_VLAN ||
                                    mtype == MatchType.DL_VLAN) {
                                match = setMatchField(match, mtype, vlan);
                            } else {
                                match = setMatchField(match, mtype);
                            }

                            ActionList actions = new ActionList(port.getNode());
                            actions.addOutput(port);
                            if (vlan >= 0) {
                                actions.addVlanId(vlan);
                            }

                            vflow1.addFlow(vtnMgr, match, actions, pri);
                            vflow2.addFlow(vtnMgr, match, actions, pri);

                            testEquals(set, vflow1, vflow2);

                            vflowMul1.addFlow(vtnMgr, match, actions, pri);
                            vflowMul2.addFlow(vtnMgr, match, actions, pri);
                        }
                    }
                }
            }
            testEquals(setMulti, vflowMul1, vflowMul2);

            // Node paths must not affect object identify.
            vflowMul2.setIngressPath(ipath);
            vflowMul2.setEgressPath(epath);
            assertFalse(setMulti.add(vflowMul2));
        }

        int required = tnames.size() * connectors.size() * MatchType.values().length
                        * vlans.length * priorities.length;
        assertEquals(required, set.size());

        required = tnames.size();
        assertEquals(required, setMulti.size());
    }

    /**
     * Ensure that {@link VTNFlow} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = new short[] {-1, 0, 1, 4095};
        int[] priorities = new int[] {1, 10, 0xefff};
        Set<String> tnames = new HashSet<String>();
        tnames.add("t");
        tnames.add("tenant");

        for (String tname : tnames) {
            FlowGroupId gid = new FlowGroupId(tname);
            VTNFlow vflow = new VTNFlow(gid);
            assertEquals(gid, vflow.getGroupId());

            for (NodeConnector port : createNodeConnectors(3, false)) {
                for (MatchType mtype : MatchType.values()) {
                    for (short vlan : vlans) {
                        for (int pri : priorities) {
                            FlowGroupId flowGidOneMatch = new FlowGroupId(tname);
                            VTNFlow flowOneMatch = new VTNFlow(flowGidOneMatch);

                            // IN_PORT must be included.
                            Match match = new Match();
                            match.setField(MatchType.IN_PORT, port);

                            if (mtype == MatchType.DL_OUTER_VLAN ||
                                    mtype == MatchType.DL_VLAN) {
                                match = setMatchField(match, mtype, vlan);
                            } else {
                                match = setMatchField(match, mtype);
                            }

                            ActionList actions = new ActionList(port.getNode());
                            actions.addOutput(port);
                            if (vlan >= 0) {
                                actions.addVlanId(vlan);
                            }

                            flowOneMatch.addFlow(vtnMgr, match, actions, pri);
                            serializeTest(flowOneMatch);

                            // add multiple match.
                            vflow.addFlow(vtnMgr, match, actions, pri);
                            serializeTest(vflow);

                            // Set node dependency.
                            VTenantPath tpath0 = new VTenantPath("t0_" + vlan);
                            VTenantPath tpath1 = new VTenantPath("t1_" + vlan);
                            Set<VTenantPath> pset = new HashSet<VTenantPath>();
                            assertTrue(pset.add(tpath0));
                            assertTrue(pset.add(tpath1));
                            vflow.addDependency(pset);

                            VTenantPath tpath2 = new VTenantPath("vtn" + vlan);
                            VBridgePath bpath2 =
                                new VBridgePath(tpath2, "vbr" + vlan);
                            VlanMapPath vpath = new VlanMapPath(bpath2, "id");
                            VTenantPath tpath3 = new VTenantPath("t" + vlan);
                            VBridgePath bpath3 =
                                new VBridgePath(tpath3, "b" + vlan);
                            VBridgeIfPath ipath =
                                new VBridgeIfPath(bpath3, "if" + vlan);
                            vflow.setIngressPath(vpath);
                            vflow.setEgressPath(ipath);
                            VTenantPath[] paths = {
                                tpath0, tpath1,
                                tpath2, bpath2, vpath,
                                tpath3, bpath3, ipath,
                            };

                            VTNFlow vf = (VTNFlow)serializeTest(vflow);
                            for (VTenantPath path: paths) {
                                assertTrue(vf.dependsOn(path));
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTNFlow#isLocal(VTNManagerImpl)}.
     */
    @Test
    public void testIsLocal() {
        int priority = 0;
        String tname = "tenant";

        Set<NodeConnector> ncSet = new HashSet<NodeConnector>();
        Node node0 = NodeCreator.createOFNode(Long.valueOf("0"));
        Node node1 = NodeCreator.createOFNode(Long.valueOf("1"));
        NodeConnector nc0 = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node0);
        NodeConnector nc1 = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node1);
        ncSet.add(nc0);
        ncSet.add(nc1);

        InetAddress ipaddr = null;
        try {
            ipaddr = InetAddress.getByAddress(new byte[] {(byte)192, (byte)168,
                                                          (byte)0, (byte)100});
        } catch (UnknownHostException e) {
           unexpected(e);
        }

        for (NodeConnector port : ncSet) {
            String emsg = port.toString();

            // create by FlowGroupId(String)
            FlowGroupId gid = new FlowGroupId(tname);
            VTNFlow vflow = new VTNFlow(gid);
            assertEquals(emsg, gid, vflow.getGroupId());

            Match match = new Match();
            match.setField(MatchType.IN_PORT, port);
            ActionList actions = new ActionList(port.getNode());
            actions.addOutput(port);

            vflow.addFlow(vtnMgr, match, actions, priority);
            assertTrue(emsg, vflow.isLocal(vtnMgr));

            // create by FlowGroupId(InetAddress, long, String)
            gid = new FlowGroupId(ipaddr, 0L, tname);
            vflow = new VTNFlow(gid);
            assertEquals(emsg, gid, vflow.getGroupId());
            vflow.addFlow(vtnMgr, match, actions, priority);
            if (port.getNode().equals(node0)) {
                assertTrue(emsg, vflow.isLocal(vtnMgr));
            } else {
                assertFalse(emsg, vflow.isLocal(vtnMgr));
            }
        }
    }

    /**
     * Set match Field.
     *
     * @param match     A target {@link Match} object.
     * @param mtype     A {@link MatchType} which is set.
     * @return  Match object.
     */
    private Match setMatchField(Match match, MatchType mtype) {
        return setMatchField(match, mtype, (short) -1);
    }

    /**
     * Set match Filed.
     *
     * @param match     A target {@link Match} object.
     * @param mtype     A {@link MatchType} which is set.
     * @param vlan      A set VLAN ID. This is used when DL_OUTER_VLAN or DL_VLAN
     *                  is specified as {@code mtype}.
     * @return  Match object.
     */
    private Match setMatchField(Match match, MatchType mtype, short vlan) {
        switch (mtype) {
        case IN_PORT:
            break;
        case DL_SRC:
            byte[] src = new byte[] {0, 0, 0, 0, 0, 0};
            match.setField(mtype, src);
            break;
        case DL_DST:
            byte[] dst = new byte[] {0, 0, 0, 0, 0, 0};
            match.setField(mtype, dst);
            break;
        case DL_OUTER_VLAN:
            if (vlan <= 0) {
                break;
            }
        case DL_VLAN:
            if (vlan >= 0) {
                match.setField(mtype, Short.valueOf(vlan));
            }
            break;
        case DL_VLAN_PR:
            match.setField(mtype, Byte.valueOf("1"));
            break;
        case DL_OUTER_VLAN_PR:
            match.setField(mtype, Short.valueOf("1"));
            break;
        case DL_TYPE:
            match.setField(mtype, EtherTypes.ARP.shortValue());
            break;
        case NW_TOS:
            match.setField(mtype, Byte.valueOf((byte) 0));
            break;
        case NW_PROTO:
            match.setField(mtype, IPProtocols.TCP.byteValue());
            break;
        case NW_SRC:
            InetAddress ipaddr = null;
            try {
                ipaddr = InetAddress.getByAddress(new byte[] {10, 0, 0, 1});
            } catch (UnknownHostException e) {
               unexpected(e);
            }
            match.setField(mtype, ipaddr);
            break;
        case NW_DST:
            ipaddr = null;
            try {
                ipaddr = InetAddress.getByAddress(new byte[] {10, 0, 0, 2});
            } catch (UnknownHostException e) {
               unexpected(e);
            }
            match.setField(mtype, ipaddr);
            break;
        case TP_SRC:
            match.setField(mtype, (short) 1000);
            break;
        case TP_DST:
            match.setField(mtype, (short) 2000);
            break;
        default:
            break;
        }
        return match;
    }
}
