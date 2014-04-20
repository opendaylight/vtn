/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;

/**
 * An instance of this class purges network resources originated by the
 * virtual node.
 */
public final class PathMapCleaner implements MapCleaner, MacTableEntryFilter {
    /**
     * A {@link VBridgePath} instance which specifies the virtual node.
     */
    private final VBridgePath  nodePath;

    /**
     * Construct a new instance.
     *
     * @param path  A {@link VBridgePath} instance which specifies the virtual
     *              node.
     */
    public PathMapCleaner(VBridgePath path) {
        nodePath = path;
    }

    // MapCleaner

    /**
     * Purge caches originated by the virtual node.
     *
     * @param mgr     VTN Manager service.
     * @param unused  Not used. Tenant name configured in this instance
     *                is used.
     */
    @Override
    public void purge(VTNManagerImpl mgr, String unused) {
        // Remove MAC addresses detected by the specified virtual node.
        // In this case we don't need to scan MAC address tables in the
        // container.
        MacAddressTable table = mgr.getMacAddressTable(nodePath);
        if (table != null) {
            table.flush(this);
        }

        // Remove flow entries originated by the specified virtual node.
        VTNThreadData.removeFlows(mgr, nodePath);
    }

    // MacTableEntryFilter

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean accept(MacTableEntry tent) {
        return nodePath.equals(tent.getEntryId().getMapPath());
    }
}
