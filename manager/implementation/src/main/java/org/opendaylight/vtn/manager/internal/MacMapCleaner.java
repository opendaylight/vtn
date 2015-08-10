/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.cluster.MacMappedHostPath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantScanFlowRemover;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeUtils;

import org.opendaylight.controller.sal.core.NodeConnector;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRoute;

/**
 * An instance of this class purges network resources superseded by a
 * MAC mapping.
 */
public final class MacMapCleaner
    implements MapCleaner, MacTableEntryFilter {
    /**
     * A global resource manager instance.
     */
    private final IVTNResourceManager  resourceManager;

    /**
     * A reference to the target MAC mapping.
     */
    private final MapReference  macMap;

    /**
     * A set of {@link MacVlan} instances which represets L2 hosts newly added
     * to a MAC mapping.
     */
    private final Set<MacVlan>  mappedHosts = new HashSet<MacVlan>();

    /**
     * A set of {@link MacVlan} instances which represets L2 hosts removed
     * from a MAC mapping.
     */
    private final Set<MacVlan>  unmappedHosts = new HashSet<MacVlan>();

    /**
     * A set of VLAN IDs newly added to a MAC mapping.
     */
    private final Set<Short>  mappedVlans = new HashSet<Short>();

    /**
     * A set of VLAN IDs removed from a MAC mapping.
     */
    private final Set<Short>  unmappedVlans = new HashSet<Short>();

    /**
     * A map which keeps L2 hosts mapped by the target MAC mapping.
     */
    private Map<MacVlan, Boolean>  hostMapping;

    /**
     * An implementation of {@link FlowRemover} which removes VTN data flows
     * to be removed for the MAC mapping.
     */
    private final class MacMapFlowRemover extends TenantScanFlowRemover {
        /**
         * Construct a new instance.
         *
         * @param tname  The name of the target VTN.
         */
        private MacMapFlowRemover(String tname) {
            super(tname);
        }

        // ScanFlowRemover

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean select(FlowCache fc) throws VTNException {
            ObjectPair<L2Host, L2Host> hosts = fc.getEdgeHosts();
            if (hosts == null) {
                return false;
            }

            // Check ingress mapping.
            L2Host in = hosts.getLeft();
            MacMapCleaner cl = MacMapCleaner.this;
            if (cl.checkHost(in.getHost(), in.getPort().getAdNodeConnector(),
                             fc.getIngressPath())) {
                return true;
            }

            // Check egress mapping.
            L2Host out = hosts.getRight();
            if (out != null &&
                cl.checkHost(out.getHost(), out.getPort().getAdNodeConnector(),
                             fc.getEgressPath())) {
                return true;
            }

            // Check redirections made by flow filters.
            List<VirtualRoute> vroutes = fc.getVirtualRoute();
            for (int i = 1; i < vroutes.size() - 1; i++) {
                VirtualRoute vr = vroutes.get(i);
                VNodePath path = VNodeUtils.
                    toVNodePath(vr.getVirtualNodePath());
                if (path instanceof MacMappedHostPath) {
                    MacMappedHostPath mpath = (MacMappedHostPath)path;
                    if (cl.checkHost(mpath.getMappedHost(), mpath)) {
                        return true;
                    }
                }
            }

            return false;
        }

        // FlowRemover

        /**
         * {@inheritDoc}
         */
        @Override
        public String getDescription() {
            return new StringBuilder("mac-map:").
                append(MacMapCleaner.this.macMap.toString()).
                toString();
        }
    }

    /**
     * Construct a new instance.
     *
     * @param resMgr  A resource manager instance.
     * @param ref     A reference to a MAC mapping.
     */
    public MacMapCleaner(IVTNResourceManager resMgr, MapReference ref) {
        resourceManager = resMgr;
        macMap = ref;
    }

    /**
     * Add a L2 host to be added to the target MAC mapping.
     *
     * @param mvlan  A {@link MacVlan} instance which represents a L2 host.
     */
    public void addMappedHost(MacVlan mvlan) {
        Short vlan = Short.valueOf(mvlan.getVlan());
        if (mvlan.getMacAddress() == MacVlan.UNDEFINED) {
            mappedVlans.add(vlan);
        } else {
            mappedHosts.add(mvlan);
        }
    }

    /**
     * Add a L2 host to be removed from the target MAC mapping.
     *
     * @param mvlan  A {@link MacVlan} instance which represents a L2 host.
     */
    public void addUnmappedHost(MacVlan mvlan) {
        Short vlan = Short.valueOf(mvlan.getVlan());
        if (mvlan.getMacAddress() == MacVlan.UNDEFINED) {
            unmappedVlans.add(vlan);
        } else {
            unmappedHosts.add(mvlan);
        }
    }

    /**
     * Determine whether network resources corresponding to the specified
     * host and switch port should be purged or not.
     *
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @param port   A {@link NodeConnector} instance corresponding to a
     *               switch port to which the host is connected.
     * @param mpath  A path to virtual node which maps the specified host.
     * @return  {@code true} is returned only if network resources
     *          corresponding to the specified host should be purged.
     */
    private boolean checkHost(MacVlan mvlan, NodeConnector port,
                              VNodePath mpath) {
        PortVlan pvlan = new PortVlan(port, mvlan.getVlan());
        MapReference ref = resourceManager.getMapReference(pvlan);
        if (ref != null && ref.getMapType() == MapType.PORT) {
            // Resources cached by a port mapping should never be changed.
            return false;
        }

        return checkHost(mvlan, mpath);
    }

    /**
     * Determine whether network resources corresponding to the specified
     * host should be purged or not.
     *
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @param mpath  A path to virtual node which maps the specified host.
     * @return  {@code true} is returned only if network resources
     *          corresponding to the specified host should be purged.
     */
    private boolean checkHost(MacVlan mvlan, VNodePath mpath) {
        // Check hosts newly added to the MAC mapping.
        Short vid = Short.valueOf(mvlan.getVlan());
        if (mappedHosts.contains(mvlan)) {
            return true;
        }

        if (mappedVlans.contains(vid) && isMapped(mvlan)) {
            return true;
        }

        // Check hosts removed from the MAC mapping.
        if (macMap.getPath().contains(mpath)) {
            if (unmappedHosts.contains(mvlan)) {
                return true;
            }
            if (unmappedVlans.contains(vid) && !isMapped(mvlan)) {
                return true;
            }
        }

        return false;
    }

    // MapCleaner

    /**
     * {@inheritDoc}
     */
    @Override
    public void purge(VTNManagerImpl mgr, String tenantName) {
        // Remove MAC addresses detected by MAC mapping.
        for (MacAddressTable table: mgr.getMacAddressTables()) {
            table.flush(this);
        }

        // Remove obsolete flow entries, and flow entries superseded by
        // MAC mapping.
        VTNThreadData.removeFlows(mgr, new MacMapFlowRemover(tenantName));
    }

    /**
     * Determine whether the specified L2 host is mapped by the target
     * MAC mapping or not.
     *
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @return  {@code true} is returned if the specified host is mapped by
     *          the target MAC mapping. Otherwise {@code false} is returned.
     *          host.
     *          {@code null} is returned if not found.
     */
    private boolean isMapped(MacVlan mvlan) {
        Map<MacVlan, Boolean> mapping = hostMapping;
        if (mapping == null) {
            mapping = new HashMap<MacVlan, Boolean>();
            hostMapping = mapping;
        }

        // At first, test cached result.
        Boolean ret = mapping.get(mvlan);
        if (ret != null) {
            return ret.booleanValue();
        }

        MapReference ref = resourceManager.getMapReference(mvlan);
        boolean mapped = macMap.equals(ref);
        mapping.put(mvlan, Boolean.valueOf(mapped));
        return mapped;
    }

    // MacTableEntryFilter

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean accept(MacTableEntry tent) {
        long mac = tent.getMacAddress();
        short vlan = tent.getVlan();
        NodeConnector port = tent.getPort();
        VNodePath path = tent.getEntryId().getMapPath();

        return checkHost(new MacVlan(mac, vlan), port, path);
    }
}
