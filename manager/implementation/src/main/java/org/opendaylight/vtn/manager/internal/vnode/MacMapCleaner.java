/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang3.tuple.Pair;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantScanFlowRemover;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.inventory.L2Host;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapHostIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.MacEntryFilter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRoute;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

/**
 * An implementation of {@link MapCleaner} that purges network resources
 * superseded by a MAC mapping.
 */
public final class MacMapCleaner implements MapCleaner, MacEntryFilter {
    /**
     * Context for MD-SAL datastore transaction.
     */
    private final TxContext  context;

    /**
     * The identifier for the target MAC mapping.
     */
    private final MacMapIdentifier  macMapId;

    /**
     * A set of {@link MacVlan} instances which represets L2 hosts newly added
     * to a MAC mapping.
     */
    private final Set<MacVlan>  mappedHosts = new HashSet<>();

    /**
     * A set of {@link MacVlan} instances which represets L2 hosts removed
     * from a MAC mapping.
     */
    private final Set<MacVlan>  unmappedHosts = new HashSet<>();

    /**
     * A set of VLAN IDs newly added to a MAC mapping.
     */
    private final Set<Integer>  mappedVlans = new HashSet<>();

    /**
     * A set of VLAN IDs removed from a MAC mapping.
     */
    private final Set<Integer>  unmappedVlans = new HashSet<>();

    /**
     * A map which keeps L2 hosts mapped by the target MAC mapping.
     */
    private Map<MacVlan, Boolean>  hostMapping;

    /**
     * An implementation of
     * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which removes
     * VTN data flows to be removed for the MAC mapping.
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
            Pair<L2Host, L2Host> hosts = fc.getEdgeHosts();
            if (hosts == null) {
                return false;
            }

            // Check ingress mapping.
            L2Host in = hosts.getLeft();
            MacMapCleaner cl = MacMapCleaner.this;
            if (cl.checkHost(new MacVlan(in.getHost().getEncodedValue()),
                             in.getPort(),
                             fc.getIngressPath())) {
                return true;
            }

            // Check egress mapping.
            L2Host out = hosts.getRight();
            if (out != null &&
                cl.checkHost(new MacVlan(out.getHost().getEncodedValue()),
                             out.getPort(), fc.getEgressPath())) {
                return true;
            }

            // Check redirections made by flow filters.
            return cl.checkRedirection(fc);
        }

        // FlowRemover

        /**
         * {@inheritDoc}
         */
        @Override
        public String getDescription() {
            return MacMapCleaner.this.macMapId.toString();
        }
    }

    /**
     * Construct a new instance.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the target MAC mapping.
     */
    public MacMapCleaner(TxContext ctx, MacMapIdentifier ident) {
        context = ctx;
        macMapId = ident;
    }

    /**
     * Add a L2 host to be added to the target MAC mapping.
     *
     * @param mvlan  A {@link MacVlan} instance which represents a L2 host.
     */
    public void addMappedHost(MacVlan mvlan) {
        if (mvlan.getAddress() == MacVlan.UNDEFINED) {
            mappedVlans.add(mvlan.getVlanId());
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
        if (mvlan.getAddress() == MacVlan.UNDEFINED) {
            unmappedVlans.add(mvlan.getVlanId());
        } else {
            unmappedHosts.add(mvlan);
        }
    }

    /**
     * Determine whether network resources corresponding to the specified host
     * and switch port should be purged or not.
     *
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @param sport  A {@link SalPort} instance corresponding to a switch port
     *               to which the host is connected.
     * @param vpath  The path to the virtual mapping which maps the specified
     *               host.
     * @return  {@code true} only if network resources corresponding to the
     *          specified host should be purged.
     * @throws VTNException  An error occurred.
     */
    private boolean checkHost(MacVlan mvlan, SalPort sport,
                              VirtualNodePath vpath) throws VTNException {
        PortVlan pvlan = new PortVlan(sport, mvlan.getVlanId());
        VNodeIdentifier<?> pident = MappingRegistry.getMapping(context, pvlan);
        boolean result;
        if (pident != null && pident.getType().isInterface()) {
            // Resources cached by a port mapping should never be changed.
            result = false;
        } else {
            result = checkHost(mvlan, vpath);
        }

        return result;
    }

    /**
     * Determine whether network resources corresponding to the specified
     * host should be purged or not.
     *
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @param vpath  The path to the virtual mapping which maps the specified
     *               host.
     * @return  {@code true} only if network resources corresponding to the
     *          specified host should be purged.
     * @throws VTNException  An error occurred.
     */
    private boolean checkHost(MacVlan mvlan, VirtualNodePath vpath)
        throws VTNException {
        // Check hosts newly added to the MAC mapping.
        Integer vid = Integer.valueOf(mvlan.getVlanId());
        boolean result = (mappedHosts.contains(mvlan) ||
                          (mappedVlans.contains(vid) && isMapped(mvlan)));

        // Check hosts removed from the MAC mapping.
        if (!result && macMapId.contains(vpath)) {
            result = (unmappedHosts.contains(mvlan) ||
                      (unmappedVlans.contains(vid) && !isMapped(mvlan)));
        }

        return result;
    }

    // MapCleaner

    /**
     * {@inheritDoc}
     */
    @Override
    public void purge(TxContext ctx, String tname) throws VTNException {
        // Remove MAC addresses detected by MAC mapping.
        new MacEntryRemover(this).scan(ctx, tname);

        // Remove obsolete flow entries, and flow entries superseded by
        // MAC mapping.
        ctx.getSpecific(FlowRemoverQueue.class).
            enqueue(new MacMapFlowRemover(tname));
    }

    /**
     * Determine whether the specified L2 host is mapped by the target
     * MAC mapping or not.
     *
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @return  {@code true} if the specified host is mapped by the target
     *          MAC mapping. Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    private boolean isMapped(MacVlan mvlan) throws VTNException {
        Map<MacVlan, Boolean> mapping = hostMapping;
        if (mapping == null) {
            mapping = new HashMap<>();
            hostMapping = mapping;
        }

        // At first, test cached result.
        boolean mapped;
        Boolean ret = mapping.get(mvlan);
        if (ret == null) {
            MacMapIdentifier mapId =
                MappingRegistry.getMapping(context, mvlan);
            mapped = macMapId.equals(mapId);
            mapping.put(mvlan, Boolean.valueOf(mapped));
        } else {
            mapped = ret.booleanValue();
        }

        return mapped;
    }

    /**
     * Determine whether the given VTN data flow is redirected by obsolete
     * MAC mapping or not.
     *
     * @param fc  A {@link FlowCache} instance that contains information
     *            about a VTN flow.
     * @return  {@code true} only if the given VTN flow is redirected
     *          by obsolte MAC mapping.
     * @throws VTNException  An error occurred.
     */
    private boolean checkRedirection(FlowCache fc) throws VTNException {
        List<VirtualRoute> vroutes = fc.getVirtualRoute();
        for (int i = 1; i < vroutes.size() - 1; i++) {
            VirtualNodePath vpath = vroutes.get(i).getVirtualNodePath();
            MacVlan mapped = MacMapHostIdentifier.getMappedHost(vpath);
            if (mapped != null && checkHost(mapped, vpath)) {
                return true;
            }
        }

        return false;
    }

    // MacEntryFilter

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean accept(MacTableEntry ment) throws VTNException {
        int vid = ment.getVlanId().intValue();
        MacVlan mv = new MacVlan(ment.getMacAddress(), vid);
        SalPort sport = SalPort.create(ment);
        VNodeIdentifier<?> ident = VNodeIdentifier.create(ment.getEntryData());

        return checkHost(mv, sport, ident.getVirtualNodePath());
    }
}
