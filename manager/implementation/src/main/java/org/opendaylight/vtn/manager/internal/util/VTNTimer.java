/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Timer;
import java.util.TimerTask;
import java.util.Date;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;

/**
 * {@code VTNTimer} describes a thread to schedule timer tasks.
 */
public final class VTNTimer extends Timer {
    /**
     * A timer task used only for the timer task synchronization.
     */
    private static final class SyncTimerTask extends TimerTask {
        /**
         * A future used to notify the completion of this task.
         */
        private final SettableVTNFuture<Void>  future;

        /**
         * Construct a new instance.
         *
         * @param f  A {@link SettableVTNFuture} instance used to notify
         *           the completion of this task.
         */
        private SyncTimerTask(SettableVTNFuture<Void> f) {
            future = f;
        }

        // Runnable

        /**
         * Execute this task on the timer thread.
         */
        @Override
        public void run() {
            future.set(null);
        }
    }

    /**
     * Keep {@code true} while this timer thread is available.
     */
    private final AtomicBoolean  available = new AtomicBoolean(true);

    /**
     * Construct a new timer thread associated with the given name.
     *
     * @param name  THe name of the timer thread.
     */
    public VTNTimer(String name) {
        super(name);
    }

    /**
     * Shut down the timer.
     *
     * <p>
     *   Tasks previously scheduled are not canceled, but no more timer task
     *   will be scheduled.
     * </p>
     *
     * @return  {@code true} if the timer shutdown has been initiated.
     *          {@code false} if the timer is already down.
     */
    public boolean shutdown() {
        return available.getAndSet(false);
    }

    /**
     * Determine whether this timer is available or not.
     *
     * @return  {@code true} only if this timer is available.
     */
    public boolean isAvailable() {
        return available.get();
    }

    /**
     * Flush the task queue for this timer.
     *
     * <p>
     *   This method can be used even if the timer shut down is already
     *   initiated by {@link #shutdown()}.
     * </p>
     *
     * @param timeout  The timeout to wait for all tasks to complete.
     * @param unit     The time unit for {@code timeout} argument.
     * @return  {@code true} if all tasks previously scheduled completed.
     *          {@code false} if this timer is already canceled.
     * @throws VTNException  An error occurred.
     */
    public boolean flush(long timeout, TimeUnit unit) throws VTNException {
        SettableVTNFuture<Void> future = new SettableVTNFuture<>();
        SyncTimerTask task = new SyncTimerTask(future);
        try {
            super.schedule(task, 0);
        } catch (IllegalStateException e) {
            // Already canceled.
            return false;
        }

        future.checkedGet(timeout, unit);
        return true;
    }

    /**
     * Schedule the given task to be executed after the given delay.
     *
     * @param task   A task to be executed.
     * @param delay  The number of milliseconds in delay before execution.
     * @return  {@code true} on success.
     *          {@code false} if this timer is unavailable.
     */
    public boolean checkedSchedule(TimerTask task, long delay) {
        boolean ret = available.get();
        if (ret) {
            try {
                super.schedule(task, delay);
            } catch (IllegalStateException e) {
                ret = false;
            }
        }

        return ret;
    }

    // Timer

    /**
     * Schedule the given task to be executed after the given delay.
     *
     * @param task   A task to be executed.
     * @param delay  The number of milliseconds in delay before execution.
     */
    @Override
    public void schedule(TimerTask task, long delay) {
        if (available.get()) {
            super.schedule(task, delay);
        }
    }

    /**
     * Schedule the given task to be executed at the given time.
     *
     * @param task  A task to be executed.
     * @param time  A {@link Date} instance which specifies the time to
     *              execute the task.
     */
    @Override
    public void schedule(TimerTask task, Date time) {
        if (available.get()) {
            super.schedule(task, time);
        }
    }

    /**
     * Schedule the given task to be executed periodically.
     *
     * @param task    A task to be executed.
     * @param delay   The number of milliseconds to be inserted before the
     *                first execution.
     * @param period  The number of milliseconds between successive task
     *                executions.
     */
    @Override
    public void schedule(TimerTask task, long delay, long period) {
        if (available.get()) {
            super.schedule(task, delay, period);
        }
    }

    /**
     * Schedule the given task to be executed periodically.
     *
     * @param task       A task to be executed.
     * @param firstTime  The time of the first execution.
     * @param period     The number of milliseconds between successive task
     *                   executions.
     */
    @Override
    public void schedule(TimerTask task, Date firstTime, long period) {
        if (available.get()) {
            super.schedule(task, firstTime, period);
        }
    }

    /**
     * Schedule the given task to be executed periodically at the fixed rate.
     *
     * @param task    A task to be executed.
     * @param delay   The number of milliseconds to be inserted before the
     *                first execution.
     * @param period  The number of milliseconds between successive task
     *                executions.
     */
    @Override
    public void scheduleAtFixedRate(TimerTask task, long delay, long period) {
        if (available.get()) {
            super.scheduleAtFixedRate(task, delay, period);
        }
    }

    /**
     * Schedule the given task to be executed periodically at the fixed rate.
     *
     * @param task       A task to be executed.
     * @param firstTime  The time of the first execution.
     * @param period     The number of milliseconds between successive task
     *                   executions.
     */
    @Override
    public void scheduleAtFixedRate(TimerTask task, Date firstTime,
                                    long period) {
        if (available.get()) {
            super.scheduleAtFixedRate(task, firstTime, period);
        }
    }

    /**
     * Cancel all tasks previously scheduled, and terminate this timer thread.
     */
    @Override
    public void cancel() {
        available.set(false);
        super.cancel();
    }
}
