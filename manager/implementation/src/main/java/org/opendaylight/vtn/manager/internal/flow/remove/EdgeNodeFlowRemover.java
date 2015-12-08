/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import org.apache.commons.lang3.tuple.Pair;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.L2Host;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortFilter;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableKey;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which removes
 * VTN data flows related to the specified edge network on a switch.
 *
 * <p>
 *   Edge network is specified by a {@link SalNode} instance,
 *   a {@link PortFilter} instance, and a VLAN ID.
 *   A {@link SalNode} instance specifies the target physical switch.
 *   This class scans all VTN data flows related to the specified switch,
 *   and removes data flows if the ingress or egress flow entry meets all
 *   the following conditions.
 * </p>
 * <ul>
 *   <li>
 *     The given VLAN ID is configured in the flow entry.
 *   </li>
 *   <li>
 *     The ingress or egress switch port is accepted by the given
 *     {@link PortFilter} instance.
 *   </li>
 * </ul>
 * <p>
 *   Note that this method always passes a {@code null} as a {@code VtnPort} to
 *   {@link PortFilter}.
 * </p>
 */
public final class EdgeNodeFlowRemover
    extends TenantIndexFlowRemover<NodeFlows> {
    /**
     * The target physical switch.
     */
    private final SalNode  flowNode;

    /**
     * A {@link PortFilter} which specifies the switch port.
     */
    private final PortFilter  portFilter;

    /**
     * The VLAN ID.
     */
    private final int  vlanId;

    /**
     * The key which specifies the target node in the node flow index.
     */
    private final NodeFlowsKey  nodeKey;

    /**
     * Construct a new instance.
     *
     * @param tname   The name of the target VTN.
     * @param snode   A {@link SalNode} instance corresponding to the target
     *                switch.
     * @param filter  A {@link PortFilter} instance which selects switch ports.
     * @param vid     A VLAN ID.
     */
    public EdgeNodeFlowRemover(String tname, SalNode snode, PortFilter filter,
                               int vid) {
        super(tname);
        flowNode = snode;
        portFilter = filter;
        vlanId = vid;
        nodeKey = new NodeFlowsKey(snode.getNodeId());
    }

    /**
     * Determine whether the given edge host meets the condition or not.
     *
     * @param l2h  A {@link L2Host} which represents the edge host of the
     *             data flow.
     * @return  {@code true} only if the given edge host meets the condition.
     * @throws VTNException  An error occurred.
     */
    private boolean match(L2Host l2h) throws VTNException {
        if (l2h != null) {
            MacVlan mv = l2h.getHost();
            if (mv.getVlanId() == vlanId) {
                return portFilter.accept(l2h.getPort(), null);
            }
        }

        return false;
    }

    // TenantIndexFlowRemover

    /**
     * Return the path to the node flow index.
     *
     * @param key  A {@link VtnFlowTableKey} instance which specifies the
     *             target VTN flow table.
     * @return  Path to the node flow index.
     */
    @Override
    protected InstanceIdentifier<NodeFlows> getPath(VtnFlowTableKey key) {
        return FlowUtils.getIdentifier(key, nodeKey);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean select(FlowCache fc) throws VTNException {
        Pair<L2Host, L2Host> edges = fc.getEdgeHosts();
        return (edges == null)
            ? false
            : (match(edges.getLeft()) || match(edges.getRight()));
    }

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return new StringBuilder("edge-node[node=").append(flowNode).
            append(", vlan=").append(vlanId).append("filter=").
            append(portFilter).append(']').toString();
    }
}
