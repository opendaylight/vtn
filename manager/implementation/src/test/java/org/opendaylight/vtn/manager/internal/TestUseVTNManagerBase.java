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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Dictionary;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.apache.felix.dm.impl.ComponentImpl;

import org.junit.After;
import org.junit.Before;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.MacMapEvent;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterServices;
import org.opendaylight.controller.containermanager.IContainerManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IDataPacketService;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;

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
    private final boolean useHostTracker;

    /**
     * The number of milliseconds to wait for VTN events.
     */
    protected static final long  EVENT_TIMEOUT = 10000L;

    /**
     * Construct a new instance.
     *
     * @param stub  An integer value to be passed to {@link TestStub}.
     */
    protected TestUseVTNManagerBase(int stub) {
        this(stub, false);
    }

    /**
     * Construct a new instance.
     *
     * @param stub  An integer value to be passed to {@link TestStub}.
     * @param ht    If {@code true}, use host tracker emulator.
     */
    protected TestUseVTNManagerBase(int stub, boolean ht) {
        stubMode = stub;
        useHostTracker = ht;
    }

    @Before
    public void before() {
        setupStartupDir();

        vtnMgr = new VTNManagerImpl();
        resMgr = new GlobalResourceManager();
        ComponentImpl c = new ComponentImpl(null, null, null);
        stubObj = new TestStub(stubMode, useHostTracker);

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
        vtnMgr.setContainerManager(stubObj);
        startVTNManager(c);
        if (stubObj.isHostTrackerEnabled()) {
            vtnMgr.addHostListener(stubObj);
        }
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
        Dictionary<?, ?> props = c.getServiceProperties();
        if (props != null) {
            String name = (String)props.get("containerName");
            IContainerManager ctmgr = vtnMgr.getContainerManager();
            if (GlobalConstants.DEFAULT.toString().equals(name)) {
                if (ctmgr == null) {
                    vtnMgr.setContainerManager(stubObj);
                }
            } else if (ctmgr != null) {
                vtnMgr.unsetContainerManager(ctmgr);
            }
        }
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
        if (stubObj.isHostTrackerEnabled()) {
            vtnMgr.removeHostListener(stubObj);
        }
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
        vtnMgr.setContainerManager(stub);
        startVTNManager(c);
        if (stub.isHostTrackerEnabled()) {
            vtnMgr.addHostListener(stub);
        }
    }

    /**
     * stop VTNManager.
     *
     * @param clearCache    if true clear cache maintained in VTNManager.
     */
    protected void stopVTNManager(boolean clearCache) {
        vtnMgr.stopping();
        if (stubObj.isHostTrackerEnabled()) {
            vtnMgr.removeHostListener(stubObj);
        }
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
        flushTasks();

        long t = (wait < EVENT_TIMEOUT) ? EVENT_TIMEOUT : wait;
        NopFlowTask task = new NopFlowTask(vtnMgr);
        vtnMgr.postFlowTask(task);
        assertTrue(task.await(t, TimeUnit.MILLISECONDS));
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
        public synchronized void run() {
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

    private static final String CONFIG_FILE_NAME = "vtnmanager.ini";

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
        private int macMapChangedCalled = 0;
        private int vtermChangedCalled = 0;
        private int vtermIfChangedCalled = 0;
        private int vtermPortMapChangedCalled = 0;

        VTNManagerAwareData<VTenantPath, VTenant> vtnChangedInfo;
        VTNManagerAwareData<VBridgePath, VBridge> vbrChangedInfo;
        VTNManagerAwareData<VBridgeIfPath, VInterface> vIfChangedInfo;
        VTNManagerAwareData<VBridgePath, VlanMap> vlanMapChangedInfo;
        VTNManagerAwareData<VBridgeIfPath, PortMap> portMapChangedInfo;
        VTNManagerAwareData<VBridgePath, MacMapConfig> macMapChangedInfo;
        VTNManagerAwareData<VTerminalPath, VTerminal> vtermChangedInfo;
        VTNManagerAwareData<VTerminalIfPath, VInterface> vtermIfChangedInfo;
        VTNManagerAwareData<VTerminalIfPath, PortMap> vtermPortMapChangedInfo;

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
        public synchronized void vTerminalChanged(VTerminalPath path,
                                                  VTerminal vterm,
                                                  UpdateType type) {
            vtermChangedCalled++;
            vtermChangedInfo =
                new VTNManagerAwareData<VTerminalPath, VTerminal>(
                    path, vterm, type, vtermChangedCalled);
            notify();
        }

        @Override
        public synchronized void vInterfaceChanged(VBridgeIfPath path,
                                                   VInterface viface,
                                                   UpdateType type) {
            vIfChangedCalled++;
            vIfChangedInfo =
                new VTNManagerAwareData<VBridgeIfPath, VInterface>(
                    path, viface, type, vIfChangedCalled);
            notify();
        }

        @Override
        public synchronized void vInterfaceChanged(VTerminalIfPath path,
                                                   VInterface viface,
                                                   UpdateType type) {
            vtermIfChangedCalled++;
            vtermIfChangedInfo =
                new VTNManagerAwareData<VTerminalIfPath, VInterface>(
                    path, viface, type, vIfChangedCalled);
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

        @Override
        public synchronized void portMapChanged(VTerminalIfPath path,
                                                PortMap pmap, UpdateType type) {
            vtermPortMapChangedCalled++;
            vtermPortMapChangedInfo =
                new VTNManagerAwareData<VTerminalIfPath, PortMap>(
                    path, pmap, type, vtermPortMapChangedCalled);
            notify();
        }

        @Override
        public synchronized void macMapChanged(VBridgePath path,
                                               MacMapConfig mcconf,
                                               UpdateType type) {
            macMapChangedCalled++;
            macMapChangedInfo =
                new VTNManagerAwareData<VBridgePath, MacMapConfig>(
                    path, mcconf, type, macMapChangedCalled);
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
         * Check information notified by {@code vTerminalChanged()}.
         *
         * @param count     An expected number of times
         *                  {@code vTerminalChanged()} was called.
         * @param path      A {@link VTerminalPath} expected to be notified.
         * @param type      A type expected to be notified.
         */
        synchronized void checkVTermInfo(int count, VTerminalPath path,
                                         UpdateType type) {
            if (vtermChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vtermChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vtermChangedCalled);

            if (path != null) {
                assertEquals(path, vtermChangedInfo.path);
                assertEquals(path.getTerminalName(),
                             vtermChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vtermChangedInfo.type);
            }
            vtermChangedCalled = 0;
            vtermChangedInfo = null;
        }

        /**
         * Check vBridge interface information notified by
         * {@code vInterfaceChanged()}.
         *
         * @param count     An expected number of times
         *                  {@code vInterfaceChanged()} was called.
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
         * Check vTerminal interface information notified by
         * {@code vInterfaceChanged()}.
         *
         * @param count     An expected number of times
         *                  {@code vInterfaceChanged()} was called.
         * @param path      A {@link VTerminalIfPath} expected to be notified.
         * @param type      A type expect to be notified.
         */
        synchronized void checkVIfInfo(int count, VTerminalIfPath path,
                                       UpdateType type) {
            if (vtermIfChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vtermIfChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vtermIfChangedCalled);

            if (path != null) {
                assertEquals(path, vtermIfChangedInfo.path);
                assertEquals(path.getInterfaceName(),
                             vtermIfChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vtermIfChangedInfo.type);
            }
            vtermIfChangedCalled = 0;
            vtermIfChangedInfo = null;
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
         * Check information notified by {@code portMapChanged()} for
         * bridge interface.
         *
         * @param count     An expected number of times {@code portMapChanged()}
         *                  was called.
         * @param path      A {@link VBridgeIfPath} expected to be notified.
         * @param pconf     A {@link PortMapConfig} expected to be notified.
         * @param type      A type expected to be notified.
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
         * Check information notified by {@code portMapChanged()} for
         * vTerminal interface.
         *
         * @param count     An expected number of times {@code portMapChanged()}
         *                  was called.
         * @param path      A {@link VTerminalIfPath} expected to be notified.
         * @param pconf     A {@link PortMapConfig} expected to be notified.
         * @param type      A type expected to be notified.
         */
        synchronized void checkPmapInfo(int count, VTerminalIfPath path,
                                        PortMapConfig pconf, UpdateType type) {
            if (vtermPortMapChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vtermPortMapChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vtermPortMapChangedCalled);

            if (path != null) {
                assertEquals(path, vtermPortMapChangedInfo.path);
            }
            if (pconf != null) {
                assertEquals(pconf, vtermPortMapChangedInfo.obj.getConfig());
            }
            if (type != null) {
                assertEquals(type, vtermPortMapChangedInfo.type);
            }
            vtermPortMapChangedCalled = 0;
            vtermPortMapChangedInfo = null;
        }

        /**
         * Check information notified by {@code macMapChanged()}.
         *
         * @param count     An expected number of times {@code macMapChanged()}
         *                  was called.
         * @param path      A {@link VBridgePath} expected to be notified.
         * @param mcconf    A {@link MacMapConfig} expected to be notified.
         * @param type      A type expected to be notified.
         */
        synchronized void checkMmapInfo(int count, VBridgePath path,
                                        MacMapConfig mcconf, UpdateType type) {
            if (macMapChangedCalled < count) {
                long milli = EVENT_TIMEOUT;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (macMapChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, macMapChangedCalled);

            if (path != null) {
                assertEquals(path, macMapChangedInfo.path);
            }
            if (mcconf != null) {
                assertEquals(mcconf, macMapChangedInfo.obj);
            }
            if (type != null) {
                assertEquals(type, macMapChangedInfo.type);
            }
            macMapChangedCalled = 0;
            macMapChangedInfo = null;
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
            assertEquals(0, macMapChangedCalled);
            assertNull(macMapChangedInfo);
            assertEquals(0, vtermChangedCalled);
            assertNull(vtermChangedInfo);
            assertEquals(0, vtermIfChangedCalled);
            assertNull(vtermIfChangedInfo);
            assertEquals(0, vtermPortMapChangedCalled);
            assertNull(vtermPortMapChangedInfo);
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
        synchronized int getHostListenerCalled() {
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
     * @param mgr       VTN Manager service.
     * @param flow      A {@link VTNFlow}.
     * @param inPort    An ingress {@link NodeConnector}.
     * @param src       A source MAC address.
     * @param inVlan    An incoming VLAN ID.
     * @param outPort   An outgoing {@link NodeConnector}.
     * @param dst       A destination MAC address.
     * @param priority  A priority of {@link FlowEntry}.
     * @return {@link VTNFlow}.
     */
    protected VTNFlow addFlowEntry(VTNManagerImpl mgr, VTNFlow flow,
                                   NodeConnector inPort, byte[] src,
                                   short inVlan, NodeConnector outPort,
                                   byte[] dst, int priority) {
        Match match = new Match();
        match.setField(MatchType.IN_PORT, inPort);
        match.setField(MatchType.DL_VLAN, inVlan);
        match.setField(MatchType.DL_SRC, src);
        match.setField(MatchType.DL_DST, dst);
        ActionList actions = new ActionList(outPort.getNode());
        actions.addOutput(outPort);
        flow.addFlow(mgr, match, actions, priority);

        return flow;
    }

    /**
     * Add FlowEntry to VTNFlow.
     * Added FlowEntry matches to IN_PORT and DL_VLAN and have action
     * output to port.
     *
     * <p>
     *   Fixed MAC address is used for source and destination MAC address.
     * </p>
     *
     * @param mgr       VTN Manager service.
     * @param flow      A {@link VTNFlow}.
     * @param inPort    An ingress {@link NodeConnector}.
     * @param inVlan    An incoming VLAN ID.
     * @param outPort   An outgoing {@link NodeConnector}.
     * @param priority  A priority of {@link FlowEntry}.
     * @return {@link VTNFlow}.
     */
    protected VTNFlow addFlowEntry(VTNManagerImpl mgr, VTNFlow flow,
                                   NodeConnector inPort, short inVlan,
                                   NodeConnector outPort, int priority) {
        byte[] src = {
            (byte)0x00, (byte)0x11, (byte)0x22,
            (byte)0x33, (byte)0x44, (byte)0x55,
        };
        byte[] dst = {
            (byte)0xf0, (byte)0xfa, (byte)0xfb,
            (byte)0xfc, (byte)0xfd, (byte)0xfe,
        };

        return addFlowEntry(mgr, flow, inPort, src, inVlan, outPort, dst,
                            priority);
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

    /**
     * Verify contents of the specified {@link MacMapEvent} instance.
     *
     * @param ev      A {@link MacMapEvent} instance to be tested.
     * @param path    An expected vBridge path.
     * @param mcconf  An expected MAC mapping configuration.
     * @param utype   An expected {@link UpdateType} instance.
     * @param save    An expected boolean value configured in {@code ev}.
     */
    protected void checkEvent(MacMapEvent ev, VBridgePath path,
                              MacMapConfig mcconf, UpdateType utype,
                              boolean save) {
        assertEquals(path, ev.getPath());
        assertEquals(mcconf, ev.getObject());
        assertEquals(mcconf, ev.getMacMapConfig());
        assertEquals(utype, ev.getUpdateType());
        assertEquals("MAC mapping", ev.getTypeName());
        assertTrue(ev.isSingleThreaded(true));
        assertTrue(ev.isSingleThreaded(false));

        boolean saveConfig = (utype == UpdateType.REMOVED)
            ? save : true;
        assertEquals(saveConfig, ev.isSaveConfig());
    }

    /**
     * Ensure that a broadcast packet was sent correctly.
     *
     * @param pkt   A broadcast packet sent to the managed network.
     * @param pset  A set of {@link PortVlan} instances to which the specified
     *              broadcast packet should be sent.
     */
    protected void checkBroadcastPacket(RawPacket pkt, Set<PortVlan> pset) {
        IDataPacketService pktSrv = vtnMgr.getDataPacketService();
        Packet decoded = pktSrv.decodeDataPacket(pkt);
        PacketContext pctx = new PacketContext(pkt, (Ethernet)decoded);

        // Packet.equals(Object) will not work if it contains byte array.
        // So we need to compare serialized payload.
        byte[] payload = null;
        try {
            payload = pctx.getPayload().serialize();
        } catch (Exception e) {
            unexpected(e);
        }

        List<RawPacket> transmitted = stubObj.getTransmittedDataPacket();
        if (pset == null || pset.isEmpty()) {
            assertEquals(0, transmitted.size());
            return;
        }
        assertEquals(pset.size(), transmitted.size());

        Set<PortVlan> bcast = new HashSet<PortVlan>(pset);
        for (RawPacket sent: transmitted) {
            PacketContext spctx = checkPacket(sent, payload);
            short vlan = spctx.getVlan();
            NodeConnector nc = spctx.getOutgoingNodeConnector();
            PortVlan pvlan = new PortVlan(nc, vlan);
            assertTrue(bcast.remove(pvlan));
        }
        assertEquals(0, bcast.size());
    }

    /**
     * Ensure that an unicast packet was sent correctly.
     *
     * @param pkt   An unicast packet sent to the managed network.
     * @param port  A {@link NodeConnector} instance corresponding to the
     *              destination switch port. {@code null} means that the
     *              packet should be dropped.
     * @param vlan  A VLAN ID of the destination network.
     */
    protected void checkUnicastPacket(RawPacket pkt, NodeConnector port,
                                      short vlan) {
        IDataPacketService pktSrv = vtnMgr.getDataPacketService();
        Packet decoded = pktSrv.decodeDataPacket(pkt);
        PacketContext pctx = new PacketContext(pkt, (Ethernet)decoded);

        // Packet.equals(Object) will not work if it contains byte array.
        // So we need to compare serialized payload.
        byte[] payload = null;
        try {
            payload = pctx.getPayload().serialize();
        } catch (Exception e) {
            unexpected(e);
        }

        List<RawPacket> transmitted = stubObj.getTransmittedDataPacket();
        if (port == null) {
            assertEquals(0, transmitted.size());
            return;
        }

        assertEquals(1, transmitted.size());
        RawPacket sent = transmitted.get(0);
        PacketContext spctx = checkPacket(sent, payload);
        NodeConnector nc = spctx.getOutgoingNodeConnector();
        assertEquals(port, nc);
        assertEquals(vlan, spctx.getVlan());
    }

    /**
     * Ensure that the given raw packet has the same payload as the specified.
     *
     * @param pkt      A {@link RawPacket} to be tested.
     *                 An outgoing node connector must be configured in the
     *                 specified packet.
     * @param payload  An array of bytes which represents the expected payload.
     * @return  A {@link PacketContext} instance which contains the specified
     *          packet.
     */
    protected PacketContext checkPacket(RawPacket pkt, byte[] payload) {
        NodeConnector port = pkt.getOutgoingNodeConnector();
        assertNotNull(port);
        Packet decoded = stubObj.decodeDataPacket(pkt);
        PacketContext pctx = new PacketContext((Ethernet)decoded, port);

        try {
            byte[] pld = pctx.getPayload().serialize();
            if (!Arrays.equals(payload, pld)) {
                StringBuilder builder =
                    new StringBuilder("Unexpecetd payload: expected=");
                for (byte b: payload) {
                    builder.append(String.format("%02x", b & 0xff));
                }
                builder.append(", actual=");
                for (byte b: pld) {
                    builder.append(String.format("%02x", b & 0xff));
                }
                fail(builder.toString());
            }
        } catch (Exception e) {
            unexpected(e);
        }

        return pctx;
    }

    /**
     * Determine broadcast domain of the specified virtual node.
     *
     * @param path    A path to the virtual node.
     * @param host    A {@link TestHost} instance which represents the source
     *                host of a broadcast packet.
     * @return  A set of {@link PortVlan} instances to which a broadcast packet
     *          from {@code host} should be sent.
     */
    protected Set<PortVlan> getBroadcastNetwork(VNodePath path,
                                                TestHost host) {
        Set<PortVlan> pset = new HashSet<PortVlan>();
        Set<Node> vlanNodes = new HashSet<Node>();
        if (!(path instanceof VBridgePath)) {
            // No broadcast domain.
            return pset;
        }

        VBridgePath bpath = (VBridgePath)path;
        try {
            // Test port mappings.
            for (VInterface vif: vtnMgr.getInterfaces(bpath)) {
                if (vif.getState() != VNodeState.UP) {
                    continue;
                }

                VBridgeIfPath ifpath = new VBridgeIfPath(bpath, vif.getName());
                PortMap pmap = vtnMgr.getPortMap(ifpath);
                if (pmap == null) {
                    continue;
                }

                NodeConnector port = pmap.getNodeConnector();
                PortMapConfig pmconf = pmap.getConfig();
                pset.add(new PortVlan(port, pmconf.getVlan()));
            }

            // Test MAC mapping.
            MacMapPath mpath = new MacMapPath(bpath);
            Set<PortVlan> nwSet = resMgr.getMacMappedNetworks(vtnMgr, mpath);
            if (nwSet != null) {
                pset.addAll(nwSet);
            }

            // Test VLAN mappings.
            ISwitchManager swMgr = vtnMgr.getSwitchManager();
            ITopologyManager topoMgr = vtnMgr.getTopologyManager();
            String container = vtnMgr.getContainerName();
            for (VlanMap vmap: vtnMgr.getVlanMaps(bpath)) {
                short vlan = vmap.getVlan();
                Node node = vmap.getNode();
                if (node == null) {
                    for (Node n: swMgr.getNodes()) {
                        for (NodeConnector nc: swMgr.getUpNodeConnectors(n)) {
                            if (swMgr.isSpecial(nc) ||
                                topoMgr.isInternal(nc)) {
                                continue;
                            }
                            PortVlan pvlan = new PortVlan(nc, vlan);
                            if (!resMgr.isPortMapped(pvlan)) {
                                pset.add(pvlan);
                            }
                        }
                    }
                    break;
                }
                if (vlanNodes.add(node)) {
                    for (NodeConnector nc: swMgr.getUpNodeConnectors(node)) {
                        if (swMgr.isSpecial(nc) || topoMgr.isInternal(nc)) {
                            continue;
                        }
                        PortVlan pvlan = new PortVlan(nc, vlan);
                        if (!resMgr.isPortMapped(pvlan)) {
                            pset.add(pvlan);
                        }
                    }
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }

        // Eliminate source VLAN network from the set.
        pset.remove(host.getPortVlan());

        return pset;
    }

    /**
     * Return a {@link VTNFlow} instance which represents the specified flow.
     *
     * @param name  The name of the virtual tenant.
     * @param src   A {@link TestHost} for the source host.
     * @param dst   A {@link TestHost} for the destination host.
     * @return  A {@link VTNFlow} instance is returned if found.
     *          {@code null} is returned if not found.
     */
    protected VTNFlow getVTNFlow(String name, TestHost src, TestHost dst) {
        final L2Host srcHost = src.getL2Host();
        final L2Host dstHost = dst.getL2Host();
        final List<VTNFlow> found = new ArrayList<VTNFlow>();

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(name);
        VTNFlowMatch fmatch = new VTNFlowMatch() {
            @Override
            public boolean accept(VTNFlow vflow) {
                ObjectPair<L2Host, L2Host> edges = vflow.getEdgeHosts();
                if (srcHost.equals(edges.getLeft()) &&
                    dstHost.equals(edges.getRight())) {
                    found.add(vflow);
                }
                return false;
            }

            @Override
            public String getDescription() {
                StringBuilder builder = new StringBuilder("src=");
                builder.append(srcHost).append(", dst=").append(dstHost);
                return builder.toString();
            }
        };

        assertNull(fdb.removeFlows(vtnMgr, fmatch));
        int sz = found.size();
        if (sz == 0) {
            return null;
        }

        assertEquals(1, sz);
        VTNFlow vflow = found.get(0);

        // Ensure that all flow entries are installed.
        for (FlowEntry fent: vflow.getFlowEntries()) {
            assertTrue(stubObj.isInstalled(fent));
        }

        return vflow;
    }

    /**
     * Ensure that the specified flow entry is installed.
     *
     * @param name   The name of the virtual tenant.
     * @param vflow  A {@link VTNFlow} instance to be tested.
     */
    protected void checkVTNFlowInstalled(String name, VTNFlow vflow) {
        final FlowGroupId gid = vflow.getGroupId();
        final List<Boolean> found = new ArrayList<Boolean>(1);
        VTNFlowMatch fmatch = new VTNFlowMatch() {
            @Override
            public boolean accept(VTNFlow vf) {
                if (gid.equals(vf.getGroupId())) {
                    found.add(Boolean.TRUE);
                }
                return false;
            }

            @Override
            public String getDescription() {
                return "gid=" + gid;
            }
        };

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(name);
        assertNull(fdb.removeFlows(vtnMgr, fmatch));
        assertFalse(found.isEmpty());

        for (FlowEntry fent: vflow.getFlowEntries()) {
            assertTrue(stubObj.isInstalled(fent));
        }
    }

    /**
     * Ensure that the specified flow entry is no longer installed.
     *
     * @param name   The name of the virtual tenant.
     * @param vflow  A {@link VTNFlow} instance to be tested.
     */
    protected void checkVTNFlowUninstalled(String name, VTNFlow vflow) {
        List<VTNFlow> flows = new ArrayList<VTNFlow>(1);
        flows.add(vflow);

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(name);
        assertNull(fdb.removeFlows(vtnMgr, flows));

        for (FlowEntry fent: vflow.getFlowEntries()) {
            assertFalse(stubObj.isInstalled(fent));
        }
    }

    /**
     * Ensure that incoming packet is handled according to the virtual mapping
     * configuration.
     *
     * @param bridges    A list of vBridge paths in the container.
     * @param allHosts   A list of hosts to be tested.
     * @param flowMap    A map which contains {@link VTNFlow} instances
     *                   currently installed to the flow database.
     * @param flowCount  The number of flow entries currently installed.
     * @return  The number of flow entries after test is returned.
     */
    protected int checkMapping(List<VBridgePath> bridges,
                               List<TestHost> allHosts,
                               Map<FlowGroupId, VTNFlow> flowMap,
                               int flowCount) {
        // At first, try to learn all hosts without installing flow entries.
        Map<TestHost, Set<PortVlan>> bcastMap =
            new HashMap<TestHost, Set<PortVlan>>();
        for (TestHost host: allHosts) {
            checkMapping(bridges, host, flowCount, null, bcastMap);
        }

        // Send unicast packet.
        int fcnt = flowCount;
        for (TestHost src: allHosts) {
            MapReference sref = src.getMapping();
            VBridgePath bpath;
            PacketResult pres;
            if (sref == null) {
                pres = PacketResult.IGNORED;
                bpath = null;
            } else {
                pres = PacketResult.KEEP_PROCESSING;
                VBridgePath mpath = (VBridgePath)sref.getPath();
                bpath = new VBridgePath(mpath.getTenantName(),
                                        mpath.getBridgeName());
            }

            for (TestHost dst: allHosts) {
                NodeConnector dport = dst.getPort();
                if (src.equals(dst) || src.getPort().equals(dport)) {
                    continue;
                }

                VTNFlow oldVflow = (bpath == null)
                    ? null : getVTNFlow(bpath.getTenantName(), src, dst);
                MapReference dref = dst.getMapping();
                RawPacket pkt = src.createArp(dst);
                assertEquals(pres, vtnMgr.receiveDataPacket(pkt));
                flushTasks();
                flushFlowTasks();

                if (bpath == null) {
                    // Source host is not mapped.
                    checkBroadcastPacket(pkt, null);
                } else {
                    String tname = bpath.getTenantName();
                    VTNFlow vflow = getVTNFlow(tname, src, dst);
                    if (dref != null && bpath.contains(dref.getPath())) {
                        // Both source and destination host are mapped to the
                        // same vBridge.
                        assertNotNull(vflow);
                        checkUnicastPacket(pkt, dport, dst.getVlan());
                        assertEquals(oldVflow,
                                     flowMap.put(vflow.getGroupId(), vflow));
                        if (oldVflow == null) {
                            fcnt += vflow.getFlowEntries().size();
                        }
                    } else {
                        // Destination host is not mapped, or is mapped to
                        // different vBridge.
                        assertNull(oldVflow);
                        assertNull(vflow);
                        checkBroadcastPacket(pkt, bcastMap.get(src));
                    }
                }
                stubObj.checkFlowCount(fcnt);
            }
        }

        return fcnt;
    }

    /**
     * Ensure that incoming packet is handled according to the virtual mapping
     * configuration.
     *
     * @param bridges    A list of vBridge paths in the container.
     * @param host       A {@link TestHost} instance to be tested.
     * @param flowCount  The number of flow entries currently installed.
     */
    protected void checkMapping(List<VBridgePath> bridges, TestHost host,
                                int flowCount) {
        checkMapping(bridges, host, flowCount, null, null);
    }

    /**
     * Ensure that incoming packet is handled according to the virtual mapping
     * configuration.
     *
     * @param bridges    A list of vBridge paths in the container.
     * @param host       A {@link TestHost} instance to be tested.
     * @param flowCount  The number of flow entries currently installed.
     * @param oldNw      A {@link PortVlan} instance which represents VLAN
     *                   network expected to be removed from broadcast domain.
     */
    protected void checkMapping(List<VBridgePath> bridges, TestHost host,
                                int flowCount, PortVlan oldNw) {
        checkMapping(bridges, host, flowCount, oldNw, null);
    }

    /**
     * Ensure that incoming packet is handled according to the virtual mapping
     * configuration.
     *
     * @param bridges    A list of vBridge paths in the container.
     * @param host       A {@link TestHost} instance to be tested.
     * @param flowCount  The number of flow entries currently installed.
     * @param oldNw      A {@link PortVlan} instance which represents VLAN
     *                   network expected to be removed from broadcast domain.
     * @param bcastMap   A map to store broadcast network.
     */
    private void checkMapping(List<VBridgePath> bridges, TestHost host,
                              int flowCount, PortVlan oldNw,
                              Map<TestHost, Set<PortVlan>> bcastMap) {
        // Send a broadcast ARP request which probes unknown host.
        byte[] unknownAddr = {(byte)192, (byte)168, (byte)200, (byte)254};
        MapReference ref = host.getMapping();
        Set<PortVlan> bcast;
        PacketResult pres;
        if (ref == null) {
            pres = PacketResult.IGNORED;
            bcast = null;
        } else {
            pres = PacketResult.KEEP_PROCESSING;
            bcast = getBroadcastNetwork(ref.getPath(), host);
            if (bcastMap != null) {
                bcastMap.put(host, bcast);
            }
        }

        RawPacket pkt = host.createArp(unknownAddr);
        assertEquals(pres, vtnMgr.receiveDataPacket(pkt));
        flushTasks();
        flushFlowTasks();

        if (oldNw != null) {
            // PACKET_IN handler should remove this VLAN network from
            // broadcast domain.
            assertTrue(bcast.remove(oldNw));
        }

        checkBroadcastPacket(pkt, bcast);
        for (VBridgePath bp: bridges) {
            host.checkLearned(vtnMgr, bp);
        }

        // This should never install new flow entry.
        stubObj.checkFlowCount(flowCount);
    }

    /**
     * Add the specified node connector.
     *
     * @param nc    A node connector corresponding to a physical switch port.
     * @param name  The name of the specified node connector.
     * @param up    The state of the specified node connector.
     */
    protected void addNodeConnector(NodeConnector nc, String name, boolean up) {
        HashMap<String, Property> props = new HashMap<String, Property>();
        if (name != null) {
            props.put(Name.NamePropName, new Name(name));
        }

        if (up) {
            props.put(Config.ConfigPropName, new Config(Config.ADMIN_UP));
            props.put(State.StatePropName, new State(State.EDGE_UP));
        } else {
            props.put(Config.ConfigPropName, new Config(Config.ADMIN_DOWN));
            props.put(State.StatePropName, new State(State.EDGE_DOWN));
        }

        Node node = nc.getNode();
        if (stubObj.addNode(node)) {
            vtnMgr.notifyNode(node, UpdateType.ADDED,
                              new HashMap<String, Property>());
        }

        if (stubObj.addNodeConnector(nc, props)) {
            vtnMgr.notifyNodeConnector(nc, UpdateType.ADDED, props);
        }
    }
}
