/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

/**
 * A task that inserts delay before the execution.
 */
public abstract class DelayedTask implements Runnable {
    /**
     * The number of milliseconds to be inserted before the execution.
     */
    private final long  delay;

    /**
     * Set true if the task was interrupted.
     */
    private boolean  interrupted;

    /**
     * Construct a new instance.
     *
     * @param msec  The number of milliseconds to be inserted before the
     *              execution.
     */
    public DelayedTask(long msec) {
        delay = msec;
    }

    /**
     * Determine whether this task was interrupted or not.
     *
     * @return  {@code true} only if this task was interrupted.
     */
    public final boolean wasInterrupted() {
        return interrupted;
    }

    /**
     * Execute the task.
     */
    public abstract void execute();

    // Runnable

    /**
     * Execute the task on a thread.
     */
    @Override
    public final void run() {
        try {
            Thread.sleep(delay);
            execute();
        } catch (InterruptedException e) {
            interrupted = true;
        }
    }
}
