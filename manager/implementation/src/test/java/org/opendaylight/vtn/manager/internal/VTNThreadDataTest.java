/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.junit.Test;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * JUnit Test for {@link VTNThreadData}.
 */
public class VTNThreadDataTest extends TestUseVTNManagerBase {
    /**
     * Construct a new instance.
     */
    public VTNThreadDataTest() {
        super(2);
    }

    /**
     * Test method for {@link VTNThreadData#create(Lock)}.
     */
    @Test
    public void testCreateAndRemove() {
        // create tenant and bridges
        VTenantPath tpath = new VTenantPath("tenant");
        VTenantPath tpath_dummy = new VTenantPath("dummytenant");
        VBridgePath bpath1 = new VBridgePath(tpath.getTenantName(), "bridge1");
        VBridgePath bpath2 = new VBridgePath(tpath.getTenantName(), "bridge2");

        Status st = vtnMgr.addTenant(tpath, new VTenantConfig("desc"));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = vtnMgr.addBridge(bpath1, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = vtnMgr.addBridge(bpath2, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // add flow entries.
        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Set<Short> portIds0 = new HashSet<Short>(createShorts((short) 10, (short) 5,
                                                              false));
        Set<NodeConnector> ncset0
            = NodeConnectorCreator.createOFNodeConnectorSet(portIds0, node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 15),
                                                         node0);
        int pri = 1;
        Set<VTNFlow> flows = new HashSet<VTNFlow>();
        Set<VTenantPath> pathSet = new HashSet<VTenantPath>();

        int numEntries1 = 0;
        int numEntries2 = 0;
        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(tpath.getTenantName());
        for (NodeConnector innc : ncset0) {
            VTNFlow flow = fdb.create(vtnMgr);

            Match match = new Match();
            match.setField(MatchType.IN_PORT, innc);
            ActionList actions = new ActionList(innc.getNode());
            actions.addOutput(outnc);
            flow.addFlow(vtnMgr, match, actions, pri);

            if (((numEntries1 + numEntries2) % 2) == 0) {
                pathSet.add(bpath1);
                numEntries1++;
            } else {
                pathSet.add(bpath2);
                numEntries2++;
            }
            flow.addDependency(pathSet);

            fdb.install(vtnMgr, flow);
            flushFlowTasks();
            flows.add(flow);

            pathSet.clear();
        }

        ConcurrentMap<FlowGroupId, VTNFlow> db = vtnMgr.getFlowDB();
        assertEquals(numEntries1 + numEntries2, db.size());
        assertEquals(numEntries1 + numEntries2, stubObj.getFlowEntries().size());

        ReentrantReadWriteLock  rwLock = new ReentrantReadWriteLock();
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());

        VTNThreadData.removeFlows(vtnMgr, tpath_dummy);
        data.cleanUp(vtnMgr);
        assertEquals(numEntries1 + numEntries2, db.size());
        assertEquals(numEntries1 + numEntries2, stubObj.getFlowEntries().size());

        // remove relevant to specified vBridge.
        data = VTNThreadData.create(rwLock.writeLock());
        VTNThreadData.removeFlows(vtnMgr, bpath2);
        data.cleanUp(vtnMgr);
        assertEquals(numEntries1, db.size());
        assertEquals(numEntries1, stubObj.getFlowEntries().size());

        // execute again.
        data = VTNThreadData.create(rwLock.writeLock());
        VTNThreadData.removeFlows(vtnMgr, bpath2);
        data.cleanUp(vtnMgr);
        assertEquals(numEntries1, db.size());
        assertEquals(numEntries1, stubObj.getFlowEntries().size());

        // remove all entry.
        data = VTNThreadData.create(rwLock.writeLock());
        VTNThreadData.removeFlows(vtnMgr, fdb);
        data.cleanUp(vtnMgr);
        assertEquals(0, db.size());
        assertEquals(0, stubObj.getFlowEntries().size());

        data = VTNThreadData.create(rwLock.writeLock());
        VTNThreadData.removeFlows(vtnMgr, fdb);
        data.cleanUp(vtnMgr);
        assertEquals(0, db.size());
        assertEquals(0, stubObj.getFlowEntries().size());

        // try to get lock.
        VTNThreadData.create(rwLock.writeLock());
        data.cleanUp(vtnMgr);

        st = vtnMgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for {@link VTNThreadData#setModeChanged()}.
     */
    @Test
    public void testSetModeChanged() {

        VTNModeListenerStub stub = new VTNModeListenerStub();
        vtnMgr.addVTNModeListener(stub);
        stub.checkCalledInfo(1);

        // create tenant and bridges
        VTenantPath tpath = new VTenantPath("tenant");
        Status st = vtnMgr.addTenant(tpath, new VTenantConfig("desc"));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub.checkCalledInfo(1);

        ReentrantReadWriteLock  rwLock = new ReentrantReadWriteLock();
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());

        // setModeChanged() and cleanup() are called in removeTenant().
        st = vtnMgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub.checkCalledInfo(1);

        data = VTNThreadData.create(rwLock.writeLock());
        data.cleanUp(vtnMgr);
        stub.checkCalledInfo(0);

        vtnMgr.removeVTNModeListener(stub);
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

        /**
         * check information registered by {@code vtnModeChanged()}.
         * @param expCount  A expected number of times {@code vtnModeChanged()}
         *                  was called.
         * @param expMode   A expected mode.
         */
        void checkCalledInfo(int expCount, Boolean expMode) {
            checkCalledInfo(expCount);
            assertEquals(expMode, this.getCalledArg());
        }

        /**
         * check information registered by {@code vtnModeChanged()}.
         * @param expCount  A expected number of times {@code vtnModeChanged()}
         *                  was called.
         */
        void checkCalledInfo(int expCount) {
            if (expCount >= 0) {
                assertEquals(expCount, this.getCalledCount(expCount));
            }
        }
    }

}
