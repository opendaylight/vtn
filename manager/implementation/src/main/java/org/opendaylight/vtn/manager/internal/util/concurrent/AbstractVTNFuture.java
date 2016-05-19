/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.util.concurrent.ExecutionList;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.md.sal.common.api.data.OptimisticLockFailedException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

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
     * Return a throwable that indicates the actual cause of failure.
     *
     * @param t  A {@link Throwable}.
     * @return  A {@link Throwable} instance that indicates the actual cause
     *          of failure.
     */
    public static final Throwable getCause(Throwable t) {
        Throwable cause = t;
        while (cause instanceof ExecutionException) {
            Throwable c = cause.getCause();
            if (c == null) {
                break;
            }
            cause = c;
        }

        return cause;
    }

    /**
     * Return a {@link VTNException} which represents the cause of failure.
     *
     * @param t  A {@link Throwable}.
     * @return  A {@link VTNException} which represents the given throwable.
     */
    public static final VTNException getException(Throwable t) {
        Throwable cause = getCause(t);
        VTNException converted;
        if (cause instanceof VTNException) {
            converted = (VTNException)cause;
        } else {
            String msg;
            VtnErrorTag etag = null;

            if (cause instanceof OptimisticLockFailedException) {
                etag = VtnErrorTag.CONFLICT;
                msg = cause.getMessage();
            } else if (cause instanceof TimeoutException) {
                etag = VtnErrorTag.TIMEOUT;
                msg = cause.getMessage();
            } else if (cause instanceof InterruptedException) {
                msg = cause.getMessage();
                if (msg == null) {
                    msg = "Interrupted.";
                }
            } else if (cause != null) {
                msg = cause.getMessage();
            } else {
                msg = "Failed to wait for the computation.";
            }

            converted = new VTNException(etag, msg, cause);
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
