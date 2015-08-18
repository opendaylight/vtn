/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.RemovedFlows;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code FlowRemoveContext} describes a runtime context for removing
 * VTN data flows.
 */
public final class FlowRemoveContext {
    /**
     * Logger instance used for logging of flow uninstallation.
     */
    public static final Logger  LOG =
        LoggerFactory.getLogger(FlowRemoveContext.class);

    /**
     * A future associated with the task for removing data flows.
     */
    private final SettableVTNFuture<Void>  contextFuture =
        new SettableVTNFuture<>();

    /**
     * A thread on which flow entries are uninstalled.
     */
    private final VTNThreadPool  flowThread;

    /**
     * Removed flow entries.
     */
    private RemovedFlows  removedFlows;

    /**
     * Flow remover which determines VTN data flows to be removed.
     */
    private final FlowRemover  flowRemover;

    /**
     * A MD-SAL flow service.
     */
    private SalFlowService  flowService;

    /**
     * Construct a new instance.
     *
     * @param sfs      A {@link SalFlowService} instance.
     * @param thread   A {@link VTNThreadPool} instance used to uninstall flow
     *                 entries.
     * @param remover  A {@link FlowRemover} instance.
     */
    public FlowRemoveContext(SalFlowService sfs, VTNThreadPool thread,
                             FlowRemover remover) {
        flowService = sfs;
        flowThread = thread;
        flowRemover = remover;
    }

    /**
     * Return the MD-SAL flow service.
     *
     * @return  A {@link SalFlowService} instance.
     */
    public SalFlowService getFlowService() {
        return flowService;
    }

    /**
     * Return the thread used to uninstall flow entries.
     *
     * @return  A {@link VTNThreadPool} instance.
     */
    public VTNThreadPool getFlowThread() {
        return flowThread;
    }

    /**
     * Returh the flow remover.
     *
     * @return  A {@link FlowRemover} instance.
     */
    public FlowRemover getFlowRemover() {
        return flowRemover;
    }

    /**
     * Return the description about the flow remover.
     *
     * @return  The description about the flow remover.
     */
    public String getRemoverDescription() {
        return flowRemover.getDescription();
    }

    /**
     * Set VTN data flows to be removed.
     *
     * @param removed  A {@link RemovedFlows} instance which specifies flow
     *                 entries to be removed.
     */
    public void setRemovedFlows(RemovedFlows removed) {
        removedFlows = removed;
    }

    /**
     * Return a {@link RemovedFlows} instance.
     *
     * @return  A {@link RemovedFlows} instance which specifies flow entries
     *          to be removed.
     */
    public RemovedFlows getRemovedFlows() {
        return removedFlows;
    }

    /**
     * Return the future associated with the task for removing data flows.
     *
     * @return  A {@link VTNFuture} instance associated with the
     *          task for removing data flows.
     */
    public VTNFuture<Void> getContextFuture() {
        return contextFuture;
    }

    /**
     * Complete the flow uninstallation successfully.
     */
    void setSuccess() {
        contextFuture.set(null);
    }

    /**
     * Set the cause of the failure of flow uninstallation.
     *
     * @param cause  A {@link Throwable} which indicates the cause of failure.
     */
    void setFailure(Throwable cause) {
        contextFuture.setException(cause);
    }
}
