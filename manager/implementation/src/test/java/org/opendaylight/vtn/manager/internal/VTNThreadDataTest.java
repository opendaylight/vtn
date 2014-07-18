/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.junit.Test;

import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

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
     * Test method for {@link VTNThreadData#create(java.util.concurrent.locks.Lock)}.
     */
    @Test
    public void testCreateAndRemove() {
        // create tenant and bridges
        VTenantPath tpath = new VTenantPath("tenant");
        VTenantPath tpathDummy = new VTenantPath("dummytenant");
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
        Set<Short> portIds0 = new HashSet<Short>(createShorts((short)10,
                                                              (short)5, false));
        Set<NodeConnector> ncset0 = NodeConnectorCreator.
            createOFNodeConnectorSet(portIds0, node0);
        NodeConnector outnc = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)15), node0);
        int pri = 1;
        Set<VTNFlow> flows = new HashSet<VTNFlow>();
        Set<VTenantPath> pathSet = new HashSet<VTenantPath>();
        Set<String> group1Set = new HashSet<String>();
        Set<String> group2Set = new HashSet<String>();

        int numEntries1 = 0;
        int numEntries2 = 0;
        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(tpath.getTenantName());
        byte[] src = {
            (byte)0x00, (byte)0x11, (byte)0x22,
            (byte)0x33, (byte)0x44, (byte)0x55,
        };
        byte[] dst = {
            (byte)0xf0, (byte)0xfa, (byte)0xfb,
            (byte)0xfc, (byte)0xfd, (byte)0xfe,
        };
        short vlan = 0;
        for (NodeConnector innc : ncset0) {
            VTNFlow flow = fdb.create(vtnMgr);

            Match match = new Match();
            match.setField(MatchType.IN_PORT, innc);
            match.setField(MatchType.DL_SRC, src);
            match.setField(MatchType.DL_DST, dst);
            match.setField(MatchType.DL_VLAN, vlan);
            ActionList actions = new ActionList(innc.getNode());
            actions.addOutput(outnc);
            flow.addFlow(vtnMgr, match, actions, pri);

            Set<String> groupSet;
            if (((numEntries1 + numEntries2) % 2) == 0) {
                pathSet.add(bpath1);
                numEntries1++;
                groupSet = group1Set;
            } else {
                pathSet.add(bpath2);
                numEntries2++;
                groupSet = group2Set;
            }
            flow.addDependency(pathSet);
            assertTrue(groupSet.add(flow.getGroupId().toString()));

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

        VTNThreadData.removeFlows(vtnMgr, tpathDummy);
        data.cleanUp(vtnMgr);
        assertEquals(numEntries1 + numEntries2, db.size());
        assertEquals(numEntries1 + numEntries2, stubObj.getFlowEntries().size());

        // remove relevant to specified vBridge.
        data = VTNThreadData.create(rwLock.writeLock());
        VTNThreadData.removeFlows(vtnMgr, bpath2);
        data.cleanUp(vtnMgr);
        assertEquals(numEntries1, db.size());
        Set<FlowEntry> entries = stubObj.getFlowEntries();
        assertEquals(numEntries1, entries.size());
        for (FlowEntry f: entries) {
            String g = f.getGroupName();
            assertTrue(f.toString(), group1Set.contains(g));
            assertFalse(f.toString(), group2Set.contains(g));
        }

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
     * Test case for {@link VTNThreadData#removeFlows(VTNManagerImpl, String, MacVlan, NodeConnector)}.
     */
    @Test
    public void testRemoveFlowsByHost() {
        // Create tenant and bridge.
        String tname = "tenant";
        VTenantPath tpath = new VTenantPath(tname);
        VBridgePath bpath = new VBridgePath(tname, "bridge");

        Status st = vtnMgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = vtnMgr.addBridge(bpath, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Collect available ports.
        Set<NodeConnector> connectors = new HashSet<NodeConnector>();
        Map<Node, NodeConnector> untestedPorts =
            new HashMap<Node, NodeConnector>();
        NodeConnector unusedPort = null;
        Set<Node> nodes = stubObj.getNodes();
        for (Node node: nodes) {
            for (NodeConnector port: stubObj.getNodeConnectors(node)) {
                if (stubObj.isSpecial(port) || stubObj.isInternal(port)) {
                    unusedPort = port;
                } else if (untestedPorts.containsKey(node)) {
                    assertTrue(connectors.add(port));
                } else {
                    untestedPorts.put(node, port);
                }
            }
        }

        assertNotNull(unusedPort);
        assertEquals(nodes.size(), untestedPorts.size());
        assertTrue(connectors.size() > 1);

        // Install flow entries.
        Map<ObjectPair<MacVlan, NodeConnector>, Set<String>> flows =
            new HashMap<ObjectPair<MacVlan, NodeConnector>, Set<String>>();
        Map<ObjectPair<MacVlan, NodeConnector>, Integer> flowSizes =
            new HashMap<ObjectPair<MacVlan, NodeConnector>, Integer>();
        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(tname);
        assertNotNull(fdb);

        long mac = 1L;
        short vlan = 0;
        long untestedMac = 0x00ffffffffffL;
        short untestedVlan = 0;
        MacVlan untested = new MacVlan(untestedMac, untestedVlan);
        int nflows = 0;

        for (NodeConnector port: connectors) {
            Set<String> groupSet = new HashSet<String>();

            MacVlan mvlan = new MacVlan(mac, vlan);
            ObjectPair<MacVlan, NodeConnector> pair =
                new ObjectPair<MacVlan, NodeConnector>(mvlan, port);

            // Install a flow entry that the source host matches.
            NodeConnector untestedPort = untestedPorts.get(port.getNode());
            VTNFlow vflow = createVTNFlow(fdb, mvlan, port, untested,
                                          untestedPort);
            assertTrue(groupSet.add(vflow.getGroupId().toString()));
            fdb.install(vtnMgr, vflow);
            int fsize1 = vflow.getFlowEntries().size();
            nflows += fsize1;

            // Install a flow entry that the destination host matches.
            vflow = createVTNFlow(fdb, untested, untestedPort, mvlan, port);
            assertTrue(groupSet.add(vflow.getGroupId().toString()));
            fdb.install(vtnMgr, vflow);
            int fsize2 = vflow.getFlowEntries().size();
            nflows += fsize2;

            assertNull(flowSizes.put(pair, fsize1 + fsize2));
            assertNull(flows.put(pair, groupSet));
            mac++;
            vlan++;
        }
        flushFlowTasks();

        ConcurrentMap<FlowGroupId, VTNFlow> db = vtnMgr.getFlowDB();
        assertEquals(nflows, db.size());
        assertEquals(nflows, stubObj.getFlowEntries().size());

        // Try to remove flow entries with specifying unused host.
        MacVlan unused = new MacVlan(untestedMac - 1L, untestedVlan);
        ReentrantReadWriteLock  rwLock = new ReentrantReadWriteLock();
        for (NodeConnector port: connectors) {
            VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
            try {
                VTNThreadData.removeFlows(vtnMgr, tname, unused, port);
            } finally {
                data.cleanUp(vtnMgr);
            }
            assertEquals(nflows, db.size());
            assertEquals(nflows, stubObj.getFlowEntries().size());
        }

        // Try to remove flow entries with specifying unused port.
        for (ObjectPair<MacVlan, NodeConnector> pair: flows.keySet()) {
            MacVlan host = pair.getLeft();
            VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
            try {
                VTNThreadData.removeFlows(vtnMgr, tname, host, unusedPort);
            } finally {
                data.cleanUp(vtnMgr);
            }
            assertEquals(nflows, db.size());
            assertEquals(nflows, stubObj.getFlowEntries().size());
        }

        // Remove flow entries by a pair of host and port.
        for (Map.Entry<ObjectPair<MacVlan, NodeConnector>, Set<String>> entry:
                 flows.entrySet()) {
            ObjectPair<MacVlan, NodeConnector> pair = entry.getKey();
            Set<String> groupSet = entry.getValue();

            VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
            try {
                VTNThreadData.removeFlows(vtnMgr, tname, pair.getLeft(),
                                          pair.getRight());
            } finally {
                data.cleanUp(vtnMgr);
            }

            int fsize = flowSizes.get(pair);
            nflows -= fsize;
            assertEquals(nflows, db.size());
            Set<FlowEntry> entries = stubObj.getFlowEntries();
            assertEquals(nflows, entries.size());
            for (FlowEntry f: entries) {
                String g = f.getGroupName();
                assertFalse(groupSet.contains(g));
            }
        }

        assertEquals(0, nflows);

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
        stub.checkCalledInfo(1, Boolean.FALSE);

        // create tenant and bridges
        VTenantPath tpath = new VTenantPath("tenant");
        Status st = vtnMgr.addTenant(tpath, new VTenantConfig("desc"));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub.checkCalledInfo(1, Boolean.TRUE);

        ReentrantReadWriteLock  rwLock = new ReentrantReadWriteLock();
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());

        // setModeChanged() and cleanup() are called in removeTenant().
        st = vtnMgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        stub.checkCalledInfo(1, Boolean.FALSE);

        data = VTNThreadData.create(rwLock.writeLock());
        data.cleanUp(vtnMgr);
        stub.checkCalledInfo(0);

        vtnMgr.removeVTNModeListener(stub);
    }

    /**
     * Create a VTN flow.
     *
     * @param fdb  A {@link VTNFlowDatabase} instance.
     * @param src  A {@link MacVlan} instance which represents the source host.
     * @param in   A {@link NodeConnector} instance which represents the
     *             incoming port.
     * @param dst  A {@link MacVlan} instance which represents the destination
     *             host.
     * @param out  A {@link NodeConnector} instance which represents the
     *             outgoing port.
     * @return  A {@link VTNFlow} instance.
     */
    private VTNFlow createVTNFlow(VTNFlowDatabase fdb, MacVlan src,
                                  NodeConnector in, MacVlan dst,
                                  NodeConnector out) {
        VTNFlow vflow = fdb.create(vtnMgr);

        Match match = new Match();
        byte[] saddr = NetUtils.longToByteArray6(src.getMacAddress());
        byte[] daddr = NetUtils.longToByteArray6(dst.getMacAddress());
        match.setField(MatchType.IN_PORT, in);
        match.setField(MatchType.DL_SRC, saddr);
        match.setField(MatchType.DL_DST, daddr);
        match.setField(MatchType.DL_VLAN, src.getVlan());

        ActionList actions = new ActionList(out.getNode());
        actions.addOutput(out);
        actions.addVlanId(dst.getVlan());
        vflow.addFlow(vtnMgr, match, actions, 10);

        return vflow;
    }

    /**
     * Mock-up of {@link IVTNModeListener}.
     */
    class VTNModeListenerStub implements IVTNModeListener {
        private int calledCount = 0;
        private Boolean oldactive = null;

        /**
         * The number of milliseconds to wait for VTN events.
         */
        private static final long  EVENT_TIMEOUT = 10000L;

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
