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
import java.util.concurrent.CancellationException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.FutureCallback;

/**
 * {@code FutureCallbackTask} is an executable task that invokes
 * {@link FutureCallback} instance when an non-listenable future completes.
 *
 * @param <T>  The type of the object returned by the future.
 */
public final class FutureCallbackTask<T> implements Runnable {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(FutureCallbackTask.class);

    /**
     * The number of milliseconds to wait for the future by sync wait.
     */
    private static final long  FUTURE_SYNC_TIMEOUT = 1000;

    /**
     * The number of milliseconds between executions of the polling timer task.
     */
    private static final long  POLLING_INTERVAL = 3000;

    /**
     * The default value for the number of pollings.
     */
    private static final int  DEFAULT_MAX_POLLS = 10;
    /**
     * The future to wait for completion.
     */
    private final Future<T>  future;

    /**
     * The future callback to be invoked.
     */
    private final FutureCallback<? super T>  callback;

    /**
     * The timer thread used for polling.
     */
    private final Timer  timer;

    /**
     * The number of pollings.
     */
    private final int  maxPolls;

    /**
     * A timer task to wait for the completion of the future.
     */
    private class PollingTask extends TimerTask {
        /**
         * The number of milliseconds to poll the completion of the future.
         */
        private static final long  POLL_TIMEOUT = 1;

        /**
         * The number of times for polling.
         */
        private int  counter;

        /**
         * Invoke future callback only if the future already completed.
         */
        @Override
        public void run() {
            if (getResult(POLL_TIMEOUT, TimeUnit.MILLISECONDS)) {
                return;
            }

            counter++;
            if (counter >= getMaxPolls()) {
                // Cancel the future.
                long elapsed = (POLLING_INTERVAL * counter) +
                    FUTURE_SYNC_TIMEOUT;
                FutureCallbackTask.this.cancel(elapsed);
                cancel();
            }
        }
    }

    /**
     * Construct a new instance with specifying 10 as the number of pollings.
     *
     * @param f   A future to wait for completion.
     * @param cb  A future callback to be invoked.
     * @param t   A timer used for polling.
     */
    public FutureCallbackTask(Future<T> f, FutureCallback<? super T> cb,
                              Timer t) {
        this(f, cb, t, DEFAULT_MAX_POLLS);
    }

    /**
     * Construct a new instance.
     *
     * @param f       A future to wait for completion.
     * @param cb      A future callback to be invoked.
     * @param t       A timer used for polling.
     * @param npolls  The maximum number of pollings.
     */
    public FutureCallbackTask(Future<T> f, FutureCallback<? super T> cb,
                              Timer t, int npolls) {
        future = f;
        callback = cb;
        timer = t;
        maxPolls = npolls;
    }

    /**
     * Return the maximum number of future pollings.
     *
     * @return  The maxumum number of pollings.
     */
    public int getMaxPolls() {
        return maxPolls;
    }

    // Runnable

    /**
     * Wait for the completion of the future on a thread.
     */
    @Override
    public void run() {
        // Wait with specifying timeout.
        if (getResult(FUTURE_SYNC_TIMEOUT, TimeUnit.MILLISECONDS)) {
            return;
        }

        if (timer == null) {
            cancel(FUTURE_SYNC_TIMEOUT);
        } else {
            LOG.debug("Switch to polling mode: future={}, callback={}",
                      future, callback);
            PollingTask task = new PollingTask();
            timer.schedule(task, POLLING_INTERVAL, POLLING_INTERVAL);
        }
    }

    /**
     * Try to get the result of the future.
     *
     * @param timeout  The maximum time to wait.
     * @param unit     The time unit of the {@code timeout} argument.
     * @return  {@code true} only if the future completed its computation.
     */
    private boolean getResult(long timeout, TimeUnit unit) {
        try {
            T ret = future.get(timeout, unit);
            callback.onSuccess(ret);
            return true;
        } catch (TimeoutException e) {
            // The future is still running.
        } catch (Exception e) {
            callback.onFailure(e);
        }

        return false;
    }

    /**
     * Cancel the future.
     *
     * @param elapsed  The number of milliseconds elapsed by the wait.
     */
    private void cancel(long elapsed) {
        boolean canceled = future.cancel(true);
        String msg = new StringBuilder("Future did not complete within ").
            append(elapsed).append(" msecs").toString();
        LOG.error("{}: future={}, callback={}, canceled={}",
                  msg, future, callback, canceled);
        if (canceled) {
            CancellationException e = new CancellationException(msg);
            callback.onFailure(e);
        }
    }
}
