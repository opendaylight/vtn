/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import static org.junit.Assert.*;

import java.util.Hashtable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.After;
import org.junit.Before;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Common class for test of using VTNManager.
 */
public class TestUseVTNManagerBase extends TestBase {
    protected VTNManagerImpl vtnMgr = null;
    protected GlobalResourceManager resMgr;
    protected TestStub stubObj = null;
    protected static int stubMode = 0;
    protected static int clusterMode = 0;

    @Before
    public void before() {
        setupStartupDir();

        vtnMgr = new VTNManagerImpl();
        resMgr = new GlobalResourceManager();
        ComponentImpl c = new ComponentImpl(null, null, null);
        stubObj = new TestStub(stubMode);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        resMgr.setClusterGlobalService(stubObj);
        resMgr.init(c);
        vtnMgr.setResourceManager(resMgr);
        vtnMgr.setClusterContainerService(stubObj);
        vtnMgr.setSwitchManager(stubObj);
        vtnMgr.setTopologyManager(stubObj);
        vtnMgr.setDataPacketService(stubObj);
        vtnMgr.setRouting(stubObj);
        vtnMgr.setHostTracker(stubObj);
        vtnMgr.setForwardingRuleManager(stubObj);
        vtnMgr.setConnectionManager(stubObj);
        startVTNManager(c);
    }

    @After
    public void after() {
        stopVTNManager(true);
        resMgr.destroy();

        cleanupStartupDir();
    }

    /**
     * startup VTNManager
     */
    protected void startVTNManager(ComponentImpl c) {
        vtnMgr.init(c);
        vtnMgr.clearDisabledNode();
    }

    /**
     * stop VTNManager
     * @param clearCache    if true clear cache maintained in VTNManager.
     */
    protected void stopVTNManager(boolean clearCache) {
        vtnMgr.stopping();
        if (clearCache) {
            vtnMgr.containerDestroy();
        }
        vtnMgr.stop();
        vtnMgr.destroy();
    }

    /**
     * restart VTNManager
     */
    protected void restartVTNManager(ComponentImpl c) {
        stopVTNManager(false);
        startVTNManager(c);
    }

    /**
     * Flush all pending tasks on the VTN task thread.
     */
    protected void flushTasks() {
        NopTask task = new NopTask();
        vtnMgr.postTask(task);
        assertTrue(task.await(10, TimeUnit.SECONDS));
    }

    /**
     * A dummy task to flush tasks on the VTN task thread.
     */
    private class NopTask implements Runnable {
        /**
         * A latch to wait for completion.
         */
        private final CountDownLatch  latch = new CountDownLatch(1);

        /**
         * Wake up all threads waiting for this task.
         */
        @Override
        public void run() {
            latch.countDown();
        }

        /**
         * Wait for completion of this task.
         *
         * @param timeout  The maximum time to wait.
         * @param unit     The time unit of the {@code timeout} argument.
         * @return  {@code true} is returned if this task completed.
         *          Otherwise {@code false} is returned.
         */
        private boolean await(long timeout, TimeUnit unit) {
            try {
                return latch.await(timeout, unit);
            } catch (InterruptedException e) {
                return false;
            }
        }
    }

    /**
     * Flush all pending tasks on the VTN flow thread.
     */
    protected void flushFlowTasks() {
        flushFlowTasks(10000L);
    }

    /**
     * Flush all pending tasks on the VTN flow thread.
     */
    protected void flushFlowTasks(long wait) {
        NopFlowTask task = new NopFlowTask(vtnMgr);
        vtnMgr.postFlowTask(task);
        assertTrue(task.await(wait, TimeUnit.MILLISECONDS));
    }

    /**
     *  A dummy flow task to flush pending tasks.
     */
    public class NopFlowTask extends FlowModTask {
        /**
         * A latch to wait for completion.
         */
        private final CountDownLatch  latch = new CountDownLatch(1);

        public NopFlowTask(VTNManagerImpl mgr) {
            super(mgr);
        }

        /**
         * Wake up all threads waiting for this task.
         *
         * @return  {@code true} is always returned.
         */
        @Override
        protected boolean execute() {
            latch.countDown();
            return true;
        }

        /**
         * Return a logger object for this class.
         *
         * @return  A logger object.
         */
        @Override
        protected Logger getLogger() {
            return LoggerFactory.getLogger(getClass());
        }

        /**
         * Wait for completion of this task.
         *
         * @param timeout  The maximum time to wait.
         * @param unit     The time unit of the {@code timeout} argument.
         * @return  {@code true} is returned if this task completed.
         *          Otherwise {@code false} is returned.
         */
        public boolean await(long timeout, TimeUnit unit) {
            try {
                return latch.await(timeout, unit);
            } catch (InterruptedException e) {
                return false;
            }
        }
    }
}
