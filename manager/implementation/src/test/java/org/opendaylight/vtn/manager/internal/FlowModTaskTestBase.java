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

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.connectionmanager.ConnectionMgmtScheme;
import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowConfig;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.forwardingrulesmanager.IForwardingRulesManager;
import org.opendaylight.controller.forwardingrulesmanager.PortGroupConfig;
import org.opendaylight.controller.forwardingrulesmanager.PortGroupProvider;
import org.opendaylight.controller.sal.connection.ConnectionConstants;
import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.flowprogrammer.IFlowProgrammerListener;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.internal.VTNManagerImplTestCommon.NopFlowTask;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResultEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Common class for tests of FlowModTask.
 */
public class FlowModTaskTestBase extends TestBase {
    protected VTNManagerImpl vtnMgr = null;
    protected GlobalResourceManager resMgr;
    protected TestStub stubObj = null;
    protected static int stubMode = 0;
    protected static int clusterMode = 0;

    protected class ConnectionManagerStub implements IConnectionManager {

        @Override
        public Node connect(String arg0, Map<ConnectionConstants, String> arg1) {
            return null;
        }

        @Override
        public Node connect(String arg0, String arg1,
                Map<ConnectionConstants, String> arg2) {
            return null;
        }

        @Override
        public Status disconnect(Node arg0) {
            return null;
        }

        @Override
        public Set<InetAddress> getControllers(Node node) {
            return null;
        }

        @Override
        public ConnectionMgmtScheme getActiveScheme() {
            return null;
        }

        @Override
        public Set<Node> getLocalNodes() {
            return null;
        }

        @Override
        public ConnectionLocality getLocalityStatus(Node arg0) {
            if (arg0.getID().equals(Long.valueOf("0"))) {
                return ConnectionLocality.LOCAL;
            } else if (arg0.getID().equals(Long.valueOf("1"))) {
                return ConnectionLocality.NOT_LOCAL;
            } else {
                return ConnectionLocality.NOT_CONNECTED;
            }
        }

        @Override
        public Set<Node> getNodes(InetAddress arg0) {
            return null;
        }

        @Override
        public boolean isLocal(Node arg0) {
            return false;
        }
    }

    protected class ForwardingRulesManagerStub implements
            IForwardingRulesManager {
        /**
         * frmMode
         *  0 : installLocal() and uninstallLocal don't return.
         *  1 : installLocal() and uninstallLocal always return
         *      set code.
         *
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
        public Status removeStaticFlow(String arg0, Node arg1) {
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
     * Stub of VTNManagerImpl.
     */
    protected class VTNManagerImplStub extends VTNManagerImpl {
        Set<ClusterEvent> eventSet = new HashSet<ClusterEvent>();

        // Override postEvent() for tests.
        @Override
        public void postEvent(ClusterEvent cev) {
           eventSet.add(cev);
        }

        public Set<ClusterEvent> getClusterEventSetStub() {
            return eventSet;
        }

        public void clearClusterEventSetStub() {
            eventSet.clear();
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


    @BeforeClass
    static public void beforeClass() {
        stubMode = 2;
    }

    @Before
    public void before() {
        setupStartupDir();

        vtnMgr = new VTNManagerImplStub();
        resMgr = new GlobalResourceManager();
        ComponentImpl c = new ComponentImpl(null, null, null);
        stubObj = new TestStub(stubMode);
        ConnectionManagerStub cm = new ConnectionManagerStub();

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
     *
     */
    protected void setupResourceManager(ComponentImpl cmp,
                                        IClusterGlobalServices cls) {
        resMgr.setClusterGlobalService(cls);
        resMgr.init(cmp);
    }

    /**
     * Flush all pending tasks on the VTN flow thread.
     */
    protected void flushFlowTasks() {
        NopFlowTask task = new NopFlowTask(vtnMgr);
        vtnMgr.postFlowTask(task);
        assertTrue(task.await(10L, TimeUnit.SECONDS));
    }

    /**
     * Flush all pending tasks on the VTN flow thread.
     */
    protected void flushFlowTasks(long wait) {
        NopFlowTask task = new NopFlowTask(vtnMgr);
        vtnMgr.postFlowTask(task);
        assertTrue(task.await(wait, TimeUnit.SECONDS));
    }

    /**
     *  A dummy flow task to flush pending tasks.
     */
    protected class NopFlowTask extends FlowModTask {
        /**
         * A latch to wait for completion.
         */
        private final CountDownLatch  latch = new CountDownLatch(1);

        protected NopFlowTask(VTNManagerImpl mgr) {
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
        private boolean await(long timeout, TimeUnit unit) {
            try {
                return latch.await(timeout, unit);
            } catch (InterruptedException e) {
                return false;
            }
        }
    }

    /**
     * Add FlowEntry to VTNFlow.
     *
     * @param   flow        A {@link VTNFlow}.
     * @param   inPort      A ingress {@link NodeConector}.
     * @param   inVlan      A incoming VLAN ID.
     * @param   outPort     A outgoing {@link NodeConnector}.
     * @param   priority    A priority of FlowEntry.
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

}
