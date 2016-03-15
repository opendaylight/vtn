/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.add;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.flow.common.FlowModContext;
import org.opendaylight.vtn.manager.internal.flow.common.FlowModContextFactory;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code FlowAddContext} describes a runtime information for adding a
 * new VTN data flow.
 */
public final class FlowAddContext
    extends FlowModContext<VtnFlowId, VtnDataFlow> {
    /**
     * Logger instance used for logging of flow installation.
     */
    public static final Logger  LOG =
        LoggerFactory.getLogger(FlowAddContext.class);

    /**
     * A VTN data flow builder which contains a data flow to be added.
     */
    private final VTNFlowBuilder  flowBuilder;

    /**
     * Factory class for instantiating {@link FlowAddContext}.
     */
    public static final class Factory
        implements FlowModContextFactory<VtnFlowId, VtnDataFlow> {
        /**
         * A VTN data flow builder which contains a data flow to be added.
         */
        private final VTNFlowBuilder  builder;

        /**
         * Construct a new instance.
         *
         * @param b  A {@link VTNFlowBuilder} instance.
         */
        public Factory(VTNFlowBuilder b) {
            builder = b;
        }

        // FlowModContextFactory

        /**
         * Construct a new context for adding VTN data flows.
         *
         * @param sfs     The MD-SAL flow service.
         * @param thread  A thread pool to run tasks that add flow entries.
         * @return  A new context for adding VTN data flows.
         */
        @Override
        public FlowAddContext newContext(SalFlowService sfs,
                                         VTNThreadPool thread) {
            return new FlowAddContext(sfs, thread, builder);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param sfs      A {@link SalFlowService} instance.
     * @param thread   A {@link VTNThreadPool} instance used to run tasks.
     * @param builder  A {@link VTNFlowBuilder} instance.
     * @see Factory
     */
    private FlowAddContext(SalFlowService sfs, VTNThreadPool thread,
                           VTNFlowBuilder builder) {
        super(sfs, thread);
        flowBuilder = builder;
    }

    /**
     * Return the VTN data flow builder.
     *
     * @return  A {@link VTNFlowBuilder} instance.
     */
    public VTNFlowBuilder getFlowBuilder() {
        return flowBuilder;
    }

    // FlowModContext

    /**
     * Create a new MD-SAL DS task that adds VTN data flows.
     *
     * @param txq  A MD-SAL DS transaction queue.
     * @return  A MD-SAL DS transaction task that adds VTN data flows.
     */
    @Override
    public PutFlowTxTask newDatastoreTask(TxQueue txq) {
        return new PutFlowTxTask(this, getFlowThread(), txq);
    }
}
