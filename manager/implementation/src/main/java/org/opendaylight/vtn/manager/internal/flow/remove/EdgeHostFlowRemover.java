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
 * VTN data flows related to the specified layer 2 host.
 *
 * <p>
 *   The layer 2 host is specified by a pair of {@link MacVlan} and
 *   {@link SalPort} instances.
 *   A {@link SalPort} instance specifies the target physical switch port.
 *   This class scans all VTN data flows related to the specified port,
 *   and removes data flows if the ingress or egress flow entry matches the
 *   given {@link MacVlan} instance.
 * </p>
 */
public final class EdgeHostFlowRemover extends PortIndexFlowRemover {
    /**
     * A pair of MAC address and a VLAN ID.
     */
    private final MacVlan  macVlan;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the target VTN.
     * @param host   A {@link L2Host} instance which indicates the layer 2
     *               host.
     */
    public EdgeHostFlowRemover(String tname, L2Host host) {
        super(tname, host.getPort());
        macVlan = host.getHost();
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the target VTN.
     * @param mvlan  A {@link MacVlan} instance.
     * @param sport  A {@link SalPort} instance corresponding to the physical
     *               switch port.
     */
    public EdgeHostFlowRemover(String tname, MacVlan mvlan, SalPort sport) {
        super(tname, sport);
        macVlan = mvlan;
    }

    /**
     * Determine whether the given edge host meets the condition or not.
     *
     * @param l2h  A {@link L2Host} which represents the edge host of the
     *             data flow.
     * @return  {@code true} only if the given edge host meets the condition.
     */
    private boolean match(L2Host l2h) {
        return (l2h == null)
            ? false
            : macVlan.equals(l2h.getHost());
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
        return new StringBuilder("edge-host[port=").append(getFlowPort()).
            append(", mvlan=").append(macVlan).append(']').toString();
    }
}
