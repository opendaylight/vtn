/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

/**
 * A task that executes the specified runnable after delay.
 */
public final class DelayedRunnableTask extends DelayedTask {
    /**
     * The runnable to execute.
     */
    private final Runnable  runnable;

    /**
     * Construct a new instance.
     *
     * @param r     The runnable to execute.
     * @param msec  The number of milliseconds to be inserted before the
     *              execution.
     */
    public DelayedRunnableTask(Runnable r, long msec) {
        super(msec);
        runnable = r;
    }

    // DelayedTask

    /**
     * Execute the specified runnable.
     */
    @Override
    public void execute() {
        runnable.run();
    }
}
