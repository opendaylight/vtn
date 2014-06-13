/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * An instance of this class purges network resources superseded by a
 * MAC mapping.
 */
public final class MacMapCleaner
    implements MapCleaner, VTNFlowMatch, MacTableEntryFilter {
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
     * host should be purged or not.
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
        short vlan = mvlan.getVlan();
        PortVlan pvlan = new PortVlan(port, vlan);
        MapReference ref = resourceManager.getMapReference(pvlan);
        if (ref != null && ref.getMapType() == MapType.PORT) {
            // Resources cached by a port mapping should never be changed.
            return false;
        }

        // Check hosts newly added to the MAC mapping.
        Short vid = Short.valueOf(vlan);
        if (mappedHosts.contains(mvlan)) {
            return true;
        }

        if (mappedVlans.contains(vid) && isMapped(mvlan)) {
            return true;
        }

        // Check hosts removed from the MAC mapping.
        if (macMap.getPath().equals(mpath)) {
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
        VTNThreadData.removeFlows(mgr, tenantName, this);
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

    // VTNFlowMatch

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean accept(VTNFlow vflow) {
        ObjectPair<L2Host, L2Host> hosts = vflow.getEdgeHosts();
        if (hosts == null) {
            return false;
        }

        L2Host in = hosts.getLeft();
        if (checkHost(in.getHost(), in.getPort(), vflow.getIngressPath())) {
            return true;
        }

        L2Host out = hosts.getRight();
        if (out != null) {
            return checkHost(out.getHost(), out.getPort(),
                             vflow.getEgressPath());
        }

        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return macMap.toString();
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
