/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import static java.util.concurrent.TimeUnit.MILLISECONDS;
import static java.util.concurrent.TimeUnit.NANOSECONDS;

import java.util.Timer;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.google.common.util.concurrent.FutureCallback;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.SlowTest;

/**
 * JUnit test for {@link FutureCallbackTask}.
 */
@Category(SlowTest.class)
public class FutureCallbackTaskTest extends TestBase {
    /**
     * The number of milliseconds to wait for the future by sync wait.
     */
    private static final long  FUTURE_SYNC_TIMEOUT = 1000;

    /**
     * The number of milliseconds between executions of the polling timer task.
     */
    private static final long  POLLING_INTERVAL = 3000;

    /**
     * The test timeout in milliseconds.
     */
    private static final long  TEST_TIMEOUT = 30000;

    /**
     * Static instance of Timer to perform unit testing.
     */
    private static Timer  timer;

    /**
     * Future callback for test.
     *
     * @param <T>  The type of the object returned by the future.
     */
    private static final class TestCallback<T> implements FutureCallback<T> {
        /**
         * The object returned by the future.
         */
        private T  result;

        /**
         * A throwable caught by the future.
         */
        private Throwable  cause;

        /**
         * Wait for the test future to complete successfully.
         *
         * @return  The object returned by the future.
         * @throws InterruptedException  The calling thread was interrupted.
         */
        private synchronized T getResult() throws InterruptedException {
            await();
            if (cause != null) {
                throw new IllegalStateException("Future failed.", cause);
            }

            return result;
        }

        /**
         * Wait for the future to fail.
         *
         * @return  A throwable which indicates the cause of error.
         * @throws InterruptedException  The calling thread was interrupted.
         */
        private synchronized Throwable getCause() throws InterruptedException {
            await();
            if (result != null) {
                throw new IllegalStateException(
                    "Future completed successfully: " + result);
            }

            return cause;
        }

        /**
         * Wait for the completion of the future.
         *
         * @throws InterruptedException  The calling thread was interrupted.
         */
        private synchronized void await() throws InterruptedException {
            if (result == null && cause == null) {
                long timeout = TEST_TIMEOUT;
                long deadline = System.nanoTime() +
                    MILLISECONDS.toNanos(timeout);

                do {
                    wait(timeout);
                    if (result != null || cause != null) {
                        return;
                    }
                    timeout = NANOSECONDS.
                        toMillis(deadline - System.nanoTime());
                } while (timeout > 0);

                throw new IllegalStateException("Future did not complete.");
            }
        }

        /**
         * Determine whether the future completed or not.
         *
         * @return  {@code true} only if the future completed.
         */
        private synchronized boolean isDone() {
            return (result != null || cause != null);
        }

        // FutureCallback

        /**
         * Invoked when the future has completed successfully.
         *
         * @param res  The object returned by the future.
         */
        @Override
        public synchronized void onSuccess(T res) {
            result = res;
            notifyAll();
        }

        /**
         * Invoked when the future has failed.
         *
         * @param t  A throwable which indicates the cause of error.
         */
        @Override
        public synchronized void onFailure(Throwable t) {
            cause = t;
            notifyAll();
        }
    }

    /**
     * This method creates the requird objects to perform unit testing.
     */
    @BeforeClass
    public static void setUpBeforeClass() {
        timer = new Timer("FutureCallbackTask test timer");
    }

    /**
     * This method makes unnecessary objects eligible for garbage collection.
     */
    @AfterClass
    public static void tearDownAfterClass() {
        timer.cancel();
        timer = null;
    }

    /**
     * Test case for {@link FutureCallbackTask#getMaxPolls()}.
     */
    @Test
    public void testGetMaxPolls() {
        Future<Void> f = new SettableVTNFuture<>();
        FutureCallback<Void> cb = new TestCallback<>();
        FutureCallbackTask<Void> task = new FutureCallbackTask<>(f, cb, timer);
        assertEquals(10, task.getMaxPolls());

        for (int i = 0; i <= 20; i++) {
            task = new FutureCallbackTask<>(f, cb, timer, i);
            assertEquals(i, task.getMaxPolls());
        }
    }

    /**
     * Ensure that {@link FutureCallbackTask} never schedules the timer if the
     * future already completed successfully.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEarlySuccess() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        TestCallback<Long> cb = new TestCallback<>();

        // In this case Timer should never be used.
        FutureCallbackTask<Long> task =
            new FutureCallbackTask<>(future, cb, null);
        Long result = Long.valueOf(System.nanoTime());
        future.set(result);

        Thread t = new Thread(task, "testEarlySuccess");
        t.start();
        assertEquals(result, cb.getResult());
        t.join(TEST_TIMEOUT);
        assertEquals(false, t.isAlive());

        assertEquals(true, cb.isDone());
        assertEquals(true, future.isDone());
        assertEquals(false, future.isCancelled());
    }

    /**
     * Ensure that {@link FutureCallbackTask} can detect the successful
     * completion of the future by polling.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccess() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        TestCallback<Long> cb = new TestCallback<>();
        FutureCallbackTask<Long> task =
            new FutureCallbackTask<>(future, cb, timer);

        Thread t = new Thread(task, "testSuccess");
        t.start();
        t.join(TEST_TIMEOUT);
        assertEquals(false, t.isAlive());
        assertEquals(false, cb.isDone());

        Long result = Long.valueOf(System.nanoTime());
        future.set(result);
        assertEquals(result, cb.getResult());

        assertEquals(true, cb.isDone());
        assertEquals(true, future.isDone());
        assertEquals(false, future.isCancelled());
    }

    /**
     * Ensure that {@link FutureCallbackTask} never schedules the timer if the
     * future already failed.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEarlyFailure() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        TestCallback<Long> cb = new TestCallback<>();

        // In this case Timer should never be used.
        FutureCallbackTask<Long> task =
            new FutureCallbackTask<>(future, cb, null);
        IllegalArgumentException iae = new IllegalArgumentException();
        future.setException(iae);

        Thread t = new Thread(task, "testEarlyFailure");
        t.start();
        Throwable cause = cb.getCause();
        t.join(TEST_TIMEOUT);
        assertEquals(false, t.isAlive());

        assertTrue(cause instanceof ExecutionException);
        assertSame(iae, cause.getCause());
        assertEquals(true, cb.isDone());
        assertEquals(true, future.isDone());
        assertEquals(false, future.isCancelled());
    }

    /**
     * Ensure that {@link FutureCallbackTask} can detect the failure of the
     * future by polling.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailure() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        TestCallback<Long> cb = new TestCallback<>();
        FutureCallbackTask<Long> task =
            new FutureCallbackTask<>(future, cb, timer);

        Thread t = new Thread(task, "testFailure");
        t.start();
        t.join(TEST_TIMEOUT);
        assertEquals(false, t.isAlive());
        assertEquals(false, cb.isDone());

        IllegalArgumentException iae = new IllegalArgumentException();
        future.setException(iae);
        Throwable cause = cb.getCause();
        assertTrue(cause instanceof ExecutionException);
        assertSame(iae, cause.getCause());
        assertEquals(true, cb.isDone());
        assertEquals(true, future.isDone());
        assertEquals(false, future.isCancelled());
    }

    /**
     * Ensure that {@link FutureCallbackTask} abandons the polling if the
     * number of pollings exceeds the limit.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testTimeout() throws Exception {
        SettableVTNFuture<Long> future = new SettableVTNFuture<>();
        TestCallback<Long> cb = new TestCallback<>();
        int maxPolls = 2;
        FutureCallbackTask<Long> task =
            new FutureCallbackTask<>(future, cb, timer, maxPolls);

        long start = System.nanoTime();
        Thread t = new Thread(task, "testTimeout");
        t.start();
        t.join(TEST_TIMEOUT);
        assertEquals(false, t.isAlive());
        assertEquals(false, cb.isDone());

        Throwable cause = cb.getCause();
        long elapsed = NANOSECONDS.toMillis(System.nanoTime() - start);
        long expected = FUTURE_SYNC_TIMEOUT + (maxPolls * POLLING_INTERVAL);
        long deadline = FUTURE_SYNC_TIMEOUT +
            ((maxPolls + 3) * POLLING_INTERVAL);
        assertTrue("elapsed=" + elapsed + ", expected=" + expected,
                   elapsed >= expected);
        assertTrue("elapsed=" + elapsed + ", deadline=" + deadline,
                   elapsed <= deadline);

        assertTrue(cause instanceof CancellationException);
        assertEquals(true, cb.isDone());
        assertEquals(true, future.isDone());
        assertEquals(true, future.isCancelled());
    }
}
