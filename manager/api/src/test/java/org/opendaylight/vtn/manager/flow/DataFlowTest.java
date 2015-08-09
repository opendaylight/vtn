/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.NodeRoute;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.DropAction;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlAttributeType;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.PushVlan;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.action.Drop;
import org.opendaylight.controller.sal.action.SetVlanPcp;
import org.opendaylight.controller.sal.action.ActionType;
import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetNwTos;
import org.opendaylight.controller.sal.action.SetTpSrc;
import org.opendaylight.controller.sal.action.SetTpDst;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;

/**
 * JUnit test for {@link DataFlow}.
 */
public class DataFlowTest extends TestBase {
    /**
     * Root XML element name associated with {@link DataFlow} class.
     */
    private static final String  XML_ROOT = "dataflow";

    /**
     * Flow ID for test.
     */
    private static final long  FLOW_ID = 0x123456789abcdeL;

    /**
     * Flow creation time for test.
     */
    private static final long  CREATION_TIME = 1414755625000L;

    /**
     * Flow idle timeout for test.
     */
    private static final short  IDLE_TIMEOUT = 300;

    /**
     * Flow hard timeout for test.
     */
    private static final short  HARD_TIMEOUT = 0;

    /**
     * Path to ingress virtual node for test.
     */
    private static final VNodePath  INGRESS_NODE_PATH =
        new VBridgePath("vtn1", "bridge1");

    /**
     * Path to egress virtual node for test.
     */
    private static final VNodePath  EGRESS_NODE_PATH =
        new VBridgeIfPath("vtn1", "bridge1", "if1");

    /**
     * Location of the ingress switch port for test.
     */
    private static final PortLocation  INGRESS_PORT;

    /**
     * Location of the egress switch port for test.
     */
    private static final PortLocation  EGRESS_PORT;

    /**
     * A set of IP protocol numbers supported by TP_SRC/TP_DST action.
     */
    private static final Set<Integer>  TP_PROTOCOLS = new HashSet<Integer>();

    /**
     * A list of unique {@link DataFlow} instances.
     */
    private static final List<DataFlow> DATAFLOWS =
        new ArrayList<DataFlow>();

    /**
     * Initialize static fields.
     */
    static {
        Node node = NodeCreator.createOFNode(Long.valueOf(1L));
        SwitchPort port = new SwitchPort("port-3");
        INGRESS_PORT = new PortLocation(node, port);

        node = NodeCreator.createOFNode(Long.valueOf(0x123456789aL));
        port = new SwitchPort("OF", "12");
        EGRESS_PORT = new PortLocation(node, port);

        TP_PROTOCOLS.add(Integer.valueOf(IPProtocols.TCP.intValue()));
        TP_PROTOCOLS.add(Integer.valueOf(IPProtocols.UDP.intValue()));
        TP_PROTOCOLS.add(Integer.valueOf(IPProtocols.ICMP.intValue()));

        // Construct a list of unique DataFlow instances.
        DataFlowGenerator gen = new DataFlowGenerator();
        for (int i = 0; i < 64; i++) {
            DATAFLOWS.add(gen.next());
        }
    }


    /**
     * Create a {@link DataFlow} instance for test.
     *
     * @return  A {@link DataFlow} instance.
     */
    private static DataFlow createDataFlow() {
        return new DataFlow(FLOW_ID, CREATION_TIME, IDLE_TIMEOUT, HARD_TIMEOUT,
                            INGRESS_NODE_PATH, INGRESS_PORT,
                            EGRESS_NODE_PATH, EGRESS_PORT);
    }

    /**
     * Create a list of SAL ingress flows.
     *
     * @param num  The number of flows to be created.
     * @return  A list of {@link Flow} instances.
     */
    static List<Flow> createIngressFlows(int num) {
        List<Flow> list = new ArrayList<Flow>();

        Short etherType = Short.valueOf(EtherTypes.IPv4.shortValue());
        Byte ipProto = Byte.valueOf((byte)6);
        long dpid = 1L;
        short portId = 1;
        byte mac = 0;
        byte ip = 1;
        short sport = 1;
        short dport = 20000;

        ArrayList<Action> actions = new ArrayList<Action>();
        Node outNode = NodeCreator.createOFNode(Long.valueOf(12345L));
        NodeConnector outPort = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)333), outNode);
        actions.add(new Output(outPort));

        do {
            Node node = NodeCreator.createOFNode(Long.valueOf(dpid));
            NodeConnector port = NodeConnectorCreator.
                createOFNodeConnector(Short.valueOf(portId), node);
            dpid++;
            portId += 2;

            Match match = new Match();
            match.setField(MatchType.IN_PORT, port);
            match.setField(MatchType.DL_VLAN,
                           Short.valueOf(MatchType.DL_VLAN_NONE));
            Flow flow = new Flow(match, new ArrayList<Action>(actions));
            list.add(flow);
            if (list.size() >= num) {
                break;
            }

            // L2 conditions.
            byte[] srcMac = {
                (byte)0x00, (byte)0x01, (byte)0x02,
                (byte)0x03, (byte)0x04, mac,
            };
            byte[] dstMac = {
                (byte)0x1a, (byte)0x2b, (byte)0x3c,
                (byte)0x4d, mac, (byte)0x6f,
            };
            mac++;

            match = new Match();
            match.setField(MatchType.IN_PORT, port);
            match.setField(MatchType.DL_VLAN, Short.valueOf((short)10));
            match.setField(MatchType.DL_SRC, srcMac);
            match.setField(MatchType.DL_DST, dstMac);
            flow = new Flow(match, new ArrayList<Action>(actions));
            list.add(flow);
            if (list.size() >= num) {
                break;
            }

            // L3 conditions.
            byte[] srcAddr = {(byte)192, (byte)168, (byte)10, ip};
            byte[] dstAddr = {(byte)10, (byte)11, ip, (byte)254};
            InetAddress srcIp = null;
            InetAddress dstIp = null;
            ip++;
            try {
                srcIp = InetAddress.getByAddress(srcAddr);
                dstIp = InetAddress.getByAddress(dstAddr);
            } catch (Exception e) {
                unexpected(e);
            }

            match = new Match();
            match.setField(MatchType.IN_PORT, port);
            match.setField(MatchType.DL_VLAN, Short.valueOf((short)100));
            match.setField(MatchType.DL_SRC, srcMac);
            match.setField(MatchType.DL_DST, dstMac);
            match.setField(MatchType.DL_TYPE, etherType);
            match.setField(MatchType.NW_SRC, srcIp);
            match.setField(MatchType.NW_DST, dstIp);
            flow = new Flow(match, new ArrayList<Action>(actions));
            list.add(flow);
            if (list.size() >= num) {
                break;
            }

            // L4 conditions.
            match = new Match();
            match.setField(MatchType.IN_PORT, port);
            match.setField(MatchType.DL_VLAN, Short.valueOf((short)100));
            match.setField(MatchType.DL_SRC, srcMac);
            match.setField(MatchType.DL_DST, dstMac);
            match.setField(MatchType.DL_TYPE, etherType);
            match.setField(MatchType.NW_SRC, srcIp);
            match.setField(MatchType.NW_DST, dstIp);
            match.setField(MatchType.NW_PROTO, ipProto);
            match.setField(MatchType.TP_SRC, Short.valueOf(sport));
            match.setField(MatchType.TP_DST, Short.valueOf(dport));
            flow = new Flow(match, new ArrayList<Action>(actions));
            list.add(flow);
            sport++;
            dport++;
        } while (list.size() < num);

        return list;
    }

    /**
     * Create a list of SAL egress flows.
     *
     * @param num    The number of flows to be created.
     * @param empty  If {@code true} is specified, flow entries that has no
     *               action will be created.
     * @return  A list of {@link Flow} instances.
     */
    static List<Flow> createEgressFlows(int num, boolean empty) {
        List<Flow> list = new ArrayList<Flow>();

        Node inNode = NodeCreator.createOFNode(Long.valueOf(12345L));
        NodeConnector inPort = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)777), inNode);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, inPort);
        match.setField(MatchType.DL_VLAN,
                       Short.valueOf(MatchType.DL_VLAN_NONE));

        long dpid = 1L;
        short portId = 1;
        byte mac = 0;
        byte ip = 1;
        short sport = 4;
        short dport = 10000;

        if (empty) {
            Flow flow = new Flow(match.clone(), null);
            list.add(flow);
            if (list.size() >= num) {
                return list;
            }

            flow = new Flow(match.clone(), new ArrayList<Action>());
            list.add(flow);
            if (list.size() >= num) {
                return list;
            }
        }

        do {
            ArrayList<Action> actions = new ArrayList<Action>();
            actions.add(new Drop());
            Flow flow = new Flow(match.clone(), actions);
            list.add(flow);
            if (list.size() >= num) {
                break;
            }

            Node node = NodeCreator.createOFNode(Long.valueOf(dpid));
            NodeConnector port = NodeConnectorCreator.
                createOFNodeConnector(Short.valueOf(portId), node);
            dpid++;
            portId += 2;

            actions = new ArrayList<Action>();
            actions.add(new Output(port));
            flow = new Flow(match.clone(), actions);
            list.add(flow);
            if (list.size() >= num) {
                break;
            }

            // L2 actions.
            byte[] srcMac = {
                (byte)0x00, (byte)0x02, (byte)0x04,
                (byte)0x06, (byte)0x08, mac,
            };
            byte[] dstMac = {
                (byte)0x10, (byte)0x2a, (byte)0x3b,
                (byte)0x4c, mac, (byte)0x6e,
            };
            mac++;

            actions = new ArrayList<Action>();
            actions.add(new SetDlSrc(srcMac));
            actions.add(new SetDlDst(dstMac));
            actions.add(new PopVlan());
            actions.add(new Output(port));
            flow = new Flow(match.clone(), actions);
            list.add(flow);
            if (list.size() >= num) {
                break;
            }

            // L3 actions.
            byte[] srcAddr = {(byte)192, (byte)168, (byte)20, ip};
            byte[] dstAddr = {(byte)10, (byte)55, ip, (byte)254};
            InetAddress srcIp = null;
            InetAddress dstIp = null;
            ip++;
            try {
                srcIp = InetAddress.getByAddress(srcAddr);
                dstIp = InetAddress.getByAddress(dstAddr);
            } catch (Exception e) {
                unexpected(e);
            }

            actions = new ArrayList<Action>();
            actions.add(new SetDlSrc(srcMac));
            actions.add(new SetDlDst(dstMac));
            actions.add(new SetNwSrc(srcIp));
            actions.add(new SetNwDst(dstIp));
            actions.add(new PushVlan(EtherTypes.VLANTAGGED));
            actions.add(new SetVlanId((short)3));
            actions.add(new Output(port));
            flow = new Flow(match.clone(), actions);
            list.add(flow);
            if (list.size() >= num) {
                break;
            }

            // L4 actions.
            actions = new ArrayList<Action>();
            actions.add(new SetDlSrc(srcMac));
            actions.add(new SetDlDst(dstMac));
            actions.add(new SetNwSrc(srcIp));
            actions.add(new SetNwDst(dstIp));
            actions.add(new SetNwTos(10));
            actions.add(new SetTpSrc(sport));
            actions.add(new SetTpDst(dport));
            actions.add(new SetVlanId((short)4095));
            actions.add(new SetVlanPcp((short)2));
            actions.add(new Output(port));
            flow = new Flow(match.clone(), actions);
            list.add(flow);
            sport++;
            dport++;
        } while (list.size() < num);

        return list;
    }

    /**
     * Test case for constructor and related getter methods.
     */
    @Test
    public void testGetter() {
        long[] flowIds = {
            Long.MIN_VALUE, -123456789012L, -555555L, 0L,
            1L, 1000L, 9999999999L, Long.MAX_VALUE};
        long[] createdTimes = {0L, 12345L, 81929828L, Long.MAX_VALUE};
        short[] idleTimeouts = {
            0, 1, 123, 1000, 20000, Short.MAX_VALUE,
        };
        short[] hardTimeouts = {
            0, 7, 82, 918, 30000, Short.MAX_VALUE,
        };

        for (long id: flowIds) {
            for (long created: createdTimes) {
                for (short idle: idleTimeouts) {
                    for (short hard: hardTimeouts) {
                        getterTest(id, created, idle, hard);
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link DataFlow#getStatistics()} and
     * {@link DataFlow#setStatistics(FlowStats)}.
     */
    @Test
    public void testStats() {
        DataFlow df = createDataFlow();

        long[] packets = {0L, 1000L, 10000000000L};
        long[] bytes = {0L, 12345L, 123456789012L};
        long[] duration = {0L, 8888888L, 9999999999999L};
        FlowStats prev = null;
        for (long p: packets) {
            for (long b: bytes) {
                for (long d: duration) {
                    FlowStats fst = new FlowStats(p, b, d);
                    assertEquals(prev, df.getStatistics());
                    df.setStatistics(fst);
                    assertEquals(fst, df.getStatistics());
                    checkDataFlow(df);
                    prev = fst;
                }
            }
        }

        df.setStatistics(null);
        assertEquals(null, df.getStatistics());
        checkDataFlow(df);
    }

    /**
     * Test case for {@link DataFlow#getAveragedStatistics()} and
     * {@link DataFlow#setAveragedStatistics(AveragedFlowStats)}.
     */
    @Test
    public void testAveragedStats() {
        DataFlow df = createDataFlow();

        double[] packets = {0D, 123D, 1234.5678D};
        double[] bytes = {0D, 3333D, 34567.8901D};
        long[] starts = {0L, 123456L, 1234567890L};
        long[] ends = {0L, 3333333L, 9999999999999L};
        AveragedFlowStats prev = null;

        for (double p: packets) {
            for (double b: bytes) {
                for (long s: starts) {
                    for (long e: ends) {
                        AveragedFlowStats stats =
                            new AveragedFlowStats(p, b, s, e);
                        assertEquals(prev, df.getAveragedStatistics());
                        df.setAveragedStatistics(stats);
                        assertEquals(stats, df.getAveragedStatistics());
                        checkDataFlow(df);
                        prev = stats;
                    }
                }
            }
        }

        df.setAveragedStatistics(null);
        assertEquals(null, df.getAveragedStatistics());
        checkDataFlow(df);
    }

    /**
     * Test case for virtual route.
     *
     * <ul>
     *   <li>{@link DataFlow#getVirtualRoute()}</li>
     *   <li>{@link DataFlow#setVirtualRoute(List)}</li>
     * </ul>
     */
    @Test
    public void testVirtualRoute() {
        LinkedList<VNodeRoute> vroutes = new LinkedList<VNodeRoute>();
        VirtualRouteReason[] reasons = VirtualRouteReason.values();
        vroutes.add(new VNodeRoute());
        for (VNodePath path: createVNodePaths(10, false)) {
            for (VirtualRouteReason reason: reasons) {
                vroutes.add(new VNodeRoute(path, reason));
            }
        }

        DataFlow df = createDataFlow();
        assertEquals(null, df.getVirtualRoute());
        df.setVirtualRoute(vroutes);
        List<VNodeRoute> result = df.getVirtualRoute();
        assertEquals(vroutes, result);
        checkDataFlow(df);

        // Ensure that VNodeRoute list was copied.
        List<VNodeRoute> result1 = df.getVirtualRoute();
        assertEquals(vroutes, result1);
        assertEquals(result, result1);
        assertNotSame(result, result1);
    }

    /**
     * Test case for physical route.
     *
     * <ul>
     *   <li>{@link DataFlow#getPhysicalRoute()}</li>
     *   <li>{@link DataFlow#setPhysicalRoute(List)}</li>
     * </ul>
     */
    @Test
    public void testPhysicalRoute() {
        ArrayList<NodeRoute> routes = new ArrayList<NodeRoute>();
        for (Node node: createNodes(5)) {
            for (SwitchPort input: createSwitchPorts(5)) {
                for (SwitchPort output: createSwitchPorts(5)) {
                    routes.add(new NodeRoute(node, input, output));
                }
            }
        }

        DataFlow df = createDataFlow();
        assertEquals(null, df.getPhysicalRoute());
        df.setPhysicalRoute(routes);
        assertEquals(routes, df.getPhysicalRoute());
        checkDataFlow(df);

        // Ensure that NodeRoute list was copied.
        List<NodeRoute> result = df.getPhysicalRoute();
        List<NodeRoute> result1 = df.getPhysicalRoute();
        assertEquals(routes, result);
        assertEquals(result, result1);
        assertNotSame(result, result1);
        checkDataFlow(df);
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link DataFlow#setMatch(FlowMatch)}</li>
     *   <li>{@link DataFlow#setActions(List)}</li>
     *   <li>{@link DataFlow#getMatch()}</li>
     *   <li>{@link DataFlow#getActions()}</li>
     * </ul>
     */
    @Test
    public void testEdgeFlows() {
        List<Flow> ingressFlows = createIngressFlows(10);
        List<Flow> egressFlows = createEgressFlows(10, true);
        for (Flow ingress: ingressFlows) {
            FlowMatch fmatch = DataFlowGenerator.
                toFlowMatch(ingress.getMatch());
            int ipproto = fmatch.getInetProtocol();
            for (Flow egress: egressFlows) {
                DataFlow df = createDataFlow();
                assertEquals(null, df.getMatch());
                assertEquals(null, df.getActions());

                DataFlowGenerator.setEdgeFlows(df, ingress, egress);
                assertEquals(fmatch, df.getMatch());
                checkDataFlow(df);

                List<FlowAction> actions = df.getActions();
                List<Action> acts = egress.getActions();
                List<FlowAction> expected = toFlowAction(acts, ipproto);
                assertEquals(expected, actions);

                if (actions != null) {
                    // Ensure that action list was copied.
                    List<FlowAction> a1 = df.getActions();
                    assertEquals(expected, a1);
                    assertEquals(actions, a1);
                    assertNotSame(actions, a1);
                }
            }
        }
    }

    /**
     * Test case for {@link DataFlow#equals(Object)} and
     * {@link DataFlow#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        DataFlowGenerator gen = new DataFlowGenerator();
        int count = 0;
        for (DataFlow df1: DATAFLOWS) {
            DataFlow df2 = gen.next();
            testEquals(set, df1, df2);
        }

        assertEquals(DATAFLOWS.size(), set.size());
    }

    /**
     * Ensure that {@link FlowStats} is serializable.
     */
    @Test
    public void testSerialize() {
        for (DataFlow df: DATAFLOWS) {
            serializeTest(df);
        }
    }

    /**
     * Ensure that {@link DataFlow} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (DataFlow df: DATAFLOWS) {
            jaxbTest(df, DataFlow.class, XML_ROOT);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(DataFlow.class,
                      new XmlAttributeType(XML_ROOT, "id", long.class),
                      new XmlAttributeType(XML_ROOT, "creationTime",
                                           long.class),
                      new XmlAttributeType(XML_ROOT, "idleTimeout",
                                           short.class),
                      new XmlAttributeType(XML_ROOT, "hardTimeout",
                                           short.class),

                      // FlowMatch
                      new XmlAttributeType("match", "index", Integer.class).
                      add(XML_ROOT),
                      new XmlAttributeType("ethernet", "type", Integer.class).
                      add(XML_ROOT, "match"),
                      new XmlAttributeType("ethernet", "vlan", Short.class).
                      add(XML_ROOT, "match"),
                      new XmlAttributeType("ethernet", "vlanpri", Byte.class).
                      add(XML_ROOT, "match"),
                      new XmlAttributeType("inet4", "srcsuffix", Short.class).
                      add(XML_ROOT, "match"),
                      new XmlAttributeType("inet4", "dstsuffix", Short.class).
                      add(XML_ROOT, "match"),
                      new XmlAttributeType("inet4", "protocol", Short.class).
                      add(XML_ROOT, "match"),
                      new XmlAttributeType("inet4", "dscp", Byte.class).
                      add(XML_ROOT, "match"),
                      new XmlAttributeType("src", "from", Integer.class).
                      add(XML_ROOT, "match", "tcp"),
                      new XmlAttributeType("src", "to", Integer.class).
                      add(XML_ROOT, "match", "tcp"),
                      new XmlAttributeType("dst", "from", Integer.class).
                      add(XML_ROOT, "match", "tcp"),
                      new XmlAttributeType("dst", "to", Integer.class).
                      add(XML_ROOT, "match", "tcp"),
                      new XmlAttributeType("src", "from", Integer.class).
                      add(XML_ROOT, "match", "udp"),
                      new XmlAttributeType("src", "to", Integer.class).
                      add(XML_ROOT, "match", "udp"),
                      new XmlAttributeType("dst", "from", Integer.class).
                      add(XML_ROOT, "match", "udp"),
                      new XmlAttributeType("dst", "to", Integer.class).
                      add(XML_ROOT, "match", "udp"),
                      new XmlAttributeType("icmp", "type", Short.class).
                      add(XML_ROOT, "match"),
                      new XmlAttributeType("icmp", "code", Short.class).
                      add(XML_ROOT, "match"),

                      // FlowAction
                      new XmlAttributeType("pushvlan", "type", int.class).
                      add(XML_ROOT, "actions"),
                      new XmlAttributeType("vlanid", "vlan", short.class).
                      add(XML_ROOT, "actions"),
                      new XmlAttributeType("vlanpcp", "priority", byte.class).
                      add(XML_ROOT, "actions"),
                      new XmlAttributeType("dscp", "dscp", byte.class).
                      add(XML_ROOT, "actions"),
                      new XmlAttributeType("tpsrc", "port", int.class).
                      add(XML_ROOT, "actions"),
                      new XmlAttributeType("tpdst", "port", int.class).
                      add(XML_ROOT, "actions"),
                      new XmlAttributeType("icmptype", "type", short.class).
                      add(XML_ROOT, "actions"),
                      new XmlAttributeType("icmpcode", "code", short.class).
                      add(XML_ROOT, "actions"));
    }

    /**
     * Ensure that {@link DataFlow} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (DataFlow df: DATAFLOWS) {
            jsonTest(df, DataFlow.class);
        }
    }

    /**
     * Test case for {@link DataFlowMode} class.
     */
    @Test
    public void testMode() {
        DataFlowMode[] values = DataFlowMode.values();
        assertEquals(3, values.length);
        assertEquals(DataFlowMode.SUMMARY, values[0]);
        assertEquals(DataFlowMode.DETAIL, values[1]);
        assertEquals(DataFlowMode.UPDATESTATS, values[2]);
        assertEquals(DataFlowMode.SUMMARY, DataFlowMode.forValue(0));
        assertEquals(DataFlowMode.DETAIL, DataFlowMode.forValue(1));
        assertEquals(DataFlowMode.UPDATESTATS, DataFlowMode.forValue(2));
        assertEquals(0, DataFlowMode.SUMMARY.ordinal());
        assertEquals(1, DataFlowMode.DETAIL.ordinal());
        assertEquals(2, DataFlowMode.UPDATESTATS.ordinal());
        assertEquals(0, DataFlowMode.SUMMARY.getIntValue());
        assertEquals(1, DataFlowMode.DETAIL.getIntValue());
        assertEquals(2, DataFlowMode.UPDATESTATS.getIntValue());
    }

    /**
     * Internal method for {@link #testGetter()}.
     *
     * @param id       An identifier of the data flow.
     * @param created  A long integer value which represents the creation
     *                 time of the data flow.
     * @param idle     The idle timeout value of the data flow.
     * @param hard     The hard timeout value of the data flow.
     */
    private void getterTest(long id, long created, short idle, short hard) {
        for (VNodePath ipath: createVNodePaths(4)) {
            for (VNodePath opath: createVNodePaths(4)) {
                for (PortLocation iport: createPortLocations(1L, 4)) {
                    for (PortLocation oport: createPortLocations(100L, 4)) {
                        DataFlow df = new DataFlow(id, created, idle, hard,
                                                   ipath, iport, opath, oport);
                        assertEquals(id, df.getFlowId());
                        assertEquals(created, df.getCreationTime());
                        assertEquals(idle, df.getIdleTimeout());
                        assertEquals(hard, df.getHardTimeout());
                        assertEquals(ipath, df.getIngressNodePath());
                        assertEquals(opath, df.getEgressNodePath());
                        assertEquals(iport, df.getIngressPort());
                        assertEquals(oport, df.getEgressPort());
                    }
                }
            }
        }
    }

    /**
     * Verify that field values created by {@link #createDataFlow()} are
     * not changed.
     *
     * @param df  A {@link DataFlow} to be tested.
     */
    private void checkDataFlow(DataFlow df) {
        assertEquals(FLOW_ID, df.getFlowId());
        assertEquals(CREATION_TIME, df.getCreationTime());
        assertEquals(IDLE_TIMEOUT, df.getIdleTimeout());
        assertEquals(HARD_TIMEOUT, df.getHardTimeout());
        assertEquals(INGRESS_NODE_PATH, df.getIngressNodePath());
        assertEquals(INGRESS_PORT, df.getIngressPort());
        assertEquals(EGRESS_NODE_PATH, df.getEgressNodePath());
        assertEquals(EGRESS_PORT, df.getEgressPort());
    }

    /**
     * Convert a list of {@link Action} instances info a list of
     * {@link FlowAction} instances.
     *
     * @param actions  A list of SAL actions.
     * @param ipproto  An IP protocol number defined in the ingress flow.
     * @return  A list of {@link FlowAction} instances.
     */
    private List<FlowAction> toFlowAction(List<Action> actions, int ipproto) {
        List<FlowAction> list = new ArrayList<FlowAction>();
        if (actions == null || actions.isEmpty()) {
            list.add(new DropAction());
        } else {
            Integer proto = Integer.valueOf(ipproto);
            for (Action act: actions) {
                ActionType type = act.getType();
                switch (type) {
                case OUTPUT:
                    break;

                case SET_TP_SRC:
                case SET_TP_DST:
                    if (!TP_PROTOCOLS.contains(proto)) {
                        break;
                    }
                    // FALLTHROUGH

                default:
                    FlowAction fact = DataFlowGenerator.
                        toFlowAction(act, ipproto);
                    assertNotNull(fact);
                    list.add(fact);
                    break;
                }
            }
        }

        return list;
    }
}
