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

import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchField;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
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
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.TestUseVTNManagerBase.NopFlowTask;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

/**
 * Common class for tests of {@link VTNManagerImpl}.
 */
public class VTNManagerImplTestCommon extends TestUseVTNManagerBase {

    /**
     *  Mock-up of IfHostListener.
     */
    class HostListener implements IfHostListener {
        private int hostListenerCalled = 0;

        @Override
        public void hostListener(HostNodeConnector host) {
            hostListenerCalled++;
        }

        /**
         * get times hostListener called.
         * @return the number of times hostListener was called.
         */
        int getHostListenerCalled () {
            int ret = hostListenerCalled;
            hostListenerCalled = 0;
            return  ret;
        }
    }

    /**
     * Mock-up of IVTNModeListener.
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
         * check information registered by vtnModeChanged().
         * @param expCount  A expected number of times vtnModeChanged() was called.
         * @param expMode   A expected mode.
         */
        void checkCalledInfo(int expCount, Boolean expMode) {
            checkCalledInfo(expCount);
            assertEquals(expMode, this.getCalledArg());
        }

        /**
         * check information registered by vtnModeChanged().
         * @param expCount  A expected number of times vtnModeChanged() was called.
         */
        void checkCalledInfo(int expCount) {
            if (expCount >= 0) {
                assertEquals(expCount, this.getCalledCount(expCount));
            }
        }
    }

    /**
     * utility class used in VTNManagerAwareStub.
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
    };

    /**
     * Mock-up of IVTNManagerAware.
     */
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

        /**
         * check information notified by vtnChanged().
         *
         * @param count     A expected number of times vtnChanged() was called.
         * @param path      A VTenantPath expect to be notified.
         * @param name      A name expect to be notified.
         * @param type      A type expect to be notified.
         */
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

        /**
         * check information notified by vBridgeChanged().
         *
         * @param count     A expected number of times vBridgeChanged() was called.
         * @param path      A VBridgePath expect to be notified.
         * @param name      A name expect to be notified.
         * @param type      A type expect to be notified.
         */
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

        /**
         * check information notified by vBridgeInterfaceChanged().
         *
         * @param count     A expected number of times
         *                  vBridgeInterfaceChanged() was called.
         * @param path      A VBridgeIfPath expect to be notified.
         * @param name      A name expect to be notified.
         * @param type      A type expect to be notified.
         */
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

        /**
         * check information notified by vlanMapChanged().
         *
         * @param count     A expected number of times vlanMapChanged() was called.
         * @param path      A VBridgePath expect to be notified.
         * @param id        A map-id expect to be notified.
         * @param type      A type expect to be notified.
         */
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

        /**
         * check information notified by portMapChanged().
         *
         * @param count     A expected number of times portMapChanged() was called.
         * @param path      A VBridgeIfPath expect to be notified.
         * @param pconf     A PortMapConfig expect to be notified.
         * @param type      A type expect to be notified.
         */
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

        /**
         * check all methods not called.
         */
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


    /**
     * method for setup the environment.
     * create 1 Tenant and bridges
     */
    protected void createTenantAndBridge(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertEquals("(VBridgePath)" + bpath.toString(), StatusCode.SUCCESS, st.getCode());
        }
    }

    /**
     * method for setup the environment.
     * create 1 Tenant and bridges and vInterfaces
     */
    protected void createTenantAndBridgeAndInterface(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths, List<VBridgeIfPath> ifpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertEquals("(VBridgePath)" + bpath.toString(), StatusCode.SUCCESS, st.getCode());
        }

        for (VBridgeIfPath ifpath : ifpaths) {
            VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
            st = mgr.addBridgeInterface(ifpath, ifconf);
            assertEquals("(VBridgeIfPath)" + ifpath.toString(), StatusCode.SUCCESS, st.getCode());
        }
    }

    /**
     * check VTN configuration.
     * note: this don't support a configuration which VBridge &gt; 1
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

    protected void checkFlowEntries(PortVlan learnedPv, PortVlan inPortVlan,
            Set<FlowEntry> flowEntries, Set<String> registerdFlows,
            byte[] src, byte[] dst, VTenant tenant,
            String emsg) {
        NodeConnector learnedNc = learnedPv.getNodeConnector();
        Node learnedNode = learnedNc.getNode();
        Node inNode = inPortVlan.getNodeConnector().getNode();

        NodeConnector edgePort
                = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 15), inNode);
        for (FlowEntry ent : flowEntries) {
            if (!registerdFlows.add(ent.getFlowName())) {
                continue;
            }

            if (ent.getFlowName().endsWith("-0")) {
                assertEquals(inNode, ent.getNode());
                for (Action act : ent.getFlow().getActions()) {
                    switch (act.getType()) {
                    case OUTPUT:
                        Output out = (Output)act;
                        if (learnedNode.equals(inNode)) {
                            assertEquals(learnedNc, out.getPort());
                        } else {
                            assertEquals(emsg, edgePort, out.getPort());
                        }
                        break;
                    case SET_VLAN_ID:
                        if (learnedNode.equals(inNode)) {
                            SetVlanId setVlan = (SetVlanId)act;
                            assertEquals(setVlan.getVlanId(), learnedPv.getVlan());
                        } else {
                            fail("in this case SET_VLAN_ID is not set.");
                        }
                        break;
                    case POP_VLAN:
                        if (learnedNode.equals(inNode)) {
                            assertTrue(learnedPv.getVlan() <= 0);
                        } else {
                            fail("in this case POP_VLAN is not set.");
                        }
                        break;
                    default :
                        fail("unexpected action type." + act.toString());
                        break;
                    }
                }
            } else if (ent.getFlowName().endsWith("-1")) {
                assertEquals(learnedNode, ent.getNode());
                for (Action act : ent.getFlow().getActions()) {
                    switch (act.getType()) {
                    case OUTPUT:
                        Output out = (Output)act;
                        assertEquals(learnedNc, out.getPort());
                        break;
                    case SET_VLAN_ID:
                        SetVlanId setVlan = (SetVlanId)act;
                        assertEquals(setVlan.getVlanId(), learnedPv.getVlan());
                        break;
                    case POP_VLAN:
                        assertTrue(learnedPv.getVlan() <= 0);
                        break;
                    default :
                        fail("unexpected action type." + act.toString());
                        break;
                    }
                }
            } else {
                fail("not supported case.");
            }

            NodeConnector inEdgePort
                = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 15), learnedNode);

            Match match = ent.getFlow().getMatch();
            for (MatchType mtype : match.getMatchesList()) {
                MatchField field = match.getField(mtype);
                switch (mtype) {
                case DL_SRC:
                    assertArrayEquals(src, (byte[]) field.getValue());
                    break;
                case DL_DST:
                    assertArrayEquals(dst, (byte[]) field.getValue());
                    break;
                case DL_VLAN:
                    assertEquals(inPortVlan.getVlan(),
                                 field.getValue());
                    break;
                case IN_PORT:
                    if (ent.getFlowName().endsWith("-0")) {
                        assertEquals(inPortVlan.getNodeConnector(),
                                 (NodeConnector) field.getValue());
                    } else {
                        assertEquals(inEdgePort,
                                (NodeConnector) field.getValue());
                    }
                    break;
                default:
                    fail("unexpected match type." + match.toString());
                    break;
                }
            }

            int pri = vtnMgr.getVTNConfig().getL2FlowPriority();
            assertEquals(pri, ent.getFlow().getPriority());
            if (ent.getFlowName().endsWith("-0")) {
                assertEquals(tenant.getHardTimeout(), ent.getFlow().getHardTimeout());
                assertEquals(tenant.getIdleTimeout(), ent.getFlow().getIdleTimeout());
            } else {
                assertEquals(0, ent.getFlow().getHardTimeout());
                assertEquals(0, ent.getFlow().getIdleTimeout());
            }
        }
    }

    /**
     * Expire flow entries.
     *
     * @param mgr    VTN Manager service.
     * @param stub   Stub for OSGi service.
     */
    protected void expireFlows(VTNManagerImpl mgr, TestStub stub) {
        // Wait for all flow modifications to complete.
        NopFlowTask task = new NopFlowTask(mgr);
        mgr.postFlowTask(task);
        assertSame(FlowModResult.SUCCEEDED, task.getResult(3000));

        Set<FlowEntry> flows = stub.getFlowEntries();
        if (!flows.isEmpty()) {
            for (FlowEntry fent : flows) {
                String flowName = fent.getFlowName();
                if (flowName.endsWith("-0")) {
                    Status status = stub.uninstallFlowEntry(fent);
                    assertEquals("(FlowEntry)" + fent.toString(), StatusCode.SUCCESS,
                            status.getCode());
                    mgr.flowRemoved(fent.getNode(), fent.getFlow());
                }
            }

            // Wait for all flow entries to be removed.
            task = new NopFlowTask(mgr);
            mgr.postFlowTask(task);
            assertSame(FlowModResult.SUCCEEDED, task.getResult(3000));
            assertSame(0, stub.getFlowEntries().size());
        }
    }

}
