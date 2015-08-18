/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.slf4j.Logger;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * A MD-SAL datastore transaction task that updates VTN node information.
 */
final class NodeUpdateTask
    extends InventoryUpdateTask<FlowCapableNode, SalNode> {
    /**
     * Construct a new instance.
     *
     * @param log  A {@link Logger} instance.
     */
    NodeUpdateTask(Logger log) {
        super(log);
    }

    // InventoryUpdateTask

    /**
     * Add a VTN node information corresponding to the given MD-SAL node.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param snode  A {@link SalNode} instance which indicates the location
     *               of the node.
     * @param path   Path to the {@link FlowCapableNode} instance in the
     *               MD-SAL datastore.
     * @param fcn    A {@link FlowCapableNode} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void add(TxContext ctx, ReadWriteTransaction tx, SalNode snode,
                       InstanceIdentifier<FlowCapableNode> path,
                       FlowCapableNode fcn) throws VTNException {
        // Read MD-SAL node.
        InstanceIdentifier<Node> npath = snode.getNodeIdentifier();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<Node> opt = DataStoreUtils.read(tx, oper, npath);
        VtnNodeBuilder builder;
        if (opt.isPresent()) {
            builder = InventoryUtils.toVtnNodeBuilder(opt.get());
        } else {
            // The given MD-SAL node is no longer present.
            // VTN node will be removed soon.
            builder = InventoryUtils.toVtnNodeBuilder(snode.getNodeId());
        }

        // Create a VTN node.
        tx.merge(oper, snode.getVtnNodeIdentifier(), builder.build(), true);
    }

    /**
     * Remove a VTN node information corresponding to the given MD-SAL node.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param snode  A {@link SalNode} instance which indicates the location
     *               of the node.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected void remove(TxContext ctx, ReadWriteTransaction tx,
                          SalNode snode)throws VTNException {
        // Remove VTN links affected by the removed node.
        InventoryUtils.removeVtnLink(tx, snode);

        // Delete a VTN node associated with the given MD-SAL node.
        DataStoreUtils.delete(tx, LogicalDatastoreType.OPERATIONAL,
                              snode.getVtnNodeIdentifier());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void fixUp(TxContext ctx, boolean added) {
        // Nothing to do here.
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        getLogger().error("Failed to update VTN node information.", t);
    }
}
