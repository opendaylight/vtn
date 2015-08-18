/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.NodeFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableKey;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which removes
 * VTN data flows related to the specified switch.
 *
 * <p>
 *   This flow remover affects flow entries in all the VTNs.
 * </p>
 */
public final class NodeFlowRemover
    extends IndexFlowRemover<NodeFlows, RemovedDataFlows> {
    /**
     * The target node identifier.
     */
    private final SalNode  flowNode;

    /**
     * The key which specifies the target node in the node flow index.
     */
    private final NodeFlowsKey  nodeKey;

    /**
     * Construct a new instance.
     *
     * @param snode  A {@link SalNode} instance.
     */
    public NodeFlowRemover(SalNode snode) {
        flowNode = snode;
        nodeKey = new NodeFlowsKey(snode.getNodeId());
    }

    // IndexFlowRemover

    /**
     * Return the path to the node flow index to be removed.
     *
     * @param key  A {@link VtnFlowTableKey} instance which specifies the
     *             target VTN flow table.
     * @return  Path to the node flow index to be removed.
     */
    @Override
    protected InstanceIdentifier<NodeFlows> getPath(VtnFlowTableKey key) {
        return FlowUtils.getIdentifier(key, nodeKey);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void add(RemovedDataFlows removed, FlowCache fc) {
        removed.add(fc);
    }

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovedDataFlows removeDataFlow(TxContext ctx)
        throws VTNException {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        RemovedDataFlows removed = new RemovedDataFlows<NodeFlowRemover>(this);
        for (VtnFlowTable table: FlowUtils.getFlowTables(tx)) {
            remove(removed, tx, table);
        }

        return removed;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return new StringBuilder("node[").
            append(flowNode).append(']').toString();
    }
}
