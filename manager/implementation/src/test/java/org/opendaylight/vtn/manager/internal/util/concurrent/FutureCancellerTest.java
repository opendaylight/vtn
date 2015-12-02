/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;

import org.apache.commons.lang3.tuple.ImmutablePair;
import org.apache.commons.lang3.tuple.ImmutableTriple;
import org.apache.commons.lang3.tuple.Pair;
import org.apache.commons.lang3.tuple.Triple;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link FutureCanceller}.
 */
public class FutureCancellerTest extends TestBase {
    /**
     * A timer task for capturing scheduled tasks.
     */
    private static final class TestTimer extends Timer {
        /**
         * A list of scheduled timers.
         */
        private List<Pair<Long, TimerTask>>  taskList = new ArrayList<>();

        /**
         * Construct a new test timer.
         */
        private TestTimer() {
            // Cancel the timer thread.
            cancel();
        }

        /**
         * Return a list of scheduled tasks, and then clear the task list.
         *
         * @return  A list of scheduled tasks.
         */
        private List<Pair<Long, TimerTask>> getTasks() {
            List<Pair<Long, TimerTask>> list = taskList;
            taskList = new ArrayList<>();
            return list;
        }

        // Timer

        /**
         * Capture the specified task.
         *
         * @param task   A task to be scheduled.
         * @param delay  Delay in milliseconds.
         */
        @Override
        public void schedule(TimerTask task, long delay) {
            Pair<Long, TimerTask> pair = new ImmutablePair<>(delay, task);
            taskList.add(pair);
        }
    }

    /**
     * Test method for
     * {@link FutureCanceller#set(Timer,long,ListenableFuture)} and
     * {@link FutureCanceller#set(Timer,long,ListenableFuture,boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSet() throws Exception {
        TestTimer timer = new TestTimer();
        List<Triple<SettableVTNFuture<Void>, Long, Boolean>> list =
            new ArrayList<>();
        IllegalStateException cause = new IllegalStateException();
        ListenableFuture<Void> succeeded = Futures.<Void>immediateFuture(null);
        ListenableFuture<Void> failed =
            Futures.<Void>immediateFailedFuture(cause);
        ListenableFuture<Void> cancelled =
            Futures.<Void>immediateCancelledFuture();
        List<ListenableFuture<Void>> ignored = new ArrayList<>();
        Collections.addAll(ignored, succeeded, failed, cancelled);

        long[] timeouts = {1L, 10L, 333333L};
        boolean[] bools = {true, false};
        for (long timeout: timeouts) {
            SettableVTNFuture<Void> f = new SettableVTNFuture<>();
            f.setThread(Thread.currentThread());
            Triple<SettableVTNFuture<Void>, Long, Boolean> expected =
                new ImmutableTriple<>(f, timeout, false);
            list.add(expected);
            FutureCanceller.set(timer, timeout, f);

            // Completed future should be ignored.
            for (ListenableFuture<Void> completed: ignored) {
                FutureCanceller.set(timer, timeout, completed);
            }

            for (boolean intr: bools) {
                f = new SettableVTNFuture<>();
                f.setThread(Thread.currentThread());
                expected = new ImmutableTriple<>(f, timeout, intr);
                list.add(expected);
                FutureCanceller.set(timer, timeout, f, intr);

                // Completed future should be ignored.
                for (ListenableFuture<Void> completed: ignored) {
                    FutureCanceller.set(timer, timeout, completed, intr);
                }
            }
        }

        Iterator<Triple<SettableVTNFuture<Void>, Long, Boolean>> it =
            list.iterator();
        for (Pair<Long, TimerTask> pair: timer.getTasks()) {
            assertTrue(it.hasNext());
            Triple<SettableVTNFuture<Void>, Long, Boolean> expected = it.next();

            // Check delay.
            assertEquals(expected.getMiddle(), pair.getLeft());

            // Check the target future.
            TimerTask task = pair.getRight();
            assertTrue(task instanceof FutureCanceller<?>);
            ListenableFuture target = getFieldValue(
                task, ListenableFuture.class, "targetTask");
            SettableVTNFuture<Void> f = expected.getLeft();
            assertEquals(target, f);

            // Check intr flag.
            Boolean intr = getFieldValue(task, Boolean.class, "needInterrupt");
            assertEquals(intr, expected.getRight());

            // Cancel the target future.
            assertEquals(false, f.isCancelled());
            assertEquals(false, f.isDone());
            task.run();
            assertEquals(intr.booleanValue(), Thread.interrupted());
            assertEquals(true, f.isCancelled());
            assertEquals(true, f.isDone());
        }

        assertFalse(it.hasNext());
    }
}
