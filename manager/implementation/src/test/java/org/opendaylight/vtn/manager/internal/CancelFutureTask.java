/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static org.junit.Assert.assertEquals;

import java.util.concurrent.Future;

/**
 * A task that cancels the specified future.
 */
public final class CancelFutureTask extends DelayedTask {
    /**
     * The target future.
     */
    private final Future<?>  targetFuture;

    /**
     * Expected result of the cancellation.
     */
    private final boolean  expectedResult;

    /**
     * Construct a new instance.
     *
     * @param future  The target future.
     * @param msec    The number of milliseconds to be inserted before
     *                cancellation.
     */
    public CancelFutureTask(Future<?> future, long msec) {
        this(future, true, msec);
    }

    /**
     * Construct a new instance.
     *
     * @param future  The target future.
     * @param res     The expected result of the cancellation.
     * @param msec    The number of milliseconds to be inserted before
     *                cancellation.
     */
    public CancelFutureTask(Future<?> future, boolean res, long msec) {
        super(msec);
        targetFuture = future;
        expectedResult = res;
    }

    // DelayedTask

    /**
     * Cancel the target future.
     */
    @Override
    public void execute() {
        assertEquals(expectedResult, targetFuture.cancel(true));
    }
}
