/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.HashSet;
import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgeNodeFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.ExtendedPortVlanMacFilter;

/**
 * An implementation of {@link MapCleaner} that purges network resources
 * superseded by VLAN mappings.
 */
public final class VlanMapCleaner implements MapCleaner {
    /**
     * A set of {@link NodeVlan} instances which represents VLANs mapped by
     * VLAN mappings.
     */
    private final Set<NodeVlan>  nodeVlans = new HashSet<>();

    /**
     * Add VLAN on the specified switch.
     *
     * @param nvlan  A {@link NodeVlan} instance which represents VLAN mapped
     *               by a VLAN mapping. Note that the specified
     *               {@link NodeVlan} instance must specifies a switch.
     */
    public void add(NodeVlan nvlan) {
        nodeVlans.add(nvlan);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void purge(TxContext ctx, String tname) throws VTNException {
        for (NodeVlan nvlan: nodeVlans) {
            SalNode snode = nvlan.getNode();
            int vid = nvlan.getVlanId();
            VlanMapPortFilter portFilter =
                VlanMapPortFilter.create(ctx, snode, vid, null);

            // Remove MAC addresses detected by VLAN mapping.
            ExtendedPortVlanMacFilter macFilter =
                new ExtendedPortVlanMacFilter(portFilter, vid);
            new MacEntryRemover(macFilter).scan(ctx, tname);

            // Remove flow entries relevant to the specified node and VLAN.
            EdgeNodeFlowRemover fr = new EdgeNodeFlowRemover(
                tname, snode, portFilter, vid);
            ctx.getSpecific(FlowRemoverQueue.class).enqueue(fr);
        }
    }
}
