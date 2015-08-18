/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicInteger;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.FutureCallback;

import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.concurrent.AbstractVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.FutureSynchronizer;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;

/**
 * An implementation of {@link VTNFuture} used to wait for completion of all
 * MD-SAL transaction tasks, including background tasks started by the target
 * task.
 *
 * <p>
 *   This class returns the value returned by the specified MD-SAL transaction
 *   task. Note that this future returns a value as long as the specified
 *   MD-SAL transaction has successfully completed.
 * </p>
 * <p>
 *   Cancellation of the target MD-SAL transaction task is not supported.
 *   {@link #cancel(boolean)} only cancels a rendezvous with additional
 *   background tasks started by the target MD-SAL transaction task.
 * </p>
 *
 * @param <T>  The type of the object to be returned by the specified
 *             MD-SAL transaction task.
 */
public final class TxSyncFuture<T> extends AbstractVTNFuture<T> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(TxSyncFuture.class);

    /**
     * The synchronizer for the future.
     */
    private final Synchronizer<T>  synchronizer;

    /**
     * {@code Synchronizer} implements the synchronization for
     * {@link TxSyncFuture}.
     *
     * @param <T>  The type of the object to be returned.
     */
    private final class Synchronizer<T> extends FutureSynchronizer<T>
        implements FutureCallback<T> {
        /**
         * Version number for serialization.
         */
        private static final long  serialVersionUID = 1L;

        /**
         * Internal state that indicates the future is about to change its
         * state to {@link #STATE_WAITING}.
         * This implies that the target task has already completed
         * successfully.
         */
        private static final int  STATE_TRANS_WAITING = 0x2;

        /**
         * Internal state that indicates the future is waiting for completion
         * of background tasks.
         * This implies that the target task has already completed
         * successfully.
         */
        private static final int  STATE_WAITING = 0x4;

        /**
         * Internal state that indicates a rendezvous with background tasks
         * has been canceled.
         * This implies that the target task is still running.
         */
        private static final int  STATE_WAIT_CANCELED = 0x8;

        /**
         * The number of background tasks to wait.
         */
        private final AtomicInteger  numberOfTasks = new AtomicInteger();

        /**
         * The target MD-SAL transaction task.
         */
        private final TxTask<T>  targetTask;

        /**
         * Construct a new instance.
         *
         * @param task  A {@link TxTask} instance.
         * @param f     A {@link VTNFuture} associated with {@code task}.
         */
        private Synchronizer(TxTask<T> task, VTNFuture<T> f) {
            targetTask = task;
            Futures.addCallback(f, this);
        }

        /**
         * Return the target task.
         *
         * @return  The target task.
         */
        private TxTask<T> getTargetTask() {
            return targetTask;
        }

        /**
         * Complete the future as succeeded.
         *
         * <p>
         *   The caller must guarantee that the result of the target task
         *   is already saved.
         * </p>
         *
         * @param expected  The expected internal state.
         * @return  {@code true} on success.
         *          {@code false} if another thread completed the task.
         */
        private boolean succeeded(int expected) {
            // Change the internal state to COMPLETING.
            boolean ret = compareAndSetState(expected, STATE_COMPLETING);
            if (ret) {
                // Release the shared lock.
                releaseShared(STATE_COMPLETED);
            } else {
                awaitCompletion(getState());
            }

            return ret;
        }

        /**
         * Change the internal state for the synchronization of background
         * tasks.
         *
         * @param result    The result of the target task.
         * @param expected  The expected internal state.
         * @param newState  The internal state to be set.
         * @return  {@code true} if the future state has been changed as
         *          completed. Otherwise {@code false}.
         */
        private boolean setStateForWait(int expected, int newState, T result) {
            // Try to change state to the new state.
            if (!compareAndSetState(expected, newState)) {
                int state = getState();
                if (state == STATE_WAIT_CANCELED) {
                    // A rendezvous with background tasks has already been
                    // canceled. So we need to complete the future here.
                    return done(state, STATE_COMPLETED, result, null);
                }

                // This should never happen.
                throw unexpectedState(state);
            }

            return false;
        }

        /**
         * Set the result of the target task associated with the future.
         *
         * @param result  The result of the target task.
         * @return  {@code true} if the future state has been changed as
         *          completed. Otherwise {@code false}.
         */
        private boolean set(T result) {
            // Try to change state to TRANS_WAITING.
            if (setStateForWait(STATE_EXECUTING, STATE_TRANS_WAITING,
                                result)) {
                return true;
            }

            // Determine background tasks to wait.
            List<VTNFuture<?>> list = targetTask.getBackgroundTasks();
            int size = list.size();
            if (size <= 0) {
                // Complete the future.
                return done(STATE_TRANS_WAITING, STATE_COMPLETED, result,
                            null);
            }

            numberOfTasks.set(size);
            setResult(result, null);

            // Try to change state to WAITING.
            if (setStateForWait(STATE_TRANS_WAITING, STATE_WAITING, result)) {
                return true;
            }

            LOG.trace("Configure the background task synchronization: " +
                      "target={}, size={}", targetTask, size);

            // Add callbacks to detect completion of background tasks.
            FutureCallback<Object> cb = new FutureCallback<Object>() {
                @Override
                public void onSuccess(Object result) {
                    LOG.trace("Backgrond task for MD-SAL transaction has " +
                              "completed: target={}, result={}", targetTask,
                              result);
                    onBackgroundCompleted();
                }

                @Override
                public void onFailure(Throwable cause) {
                    LOG.warn("Background task for MD-SAL transaction has " +
                             "failed: target=" + targetTask, cause);
                    onBackgroundCompleted();
                }
            };

            for (VTNFuture<?> f: list) {
                Futures.addCallback(f, cb);
            }

            return false;
        }

        /**
         * Invoked when a background task has completed.
         */
        private void onBackgroundCompleted() {
            // Try to complete the future.
            if (numberOfTasks.decrementAndGet() == 0 &&
                succeeded(STATE_WAITING)) {
                LOG.trace("All the background tasks have completed: {}",
                          targetTask);
                TxSyncFuture.this.done();
            }
        }

        /**
         * Cancel a rendezvous with background tasks started by the target
         * task.
         *
         * @return  {@code true} if the future state has been changed as
         *          completed. Otherwise {@code false}.
         */
        private boolean cancel() {
            int state;

            do {
                state = getState();
                if (state >= STATE_WAIT_CANCELED) {
                    // Already canceled or completed.
                    break;
                }
                if (state == STATE_WAITING) {
                    // The result of the target future has already been saved.
                    // So we can complete this future here.
                    return succeeded(state);
                }
            } while (!compareAndSetState(state, STATE_WAIT_CANCELED));

            return false;
        }

        // FutureCallback

        /**
         * Invoked when the target task has successfuly completed.
         *
         * @param result  The result of the target task.
         */
        @Override
        public void onSuccess(T result) {
            if (set(result)) {
                LOG.trace("No background tasks to wait: {}", targetTask);
                TxSyncFuture.this.done();
            }
        }

        /**
         * Invoked when the target task has failed.
         *
         * @param cause  A {@link Throwable} that indicates the cause of
         *               failure.
         */
        @Override
        public void onFailure(Throwable cause) {
            int state = getState();
            if ((state & (STATE_EXECUTING | STATE_WAIT_CANCELED)) != 0 &&
                done(state, STATE_COMPLETED, null, cause)) {
                TxSyncFuture.this.done();
            }
        }
    }

    /**
     * Construct a new instance.
     *
     * @param task  A {@link TxTask} instance.
     * @param f     A {@link VTNFuture} associated with {@code task}.
     */
    public TxSyncFuture(TxTask<T> task, VTNFuture<T> f) {
        synchronizer = new Synchronizer<T>(task, f);
    }

    // Future

    /**
     * Attempt to cancel execution of the task.
     *
     * <p>
     *   Note that this method cancels only a rendezvous with additional
     *   background tasks, and never cancels the target MD-SAL transaction
     *   task.
     * </p>
     *
     * @param intr  Unused because this future is designed to run using
     *              callback.
     * @return  {@code false} is always returned because the target MD-SAL
     *          transaction task will never be canceled.
     */
    @Override
    public boolean cancel(boolean intr) {
        if (synchronizer.cancel()) {
            LOG.trace("Background task synchronization has been canceled: ",
                      synchronizer.getTargetTask());
            done();
        }

        return false;
    }

    /**
     * Determine whether the task was canceled before it completed normally.
     *
     * @return  {@code false} is always returned because the target MD-SAL
     *          transaction task will never be canceled.
     */
    @Override
    public boolean isCancelled() {
        return false;
    }

    /**
     * Determine whether the task completed or not.
     *
     * @return  {@code true} only if the task completed.
     */
    @Override
    public boolean isDone() {
        return synchronizer.isDone();
    }

    /**
     * Wait for the task to complete, and then return the result of the task.
     *
     * @return  The result of the task.
     * @throws InterruptedException
     *    The current thread was interrupted while waiting.
     * @throws ExecutionException
     *    The task caught an exception.
     */
    @Override
    public T get() throws InterruptedException, ExecutionException {
        return synchronizer.get();
    }

    /**
     * Wait for the task to complete, and then return the result of the task.
     *
     * @param timeout  The maximum time to wait
     * @param unit     The time unit of {@code timeout}.
     * @return  The result of the task.
     * @throws InterruptedException
     *    The current thread was interrupted while waiting.
     * @throws ExecutionException
     *    The task caught an exception.
     * @throws TimeoutException
     *    The task did not complete within the given timeout.
     */
    @Override
    public T get(long timeout, TimeUnit unit)
        throws InterruptedException, ExecutionException, TimeoutException {
        return synchronizer.get(unit.toNanos(timeout));
    }
}
