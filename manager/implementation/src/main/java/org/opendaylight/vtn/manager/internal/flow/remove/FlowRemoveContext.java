/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
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
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.flow.common.FlowModContext;
import org.opendaylight.vtn.manager.internal.flow.common.FlowModContextFactory;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code FlowRemoveContext} describes a runtime context for removing
 * VTN data flows.
 */
public final class FlowRemoveContext
    extends FlowModContext<Void, RemovedFlows> {
    /**
     * Logger instance used for logging of flow uninstallation.
     */
    public static final Logger  LOG =
        LoggerFactory.getLogger(FlowRemoveContext.class);

    /**
     * Removed flow entries.
     */
    private RemovedFlows  removedFlows;

    /**
     * Flow remover which determines VTN data flows to be removed.
     */
    private final FlowRemover  flowRemover;

    /**
     * Factory class for instantiating {@link FlowRemoveContext}.
     */
    public static final class Factory
        implements FlowModContextFactory<Void, RemovedFlows> {
        /**
         * Flow remover which determines VTN data flows to be removed.
         */
        private final FlowRemover  remover;

        /**
         * Construct a new instance.
         *
         * @param r  A {@link FlowRemover} instance.
         */
        public Factory(FlowRemover r) {
            remover = r;
        }

        // FlowModContextFactory

        /**
         * Construct a new context for removing VTN data flows.
         *
         * @param sfs     The MD-SAL flow service.
         * @param thread  A thread pool to run tasks that remove flow entries.
         * @return  A new context for removing VTN data flows.
         */
        @Override
        public FlowRemoveContext newContext(SalFlowService sfs,
                                            VTNThreadPool thread) {
            return new FlowRemoveContext(sfs, thread, remover);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param sfs      A {@link SalFlowService} instance.
     * @param thread   A {@link VTNThreadPool} instance used to uninstall flow
     *                 entries.
     * @param remover  A {@link FlowRemover} instance.
     * @see Factory
     */
    private FlowRemoveContext(SalFlowService sfs, VTNThreadPool thread,
                              FlowRemover remover) {
        super(sfs, thread);
        flowRemover = remover;
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

    // FlowModContext

    /**
     * Create a new MD-SAL DS task that deletes VTN data flows.
     *
     * @param txq  A MD-SAL DS transaction queue.
     * @return  A MD-SAL DS transaction task that delets VTN data flows.
     */
    @Override
    public DeleteFlowTxTask newDatastoreTask(TxQueue txq) {
        return new DeleteFlowTxTask(this);
    }
}
