/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.ExecutionException;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link RunnableVTNFuture}.
 */
public class RunnableVTNFutureTest extends TestBase {
    /**
     * A test class that extends {@link RunnableVTNFuture}.
     */
    private static final class TestTask extends RunnableVTNFuture<Integer> {
        /**
         * A value to be returned.
         */
        private final Integer  value;

        /**
         * A throwable to be thrown.
         */
        private final Throwable  throwable;

        /**
         * Construct a new instance that returns the specified value.
         *
         * @param v  A value to be returned.
         */
        private TestTask(Integer v) {
            value = v;
            throwable = null;
        }

        /**
         * Construct a new instance that throws the specified throwable.
         *
         * @param t  A throwable to be thrown.
         */
        private TestTask(Throwable t) {
            value = null;
            throwable = t;
        }

        // Runnable

        /**
         * Run the task.
         */
        @Override
        public void run() {
            if (throwable == null) {
                set(value);
            } else {
                setException(throwable);
            }
        }
    }

    /**
     * Test for successful completion.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSuccess() throws Exception {
        for (int i = 0; i < 10; i++) {
            Integer value = i;
            TestTask task = new TestTask(value);
            task.run();
            assertEquals(value, task.get());
        }
    }

    /**
     * Test for failure.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFailure() throws Exception {
        for (int i = 0; i < 10; i++) {
            IllegalStateException ise = new IllegalStateException("Fail:" + i);
            TestTask task = new TestTask(ise);
            task.run();
            try {
                task.get();
                unexpected();
            } catch (ExecutionException e) {
                assertEquals(ise, e.getCause());
            }
        }
    }
}
