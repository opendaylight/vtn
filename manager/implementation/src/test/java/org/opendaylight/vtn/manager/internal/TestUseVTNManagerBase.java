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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.After;
import org.junit.Before;
import org.opendaylight.controller.sal.utils.GlobalConstants;
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
        flushTasks(vtnMgr);
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

    /**
     * The maximum number of threads in the thread pool for asynchronous tasks
     * of VTN Manager.
     *
     * <p>
     *  note: need to synchronize with the definition in GlobalResourceManager.
     * </p>
     */
    private static final int  THREAD_POOL_MAXSIZE = 8;

    /**
     * Wait until specified number of threads run in thread pool.
     *
     * @param timeout   A waiting time.
     */
    protected void flushAsyncTask(long timeout) {
        CountDownLatch latch = new CountDownLatch(THREAD_POOL_MAXSIZE);
        Set<WaitThreadTask> waitTasks = new HashSet<WaitThreadTask>();

        for (int i = 0; i < THREAD_POOL_MAXSIZE; i++) {
            WaitThreadTask nop = new WaitThreadTask(timeout, latch);
            vtnMgr.postAsync(nop);
            waitTasks.add(nop);
        }

        for (WaitThreadTask task : waitTasks) {
            task.await(timeout, TimeUnit.MILLISECONDS);
        }
    }

    /**
     * A task thread class used for test.
     * this task wait until a latch is counted down to zero.
     */
    private class WaitThreadTask implements Runnable {
        protected long waitTime = 0;
        protected CountDownLatch latch = null;

        WaitThreadTask(long timeout, CountDownLatch latch) {
            this.waitTime = timeout;
            this.latch = latch;
        }

        @Override
        synchronized public void run() {
            latch.countDown();
            await(waitTime, TimeUnit.SECONDS);
        }

        protected boolean await(long timeout, TimeUnit unit) {
            try {
                return latch.await(timeout, unit);
            } catch (InterruptedException e) {
                return false;
            }
        }
    }

    /**
     * setup configuraion file and restart VTN Manager
     *
     * @param timeout   A timeout value set to remoteFlowModTimeout.
     */
    private final String CONFIG_FILE_NAME = "vtnmanager.ini";
    protected void setupVTNManagerForRemoteTaskTest(long localTimeout,
                                                    long remoteTimeout) {
        setupVTNManagerForRemoteTaskTest(GlobalConstants.STARTUPHOME.toString(),
                               CONFIG_FILE_NAME, localTimeout, remoteTimeout);
    }

    protected void setupVTNManagerForRemoteTaskTest(String dir, String fileName,
                                                    long localTimeout,
                                                    long remoteTimeout) {
        FileWriter gWriter;
        File gIniFile = new File(dir, fileName);
        try {
            gWriter = new FileWriter(gIniFile);
            if (localTimeout > 0) {
                gWriter.write("flowModTimeout=" + localTimeout + "\n");
            }
            if (remoteTimeout > 0) {
                gWriter.write("remoteFlowModTimeout=" + remoteTimeout);
            }
            gWriter.close();
        } catch (IOException e) {
            unexpected(e);
        }


        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        restartVTNManager(c);
    }

    /**
     * remove configuration file.
     */
    protected void cleanupSetupFile() {
        cleanupSetupFile(GlobalConstants.STARTUPHOME.toString(),
                         CONFIG_FILE_NAME);
    }

    protected void cleanupSetupFile(String dir, String fileName) {
        File gIniFile = new File(dir, fileName);
        gIniFile.delete();
    }
}
