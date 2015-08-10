/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.add;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;

/**
 * {@code RollbackTxTask} describes the MD-SAL datastore transaction task that
 * rollbacks changes made by {@code PutFlowTxTask}.
 */
public final class RollbackTxTask extends AbstractTxTask<Void> {
    /**
     * A {@link VTNFlowBuilder} instance which contains a data flow to be
     * removed from the datastore.
     */
    private final VTNFlowBuilder  flowBuilder;

    /**
     * Construct a new instance.
     *
     * @param ctx  A {@link FlowAddContext} instance.
     */
    public RollbackTxTask(FlowAddContext ctx) {
        flowBuilder = ctx.getFlowBuilder();
    }

    // AbstractTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public Void execute(TxContext ctx) throws VTNException {
        FlowUtils.removeDataFlow(ctx.getReadWriteTransaction(),
                                 flowBuilder.getTenantName(), flowBuilder);
        return null;
    }

    // TxTask

    /**
     * Invoked when the task has failed.
     *
     * @param provider  VTN Manager provider service.
     * @param t         A {@link Throwable} thrown by the task.
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        VtnFlowId flowId = flowBuilder.getDataFlow().getFlowId();
        String msg = new StringBuilder(flowId.getValue().toString()).
            append(": Failed to rollback dataflow.").toString();
        FlowAddContext.LOG.error(msg, t);
    }
}
