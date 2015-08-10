/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which removes
 * VTN data flows related to the specified edge network on a switch port.
 *
 * <p>
 *   Edge network is specified by a pair of {@link SalPort} instance and a
 *   VLAN ID.
 *   A {@link SalPort} instance specifies the target physical switch port.
 *   This class scans all VTN data flows related to the specified port,
 *   and removes data flows if the ingress or egress flow entry meets all
 *   the following conditions.
 * </p>
 * <ul>
 *   <li>
 *     The given VLAN ID is configured in the flow entry.
 *   </li>
 *   <li>
 *     The given switch port is configured as the ingress or the egress
 *     switch port.
 *   </li>
 * </ul>
 */
public final class EdgePortFlowRemover extends PortIndexFlowRemover {
    /**
     * The VLAN ID.
     */
    private final int  vlanId;

    /**
     * Construct a new instance.
     *
     * @param tname   The name of the target VTN.
     * @param sport   A {@link SalPort} instance corresponding to the target
     *                switch port.
     * @param vid     A VLAN ID.
     */
    public EdgePortFlowRemover(String tname, SalPort sport, int vid) {
        super(tname, sport);
        vlanId = vid;
    }

    /**
     * Determine whether the given edge host meets the condition or not.
     *
     * @param l2h  A {@link L2Host} which represents the edge host of the
     *             data flow.
     * @return  {@code true} only if the given edge host meets the condition.
     */
    private boolean match(L2Host l2h) {
        if (l2h != null) {
            MacVlan mv = l2h.getHost();
            return ((int)mv.getVlan() == vlanId &&
                    getFlowPort().equals(l2h.getPort()));
        }

        return false;
    }

    // TenantIndexFlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean select(FlowCache fc) throws VTNException {
        ObjectPair<L2Host, L2Host> edges = fc.getEdgeHosts();
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
        return new StringBuilder("edge-port[port=").append(getFlowPort()).
            append(", vlan=").append(vlanId).append(']').toString();
    }
}
