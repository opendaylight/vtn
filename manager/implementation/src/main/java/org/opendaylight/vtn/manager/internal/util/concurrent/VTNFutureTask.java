/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.Callable;
import java.util.concurrent.Executor;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;

import com.google.common.util.concurrent.ExecutionList;

import org.opendaylight.vtn.manager.VTNException;

/**
 * An implementation of {@link FutureTask} that also implements
 * {@link VTNFuture}.
 *
 * @param <T>   The type of the object to be returned.
 */
public class VTNFutureTask<T> extends FutureTask<T> implements VTNFuture<T> {
    /**
     * The execution list to hold executors.
     */
    private final ExecutionList  executionList = new ExecutionList();

    /**
     * Construct a new instance.
     *
     * @param callable  A {@link Callable} task.
     */
    public VTNFutureTask(Callable<T> callable) {
        super(callable);
    }

    /**
     * Create a {@link VTNFutureTask} instance that will execute the given
     * runnable.
     *
     * @param runnable  A {@link Runnable} task.
     * @param result    The result to return on successful task completion.
     */
    public VTNFutureTask(Runnable runnable, T result) {
        super(runnable, result);
    }

    // ListenableFuture

    /**
     * Add a listener to be executed on the specified executor.
     *
     * @param listener  A listener to run when the computation is complete.
     * @param executor  A executor to run the given listener on.
     */
    @Override
    public final void addListener(Runnable listener, Executor executor) {
        executionList.add(listener, executor);
    }

    // FutureTask

    /**
     * Invoked when this task completed.
     */
    @Override
    protected final void done() {
        executionList.execute();
    }

    // VTNFuture

    /**
     * {@inheritDoc}
     */
    @Override
    public final T checkedGet() throws VTNException {
        try {
            return get();
        } catch (Exception e) {
            throw AbstractVTNFuture.getException(e);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final T checkedGet(long timeout, TimeUnit unit)
        throws VTNException {
        try {
            return get(timeout, unit);
        } catch (Exception e) {
            throw AbstractVTNFuture.getException(e);
        }
    }
}
