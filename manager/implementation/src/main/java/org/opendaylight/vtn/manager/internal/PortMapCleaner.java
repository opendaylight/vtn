/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashSet;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * An instance of this class purges network resources superseded by
 * port mappings.
 */
public final class PortMapCleaner implements MapCleaner {
    /**
     * A set of {@link PortVlan} instances which represents VLAN networks
     * mapped by port mappings.
     */
    private final Set<PortVlan>  portVlans = new HashSet<PortVlan>();

    /**
     * Add the specified network.
     *
     * @param pvlan  A {@link PortVlan} instance which represents VLAN network
     * mapped by a port mapping.
     */
    public void add(PortVlan pvlan) {
        portVlans.add(pvlan);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void purge(VTNManagerImpl mgr, String tenantName) {
        for (PortVlan pvlan: portVlans) {
            NodeConnector port = pvlan.getNodeConnector();
            short vlan = pvlan.getVlan();

            // Remove MAC addresses detected on the specified port.
            for (MacAddressTable table: mgr.getMacAddressTables()) {
                table.flush(port, vlan);
            }

            // Remove flow entries relevant to the specified port and VLAN.
            VTNThreadData.removeFlows(mgr, tenantName, port, vlan);
        }
    }
}
