/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.CancellationException;

/**
 * An implementation of {@link VTNFuture} that cancels the task immediately.
 *
 * @param <T>   The type of the object to be returned.
 */
public final class CanceledFuture<T> extends AbstractVTNFuture<T> {
    /**
     * Attempt to cancel execution of the task.
     *
     * @param intr  Unused.
     * @return  {@code true}.
     */
    @Override
    public boolean cancel(boolean intr) {
        return true;
    }

    /**
     * Determine whether the task was canceled before it completed normally.
     *
     * @return  {@code true}.
     */
    @Override
    public boolean isCancelled() {
        return true;
    }

    /**
     * Determine whether the task completed or not.
     *
     * @return  {@code true}.
     */
    @Override
    public boolean isDone() {
        return true;
    }

    /**
     * A {@link CancellationException} is always thrown.
     *
     * @return  Never returns.
     * @throws CancellationException  Always thrown.
     */
    @Override
    public T get() {
        throw newException();
    }

    /**
     * A {@link CancellationException} is always thrown.
     *
     * @param timeout  Unused.
     * @param unit     Unused.
     * @return  Never returns.
     * @throws CancellationException  Always thrown.
     */
    public T get(long timeout, TimeUnit unit) {
        throw newException();
    }

    /**
     * Return a new {@link CancellationException} instance.
     *
     * @return  A {@link CancellationException} instance.
     */
    private CancellationException newException() {
        return new CancellationException("Already canceled");
    }
}
