/**
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import static org.junit.Assert.*;

import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.junit.Test;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.VTNException;

/**
 * test for {@link VTNThreadPool}
 */
public class VTNThreadPoolTest extends TestBase {

    /**
     * task Thread class used for test.
     * this task wait until a latch is counted down to zero.
     */
    private class ThreadTask implements Runnable {
        private static final int STATE_NOTSTART = 1;
        private static final int STATE_WAIT     = 2;
        private static final int STATE_FINISHED = 3;
        int state = STATE_NOTSTART;

        protected long waitTime = 0;
        protected CountDownLatch latch = null;

        ThreadTask(long timeout, CountDownLatch latch) {
            this.waitTime = timeout;
            this.latch = latch;
        }

        @Override
        synchronized public void run() {
            latch.countDown();
            state = STATE_WAIT;
            await(waitTime, TimeUnit.SECONDS);
            state = STATE_FINISHED;
        }

        protected boolean await(long timeout, TimeUnit unit) {
            try {
                return latch.await(timeout, unit);
            } catch (InterruptedException e) {
                return false;
            }
        }

        private int getState() {
            return state;
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
        synchronized public void run() {
           latch.countDown();
           throw new RuntimeException();
        }
    }

    /**
     * Test method for
     * {@link VTNThreadPool#VTNThreadPool(String, int, long)}.
     *
     * <p>
     * This also tests
     * {@link VTNThreadPool#execute(Runnable)},
     * {@link VTNThreadPool#shutdown()},
     * {@link VTNThreadPool#terminate()}.
     * </p>
     */
    @Test
    public void testVTNThreadPool() {

        // 1 task on 1 worker.
        CountDownLatch latch = new CountDownLatch(1);
        VTNThreadPool pool = new VTNThreadPool("test", 1, 1000L);
        ThreadTask task1 = new ThreadTask(10000L, latch);
        assertTrue(pool.execute(task1));
        assertTrue(task1.await(10L, TimeUnit.SECONDS));

        // 2 tasks on 1 worker.
        CountDownLatch latch1 = new CountDownLatch(1);
        CountDownLatch latch2 = new CountDownLatch(1);
        task1 = new ThreadTask(10000L, latch1);
        ThreadTask task2 = new ThreadTask(10000L, latch2);
        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));
        assertTrue(task1.await(10L, TimeUnit.SECONDS));
        assertTrue(task2.await(10L, TimeUnit.SECONDS));

        // 2 tasks on 1 worker.
        latch = new CountDownLatch(2);
        task1 = new ThreadTask(1000L, latch);
        task2 = new ThreadTask(1000L, latch);
        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));
        assertFalse(task1.await(100L, TimeUnit.MILLISECONDS));
        assertFalse(task2.await(100L, TimeUnit.MILLISECONDS));

        pool.terminate();

        // 2 tasks on 2 workers.
        pool = new VTNThreadPool("test2", 2, 1000L);
        latch = new CountDownLatch(2);
        task1 = new ThreadTask(1000L, latch);
        task2 = new ThreadTask(1000L, latch);

        assertTrue(pool.execute(task1));
        assertFalse(task1.await(500L, TimeUnit.MILLISECONDS));

        // task1 finish after task2 is executed
        assertTrue(pool.execute(task2));
        assertTrue(task2.await(10L, TimeUnit.SECONDS));
        assertTrue(task1.await(10L, TimeUnit.SECONDS));

        // wait until poll() timeout.
        try {
            Thread.sleep(1000L);
        } catch (InterruptedException e) {
            unexpected(e);
        }

        // in case poll and getTask is interrupted.
        // check if work normally after worker is interrupted.
        pool = new VTNThreadPool("test_intr", 2, 10000L);
        latch = new CountDownLatch(2);
        task1 = new ThreadTask(10000L, latch);
        task2 = new ThreadTask(10000L, latch);

        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));
        assertTrue(task2.await(10L, TimeUnit.SECONDS));
        assertTrue(task1.await(10L, TimeUnit.SECONDS));

        latch = new CountDownLatch(1);
        InterruptTask itask = new InterruptTask(Thread.currentThread(),
                                                10000L, latch);
        Timer timer = new Timer();
        timer.schedule(itask, 1L);
        itask.await(10L, TimeUnit.SECONDS);

        latch = new CountDownLatch(2);
        task1 = new ThreadTask(10000L, latch);
        task2 = new ThreadTask(10000L, latch);
        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));
        assertTrue(task2.await(10L, TimeUnit.SECONDS));
        assertTrue(task1.await(10L, TimeUnit.SECONDS));

        itask.cancel();
        timer.cancel();

        // in case one thread throw Exception.
        pool = new VTNThreadPool("test_exception", 1, 10000L);
        latch1 = new CountDownLatch(1);
        latch2 = new CountDownLatch(1);
        ExpThreadTask etask = new ExpThreadTask(1000L, latch1);
        task1 = new ThreadTask(10000L, latch2);

        assertTrue(pool.execute(etask));
        assertTrue(etask.await(10L, TimeUnit.SECONDS));

        assertTrue(pool.execute(task1));
        assertTrue(task1.await(10L, TimeUnit.SECONDS));

        // test shutdown()
        pool = new VTNThreadPool("test3", 2, 1000L);
        pool.shutdown();
        task1 = new ThreadTask(10000L, latch);
        assertFalse(pool.execute(task1));

        // test terminate()
        pool = new VTNThreadPool("test4", 2, 1000L);
        pool.terminate();
        task1 = new ThreadTask(10000L, latch);
        assertFalse(pool.execute(task1));

        pool = new VTNThreadPool("test5", 2, 1000L);
        latch = new CountDownLatch(3);
        task1 = new ThreadTask(10000L, latch);
        task2 = new ThreadTask(10000L, latch);
        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));
        pool.terminate();

        pool = new VTNThreadPool("test5", 2, 1000L);
        try {
            pool.finalize();
        } catch (Throwable e) {
            unexpected(e);
        }
        task1 = new ThreadTask(10000L, latch);
        assertFalse(pool.execute(task1));
    }


    /**
     * Test method for {@link VTNThreadPool#join(long)}.
     */
    @Test
    public void testJoin() {

        // in case all worker end normally.
        VTNThreadPool pool = new VTNThreadPool("test1", 3, 10000L);
        CountDownLatch latch = new CountDownLatch(3);
        ThreadTask task1 = new ThreadTask(10000L, latch);
        ThreadTask task2 = new ThreadTask(10000L, latch);
        ThreadTask task3 = new ThreadTask(10000L, latch);

        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));
        assertTrue(pool.execute(task3));

        assertTrue(task1.await(10L, TimeUnit.SECONDS));
        assertTrue(task2.await(10L, TimeUnit.SECONDS));
        assertTrue(task3.await(10L, TimeUnit.SECONDS));

        assertTrue(pool.join(10000L));

        // in case all worker end normally.
        pool = new VTNThreadPool("test2", 3, 10000L);
        latch = new CountDownLatch(3);
        task1 = new ThreadTask(10000L, latch);
        task2 = new ThreadTask(10000L, latch);
        task3 = new ThreadTask(10000L, latch);

        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));
        assertTrue(pool.execute(task3));

        assertTrue(task1.await(10L, TimeUnit.SECONDS));
        assertTrue(task2.await(10L, TimeUnit.SECONDS));
        assertTrue(task3.await(10L, TimeUnit.SECONDS));

        assertTrue(pool.join(0L));

        // in case all worker don't end.
        pool = new VTNThreadPool("test2", 2, 10000L);

        latch = new CountDownLatch(3);
        task1 = new ThreadTask(10000L, latch);
        task2 = new ThreadTask(10000L, latch);

        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));

        assertFalse(pool.join(1L));


        // in case join is interrupted.
        pool = new VTNThreadPool("test3", 2, 10000L);

        latch = new CountDownLatch(3);
        task1 = new ThreadTask(10000L, latch);
        task2 = new ThreadTask(10000L, latch);

        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));

        CountDownLatch latch1 = new CountDownLatch(1);
        InterruptTask itask = new InterruptTask(Thread.currentThread(),
                                                10000L, latch1);
        Timer timer = new Timer();
        timer.schedule(itask, 1L);

        assertFalse(pool.join(10000L));

        itask.cancel();

        pool = new VTNThreadPool("test3", 2, 10000L);

        latch = new CountDownLatch(3);
        task1 = new ThreadTask(10000L, latch);
        task2 = new ThreadTask(10000L, latch);

        assertTrue(pool.execute(task1));
        assertTrue(pool.execute(task2));

        latch1 = new CountDownLatch(1);
        itask = new InterruptTask(Thread.currentThread(), 10000L, latch1);
        timer.schedule(itask, 1L);

        assertFalse(pool.join(0L));

        itask.cancel();

        //
        pool = new VTNThreadPool("test4", 2, 10000L);
        assertTrue(pool.join(0));

        timer.cancel();
    }
}
