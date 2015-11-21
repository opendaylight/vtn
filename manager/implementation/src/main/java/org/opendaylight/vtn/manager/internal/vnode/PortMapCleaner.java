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
import org.opendaylight.vtn.manager.internal.flow.remove.EdgePortFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.PortVlanMacFilter;

/**
 * An implementation of {@link MapCleaner} that purges network resources
 * superseded by port mappings.
 */
public final class PortMapCleaner implements MapCleaner {
    /**
     * A set of {@link PortVlan} instances which represents VLANs mapped by
     * port mappings.
     */
    private final Set<PortVlan>  portVlans = new HashSet<>();

    /**
     * Add the specified network.
     *
     * @param pvlan  A {@link PortVlan} instance which represents VLAN mapped
     *               by a port mapping.
     */
    public void add(PortVlan pvlan) {
        portVlans.add(pvlan);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void purge(TxContext ctx, String tname) throws VTNException {
        for (PortVlan pvlan: portVlans) {
            SalPort sport = pvlan.getPort();
            int vid = pvlan.getVlanId();

            // Remove MAC addresses detected on the specified port.
            PortVlanMacFilter filter = new PortVlanMacFilter(sport, vid);
            new MacEntryRemover(filter).scan(ctx, tname);

            // Remove flow entries relevant to the specified port and VLAN.
            ctx.getSpecific(FlowRemoverQueue.class).
                enqueue(new EdgePortFlowRemover(tname, sport, vid));
        }
    }
}
