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

import org.opendaylight.vtn.manager.internal.cluster.NodeVlan;

import org.opendaylight.controller.sal.core.Node;

/**
 * An instance of this class purges network resources superseded by
 * VLAN mappings.
 */
public final class VlanMapCleaner implements MapCleaner {
    /**
     * A set of {@link NodeVlan} instances which represents VLAN networks
     * mapped by VLAN mappings.
     */
    private final Set<NodeVlan>  nodeVlans = new HashSet<NodeVlan>();

    /**
     * Add VLAN network on the specified switch.
     *
     * @param nvlan  A {@link NodeVlan} instance which represents VLAN network
     *               mapped by a VLAN mapping. Note that the specified
     *               {@link NodeVlan} instance must have a non-{@code null}
     *               {@link Node} instance.
     */
    public void add(NodeVlan nvlan) {
        nodeVlans.add(nvlan);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void purge(VTNManagerImpl mgr, String tenantName) {
        IVTNResourceManager resMgr = mgr.getResourceManager();
        for (NodeVlan nvlan: nodeVlans) {
            Node node = nvlan.getNode();
            short vlan = nvlan.getVlan();
            VlanMapPortFilter filter =
                VlanMapPortFilter.create(resMgr, node, vlan, null);

            // Remove MAC addresses detected by VLAN mapping.
            for (MacAddressTable table: mgr.getMacAddressTables()) {
                table.flush(filter, vlan);
            }

            // Remove flow entries relevant to the specified node and VLAN.
            VTNThreadData.removeFlows(mgr, tenantName, node, filter, vlan);
        }
    }
}
