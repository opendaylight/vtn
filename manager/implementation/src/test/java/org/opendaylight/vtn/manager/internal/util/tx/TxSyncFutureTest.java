/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFutureTask;

import org.opendaylight.vtn.manager.internal.SlowTest;

/**
 * JUnit test for {@link TxSyncFuture}
 */
@Category(SlowTest.class)
public class TxSyncFutureTest {
    /**
     * Static instance of TxTask to perform unit testing.
     */
    public static TxTask<Integer> txTask;

    /**
     * Static instance of VTNFuture to perform unit testing.
     */
    public static VTNFuture<Integer> vtnFuture;

    /**
     * Static instance of Callable to perform unit testing.
     */
    public static Callable<Integer> callable;

    /**
     * Static instance of CompositeTxTask to perform unit testing.
     */
    public static CompositeTxTask compostiteTxTask;

    /**
     * Static instance of TxSyncFuture to perform unit testing.
     */
    public static TxSyncFuture<Integer> txSyncFuture;

    /**
     * This method creates the requird objects to perform unit testing
     */
    @BeforeClass
    public static void setUpBeforeClass() {

        AbstractTxTask<Integer> abstractTxTask1 = new AbstractTxTask<Integer>() {
            public Integer execute(TxContext ctx) {
                return new Integer(1000);
            }
        };
        AbstractTxTask<Integer> abstractTxTask2 = new AbstractTxTask<Integer>() {
            public Integer execute(TxContext ctx) {
                return new Integer(2000);
            }
        };

        List<AbstractTxTask> list = new ArrayList<AbstractTxTask>();
        list.add(abstractTxTask1);
        list.add(abstractTxTask2);

        txTask = new CompositeTxTask(list);

        callable = new Callable<Integer>() {
            public Integer call() {
                return new Integer(1000);
            }
        };

        vtnFuture = new VTNFutureTask<Integer>(callable);

        txSyncFuture = new TxSyncFuture<Integer>(txTask, vtnFuture);

    }

    /**
     * This method makes unnecessary objects eligible for garbage collection
     */
    @AfterClass
    public static void tearDownAfterClass() {

        txSyncFuture = null;
        txTask = null;
        vtnFuture = null;
        callable = null;
    }

    /**
     * Test method for
     * {@link TxSyncFuture#cancel(boolean)}.
     */
    @Test
    public void testCancel() {
        try {
            txSyncFuture.cancel(true);
        } catch (Exception exception) {
            Assert.fail("TxSyncFuture test case failed............");
        }
        try {
            txSyncFuture.cancel(false);
        } catch (Exception exception) {
            Assert.fail("TxSyncFuture test case failed............");
        }
    }

    /**
     * Test method for
     * {@link TxSyncFuture#isCancelled()}.
     */
    @Test
    public void testIsCancelled() {
        boolean result = true;
        try {
            result = txSyncFuture.isCancelled();
        } catch (Exception exception) {
            Assert.assertFalse(result);
        }
    }

    /**
     * Test method for
     * {@link TxSyncFuture#isDone()}.
     */
    @Test
    public void testIsDone() {
        boolean result = false;
        try {
            result = txSyncFuture.isDone();
        } catch (Exception exception) {
            Assert.assertTrue(result);
        }
    }

    /**
     * Test method for
     * {@link TxSyncFuture#get()} and
     * {@link TxSyncFuture#get(long, TimeUnit)}.
     */
    @Test
    public void testGet() {
        Integer result = null;
        ExecutorService executorService = Executors.newSingleThreadExecutor();
        try {
            Callable<Integer> callable2 = new Callable<Integer>() {
                public Integer call() throws Exception {
                    return txSyncFuture.get();
                }
            };
            executorService.submit(callable2).get(20, TimeUnit.SECONDS);
        } catch (Exception exception) {
            Assert.assertTrue(exception instanceof TimeoutException);
        }
        try {
            result = txSyncFuture.get(100, TimeUnit.MILLISECONDS);
        } catch (Exception exception) {
            Assert.assertTrue(exception instanceof TimeoutException);
        }
        try {
            result = txSyncFuture.get(1000, TimeUnit.MILLISECONDS);
        } catch (Exception exception) {
            Assert.assertTrue(exception instanceof TimeoutException);
        }
    }
}

