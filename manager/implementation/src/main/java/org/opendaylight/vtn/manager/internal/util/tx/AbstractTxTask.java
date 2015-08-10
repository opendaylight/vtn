/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

/**
 * An abstract simple implementation of {@link TxTask}.
 *
 * <p>
 *   This class attempt to repeat the transaction at most 5 times if the
 *   transaction fails due to data conflict.
 * </p>
 *
 * @param <T>  The type of the object to be returned by the task.
 */
public abstract class AbstractTxTask<T> implements TxTask<T> {
    /**
     * The number of times for retrying transaction.
     */
    private static final int  MAX_RETRY = 5;

    /**
     * A list of {@link VTNFuture} instances associated with background tasks.
     */
    private List<VTNFuture<?>>  backgroundTasks;

    /**
     * Modify the MD-SAL datastore using the given transaction.
     *
     * <p>
     *   Note that this method may be called more than once when a MD-SAL
     *   datastore transaction is aborted due to data conflict.
     * </p>
     *
     * @param ctx  A runtime context for transaction task.
     * @return  Result of this task.
     * @throws VTNException  An error occurred.
     */
    protected abstract T execute(TxContext ctx) throws VTNException;

    /**
     * Return the number of times for retrying transaction.
     *
     * <p>
     *   This method can be used to control the number of transaction retry
     *   when the transaction failed due to data confliction.
     * </p>
     * <p>
     *   This method in this class returns {@link #MAX_RETRY}.
     *   Subclass can override this method to change the number of times for
     *   retrying transaction.
     * </p>
     *
     * @return  The number of times for retrying transaction.
     */
    protected int getMaxRetry() {
        return MAX_RETRY;
    }

    /**
     * Add a background task started by this task.
     *
     * @param future  A {@link VTNFuture} instance associated with background
     *                task.
     */
    protected final void addBackgroundTask(VTNFuture<?> future) {
        List<VTNFuture<?>> list = backgroundTasks;
        if (list == null) {
            list = new ArrayList<>();
            backgroundTasks = list;
        }
        list.add(future);
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public final T execute(TxContext ctx, int attempts) throws VTNException {
        if (attempts > getMaxRetry()) {
            throw new VTNException("Data conflict could not be resolved.",
                                   null);
        }

        return execute(ctx);
    }

    /**
     * Determine whether the transaction queue should log the given error
     * or not.
     *
     * @param t  A {@link Throwable} that is going to be thrown.
     * @return   {@code true} is always returned.
     */
    @Override
    public boolean needErrorLog(Throwable t) {
        return true;
    }

    /**
     * Invoked when the task has been completed successfully.
     *
     * <p>
     *   This method of this class does nothing.
     * </p>
     *
     * @param provider  VTN Manager provider service.
     * @param result    An object returned by {@link #execute(TxContext, int)}.
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, T result) {
    }

    /**
     * Invoked when the task has failed.
     *
     * <p>
     *   This method of this class does nothing.
     * </p>
     *
     * @param provider  VTN Manager provider service.
     * @param t         A {@link Throwable} thrown by the task.
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final List<VTNFuture<?>> getBackgroundTasks() {
        List<VTNFuture<?>> list = backgroundTasks;
        return (list == null)
            ? Collections.<VTNFuture<?>>emptyList()
            : Collections.unmodifiableList(list);
    }
}
