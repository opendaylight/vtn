/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.After;
import org.junit.Before;
import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterServices;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Common class for test of using VTNManager.
 */
public class TestUseVTNManagerBase extends TestBase {
    protected VTNManagerImpl vtnMgr;
    protected GlobalResourceManager resMgr;
    protected TestStub stubObj;
    protected int stubMode;

    /**
     * The number of milliseconds to wait for VTN events.
     */
    private static final long  EVENT_TIMEOUT = 10000L;

    /**
     * Construct a new instance.
     *
     * @param stub  An integer value to be passed to {@link TestStub}.
     */
    protected TestUseVTNManagerBase(int stub) {
        stubMode = stub;
    }

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
     * startup VTNManager.
     */
    protected void startVTNManager(ComponentImpl c) {
        vtnMgr.init(c);
        vtnMgr.clearDisabledNode();
    }

    /**
     * Change stub and Startup VTNManager.
     *
     * @param stub  TestStub object.
     * @param c     ComponentImpl object.
     */
    protected void changeStubAndStartVTNManager(TestStub stub, ComponentImpl c) {
        resMgr.setClusterGlobalService(stub);
        resMgr.init(c);
        vtnMgr.setResourceManager(resMgr);
        vtnMgr.setClusterContainerService(stub);
        vtnMgr.setSwitchManager(stub);
        vtnMgr.setTopologyManager(stub);
        vtnMgr.setDataPacketService(stub);
        vtnMgr.setRouting(stub);
        vtnMgr.setHostTracker(stub);
        vtnMgr.setForwardingRuleManager(stub);
        vtnMgr.setConnectionManager(stub);
        startVTNManager(c);
    }

    /**
     * stop VTNManager.
     *
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
        flushFlowTasks(EVENT_TIMEOUT);
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
     *  note: need to synchronize with the definition
     *  in {@link GlobalResourceManager}.
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

    private final String CONFIG_FILE_NAME = "vtnmanager.ini";

    /**
     * setup configuration file and restart VTN Manager
     *
     * @param localTimeout   A timeout value set to {@code flowModTimeout}.
     * @param remoteTimeout  A timeout value set to {@code remoteFlowModTimeout}.
     */
    protected void setupVTNManagerForRemoteTaskTest(long localTimeout,
                                                    long remoteTimeout) {
        setupVTNManagerForRemoteTaskTest(GlobalConstants.STARTUPHOME.toString(),
                               CONFIG_FILE_NAME, localTimeout, remoteTimeout);
    }

    /**
     * setup configuration file and restart VTN Manager
     *
     * @param localTimeout      A timeout value set to {@code flowModTimeout}.
     * @param remoteTimeout     A timeout value set to {@code remoteFlowModTimeout}.
     * @param remoteBulkTimeout A timeout value set to
     *                          {@code remoteBulkFlowModTimeout}.
     */
    protected void setupVTNManagerForRemoteTaskTest(long localTimeout,
                                                    long remoteTimeout,
                                                    long remoteBulkTimeout) {
        setupVTNManagerForRemoteTaskTest(GlobalConstants.STARTUPHOME.toString(),
                                         CONFIG_FILE_NAME, localTimeout,
                                         remoteTimeout, remoteBulkTimeout);
    }

    /**
     * Restart VTNManager after configurations of flowModTimeout and
     * remoteFlowModTimeout are set.
     *
     * @param dir           A directory configuration file is located.
     * @param fileName      A name of configuration file.
     * @param localTimeout  A time set as {@code localTimeout}.
     * @param remoteTimeout A time set as {@code remoteTimeout}.
     */
    protected void setupVTNManagerForRemoteTaskTest(String dir, String fileName,
                                                    long localTimeout,
                                                    long remoteTimeout) {
        setupVTNManagerForRemoteTaskTest(GlobalConstants.STARTUPHOME.toString(),
                CONFIG_FILE_NAME, localTimeout, remoteTimeout, -1L);
    }

    /**
     * Restart VTNManager after configurations of flowModTimeout and
     * remoteFlowModTimeout are set.
     *
     * @param dir           A directory configuration file is located.
     * @param fileName      A name of configuration file.
     * @param localTimeout  A time set as {@code localTimeout}.
     * @param remoteTimeout A time set as {@code remoteTimeout}.
     */
    protected void setupVTNManagerForRemoteTaskTest(String dir, String fileName,
                                                    long localTimeout,
                                                    long remoteTimeout,
                                                    long remoteBulkTimeout) {
        FileWriter gWriter;
        File gIniFile = new File(dir, fileName);
        try {
            gWriter = new FileWriter(gIniFile);
            if (localTimeout > 0) {
                gWriter.write("flowModTimeout=" + localTimeout + "\n");
            }
            if (remoteTimeout > 0) {
                gWriter.write("remoteFlowModTimeout=" + remoteTimeout + "\n");
            }
            if (remoteBulkTimeout > 0) {
                gWriter.write("remoteBulkFlowModTimeout=" + remoteBulkTimeout + "\n");
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

    /**
     * Mock-up of {@link IVTNManagerAware}.
     */
    public class VTNManagerAwareStub implements IVTNManagerAware {
        /**
         * Utility class used in VTNManagerAwareStub.
         */
        class VTNManagerAwareData<T, S> {
            T path = null;
            S obj = null;
            UpdateType type = null;
            int count = 0;

            VTNManagerAwareData(T p, S o, UpdateType t, int c) {
                path = p;
                obj = o;
                type = t;
                count = c;
            }
        }

        private final long sleepMilliTime = 10L;

        private int vtnChangedCalled = 0;
        private int vbrChangedCalled = 0;
        private int vIfChangedCalled = 0;
        private int vlanMapChangedCalled = 0;
        private int portMapChangedCalled = 0;
        VTNManagerAwareData<VTenantPath, VTenant> vtnChangedInfo = null;
        VTNManagerAwareData<VBridgePath, VBridge> vbrChangedInfo = null;
        VTNManagerAwareData<VBridgeIfPath, VInterface> vIfChangedInfo = null;
        VTNManagerAwareData<VBridgePath, VlanMap> vlanMapChangedInfo = null;
        VTNManagerAwareData<VBridgeIfPath, PortMap> portMapChangedInfo = null;

        @Override
        public synchronized void vtnChanged(VTenantPath path, VTenant vtenant,
                                            UpdateType type) {
            vtnChangedCalled++;
            vtnChangedInfo = new VTNManagerAwareData<VTenantPath, VTenant>(path, vtenant,
                    type, vtnChangedCalled);
            notify();
        }

        @Override
        public synchronized void vBridgeChanged(VBridgePath path, VBridge vbridge,
                                                UpdateType type) {
            vbrChangedCalled++;
            vbrChangedInfo = new VTNManagerAwareData<VBridgePath, VBridge>(path, vbridge, type,
                    vbrChangedCalled);
            notify();
        }

        @Override
        public synchronized void vBridgeInterfaceChanged(VBridgeIfPath path,
                                                         VInterface viface,
                                                         UpdateType type) {
            vIfChangedCalled++;
            vIfChangedInfo = new VTNManagerAwareData<VBridgeIfPath, VInterface>(path, viface, type,
                    vIfChangedCalled);
            notify();
        }

        @Override
        public synchronized void vlanMapChanged(VBridgePath path, VlanMap vlmap,
                                                UpdateType type) {
            vlanMapChangedCalled++;
            vlanMapChangedInfo = new VTNManagerAwareData<VBridgePath, VlanMap>(path, vlmap, type,
                    vlanMapChangedCalled);
            notify();
        }

        @Override
        public synchronized void portMapChanged(VBridgeIfPath path, PortMap pmap,
                                                UpdateType type) {
            portMapChangedCalled++;
            portMapChangedInfo = new VTNManagerAwareData<VBridgeIfPath, PortMap>(path, pmap, type,
                    portMapChangedCalled);
            notify();
        }

        /**
         * Check information notified by {@code vtnChanged()}.
         *
         * @param count     An expected number of times {@code vtnChanged()}
         *                  was called.
         * @param path      A {@link VTenantPath} expect to be notified.
         * @param type      A type expect to be notified.
         */
        public synchronized void checkVtnInfo(int count, VTenantPath path,
                                       UpdateType type) {
            if (vtnChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vtnChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vtnChangedCalled);

            if (path != null) {
                assertEquals(path, vtnChangedInfo.path);
                assertEquals(path.getTenantName(), vtnChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vtnChangedInfo.type);
            }
            vtnChangedCalled = 0;
            vtnChangedInfo = null;
        }

        /**
         * Check information notified by {@code vBridgeChanged()}.
         *
         * @param count     An expected number of times {@code vBridgeChanged()}
         *                  was called.
         * @param path      A {@link VBridgePath} expect to be notified.
         * @param type      A type expect to be notified.
         */
        synchronized void checkVbrInfo(int count, VBridgePath path,
                                       UpdateType type) {
            if (vbrChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vbrChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vbrChangedCalled);

            if (path != null) {
                assertEquals(path, vbrChangedInfo.path);
                assertEquals(path.getBridgeName(), vbrChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vbrChangedInfo.type);
            }
            vbrChangedCalled = 0;
            vbrChangedInfo = null;
        }

        /**
         * Check information notified by {@code vBridgeInterfaceChanged()}.
         *
         * @param count     An expected number of times
         *                  {@code vBridgeInterfaceChanged()} was called.
         * @param path      A {@link VBridgeIfPath} expect to be notified.
         * @param type      A type expect to be notified.
         */
        synchronized void checkVIfInfo(int count, VBridgeIfPath path,
                                      UpdateType type) {
            if (vIfChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vIfChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vIfChangedCalled);

            if (path != null) {
                assertEquals(path, vIfChangedInfo.path);
                assertEquals(path.getInterfaceName(), vIfChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vIfChangedInfo.type);
            }
            vIfChangedCalled = 0;
            vIfChangedInfo = null;
        }

        /**
         * Check information notified by {@code vlanMapChanged()}.
         *
         * @param count     An expected number of times {@code vlanMapChanged()}
         *                  was called.
         * @param path      A {@link VBridgePath} expect to be notified.
         * @param id        A map-id expect to be notified.
         * @param type      A type expect to be notified.
         */
        synchronized void checkVlmapInfo(int count, VBridgePath path,
                                         String id, UpdateType type) {
            if (vlanMapChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vlanMapChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vlanMapChangedCalled);

            if (path != null) {
                assertEquals(path, vlanMapChangedInfo.path);
            }
            if (id != null) {
                assertEquals(id, vlanMapChangedInfo.obj.getId());
            }
            if (type != null) {
                assertEquals(type, vlanMapChangedInfo.type);
            }
            vlanMapChangedCalled = 0;
            vlanMapChangedInfo = null;
        }

        /**
         * Check information notified by {@code portMapChanged()}.
         *
         * @param count     An expected number of times {@code portMapChanged()}
         *                  was called.
         * @param path      A {@link VBridgeIfPath} expect to be notified.
         * @param pconf     A {@link PortMapConfig} expect to be notified.
         * @param type      A type expect to be notified.
         */
        synchronized void checkPmapInfo(int count, VBridgeIfPath path,
                                        PortMapConfig pconf, UpdateType type) {
            if (portMapChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (portMapChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, portMapChangedCalled);

            if (path != null) {
                assertEquals(path, portMapChangedInfo.path);
            }
            if (pconf != null) {
                assertEquals(pconf, portMapChangedInfo.obj.getConfig());
            }
            if (type != null) {
                assertEquals(type, portMapChangedInfo.type);
            }
            portMapChangedCalled = 0;
            portMapChangedInfo = null;
        }

        /**
         * Check all methods not called.
         */
        public synchronized void checkAllNull() {
            sleep(sleepMilliTime);
            assertEquals(0, vtnChangedCalled);
            assertNull(vtnChangedInfo);
            assertEquals(0, vbrChangedCalled);
            assertNull(vbrChangedInfo);
            assertEquals(0, vIfChangedCalled);
            assertNull(vIfChangedInfo);
            assertEquals(0, vlanMapChangedCalled);
            assertNull(vlanMapChangedInfo);
            assertEquals(0, portMapChangedCalled);
            assertNull(portMapChangedInfo);
        }
    }

    /**
     * Add {@link IVTNManagerAware} service to VTNManager service.
     * @param listener  {@link IVTNManagerAware} service.
     */
    public void addVTNManagerAware(IVTNManagerAware listener) {
        vtnMgr.addVTNManagerAware(listener);
    }

    /**
     *  Mock-up of {@link IfHostListener}.
     */
    class HostListener implements IfHostListener {
        private int hostListenerCalled = 0;

        @Override
        public synchronized void hostListener(HostNodeConnector host) {
            hostListenerCalled++;
        }

        /**
         * get times hostListener called.
         * @return the number of times hostListener was called.
         */
        synchronized int getHostListenerCalled () {
            int ret = hostListenerCalled;
            hostListenerCalled = 0;
            return  ret;
        }
    }

    /**
     * Mock-up of {@link IVTNModeListener}.
     */
    class VTNModeListenerStub implements IVTNModeListener {
        private int calledCount = 0;
        private Boolean oldactive = null;

        @Override
        public synchronized void vtnModeChanged(boolean active) {
            calledCount++;
            oldactive = Boolean.valueOf(active);
            notify();
        }

        private synchronized int getCalledCount(int expected) {
            if (calledCount < expected) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (calledCount >= expected) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }

            int ret = calledCount;
            calledCount = 0;
            return ret;
        }

        private synchronized Boolean getCalledArg() {
            if (oldactive == null) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (oldactive != null) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }

            Boolean ret = oldactive;
            oldactive = null;
            return ret;
        }

        /**
         * Check information registered by {@code vtnModeChanged()}.
         *
         * @param expCount  An expected number of times {@code vtnModeChanged()}
         *                  was called.
         * @param expMode   An expected mode.
         */
        void checkCalledInfo(int expCount, Boolean expMode) {
            checkCalledInfo(expCount);
            assertEquals(expMode, this.getCalledArg());
        }

        /**
         * Check information registered by {@code vtnModeChanged()}.
         * @param expCount  An expected number of times {@code vtnModeChanged()}
         *                  was called.
         */
        void checkCalledInfo(int expCount) {
            if (expCount >= 0) {
                assertEquals(expCount, this.getCalledCount(expCount));
            }
        }
    }

    /**
     * Create caches shared between cluster nodes.
     *
     * @param cs    Cluster Service managing cache.
     * @param isClustered   if {@code true} create macAddressDB cache.
     *                      if {@code false} don't create it.
     */
    protected void createClusterCache(IClusterContainerServices cs,
                                      boolean isClustered) {
        Set<IClusterServices.cacheMode> cmode =
                EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL,
                           IClusterServices.cacheMode.SYNC);
        Set<IClusterServices.cacheMode> cmodeNonTransactional =
                EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL,
                           IClusterServices.cacheMode.SYNC);

        try {
            cs.createCache(VTNManagerImpl.CACHE_TENANT, cmode);
            cs.createCache(VTNManagerImpl.CACHE_STATE, cmode);
            cs.createCache(VTNManagerImpl.CACHE_NODES, cmode);
            cs.createCache(VTNManagerImpl.CACHE_PORTS, cmode);
            cs.createCache(VTNManagerImpl.CACHE_ISL, cmode);
            if (isClustered) {
                cs.createCache(VTNManagerImpl.CACHE_MAC, cmode);
            }
            cs.createCache(VTNManagerImpl.CACHE_EVENT, cmodeNonTransactional);
            cs.createCache(VTNManagerImpl.CACHE_FLOWS, cmodeNonTransactional);
        } catch (CacheExistException e) {
            unexpected(e);
        } catch (CacheConfigException e) {
            unexpected(e);
        }
    }

    /**
     * Add FlowEntry to VTNFlow.
     * Added FlowEntry matches to IN_PORT and DL_VLAN and have action
     * output to port.
     *
     * @param   flow        A {@link VTNFlow}.
     * @param   inPort      An ingress {@link NodeConnector}.
     * @param   inVlan      An incoming VLAN ID.
     * @param   outPort     An outgoing {@link NodeConnector}.
     * @param   priority    A priority of {@link FlowEntry}.
     * @return {@link VTNFlow}.
     */
    protected VTNFlow addFlowEntry(VTNManagerImpl mgr, VTNFlow flow,
            NodeConnector inPort, short inVlan, NodeConnector outPort,
            int priority) {
        Match match = new Match();
        match.setField(MatchType.IN_PORT, inPort);
        match.setField(MatchType.DL_VLAN, inVlan);
        ActionList actions = new ActionList(outPort.getNode());
        actions.addOutput(outPort);
        flow.addFlow(mgr, match, actions, priority);

        return flow;
    }

    /**
     * Check specified Flow Entry is registered correctly.
     *
     * @param numFlows          The number of Flows.
     * @param registeredFlow    VTNFlow which is registered.
     * @param numFlowEntries    The number of Flow Entries.
     */
    protected void checkRegisteredFlowEntry(VTNManagerImpl mgr, int numFlows,
                                         VTNFlow registeredFlow, VTNFlow expectedFlow,
                                         int numFlowEntries, String emsg) {
        ConcurrentMap<FlowGroupId, VTNFlow> db = mgr.getFlowDB();
        assertEquals(emsg, numFlows, db.size());
        assertEquals(emsg, expectedFlow, db.get(registeredFlow.getGroupId()));
        assertEquals(emsg, numFlowEntries, stubObj.getFlowEntries().size());
    }
}
