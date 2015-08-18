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

import org.junit.Assert;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.SlowTest;

/**
 * JUnit test for {@link TimeoutCounter}.
 */
@Category(SlowTest.class)
public class TimeoutCounterTest extends TestBase {
    /**
     * A {@link Runnable} that waits for an notification.
     */
    private static class Waiter implements Runnable {
        /**
         * A {@link TimeoutCounter} instance.
         */
        private final TimeoutCounter  counter;

        /**
         * Set true if the main thread is ready for testing.
         */
        private boolean  ready;

        /**
         * Set true if the test thread is waiting for a notification.
         */
        private boolean  waiting;

        /**
         * An exception caught by the test thread.
         */
        private Exception  exception;

        /**
         * Construct a new instance.
         *
         * @param tc   A {@link TimeoutCounter} instance.
         */
        private Waiter(TimeoutCounter tc) {
            counter = tc;
        }

        /**
         * Reset the state.
         */
        private synchronized void reset() {
            ready = false;
            waiting = false;
            exception = null;
        }

        /**
         * Ensure that the test thread is waiting for noticication.
         *
         * @return  {@code true} on success. {@code false} on failure.
         * @throws InterruptedException
         *   The callin thread was interrupted.
         */
        private synchronized boolean prepare() throws InterruptedException {
            ready = true;
            notifyAll();

            if (!waiting) {
                wait(10000L);
            }

            return waiting;
        }

        /**
         * Return an exception caught by the test thread.
         *
         * @return  An exception caught by the test thread.
         */
        private synchronized Exception getException() {
            return exception;
        }

        /**
         * Wait for notification.
         */
        @Override
        public synchronized void run() {
            if (!ready) {
                try {
                    wait(10000L);
                } catch (Exception e) {
                    Assert.fail("Unexpected exception: " + e);
                }
                Assert.assertTrue(ready);
            }

            waiting = true;
            notifyAll();
            try {
                counter.await(this);
            } catch (Exception e) {
                exception = e;
            }
        }
    }

    /**
     * Test case for infinite timeout.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInfinite() throws Exception {
        TimeoutCounter[] counters = {
            TimeoutCounter.newInfiniteTimeout(),
            TimeoutCounter.newTimeout(0, TimeUnit.SECONDS),
            TimeoutCounter.newTimeout(-1, TimeUnit.NANOSECONDS),
        };

        for (TimeoutCounter tc: counters) {
            assertEquals("Infinite", tc.getClass().getSimpleName());
            Waiter w = new Waiter(tc);
            Thread t = new Thread(w);
            t.start();
            assertTrue(w.prepare());
            Thread.sleep(100L);
            assertTrue(t.isAlive());
            synchronized (w) {
                w.notifyAll();
            }
            t.join(10000L);
            assertFalse(t.isAlive());
            assertEquals(null, w.getException());
        }
    }

    /**
     * Test case for waiting with specifying timeout.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNotified() throws Exception {
        TimeoutCounter tc = TimeoutCounter.newTimeout(10, TimeUnit.SECONDS);
        assertEquals("Deadline", tc.getClass().getSimpleName());

        // Notified without any interrption.
        Waiter w = new Waiter(tc);
        Thread t = new Thread(w);
        t.start();
        assertTrue(w.prepare());
        Thread.sleep(100L);
        assertTrue(t.isAlive());
        synchronized (w) {
            w.notifyAll();
        }
        t.join(10000L);
        assertFalse(t.isAlive());
        assertEquals(null, w.getException());

        // Ensure that the timeout counter can resume the wait after
        // interruption.
        tc = TimeoutCounter.newTimeout(10, TimeUnit.SECONDS);
        assertEquals("Deadline", tc.getClass().getSimpleName());
        w = new Waiter(tc);
        t = new Thread(w);
        t.start();
        assertTrue(w.prepare());
        Thread.sleep(100L);
        assertTrue(t.isAlive());
        t.interrupt();
        t.join(10000L);
        assertFalse(t.isAlive());
        assertTrue(w.getException() instanceof InterruptedException);

        w.reset();
        t = new Thread(w);
        t.start();
        assertTrue(w.prepare());
        Thread.sleep(100L);
        assertTrue(t.isAlive());
        synchronized (w) {
            w.notifyAll();
        }
        t.join(10000L);
        assertFalse(t.isAlive());
        assertEquals(null, w.getException());
    }

    /**
     * Test case for timeout.
     *
     * <p>
     *   Note that this test takes more than 10 seconds.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testTimedOut() throws Exception {
        // The wait should be timed out after 5 seconds.
        long timeout = 5;
        long nanoTimeout = TimeUnit.SECONDS.toNanos(timeout);
        long resolution = TimeUnit.MILLISECONDS.toNanos(1);
        long joinTimeout = TimeUnit.SECONDS.toMillis(timeout + 10);
        TimeoutCounter tc = TimeoutCounter.
            newTimeout(timeout, TimeUnit.SECONDS);
        assertEquals("Deadline", tc.getClass().getSimpleName());
        Waiter w = new Waiter(tc);
        Thread t = new Thread(w);
        t.start();
        long start = System.nanoTime();
        assertTrue(w.prepare());
        Thread.sleep(100L);
        assertTrue(t.isAlive());
        t.join(joinTimeout);
        long end = System.nanoTime();
        assertFalse(t.isAlive());
        assertEquals(null, w.getException());
        long duration = end - start;
        if (duration < nanoTimeout) {
            assertTrue("Duration: " + duration,
                       duration + resolution >= nanoTimeout);
        }

        checkTimedOut(tc);

        // Ensure that the timeout counter can resume the wait after
        // interruption.
        tc = TimeoutCounter.newTimeout(timeout, TimeUnit.SECONDS);
        assertEquals("Deadline", tc.getClass().getSimpleName());
        w = new Waiter(tc);
        t = new Thread(w);
        t.start();
        start = System.nanoTime();
        assertTrue(w.prepare());
        Thread.sleep(1000L);
        assertTrue(t.isAlive());
        t.interrupt();
        t.join(10000L);
        assertFalse(t.isAlive());
        assertTrue(w.getException() instanceof InterruptedException);

        w.reset();
        t = new Thread(w);
        t.start();
        assertTrue(w.prepare());
        Thread.sleep(100L);
        assertTrue(t.isAlive());
        t.join(joinTimeout);
        end = System.nanoTime();
        assertFalse(t.isAlive());
        assertEquals(null, w.getException());
        duration = end - start;
        if (duration < nanoTimeout) {
            assertTrue("Duration: " + duration,
                       duration + resolution >= nanoTimeout);
        }

        checkTimedOut(tc);
    }

    /**
     * Ensure that the given timeout counter has already expired.
     *
     * @param tc   A {@link TimeoutCounter} instance.
     * @throws Exception  An error occurred.
     */
    private void checkTimedOut(TimeoutCounter tc) throws Exception {
        Thread.sleep(1);
        try {
            tc.await(this);
        } catch (TimeoutException e) {
            assertEquals("The timeout counter expired.", e.getMessage());
        }
    }
}
