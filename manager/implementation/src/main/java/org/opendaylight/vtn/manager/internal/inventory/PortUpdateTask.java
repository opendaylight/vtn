/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;

/**
 * A MD-SAL datastore transaction task that updates VTN port information.
 */
final class PortUpdateTask
    extends InventoryUpdateTask<FlowCapableNodeConnector, SalPort> {
    /**
     * Construct a new instance.
     *
     * @param log  A {@link Logger} instance.
     */
    PortUpdateTask(Logger log) {
        super(log);
    }

    // InventoryUpdateTask
    /**
     * Add a VTN port information corresponding to the given MD-SAL node
     * connector.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param sport  A {@link SalPort} instance which indicates the location
     *               of the node connector.
     * @param path   Path to the {@link FlowCapableNodeConnector} instance in
     *               the MD-SAL datastore.
     * @param fcnc   A {@link FlowCapableNodeConnector} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void add(TxContext ctx, ReadWriteTransaction tx, SalPort sport,
                       InstanceIdentifier<FlowCapableNodeConnector> path,
                       FlowCapableNodeConnector fcnc) throws VTNException {
        // Create or update a VTN port.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        VtnPort vport = InventoryUtils.
            toVtnPortBuilder(sport.getNodeConnectorId(), fcnc).build();
        tx.merge(oper, sport.getVtnPortIdentifier(), vport, true);

        // Read VTN node from the datastore.
        SalNode snode = sport.getSalNode();
        InstanceIdentifier<VtnNode> vnpath = snode.getVtnNodeIdentifier();
        VtnNode vnode = DataStoreUtils.read(tx, oper, vnpath).orNull();
        if (vnode == null || vnode.getOpenflowVersion() == null) {
            // Estimate protocol version.
            VtnOpenflowVersion v = InventoryUtils.getOpenflowVersion(fcnc);
            VtnNode vn = new VtnNodeBuilder().setId(snode.getNodeId()).
                setOpenflowVersion(v).build();
            tx.merge(oper, vnpath, vn, true);
        }

        // Cache this port into the inventory reader.
        InventoryReader reader = ctx.getInventoryReader();
        reader.prefetch(sport, vport);
    }

    /**
     * Remove a VTN port information corresponding to the given MD-SAL node
     * connector.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param sport  A {@link SalPort} instance which indicates the location
     *               of the node connector.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void remove(TxContext ctx, ReadWriteTransaction tx,
                          SalPort sport) throws VTNException {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnPort> vpath = sport.getVtnPortIdentifier();
        VtnPort vport = DataStoreUtils.read(tx, oper, vpath).orNull();
        if (vport != null) {
            // Remove VTN links affected by the removed port.
            InventoryUtils.removeVtnLink(tx, vport);

            // Remove a VTN port.
            tx.delete(oper, vpath);
        }

        // Add negative cache for the removed port.
        InventoryReader reader = ctx.getInventoryReader();
        reader.prefetch(sport, (VtnPort)null);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void fixUp(TxContext ctx, boolean added) throws VTNException {
        if (added) {
            // Resolve ignored inter-switch links.
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            InventoryReader reader = ctx.getInventoryReader();
            InventoryUtils.resolveIgnoredLinks(tx, reader, getLogger());
        }
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        getLogger().error("Failed to update VTN port information.", t);
    }
}
