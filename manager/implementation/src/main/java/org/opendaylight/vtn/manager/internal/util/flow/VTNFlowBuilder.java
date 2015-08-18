/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNActionList;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchUtils;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntryBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * {@code VTNFlowBuilder} describes a request for adding a new VTN data flow.
 */
public final class VTNFlowBuilder implements VTNDataFlow {
    /**
     * A set of {@link VirtualRouteReason} instances which indicates the
     * mapping to the vBridge.
     */
    private static final Set<VirtualRouteReason> VBRIDGE_MAP_REASONS =
        EnumSet.of(VirtualRouteReason.VLANMAPPED,
                   VirtualRouteReason.MACMAPPED);

    /**
     * Idle/Hard timeout value for intermediate and egress flow entries.
     */
    private static final int  INTER_FLOW_TIMEOUT = 0;

    /**
     * Flags for FLOW_MOD message.
     */
    private static final FlowModFlags  FLOW_MOD_FLAGS =
        new FlowModFlagsBuilder().setSendFlowRem(true).build();

    /**
     * The name of the VTN.
     */
    private final String  tenantName;

    /**
     * A builder for a VTN data flow.
     */
    private final VtnDataFlowBuilder  flowBuilder;

    /**
     * A VTN data flow instance.
     */
    private VtnDataFlow  dataFlow;

    /**
     * A list of flow entries to be installed.
     */
    private final List<VtnFlowEntryBuilder>  flowEntries = new ArrayList<>();

    /**
     * A {@link VTNMatch} instance which contains the condition to match
     * against packets.
     */
    private final VTNMatch  vtnMatch;

    /**
     * A flow match builder which contains the condition to match against
     * packets.
     */
    private final MatchBuilder  matchBuilder;

    /**
     * A priority for VTN data flow entries.
     */
    private final int  priority;

    /**
     * Idle timeout for VTN data flow.
     */
    private int  idleTimeout;

    /**
     * Hard timeout for VTN data flow.
     */
    private int  hardTimeout;

    /**
     * A set of nodes related to this data flow.
     */
    private final Set<SalNode>  flowNodes = new HashSet<>();

    /**
     * A set of switch ports related to this data flow.
     */
    private final Set<SalPort>  flowPorts = new HashSet<>();

    /**
     * The source L2 host of this data flow.
     */
    private SourceHostFlowsKey  sourceHost;

    /**
     * A sequence of virtual packet routing.
     */
    private final List<VNodeRoute>  virtualRoute = new ArrayList<>();

    /**
     * A string which specifies the flow match coinfigured in the ingress flow
     * entry.
     */
    private String  ingressMatchKey;

    /**
     * Construct a new instance.
     *
     * @param tname   The name of the VTN.
     * @param mac     The MAC address of the controller.
     * @param vmatch  A {@link VTNMatch} instance.
     * @param pri     A flow priority.
     * @param idle    Idle timeout in seconds.
     * @param hard    Hard timeout in seconds.
     */
    public VTNFlowBuilder(String tname, EtherAddress mac, VTNMatch vmatch,
                          int pri, int idle, int hard) {
        flowBuilder = new VtnDataFlowBuilder().
            setControllerAddress(mac.getAddress());
        tenantName = tname;
        vtnMatch = vmatch;
        matchBuilder = vmatch.toMatchBuilder();
        priority = pri;
        idleTimeout = idle;
        hardTimeout = hard;
    }

    /**
     * Return the name of the VTN.
     *
     * @return  The name of the VTN.
     */
    public String getTenantName() {
        return tenantName;
    }

    /**
     * Add an intermediate flow entry which forwards packets via the given
     * link edge.
     *
     * @param src  The source port of the link edge.
     * @param dst  The destination port of the link edge.
     * @return  This instance.
     */
    public VTNFlowBuilder addInternalFlow(SalPort src, SalPort dst) {
        VTNActionList actions = new VTNActionList().addOutputAction(dst);
        createFlowEntry(src, actions);
        flowPorts.add(dst);
        return this;
    }

    /**
     * Add the egress flow entry.
     *
     * @param ingress  A {@link SalPort} instance corresponding to the ingress
     *                 switch port.
     * @param egress   A {@link SalPort} instance corresponding to the egress
     *                 switch port.
     * @param inVid    A VLAN ID configured in the origial packet.
     * @param outVid   A VLAN ID to be set.
     * @param filters  A collection of actions configured to be applied.
     * @return  This instance.
     */
    public VTNFlowBuilder addEgressFlow(
        SalPort ingress, SalPort egress, int inVid, int outVid,
        Collection<? extends FlowFilterAction> filters) {
        VTNActionList actions = new VTNActionList().
            addVlanAction(inVid, outVid).addAll(filters).
            addOutputAction(egress);
        createFlowEntry(ingress, actions);
        flowPorts.add(egress);
        return this;
    }

    /**
     * Add a flow entry that discards packets.
     *
     * @param ingress  A {@link SalPort} instance corresponding to the ingress
     *                 switch port.
     * @return  This instance.
     */
    public VTNFlowBuilder addDropFlow(SalPort ingress) {
        VTNActionList actions = new VTNActionList();
        createFlowEntry(ingress, actions);
        return this;
    }

    /**
     * Set the identifier of the path policy which routed this flow.
     *
     * @param id  The identifier of the path policy.
     * @return  This instance.
     */
    public VTNFlowBuilder setPathPolicyId(int id) {
        flowBuilder.setPathPolicyId(id);
        return this;
    }

    /**
     * Add the specified {@link VNodeRoute} to the tail of the virtual packet
     * routing path.
     *
     * @param vroute  A {@link VNodeRoute} instance.
     * @return  This instance.
     */
    public VTNFlowBuilder addVirtualRoute(VNodeRoute vroute) {
        virtualRoute.add(vroute);
        return this;
    }

    /**
     * Add the specified sequence of {@link VNodeRoute} to the tail of the
     * virtual packet routing path.
     *
     * @param c  A collection of {@link VNodeRoute} instances.
     * @return  This instance.
     */
    public VTNFlowBuilder addVirtualRoute(Collection<VNodeRoute> c) {
        virtualRoute.addAll(c);
        return this;
    }

    /**
     * Set the virtual node hop to the egress virtual node of this flow.
     *
     * @param vroute  A {@link VNodeRoute} instance which represents the hop
     *                to the egress virtual node.
     *                Specifying {@code null} or an empty route means that
     *                this VTN flow always discards packets.
     * @return  This instance.
     */
    public VTNFlowBuilder setEgressVNodeRoute(VNodeRoute vroute) {
        int lastIndex = virtualRoute.size() - 1;
        VNodePath path = (vroute == null) ? null : vroute.getPath();
        if (path == null) {
            // Append an empty route which represents a negative flow.
            addEmptyVNodeRoute(lastIndex);
            return this;
        }

        // Need to adjust the last element of the virtual node path if
        // it contains more than 1 elements.
        if (lastIndex > 0) {
            // A VNodePath which specifies the virtual mapping which
            // established the egress flow must be always configured in the
            // last VNodeRoute element of the virtual packet routing path.
            VNodeRoute last = virtualRoute.get(lastIndex);
            VNodePath lastPath = last.getPath();
            if (path.equals(lastPath)) {
                // The last route element already points the specified virtual
                // mapping.
                return this;
            }

            if (VBRIDGE_MAP_REASONS.contains(vroute.getReason()) &&
                lastPath.contains(path)) {
                // A path to the virtual mapping must be configured in the last
                // virtual packet route.
                VNodeRoute newLast = new VNodeRoute(path, last.getReason());
                virtualRoute.set(lastIndex, newLast);
                return this;
            }
        }

        virtualRoute.add(vroute);
        return this;
    }

    /**
     * Create a VTN data flow instance.
     *
     * @param fid  The identifier for this data flow.
     * @return  A {@link VtnDataFlow} instance.
     */
    public VtnDataFlow createVtnDataFlow(VtnFlowId fid) {
        if (dataFlow == null) {
            // Configure flow entries.
            int order = MiscUtils.ORDER_MIN;
            FlowCookie cookie = FlowUtils.createCookie(fid);
            List<VtnFlowEntry> entries = new ArrayList<>(flowEntries.size());
            for (VtnFlowEntryBuilder flow: flowEntries) {
                entries.add(flow.setCookie(cookie).setOrder(order).build());
                order++;
            }

            // Configure VTN data flow.
            flowBuilder.setFlowId(fid).
                setVirtualRoute(FlowUtils.toVirtualRouteList(virtualRoute)).
                setVtnFlowEntry(entries);
        }

        // Update the creation time.
        Long created = Long.valueOf(System.currentTimeMillis());
        dataFlow = flowBuilder.setCreationTime(created).build();

        return dataFlow;
    }

    /**
     * Return a VTN data flow instance created by the last call of
     * {@link #createVtnDataFlow(VtnFlowId)}.
     *
     * @return  A {@link VtnDataFlow} or {@code null}.
     */
    public VtnDataFlow getDataFlow() {
        return dataFlow;
    }

    /**
     * Create a new flow entry builder.
     *
     * @param ingress  A {@link SalPort} instance which specifies the ingress
     *                 switch port.
     * @param actions  A {@link VTNActionList} instance which specifies flow
     *                 actions.
     * @return  A {@link VtnFlowEntryBuilder} instance.
     */
    private VtnFlowEntryBuilder createFlowEntry(SalPort ingress,
                                                VTNActionList actions) {
        Match match = matchBuilder.
            setInPort(ingress.getNodeConnectorId()).build();
        SalNode snode = ingress.getSalNode();
        NodeId node = snode.getNodeId();
        VtnFlowEntryBuilder builder = new VtnFlowEntryBuilder().
            setNode(node).
            setPriority(priority).
            setTableId(Short.valueOf(FlowUtils.TABLE_ID)).
            setIdleTimeout(idleTimeout).
            setHardTimeout(hardTimeout).
            setMatch(match).
            setFlags(FLOW_MOD_FLAGS).
            setInstructions(actions.toInstructions());

        if (ingressMatchKey == null) {
            ingressMatchKey = vtnMatch.getFlowKey(snode, priority, ingress);
            L2Host src = FlowMatchUtils.getSourceHost(match);
            if (src != null) {
                sourceHost = src.getHost().getSourceHostFlowsKey();
            }
        }

        flowEntries.add(builder);
        flowPorts.add(ingress);
        if (!flowNodes.add(snode)) {
            // This should never happen.
            throw new IllegalStateException("Flow loop detected: " + snode);
        }

        // Flow timeout should be configured only in the ingress flow entry.
        idleTimeout = INTER_FLOW_TIMEOUT;
        hardTimeout = INTER_FLOW_TIMEOUT;

        return builder;
    }

    /**
     * Append an empty virtual node route to the virtual route path.
     *
     * @param lastIndex  The index of the last element in the virtual route
     *                   path.
     */
    private void addEmptyVNodeRoute(int lastIndex) {
        if (lastIndex < 0 || virtualRoute.get(lastIndex).getPath() != null) {
            virtualRoute.add(new VNodeRoute());
        }
    }

    // VTNDataFlow

    /**
     * Return the key assocaited with the VTN data flow in the MD-SAL
     * datastore.
     *
     * <p>
     *   Note that this method must be called after the data flow is created
     *   by {@link #createVtnDataFlow(VtnFlowId)}.
     * </p>
     *
     * @return  A {@link VtnDataFlowKey} instance.
     */
    @Override
    public VtnDataFlowKey getKey() {
        return dataFlow.getKey();
    }

    /**
     * Return a string which represents the condition configured in the ingress
     * flow entry.
     *
     * <p>
     *   Note that this method needs to be called after all flow entries are
     *   configured.
     * </p>
     *
     * @return  A string which representse the ingress match condition.
     */
    @Override
    public String getIngressMatchKey() {
        return ingressMatchKey;
    }

    /**
     * Return a set of switches related to this data flow.
     *
     * <ul>
     *   <li>Note that this method returns a read-only set.</li>
     *   <li>
     *     Note that this method needs to be called after all flow entries are
     *     configured.
     *   </li>
     * </ul>
     *
     * @return  A set of {@link SalNode} instances.
     */
    @Override
    public Set<SalNode> getFlowNodes() {
        return Collections.unmodifiableSet(flowNodes);
    }

    /**
     * Return a set of switch ports related to this data flow.
     *
     * <ul>
     *   <li>Note that this method returns a read-only set.</li>
     *   <li>
     *     Note that this method needs to be called after all flow entries are
     *     configured.
     *   </li>
     * </ul>
     *
     * @return  A set of {@link SalPort} instances.
     */
    @Override
    public Set<SalPort> getFlowPorts() {
        return Collections.unmodifiableSet(flowPorts);
    }

    /**
     * Return the source L2 host of this data flow.
     *
     * <p>
     *   Note that this method needs to be called after all flow entries are
     *   configured.
     * </p>
     *
     * @return  A {@link SourceHostFlowsKey} instance which specifies the
     *          source L2 host. {@code null} if the source L2 host could not
     *          be determined.
     */
    @Override
    public SourceHostFlowsKey getSourceHostFlowsKey() {
        return sourceHost;
    }
}
