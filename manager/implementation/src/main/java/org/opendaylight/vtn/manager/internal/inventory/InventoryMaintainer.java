/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.List;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * Base class for MD-SAL data change listeners that maintain the VTN inventory
 * data.
 *
 * @param <T>  Type of data object in the MD-SAL datastore to listen.
 * @param <C>  Type of event context.
 */
public abstract class InventoryMaintainer<T extends DataObject, C>
    extends DataStoreListener<T, C> {
    /**
     * The transaction submit queue for the VTN inventory data models.
     */
    private final TxQueue  txQueue;

    /**
     * Construct a new instance.
     *
     * @param queue   A {@link TxQueue} instance used to update the
     *                VTN inventory.
     * @param broker  A {@link DataBroker} service instance.
     * @param clz     A {@link Class} instance that represents the target type.
     * @param scope   A {@link DataChangeScope} instance used to register
     *                data change listener.
     */
    protected InventoryMaintainer(TxQueue queue, DataBroker broker,
                                  Class<T> clz, DataChangeScope scope) {
        super(clz);
        txQueue = queue;
        registerListener(broker, LogicalDatastoreType.OPERATIONAL, scope);
    }

    /**
     * Execute the given transaction task on the transaction queue.
     *
     * @param task  A {@link TxTask} that updates the MD-SAL datastore.
     */
    protected final void submit(TxTask<?> task) {
        txQueue.post(task);
    }

    /**
     * Execute the given transaction task for initialization on the
     * transaction queue.
     *
     * @param task  A {@link TxTask} that initializes the MD-SAL datastore.
     */
    protected final void submitInitial(TxTask<?> task) {
        txQueue.postFirst(task);
    }

    /**
     * Read the specified VTN port asynchronously.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param sport  A {@link SalPort} instance corresponding to the VTN port.
     * @return  A future associated with a MD-SAL datastore read transaction.
     */
    protected final CheckedFuture<Optional<VtnPort>, ReadFailedException> read(
        ReadWriteTransaction tx, SalPort sport) {
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        return tx.read(LogicalDatastoreType.OPERATIONAL, path);
    }

    /**
     * Determine whether the given VTN port is present or not.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param sport  A {@link SalPort} instance corresponding to the VTN port.
     * @return  {@code true} only if the specified VTN port is present.
     * @throws VTNException
     *    An error occurred.
     */
    protected final boolean isPresent(ReadWriteTransaction tx, SalPort sport)
        throws VTNException {
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        return DataStoreUtils.read(tx, oper, path).isPresent();
    }

    /**
     * Add the given inter-switch link information into the VTN inventory data.
     *
     * @param tx   A {@link ReadWriteTransaction} instance.
     * @param lid  The identifier of the created link.
     * @param src  A {@link SalPort} instance corresponding to the source
     *             of the created link.
     * @param dst  A {@link SalPort} instance corresponding to the destination
     *             of the created link.
     * @return  {@code true} if the link was added to the vtn-topology list.
     *          {@code false} if the link was added to the ignored-links list.
     * @throws VTNException
     *    An error occurred.
     */
    protected final boolean addVtnLink(ReadWriteTransaction tx, LinkId lid,
                                       SalPort src, SalPort dst)
        throws VTNException {
        // Determine whether the VTN port for both termination points are
        // present or not.
        CheckedFuture<Optional<VtnPort>, ReadFailedException> sf =
            read(tx, src);
        CheckedFuture<Optional<VtnPort>, ReadFailedException> df =
            read(tx, dst);
        boolean srcPresent = DataStoreUtils.read(sf).isPresent();
        boolean dstPresent = DataStoreUtils.read(df).isPresent();

        boolean ret;
        if (srcPresent && dstPresent) {
            // Create link information.
            createVtnLink(tx, lid, src, dst);
            ret = true;
        } else {
            // Put link information into ignored link list.
            InstanceIdentifier<IgnoredLink> key =
                InventoryUtils.toIgnoredLinkIdentifier(lid);
            IgnoredLink ilink =
                InventoryUtils.toIgnoredLinkBuilder(lid, src, dst).build();
            tx.merge(LogicalDatastoreType.OPERATIONAL, key, ilink, true);
            ret = false;
        }

        return ret;
    }

    /**
     * Remove all VTN links affected by the the removed VTN node.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param snode  A {@link SalNode} instance corresponding to the removed
     *               VTN node.
     * @throws VTNException
     *    An error occurred.
     */
    protected final void removeVtnLink(ReadWriteTransaction tx, SalNode snode)
        throws VTNException {
        removeVtnTopologyLink(tx, snode);
        removeIgnoredLink(tx, snode);
    }

    /**
     * Remove all VTN links in vtn-topology affected by the removed VTN node.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param snode  A {@link SalNode} instance corresponding to the removed
     *               VTN node.
     * @throws VTNException
     *    An error occurred.
     */
    protected final void removeVtnTopologyLink(ReadWriteTransaction tx,
                                               SalNode snode)
        throws VTNException {
        InstanceIdentifier<VtnTopology> topoPath =
            InstanceIdentifier.create(VtnTopology.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        VtnTopology topology = DataStoreUtils.read(tx, oper, topoPath).
            orNull();
        if (topology == null) {
            return;
        }

        List<VtnLink> links = topology.getVtnLink();
        if (links == null) {
            return;
        }

        long dpid = snode.getNodeNumber();
        for (VtnLink vlink: links) {
            LinkId lid = vlink.getLinkId();
            SalPort src = SalPort.create(vlink.getSource());
            SalPort dst = SalPort.create(vlink.getDestination());
            long srcDpid = src.getNodeNumber();
            long dstDpid = dst.getNodeNumber();

            boolean rmLink = false;
            if (srcDpid == dpid) {
                rmLink = true;
                if (dstDpid != dpid) {
                    removePortLink(tx, dst, lid);
                }
            } else if (dstDpid == dpid) {
                rmLink = true;
                removePortLink(tx, src, lid);
            }

            if (rmLink) {
                InstanceIdentifier<VtnLink> lpath =
                    InventoryUtils.toVtnLinkIdentifier(lid);
                tx.delete(oper, lpath);
            }
        }
    }

    /**
     * Remove all VTN links in ignored-links affected by the removed VTN node.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param snode  A {@link SalNode} instance corresponding to the removed
     *               VTN node.
     * @throws VTNException
     *    An error occurred.
     */
    protected final void removeIgnoredLink(ReadWriteTransaction tx,
                                           SalNode snode)
        throws VTNException {
        InstanceIdentifier<IgnoredLinks> igPath =
            InstanceIdentifier.create(IgnoredLinks.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        IgnoredLinks igList = DataStoreUtils.read(tx, oper, igPath).orNull();
        if (igList == null) {
            return;
        }

        List<IgnoredLink> links = igList.getIgnoredLink();
        if (links == null) {
            return;
        }

        long dpid = snode.getNodeNumber();
        for (IgnoredLink vlink: links) {
            LinkId lid = vlink.getLinkId();
            SalPort src = SalPort.create(vlink.getSource());
            SalPort dst = SalPort.create(vlink.getDestination());
            long srcDpid = src.getNodeNumber();
            long dstDpid = dst.getNodeNumber();

            if (srcDpid == dpid || dstDpid == dpid) {
                InstanceIdentifier<VtnLink> lpath =
                    InventoryUtils.toVtnLinkIdentifier(lid);
                tx.delete(LogicalDatastoreType.OPERATIONAL, lpath);
            }
        }
    }

    /**
     * Remove all VTN links affected by the removed VTN port.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param vport  A {@link VtnPort} instance corresponding to the removed
     *               VTN port.
     */
    protected final void removeVtnLink(ReadWriteTransaction tx,
                                       VtnPort vport) {
        List<PortLink> links = vport.getPortLink();
        if (links == null) {
            return;
        }

        for (PortLink plink: links) {
            LinkId lid = plink.getLinkId();
            NodeConnectorId peer = plink.getPeer();
            SalPort p = SalPort.create(peer);
            removePortLink(tx, p, lid);

            InstanceIdentifier<VtnLink> lpath =
                InventoryUtils.toVtnLinkIdentifier(lid);
            tx.delete(LogicalDatastoreType.OPERATIONAL, lpath);
        }
    }

    /**
     * Remove the specified port link.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param sport  A {@link SalPort} instance corresponding to a VTN port.
     * @param lid    A {@link LinkId} that specifies inter-switch link to be
     *               removed.
     */
    protected final void removePortLink(ReadWriteTransaction tx, SalPort sport,
                                        LinkId lid) {
        InstanceIdentifier<PortLink> path = sport.getPortLinkIdentifier(lid);
        tx.delete(LogicalDatastoreType.OPERATIONAL, path);
    }

    /**
     * Try to resolve ignored inter-switch links.
     *
     * @param ctx     A runtime context for transaction task.
     * @param logger  A {@link Logger} instance.
     * @throws VTNException
     *    An error occurred.
     */
    protected final void resolveIgnoredLinks(TxContext ctx, Logger logger)
        throws VTNException {
        // Read all ignored inter-switch links.
        InstanceIdentifier<IgnoredLinks> igPath =
            InstanceIdentifier.create(IgnoredLinks.class);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        IgnoredLinks igLinks = DataStoreUtils.read(tx, oper, igPath).orNull();
        if (igLinks == null) {
            return;
        }

        List<IgnoredLink> igList = igLinks.getIgnoredLink();
        if (igList == null) {
            return;
        }

        InventoryReader reader = ctx.getInventoryReader();
        for (IgnoredLink ignored: igList) {
            SalPort src = SalPort.create(ignored.getSource());
            SalPort dst = SalPort.create(ignored.getDestination());
            if (reader.get(src) != null && reader.get(dst) != null) {
                // Move this link to vtn-topology.
                LinkId lid = ignored.getLinkId();
                InstanceIdentifier<IgnoredLink> ipath =
                    InventoryUtils.toIgnoredLinkIdentifier(lid);
                tx.delete(oper, ipath);
                createVtnLink(tx, lid, src, dst);
                logger.info("Inter-switch link has been resolved: {}: {} -> {}",
                            lid.getValue(), src, dst);
            }
        }
    }

    /**
     * Create a VTN link information.
     *
     * @param tx   A {@link ReadWriteTransaction} instance.
     * @param lid  The identifier of the created link.
     * @param src  A {@link SalPort} instance corresponding to the source
     *             of the created link.
     * @param dst  A {@link SalPort} instance corresponding to the destination
     *             of the created link.
     */
    private void createVtnLink(ReadWriteTransaction tx, LinkId lid,
                               SalPort src, SalPort dst) {
        // Put the link information into vtn-topology list.
        InstanceIdentifier<VtnLink> key =
            InventoryUtils.toVtnLinkIdentifier(lid);
        VtnLink vlink = InventoryUtils.toVtnLinkBuilder(lid, src, dst).build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        tx.merge(oper, key, vlink, true);

        // Create source port link.
        InstanceIdentifier<PortLink> pkey = src.getPortLinkIdentifier(lid);
        PortLink plink = InventoryUtils.toPortLinkBuilder(lid, dst).build();
        tx.merge(oper, pkey, plink, true);

        // Create destination port link.
        pkey = dst.getPortLinkIdentifier(lid);
        plink = InventoryUtils.toPortLinkBuilder(lid, src).build();
        tx.merge(oper, pkey, plink, true);
    }
}
