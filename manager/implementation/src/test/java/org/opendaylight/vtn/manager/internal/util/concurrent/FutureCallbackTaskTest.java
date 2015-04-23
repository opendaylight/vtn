/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.Future;
import java.util.concurrent.FutureTask;
import java.util.Timer;
import java.util.concurrent.Callable;

import com.google.common.util.concurrent.FutureCallback;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.flow.cond.ClearFlowConditionTask;

import org.junit.Assert;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 * JUnit test for {@link FutureCallbackTask}
 */
public class FutureCallbackTaskTest {
    /**
      * Static instance of Future to perform unit testing.
      */
    private static Future future;
    /**
      * Static instance of FutureCallback to perform unit testing.
      */
    private static FutureCallback futureCallback;
    /**
      * Static instance of Timer to perform unit testing.
      */
    private static Timer timer;
    /**
      * Static instance of VTNFuture to perform unit testing.
      */
    private static VTNFuture vtnFuture;
    /**
      * Static instance of RpcOutputGenerator to perform unit testing.
      */
    private static RpcOutputGenerator rpcOutputGenerator;
    /**
      * Static instance of FutureCallbackTask to perform unit testing.
      */
    private static FutureCallbackTask futureCallbackTask;
    /**
      * Static instance of Callable to perform unit testing.
      */
    private static Callable callable;

    /**
     * This method creates the requird objects to perform unit testing
     */
    @BeforeClass
    public static void setUpBeforeClass() {

        callable = new Callable() {
            @Override
            public Object call() throws Exception {
                return new Object();
            }
        };
        future = new FutureTask(callable);

        vtnFuture = new VTNFutureTask(callable);

        rpcOutputGenerator = new ClearFlowConditionTask();

        futureCallback = new RpcFuture(vtnFuture, rpcOutputGenerator);

        timer = new Timer();

        futureCallbackTask = new FutureCallbackTask(future, futureCallback, timer);
    }

    /**
     * This method makes unnecessary objects eligible for garbage collection
     */
    @AfterClass
    public static void tearDownAfterClass() {

        futureCallbackTask = null;
        future = null;
        futureCallback = null;
        timer = null;
        vtnFuture = null;
        rpcOutputGenerator = null;
        callable = null;
    }

    /**
     * Test method for
     * {@link FutureCallbackTask#run()}.
     */
    @Test
    public void testRun() {
        try {
            futureCallbackTask.run();
        } catch (Exception exception) {
            Assert.fail("FutureCallbackTask test case failed.......");
        }
        try {
            timer = null;
            futureCallbackTask = new FutureCallbackTask(future, futureCallback, timer);
            futureCallbackTask.run();
        } catch (Exception exception) {
            Assert.fail("FutureCallbackTask test case failed........");
        }

    }
}
