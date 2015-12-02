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
 * A task that sets the specified exception to the specified future.
 */
public final class FailFutureTask extends DelayedTask {
    /**
     * The target {@link SettableVTNFuture}.
     */
    private final SettableVTNFuture<?>  targetVTNFuture;

    /**
     * The cause of the failure to be set.
     */
    private final Throwable  cause;

    /**
     * Construct a new instance.
     *
     * @param future  The target future.
     * @param t       A throwable that indicates the cause of failure.
     * @param msec    The number of milliseconds to be inserted before
     *                setting the exception.
     */
    public FailFutureTask(SettableVTNFuture<?> future, Throwable t, long msec) {
        super(msec);
        targetVTNFuture = future;
        cause = t;
    }

    // DelayedTask

    /**
     * Set the cause of failure to the target future.
     */
    @Override
    public void execute() {
        targetVTNFuture.setException(cause);
    }
}
