/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.concurrent.CancellationException;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.RemovedFlows;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

/**
 * {@code PutFlowTxTask} describes the MD-SAL datastore transaction task that
 * uninstalls VTN data flows.
 *
 * <p>
 *   This task returns a list of VTN data flows to be removed.
 * </p>
 */
public final class DeleteFlowTxTask extends AbstractTxTask<RemovedFlows> {
    /**
     * A {@link FlowRemoveContext} instance.
     */
    private final FlowRemoveContext  context;

    /**
     * Construct a new instance.
     *
     * @param ctx     A {@link FlowRemoveContext} instance.
     */
    public DeleteFlowTxTask(FlowRemoveContext ctx) {
        context = ctx;
    }

    // AbstractTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovedFlows execute(TxContext ctx) throws VTNException {
        return context.getFlowRemover().removeDataFlow(ctx);
    }

    // TxTask

    /**
     * Invoked when the task has completed successfully.
     *
     * @param provider  VTN Manager provider service.
     * @param removed   A {@link RemovedFlows} instance which specifies flow
     *                  entries to be removed.
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, RemovedFlows removed) {
        if (removed.isEmpty()) {
            // No flow entry needs to be removed.
            FlowRemoveContext.LOG.debug(
                "No data flow was removed: remover={}",
                context.getRemoverDescription());
            context.setResult(null);
        } else {
            // Uninstall flow entries from switches.
            context.setRemovedFlows(removed);
            FlowRemoveTask task = new FlowRemoveTask(provider, context);
            if (!context.getFlowThread().executeTask(task)) {
                String msg = "Flow thread is already closed";
                FlowRemoveContext.LOG.
                    warn("{}: remover={}", msg,
                         context.getRemoverDescription());
                context.setFailure(new CancellationException(msg));
            }
        }
    }

    /**
     * Invoked when the task has failed.
     *
     * @param provider  VTN Manager provider service.
     * @param t         A {@link Throwable} thrown by the task.
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        String msg = "Failed to remove data flow from DS: remover=" +
            context.getRemoverDescription();
        FlowRemoveContext.LOG.error(msg, t);
        context.setFailure(t);
    }
}
