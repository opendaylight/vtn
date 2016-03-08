/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import static org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoveContext.LOG;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowFinder;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * An implementation of {@link FlowRemover} which removes a VTN data flow
 * associated with a removed flow entry.
 */
public final class RemovedFlowRemover implements FlowRemover {
    /**
     * The VTN flow ID that specifies the target VTN flow.
     */
    private final VtnFlowId  flowId;

    /**
     * The switch which removed the flow entry.
     */
    private final SalNode  flowNode;

    /**
     * Construct a new instance.
     *
     * @param id     An {@link VtnFlowId} instance that specifies the VTN data
     *               flow to be removed.
     * @param snode  A {@link SalNode} instance which specifies the switch
     *               which removed the flow entry.
     */
    public RemovedFlowRemover(VtnFlowId id, SalNode snode) {
        flowId = id;
        flowNode = snode;
    }

    /**
     * Return the switch which removed the flow entry.
     *
     * @return  A {@link SalNode} instance which specifies the switch.
     */
    public SalNode getFlowNode() {
        return flowNode;
    }

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovedDataFlow removeDataFlow(TxContext ctx) throws VTNException {
        FlowCache fc;

        // Ensure that the target node is present in opendaylight-inventory.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<Node> opt =
            DataStoreUtils.read(tx, oper, flowNode.getNodeIdentifier());
        if (opt.isPresent()) {
            // Search for the specified VTN data flow.
            IdentifiedData<VtnDataFlow> data = new FlowFinder(tx).find(flowId);
            if (data == null) {
                ctx.log(LOG, VTNLogLevel.DEBUG,
                        "Data flow not found: node={}, flow={}", flowNode,
                        flowId.getValue());
                fc = null;
            } else {
                // Delete the specified data flow.
                fc = new FlowCache(data.getValue());
                InstanceIdentifier<VtnDataFlow> path = data.getIdentifier();
                tx.delete(oper, path);

                // Clean up indices.
                InstanceIdentifier<VtnFlowTable> tpath =
                    path.firstIdentifierOf(VtnFlowTable.class);
                FlowUtils.removeIndex(tx, tpath, fc);
            }
        } else {
            // Nothing to do here.
            // Rest of work will be done by VTN node REMOVED event handler.
            ctx.log(LOG, VTNLogLevel.DEBUG,
                    "Ignore FLOW_REMOVED on removed node: node={}, flow={}",
                    flowNode, flowId.getValue());
            fc = null;
        }

        return new RemovedDataFlow(this, fc);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return new StringBuilder("flow-removed[id=").append(flowId.getValue()).
            append(", node=").append(flowNode).append(']').toString();
    }
}
