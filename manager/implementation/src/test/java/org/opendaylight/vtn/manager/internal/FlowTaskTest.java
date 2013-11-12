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

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentMap;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.Test;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * test for {@link FlowAddTaskTest} and {@link FlowRemoveTaskTest}.
 */
public class FlowTaskTest extends FlowModTaskTestBase {

    /**
     * Test method for
     * {@link FlowAddTask#execute()},
     * {@link FlowRemoveTask#execute()},
     * {@link FlowModTask#run()}.
     */
    @Test
    public void testFlowTaskTest() {
        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();
        long remoteTimeout = vtnMgr.getVTNConfig().getRemoteFlowModTimeout();
        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));
        Node node2 = NodeCreator.createOFNode(Long.valueOf(2L));

        // in case ingress only.
        NodeConnector inncIngress
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node0);
        NodeConnector outncIngress
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        int pri = 1;

        flow = addFlowEntry(vtnMgr, flow, inncIngress, (short) 1,
                            outncIngress, pri);
        FlowAddTask task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 1, flow, flow, 1);

        Iterator<FlowEntry> it = flow.getFlowEntries().iterator();
        FlowEntry ingress = it.next();
        FlowRemoveTask rtask = new FlowRemoveTask(vtnMgr, flow.getGroupId(),
                                                  ingress, it);
        rtask.run();
        assertEquals(FlowModResult.SUCCEEDED, rtask.getResult());
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        task.run();
        assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 1, flow, flow, 1);

        fdb.createIndex(vtnMgr, flow);
        fdb.clear(vtnMgr);
        flushFlowTasks(remoteTimeout);
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // in case ingress + local.
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);
        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 1, flow, flow, 2);

        it = flow.getFlowEntries().iterator();
        ingress = it.next();
        rtask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        rtask.run();
        assertEquals(FlowModResult.SUCCEEDED, rtask.getResult());
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        task.run();
        assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 1, flow, flow, 2);

        // in case uninstallLocal() fails.
        ForwardingRulesManagerStub stub
            = new ForwardingRulesManagerStub(1, StatusCode.BADREQUEST);
        vtnMgr.setForwardingRuleManager(stub);
        it = flow.getFlowEntries().iterator();
        ingress = it.next();
        rtask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        rtask.run();
        assertEquals(FlowModResult.FAILED, rtask.getResult());

        vtnMgr.setForwardingRuleManager(stubObj);
        fdb.createIndex(vtnMgr, flow);
        fdb.clear(vtnMgr);
        flushFlowTasks(remoteTimeout);
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // in case ingress + local + remote.
        // in this case failed to add remote flow entry.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node1);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node1);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);
        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        it = flow.getFlowEntries().iterator();
        ingress = it.next();
        rtask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        rtask.run();
        assertEquals(FlowModResult.FAILED, rtask.getResult());
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        fdb.clear(vtnMgr);
        flushFlowTasks(remoteTimeout);
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // ingress + local + remote + not connected node.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node2);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node2);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);
        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        it = flow.getFlowEntries().iterator();
        ingress = it.next();
        rtask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        rtask.run();
        assertEquals(FlowModResult.FAILED, rtask.getResult());
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        fdb.clear(vtnMgr);
        flushFlowTasks(remoteTimeout);
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // ingress + remote
        // in this case fail to add remote flow entry.
        VTNFlow flowRemote = fdb.create(vtnMgr);
        flowRemote = addFlowEntry(vtnMgr, flowRemote, inncIngress, (short) 1,
                                  outncIngress, pri);

        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node1);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node1);
        flowRemote = addFlowEntry(vtnMgr, flowRemote, innc, (short) 1, outnc, pri);

        task = new FlowAddTask(vtnMgr, flowRemote);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flowRemote, null, 0);

        // in case ingress is remote node.
        flow = fdb.create(vtnMgr);
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node1);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node1);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);
        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // in case ingress is not connected node.
        flow = fdb.create(vtnMgr);
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node2);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node2);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // empty flow
        flow = fdb.create(vtnMgr);
        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);
    }

    /**
     * Test method for
     * {@link FlowAddTask#execute()},
     * {@link FlowRemoveTask#execute()},
     * {@link FlowModTask#run()}.
     *
     * In this case {@link installLocal()} and
     * {@link uninstallLocal()} timeout.
     */
    @Test
    public void testFlowTaskTestLocalTimeout() {
        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();

        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));

        // replace forwardingRulesManager to stub.
        // installLocal() and uninstallLocal() implemented in it
        // wait until timeout.
        ForwardingRulesManagerStub frm = new ForwardingRulesManagerStub();
        vtnMgr.setForwardingRuleManager(frm);

        // in case ingress only.
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        int pri = 1;
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        fdb.createIndex(vtnMgr, flow);
        FlowAddTask task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // in case ingress + local
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);
        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // in case thread is interrupted.
        TimerTask timerTask = new InterruptTask(Thread.currentThread());
        Timer timer = new Timer();

        task = new FlowAddTask(vtnMgr, flow);
        timer.schedule(timerTask, timeout / 2);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);
    }

    /**
     * Test method for
     * {@link FlowAddTask#execute()},
     * {@link FlowRemoveTask#execute()},
     * {@link FlowModTask#run()}.
     *
     * <p>
     *  in case system have Remote cluster nodes.
     * </p>
     */
    @Test
    public void testFlowTaskTestHavingRemoteNodes() {
        // set IClusterGlobalService to stub which work
        // as have multiple cluster nodes.
        TestStubCluster stubNew = new TestStubCluster(2);
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);
        resMgr.setClusterGlobalService(stubNew);
        resMgr.init(c);
        stopVTNManager(true);
        startVTNManager(c);

        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();
        long remoteTimeout = vtnMgr.getVTNConfig().getRemoteFlowModTimeout();
        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));
        Node node2 = NodeCreator.createOFNode(Long.valueOf(2L));

        // ingress
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short) 1);
        ActionList actions = new ActionList(outnc.getNode());
        actions.addOutput(outnc);
        int pri = 1;
        flow.addFlow(vtnMgr, match, actions, pri);

        // + local entry.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        // + remote entry.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node1);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node1);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);
        FlowAddTask task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        Set<ClusterEvent> events = getPostedClusterEvent();
        assertEquals(2, events.size());
        clearPostedClusterEvent();

        Iterator<FlowEntry> it = flow.getFlowEntries().iterator();
        FlowEntry ingress = it.next();
        FlowRemoveTask rtask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        rtask.run();
        assertEquals(FlowModResult.FAILED, rtask.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();

        fdb.clear(vtnMgr);
        flushFlowTasks(remoteTimeout);
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // ingress + local + remote.
        // in this case succeed to add and remove remote flow entry.
        FlowEntry rent = null;
        it = flow.getFlowEntries().iterator();
        while (it.hasNext()) {
            rent = it.next();
        }
        TimerTask timerTask = new ResultTimerTask(vtnMgr, rent,
                                                  FlowModResult.SUCCEEDED, false);
        Timer timer = new Timer();
        timer.schedule(timerTask, remoteTimeout / 2);

        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.SUCCEEDED, task.getResult(remoteTimeout));
        // entry managed by local is installed.
        checkRegsiterdFlowEntry(vtnMgr, 1, flow, flow, 2);

        events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();

        it = flow.getFlowEntries().iterator();
        ingress = it.next();
        timerTask = new ResultTimerTask(vtnMgr, rent, FlowModResult.SUCCEEDED,
                                        false);
        timer.schedule(timerTask, remoteTimeout / 2);
        rtask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        rtask.run();
        assertEquals(FlowModResult.SUCCEEDED, rtask.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();

        fdb.clear(vtnMgr);
        flushFlowTasks(remoteTimeout);
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // ingress + local + remote + not connected node.
        // in this case succeed to add and remove remote flow entry.
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node2);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node2);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        rent = null;
        it = flow.getFlowEntries().iterator();
        while (it.hasNext()) {
            rent = it.next();
        }
        timerTask = new ResultTimerTask(vtnMgr, rent, FlowModResult.SUCCEEDED,
                                        false);
        timer.schedule(timerTask, remoteTimeout / 2);

        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(remoteTimeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        events = getPostedClusterEvent();
        // when include not connected node,
        // don't execute install of remote entry.
        assertEquals(0, events.size());
        clearPostedClusterEvent();

        fdb.clear(vtnMgr);
        flushFlowTasks(remoteTimeout);
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // in case installLocal() timeout.
        ForwardingRulesManagerStub frm = new ForwardingRulesManagerStub();
        vtnMgr.setForwardingRuleManager(frm);

        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        fdb.createIndex(vtnMgr, flow);
        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.FAILED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);
    }

    /**
     * Test method for
     * {@link FlowRemoveTask}.
     */
    @Test
    public void testFlowRemoveTask() {
        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();

        Set<FlowGroupId> gidset = new HashSet<FlowGroupId>();
        List<FlowEntry> ingress = new ArrayList<FlowEntry>();
        List<FlowEntry> entries = new ArrayList<FlowEntry>();

        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));

        // add a VTNFlow.
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short) 1);

        ActionList actions = new ActionList(outnc.getNode());
        actions.addOutput(outnc);
        int pri = 1;
        flow.addFlow(vtnMgr, match, actions, pri);

        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        fdb.createIndex(vtnMgr, flow);
        FlowAddTask task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 1, flow, flow, 2);

        gidset.add(flow.getGroupId());
        Iterator<FlowEntry> it = flow.getFlowEntries().iterator();
        ingress.add(it.next());
        while (it.hasNext()) {
            entries.add(it.next());
        }

        // add a second VTNFlow.
        flow = fdb.create(vtnMgr);
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("12"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);


        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("13"),
                                                          node0);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("14"),
                                                           node0);
        flow = addFlowEntry(vtnMgr, flow, innc, (short) 1, outnc, pri);

        fdb.createIndex(vtnMgr, flow);
        task = new FlowAddTask(vtnMgr, flow);
        task.run();
        assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 2, flow, flow, 4);

        gidset.add(flow.getGroupId());
        it = flow.getFlowEntries().iterator();
        ingress.add(it.next());
        while (it.hasNext()) {
            entries.add(it.next());
        }

        // test FlowRemoveTask
        FlowRemoveTask rtask = new FlowRemoveTask(vtnMgr, gidset, ingress,
                                                  entries);
        rtask.run();
        assertEquals(FlowModResult.SUCCEEDED, rtask.getResult());
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);

        // remove with no entry existing.
        gidset.clear();
        ingress.clear();
        entries.clear();
        gidset.add(flow.getGroupId());
        it = flow.getFlowEntries().iterator();
        ingress.add(it.next());
        while (it.hasNext()) {
            entries.add(it.next());
        }

        rtask = new FlowRemoveTask(vtnMgr, gidset, ingress, entries);
        rtask.run();
        assertEquals(FlowModResult.FAILED, rtask.getResult(timeout));
        checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0);
    }

    /**
     * Test method for
     * {@link FlowModTask#getVTNManager()}.
     */
    @Test
    public void testGetVTNManager() {
        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);

        FlowAddTask addTask = new FlowAddTask(vtnMgr, flow);
        assertSame(vtnMgr, addTask.getVTNManager());

        Iterator<FlowEntry> it = flow.getFlowEntries().iterator();
        FlowRemoveTask removeTask = new FlowRemoveTask(vtnMgr,
                                                       flow.getGroupId(),
                                                       null, it);
        assertSame(vtnMgr, removeTask.getVTNManager());
    }

    /**
     * Test method for
     * {@link FlowModTask#getResult(long)},
     * {@link FlowModTask#getResultAbs(long)},
     * {@link FlowModTask#setResult(boolean)}.
     */
    @Test
    public void testGetResult() {
        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);
        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();

        // getResult() timeout
        FlowAddTask addTask = new FlowAddTask(vtnMgr, flow);
        long start = System.currentTimeMillis();
        assertEquals(FlowModResult.TIMEDOUT, addTask.getResult(timeout));
        long end = System.currentTimeMillis();
        assertTrue(timeout <= (end - start));

        long timeoutRemote = vtnMgr.getVTNConfig().getRemoteFlowModTimeout();
        FlowEntry ingress = null;
        Iterator<FlowEntry> it = flow.getFlowEntries().iterator();
        FlowRemoveTask rmTask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        start = System.currentTimeMillis();
        assertEquals(FlowModResult.TIMEDOUT, rmTask.getResult());
        end = System.currentTimeMillis();
        assertTrue(timeoutRemote <= (end - start));

        long timeoutRemoteBulk = vtnMgr.getVTNConfig().getRemoteBulkFlowModTimeout();
        List<FlowEntry> ingresss = new ArrayList<FlowEntry>();
        List<FlowEntry> entries = new ArrayList<FlowEntry>();
        Set<FlowGroupId> gidset = new HashSet<FlowGroupId>();
        rmTask = new FlowRemoveTask(vtnMgr, gidset, ingresss, entries);
        start = System.currentTimeMillis();
        assertEquals(FlowModResult.TIMEDOUT, rmTask.getResult());
        end = System.currentTimeMillis();
        assertTrue(timeoutRemoteBulk <= (end - start));

        // succeeded
        addTask = new FlowAddTask(vtnMgr, flow);
        addTask.setResult(true);
        assertEquals(FlowModResult.SUCCEEDED, addTask.getResult(timeout));

        rmTask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        rmTask.setResult(true);
        assertEquals(FlowModResult.SUCCEEDED, rmTask.getResult(timeoutRemote));

        // failed
        addTask = new FlowAddTask(vtnMgr, flow);
        addTask.setResult(false);
        assertEquals(FlowModResult.FAILED, addTask.getResult(timeout));

        rmTask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        rmTask.setResult(false);
        assertEquals(FlowModResult.FAILED, rmTask.getResult(timeoutRemote));

        // getResult() interrupted
        TimerTask timerTask = new InterruptTask(Thread.currentThread());
        Timer timer = new Timer();

        addTask = new FlowAddTask(vtnMgr, flow);
        timer.schedule(timerTask, timeout / 2);
        assertEquals(FlowModResult.INTERRUPTED, addTask.getResult(timeout));

        timerTask = new InterruptTask(Thread.currentThread());
        rmTask = new FlowRemoveTask(vtnMgr, flow.getGroupId(), ingress, it);
        timer.schedule(timerTask, timeoutRemote / 2);
        assertEquals(FlowModResult.INTERRUPTED, rmTask.getResult(timeoutRemote));
    }


    // private methods

    /**
     * check specified Flow Entry is registerd correctly.
     *
     * @param numFlows          the number of Flows.
     * @param registerdFlow     VTNFlow which is registerd.
     * @param numFlowEntries    the number of Flow Entries.
     */
    private void checkRegsiterdFlowEntry(VTNManagerImpl mgr, int numFlows,
                                         VTNFlow registerdFlow, VTNFlow expectedFlow,
                                         int numFlowEntries) {
        ConcurrentMap<FlowGroupId, VTNFlow> db = mgr.getFlowDB();
        assertEquals(numFlows, db.size());
        assertEquals(expectedFlow, db.get(registerdFlow.getGroupId()));
        assertEquals(numFlowEntries, stubObj.getFlowEntries().size());
    }
}
