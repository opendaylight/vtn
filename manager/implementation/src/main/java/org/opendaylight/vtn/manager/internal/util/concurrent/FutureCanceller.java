/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.Timer;
import java.util.TimerTask;

import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;

/**
 * An implementation of {@link Runnable} that cancels the task associated with
 * the given {@link ListenableFuture} instance.
 *
 * @param <T>  The type of the result returned by the target task.
 */
public final class FutureCanceller<T> extends TimerTask
    implements FutureCallback<T> {
    /**
     * The target task.
     */
    private final ListenableFuture<T>  targetTask;

    /**
     * Determine whether the target task should be interrupted or not.
     */
    private final boolean  needInterrupt;

    /**
     * Set a future canceller that cancels the given future without
     * interrupting the thread.
     *
     * @param timer    A {@link Timer} instance to execute the timer.
     * @param timeout  The number of milliseconds to wait for completion of
     *                 the target task.
     * @param f        A {@link ListenableFuture} instance associated with the
     *                 task to wait.
     * @param <T>      The type of the result returned by the target task.
     */
    public static <T> void set(Timer timer, long timeout,
                               ListenableFuture<T> f) {
        set(timer, timeout, f, false);
    }

    /**
     * Set a future canceller that cancels the given future.
     *
     * @param timer    A {@link Timer} instance to execute the timer.
     * @param timeout  The number of milliseconds to wait for completion of
     *                 the target task.
     * @param f        A {@link ListenableFuture} instance associated with the
     *                 task to wait.
     * @param intr     {@code true} if the task runner thread should be
     *                 interrupted. Otherwise in-progress task are allowed
     *                 to complete.
     * @param <T>      The type of the result returned by the target task.
     */
    public static <T> void set(Timer timer, long timeout,
                               ListenableFuture<T> f, boolean intr) {
        if (!f.isDone()) {
            FutureCanceller<T> canceller = new FutureCanceller<T>(f, intr);
            timer.schedule(canceller, timeout);
            Futures.addCallback(f, canceller);
        }
    }

    /**
     * Construct a new instance that cancels the given
     * {@link ListenableFuture}.
     *
     * @param f     A {@link ListenableFuture} instance to be canceled.
     * @param intr  {@code true} if the task runner thread should be
     *              interrupted. Otherwise in-progress task are allowed
     *              to complete.
     */
    private FutureCanceller(ListenableFuture<T> f, boolean intr) {
        targetTask = f;
        needInterrupt = intr;
    }

    // Runnable

    /**
     * Cancel the specified task.
     */
    @Override
    public void run() {
        targetTask.cancel(needInterrupt);
    }

    // FutureCallback

    /**
     * Invoked when the target task has successfully completed.
     *
     * <p>
     *   This method only cancels this timer task.
     * </p>
     *
     * @param result  The result of the target task.
     */
    @Override
    public void onSuccess(T result) {
        cancel();
    }

    /**
     * Invoked when the taret task has failed.
     *
     * <p>
     *   This method only cancels this timer task.
     * </p>
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    @Override
    public void onFailure(Throwable cause) {
        cancel();
    }
}
