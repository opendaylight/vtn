/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.opendaylight.vtn.manager.NodeRoute;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.utils.NodeCreator;

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
         * Add the given objects to the argument list.
         *
         * @param args  Arguments to be added.
         */
        private void add(T ... args) {
            for (T a: args) {
                arguments.add(a);
            }
        }

        /**
         * Add all the objects in the given list to the argument list.
         *
         * @param args  A list of arguments to be added.
         */
        private void addAll(List<T> args) {
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
     * A list of {@link VNodeRoute} instances which represents the virtual
     * packet routing path.
     */
    private final ArgumentList<List<VNodeRoute>>  virtualRoutes =
        new ArgumentList<List<VNodeRoute>>();

    /**
     * A list of {@link NodeRoute} instances which represents the physical
     * packet routing path.
     */
    private final ArgumentList<List<NodeRoute>>  physicalRoutes =
        new ArgumentList<List<NodeRoute>>();

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
        flowIds.add(0L, 10L, 112233L, 9876543210L, 0x123456789abcdL,
                    0xaaaabbbbccccL, 0xffffffffffffffffL);
        creationTimes.add(0L, 111111L, 333333333L, 1234567890L);
        idleTimeouts.add((short)0, (short)300, (short)1500, (short)2800);
        hardTimeouts.add((short)0, (short)10000, (short)32767);
        ingressPaths.addAll(createVNodePaths(DATAFLOW_COUNT / 3));
        ingressPorts.addAll(createPortLocations(1L, DATAFLOW_COUNT / 7));
        egressPaths.addAll(createVNodePaths(DATAFLOW_COUNT / 2));
        egressPorts.addAll(createPortLocations(0x123456789aL, 13));
        statistics.add((FlowStats)null, new FlowStats(0L, 0L, 0L),
                       new FlowStats(100L, 3000L, 50000L),
                       new FlowStats(12345L, 67890L, 123456789L),
                       new FlowStats(0x12345L, 0x67890L, 0x123456789L),
                       new FlowStats(10000000L, 123456712L, 9999999999L));

        virtualRoutes.add((List<VNodeRoute>)null);
        String tname = "tenant_1";
        VNodePath vpath = new VBridgePath(tname, "bridge_1");
        List<VNodeRoute> vroute = new ArrayList<VNodeRoute>();
        vroute.add(new VNodeRoute(vpath, VNodeRoute.Reason.VLANMAPPED));
        virtualRoutes.add(vroute);

        vroute = new ArrayList<VNodeRoute>(vroute);
        vpath = new VBridgeIfPath(tname, "bridge_2", "if_3");
        vroute.add(new VNodeRoute(vpath, VNodeRoute.Reason.REDIRECTED));
        vpath = new VTerminalPath(tname, "term_3");
        vroute.add(new VNodeRoute(vpath, VNodeRoute.Reason.REDIRECTED));
        vpath = new VTerminalIfPath(tname, "term_1", "if_11");
        vroute.add(new VNodeRoute(vpath, VNodeRoute.Reason.REDIRECTED));
        virtualRoutes.add(vroute);

        vroute = new ArrayList<VNodeRoute>();
        vpath = new VBridgeIfPath(tname, "bridge_3", "if_123");
        vroute.add(new VNodeRoute(vpath, VNodeRoute.Reason.PORTMAPPED));
        vpath = new VBridgePath(tname, "bridge_3");
        vroute.add(new VNodeRoute(vpath, VNodeRoute.Reason.MACMAPPED));
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
        List<VNodeRoute> vroutes = virtualRoutes.get(1);
        List<NodeRoute> routes = physicalRoutes.get(1);
        EdgeFlow ef = edgeFlows.get(1);

        DataFlow df = new DataFlow(id, created, idle, hard, ipath, iport,
                                   epath, eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        long id1 = id + 100L;
        df = new DataFlow(id1, created, idle, hard, ipath, iport,
                          epath, eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        long created1 = created + 9999L;
        df = new DataFlow(id, created1, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        short idle1 = (short)(idle + 123);
        df = new DataFlow(id, created, idle1, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        short hard1 = (short)(hard + 5500);
        df = new DataFlow(id, created, idle, hard1, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        VNodePath ipath1 = new VTerminalIfPath(tname, bname, biname);
        df = new DataFlow(id, created, idle, hard, ipath1, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        swport = new SwitchPort(ncType, "20");
        PortLocation iport1 = new PortLocation(inode, swport);
        df = new DataFlow(id, created, idle, hard, ipath, iport1, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        VNodePath epath1 = new VTerminalPath(tname, bname);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath1,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        swport = new SwitchPort("GBE101");
        PortLocation eport1 = new PortLocation(enode, swport);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport1);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        FlowStats stats1 = statistics.get(2);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats1);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        List<VNodeRoute> vroutes1 = virtualRoutes.get(2);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes1);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        List<NodeRoute> routes1 = physicalRoutes.get(2);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes1) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        EdgeFlow ef1 = edgeFlows.get(2);
        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef1.getIngressFlow(), ef.getEgressFlow());
        uniqueFlows.add(df);

        df = new DataFlow(id, created, idle, hard, ipath, iport, epath,
                          eport);
        df.setStatistics(stats);
        df.setVirtualRoute(vroutes);
        for (NodeRoute nr: routes) {
            df.addPhysicalRoute(nr);
        }
        df.setEdgeFlows(ef.getIngressFlow(), ef1.getEgressFlow());
        uniqueFlows.add(df);
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

            List<VNodeRoute> vroutes = virtualRoutes.next();
            if (vroutes != null) {
                df.setVirtualRoute(vroutes);
            }

            List<NodeRoute> routes = physicalRoutes.next();
            if (routes != null) {
                for (NodeRoute nr: routes) {
                    df.addPhysicalRoute(nr);
                }
            }

            EdgeFlow ef = edgeFlows.next();
            if (ef != null) {
                df.setEdgeFlows(ef.getIngressFlow(), ef.getEgressFlow());
            }
        }

        count++;
        return df;
    }
}
