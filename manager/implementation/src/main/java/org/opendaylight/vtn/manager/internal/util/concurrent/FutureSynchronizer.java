/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.locks.AbstractQueuedSynchronizer;

/**
 * An abstract class that provides the synchronization semantics for
 * {@code Future} implementation.
 *
 * <p>
 *   This class implements the shared lock using
 *   {@link AbstractQueuedSynchronizer}.
 *   The shared lock is unavailable until the task completes.
 * </p>
 *
 * @param <T>  The type of the object to be returned.
 */
public abstract class FutureSynchronizer<T>
    extends AbstractQueuedSynchronizer {
    /**
     * Version number for serialization.
     */
    protected static final long  serialVersionUID = 1L;

    /**
     * Internal state that indicates the task associated with the future is
     * executing.
     */
    protected static final int  STATE_EXECUTING = 0x1;

    /**
     * Internal state that indicates the task is going to complete.
     */
    protected static final int  STATE_COMPLETING = 0x10000000;

    /**
     * Internal state that indicates the task has completed.
     */
    protected static final int  STATE_COMPLETED = 0x20000000;

    /**
     * Internal state that indicates the task has been canceled.
     */
    protected static final int  STATE_CANCELED = 0x40000000;

    /**
     * An integer value to be passed to shared lock methods.
     */
    protected static final int  LOCK_ARG = 0;

    /**
     * An integer value that indicates the shared lock is available for
     * shared lock.
     */
    protected static final int  LOCK_SUCCEEDED = 1;

    /**
     * An integer value that indicates the shared lock is unavailable.
     */
    protected static final int  LOCK_FAILED = -1;

    /**
     * A value to be returned by the task.
     */
    private T  taskResult;

    /**
     * A throwable caught by the task.
     */
    private Throwable  taskCause;

    /**
     * A thread that is executing the task.
     */
    private Thread  taskThread;

    /**
     * Return an {@link InterruptedException} that indicates the task was
     * interrupted.
     *
     * @return  An {@link InterruptedException} instance.
     */
    protected static final InterruptedException interrupted() {
        return new InterruptedException("The task was interrupted.");
    }

    /**
     * Return a {@link CancellationException} that indicates the task was
     * canceled.
     *
     * @return  A {@link CancellationException} instance.
     */
    protected static final CancellationException canceled() {
        return new CancellationException("The task was canceled.");
    }

    /**
     * Return an {@link IllegalStateException} that indicates the current
     * future is in unexpected state.
     *
     * @param state  The current internal state value.
     * @return  A {@link IllegalStateException} instance.
     */
    protected static final IllegalStateException unexpectedState(int state) {
        String msg = String.format("Unexpected future state: 0x%x", state);
        return new IllegalStateException(msg);
    }

    /**
     * Construct a new instance.
     */
    protected FutureSynchronizer() {
        setState(STATE_EXECUTING);
    }

    /**
     * Set a thread that is executing the task.
     *
     * @param t  A {@link Thread} instance.
     * @return  {@code true} only if the task has already been canceled.
     */
    public final boolean setThread(Thread t) {
        taskThread = t;
        return isCancelled();
    }

    /**
     * Determine whether the task was canceled before it completed normally.
     *
     * @return  {@code true} only if the task was canceled.
     */
    public final boolean isCancelled() {
        return (getState() == STATE_CANCELED);
    }

    /**
     * Determine whether the task completed or not.
     *
     * @return  {@code true} only if the task completed.
     */
    public final boolean isDone() {
        return (getState() >= STATE_COMPLETED);
    }

    /**
     * Set the result of the task.
     *
     * <p>
     *   Note that this method never changes the state of this future.
     * </p>
     *
     * @param result  The result of the task.
     * @param cause   The cause of the task failure.
     */
    public final void setResult(T result, Throwable cause) {
        taskResult = result;
        taskCause = cause;
    }

    /**
     * Interrupt the task executing the task.
     */
    public final void interruptTask() {
        Thread t = taskThread;
        if (t != null) {
            t.interrupt();
        }
    }

    /**
     * Return a {@link Throwable} that indicates the cause of failure.
     *
     * @return  A {@link Throwable} instance.
     */
    public final Throwable getTaskCause() {
        return taskCause;
    }

    /**
     * Return the result of the task.
     *
     * @return  The result of the task.
     * @throws CancellationException
     *    The task was canceled.
     * @throws ExecutionException
     *    The task caught an exception.
     */
    public final T getResult() throws ExecutionException {
        int state = getState();
        if (state == STATE_COMPLETED) {
            Throwable cause = taskCause;
            if (cause != null) {
                String msg = "Caught an exception while executing the task.";
                throw new ExecutionException(msg, cause);
            }

            return taskResult;
        }

        if (state == STATE_CANCELED) {
            throw canceled();
        }

        // This should never happen.
        throw unexpectedState(state);
    }

    /**
     * Await completion of the task only if the task is about to complete.
     *
     * @param state  The current state of the future.
     */
    public final void awaitCompletion(int state) {
        if (state == STATE_COMPLETING) {
            // Wait for completion of the task with ignoring interrupts.
            acquireShared(LOCK_ARG);
        }
    }

    /**
     * Complete the future.
     *
     * @param expected  The expected internal state.
     * @param state     The internal state to be set.
     * @param result    The result of the target task.
     * @param cause     The {@link Throwable} that indicates the cause of
     *                  failure.
     * @return  {@code true} on success.
     *          {@code false} if another thread completed the task.
     */
    public final boolean done(int expected, int state, T result,
                              Throwable cause) {
        // Change the internal state to COMPLETING.
        boolean ret = compareAndSetState(expected, STATE_COMPLETING);
        if (ret) {
            // The future state has been changed to COMPLETING.
            // So we can update the task results.
            taskResult = result;
            taskCause = cause;

            // Release the shared lock.
            releaseShared(state);
        } else {
            awaitCompletion(getState());
        }

        return ret;
    }

    /**
     * Wait for the task to complete, and then return the result of the task.
     *
     * @return  The result of the task.
     * @throws CancellationException
     *    The task was canceled.
     * @throws InterruptedException
     *    The current thread was interrupted while waiting.
     * @throws ExecutionException
     *    The task caught an exception.
     */
    public final T get() throws InterruptedException, ExecutionException {
        // Acquire the shared lock.
        acquireSharedInterruptibly(LOCK_ARG);
        return getResult();
    }

    /**
     * Wait for the task to complete, and then return the result of the task.
     *
     * @param nano  The number of nanoseconds to wait.
     * @return  The result of the task.
     * @throws CancellationException
     *    The task was canceled.
     * @throws InterruptedException
     *    The current thread was interrupted while waiting.
     * @throws ExecutionException
     *    The task caught an exception.
     * @throws TimeoutException
     *    The task did not complete within the given timeout.
     */
    public final T get(long nano)
        throws InterruptedException, ExecutionException, TimeoutException {
        // Try to acquire the shared lock.
        if (tryAcquireSharedNanos(LOCK_ARG, nano)) {
            return getResult();
        }

        StringBuilder builder = new StringBuilder(
            "The task did not complete within ");
        builder.append(nano).append(" nanoseconds.");
        throw new TimeoutException(builder.toString());
    }

    // AbstractQueuedSynchronizer

    /**
     * Attempts to acquire the shared lock.
     *
     * @param arg  Unused.
     * @return  A positive value if the task associated with this future
     *          has already completed. Otherwise a negative value is returned.
     */
    @Override
    protected final int tryAcquireShared(int arg) {
        return (isDone()) ? LOCK_SUCCEEDED : LOCK_FAILED;
    }

    /**
     * Attempts to release the shared lock.
     *
     * @param state  The internal state to be set.
     * @return  {@code true} is always returned.
     */
    @Override
    protected final boolean tryReleaseShared(int state) {
        setState(state);
        return true;
    }
}
