/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import static org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoveContext.LOG;

import java.util.concurrent.TimeUnit;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.RemovedFlows;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpcList;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code FlowRemoveTask} describes a task that uninstalls flow entries from
 * switches.
 */
public final class FlowRemoveTask implements Runnable {
    /**
     * VTN Manager service provider.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A {@link FlowRemoveContext} instance.
     */
    private final FlowRemoveContext  context;

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     * @param ctx       A {@link FlowRemoveContext} instance.
     */
    public FlowRemoveTask(VTNManagerProvider provider, FlowRemoveContext ctx) {
        vtnProvider = provider;
        context = ctx;
    }

    /**
     * Check the result of remove-flow RPC invocations.
     *
     * @param rpcs  A {@link RemoveFlowRpcList} instance.
     * @throws VTNException  An error occurred.
     */
    private void checkResults(RemoveFlowRpcList rpcs) throws VTNException {
        int size = rpcs.size();
        if (size > 0) {
            // Determine timeout.
            VTNConfig vcfg = vtnProvider.getVTNConfig();
            int timeout = (size == 1)
                ? vcfg.getFlowModTimeout()
                : vcfg.getBulkFlowModTimeout();
            rpcs.verify(LOG, timeout, TimeUnit.MILLISECONDS);
        }
    }

    // Runnable

    /**
     * Uninstall flow entries.
     */
    @Override
    public void run() {
        SalFlowService sfs = context.getFlowService();
        RemovedFlows removed = context.getRemovedFlows();
        TxContext ctx = vtnProvider.newTxContext();
        try {
            checkResults(removed.removeFlowEntries(ctx, sfs));
        } catch (VTNException e) {
            context.setFailure(e);
            return;
        } catch (RuntimeException e) {
            String msg = new StringBuilder("Failed to remove flow entries: " +
                                           "remover=").
                append(context.getRemoverDescription()).toString();
            LOG.error(msg, e);
            context.setFailure(e);
            return;
        } finally {
            ctx.cancelTransaction();
        }

        LOG.debug("Flow entries have been removed: remover={}",
                  context.getRemoverDescription());
        context.setSuccess();
    }
}
