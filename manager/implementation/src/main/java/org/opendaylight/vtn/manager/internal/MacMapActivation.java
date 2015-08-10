/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * An instance of {@code MacMapActivation} class represents the result of
 * MAC mapping activation.
 */
public final class MacMapActivation {
    /**
     * A {@link NodeConnector} instance that was previously associated with
     * the L2 host in the MAC mapping.
     */
    private final NodeConnector  oldPort;

    /**
     * A boolean value which incaites whether the MAC mapping was activated
     * or not.
     */
    private final boolean  activated;

    /**
     * A {@link PortVlan} instance which represents the VLAN network
     * to be released.
     */
    private final PortVlan  releasedNetwork;

    /**
     * A {@link PortVlan} instance which represents the VLAN network
     * reserved by the MAC mapping.
     */
    private PortVlan  reservedNetwork;

    /**
     * A reference to the VLAN mapping which maps the VLAN network
     * specified by {@link #reservedNetwork}.
     */
    private MapReference  vlanMap;

    /**
     * Construct a new instance.
     *
     * @param old        A {@link NodeConnector} instance which was previously
     *                   associated with the host in the MAC mapping.
     * @param released   A {@link PortVlan} instance which represents the
     *                   VLAN network to be released.
     * @param activated  {@code true} means that the target MAC mapping has
     *                   been activated.
     */
    public MacMapActivation(NodeConnector old, PortVlan released,
                            boolean activated) {
        this.oldPort = old;
        this.releasedNetwork = released;
        this.activated = activated;
    }

    /**
     * Set VLAN mapping superseded by this MAC mapping.
     *
     * @param ref    A reference to the VLAN mapping superseded by this
     *               MAC mapping.
     *               Specifying {@code null} results in undefined behavior.
     * @param pvlan  A {@link PortVlan} instance which represents the
     *               VLAN network reserved by the MAC mapping.
     *               Specifying {@code null} results in undefined behavior.
     */
    public void setObsoleteVlanMap(MapReference ref, PortVlan pvlan) {
        vlanMap = ref;
        reservedNetwork = pvlan;
    }

    /**
     * Return a {@link NodeConnector} instance which was previously associated
     * with the L2 host in the MAC mapping.
     *
     * @return  A {@link NodeConnector} instance.
     *          {@code null} is returned if there was no mapping for the
     *          L2 host.
     */
    public NodeConnector getOldPort() {
        return oldPort;
    }

    /**
     * Return a {@link PortVlan} instance which represents the VLAN network
     * to be released.
     *
     * @return  A {@link PortVlan} instance is returned if the caller needs
     *          to release the VLAN network on a switch port as a result of
     *          activation.
     *          {@code null} is returned if no VLAN network need to be
     *          released.
     */
    public PortVlan getReleasedNetwork() {
        return releasedNetwork;
    }

    /**
     * Determine whether the MAC mapping has been activated or not.
     *
     * @return  {@code true} is returned only if the MAC mapping has been
     *          activated.
     *          {@code false} is returned if the MAC mapping is already active.
     */
    public boolean isActivated() {
        return activated;
    }

    /**
     * Clean up transaction of MAC mapping activation.
     *
     * @param log    A {@link Logger} instance.
     * @param mgr    A VTN Manager service which manages the target
     *               MAC mapping.
     * @param ref    A reference to the target MAC mapping.
     * @param mvlan  A {@link MacVlan} instance which specifies the host
     *               mapped by the MAC mapping.
     * @param port   A {@link NodeConnector} instance corresponding to
     *               a switch port where the host was detected.
     */
    public void cleanUp(Logger log, VTNManagerImpl mgr, MapReference ref,
                        MacVlan mvlan, NodeConnector port) {
        VBridgePath path = (VBridgePath)ref.getPath();
        if (vlanMap != null) {
            // Purge obsolete network caches.
            purge(mgr, path);
        }

        if (oldPort == null) {
            if (log.isTraceEnabled()) {
                StringBuilder builder = new StringBuilder("host={");
                mvlan.appendContents(builder);
                builder.append("}, port=").append(port.toString());
                log.trace("{}:{}: MAC mapping has been activated: {}",
                          mgr.getContainerName(), path, builder.toString());
            }
            return;
        }

        if (log.isTraceEnabled()) {
            StringBuilder builder = new StringBuilder("host={");
            mvlan.appendContents(builder);
            builder.append("}, from=").append(oldPort.toString()).
                append(",to=").append(port.toString());
            log.trace("{}:{}: MAC mapped host has been moved: {}",
                      mgr.getContainerName(), path, builder.toString());
        }

        // Remove MAC addresses registered by old MAC mapping.
        MacAddressTable table = mgr.getMacAddressTable(path);
        if (table != null) {
            Long mac = Long.valueOf(mvlan.getMacAddress());
            table.remove(mac);
        }

        // Remove flow entries registered by old MAC mapping.
        VTNThreadData.removeFlows(mgr, path.getTenantName(), mvlan, oldPort);
    }

    /**
     * Purge network caches superseded by this MAC mapping.
     *
     * @param mgr    VTN Manager service.
     * @param mpath  A path to the target MAC mapping.
     */
    private void purge(VTNManagerImpl mgr, VNodePath mpath) {
        String container = mgr.getContainerName();
        String tenant = mpath.getTenantName();

        // Use PortMapCleaner to purge network caches.
        PortMapCleaner cleaner = new PortMapCleaner();
        cleaner.add(reservedNetwork);

        VNodePath vpath = vlanMap.getPath();
        String cname = vlanMap.getContainerName();
        String tname = vpath.getTenantName();
        if (container.equals(cname) && tenant.equals(tname)) {
            // Network caches in the target tenant can be purged immediately
            // because this method is being called from PACKET_IN handler with
            // holding the manager lock.
            cleaner.purge(mgr, tenant);
        } else {
            // Purge network caches asynchronously.
            IVTNResourceManager resMgr = mgr.getResourceManager();
            VTNManagerImpl cmgr = resMgr.getVTNManager(cname);
            if (cmgr != null) {
                cmgr.purge(cleaner, tname);
            }
        }
    }
}
