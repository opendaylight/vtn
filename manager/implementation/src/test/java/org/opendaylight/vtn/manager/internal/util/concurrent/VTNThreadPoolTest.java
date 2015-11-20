/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.TimeUnit;

import ch.qos.logback.classic.Level;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.LoggerUtils;
import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link VTNThreadPool}
 */
public class VTNThreadPoolTest extends TestBase {
    /**
     * task Thread class used for test.
     * this task wait until a latch is counted down to zero.
     */
    private class ThreadTask implements Runnable {
        protected long waitTime;
        protected CountDownLatch latch;
        private boolean  interrupted;
        private Thread  worker;

        ThreadTask(long timeout, CountDownLatch latch) {
            this.waitTime = timeout;
            this.latch = latch;
        }

        @Override
        public void run() {
            latch.countDown();
            setWorker();
            await(waitTime, TimeUnit.SECONDS);
        }

        protected boolean await(long timeout, TimeUnit unit) {
            try {
                return latch.await(timeout, unit);
            } catch (InterruptedException e) {
                setInterrupted();
                return false;
            }
        }

        /**
         * Set the worker thread that runs this task.
         */
        private synchronized void setWorker() {
            worker = Thread.currentThread();
            notifyAll();
        }

        /**
         * Return a worker thread.
         *
         * @param timeout  The number of milliseconds to wait.
         * @return  A worker thread if found. {@code null} otherwise.
         * @throws InterruptedException
         *    The calling thread was interrupted.
         */
        private synchronized Thread getWorker(long timeout)
            throws InterruptedException {
            Thread t = worker;
            if (t == null) {
                long to = timeout;
                long limit = System.currentTimeMillis() + to;
                do {
                    wait(to);
                    t = worker;
                    if (t != null) {
                        break;
                    }

                    to = limit - System.currentTimeMillis();
                } while (to > 0);
            }

            return t;
        }

        /**
         * Turn the interrupted flag on.
         */
        private synchronized void setInterrupted() {
            interrupted = true;
            notifyAll();
        }

        /**
         * Wait for this task to be interrupted.
         *
         * @param timeout  The number of milliseconds to wait.
         * @throws InterruptedException
         *    The calling thread was interrupted.
         */
        private synchronized boolean isInterrupted(long timeout)
            throws InterruptedException {
            boolean intr = interrupted;
            if (!intr) {
                long to = timeout;
                long limit = System.currentTimeMillis() + to;
                do {
                    wait(to);
                    intr = interrupted;
                    if (intr) {
                        break;
                    }

                    to = limit - System.currentTimeMillis();
                } while (to > 0);
            }

            return intr;
        }
    }

    /**
     * task thread class which interrupt to specified thread.
     */
    private class InterruptTask extends TimerTask {
        private Thread targetThread = null;
        private CountDownLatch latch = null;
        private long waitTime = 0;

        InterruptTask(Thread th, long waitTime, CountDownLatch latch) {
            targetThread = th;
            this.latch = latch;
            this.waitTime = waitTime;
        }

        @Override
        public void run() {
            targetThread.interrupt();
            latch.countDown();
        }

        private boolean await(long timeout, TimeUnit unit) {
            try {
                return latch.await(timeout, unit);
            } catch (InterruptedException e) {
                return false;
            }
        }
    }

    /**
     * task thread class which throw exception.
     */
    private class ExpThreadTask extends ThreadTask {
        ExpThreadTask(long timeout, CountDownLatch latch) {
            super(timeout, latch);
        }

        @Override
        public void run() {
            latch.countDown();
            throw new RuntimeException();
        }
    }

    /**
     * Test method for {@link VTNThreadPool#VTNThreadPool(String)}.
     *
     * <p>
     *   This also tests the following methods.
     * </p>
     * <ul>
     *   <li>{@link VTNThreadPool#execute(Runnable)}</li>
     *   <li>{@link VTNThreadPool#executeTask(Runnable)}</li>
     *   <li>{@link VTNThreadPool#shutdown()}</li>
     *   <li>{@link VTNThreadPool#terminate()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSingleThreadPool() throws Exception {
        // 1 task on 1 worker.
        try (VTNThreadPool pool = new VTNThreadPool("test")) {
            CountDownLatch latch = new CountDownLatch(1);
            ThreadTask task1 = new ThreadTask(10000L, latch);
            pool.execute(task1);
            Thread worker = task1.getWorker(10000L);
            assertNotNull(worker);

            // 2 independent tasks.
            CountDownLatch latch1 = new CountDownLatch(1);
            CountDownLatch latch2 = new CountDownLatch(1);
            task1 = new ThreadTask(10000L, latch1);
            ThreadTask task2 = new ThreadTask(10000L, latch2);
            pool.execute(task1);
            pool.execute(task2);
            assertSame(worker, task1.getWorker(10000L));
            assertSame(worker, task2.getWorker(10000L));

            // 2 tasks that share the same latch.
            latch = new CountDownLatch(2);
            task1 = new ThreadTask(10000L, latch);
            task2 = new ThreadTask(10000L, latch);
            assertTrue(pool.executeTask(task1));
            assertTrue(pool.executeTask(task2));
            assertFalse(task1.await(100L, TimeUnit.MILLISECONDS));
            assertFalse(task2.await(100L, TimeUnit.MILLISECONDS));

            // task2 should never be dispatched because task1 blocks.
            assertSame(worker, task1.getWorker(10000L));
            assertSame(null, task2.getWorker(100L));

            pool.terminate();
        }
    }

    /**
     * Test method for
     * {@link VTNThreadPool#VTNThreadPool(String, int, long)}.
     *
     * <p>
     *   This also tests the following methods.
     * </p>
     * <ul>
     *   <li>{@link VTNThreadPool#execute(Runnable)}</li>
     *   <li>{@link VTNThreadPool#executeTask(Runnable)}</li>
     *   <li>{@link VTNThreadPool#shutdown()}</li>
     *   <li>{@link VTNThreadPool#terminate()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVTNThreadPool() throws Exception {
        // 2 tasks on 2 workers.
        try (VTNThreadPool pool = new VTNThreadPool("test2", 2, 1000L)) {
            // Only the main thread should be present.
            checkWaiting(pool, 1);

            CountDownLatch latch = new CountDownLatch(2);
            ThreadTask task1 = new ThreadTask(10000L, latch);
            ThreadTask task2 = new ThreadTask(10000L, latch);
            assertTrue(pool.executeTask(task1));

            // Ensure that task1 was dispatched to the main thread.
            checkWaiting(pool, 0);
            assertFalse(task1.await(100L, TimeUnit.MILLISECONDS));
            Thread worker1 = task1.getWorker(10000L);
            assertNotNull(worker1);

            // Task2 will count down the latch.
            assertTrue(pool.executeTask(task2));
            assertTrue(task2.await(10L, TimeUnit.SECONDS));
            assertTrue(task1.await(10L, TimeUnit.SECONDS));

            Thread worker2 = task2.getWorker(1L);
            assertNotNull(worker2);
            assertNotSame(worker1, worker2);

            pool.terminate();
        }

        // In case where a worker thread is interrupted while it is waiting
        // for a new task.
        try (VTNThreadPool pool = new VTNThreadPool("test_intr", 1, 10000L)) {
            CountDownLatch latch = new CountDownLatch(1);
            ThreadTask task = new ThreadTask(100L, latch);
            pool.execute(task);
            Thread worker = task.getWorker(10000L);
            assertNotNull(worker);

            // Interrupt the worker thread that is waiting for a new task.
            checkWaiting(pool, 1);
            worker.interrupt();

            // Ensure that an interruption was ignored.
            task = new ThreadTask(100L, latch);
            pool.execute(task);
            assertSame(worker, task.getWorker(10000L));
        }

        // In case where one thread throws an Exception.
        try (VTNThreadPool pool =
             new VTNThreadPool("test_exception", 1, 10000L)) {
            CountDownLatch latch1 = new CountDownLatch(1);
            CountDownLatch latch2 = new CountDownLatch(1);
            ExpThreadTask etask = new ExpThreadTask(1000L, latch1);
            ThreadTask task = new ThreadTask(100L, latch2);

            Level lvl = LoggerUtils.setLevel(VTNThreadPool.class, Level.OFF);
            try {
                pool.execute(etask);
                assertTrue(etask.await(10L, TimeUnit.SECONDS));

                pool.execute(task);
                assertTrue(task.await(10L, TimeUnit.SECONDS));
            } finally {
                LoggerUtils.setLevel(VTNThreadPool.class, lvl);
            }
        }

        // Test case for shutdown()
        try (VTNThreadPool pool = new VTNThreadPool("test3", 2, 1000L)) {
            // This task should complete.
            CountDownLatch latch = new CountDownLatch(2);
            ThreadTask task = new ThreadTask(10000L, latch);
            assertTrue(pool.executeTask(task));

            pool.shutdown();
            assertFalse(task.await(10L, TimeUnit.MILLISECONDS));
            latch.countDown();
            assertTrue(task.await(10L, TimeUnit.SECONDS));

            // No more task should be accepted.
            task = new ThreadTask(10000L, latch);
            assertFalse(pool.executeTask(task));
            try {
                pool.execute(task);
                unexpected();
            } catch (RejectedExecutionException e) {
            }
        }

        // Test case for terminate()
        try (VTNThreadPool pool = new VTNThreadPool("test4", 2, 1000L)) {
            // This task should be canceled by terminate().
            CountDownLatch latch = new CountDownLatch(2);
            ThreadTask task = new ThreadTask(10000L, latch);
            assertTrue(pool.executeTask(task));
            assertNotNull(task.getWorker(10000L));
            assertFalse(task.await(10L, TimeUnit.MILLISECONDS));

            pool.terminate();
            assertTrue(task.isInterrupted(10000L));

            // No more task should be accepted.
            task = new ThreadTask(10000L, latch);
            assertFalse(pool.executeTask(task));
            try {
                pool.execute(task);
                unexpected();
            } catch (RejectedExecutionException e) {
            }
        }

        // Test case for finalize().
        try (VTNThreadPool pool = new VTNThreadPool("test5", 2, 1000L)) {
            // This task should be canceled by finalize().
            CountDownLatch latch = new CountDownLatch(2);
            ThreadTask task = new ThreadTask(10000L, latch);
            assertTrue(pool.executeTask(task));
            assertNotNull(task.getWorker(10000L));
            assertFalse(task.await(10L, TimeUnit.MILLISECONDS));

            try {
                pool.finalize();
            } catch (Throwable t) {
                unexpected(t);
            }
            assertTrue(task.isInterrupted(10000L));

            // No more task should be accepted.
            task = new ThreadTask(10000L, latch);
            assertFalse(pool.executeTask(task));
            try {
                pool.execute(task);
                unexpected();
            } catch (RejectedExecutionException e) {
            }
        }
    }

    /**
     * Test method for {@link VTNThreadPool#join(long)} and
     * {@link VTNThreadPool#close()}.
     */
    @Test
    public void testJoin() {
        Level lvl = LoggerUtils.setLevel(VTNThreadPool.class, Level.OFF);
        Timer timer = new Timer();

        // in case all worker end normally.
        try {
            try (VTNThreadPool pool = new VTNThreadPool("test1", 3, 10000L)) {
                CountDownLatch latch = new CountDownLatch(3);
                ThreadTask task1 = new ThreadTask(10000L, latch);
                ThreadTask task2 = new ThreadTask(10000L, latch);
                ThreadTask task3 = new ThreadTask(10000L, latch);

                pool.execute(task1);
                pool.execute(task2);
                pool.execute(task3);

                assertTrue(task1.await(10L, TimeUnit.SECONDS));
                assertTrue(task2.await(10L, TimeUnit.SECONDS));
                assertTrue(task3.await(10L, TimeUnit.SECONDS));

                pool.close();
                assertTrue(pool.join(10000L));
            }

            // in case all worker end normally.
            try (VTNThreadPool pool = new VTNThreadPool("test2", 3, 10000L)) {
                CountDownLatch latch = new CountDownLatch(3);
                ThreadTask task1 = new ThreadTask(10000L, latch);
                ThreadTask task2 = new ThreadTask(10000L, latch);
                ThreadTask task3 = new ThreadTask(10000L, latch);

                pool.execute(task1);
                pool.execute(task2);
                pool.execute(task3);

                assertTrue(task1.await(10L, TimeUnit.SECONDS));
                assertTrue(task2.await(10L, TimeUnit.SECONDS));
                assertTrue(task3.await(10L, TimeUnit.SECONDS));

                assertTrue(pool.join(0L));
            }

            // in case all worker don't end.
            try (VTNThreadPool pool = new VTNThreadPool("test2", 2, 10000L)) {
                CountDownLatch latch = new CountDownLatch(3);
                ThreadTask task1 = new ThreadTask(10000L, latch);
                ThreadTask task2 = new ThreadTask(10000L, latch);

                pool.execute(task1);
                pool.execute(task2);

                assertFalse(pool.join(1L));
                pool.terminate();
            }

            // in case join is interrupted.
            try (VTNThreadPool pool = new VTNThreadPool("test3", 2, 10000L)) {
                CountDownLatch latch = new CountDownLatch(3);
                ThreadTask task1 = new ThreadTask(10000L, latch);
                ThreadTask task2 = new ThreadTask(10000L, latch);

                pool.execute(task1);
                pool.execute(task2);

                CountDownLatch latch1 = new CountDownLatch(1);
                InterruptTask itask = new InterruptTask(Thread.currentThread(),
                                                        10000L, latch1);
                timer.schedule(itask, 1L);

                assertFalse(pool.join(10000L));

                itask.cancel();
                pool.terminate();
            }

            try (VTNThreadPool pool = new VTNThreadPool("test3", 2, 10000L)) {
                CountDownLatch latch = new CountDownLatch(3);
                ThreadTask task1 = new ThreadTask(10000L, latch);
                ThreadTask task2 = new ThreadTask(10000L, latch);

                pool.execute(task1);
                pool.execute(task2);

                CountDownLatch latch1 = new CountDownLatch(1);
                InterruptTask itask =
                    new InterruptTask(Thread.currentThread(), 10000L, latch1);
                timer.schedule(itask, 1L);

                assertFalse(pool.join(0L));

                itask.cancel();
                pool.terminate();
            }

            try (VTNThreadPool pool = new VTNThreadPool("test4", 2, 10000L)) {
                assertTrue(pool.join(0));
            }
        } finally {
            timer.cancel();
            LoggerUtils.setLevel(VTNThreadPool.class, lvl);
        }
    }

    /**
     * Verify the number of threads that are waiting for task.
     *
     * @param pool      A {@link VTNThreadPool} instance to be tested.
     * @param expected  The expected number of threads that are waiting for
     *                  task.
     * @throws Exception  An error occurred.
     */
    private void checkWaiting(VTNThreadPool pool, int expected)
        throws Exception {

        long limit = System.currentTimeMillis() + 10000L;
        do {
            Integer waiting = getFieldValue(pool, Integer.class, "waiting");
            if (waiting.intValue() == expected) {
                return;
            }

            sleep(10L);
        } while (System.currentTimeMillis() < limit);

        assertEquals(Integer.valueOf(expected),
                     getFieldValue(pool, Integer.class, "waiting"));
    }
}
