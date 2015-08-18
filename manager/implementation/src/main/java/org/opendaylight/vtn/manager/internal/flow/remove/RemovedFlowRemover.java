/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * An implementation of {@link FlowRemover} which removes a VTN data flow
 * associated with a removed flow entry.
 */
public final class RemovedFlowRemover implements FlowRemover {
    /**
     * The VTN data flow to be removed.
     */
    private final IdentifiedData<VtnDataFlow>  dataFlow;

    /**
     * The switch which removed the flow entry.
     */
    private final SalNode  flowNode;

    /**
     * Construct a new instance.
     *
     * @param data   An {@link IdentifiedData} instance which contains the
     *               VTN data flow to be removed.
     * @param snode  A {@link SalNode} instance which specifies the switch
     *               which removed the flow entry.
     */
    public RemovedFlowRemover(IdentifiedData<VtnDataFlow> data,
                              SalNode snode) {
        dataFlow = data;
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
        // Check to see if the target data flow is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnDataFlow> path = dataFlow.getIdentifier();
        FlowCache fc;
        Optional<VtnDataFlow> opt = DataStoreUtils.read(tx, oper, path);
        if (opt.isPresent()) {
            // Delete the data flow.
            fc = new FlowCache(opt.get());
            tx.delete(oper, path);

            // Clean up indices.
            InstanceIdentifier<VtnFlowTable> tpath =
                path.firstIdentifierOf(VtnFlowTable.class);
            FlowUtils.removeIndex(tx, tpath, fc);
        } else {
            fc = null;
        }

        return new RemovedDataFlow(this, fc);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        String tname = FlowUtils.getTenantName(dataFlow.getIdentifier());
        return new StringBuilder("flow-removed[vtn=").
            append(tname).append(", id=").
            append(dataFlow.getValue().getFlowId().getValue()).
            append(", node=").append(flowNode).append(']').toString();
    }
}
