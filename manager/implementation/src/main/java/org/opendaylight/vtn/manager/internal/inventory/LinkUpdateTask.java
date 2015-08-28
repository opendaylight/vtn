/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.HashSet;
import java.util.Set;

import com.google.common.base.Optional;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkUpdateContext;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;

/**
 * A MD-SAL datastore transaction task that updates VTN topology information.
 *
 * <p>
 *   This task returns a {@link LinkUpdateContext} instance.
 * </p>
 */
final class LinkUpdateTask extends AbstractTxTask<LinkUpdateContext> {
    /**
     * A set of paths to updated inter-switch links.
     */
    private final Set<InstanceIdentifier<Link>>  updated = new HashSet<>();

    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * Construct a new instance.
     *
     * @param log  A {@link Logger} instance.
     */
    LinkUpdateTask(Logger log) {
        logger = log;
    }

    /**
     * Add a link information notified by a data change event.
     *
     * @param path  Path to the target link.
     */
    void addUpdated(InstanceIdentifier<Link> path) {
        updated.add(path);
    }

    /**
     * Determine whether this task contains at least one notification or not.
     *
     * @return  {@code true} only if this instance contains at least one
     *          notification.
     */
    boolean hasUpdates() {
        return !updated.isEmpty();
    }

    /**
     * Add a VTN link information corresponding to the given MD-SAL
     * inter-switch link.
     *
     * @param luctx  A {@link LinkUpdateContext} instance.
     * @param link   A {@link Link} instance.
     * @throws VTNException  An error occurred.
     */
    private void add(LinkUpdateContext luctx, Link link) throws VTNException {
        LinkEdge le;
        try {
            le = new LinkEdge(link);
        } catch (RuntimeException e) {
            logger.debug("Ignore unsupported inter-switch link: " + link, e);
            return;
        }

        LinkId lid = link.getLinkId();
        SalPort src = le.getSourcePort();
        SalPort dst = le.getDestinationPort();
        luctx.addVtnLink(lid, src, dst);
    }

    /**
     * Remove a VTN link information corresponding to the given MD-SAL
     * inter-switch link.
     *
     * @param tx    A {@link ReadWriteTransaction} instance.
     * @param path  Path to the removed inter-switch link.
     * @throws VTNException  An error occurred.
     */
    private void remove(ReadWriteTransaction tx, InstanceIdentifier<Link> path)
        throws VTNException {
        LinkId lid = InventoryUtils.getLinkId(path);
        if (lid == null) {
            logger.warn("Ignore invalid inter-switch link path: {}", path);
            return;
        }

        // Read the VTN link information corresponding to the given link.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnLink> lpath =
            InventoryUtils.toVtnLinkIdentifier(lid);
        VtnLink vlink = DataStoreUtils.read(tx, oper, lpath).orNull();

        // Note that we should not remove the link if it is configured
        // as static link.
        if (vlink != null && !Boolean.TRUE.equals(vlink.isStaticLink())) {
            SalPort src = SalPort.create(vlink.getSource());
            SalPort dst = SalPort.create(vlink.getDestination());

            // Remove the link from vtn-topology list.
            tx.delete(oper, lpath);

            // Remove port links.
            InventoryUtils.removePortLink(tx, src, lid);
            InventoryUtils.removePortLink(tx, dst, lid);
        } else {
            // Remove the link from ignored-links list.
            InstanceIdentifier<IgnoredLink> ipath =
                InventoryUtils.toIgnoredLinkIdentifier(lid);
            DataStoreUtils.delete(tx, oper, ipath);
        }
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public LinkUpdateContext execute(TxContext ctx) throws VTNException {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InventoryReader reader = ctx.getInventoryReader();
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);
        for (InstanceIdentifier<Link> path: updated) {
            // Read the notified link.
            Optional<Link> opt = DataStoreUtils.read(tx, oper, path);
            if (opt.isPresent()) {
                // Add the inter-switch link information.
                add(luctx, opt.get());
            } else {
                // Remove the inter-swtich link information.
                remove(tx, path);
            }
        }

        return luctx;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider,
                          LinkUpdateContext luctx) {
        luctx.recordLogs(logger);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        logger.error("Failed to update VTN link information.", t);
    }
}
