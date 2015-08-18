/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;

import org.mockito.Mockito;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link CompositeAutoCloseable}.
 */
public class CompositeAutoCloseableTest extends TestBase {
    /**
     * An implementation of {@link AutoCloseable} that counts the number
     * of {@link #close()} calls.
     */
    private static class CloseCounter implements AutoCloseable {
        /**
         * An {@link AtomicInteger} that assigns a sequence number of the
         * {@link #close()} call.
         */
        private static final AtomicInteger  SEQUENCE = new AtomicInteger();

        /**
         * The number of {@link #close()} calls.
         */
        private final AtomicInteger  closed = new AtomicInteger();

        /**
         * The sequence number.
         */
        private int  sequence;

        /**
         * Close this instance.
         */
        @Override
        public void close() {
            closed.getAndIncrement();
            sequence = SEQUENCE.incrementAndGet();
        }

        /**
         * Return the number of {@link #close()} calls.
         *
         * @return  The number of {@link #close()} calls.
         */
        private int getClosedCount() {
            return closed.get();
        }

        /**
         * Return the sequence number of {@link #close()} call.
         *
         * @return  A sequence number.
         */
        private int getSequence() {
            return sequence;
        }
    }

    /**
     * An implementation of {@link AutoCloseable} that always throws the
     * specified exception.
     */
    private static class BadCloseable extends CloseCounter {
        /**
         * An exception to be thrown by {@link #close()}.
         */
        private final RuntimeException  exception;

        /**
         * Construct a new instance.
         *
         * @param e  An exception to be thrown by {@link #close()}.
         */
        private BadCloseable(RuntimeException e) {
            exception = e;
        }

        /**
         * Close this instance.
         */
        @Override
        public void close() {
            super.close();
            throw exception;
        }
    }

    /**
     * Test case for {@link CompositeAutoCloseable} methods.
     */
    @Test
    public void test() {
        Logger logger = Mockito.mock(Logger.class);
        CompositeAutoCloseable cc = new CompositeAutoCloseable(logger);
        assertEquals(false, cc.isClosed());

        List<CloseCounter> counters = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            CloseCounter counter = new CloseCounter();
            counters.add(counter);
            for (int j = 0; j < 5; j++) {
                cc.add(counter);
            }
        }

        IllegalStateException e = new IllegalStateException();
        BadCloseable bad = new BadCloseable(e);
        counters.add(bad);
        for (int i = 0; i < 5; i++) {
            cc.add(bad);
        }
        assertEquals(false, cc.isClosed());

        cc.close();
        assertEquals(true, cc.isClosed());
        int sequence = counters.size();
        for (CloseCounter counter: counters) {
            assertEquals(1, counter.getClosedCount());

            // Closeables should be closed in reverse order of addition.
            assertEquals(sequence, counter.getSequence());
            sequence--;
        }

        Mockito.verify(logger).error("Failed to close instance: " + bad, e);
        Mockito.verify(logger, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(logger, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(logger, Mockito.never()).trace(Mockito.anyString());
        Mockito.reset(logger);

        // Ensure that closeables in CompositeAutoCloseable are closed
        // only once.
        for (int i = 0; i < 10; i++) {
            cc.close();
        }
        for (CloseCounter counter: counters) {
            assertEquals(1, counter.getClosedCount());
        }

        // Ensure that closeables are closed if it is added to closed
        // CompositeAutoCloseable.
        CloseCounter counter1 = new CloseCounter();
        cc.add(counter1);
        assertEquals(1, counter1.getClosedCount());

        IllegalArgumentException iae = new IllegalArgumentException();
        BadCloseable bad1 = new BadCloseable(iae);
        cc.add(bad1);
        Mockito.verify(logger).error("Failed to close instance: " + bad1, iae);
        Mockito.verify(logger, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(logger, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(logger, Mockito.never()).trace(Mockito.anyString());

        // This should do nothing.
        cc.close();
        assertEquals(1, counter1.getClosedCount());
        Mockito.verify(logger).error("Failed to close instance: " + bad1, iae);
        Mockito.verify(logger, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(logger, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(logger, Mockito.never()).trace(Mockito.anyString());

        sequence = counters.size();
        for (CloseCounter counter: counters) {
            assertEquals(1, counter.getClosedCount());
            assertEquals(sequence, counter.getSequence());
            sequence--;
        }
    }
}
