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
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.After;
import org.junit.Before;

import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.topology.TopoEdgeUpdate;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;

/**
 * Common class for tests of {@link VTNManagerImpl}.
 */
public class VTNManagerImplTestCommon extends TestBase {
    protected VTNManagerImpl vtnMgr = null;
    protected GlobalResourceManager resMgr;
    protected TestStub stubObj = null;
    protected static int stubMode = 0;

    class HostListener implements IfHostListener {
        private int hostListenerCalled = 0;

        @Override
        public void hostListener(HostNodeConnector host) {
            hostListenerCalled++;
        }

        int getHostListenerCalled () {
            int ret = hostListenerCalled;
            hostListenerCalled = 0;
            return  ret;
        }
    }

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
                long milli = 1000;
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
                long milli = 1000;
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

        void checkCalledInfo(int expCount, Boolean expMode) {
            checkCalledInfo(expCount);
            assertEquals(expMode, this.getCalledArg());
        }

        void checkCalledInfo(int expCount) {
            if (expCount >= 0) {
                assertEquals(expCount, this.getCalledCount(expCount));
            }
        }
    }

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
    };

    class VTNManagerAwareStub implements IVTNManagerAware {
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
        public synchronized void vtnChanged(VTenantPath path, VTenant vtenant, UpdateType type) {
            vtnChangedCalled++;
            vtnChangedInfo = new VTNManagerAwareData<VTenantPath, VTenant>(path, vtenant,
                    type, vtnChangedCalled);
        }

        @Override
        public synchronized void vBridgeChanged(VBridgePath path, VBridge vbridge, UpdateType type) {
            vbrChangedCalled++;
            vbrChangedInfo = new VTNManagerAwareData<VBridgePath, VBridge>(path, vbridge, type,
                    vbrChangedCalled);
        }

        @Override
        public synchronized void vBridgeInterfaceChanged(VBridgeIfPath path, VInterface viface, UpdateType type) {
            vIfChangedCalled++;
            vIfChangedInfo = new VTNManagerAwareData<VBridgeIfPath, VInterface>(path, viface, type,
                    vIfChangedCalled);
        }

        @Override
        public synchronized void vlanMapChanged(VBridgePath path, VlanMap vlmap, UpdateType type) {
            vlanMapChangedCalled++;
            vlanMapChangedInfo = new VTNManagerAwareData<VBridgePath, VlanMap>(path, vlmap, type,
                    vlanMapChangedCalled);
        }

        @Override
        public synchronized void portMapChanged(VBridgeIfPath path, PortMap pmap, UpdateType type) {
            portMapChangedCalled++;
            portMapChangedInfo = new VTNManagerAwareData<VBridgeIfPath, PortMap>(path, pmap, type,
                    portMapChangedCalled);
        }

        synchronized void checkVtnInfo(int count, VTenantPath path,
                                       String name, UpdateType type) {
            if (vtnChangedCalled < count) {
                long milli = 1000;
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
            }
            if (name != null) {
                assertEquals(name, vtnChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vtnChangedInfo.type);
            }
            vtnChangedCalled = 0;
            vtnChangedInfo = null;
        }

        synchronized void checkVbrInfo(int count, VBridgePath path,
                                       String name, UpdateType type) {
            if (vbrChangedCalled < count) {
                long milli = 1000;
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
            }
            if (name != null) {
                assertEquals(name, vbrChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vbrChangedInfo.type);
            }
            vbrChangedCalled = 0;
            vbrChangedInfo = null;
        }

        synchronized void checkVIfInfo(int count, VBridgeIfPath path,
                                       String name, UpdateType type) {
            if (vIfChangedCalled < count) {
                long milli = 1000;
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
            }
            if (name != null) {
                assertEquals(name, vIfChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vIfChangedInfo.type);
            }
            vIfChangedCalled = 0;
            vIfChangedInfo = null;
        }

        synchronized void checkVlmapInfo(int count, VBridgePath path,
                                         String id, UpdateType type) {
            if (vlanMapChangedCalled < count) {
                long milli = 1000;
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

        synchronized void checkPmapInfo(int count, VBridgeIfPath path,
                                        PortMapConfig pconf, UpdateType type) {
            if (portMapChangedCalled < count) {
                long milli = 1000;
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

        void checkAllNull() {
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
    };


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
     * method for setup enviroment.
     * create 1 Tenant and bridges
     */
    protected void createTenantAndBridge(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertTrue(st.isSuccess());
        }
    }

    /**
     * method for setup enviroment.
     * create 1 Tenant and bridges and vinterfaces
     */
    protected void createTenantAndBridgeAndInterface(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths, List<VBridgeIfPath> ifpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertTrue(st.isSuccess());
        }

        for (VBridgeIfPath ifpath : ifpaths) {
            VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
            st = mgr.addBridgeInterface(ifpath, ifconf);
            assertTrue(st.isSuccess());
        }
    }

    /**
     * check VTN configuraion.
     * note: this don't support a configuration which VBridge > 1
     */
    protected void checkVTNconfig(VTenantPath tpath, List<VBridgePath> bpathlist,
            Map<VBridgeIfPath, PortMapConfig> pmaps, Map<VlanMap, VlanMapConfig> vmaps) {
        VBridgePath bpath = bpathlist.get(0);

        List<VTenant> tlist = null;
        List<VBridge> blist = null;
        List<VInterface> iflist = null;
        try {
            tlist = vtnMgr.getTenants();
            blist = vtnMgr.getBridges(tpath);
            iflist = vtnMgr.getBridgeInterfaces(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }

        assertNotNull(tlist);
        assertNotNull(blist);
        assertEquals(1, tlist.size());
        assertEquals(tpath.getTenantName(), tlist.get(0).getName());
        assertEquals(1, blist.size());
        assertEquals(bpath.getBridgeName(), blist.get(0).getName());
        assertEquals(pmaps.size(), iflist.size());

        for (VInterface vif : iflist) {
            VBridgeIfPath bif = new VBridgeIfPath(tpath.getTenantName(), bpath.getBridgeName(), vif.getName());
            assertTrue(bif.toString(), pmaps.containsKey(bif));
        }

        if (pmaps != null) {
            for (Map.Entry<VBridgeIfPath, PortMapConfig> ent : pmaps.entrySet()) {
                PortMap pmap = null;
                try {
                    pmap = vtnMgr.getPortMap(ent.getKey());
                } catch (VTNException e) {
                    unexpected(e);
                }
                if (ent.getValue() != null) {
                    assertEquals(ent.getValue(), pmap.getConfig());
                }
            }
        }

        if (vmaps != null) {
            for (Map.Entry<VlanMap, VlanMapConfig> ent : vmaps.entrySet()) {
                VlanMap vmap = null;
                try {
                    vmap = vtnMgr.getVlanMap(bpath, ent.getKey().getId());
                } catch (VTNException e) {
                    unexpected(e);
                }
                if (ent.getValue() != null) {
                    assertEquals(ent.getValue().getVlan(), vmap.getVlan());
                    assertEquals(ent.getValue().getNode(), vmap.getNode());
                }
            }
        }
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
}
