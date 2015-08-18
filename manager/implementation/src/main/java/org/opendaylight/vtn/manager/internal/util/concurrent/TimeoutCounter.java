/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * {@code TimeoutCounter} describes a timeout period to suspend thread
 * execution.
 * It uses the monotonic system clock, so it is never affected by the
 * change of the realtime system clock.
 *
 * <p>
 *   Note that this class is not synchronized.
 *   An instance of this class must not be shared with multiple threads.
 * </p>
 */
public abstract class TimeoutCounter {
    /**
     * Create a new timeout counter that waits for the notification with
     * infinite timeout.
     *
     * @return  A {@link TimeoutCounter} instance that has an infinite timeout.
     */
    public static final TimeoutCounter newInfiniteTimeout() {
        return new Infinite();
    }

    /**
     * Create a new timeout counter that waits for the notificaition within
     * the specified timeout.
     *
     * @param timeout  The maximum time to wait.
     *                 Zero or a negative value is treated as an infinite
     *                 timeout.
     * @param unit     The time unit of the {@code timeout} argument.
     * @return  A {@link TimeoutCounter} instance.
     * @throws NullPointerException
     *    {@code unit} is {@code null}.
     */
    public static final TimeoutCounter newTimeout(long timeout, TimeUnit unit) {
        return (timeout > 0) ? new Deadline(timeout, unit) : new Infinite();
    }

    /**
     * Implementation of {@link TimeoutCounter} that waits for the notification
     * with infinite timeout.
     */
    private static final class Infinite extends TimeoutCounter {
        /**
         * Construct a new instance.
         */
        private Infinite() {}

        /**
         * Wait for the given object to be notified.
         *
         * @param obj  An object to wait on.
         *             Note that the caller hold the monitor of the specified
         *             object.
         * @throws InterruptedException
         *    The calling thread was interrupted.
         * @throws NullPointerException
         *    {@code obj} is {@code null}.
         */
        @Override
        public void await(Object obj) throws InterruptedException {
            obj.wait(0);
        }

        /**
         * Return the number of nanoseconds to wait.
         *
         * @return  Zero is always returned.
         */
        @Override
        public long getRemains() {
            return 0;
        }
    }

    /**
     * Implementation of {@link TimeoutCounter} that waits for the notificaiton
     * within the specified timeout.
     */
    private static final class Deadline extends TimeoutCounter {
        /**
         * The minimum number of nanoseconds to wait for.
         */
        private static final long  MIN_TIMEOUT = 1L;

        /**
         * The monotonic system clock value which specifies the deadline of
         * the notification.
         */
        private long  deadline;

        /**
         * The number of nanoseconds to wait for.
         */
        private long  remains;

        /**
         * Construct a new instance.
         *
         * @param timeout  The maximum time to wait.
         * @param unit     The time unit of the {@code timeout} argument.
         * @throws NullPointerException
         *    {@code unit} is {@code null}.
         */
        private Deadline(long timeout, TimeUnit unit) {
            remains = Math.max(unit.toNanos(timeout), MIN_TIMEOUT);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void await(Object obj)
            throws InterruptedException, TimeoutException {
            long r = getRemains();
            TimeUnit.NANOSECONDS.timedWait(obj, r);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public long getRemains() throws TimeoutException {
            if (deadline == 0) {
                // Initialize deadline.
                deadline = System.nanoTime() + remains;
            } else {
                // Update the timeout counter.
                remains = deadline - System.nanoTime();
                if (remains <= 0L) {
                    throw new TimeoutException("The timeout counter expired.");
                }
            }

            return remains;
        }
    }

    /**
     * Wait for the given object to be notified.
     *
     * @param obj  An object to wait on.
     *             Note that the caller hold the monitor of the specified
     *             object.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    The wait has been timed out.
     * @throws NullPointerException
     *    {@code obj} is {@code null}.
     */
    public abstract void await(Object obj)
        throws InterruptedException, TimeoutException;

    /**
     * Return the number of nanoseconds to wait.
     *
     * @return  The number of nanoseconds to wait.
     *          Note that zero is returned if this counter representse an
     *          infinite timeout.
     * @throws TimeoutException
     *    The wait has been timed out.
     */
    public abstract long getRemains() throws TimeoutException;
}
