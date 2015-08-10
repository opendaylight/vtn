/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import org.junit.Test;
import org.opendaylight.vtn.manager.internal.TestBase;

import java.util.concurrent.Callable;
import java.util.ArrayDeque;
import java.util.Queue;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;
/**
 * JUnit test for {@link VTNFutureTask}
 */
public class VTNFutureTaskTest extends TestBase {
    /**
     * Sumtask Thread class used for test.
     *
     */
    private class SumTask implements Callable<Integer> {
        private int num = 0;

        public SumTask(int num) {
            this.num = num;
        }

        @Override
        public Integer call() throws Exception {
            int result = 0;
            for (int i = 1; i <= num; i++) {
                result += i;
            }
            return result;
        }
    }

    /**
     * Runnable Thread class used for test.
     *
     */
    private class RunnableThread implements Runnable {
        int seq;
        public RunnableThread(int seq) {
            this.seq = seq;
        }
        @Override
        public void run() {
            for (int cnt = 0; cnt < 5; cnt++) {
            }
        }
    }

/**
     * Executor class used for test.
     *
     */
    private class SequentialExecutor implements Executor {

        final Queue<Runnable> queue = new ArrayDeque<Runnable>();
        Runnable task = new RunnableThread(1);;
        public synchronized void execute(final Runnable r) {
            queue.offer(task);
            if (task == null) {
                next();
            }
        }
        private synchronized void next() {
            task = queue.poll();
            if (task != null) {
                new Thread(task).start();
            }
        }
    }

    /**
     * Test method for
     * {@link VTNFutureTask#addListener(Runnable,Executor)}.
     */
    @Test
    public void testAddListener() {
        // creating runnable object used for test.
        RunnableThread listener = new RunnableThread(1);

        // creating executor object used for test.
        Executor executor = new SequentialExecutor();
        int i = 2;
        VTNFutureTask vtnFutureTask = new VTNFutureTask(listener, i);

        vtnFutureTask.addListener(listener, executor);
    }

    /**
     * Test method for
     * {@link VTNFutureTask#done()}.
     */
    @Test
    public void testDone() {

    /**
     * creating runnable object used for test.
     *
     */
        RunnableThread listener = new RunnableThread(1);
        int i = 2;
        VTNFutureTask vtnFutureTask = new VTNFutureTask(listener, i);
        vtnFutureTask.done();

    }

    /**
     * Test method for
     * {@link VTNFutureTask#checkedGet(long,TimeUnit)}.
     */
    @Test
    public void testcheckedGetinTime() {
        try {
            Long l = 100L;
            SumTask sumTask = new SumTask(2);
            VTNFutureTask vtnFutureTask = new VTNFutureTask(sumTask);

            assertTrue(vtnFutureTask.checkedGet(l, TimeUnit.MILLISECONDS) instanceof Integer);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }
}
