/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.List;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.RemovedFlows;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.flow.FlowEntryDesc;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpc;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpcList;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@link RemovedDataFlow} describes a VTN data flow notified by a FLOW_REMOVED
 * notification.
 */
public final class RemovedDataFlow implements RemovedFlows {
    /**
     * A flow remover that removed data flows.
     */
    private final RemovedFlowRemover  flowRemover;

    /**
     * A removed VTN data flow.
     */
    private final FlowCache  removedFlow;

    /**
     * Construct a new instance.
     *
     * @param remover  A {@link RemovedFlowRemover} instance that removed data
     *                 flow.
     * @param fc       A {@link FlowCache} instance which contains the removed
     *                 data flow.
     */
    public RemovedDataFlow(RemovedFlowRemover remover, FlowCache fc) {
        flowRemover = remover;
        removedFlow = fc;
    }

    // RemovedFlows

    /**
     * {@inheritDoc}
     */
    @Override
    public List<RemoveFlowRpc> removeFlowEntries(TxContext ctx,
                                                 SalFlowService sfs)
        throws VTNException {
        Logger logger = FlowRemoveContext.LOG;
        if (logger.isDebugEnabled()) {
            String desc = flowRemover.getDescription();
            FlowUtils.removedLog(logger, desc, removedFlow);
        }

        List<VtnFlowEntry> entries = removedFlow.getFlowEntries();
        RemoveFlowRpcList rpcs = new RemoveFlowRpcList();
        InventoryReader reader = ctx.getInventoryReader();
        String flowNode = flowRemover.getFlowNode().toString();
        for (VtnFlowEntry vfent: entries) {
            String nid = vfent.getNode().getValue();
            if (flowNode.equals(nid)) {
                FlowRemoveContext.LOG.
                    trace("Ignore flow entry notified by FLOW_REMOVED: {}",
                          new FlowEntryDesc(vfent));
            } else {
                SalNode snode = SalNode.create(nid);
                RemoveFlowInputBuilder builder = FlowUtils.
                    createRemoveFlowInputBuilder(snode, vfent, reader);
                if (builder != null) {
                    rpcs.add(snode, builder);
                }
            }
        }

        return rpcs.invoke(sfs);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isEmpty() {
        return (removedFlow == null);
    }
}
