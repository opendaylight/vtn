/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.List;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.RemovedFlows;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.flow.RemoveFlowRpc;

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
     * @param rpcs  A list of remove-flow RPC invocations.
     * @throws VTNException  An error occurred.
     */
    private void checkResults(List<RemoveFlowRpc> rpcs) throws VTNException {
        int size = rpcs.size();
        if (size <= 0) {
            return;
        }

        // Determine timeout.
        TimeUnit nano = TimeUnit.NANOSECONDS;
        TimeUnit milli = TimeUnit.MILLISECONDS;
        VTNConfig vcfg = vtnProvider.getVTNConfig();
        int msec = (size == 1)
            ? vcfg.getFlowModTimeout()
            : vcfg.getBulkFlowModTimeout();
        long timeout = milli.toNanos((long)msec);
        long deadline = System.nanoTime() + timeout;

        Logger logger = FlowRemoveContext.LOG;
        VTNException firstError = null;
        for (RemoveFlowRpc rpc: rpcs) {
            try {
                rpc.getResult(timeout, nano, logger);
            } catch (VTNException e) {
                if (firstError == null) {
                    firstError = e;
                }
                continue;
            }
            logger.trace("remove-flow has completed successfully: " +
                         "remover={}, input={}",
                         context.getRemoverDescription(), rpc.getInput());

            timeout = deadline - System.nanoTime();
            if (timeout <= 0) {
                // Wait one more millisecond.
                timeout = milli.toNanos(1L);
            }
        }

        if (firstError != null) {
            throw firstError;
        }
    }

    // Runnable

    /**
     * Uninstall flow entries.
     */
    @Override
    public void run() {
        Logger logger = FlowRemoveContext.LOG;
        SalFlowService service = context.getFlowService();
        RemovedFlows removed = context.getRemovedFlows();
        TxContext ctx = vtnProvider.newTxContext();
        try {
            List<RemoveFlowRpc> rpcs = removed.removeFlowEntries(ctx, service);
            checkResults(rpcs);
        } catch (VTNException e) {
            context.setFailure(e);
            return;
        } catch (RuntimeException e) {
            String msg = new StringBuilder("Failed to remove flow entries: " +
                                           "remover=").
                append(context.getRemoverDescription()).toString();
            logger.error(msg, e);
            context.setFailure(e);
            return;
        } finally {
            ctx.cancelTransaction();
        }

        logger.debug("Flow entries have been removed: remover={}",
                     context.getRemoverDescription());
        context.setSuccess();
    }
}
