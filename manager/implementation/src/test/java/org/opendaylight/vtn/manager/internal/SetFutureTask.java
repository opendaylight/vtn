/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;

/**
 * A task that sets the result for the specified future.
 *
 * @param <T>  The type of the result of the future.
 */
public final class SetFutureTask<T> extends DelayedTask {
    /**
     * The target future.
     */
    private final SettableVTNFuture<T>  targetFuture;

    /**
     * The result to be set.
     */
    private final T  result;

    /**
     * Construct a new instance.
     *
     * @param future  The target future.
     * @param res     The result to be set.
     */
    public SetFutureTask(SettableVTNFuture<T> future, T res, long msec) {
        super(msec);
        targetFuture = future;
        result = res;
    }

    // DelayedTask

    /**
     * Set the result of the target future.
     */
    @Override
    public void execute() {
        targetFuture.set(result);
    }
}
