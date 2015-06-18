/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.util.concurrent.ExecutionList;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.md.sal.common.api.data.OptimisticLockFailedException;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * An abstract implementation of {@link VTNFuture}.
 *
 * @param <T>   The type of the object to be returned.
 */
public abstract class AbstractVTNFuture<T> implements VTNFuture<T> {
    /**
     * The execution list to hold executors.
     */
    private final ExecutionList  executionList = new ExecutionList();

    /**
     * Return a {@link VTNException} which represents the cause of failure.
     *
     * @param t  A {@link Throwable}.
     * @return  A {@link VTNException} which represents the given throwable.
     */
    public static final VTNException getException(Throwable t) {
        Throwable cause = t;
        while (cause instanceof ExecutionException) {
            Throwable c = cause.getCause();
            if (c == null) {
                break;
            }
            cause = c;
        }

        VTNException converted;
        if (cause instanceof VTNException) {
            converted = (VTNException)cause;
        } else {
            Status status;

            if (cause instanceof OptimisticLockFailedException) {
                status = new Status(StatusCode.CONFLICT, cause.getMessage());
            } else if (cause instanceof TimeoutException) {
                status = new Status(StatusCode.TIMEOUT, cause.getMessage());
            } else if (cause instanceof InterruptedException) {
                String msg = cause.getMessage();
                if (msg == null) {
                    msg = "Interrupted.";
                }
                status = new Status(StatusCode.INTERNALERROR, msg);
            } else if (cause != null) {
                status = new Status(StatusCode.INTERNALERROR,
                                    cause.getMessage());
            } else {
                status = new Status(StatusCode.INTERNALERROR,
                                    "Failed to wait for the computation.");
            }

            converted = new VTNException(status, cause);
        }

        return converted;
    }

    /**
     * Execute registered listeners.
     *
     * <p>
     *   Subclass must call this method when the computation is complete.
     * </p>
     */
    protected final void done() {
        executionList.execute();
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

    // VTNFuture

    /**
     * {@inheritDoc}
     */
    @Override
    public final T checkedGet() throws VTNException {
        try {
            return get();
        } catch (Exception e) {
            throw getException(e);
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
            throw getException(e);
        }
    }
}
