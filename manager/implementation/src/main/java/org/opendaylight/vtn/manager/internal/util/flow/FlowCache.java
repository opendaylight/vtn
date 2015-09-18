/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.OrderedComparator;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtils;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchUtils;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalEgressPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalEgressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalIngressPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.physical.route.info.PhysicalIngressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRoute;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRouteBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataEgressNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataEgressPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataEgressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataIngressNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataIngressPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataIngressPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.PhysicalRoute;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.PhysicalRouteBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortLocation;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code FlowCache} describes a set of cached attributes for a data flow.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
public final class FlowCache implements VTNDataFlow {
    /**
     * A {@link VtnDataFlow} instance associated with the target data flow.
     */
    private final VtnDataFlow  dataFlow;

    /**
     * Comparator for ordered list.
     */
    private final OrderedComparator  comparator = new OrderedComparator();

    /**
     * A list of flow entries sorted in ascending order of "order" value.
     */
    private List<VtnFlowEntry>  flowEntries;

    /**
     * The ingress flow entry in the data flow.
     */
    private VtnFlowEntry  ingressFlow;

    /**
     * The egress flow entry in the data flow.
     */
    private VtnFlowEntry  egressFlow;

    /**
     * The source L2 host of the data flow.
     */
    private L2Host  sourceHost;

    /**
     * The edge hosts of the data flow.
     */
    private ObjectPair<L2Host, L2Host>  edgeHosts;

    /**
     * A set of MD-SAL node identifiers which indicates physical switches
     * on the physical packet route of the data flow.
     */
    private Set<SalNode>  nodeSet;

    /**
     * A set of MD-SAL node connector identifiers which indicates physical
     * switch ports on the physical packet route of the data flow.
     */
    private Set<SalPort>  portSet;

    /**
     * A list of {@link VirtualRoute} instances sorted in ascending order of
     * "order" value.
     */
    private List<VirtualRoute>  virtualRoute;

    /**
     * Path to the virtual node which maps the ingress flow.
     */
    private VNodePath  ingressPath;

    /**
     * Path to the virtual node which maps the egress flow.
     */
    private VNodePath  egressPath;

    /**
     * A list of {@link PhysicalRoute} instances which represents the packet
     * route in the physical network.
     */
    private List<PhysicalRoute>  physicalRoute;

    /**
     * A list of MD-SAL flow actions configured in the egress flow entry.
     */
    private List<Action>  egressActions;

    /**
     * A string which specifies the flow match coinfigured in the ingress flow
     * entry.
     */
    private String  ingressMatchKey;

    /**
     * Construct a new instance.
     *
     * @param vdf  A {@link VtnDataFlow} instance.
     *             Specifying {@code null} results in undefined behavior.
     */
    public FlowCache(VtnDataFlow vdf) {
        dataFlow = vdf;
    }

    /**
     * Return a {@link VtnDataFlow} instance associated with the target data
     * flow.
     *
     * @return  A {@link VtnDataFlow} instance.
     */
    public VtnDataFlow getDataFlow() {
        return dataFlow;
    }

    /**
     * Return a list of flow entries sorted in order of order value.
     *
     * <p>
     *   Note that this method returns unmodifiable list.
     * </p>
     *
     * @return  A list of {@link VtnFlowEntry} instances.
     */
    public List<VtnFlowEntry> getFlowEntries() {
        List<VtnFlowEntry> entries = flowEntries;
        if (entries == null) {
            entries = dataFlow.getVtnFlowEntry();
            entries = (entries == null || entries.isEmpty())
                ? Collections.<VtnFlowEntry>emptyList()
                : Collections.unmodifiableList(
                    MiscUtils.sortedCopy(entries, comparator));
            flowEntries = entries;
        }

        return entries;
    }

    /**
     * Return the ingress flow entry of the data flow.
     *
     * @return  The first {@link VtnFlowEntry} instance in the flow entry list.
     *          {@code null} if the data flow has no flow entry.
     */
    public VtnFlowEntry getIngressFlow() {
        VtnFlowEntry ingress = ingressFlow;
        if (ingress == null) {
            List<VtnFlowEntry> entries = getFlowEntries();
            ingress = (entries.isEmpty()) ? null : entries.get(0);
            ingressFlow = ingress;
        }

        return ingress;
    }

    /**
     * Return the egress flow entry of the data flow.
     *
     * @return  The last {@link VtnFlowEntry} instance in the flow entry list.
     *          {@code null} if the data flow has no flow entry.
     */
    public VtnFlowEntry getEgressFlow() {
        VtnFlowEntry egress = egressFlow;
        if (egress == null) {
            List<VtnFlowEntry> entries = getFlowEntries();
            int len = entries.size();
            if (len > 0) {
                egress = entries.get(len - 1);
                egressFlow = egress;
            }
        }

        return egress;
    }

    /**
     * Return the source L2 host of the data flow.
     *
     * @return  A {@link L2Host} instance if found.
     *          {@code null} if not found.
     */
    public L2Host getSourceHost() {
        L2Host src = sourceHost;
        if (src == null) {
            VtnFlowEntry ingress = getIngressFlow();
            if (ingress != null) {
                src = FlowMatchUtils.getSourceHost(ingress.getMatch());
                sourceHost = src;
            }
        }

        return src;
    }

    /**
     * Return the source L2 host of the data flow.
     *
     * @return  A {@link DataFlowSource} instance if found.
     *          {@code null} if not found.
     */
    public DataFlowSource getDataFlowSource() {
        L2Host src = getSourceHost();
        return (src == null)
            ? null
            : src.getHost().getDataFlowSource();
    }

    /**
     * Get two {@link L2Host} instances which represents host information
     * matched by edge flow entries.
     *
     * @return  A {@link ObjectPair} instance which contains two
     *          {@link L2Host} instances. A {@link L2Host} instance which
     *          represents incoming host address is set to the left side,
     *          and outgoing host address is set to the right side.
     *          Note that {@code null} may be set for outgoing host.
     *          {@code null} is returned if not found.
     */
    public ObjectPair<L2Host, L2Host> getEdgeHosts() {
        ObjectPair<L2Host, L2Host> edges = edgeHosts;
        if (edges == null) {
            L2Host src = getSourceHost();
            L2Host dst;
            if (src == null) {
                dst = null;
            } else {
                // Ingress flow entry should never be null because the source
                // host is specified by the ingress flow match.
                VtnFlowEntry ingress = getIngressFlow();
                MacAddress dmac = FlowMatchUtils.getDestinationMacAddress(
                    ingress.getMatch());
                List<Action> actions = getEgressActions();
                dst = FlowActionUtils.getDestinationHost(
                    actions, dmac, (int)src.getHost().getVlan());
            }
            edges = new ObjectPair<>(src, dst);
            edgeHosts = edges;
        }

        return (edges.getLeft() == null) ? null : edges;
    }

    /**
     * Return a list of {@link VirtualRoute} instances which indicates the
     * virtual route of the data flow.
     *
     * <p>
     *   Note that this method returns unmodifiable list.
     * </p>
     *
     * @return  A list of {@link VirtualRoute} instances.
     */
    public List<VirtualRoute> getVirtualRoute() {
        List<VirtualRoute> vroutes = virtualRoute;
        if (vroutes == null) {
            vroutes = dataFlow.getVirtualRoute();
            vroutes = (vroutes == null || vroutes.isEmpty())
                ? Collections.<VirtualRoute>emptyList()
                : Collections.unmodifiableList(
                    MiscUtils.sortedCopy(vroutes, comparator));
            virtualRoute = vroutes;
        }

        return vroutes;
    }

    /**
     * Return the path to the virtual node which maps the ingress flow.
     *
     * @return  A path to virtual node which maps the ingress flow if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    public VNodePath getIngressPath() throws VTNException {
        VNodePath path = ingressPath;
        if (path == null) {
            List<VirtualRoute> vroutes = getVirtualRoute();
            if (!vroutes.isEmpty()) {
                VirtualRoute vr = vroutes.get(0);
                path = VNodeUtils.toVNodePath(vr.getVirtualNodePath());
                ingressPath = path;
            }
        }

        return path;
    }

    /**
     * Return the path to the virtual node which maps the egress flow.
     *
     * @return  A path to virtual node which maps the egress flow if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    public VNodePath getEgressPath() throws VTNException {
        VNodePath path = egressPath;
        if (path == null) {
            List<VirtualRoute> vroutes = getVirtualRoute();
            int size = vroutes.size();
            if (size > 0) {
                VirtualRoute vr = vroutes.get(size - 1);
                path = VNodeUtils.toVNodePath(vr.getVirtualNodePath());
                egressPath = path;
            }
        }

        return path;
    }

    /**
     * Return a list of {@link PhysicalRoute} instances which indicates the
     * physical route of the data flow.
     *
     * <p>
     *   Note that this method returns unmodifiable list.
     * </p>
     *
     * @param inv  An {@link InventoryReader} instance.
     * @return  A list of {@link PhysicalRoute} instances.
     *          {@code null} if no physical route is configured.
     */
    public List<PhysicalRoute> getPhysicalRoute(InventoryReader inv) {
        initPhysicalRoute(inv);
        return physicalRoute;
    }

    /**
     * Return a list of MD-SAL flow actions configured in the egress flow entry
     * of the data flow.
     *
     * <ul>
     *   <li>
     *     Actions in the returned list are sorted in ascending order of
     *     flow order.
     *   </li>
     *   <li>Note that this method returns unmodifiable list.</li>
     * </ul>
     *
     * @return  A list of {@link Action} instances.
     */
    public List<Action> getEgressActions() {
        List<Action> actions = egressActions;
        if (actions == null) {
            VtnFlowEntry egress = getEgressFlow();
            actions = (egress == null)
                ? Collections.<Action>emptyList()
                : FlowActionUtils.getActions(egress.getInstructions(),
                                             comparator);
            egressActions = actions;
        }

        return actions;
    }

    /**
     * Return a {@link DataFlowInfoBuilder} that contains information about
     * the data flow.
     *
     * <p>
     *   Note that this method does not set flow statistics.
     * </p>
     *
     * @param inv   An {@link InventoryReader} instance.
     * @param mode  A {@link DataFlowMode} instance which determines fields
     *              to copy.
     * @return  A {@link DataFlowInfoBuilder} instance.
     * @throws VTNException  An error occurred.
     */
    public DataFlowInfoBuilder toDataFlowInfoBuilder(
        InventoryReader inv, DataFlowMode mode) throws VTNException {
        DataFlowInfoBuilder builder = new DataFlowInfoBuilder();
        builder.fieldsFrom(dataFlow);

        // Determine ingress and egress virtual nodes.
        List<VirtualRoute> vroutes = getVirtualRoute();
        int vrlen = vroutes.size();
        if (vrlen > 0) {
            VirtualNodePath in = getVirtualNodePath(vroutes, 0);
            VirtualNodePath out = getVirtualNodePath(vroutes, vrlen - 1);
            if (in != null) {
                DataIngressNodeBuilder ib = new DataIngressNodeBuilder();
                ib.fieldsFrom(in);
                builder.setDataIngressNode(ib.build());
            }
            if (out != null) {
                DataEgressNodeBuilder eb = new DataEgressNodeBuilder();
                eb.fieldsFrom(out);
                builder.setDataEgressNode(eb.build());
            }
        }

        // Copy attributes in the ingress flow entry.
        VtnFlowEntry ingress = getIngressFlow();
        Match match = null;
        if (ingress != null) {
            builder.setIdleTimeout(ingress.getIdleTimeout()).
                setHardTimeout(ingress.getHardTimeout());

            match = ingress.getMatch();
            SalPort inPort = SalPort.create(
                FlowMatchUtils.getIngressPort(match));
            builder.setDataIngressPort(toDataIngressPort(inv, inPort));
        }

        // Set the egress port.
        List<Action> actions = getEgressActions();
        SalPort outPort = SalPort.create(FlowActionUtils.
                                         getOutputPort(actions));
        builder.setDataEgressPort(toDataEgressPort(inv, outPort));

        if (mode == DataFlowMode.SUMMARY) {
            builder.setVirtualRoute(null);
        } else {
            VTNMatch vmatch = new VTNMatch(match);
            Short ipproto = vmatch.getInetProtocol();
            List<VtnFlowAction> vfacts = FlowActionUtils.
                toVtnFlowActions(actions, comparator, ipproto);
            builder.setDataFlowMatch(vmatch.toDataFlowMatchBuilder().build()).
                setVirtualRoute(getPublicVirtualRoute(vroutes)).
                setVtnFlowAction(vfacts).
                setPhysicalRoute(getPhysicalRoute(inv));
        }

        return builder;
    }

    /**
     * Eliminate internal information from the given {@link VirtualRoute} list.
     *
     * @param vroutes  A list of {@link VirtualRoute} instances.
     * @return  A list of {@link VirtualRoute} instances to be exported.
     */
    private List<VirtualRoute> getPublicVirtualRoute(
        List<VirtualRoute> vroutes) {
        List<VirtualRoute> ret = new ArrayList<VirtualRoute>(vroutes.size());
        for (VirtualRoute vr: vroutes) {
            VirtualRouteBuilder builder = new VirtualRouteBuilder(vr);
            VirtualNodePath path = vr.getVirtualNodePath();
            if (path != null) {
                VirtualNodePath newPath = new VirtualNodePathBuilder(path).
                    removeAugmentation(BridgeMapInfo.class).
                    build();
                builder.setVirtualNodePath(newPath);
            }
            ret.add(builder.build());
        }

        return ret;
    }

    /**
     * Initialize caches for physical packet routing.
     *
     * @param inv  An {@link InventoryReader} instance.
     */
    private void initPhysicalRoute(InventoryReader inv) {
        if (physicalRoute != null) {
            return;
        }

        Set<SalPort> ports = new HashSet<SalPort>();
        List<PhysicalRoute> proutes = new ArrayList<>();
        int order = MiscUtils.ORDER_MIN;
        for (VtnFlowEntry vfent: getFlowEntries()) {
            SalPort ingress = SalPort.create(
                FlowMatchUtils.getIngressPort(vfent.getMatch()));
            if (ingress == null) {
                // This should never happen.
                proutes = null;
            } else {
                ports.add(ingress);
            }

            List<Action> actions = FlowActionUtils.
                getActions(vfent.getInstructions(), null);
            SalPort egress = SalPort.create(
                FlowActionUtils.getOutputPort(actions));
            if (egress == null) {
                // No physical packet route.
                proutes = null;
            } else {
                ports.add(egress);
                if (proutes != null && ingress != null) {
                    SalNode snode = ingress.getSalNode();
                    assert snode.getNodeNumber() == egress.getNodeNumber();
                    PhysicalRoute pr = new PhysicalRouteBuilder().
                        setNode(snode.getNodeId()).
                        setPhysicalIngressPort(
                            toPhysicalIngressPort(inv, ingress)).
                        setPhysicalEgressPort(
                            toPhysicalEgressPort(inv, egress)).
                        setOrder(order).
                        build();
                    proutes.add(pr);
                    order++;
                }
            }
        }

        if (portSet == null) {
            portSet = Collections.unmodifiableSet(ports);
        }
        if (proutes != null) {
            physicalRoute = Collections.unmodifiableList(proutes);
        }
    }

    /**
     * Return the virtual node path configured in the virtual packet route at
     * the specified index.
     *
     * @param vroutes  A list of {@link VirtualRoute} instances.
     * @param index    Index of the virtual packet route.
     * @return  A {@link VirtualNodePath} instance or {@code null}.
     */
    private VirtualNodePath getVirtualNodePath(List<VirtualRoute> vroutes,
                                               int index) {
        VirtualRoute vr = vroutes.get(index);
        return (vr == null) ? null : vr.getVirtualNodePath();
    }

    /**
     * Return the name of the specified switch port.
     *
     * @param inv    An {@link InventoryReader} instance.
     * @param sport  A {@link SalPort} instance corresponding to a switch port.
     * @return  The name of the specified switch port on success.
     *          {@code null} on failure.
     */
    private String getPortName(InventoryReader inv, SalPort sport) {
        try {
            VtnPort vport = inv.get(sport);
            if (vport == null) {
                Logger logger = LoggerFactory.getLogger(FlowCache.class);
                logger.warn("Unknown port: {}", sport);
                return null;
            }

            return vport.getName();
        } catch (VTNException e) {
            Logger logger = LoggerFactory.getLogger(FlowCache.class);
            logger.warn("Failed to get port information: " + sport, e);
            return null;
        }
    }

    /**
     * Create a {@link DataIngressPort} instance.
     *
     * @param inv    An {@link InventoryReader} instance.
     * @param sport  A {@link SalPort} instance to be converted.
     * @return  A {@link DataIngressPort} instance.
     */
    private DataIngressPort createDataIngressPort(InventoryReader inv,
                                                  SalPort sport) {
        return new DataIngressPortBuilder().
            setNode(sport.getNodeId()).
            setPortId(Long.toString(sport.getPortNumber())).
            setPortName(getPortName(inv, sport)).
            build();
    }

    /**
     * Convert the given {@link SalPort} instance into a
     * {@link DataIngressPort} instance.
     *
     * @param inv    An {@link InventoryReader} instance.
     * @param sport  A {@link SalPort} instance to be converted.
     * @return  A {@link DataIngressPort} instance if {@code sport} is not
     *          {@code null}. Otherwise {@code null}.
     */
    private DataIngressPort toDataIngressPort(InventoryReader inv,
                                              SalPort sport) {
        return (sport == null)
            ? null
            : createDataIngressPort(inv, sport);
    }

    /**
     * Convert the given {@link SalPort} instance into a
     * {@link DataEgressPort} instance.
     *
     * @param inv    An {@link InventoryReader} instance.
     * @param sport  A {@link SalPort} instance to be converted.
     * @return  A {@link DataEgressPort} instance if {@code sport} is not
     *          {@code null}. Otherwise {@code null}.
     */
    private DataEgressPort toDataEgressPort(InventoryReader inv,
                                            SalPort sport) {
        if (sport == null) {
            return null;
        }

        VtnPortLocation vpl = createDataIngressPort(inv, sport);
        return new DataEgressPortBuilder(vpl).build();
    }

    /**
     * Convert the given {@link SalPort} instance into a
     * {@link PhysicalIngressPort} instance.
     *
     * @param inv    An {@link InventoryReader} instance.
     * @param sport  A {@link SalPort} instance to be converted.
     * @return  A {@link PhysicalIngressPort} instance if {@code sport} is not
     *          {@code null}. Otherwise {@code null}.
     */
    private PhysicalIngressPort toPhysicalIngressPort(InventoryReader inv,
                                                      SalPort sport) {
        VtnPortLocation vpl = createDataIngressPort(inv, sport);
        return new PhysicalIngressPortBuilder(vpl).build();
    }

    /**
     * Convert the given {@link SalPort} instance into a
     * {@link PhysicalEgressPort} instance.
     *
     * @param inv    An {@link InventoryReader} instance.
     * @param sport  A {@link SalPort} instance to be converted.
     * @return  A {@link PhysicalEgressPort} instance if {@code sport} is not
     *          {@code null}. Otherwise {@code null}.
     */
    private PhysicalEgressPort toPhysicalEgressPort(InventoryReader inv,
                                                    SalPort sport) {
        VtnPortLocation vpl = createDataIngressPort(inv, sport);
        return new PhysicalEgressPortBuilder(vpl).build();
    }

    // VTNDataFlow

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnDataFlowKey getKey() {
        return dataFlow.getKey();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getIngressMatchKey() throws VTNException {
        String key = ingressMatchKey;
        if (key == null) {
            VtnFlowEntry ingress = getIngressFlow();
            if (ingress != null) {
                Match match = ingress.getMatch();
                VTNMatch vmatch = new VTNMatch(match);
                SalNode snode = SalNode.create(ingress.getNode());
                SalPort sport = SalPort.create(
                    FlowMatchUtils.getIngressPort(match));
                Integer p = ingress.getPriority();
                int pri = (p == null) ? 0 : p.intValue();
                key = vmatch.getFlowKey(snode, pri, sport);
                ingressMatchKey = key;
            }
        }

        return key;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<SalNode> getFlowNodes() {
        Set<SalNode> nodes = nodeSet;
        if (nodes == null) {
            nodes = new HashSet<>();
            for (VtnFlowEntry vfent: getFlowEntries()) {
                SalNode snode = SalNode.create(vfent.getNode());
                if (snode != null) {
                    nodes.add(snode);
                }
            }
            nodes = Collections.unmodifiableSet(nodes);
            nodeSet = nodes;
        }

        return nodes;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<SalPort> getFlowPorts() {
        Set<SalPort> ports = portSet;
        if (portSet == null) {
            ports = new HashSet<>();
            for (VtnFlowEntry vfent: getFlowEntries()) {
                SalPort ingress = SalPort.
                    create(FlowMatchUtils.getIngressPort(vfent.getMatch()));
                if (ingress != null) {
                    ports.add(ingress);
                }

                List<Action> actions = FlowActionUtils.
                    getActions(vfent.getInstructions(), null);
                SalPort egress = SalPort.
                    create(FlowActionUtils.getOutputPort(actions));
                if (egress != null) {
                    ports.add(egress);
                }
            }
            ports = Collections.unmodifiableSet(ports);
            portSet = ports;
        }

        return ports;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SourceHostFlowsKey getSourceHostFlowsKey() {
        L2Host src = getSourceHost();
        return (src == null)
            ? null
            : src.getHost().getSourceHostFlowsKey();
    }
}
