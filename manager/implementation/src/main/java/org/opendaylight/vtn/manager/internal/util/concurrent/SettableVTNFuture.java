/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * An implementation of {@link VTNFuture} that requires the task result to be
 * set externally.
 *
 * @param <T>   The type of the object to be returned.
 */
public class SettableVTNFuture<T> extends AbstractVTNFuture<T> {
    /**
     * The synchronizer for the future.
     */
    private final Synchronizer<T>  synchronizer = new Synchronizer<T>();

    /**
     * {@code Synchronizer} implements the synchronization for
     * {@link SettableVTNFuture}.
     *
     * @param <T>  The type of the object to be returned.
     */
    private static final class Synchronizer<T> extends FutureSynchronizer<T> {
        /**
         * Version number for serialization.
         */
        private static final long  serialVersionUID = 1L;

        /**
         * Internal state that indicates the cancellation is temporarily
         * masked. This implies that the task is still executing.
         */
        private static final int  STATE_CANCEL_MASKED = 0x2;

        /**
         * Internal state that indicates a cancel request is pending.
         * This implies that the task is still executing and cancellation
         * is masked.
         */
        private static final int  STATE_CANCEL_PENDING = 0x4;

        /**
         * A bitmask for internal state used to test whether the task is
         * executing with cancellation mask.
         */
        private static final int  STATE_MASK_NOCANCEL =
            STATE_CANCEL_MASKED | STATE_CANCEL_PENDING;

        /**
         * Construct a new instance.
         */
        private Synchronizer() {
        }

        /**
         * Mask cancellation temporarily.
         *
         * <p>
         *   This method is expected to be called on the thread that is
         *   executing the task. This method has no effect if the task is
         *   already completed successfully.
         * </p>
         *
         * @return  {@code true} on success.
         *          {@code false} if the task has already completed.
         * @throws InterruptedException
         *    The task has already been canceled.
         */
        private boolean maskCancel() throws InterruptedException {
            for (;;) {
                int state = getState();
                if (state == STATE_EXECUTING) {
                    // Try to change state to CANCEL_MASKED.
                    if (compareAndSetState(state, STATE_CANCEL_MASKED)) {
                        break;
                    }

                    // Lost the race.
                    continue;
                }

                if ((state & STATE_MASK_NOCANCEL) != 0) {
                    // Already masked.
                    break;
                }
                if (state == STATE_COMPLETED) {
                    // Already completed.
                    return false;
                }
                if (state == STATE_CANCELED) {
                    // Already canceled.
                    throw interrupted();
                }

                awaitCompletion(state);
            }

            return true;
        }

        /**
         * Unmask cancellation masked by the previous call of
         * {@link #maskCancel()}.
         *
         * <p>
         *   This method is expected to be called on the thread that is
         *   executing the task. This method has no effect if the task is
         *   already completed successfully.
         * </p>
         *
         * @param future  A {@link SettableVTNFuture} instance.
         * @return  {@code true} on success.
         *          {@code false} if the task has already completed.
         * @throws InterruptedException
         *    The task has already been canceled.
         */
        private boolean unmaskCancel(SettableVTNFuture future)
            throws InterruptedException {
            boolean result = true;
            for (;;) {
                int state = getState();
                if (state == STATE_CANCEL_MASKED) {
                    // Try to change state to EXECUTING.
                    if (compareAndSetState(state, STATE_EXECUTING)) {
                        break;
                    }

                    // Lost the race.
                    continue;
                }

                if (state == STATE_EXECUTING) {
                    // Not masked.
                    break;
                }
                if (state == STATE_COMPLETED) {
                    // Already completed.
                    result = false;
                    break;
                }
                if (state == STATE_CANCEL_PENDING) {
                    // Activate pending cancel request.
                    activatePendingCancel(future);

                    // Lost the race.
                    continue;
                }
                if (state == STATE_CANCELED) {
                    // Already canceled.
                    throw interrupted();
                }

                awaitCompletion(state);
            }

            return result;
        }

        /**
         * Activate pending cancel request.
         *
         * <p>
         *   This method must be called only if the current state is
         *   {@link #STATE_CANCEL_PENDING}.
         * </p>
         *
         * @param future  A {@link SettableVTNFuture} instance.
         * @throws InterruptedException
         *    The task has been canceled.
         */
        private void activatePendingCancel(SettableVTNFuture future)
            throws InterruptedException {
            InterruptedException e = interrupted();
            if (done(STATE_CANCEL_PENDING, STATE_CANCELED, null, e)) {
                future.onCanceled();
                throw e;
            }
        }

        /**
         * Attempt to cancel execution of the task.
         *
         * @param intr  {@code true} if the task runner thread should be
         *              interrupted. Otherwise in-progress task are allowed
         *              to complete.
         * @return  {@code true} only if the task was canceled.
         */
        private boolean cancel(boolean intr) {
            // Try to change the internal state to CANCELED.
            boolean ret = done(STATE_EXECUTING, STATE_CANCELED, null,
                               interrupted());
            if (!ret) {
                // Change state to CANCEL_PENDING if the cancellation is
                // masked.
                compareAndSetState(STATE_CANCEL_MASKED, STATE_CANCEL_PENDING);
            } else if (intr) {
                // Interrupt the thread executing the task if requested.
                interruptTask();
            }

            return ret;
        }

        /**
         * Set the result of the task associated with the future.
         *
         * @param result  A value returned by the task.
         * @return  {@code true} if the future state has been changed.
         *          {@code false} if the task already completed.
         */
        private boolean set(T result) {
            return done(STATE_COMPLETED, result, null);
        }

        /**
         * Set a {@link Throwable} that indicates the cause of failure.
         *
         * @param cause  A {@link Throwable} instance.
         * @return  {@code true} if the future state has been changed.
         *          {@code false} if the task already completed.
         */
        private boolean setException(Throwable cause) {
            return done(STATE_COMPLETED, null, cause);
        }

        /**
         * Complete the future.
         *
         * <p>
         *   This method always clears pending cancel request.
         * </p>
         *
         * @param state  The internal state to be set.
         * @param value  The value to be returned by the future.
         * @param cause  The {@link Throwable} that indicates the cause of
         *               failure.
         * @return  {@code true} on success.
         *          {@code false} if another thread completed the task.
         */
        private boolean done(int state, T value, Throwable cause) {
            int cur = getState();
            if (cur <= STATE_CANCEL_PENDING) {
                return done(cur, state, value, cause);
            }

            return false;
        }
    }

    /**
     * Set a thread that is executing the task.
     *
     * @param t  A {@link Thread} instance.
     * @return  {@code true} only if the task has already been canceled.
     */
    public final boolean setThread(Thread t) {
        return synchronizer.setThread(t);
    }

    /**
     * Set the result of the task associated with the future.
     *
     * <p>
     *   Subclass needs to call this method when the computation is complete.
     * </p>
     *
     * @param result  A value returned by the task.
     * @return  {@code true} if the future state has been changed.
     *          {@code false} if the task already completed.
     */
    public final boolean set(T result) {
        boolean ret = synchronizer.set(result);
        if (ret) {
            onFutureSucceeded(result);
            done();
        }

        return ret;
    }

    /**
     * Set a {@link Throwable} that indicates the cause of error.
     *
     * <p>
     *   Subclass needs to call this method when the task has caught an
     *   unhandled exception.
     * </p>
     *
     * @param cause  A {@link Throwable} instance.
     * @return  {@code true} if the future state has been changed.
     *          {@code false} if the task already completed.
     */
    public final boolean setException(Throwable cause) {
        boolean ret = synchronizer.setException(cause);
        if (ret) {
            onFutureFailed(cause);
            done();
        }

        if (cause instanceof Error) {
            // A fatal error should be thrown.
            throw (Error)cause;
        }

        return ret;
    }

    /**
     * Mask cancellation temporarily.
     *
     * <p>
     *   This method is expected to be called on the thread that is
     *   executing the task. This method has no effect if the task is
     *   already completed successfully.
     * </p>
     *
     * @return  {@code true} on success.
     *          {@code false} if the task has already completed.
     * @throws InterruptedException
     *    The task has already been canceled.
     */
    protected final boolean maskCancel() throws InterruptedException {
        return synchronizer.maskCancel();
    }

    /**
     * Unmask cancellation masked by the previous call of
     * {@link #maskCancel()}.
     *
     * <p>
     *   This method is expected to be called on the thread that is
     *   executing the task. This method has no effect if the task is
     *   already completed successfully.
     * </p>
     *
     * @return  {@code true} on success.
     *          {@code false} if the task has already completed.
     * @throws InterruptedException
     *    The task has already been canceled.
     */
    protected final boolean unmaskCancel() throws InterruptedException {
        return synchronizer.unmaskCancel(this);
    }

    /**
     * Invoked when the computation has completed normally.
     *
     * <p>
     *   Note that this method is called prior to
     *   {@link com.google.common.util.concurrent.FutureCallback}.
     * </p>
     *
     * @param result  The result of the task.
     */
    protected void onFutureSucceeded(T result) {
    }

    /**
     * Invoked when the computation has completed abnormally.
     *
     * <p>
     *   Note that this method is called prior to
     *   {@link com.google.common.util.concurrent.FutureCallback}.
     * </p>
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    protected void onFutureFailed(Throwable cause) {
    }

    /**
     * Invoke callbacks to notify that this future has been canceled.
     */
    private void onCanceled() {
        onFutureFailed(FutureSynchronizer.canceled());
        done();
    }

    // Future

    /**
     * Attempt to cancel execution of the task.
     *
     * @param intr  {@code true} if the task runner thread should be
     *              interrupted. Otherwise in-progress task are allowed
     *              to complete.
     * @return  {@code true} only if the task was canceled.
     */
    @Override
    public boolean cancel(boolean intr) {
        boolean ret = synchronizer.cancel(intr);
        if (ret) {
            onCanceled();
        }

        return ret;
    }

    /**
     * Determine whether the task was canceled before it completed normally.
     *
     * @return  {@code true} only if the task was canceled.
     */
    @Override
    public boolean isCancelled() {
        return synchronizer.isCancelled();
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
     * @throws java.util.concurrent.CancellationException
     *    The task was canceled.
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
     * @throws java.util.concurrent.CancellationException
     *    The task was canceled.
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
