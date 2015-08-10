/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

/**
 * The interface to be implemented by a task that modifies data tree in the
 * MD-SAL datastore on {@link TxQueue}.
 *
 * <p>
 *   The MD-SAL transaction task may start some background tasks associated
 *   with {@link VTNFuture} instances.
 * </p>
 *
 * @param <T>  The type of the object to be returned by the task.
 * @see TxQueue
 */
public interface TxTask<T> {
    /**
     * Modify the MD-SAL datastore using the given transaction.
     *
     * <p>
     *   Note that this method may be called more than once when a MD-SAL
     *   datastore transaction is aborted due to data conflict.
     * </p>
     *
     * @param ctx       A runtime context for transaction task.
     * @param attempts  The number of calls of this method.
     *                  0 is passed for the first call.
     * @return  Result of this task.
     * @throws VTNException  An error occurred.
     */
    T execute(TxContext ctx, int attempts) throws VTNException;

    /**
     * Determine whether the transaction queue should log the given error
     * or not.
     *
     * @param t  A {@link Throwable} that is going to be thrown.
     * @return   {@code true} if an error should be logged.
     *           {@code false} if an error log is not required.
     */
    boolean needErrorLog(Throwable t);

    /**
     * Invoked when the task has been completed successfully.
     *
     * <p>
     *   Note that this method must be called prior to
     *   {@link com.google.common.util.concurrent.FutureCallback}.
     * </p>
     *
     * @param provider  VTN Manager provider service.
     * @param result    An object returned by {@link #execute(TxContext, int)}.
     */
    void onSuccess(VTNManagerProvider provider, T result);

    /**
     * Invoked when the task has failed.
     *
     * <p>
     *   Note that this method must be called prior to
     *   {@link com.google.common.util.concurrent.FutureCallback}.
     * </p>
     *
     * @param provider  VTN Manager provider service.
     * @param t         A {@link Throwable} thrown by the task.
     */
    void onFailure(VTNManagerProvider provider, Throwable t);

    /**
     * Return an immutable list of {@link VTNFuture} instances associated with
     * background tasks started by the MD-SAL transaction task.
     *
     * @return  An immutable list of {@link VTNFuture} instances.
     *          An empty list is returned if no background task has started or
     *          this task does not complete yet.
     *          Note that this method must not return {@code null}.
     */
    List<VTNFuture<?>> getBackgroundTasks();
}
