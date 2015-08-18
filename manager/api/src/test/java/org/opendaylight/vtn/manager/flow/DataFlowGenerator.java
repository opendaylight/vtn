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
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.NodeRoute;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.flow.action.DropAction;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.PopVlanAction;
import org.opendaylight.vtn.manager.flow.action.PushVlanAction;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanIdAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowMatchBuilder;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Drop;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.PushVlan;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.action.SetNwTos;
import org.opendaylight.controller.sal.action.SetTpDst;
import org.opendaylight.controller.sal.action.SetTpSrc;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.action.SetVlanPcp;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchField;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.NodeCreator;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;

/**
 * Generator of unique {@link DataFlow} instances.
 *
 * <p>
 *   This class always generates the same sequence of data flows.
 * </p>
 */
public final class DataFlowGenerator extends TestBase {
    /**
     * The number of data flows expected to be created.
     */
    private static final int  DATAFLOW_COUNT = 64;

    /**
     * Convert the given AD-SAL match into {@link FlowMatch} instance.
     *
     * @param match  An AD-SAL match.
     * @return  A {@link FlowMatch} instance.
     * @throws IllegalArgumentException
     *    The given AD-SAL match contains unexpected value.
     */
    public static FlowMatch toFlowMatch(Match match) {
        FlowMatchBuilder builder = new FlowMatchBuilder();
        if (match != null) {
            builder.setSourceMacAddress(getEtherAddress(
                                            match, MatchType.DL_SRC)).
                setDestinationMacAddress(getEtherAddress(
                                             match, MatchType.DL_DST)).
                setVlanId(getValue(match, MatchType.DL_VLAN, Short.class)).
                setVlanPriority(getValue(match, MatchType.DL_VLAN_PR,
                                         Byte.class)).
                setSourceInetAddress(getValue(
                                         match, MatchType.NW_SRC,
                                         InetAddress.class)).
                setDestinationInetAddress(getValue(
                                              match, MatchType.NW_DST,
                                              InetAddress.class)).
                setInetDscp(getValue(match, MatchType.NW_TOS, Byte.class));

            Short sval = getValue(match, MatchType.DL_TYPE, Short.class);
            if (sval != null) {
                int etype = NumberUtils.getUnsigned(sval.shortValue());
                builder.setEtherType(Integer.valueOf(etype));
            }

            InetAddress iaddr = getMask(match, MatchType.NW_SRC,
                                        InetAddress.class);
            if (iaddr != null) {
                int len = Ip4Network.getPrefixLength(iaddr.getAddress());
                builder.setSourceInetSuffix(Short.valueOf((short)len));
            }

            iaddr = getMask(match, MatchType.NW_DST, InetAddress.class);
            if (iaddr != null) {
                int len = Ip4Network.getPrefixLength(iaddr.getAddress());
                builder.setDestinationInetSuffix(Short.valueOf((short)len));
            }

            Byte bval = getValue(match, MatchType.NW_PROTO, Byte.class);
            Short ipproto;
            if (bval != null) {
                short proto = (short)NumberUtils.getUnsigned(bval.byteValue());
                ipproto = Short.valueOf(proto);
                builder.setInetProtocol(ipproto);
            } else {
                ipproto = null;
            }

            Short srcTp = getValue(match, MatchType.TP_SRC, Short.class);
            Short dstTp = getValue(match, MatchType.TP_DST, Short.class);
            if (srcTp != null || dstTp != null) {
                if (ipproto == null) {
                    throw new IllegalArgumentException(
                        "L4 match without NW_PROTO: " + srcTp + ", " + dstTp);
                }

                int proto = ipproto.intValue();
                if (IPProtocols.TCP.intValue() == proto) {
                    if (srcTp != null) {
                        int port = NumberUtils.getUnsigned(srcTp.shortValue());
                        builder.setTcpSourcePort(port);
                    }
                    if (dstTp != null) {
                        int port = NumberUtils.getUnsigned(dstTp.shortValue());
                        builder.setTcpDestinationPort(port);
                    }
                } else if (IPProtocols.UDP.intValue() == proto) {
                    if (srcTp != null) {
                        int port = NumberUtils.getUnsigned(srcTp.shortValue());
                        builder.setUdpSourcePort(port);
                    }
                    if (dstTp != null) {
                        int port = NumberUtils.getUnsigned(dstTp.shortValue());
                        builder.setUdpDestinationPort(port);
                    }
                } else if (IPProtocols.ICMP.intValue() == proto) {
                    builder.setIcmpType(srcTp);
                    builder.setIcmpCode(dstTp);
                } else {
                    throw new IllegalArgumentException(
                        "Unexpected IP protocol: " + ipproto);
                }
            }
        }

        return builder.build();
    }

    /**
     * Return the value associated with the given match type in the given
     * AD-SAL match.
     *
     * @param match  An AD-SAL match.
     * @param mtype  A {@link MatchType} instance which specifies the type
     *               of match field.
     * @param type   A class which indicates the type of the value.
     * @param <T>    The type of the value configured in the specified
     *               match field.
     * @return  A value associated with the given match type.
     *          {@code null} if no value is associated.
     * @throws IllegalArgumentException
     *    Unexpected value is configured in the given AD-SAL match.
     */
    private static <T> T getValue(Match match, MatchType mtype,
                                  Class<T> type) {
        MatchField mf = match.getField(mtype);
        if (mf == null) {
            return null;
        }

        Object value = mf.getValue();
        if (type.isInstance(value)) {
            return type.cast(value);
        }

        // This should never happen.
        String msg = "Unexpected match field: type=" + mtype + ", field=" + mf;
        throw new IllegalArgumentException(msg);
    }

    /**
     * Return the mask value associated with the given match type in the given
     * AD-SAL match.
     *
     * @param match  An AD-SAL match.
     * @param mtype  A {@link MatchType} instance which specifies the type
     *               of match field.
     * @param type   A class which indicates the type of the mask.
     * @param <T>    The type of the mask value configured in the specified
     *               match field.
     * @return  A mask value associated with the given match type.
     *          {@code null} if no mask value is associated.
     * @throws IllegalArgumentException
     *    Unexpected value is configured in the given AD-SAL match.
     */
    private static <T> T getMask(Match match, MatchType mtype,
                                 Class<T> type) {
        MatchField mf = match.getField(mtype);
        if (mf == null) {
            return null;
        }

        Object mask = mf.getMask();
        if (mask == null) {
            return null;
        }

        if (type.isInstance(mask)) {
            return type.cast(mask);
        }

        // This should never happen.
        String msg = "Unexpected match mask: type=" + mtype + ", field=" + mf;
        throw new IllegalArgumentException(msg);
    }

    /**
     * Return the {@link EtherAddress} instance associated with the given
     * match type in the given AD-SAL match.
     *
     * @param match  An AD-SAL match.
     * @param mtype  A {@link MatchType} instance which specifies the type
     *               of match field.
     * @return  The {@link EtherAddress} instance associated with the given
     *          match type. {@code null} if no value is associated.
     * @throws IllegalArgumentException
     *    Unexpected value is configured in the given AD-SAL match.
     */
    private static EtherAddress getEtherAddress(Match match, MatchType mtype) {
        byte[] mac = getValue(match, mtype, byte[].class);
        return EtherAddress.create(mac);
    }

    /**
     * Convert the given AD-SAL action into {@code FlowAction} instance.
     *
     * @param act      An AD-SAL action.
     * @param ipproto  IP protocol number.
     *                 This parameter is used if SET_TP_SRC or SET_TP_DST
     *                 action is passed to {@code act}.
     * @return  A {@link FlowAction} instance converted from the given
     *          SAL action. {@code null} is returned if the given SAL action
     *          is not supported.
     */
    public static FlowAction toFlowAction(Action act, int ipproto) {
        if (act == null) {
            return null;
        }

        if (act instanceof Drop) {
            return new DropAction();
        }
        if (act instanceof PopVlan) {
            return new PopVlanAction();
        }
        if (act instanceof PushVlan) {
            PushVlan a = (PushVlan)act;
            return new PushVlanAction(a.getTag());
        }
        if (act instanceof SetDlDst) {
            SetDlDst a = (SetDlDst)act;
            return new SetDlDstAction(a.getDlAddress());
        }
        if (act instanceof SetDlSrc) {
            SetDlSrc a = (SetDlSrc)act;
            return new SetDlSrcAction(a.getDlAddress());
        }
        if (act instanceof SetNwTos) {
            SetNwTos a = (SetNwTos)act;
            return new SetDscpAction((byte)a.getNwTos());
        }
        if (act instanceof SetNwDst) {
            SetNwDst a = (SetNwDst)act;
            return new SetInet4DstAction(a.getAddress());
        }
        if (act instanceof SetNwSrc) {
            SetNwSrc a = (SetNwSrc)act;
            return new SetInet4SrcAction(a.getAddress());
        }
        if (act instanceof SetVlanId) {
            SetVlanId a = (SetVlanId)act;
            return new SetVlanIdAction((short)a.getVlanId());
        }
        if (act instanceof SetVlanPcp) {
            SetVlanPcp a = (SetVlanPcp)act;
            return new SetVlanPcpAction((byte)a.getPcp());
        }

        if (act instanceof SetTpDst) {
            SetTpDst a = (SetTpDst)act;
            int port = a.getPort();
            if (ipproto == IPProtocols.TCP.intValue() ||
                ipproto == IPProtocols.UDP.intValue()) {
                return new SetTpDstAction(port);
            } else if (ipproto == IPProtocols.ICMP.intValue()) {
                return new SetIcmpCodeAction((short)port);
            }
        } else if (act instanceof SetTpSrc) {
            SetTpSrc a = (SetTpSrc)act;
            int port = a.getPort();
            if (ipproto == IPProtocols.TCP.intValue() ||
                ipproto == IPProtocols.UDP.intValue()) {
                return new SetTpSrcAction(port);
            } else if (ipproto == IPProtocols.ICMP.intValue()) {
                return new SetIcmpTypeAction((short)port);
            }
        }

        return null;
    }

    /**
     * Set information about ingress and egress flow entries to the given
     * {@link DataFlow} instance.
     *
     * @param df       A {@link DataFlow} instance.
     * @param ingress  A SAL flow which represents the ingress flow entry of
     *                 the data flow.
     * @param egress   A SAL flow which represents the egress flow entry of
     *                 the data flow.
     */
    public static void setEdgeFlows(DataFlow df, Flow ingress, Flow egress) {
        FlowMatch fmatch = toFlowMatch(ingress.getMatch());
        df.setMatch(fmatch);

        List<Action> actlist = egress.getActions();
        List<FlowAction> facts = new ArrayList<>();
        if (actlist == null || actlist.isEmpty()) {
            facts.add(new DropAction());
        } else {
            int ipproto = fmatch.getInetProtocol();
            for (Action act: actlist) {
                FlowAction fact = toFlowAction(act, ipproto);
                if (fact != null) {
                    facts.add(fact);
                }
            }
        }
        df.setActions(facts);
    }

    /**
     * Abstract cyclic list of objects to be passed to {@link DataFlow}
     * instance.
     *
     * @param <T>  Type of arguments.
     */
    private static class ArgumentList<T> {
        /**
         * List of arguments.
         */
        private final List<T>  arguments = new ArrayList<T>();

        /**
         * Arguments iterator.
         */
        private Iterator<T>  iterator;

        /**
         * Add the given object to the argument list.
         *
         * @param arg  An argument to be added.
         * @return  This instance.
         */
        protected ArgumentList<T> add(T arg) {
            arguments.add(arg);
            return this;
        }

        /**
         * Add all the objects in the given list to the argument list.
         *
         * @param args  A list of arguments to be added.
         */
        protected void addAll(List<T> args) {
            arguments.addAll(args);
        }

        /**
         * Set up the iterator.
         */
        private void setUp() {
            iterator = arguments.iterator();
            assert iterator.hasNext();
        }

        /**
         * Return the next argument in the iteration.
         *
         * @return  The next argument in the iteration.
         */
        private T next() {
            if (!iterator.hasNext()) {
                setUp();
            }

            return iterator.next();
        }

        /**
         * Return the object at the given index.
         *
         * @param index  The index of the argument list.
         * @return  The object at the given index.
         */
        private T get(int index) {
            return arguments.get(index);
        }
    }

    /**
     * Cyclic argument list for lists.
     *
     * @param <T>  Type of list elements.
     */
    private static class ListArgumentList<T> extends ArgumentList<List<T>> {
        /**
         * Add the given object to the argument list.
         *
         * @param arg  An argument to be added.
         * @return  This instance.
         */
        @Override
        protected ArgumentList<List<T>> add(List<T> arg) {
            List<T> list = (arg == null)
                ? null : Collections.unmodifiableList(arg);
            super.add(list);
            return this;
        }

        /**
         * Add all the objects in the given list to the argument list.
         *
         * @param args  A list of arguments to be added.
         */
        @Override
        protected void addAll(List<List<T>> args) {
            for (List<T> list: args) {
                add(list);
            }
        }
    }

    /**
     * A pair of ingress and egress flows.
     */
    private static class EdgeFlow {
        /**
         * Ingress flow entry.
         */
        private final Flow  ingressFlow;

        /**
         * Egress flow entry.
         */
        private final Flow  egressFlow;

        /**
         * Construct a new instance.
         *
         * @param ingress  An ingress flow entry.
         * @param egress   An egress flow entry.
         */
        private EdgeFlow(Flow ingress, Flow egress) {
            ingressFlow = ingress;
            egressFlow = egress;
        }

        /**
         * Return an ingress flow entry.
         *
         * @return  Ingress flow entry.
         */
        private Flow getIngressFlow() {
            return ingressFlow;
        }

        /**
         * Return an egress flow entry.
         *
         * @return  Egress flow entry.
         */
        private Flow getEgressFlow() {
            return egressFlow;
        }
    }

    /**
     * Data flow identifier.
     */
    private final ArgumentList<Long>  flowIds = new ArgumentList<Long>();

    /**
     * Creation time of the data flow.
     */
    private final ArgumentList<Long> creationTimes = new ArgumentList<Long>();

    /**
     * Idle timeout of the data flow.
     */
    private final ArgumentList<Short>  idleTimeouts =
        new ArgumentList<Short>();

    /**
     * Hard timeout of the data flow.
     */
    private final ArgumentList<Short>  hardTimeouts =
        new ArgumentList<Short>();

    /**
     * The location of the virtual node which maps the ingress flow.
     */
    private final ArgumentList<VNodePath>  ingressPaths =
        new ArgumentList<VNodePath>();

    /**
     * The location of the ingress switch port.
     */
    private final ArgumentList<PortLocation>  ingressPorts =
        new ArgumentList<PortLocation>();

    /**
     * The location of the virtual node which maps the egress flow.
     */
    private final ArgumentList<VNodePath>  egressPaths =
        new ArgumentList<VNodePath>();

    /**
     * The location of the egress switch port.
     */
    private final ArgumentList<PortLocation>  egressPorts =
        new ArgumentList<PortLocation>();

    /**
     * Statistics information of the data flow.
     */
    private final ArgumentList<FlowStats>  statistics =
        new ArgumentList<FlowStats>();

    /**
     * Averaged statistics information of the data flow.
     */
    private final ArgumentList<AveragedFlowStats>  averagedStats =
        new ArgumentList<AveragedFlowStats>();

    /**
     * A list of {@link VNodeRoute} instances which represents the virtual
     * packet routing path.
     */
    private final ArgumentList<List<VNodeRoute>>  virtualRoutes =
        new ListArgumentList<VNodeRoute>();

    /**
     * A list of {@link NodeRoute} instances which represents the physical
     * packet routing path.
     */
    private final ArgumentList<List<NodeRoute>>  physicalRoutes =
        new ListArgumentList<NodeRoute>();

    /**
     * SAL flows which represents edge flows.
     */
    private final ArgumentList<EdgeFlow>  edgeFlows =
        new ArgumentList<EdgeFlow>();

    /**
     * A list of {@link DataFlow} instances used to increase coverage of
     * {@link DataFlow#equals(Object)} test.
     */
    private final List<DataFlow>  uniqueFlows = new ArrayList<DataFlow>();

    /**
     * The number of generated instances.
     */
    private int  count;

    /**
     * Construct a new generator instance.
     */
    public DataFlowGenerator() {
        flowIds.add(0L).add(10L).add(112233L).add(9876543210L).
            add(0x123456789abcdL).add(0xaaaabbbbccccL).
            add(0xffffffffffffffffL);
        creationTimes.add(0L).add(111111L).add(333333333L).add(1234567890L);
        idleTimeouts.add((short)0).add((short)300).add((short)1500).
            add((short)2800);
        hardTimeouts.add((short)0).add((short)10000).add((short)32767);
        ingressPaths.addAll(createVNodePaths(DATAFLOW_COUNT / 3));
        ingressPorts.addAll(createPortLocations(1L, DATAFLOW_COUNT / 7));
        egressPaths.addAll(createVNodePaths(DATAFLOW_COUNT / 2));
        egressPorts.addAll(createPortLocations(0x123456789aL, 13));
        statistics.add((FlowStats)null).add(new FlowStats(0L, 0L, 0L)).
            add(new FlowStats(100L, 3000L, 50000L)).
            add(new FlowStats(12345L, 67890L, 123456789L)).
            add(new FlowStats(0x12345L, 0x67890L, 0x123456789L)).
            add(new FlowStats(10000000L, 123456712L, 9999999999L));
        averagedStats.add((AveragedFlowStats)null).
            add(new AveragedFlowStats(0D, 0D, 0L, 0L)).
            add(new AveragedFlowStats(333.44D, 5555.678D, 1234567L, 1235000L)).
            add(new AveragedFlowStats(9999.0D, 98765.4D, 333333L, 444444L)).
            add(new AveragedFlowStats(12345678.9D, 999999999.123D,
                                      123456789L, 1234567890L));

        virtualRoutes.add((List<VNodeRoute>)null);
        String tname = "tenant_1";
        VNodePath vpath = new VBridgePath(tname, "bridge_1");
        List<VNodeRoute> vroute = new ArrayList<VNodeRoute>();
        vroute.add(new VNodeRoute(vpath, VirtualRouteReason.VLANMAPPED));
        virtualRoutes.add(vroute);

        vroute = new ArrayList<VNodeRoute>(vroute);
        vpath = new VBridgeIfPath(tname, "bridge_2", "if_3");
        vroute.add(new VNodeRoute(vpath, VirtualRouteReason.REDIRECTED));
        vpath = new VTerminalPath(tname, "term_3");
        vroute.add(new VNodeRoute(vpath, VirtualRouteReason.REDIRECTED));
        vpath = new VTerminalIfPath(tname, "term_1", "if_11");
        vroute.add(new VNodeRoute(vpath, VirtualRouteReason.REDIRECTED));
        virtualRoutes.add(vroute);

        vroute = new ArrayList<VNodeRoute>();
        vpath = new VBridgeIfPath(tname, "bridge_3", "if_123");
        vroute.add(new VNodeRoute(vpath, VirtualRouteReason.PORTMAPPED));
        vpath = new VBridgePath(tname, "bridge_3");
        vroute.add(new VNodeRoute(vpath, VirtualRouteReason.MACMAPPED));
        virtualRoutes.add(vroute);

        physicalRoutes.add((List<NodeRoute>)null);
        Node node = NodeCreator.createOFNode(Long.valueOf(1L));
        List<SwitchPort> swports = createSwitchPorts(20);
        Iterator<SwitchPort> it = swports.iterator();
        List<NodeRoute> route = new ArrayList<NodeRoute>();
        route.add(new NodeRoute(node, it.next(), it.next()));
        node = NodeCreator.createOFNode(Long.valueOf(2L));
        route.add(new NodeRoute(node, it.next(), it.next()));
        physicalRoutes.add(route);

        route = new ArrayList<NodeRoute>();
        Long dpid = Long.valueOf(0x1234L);
        for (int i = 0; i < 3; i++) {
            node = NodeCreator.createOFNode(dpid);
            route.add(new NodeRoute(node, it.next(), it.next()));
        }
        physicalRoutes.add(route);

        route = new ArrayList<NodeRoute>();
        dpid = Long.valueOf(0xabcdef12345L);
        for (int i = 0; i < 4; i++) {
            node = NodeCreator.createOFNode(dpid);
            route.add(new NodeRoute(node, it.next(), it.next()));
        }
        physicalRoutes.add(route);

        route = new ArrayList<NodeRoute>();
        dpid = Long.valueOf(0xffffffffffffL);
        while (it.hasNext()) {
            node = NodeCreator.createOFNode(dpid);
            route.add(new NodeRoute(node, it.next(), it.next()));
        }
        physicalRoutes.add(route);

        edgeFlows.add((EdgeFlow)null);
        final int nflows = DATAFLOW_COUNT / 4;
        List<Flow> in = DataFlowTest.createIngressFlows(nflows);
        List<Flow> out = DataFlowTest.createEgressFlows(nflows, false);
        for (int i = 0; i < nflows; i++) {
            EdgeFlow ef = new EdgeFlow(in.get(i), out.get(i));
            edgeFlows.add(ef);
        }

        flowIds.setUp();
        creationTimes.setUp();
        idleTimeouts.setUp();
        hardTimeouts.setUp();
        ingressPaths.setUp();
        ingressPorts.setUp();
        egressPaths.setUp();
        egressPorts.setUp();
        statistics.setUp();
        averagedStats.setUp();
        virtualRoutes.setUp();
        physicalRoutes.setUp();
        edgeFlows.setUp();

        // Construct unique data flows for equals() test.
        long id = 1L;
        long created = 77777777L;
        short idle = 500;
        short hard = 1;
        tname = "vtn1";
        String bname = "vbr1";
        String biname = "if5";
        VNodePath ipath = new VBridgeIfPath(tname, bname, biname);
        Node inode = NodeCreator.createOFNode(Long.valueOf(10L));
        SwitchPort swport = new SwitchPort("GBE100");
        PortLocation iport = new PortLocation(inode, swport);
        VNodePath epath = new VBridgePath(tname, bname);
        Node enode = NodeCreator.createOFNode(Long.valueOf(20L));
        String ncType = "OF";
        swport = new SwitchPort(ncType, "19");
        PortLocation eport = new PortLocation(enode, swport);
        FlowStats stats = statistics.get(1);
        AveragedFlowStats average = averagedStats.get(1);
        List<VNodeRoute> vroutes = virtualRoutes.get(1);
        List<NodeRoute> routes = physicalRoutes.get(1);
        EdgeFlow ef = edgeFlows.get(1);

        DataFlow df = new DataFlow(id, created, idle, hard, ipath, iport,
                                   epath, eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        Set<DataFlow> dfSet = new HashSet<>();
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        long id1 = id + 100L;
        df = new DataFlow(id1, created, idle, hard, ipath, iport,
                          epath, eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        long created1 = created + 9999L;
        df = new DataFlow(id, created1, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        short idle1 = (short)(idle + 123);
        df = new DataFlow(id, created, idle1, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        short hard1 = (short)(hard + 5500);
        df = new DataFlow(id, created, idle, hard1, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        VNodePath ipath1 = new VTerminalIfPath(tname, bname, biname);
        df = new DataFlow(id, created, idle, hard, ipath1, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        swport = new SwitchPort(ncType, "20");
        PortLocation iport1 = new PortLocation(inode, swport);
        df = new DataFlow(id, created, idle, hard, ipath, iport1, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        VNodePath epath1 = new VTerminalPath(tname, bname);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath1,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        swport = new SwitchPort("GBE101");
        PortLocation eport1 = new PortLocation(enode, swport);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport1);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        FlowStats stats1 = statistics.get(2);
        AveragedFlowStats average1 = averagedStats.get(2);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats1);
        df.setAveragedStatistics(average1);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        List<VNodeRoute> vroutes1 = virtualRoutes.get(2);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes1);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        List<NodeRoute> routes1 = physicalRoutes.get(2);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes1);
        setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        EdgeFlow ef1 = edgeFlows.get(2);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef1.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));

        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setAveragedStatistics(average);
        df.setVirtualRoute(vroutes);
        df.setPhysicalRoute(routes);
        setEdgeFlows(df, ef.getIngressFlow(), ef1.getEgressFlow());
        uniqueFlows.add(df);
        assertEquals(true, dfSet.add(df));
    }

    /**
     * Return the next {@link DataFlow} instance in the iteration.
     *
     * @return  The next {@link DataFlow} instance in the iteration.
     */
    public DataFlow next() {
        DataFlow df;
        if (count < uniqueFlows.size()) {
            df = uniqueFlows.get(count);
        } else {
            long id = flowIds.next().longValue();
            long created = creationTimes.next().longValue();
            short idle = idleTimeouts.next().shortValue();
            short hard = hardTimeouts.next().shortValue();
            VNodePath ipath = ingressPaths.next();
            PortLocation iport = ingressPorts.next();
            VNodePath epath = egressPaths.next();
            PortLocation eport = egressPorts.next();
            df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                              eport);
            FlowStats stats = statistics.next();
            if (stats != null) {
                df.setStatistics(stats);
            }

            AveragedFlowStats average = averagedStats.next();
            if (average != null) {
                df.setAveragedStatistics(average);
            }

            List<VNodeRoute> vroutes = virtualRoutes.next();
            if (vroutes != null) {
                df.setVirtualRoute(vroutes);
            }

            List<NodeRoute> routes = physicalRoutes.next();
            if (routes != null) {
                df.setPhysicalRoute(routes);
            }

            EdgeFlow ef = edgeFlows.next();
            if (ef != null) {
                setEdgeFlows(df, ef.getIngressFlow(), ef.getEgressFlow());
            }
        }

        count++;
        return df;
    }
}
