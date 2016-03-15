/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.common;

import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNThreadPool;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;

/**
 * Describes runtime information for modifying VTN data flows.
 *
 * @param <T>  The type of the value to be returned by the future associated
 *             with the task.
 * @param <D>  The type of the value to be returned by the MD-SAL DS
 *             transaction task.
 */
public abstract class FlowModContext<T, D> {
    /**
     * The MD-SAL flow service.
     */
    private final SalFlowService  flowService;

    /**
     * The future associated with this context.
     */
    private final SettableVTNFuture<T>  contextFuture =
        new SettableVTNFuture<>();

    /**
     * A thread pool to run tasks for modifying flow entries.
     */
    private final VTNThreadPool  flowThread;

    /**
     * Construct a new instance.
     *
     * @param sfs     The MD-SAL flow service.
     * @param thread  A thread pool to run tasks for modifying flow entries.
     */
    protected FlowModContext(SalFlowService sfs, VTNThreadPool thread) {
        flowService = sfs;
        flowThread = thread;
    }

    /**
     * Return the MD-SAL flow service.
     *
     * @return  A {@link SalFlowService} instance.
     *          Note that {@code null} is returned if the flow service is
     *          already closed.
     */
    public final SalFlowService getFlowService() {
        return flowService;
    }

    /**
     * Return the thread used to run tasks for modifying flow entries.
     *
     * @return  A {@link VTNThreadPool} instance.
     */
    public final VTNThreadPool getFlowThread() {
        return flowThread;
    }

    /**
     * Return the future associated with this context.
     *
     * @return  A {@link VTNFuture} instance associated with this context.
     */
    public final VTNFuture<T> getContextFuture() {
        return contextFuture;
    }

    /**
     * Set the result of the flow modification.
     *
     * @param result  The result of the flow modification.
     */
    public final void setResult(T result) {
        contextFuture.set(result);
    }

    /**
     * Set the cause of the failure of the flow modification.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    public final void setFailure(Throwable cause) {
        contextFuture.setException(cause);
    }

    /**
     * Create a new MD-SAL DS task that modifies VTN data flows.
     *
     * @param txq  A MD-SAL DS transaction queue.
     * @return  A {@link TxTask} instance associated with the new MD-SAL DS
     *          task.
     */
    public abstract TxTask<D> newDatastoreTask(TxQueue txq);
}
