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

import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * {@code FlowAddContext} describes a runtime information for adding a
 * new VTN data flow.
 */
public final class FlowAddContext {
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
     * A future associated with the task for adding a new data flow.
     */
    private final SettableVTNFuture<VtnFlowId>  contextFuture =
        new SettableVTNFuture<>();

    /**
     * A MD-SAL flow service.
     */
    private final SalFlowService  flowService;

    /**
     * Construct a new instance.
     *
     * @param sfs      A {@link SalFlowService} instance.
     * @param builder  A {@link VTNFlowBuilder} instance.
     */
    public FlowAddContext(SalFlowService sfs, VTNFlowBuilder builder) {
        flowBuilder = builder;
        flowService = sfs;
    }

    /**
     * Return the VTN data flow builder.
     *
     * @return  A {@link VTNFlowBuilder} instance.
     */
    public VTNFlowBuilder getFlowBuilder() {
        return flowBuilder;
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
     * Return the future associated with the task for adding a new data flow.
     *
     * @return  A {@link VTNFuture} instance which returns the identifier for
     *          a new data flow.
     *          Note that the future will return {@code null} if the data flow
     *          configured in this instance is already installed.
     */
    public VTNFuture<VtnFlowId> getContextFuture() {
        return contextFuture;
    }

    /**
     * Set the result of the flow installation.
     *
     * @param result  A VTN flow ID for a new data flow.
     *                {@code null} indicates that the data flow is already
     *                installed.
     */
    void setResult(VtnFlowId result) {
        contextFuture.set(result);
    }

    /**
     * Set the cause of the failure of flow installation.
     *
     * @param cause  A {@link Throwable} which indicates the cause of failure.
     */
    void setFailure(Throwable cause) {
        contextFuture.setException(cause);
    }
}
