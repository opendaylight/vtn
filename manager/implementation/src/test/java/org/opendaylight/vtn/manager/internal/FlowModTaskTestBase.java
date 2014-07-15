/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Hashtable;

import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TimerTask;

import org.apache.felix.dm.impl.ComponentImpl;

import org.junit.Before;

import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.forwardingrulesmanager.FlowConfig;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.forwardingrulesmanager.IForwardingRulesManager;
import org.opendaylight.controller.forwardingrulesmanager.PortGroupConfig;
import org.opendaylight.controller.forwardingrulesmanager.PortGroupProvider;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResultEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * Common class for tests of {@link FlowModTask}.
 */
public class FlowModTaskTestBase extends TestUseVTNManagerBase {

    /**
     * Stub class of {@link IForwardingRulesManager}.
     */
    protected class ForwardingRulesManagerStub implements
            IForwardingRulesManager {
        /**
         * frmMode
         *  0 : installLocal() and uninstallLocal don't return.
         *  1 : installLocal() and uninstallLocal always return
         *      set code.
         */
        private int frmMode = 0;

        private StatusCode returnStatusCode = null;

        /**
         * constructor
         */
        ForwardingRulesManagerStub () {
        }

        /**
         * constructor
         */
        ForwardingRulesManagerStub(int mode, StatusCode code) {
            frmMode = mode;
            returnStatusCode = code;
        }

        @Override
        public void addOutputPort(Node arg0, String arg1,
                List<NodeConnector> arg2) {
        }

        @Override
        public boolean addPortGroupConfig(String arg0, String arg1, boolean arg2) {
            return false;
        }

        @Override
        public Status addStaticFlow(FlowConfig arg0) {
            return null;
        }

        @Override
        public Status addStaticFlowAsync(FlowConfig config) {
            return null;
        }

        @Override
        public boolean checkFlowEntryConflict(FlowEntry arg0) {
            return false;
        }

        @Override
        public boolean delPortGroupConfig(String arg0) {
            return false;
        }

        @Override
        public List<FlowEntry> getFlowEntriesForGroup(String arg0) {
            return null;
        }

        @Override
        public List<FlowEntry> getInstalledFlowEntriesForGroup(String arg0) {
            return null;
        }

        @Override
        public List<Node> getListNodeWithConfiguredFlows() {
            return null;
        }

        @Override
        public NodeConnector getOutputPort(Node arg0, String arg1) {
            return null;
        }

        @Override
        public Map<String, PortGroupConfig> getPortGroupConfigs() {
            return null;
        }

        @Override
        public PortGroupProvider getPortGroupProvider() {
            return null;
        }

        @Override
        public FlowConfig getStaticFlow(String arg0, Node arg1) {
            return null;
        }

        @Override
        public List<String> getStaticFlowNamesForNode(Node arg0) {
            return null;
        }

        @Override
        public List<FlowConfig> getStaticFlows() {
            return null;
        }

        @Override
        public List<FlowConfig> getStaticFlows(Node arg0) {
            return null;
        }

        @Override
        public Map<String, Object> getTSPolicyData() {
            return null;
        }

        @Override
        public Object getTSPolicyData(String arg0) {
            return null;
        }

        @Override
        public Status installFlowEntry(FlowEntry arg0) {
            if (frmMode == 1) {
                return new Status(returnStatusCode, null);
            }

            long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();
            try {
                Thread.sleep(timeout * 2);
            } catch (InterruptedException e) {
                return new Status(StatusCode.UNDEFINED);
            }
            return new Status(StatusCode.SUCCESS);
        }

        @Override
        public Status installFlowEntryAsync(FlowEntry arg0) {
            return null;
        }

        @Override
        public Status modifyFlowEntry(FlowEntry arg0, FlowEntry arg1) {
            return null;
        }

        @Override
        public Status modifyFlowEntryAsync(FlowEntry arg0, FlowEntry arg1) {
            return null;
        }

        @Override
        public Status modifyOrAddFlowEntry(FlowEntry arg0) {
            return null;
        }

        @Override
        public Status modifyOrAddFlowEntryAsync(FlowEntry arg0) {
            return null;
        }

        @Override
        public Status modifyStaticFlow(FlowConfig arg0) {
            return null;
        }

        @Override
        public void removeOutputPort(Node arg0, String arg1,
                List<NodeConnector> arg2) {
        }

        @Override
        public Status removeStaticFlow(FlowConfig arg0) {
            return null;
        }

        @Override
        public Status removeStaticFlowAsync(FlowConfig config) {
            return null;
        }

        @Override
        public Status removeStaticFlow(String arg0, Node arg1) {
            return null;
        }

        @Override
        public Status removeStaticFlowAsync(String name, Node node) {
            return null;
        }

        @Override
        public void replaceOutputPort(Node arg0, String arg1, NodeConnector arg2) {
        }

        @Override
        public Status saveConfig() {
            return null;
        }

        @Override
        public void setTSPolicyData(String arg0, Object arg1, boolean arg2) {
        }

        @Override
        public Status solicitStatusResponse(Node arg0, boolean arg1) {
            return null;
        }

        @Override
        public Status toggleStaticFlowStatus(FlowConfig arg0) {
            return null;
        }

        @Override
        public Status toggleStaticFlowStatus(String arg0, Node arg1) {
            return null;
        }

        @Override
        public Status uninstallFlowEntry(FlowEntry arg0) {
            if (frmMode == 1) {
                return new Status(returnStatusCode, null);
            }

            long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();
            try {
                Thread.sleep(timeout * 2);
            } catch (InterruptedException e) {
                return new Status(StatusCode.UNDEFINED);
            }
            return new Status(StatusCode.SUCCESS);
        }

        @Override
        public Status uninstallFlowEntryAsync(FlowEntry arg0) {
            return null;
        }

        @Override
        public Status uninstallFlowEntryGroup(String arg0) {
            return null;
        }

        @Override
        public Status uninstallFlowEntryGroupAsync(String arg0) {
            return null;
        }

        @Override
        public List<FlowEntry> getFlowEntriesForNode(Node node) {
            return null;
        }

        @Override
        public List<FlowEntry> getInstalledFlowEntriesForNode(Node node) {
            return null;
        }
    }

    /**
     * Stub of {@link VTNManagerImpl}.
     */
    protected class VTNManagerImplStub extends VTNManagerImpl {
        Set<ClusterEvent> stubEventSet = new HashSet<ClusterEvent>();

        // Override {@code postEvent()} for tests.
        @Override
        public void postEvent(ClusterEvent cev) {
           stubEventSet.add(cev);
        }

        public Set<ClusterEvent> getClusterEventSetStub() {
            return stubEventSet;
        }

        public void clearClusterEventSetStub() {
            stubEventSet.clear();
        }
    }

    protected Set<ClusterEvent> getPostedClusterEvent() {
        Set<ClusterEvent> sets
            = ((VTNManagerImplStub) vtnMgr).getClusterEventSetStub();
        return new HashSet<ClusterEvent>(sets);
    }

    protected void clearPostedClusterEvent() {
        ((VTNManagerImplStub) vtnMgr).clearClusterEventSetStub();
    }

    /**
     * A timer task used to emulate remote event.
     */
    protected class ResultTimerTask extends TimerTask {
        private VTNManagerImpl vtnManager = null;
        private FlowEntry flowEntry = null;
        private FlowModResult result = null;
        private boolean isLocal = false;

        public ResultTimerTask(VTNManagerImpl mgr, FlowEntry ent, FlowModResult res,
                               boolean local) {
            vtnManager = mgr;
            flowEntry = ent;
            result = res;
            isLocal = local;
        }

        @Override
        public void run() {
            FlowModResultEvent re
                = new FlowModResultEvent(flowEntry.getFlowName(), result);
            re.received(vtnManager, isLocal);
        }
    }

    /**
     * A timer task used to interrupt a thread.
     */
    protected class InterruptTask extends TimerTask {
        private Thread targetThread = null;

        InterruptTask(Thread th) {
            targetThread = th;
        }

        @Override
        public void run() {
            targetThread.interrupt();
        }
    }

    /**
     * Construct a new instance.
     */
    public FlowModTaskTestBase() {
        super(2);
    }

    @Before
    @Override
    public void before() {
        setupStartupDir();

        vtnMgr = new VTNManagerImplStub();
        resMgr = new GlobalResourceManager();
        ComponentImpl c = new ComponentImpl(null, null, null);
        stubObj = new TestStub(stubMode);
        TestStubCluster cm = new TestStubCluster(stubMode);

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
        vtnMgr.setConnectionManager(cm);
        vtnMgr.setContainerManager(stubObj);
        startVTNManager(c);
    }

    /**
     * set resource manager service
     *
     * @param cmp   A {@link ComponentImpl} Object.
     * @param cls   A {@link IClusterGlobalServices} service.
     */
    protected void setupResourceManager(ComponentImpl cmp,
                                        IClusterGlobalServices cls) {
        resMgr.setClusterGlobalService(cls);
        resMgr.init(cmp);
    }
}
