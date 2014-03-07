/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Date;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;

import javax.transaction.SystemException;
import javax.transaction.Transaction;

import org.junit.After;
import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.clustering.services.IClusterServicesCommon;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link CacheTransaction}.
 */
public class CacheTransactionTest extends TestBase {
    /**
     * A error message to be embedded in an exception for test.
     */
    private static final String  ERR_MESSAGE = "Test exception";

    /**
     * Stub object for test.
     */
    private final TestStub  stubObject = new TestStub(0);

    /**
     * Implementation of {@link CacheTransaction} for test.
     */
    private class TestClass extends CacheTransaction<Object> {
        /**
         * An object expected to be returned.
         */
        private final Object  returnValue;

        /**
         * A list of cache transactions created by test.
         */
        private List<Transaction>  transactions = new ArrayList<Transaction>();

        /**
         * An exception to be thrown by {@link #executeImpl()}.
         */
        private final Exception  exception;

        /**
         * Set {@code true} if the transaction should be aborted.
         */
        private final boolean  doAbort;

        /**
         * The number of cache operations retries.
         */
        private int  retryCount;

        /**
         * Construct a new object.
         *
         * @param timeout  The number of milliseconds to wait for a cluster
         *                 cache transaction to be established.
         * @param abort    The transaction will be aborted if {@code true}
         *                 is specified.
         * @param ret      An object expected to be returned.
         */
        private TestClass(long timeout, boolean abort,
                          Object ret) {
            this(timeout, abort, ret, null);
        }

        /**
         * Construct a new object.
         *
         * @param timeout  The number of milliseconds to wait for a cluster
         *                 cache transaction to be established.
         * @param abort    The transaction will be aborted if {@code true}
         *                 is specified.
         * @param ret  An object expected to be returned.
         * @param e    An exception to be thrown by {@link #executeImpl()}.
         */
        private TestClass(long timeout, boolean abort, Object ret,
                          Exception e) {
            super(stubObject, timeout);
            returnValue = ret;
            doAbort = abort;
            exception = e;
        }

        /**
         * Execute a procedure in a cluster cache transaction.
         *
         * @return  An object specified by the constructor.
         * @throws VTNException
         *   An exception specified by the constructor is throw if it is an
         *   instance of {@link VTNException}.
         *   If it is not an instance of {@link VTNException},
         *   a {@link VTNException} that includes it is thrown.
         */
        protected Object executeImpl() throws VTNException {
            transactions.add(stubObject.tgetTransaction());

            int retry = retryCount;
            if (retry > 0) {
                retryCount = retry - 1;
                throw new CacheRetryException();
            }

            if (exception != null) {
                VTNException e;
                if (exception instanceof VTNException) {
                    e = (VTNException)exception;
                } else {
                    e = new VTNException(ERR_MESSAGE, exception);
                }

                throw e;
            }

            if (doAbort) {
                // Abort the transaction.
                abort();
            }

            return returnValue;
        }

        /**
         * Return a list of {@link Transaction} objects created by test.
         *
         * @return  A list of transaction objects.
         */
        private List<Transaction> getTransactions() {
            return transactions;
        }

        /**
         * Set the number of {@link CacheRetryException} to be thrown.
         *
         * @param count  The number of {@link CacheRetryException} to be
         *               thrown.
         */
        private void setRetryCount(int count) {
            retryCount = count;
        }
    }

    /**
     * Reset transaction test mode.
     */
    @After
    public void resetTestMode() {
        stubObject.setTransactionTestMode(null);
    }

    /**
     * Test for normal case.
     */
    @Test
    public void testNormal() {
        for (long timeout = 10; timeout <= 20; timeout++) {
            Date d = new Date();
            TestClass t = new TestClass(timeout, false, d);
            try {
                assertEquals(d, t.execute());
            } catch (Exception e) {
                unexpected(e);
            }

            List<Transaction> tlist = t.getTransactions();
            assertEquals(1, tlist.size());
            TestTransaction xact = (TestTransaction)tlist.get(0);
            assertNotNull(xact);
            assertEquals(timeout, xact.getTimeout());
            assertEquals(TimeUnit.MILLISECONDS, xact.getTimeUnit());
            assertEquals(TestTransaction.STATUS_COMMITTED, xact.getStatus());
        }
    }

    /**
     * Test for normal case with retrying operation.
     */
    @Test
    public void testNormalRetry() {
        for (int retry = 1; retry <= 5; retry++) {
            long timeout = 10 + retry;
            Date d = new Date();
            TestClass t = new TestClass(timeout, false, d);
            t.setRetryCount(retry);

            try {
                assertEquals(d, t.execute());
            } catch (Exception e) {
                unexpected(e);
            }

            List<Transaction> tlist = t.getTransactions();
            assertEquals(retry + 1, tlist.size());

            for (int i = 0; i < retry + 1; i++) {
                TestTransaction xact = (TestTransaction)tlist.get(i);
                assertNotNull(xact);
                assertEquals(timeout, xact.getTimeout());
                assertEquals(TimeUnit.MILLISECONDS, xact.getTimeUnit());

                int required = (i == retry)
                    ? TestTransaction.STATUS_COMMITTED
                    : TestTransaction.STATUS_ROLLBACKED;
                assertEquals(required, xact.getStatus());
            }
        }
    }

    /**
     * Failure test: Transaction abort requested by the caller.
     */
    @Test
    public void testAbort() {
        for (int retry = 0; retry <= 2; retry++) {
            long timeout = 10 + retry;
            Date d = new Date();
            TestClass t = new TestClass(timeout, true, d);
            t.setRetryCount(retry);

            try {
                assertEquals(d, t.execute());
            } catch (VTNException e) {
                unexpected(e);
            }

            // Transaction must be aborted.
            List<Transaction> tlist = t.getTransactions();
            assertEquals(retry + 1, tlist.size());

            for (int i = 0; i < retry + 1; i++) {
                TestTransaction xact = (TestTransaction)tlist.get(i);
                assertNotNull(xact);
                assertEquals(timeout, xact.getTimeout());
                assertEquals(TimeUnit.MILLISECONDS, xact.getTimeUnit());
                assertEquals(TestTransaction.STATUS_ROLLBACKED,
                             xact.getStatus());
            }
        }
    }

    /**
     * Failure test: Transaction is not started.
     */
    @Test
    public void testFailureBegin() {
        stubObject.setTransactionTestMode(TestTransaction.Mode.FAIL_BEGIN);

        long timeout = 10;
        Date d = new Date();
        TestClass t = new TestClass(timeout, false, d);
        try {
            t.execute();
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.INTERNALERROR, st.getCode());

            Throwable th = e.getCause();
            assertTrue(th instanceof SystemException);
            assertEquals(TestTransaction.ERR_FAIL_BEGIN, th.getMessage());
        }

        // executeImpl() must not be called.
        List<Transaction> tlist = t.getTransactions();
        assertTrue(tlist.isEmpty());
    }

    /**
     * Failure test: Transaction is unable to commit.
     */
    @Test
    public void testFailureCommit() {
        stubObject.setTransactionTestMode(TestTransaction.Mode.FAIL_ABORT);

        long timeout = 10;
        Date d = new Date();
        TestClass t = new TestClass(timeout, false, d);
        try {
            t.execute();
            fail("An exception must be thrown.");
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.INTERNALERROR, st.getCode());

            Throwable th = e.getCause();
            assertTrue(th instanceof SystemException);
            assertEquals(TestTransaction.ERR_FAIL_ABORT, th.getMessage());
        }

        // Transaction must be aborted.
        List<Transaction> tlist = t.getTransactions();
        assertEquals(1, tlist.size());
        TestTransaction xact = (TestTransaction)tlist.get(0);
        assertNotNull(xact);
        assertEquals(timeout, xact.getTimeout());
        assertEquals(TimeUnit.MILLISECONDS, xact.getTimeUnit());
        assertEquals(TestTransaction.STATUS_ROLLBACKED, xact.getStatus());
    }

    /**
     * Failure test: Catch a {@link VTNException}.
     */
    @Test
    public void testFailureVTNException() {
        VTNException vex =
            new VTNException(StatusCode.FORBIDDEN, "Thrown for a test.");

        for (int retry = 0; retry <= 2; retry++) {
            long timeout = 10 + retry;
            Date d = new Date();
            TestClass t = new TestClass(timeout, false, d, vex);
            t.setRetryCount(retry);

            try {
                t.execute();
                fail("An exception must be thrown.");
            } catch (VTNException e) {
                assertSame(vex, e);
            }

            // Transaction must be aborted.
            List<Transaction> tlist = t.getTransactions();
            assertEquals(retry + 1, tlist.size());

            for (int i = 0; i < retry + 1; i++) {
                TestTransaction xact = (TestTransaction)tlist.get(i);
                assertNotNull(xact);
                assertEquals(timeout, xact.getTimeout());
                assertEquals(TimeUnit.MILLISECONDS, xact.getTimeUnit());
                assertEquals(TestTransaction.STATUS_ROLLBACKED,
                             xact.getStatus());
            }
        }
    }

    /**
     * Failure test: Catch a non-{@link VTNException}.
     */
    @Test
    public void testFailureException() {
        IllegalArgumentException iex =
            new IllegalArgumentException("Thrown for a test.");

        for (int retry = 0; retry <= 2; retry++) {
            long timeout = 10 + retry;
            Date d = new Date();
            TestClass t = new TestClass(timeout, false, d, iex);
            t.setRetryCount(retry);

            try {
                t.execute();
                fail("An exception must be thrown.");
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.INTERNALERROR, st.getCode());
                assertSame(iex, e.getCause());
            }

            // Transaction must be aborted.
            List<Transaction> tlist = t.getTransactions();
            assertEquals(retry + 1, tlist.size());

            for (int i = 0; i < retry + 1; i++) {
                TestTransaction xact = (TestTransaction)tlist.get(i);
                assertNotNull(xact);
                assertEquals(timeout, xact.getTimeout());
                assertEquals(TimeUnit.MILLISECONDS, xact.getTimeUnit());
                assertEquals(TestTransaction.STATUS_ROLLBACKED,
                             xact.getStatus());
            }
        }
    }
}
