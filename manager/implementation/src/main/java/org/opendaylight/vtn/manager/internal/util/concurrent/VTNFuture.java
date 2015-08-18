/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.TimeUnit;

import com.google.common.util.concurrent.CheckedFuture;

import org.opendaylight.vtn.manager.VTNException;

/**
 * A {@code VTNFuture} is a {@link CheckedFuture} that wraps exception
 * in a {@link VTNException}.
 *
 * @param <T>   The type of the object to be returned.
 */
public interface VTNFuture<T> extends CheckedFuture<T, VTNException> {
    /**
     * Wait for the computation to complete, and then return the result.
     *
     * <p>
     *   An exception will be always wrapped by a {@link VTNException}.
     * </p>
     *
     * @return The result of the computation.
     * @throws VTNException   An error occurred.
     */
    @Override
    T checkedGet() throws VTNException;

    /**
     * Wait for the computation to complete within the given timeout, and then
     * return the result.
     * <p>
     *   An exception will be always wrapped by a {@link VTNException},
     *   including {@link java.util.concurrent.TimeoutException}.
     * </p>
     *
     * @param timeout  The maximum time to wait.
     * @param unit     The time unit of {@code timeout} argument.
     * @return The result of the computation.
     * @throws VTNException   An error occurred.
     */
    @Override
    T checkedGet(long timeout, TimeUnit unit) throws VTNException;
}
