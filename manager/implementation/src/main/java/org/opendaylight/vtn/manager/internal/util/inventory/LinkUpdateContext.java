/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.google.common.base.Optional;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.FixedLogger;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * {@code LinkUpdateContext} describes runtime information for updating
 * inter-switch links in the MD-SAL datastore.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
public final class LinkUpdateContext {
    /**
     * A MD-SAL datastore transaction.
     */
    private final ReadWriteTransaction  transaction;

    /**
     * An {@link InventoryReader} instance.
     */
    private final InventoryReader  inventoryReader;

    /**
     * A map which keeps links added to ignored-links container.
     * A string which indicates the cause is associated with ignored link.
     */
    private Map<IgnoredLink, String>  ignoredLinks;

    /**
     * A map which keeps links that were not resolved by
     * {@link #resolveIgnoredLinks()}.
     * A string which indicates the cause is associated with ignored link.
     */
    private Map<IgnoredLink, String>  unresolvedLinks;

    /**
     * A list of resolved links, which are moved from ignored-links container
     * from vtn-topology container.
     */
    private List<VtnLink>  resolvedLinks;

    /**
     * A list of VTN links moved to ignored-links container.
     */
    private List<VtnLink>  movedLinks;

    /**
     * A set of switch ports updated as static edge ports.
     */
    private Set<SalPort>  staticEdgePorts;

    /**
     * Construct a new instance.
     *
     * @param tx      A {@link ReadWriteTransaction} instance.
     * @param reader  A {@link InventoryReader} instance.
     */
    public LinkUpdateContext(ReadWriteTransaction tx, InventoryReader reader) {
        transaction = tx;
        inventoryReader = reader;
    }

    /**
     * Add the given inter-switch link information detected by topology-manager
     * into the VTN inventory data.
     *
     * @param lid  The identifier of the created link.
     * @param src  A {@link SalPort} instance corresponding to the source
     *             of the created link.
     * @param dst  A {@link SalPort} instance corresponding to the
     *             destination of the created link.
     * @return  A {@link VtnLink} instance if the given link was added.
     *          {@code null} if not added.
     * @throws VTNException  An error occurred.
     */
    public VtnLink addVtnLink(LinkId lid, SalPort src, SalPort dst)
        throws VTNException {
        String cause = canCreateVtnLink(lid, src, dst);
        if (cause == null) {
            // Create link information.
            return createVtnLink(lid, src, dst, false);
        }

        // Put link information into ignored link list.
        InstanceIdentifier<IgnoredLink> key = InventoryUtils.
            toIgnoredLinkIdentifier(lid);
        IgnoredLink ilink = InventoryUtils.
            toIgnoredLinkBuilder(lid, src, dst).build();
        transaction.put(LogicalDatastoreType.OPERATIONAL, key, ilink, true);

        Map<IgnoredLink, String> map = ignoredLinks;
        if (map == null) {
            map = new HashMap<>();
            ignoredLinks = map;
        }
        map.put(ilink, cause);

        return null;
    }

    /**
     * Try to resolve ignored inter-switch links.
     *
     * @throws VTNException  An error occurred.
     */
    public void resolveIgnoredLinks() throws VTNException {
        // Read all ignored inter-switch links.
        InstanceIdentifier<IgnoredLinks> igPath =
            InstanceIdentifier.create(IgnoredLinks.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<IgnoredLinks> opt =
            DataStoreUtils.read(transaction, oper, igPath);
        if (!opt.isPresent()) {
            return;
        }

        List<IgnoredLink> links = opt.get().getIgnoredLink();
        if (links == null) {
            return;
        }

        List<VtnLink> resolved = new ArrayList<>();
        Map<IgnoredLink, String> unresolved = new HashMap<>();
        for (IgnoredLink ignored: links) {
            LinkId lid = ignored.getLinkId();
            SalPort src = SalPort.create(ignored.getSource());
            SalPort dst = SalPort.create(ignored.getDestination());
            String cause = canCreateVtnLink(lid, src, dst);
            if (cause == null) {
                // Move this link to vtn-topology.
                InstanceIdentifier<IgnoredLink> ipath =
                    InventoryUtils.toIgnoredLinkIdentifier(lid);
                transaction.delete(oper, ipath);
                resolved.add(createVtnLink(lid, src, dst, false));
            } else {
                unresolved.put(ignored, cause);
            }
        }

        if (!resolved.isEmpty()) {
            resolvedLinks = resolved;
        }
        if (!unresolved.isEmpty()) {
            unresolvedLinks = unresolved;
        }
    }

    /**
     * Update static network topology configured to the given switch port.
     *
     * <p>
     *   Note that this method never resolves ignored links even if one or
     *   more static links are removed.
     * </p>
     *
     * @param sport  A {@link SalPort} instance which specifies the target
     *               port.
     * @param vport  A {@link VtnPort} associated with {@code sport}.
     * @throws VTNException  An error occurred.
     */
    public void updateStaticTopology(SalPort sport, VtnPort vport)
        throws VTNException {
        if (!InventoryUtils.isEnabled(vport)) {
            // Remove all links configured in the given VTN port.
            InventoryUtils.clearPortLink(transaction, sport, vport);
            return;
        }

        if (inventoryReader.isStaticEdgePort(sport)) {
            // Update the given port as an edge port statically.
            updateAsStaticEdge(sport, vport);
            return;
        }

        // Determine reverse static inter-switch links in order to cache
        // static link configuration into the inventory reader.
        Set<SalPort> revLinks = inventoryReader.getReverseStaticLinks(sport);

        // Remove obsolete static inter-switch links.
        Set<SalPort> established = removeObsoleteStaticLink(sport, vport);

        // From here, we should not use vport variable because the VTN port
        // associated with vport variable in the MD-SAL DS may be updated by
        // removeObsoleteStaticLink().

        // Try to configure static inter-switch links which point to the
        // given port.
        for (SalPort src: revLinks) {
            if (!established.contains(src)) {
                addStaticLink(src, sport);
            }
        }

        if (!established.contains(sport)) {
            // Try to configure static inter-switch links configured to the
            // given port.
            SalPort dst = inventoryReader.getStaticLink(sport);
            if (dst != null) {
                addStaticLink(sport, dst);
            }
        }
    }

    /**
     * Record logs for updated links.
     *
     * @param log  A {@link Logger} instance.
     */
    public void recordLogs(Logger log) {
        if (staticEdgePorts != null) {
            for (SalPort sport: staticEdgePorts) {
                log.info("{}: Port has been updated as a static edge port.",
                         sport);
            }
        }

        FixedLogger info = new FixedLogger.Info(log);
        recordLogs(info, ignoredLinks, "Ignore inter-switch link");
        recordLogs(info, resolvedLinks, "Inter-switch link has been resolved");
        recordLogs(info, movedLinks,
                   "Inter-switch link has been superseded by static topology");

        if (log.isDebugEnabled()) {
            FixedLogger debug = new FixedLogger.Debug(log);
            recordLogs(debug, unresolvedLinks, "Skip ignored-link");
        }
    }

    /**
     * Return a set of ignored inter-switch links.
     *
     * @return  A set of ignored inter-switch links.
     */
    Set<IgnoredLink> getIgnoredLinks() {
        return MiscUtils.unmodifiableKeySet(ignoredLinks);
    }

    /**
     * Return a set of ignored links that were not resolved.
     *
     * @return  A set of ignored links that were not resolved.
     */
    Set<IgnoredLink> getUnresolvedLinks() {
        return MiscUtils.unmodifiableKeySet(unresolvedLinks);
    }

    /**
     * Return a list of resolved links.
     *
     * @return  A list of resolved links.
     */
    List<VtnLink> getResolvedLinks() {
        return MiscUtils.unmodifiableList(resolvedLinks);
    }

    /**
     * Return a list of VTN links moved to ignored-links container.
     *
     * @return  A list of VTN links moved to ignored-links container.
     */
    List<VtnLink> getMovedLinks() {
        return MiscUtils.unmodifiableList(movedLinks);
    }

    /**
     * Return a set of switch ports updated as static edge ports.
     *
     * @return  A set of {@link SalPort} instances.
     */
    Set<SalPort> getStaticEdgePorts() {
        return MiscUtils.unmodifiableSet(staticEdgePorts);
    }

    /**
     * Create a VTN link information, and put it into the MD-SAL datastore.
     *
     * @param lid  The identifier of the created link.
     * @param src  A {@link SalPort} instance corresponding to the source
     *             of the created link.
     * @param dst  A {@link SalPort} instance corresponding to the destination
     *             of the created link.
     * @param st   {@code true} means the link is configured as static link.
     *             {@code false} means the link was detected by
     *             topology-manager.
     * @return  A newly created {@link VtnLink} instance.
     */
    private VtnLink createVtnLink(LinkId lid, SalPort src, SalPort dst,
                                  boolean st) {
        // Put the link information into vtn-topology list.
        InstanceIdentifier<VtnLink> key =
            InventoryUtils.toVtnLinkIdentifier(lid);
        VtnLinkBuilder builder =
            InventoryUtils.toVtnLinkBuilder(lid, src, dst);
        if (st) {
            builder.setStaticLink(Boolean.TRUE);
        }
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        VtnLink vlink = builder.build();
        transaction.put(oper, key, vlink, true);

        // Create source port link.
        InstanceIdentifier<PortLink> pkey = src.getPortLinkIdentifier(lid);
        PortLink plink = InventoryUtils.toPortLinkBuilder(lid, dst).build();
        transaction.put(oper, pkey, plink, true);

        // Create destination port link.
        pkey = dst.getPortLinkIdentifier(lid);
        plink = InventoryUtils.toPortLinkBuilder(lid, src).build();
        transaction.put(oper, pkey, plink, true);

        return vlink;
    }

    /**
     * Determine whether the given link detected by the topology-manager
     * can be created or not.
     *
     * @param lid  The identifier of the created link.
     * @param src  A {@link SalPort} instance corresponding to the source
     *             of the created link.
     * @param dst  A {@link SalPort} instance corresponding to the destination
     *             of the created link.
     * @return  {@code null} if the given link can be created.
     *          Otherwise a string which indicates the cause is returned.
     * @throws VTNException  An error occurred.
     */
    private String canCreateVtnLink(LinkId lid, SalPort src, SalPort dst)
        throws VTNException {
        if (inventoryReader.isStaticEdgePort(src)) {
            return "Source port is configured as static edge port.";
        }
        if (inventoryReader.isStaticEdgePort(dst)) {
            return "Destination port is configured as static edge port.";
        }
        if (inventoryReader.getStaticLink(src) != null) {
            return "Static inter-switch link is configured.";
        }
        if (inventoryReader.get(src) == null) {
            return "Source port is not present.";
        }
        if (inventoryReader.get(dst) == null) {
            return "Destination port is not present.";
        }

        return null;
    }

    /**
     * Update the given switch port as an static edge port.
     *
     * @param sport  A {@link SalPort} instance which specifies the target
     *               port.
     * @param vport  A {@link VtnPort} associated with {@code sport}.
     * @throws VTNException  An error occurred.
     */
    private void updateAsStaticEdge(SalPort sport, VtnPort vport)
        throws VTNException {
        List<PortLink> links = vport.getPortLink();
        if (links == null) {
            return;
        }

        // Remove all the links configured on the given port.
        for (PortLink plink: links) {
            removeVtnLink(sport, plink);
        }

        Set<SalPort> edgePorts = staticEdgePorts;
        if (edgePorts == null) {
            edgePorts = new HashSet<>();
            staticEdgePorts = edgePorts;
        }

        edgePorts.add(sport);
    }

    /**
     * Remove obsolete static links configured in the given VTN port.
     *
     * <p>
     *   Note that this method assumes that the VTN port specified by
     *   {@code sport} is in UP state.
     * </p>
     *
     * @param sport  A {@link SalPort} instance which specifies the target
     *               port.
     * @param vport  A {@link VtnPort} associated with {@code sport}.
     * @return  A set of {@link SalPort} instances which represents source
     *          ports of valid static links already established in the given
     *          port.
     * @throws VTNException  An error occurred.
     */
    private Set<SalPort> removeObsoleteStaticLink(SalPort sport, VtnPort vport)
        throws VTNException {
        List<PortLink> links = vport.getPortLink();
        Set<SalPort> established;
        if (links == null) {
            established = Collections.<SalPort>emptySet();
        } else {
            established = new HashSet<>();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            for (PortLink plink: links) {
                // Read the VTN link in vtn-topology container.
                LinkId lid = plink.getLinkId();
                InstanceIdentifier<VtnLink> lpath =
                    InventoryUtils.toVtnLinkIdentifier(lid);
                Optional<VtnLink> opt =
                    DataStoreUtils.read(transaction, oper, lpath);
                if (!opt.isPresent()) {
                    continue;
                }

                VtnLink vlink = opt.get();
                if (Boolean.TRUE.equals(vlink.isStaticLink()) &&
                    isStaticLinkObsolete(sport, plink, established)) {
                    // This static link needs to be removed.
                    transaction.delete(oper, lpath);
                    InstanceIdentifier<PortLink> ppath =
                        sport.getPortLinkIdentifier(lid);
                    transaction.delete(oper, ppath);
                    SalPort peer = SalPort.create(plink.getPeer());
                    InventoryUtils.removePortLink(transaction, peer, lid);

                    // Purge ports cached in the inventory reader.
                    inventoryReader.purge(sport);
                    inventoryReader.purge(peer);
                }
            }
        }

        return established;
    }

    /**
     * Determine whether the given static inter-switch link should be removed
     * or not.
     *
     * <p>
     *   Note that this method assumes that the VTN port specified by
     *   {@code sport} is in UP state.
     * </p>
     *
     * @param sport  A {@link SalPort} instance.
     * @param plink  A {@link PortLink} instance configured in the VTN port
     *               specified by {@code sport}.
     * @param srcs   A set of {@link SalPort} instances.
     *               If the given port link is still configured as a static
     *               inter-switch link, its source port is added to this set.
     * @return  {@code true} if the given static inter-switch link should be
     *          removed. Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    private boolean isStaticLinkObsolete(SalPort sport, PortLink plink,
                                         Set<SalPort> srcs)
        throws VTNException {
        // The source port ID is used as the static link ID.
        String linkId = plink.getLinkId().getValue();
        SalPort src = SalPort.create(linkId);
        SalPort dst = inventoryReader.getStaticLink(src);
        if (dst == null) {
            // No static link is configured.
            return true;
        }

        String peerId = plink.getPeer().getValue();
        SalPort peer;
        if (src.equals(sport) && peerId.equals(dst.toString())) {
            // A static link from the given port to the peer is still
            // configured.
            peer = dst;
        } else if (peerId.equals(linkId) && dst.equals(sport)) {
            // A static link from the peer port to the given port is still
            // configured.
            peer = src;
        } else {
            // Static link configuration has been changed.
            return true;
        }

        // Check to see if the peer port is in UP state.
        VtnPort vpeer = inventoryReader.get(peer);
        boolean ret = (vpeer == null || !InventoryUtils.isEnabled(vpeer));
        if (!ret) {
            srcs.add(src);
        }

        return ret;
    }

    /**
     * Configure the specified static inter-switch link.
     *
     * <ul>
     *   <li>
     *     Note that this method needs to be called when no obsolete static
     *     inter-switch link is present.
     *   </li>
     *   <li>
     *     Note that this method must be called only if the given static link
     *     is not established.
     *   </li>
     * </ul>
     *
     * @param src  The source port of the link.
     *             The caller needs to ensure that this port is not configured
     *             as a static edge port.
     * @param dst  The destination port of the link.
     *             The caller needs to ensure that this port is not configured
     *             as a static edge port.
     * @throws VTNException  An error occurred.
     */
    private void addStaticLink(SalPort src, SalPort dst) throws VTNException {
        // Ensure that both ports are in UP state.
        VtnPort vsrc = inventoryReader.get(src);
        if (vsrc == null || !InventoryUtils.isEnabled(vsrc)) {
            return;
        }

        VtnPort vdst = inventoryReader.get(dst);
        if (vdst == null || !InventoryUtils.isEnabled(vdst)) {
            return;
        }

        // The source port ID is used as the static link ID.
        LinkId lid = new LinkId(src.toString());

        // Check to see if the given link is already present.
        InstanceIdentifier<VtnLink> lpath =
            InventoryUtils.toVtnLinkIdentifier(lid);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnLink> opt = DataStoreUtils.read(transaction, oper, lpath);
        if (opt.isPresent()) {
            // The given static link supersedes this link.
            VtnLink vlink = opt.get();
            transaction.delete(oper, lpath);
            moveVtnLink(vlink);
            InventoryUtils.removePortLink(transaction, src, lid);
            SalPort ldst = SalPort.create(vlink.getDestination());
            InventoryUtils.removePortLink(transaction, ldst, lid);
            inventoryReader.purge(ldst);
        }

        // Install the given static link.
        createVtnLink(lid, src, dst, true);

        // Purge ports cached in the inventory reader.
        inventoryReader.purge(src);
        inventoryReader.purge(dst);
    }

    /**
     * Remove the given inter-switch link.
     *
     * <p>
     *   If the given link is a dynamic link detected by topology-manager,
     *   it will be moved to ignored-links container.
     * </p>
     *
     * @param sport  A {@link SalPort} instance which specifies the target
     *               port which contains {@code plink}.
     * @param plink  A {@link PortLink} instance which specifies the link
     *               to be removed.
     * @throws VTNException  An error occurred.
     */
    private void removeVtnLink(SalPort sport, PortLink plink)
        throws VTNException {
        LinkId lid = plink.getLinkId();
        InstanceIdentifier<VtnLink> lpath =
            InventoryUtils.toVtnLinkIdentifier(lid);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnLink> opt = DataStoreUtils.read(transaction, oper, lpath);
        if (opt.isPresent()) {
            // Remove the given link from vtn-topology.
            transaction.delete(oper, lpath);

            // Move this link to ignored-links.
            moveVtnLink(opt.get());
        }

        // Remove this link from the target port.
        InstanceIdentifier<PortLink> ppath = sport.getPortLinkIdentifier(lid);
        transaction.delete(oper, ppath);

        // Remove this link from peer port.
        SalPort peer = SalPort.create(plink.getPeer());
        InventoryUtils.removePortLink(transaction, peer, lid);

        // Purge ports cached in the inventory reader.
        inventoryReader.purge(sport);
        inventoryReader.purge(peer);
    }

    /**
     * Move the given link from vtn-topology container to ignored-links
     * container.
     *
     * @param vlink  A {@link VtnLink} instance which indicates the link
     *               to be moved.
     * @throws VTNException  An error occurred.
     */
    private void moveVtnLink(VtnLink vlink) throws VTNException {
        // Never put static link into ignored-links.
        if (!Boolean.TRUE.equals(vlink.isStaticLink())) {
            InstanceIdentifier<IgnoredLink> ipath =
                InventoryUtils.toIgnoredLinkIdentifier(vlink.getLinkId());
            IgnoredLink ilink = new IgnoredLinkBuilder(vlink).build();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            transaction.put(oper, ipath, ilink, true);
            List<VtnLink> moved = movedLinks;
            if (moved == null) {
                moved = new ArrayList<>();
                movedLinks = moved;
            }
            moved.add(vlink);
        }
    }

    /**
     * Record logs about the given links.
     *
     * @param logger  A {@link FixedLogger} instance.
     * @param links   A list of {@link VtnLink} instances.
     * @param msg     A string to be logged.
     */
    private void recordLogs(FixedLogger logger, List<VtnLink> links,
                            String msg) {
        if (links != null) {
            for (VtnLink vlink: links) {
                logger.log("{}: {}: {} -> {}", msg,
                           vlink.getLinkId().getValue(),
                           vlink.getSource().getValue(),
                           vlink.getDestination().getValue());
            }
        }
    }

    /**
     * Record logs about the given ignored links.
     *
     * @param logger  A {@link FixedLogger} instance.
     * @param map     A map which keeps ignored links.
     * @param msg     A string to be logged.
     */
    private void recordLogs(FixedLogger logger, Map<IgnoredLink, String> map,
                            String msg) {
        if (map != null) {
            for (Map.Entry<IgnoredLink, String> entry: map.entrySet()) {
                IgnoredLink ilink = entry.getKey();
                String cause = entry.getValue();
                logger.log("{}: {}: {} -> {}: cause={}", msg,
                           ilink.getLinkId().getValue(),
                           ilink.getSource().getValue(),
                           ilink.getDestination().getValue(), cause);
            }
        }
    }
}
