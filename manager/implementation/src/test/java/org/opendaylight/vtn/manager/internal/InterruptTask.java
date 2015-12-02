/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

/**
 * A task that interrupts the specified thread.
 */
public final class InterruptTask extends DelayedTask {
    /**
     * The target thread.
     */
    private final Thread  targetThread;

    /**
     * Construct a new instance.
     *
     * @param thr   The target thread.
     * @param msec  The number of milliseconds to be inserted before
     *              interruption.
     */
    public InterruptTask(Thread thr, long msec) {
        super(msec);
        targetThread = thr;
    }

    // DelayedTask

    /**
     * Interrupt the target thread.
     */
    @Override
    public void execute() {
        targetThread.interrupt();
    }
}
