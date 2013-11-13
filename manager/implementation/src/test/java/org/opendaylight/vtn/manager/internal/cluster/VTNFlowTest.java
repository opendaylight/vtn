/**
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import static org.junit.Assert.*;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.ActionList;
import org.opendaylight.vtn.manager.internal.FlowModTaskTestBase;

/**
 * JUnit Test for {@link VTNFlow}
 */
public class VTNFlowTest extends FlowModTaskTestBase {

    /**
     * test case for getter methods.
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
    }

    /**
     * test case for
     * {@link VTNFlow#addDependency(Set)},
     * {@link VTNFlow#dependsOn(VTenantPath)}.
     */
    @Test
    public void testDependencyTenantPath() {
        VTenantPath tpathNotMatch = new VTenantPath("not_match_tenant");
        Set<VTenantPath> pathSet = new HashSet<VTenantPath>();

        FlowGroupId gid = new FlowGroupId("test");
        VTNFlow vflow = new VTNFlow(gid);
        for (String tname : createStrings("vtn", false)) {
            pathSet.clear();
            pathSet.add(new VTenantPath(tname));
            vflow.addDependency(pathSet);

            for (VTenantPath path : pathSet) {
                assertTrue(vflow.dependsOn(new VTenantPath(path.getTenantName())));
            }
            assertFalse(vflow.dependsOn(tpathNotMatch));
        }
        assertEquals(gid, vflow.getGroupId());

        pathSet.clear();
        for (String tname : createStrings("strings", false)) {
            gid = new FlowGroupId(tname);
            vflow = new VTNFlow(gid);
            assertEquals(tname, gid, vflow.getGroupId());

            pathSet.add(new VTenantPath(tname));
            vflow.addDependency(pathSet);

            for (VTenantPath path : pathSet) {
                assertTrue(path.toString(),
                           vflow.dependsOn(new VTenantPath(path.getTenantName())));
            }
            assertFalse(tname, vflow.dependsOn(tpathNotMatch));
        }
    }

    /**
     * test case for
     * {@link VTNFlow#addDependency(MacVlan)},
     * {@link VTNFlow#dependsOn(MacVlan)}.
     */
    @Test
    public void testDependencyMacVlan() {
        short[] vlans = new short[] {-1, 0, 1, 4095};

        FlowGroupId gid = new FlowGroupId("test");
        VTNFlow vflow = new VTNFlow(gid);

        MacVlan mvNotMatch = new MacVlan(new byte[] {0, 0, 0, 0, 0, (byte)0xff},
                                         (short) 9);

        Set<MacVlan> mvSet = new HashSet<MacVlan>();
        for (short vlan : vlans) {
            for (EthernetAddress ea : createEthernetAddresses(false)) {
                MacVlan mv = new MacVlan(ea.getValue(), vlan);
                mvSet.add(mv);

                vflow.addDependency(mv);

                for (MacVlan regMacVlan : mvSet) {
                    String emsg = "(mvSet)" + mvSet.toString()
                            + ",(regMacVlan)" + regMacVlan.toString();
                    byte[] mac = NetUtils.longToByteArray6(regMacVlan.getMacAddress());
                    assertTrue(emsg,
                               vflow.dependsOn(new MacVlan(mac, regMacVlan.getVlan())));
                }
                assertFalse(mv.toString(), vflow.dependsOn(mvNotMatch));
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
        }

        int required = tnames.size() * connectors.size() * MatchType.values().length
                        * vlans.length * priorities.length;
        assertEquals(required, set.size());

        required = tnames.size();
        assertEquals(required, setMulti.size());
    }

    /**
     *  Ensure that {@link VTNFlow} is serializable.
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
                        }
                    }
                }
            }
        }
    }

    /**
     * Test method for {@link VTNFlow#isLocal(VTNManagerImpl)}.
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
     * @param match     A target Match object.
     * @param mtype     A MatchType which is set.
     * @return  Match object.
     */
    private Match setMatchField(Match match, MatchType mtype) {
        return setMatchField(match, mtype, (short) -1);
    }

    /**
     * Set match Filed.
     *
     * @param match     A target Match object.
     * @param mtype     A MatchType which is set.
     * @param vlan      A set VLAN ID. This is used when DL_OUTER_VLAN or DL_VLAN
     *                  is specified as mtype.
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
