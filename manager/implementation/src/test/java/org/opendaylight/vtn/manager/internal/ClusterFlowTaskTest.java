/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * test for {@link ClusterFlowAddTask} and {@link ClusterFlowRemoveTask}.
 */
@Category(SlowTest.class)
public class ClusterFlowTaskTest extends FlowModTaskTestBase {

    @Before
    public void setupTimeoutConfig() {
        setupVTNManagerForRemoteTaskTest(1000L, 1000L);
    }

    @After
    public void cleaupTimeoutConfig() {
        cleanupSetupFile();
    }

    /**
     * Test method for
     * {@link ClusterFlowModTask#ClusterFlowModTask(VTNManagerImpl, FlowEntry)},
     * {@link ClusterFlowAddTask#ClusterFlowAddTask(VTNManagerImpl, FlowEntry)},
     * {@link ClusterFlowRemoveTask#ClusterFlowRemoveTask(VTNManagerImpl, FlowEntry)},
     */
    @Test
    public void testClusterFlowTask() {
        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();

        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));

        // ingress only.
        NodeConnector innc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                         node0);
        NodeConnector outnc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                         node0);
        Match match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short)1);
        ActionList actions = new ActionList(outnc.getNode(), (short)1);
        actions.addOutput(outnc);
        int pri = 1;
        flow.addFlow(vtnMgr, match, actions, pri);

        // test ClusterFlowAddTask
        int i = 0;
        for (FlowEntry ent : flow.getFlowEntries()) {
            ClusterFlowAddTask task = new ClusterFlowAddTask(vtnMgr, ent);
            assertEquals(ent, task.getFlowEntry());
            assertSame(vtnMgr, task.getVTNManager());

            task.run();
            assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
            assertEquals(++i, stubObj.getFlowEntries().size());
            assertTrue(stubObj.getFlowEntries().contains(ent));

            Set<ClusterEvent> events = getPostedClusterEvent();
            assertEquals(1, events.size());
            clearPostedClusterEvent();
        }

        // add conflict flow entry.
        FlowEntry entConflict = flow.getFlowEntries().iterator().next();
        ClusterFlowAddTask taskConflict = new  ClusterFlowAddTask(vtnMgr, entConflict);
        taskConflict.run();
        assertEquals(FlowModResult.FAILED, taskConflict.getResult(timeout));
        assertEquals(i, stubObj.getFlowEntries().size());
        assertTrue(stubObj.getFlowEntries().contains(entConflict));

        Set<ClusterEvent> events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();

        // test ClusterFlowRemoveTask
        for (FlowEntry ent : flow.getFlowEntries()) {
            ClusterFlowRemoveTask task = new ClusterFlowRemoveTask(vtnMgr, ent);
            assertEquals(ent, task.getFlowEntry());
            assertSame(vtnMgr, task.getVTNManager());

            task.run();
            assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
            assertEquals(--i, stubObj.getFlowEntries().size());
            assertFalse(stubObj.getFlowEntries().contains(ent));

            events = getPostedClusterEvent();
            assertEquals(1, events.size());
            clearPostedClusterEvent();
        }

        // remove not existing entry.
        FlowEntry entNoexist = flow.getFlowEntries().iterator().next();
        ClusterFlowRemoveTask taskNoexist
            = new  ClusterFlowRemoveTask(vtnMgr, entNoexist);
        taskNoexist.run();
        assertEquals(FlowModResult.FAILED, taskNoexist.getResult(timeout));
        assertEquals(0, stubObj.getFlowEntries().size());

        events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();

        // timed out
        ForwardingRulesManagerStub frm = new ForwardingRulesManagerStub();
        vtnMgr.setForwardingRuleManager(frm);

        FlowEntry entTimeout = flow.getFlowEntries().iterator().next();
        ClusterFlowAddTask taskTimeoutAdd = new  ClusterFlowAddTask(vtnMgr, entTimeout);
        taskTimeoutAdd.run();
        assertEquals(FlowModResult.FAILED, taskTimeoutAdd.getResult(timeout));
        assertEquals(0, stubObj.getFlowEntries().size());

        events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();

        ClusterFlowRemoveTask taskTimeoutRemove
            = new ClusterFlowRemoveTask(vtnMgr, entTimeout);
        taskTimeoutRemove.run();
        assertEquals(FlowModResult.FAILED, taskTimeoutRemove.getResult(timeout));
        assertEquals(0, stubObj.getFlowEntries().size());

        events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();

        // not local entry.
        flow = fdb.create(vtnMgr);
        innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                          node1);
        outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("11"),
                                                           node1);
        match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short)1);
        actions = new ActionList(outnc.getNode(), (short)1);
        actions.addOutput(outnc);
        flow.addFlow(vtnMgr, match, actions, pri);
        ClusterFlowAddTask addTask = new ClusterFlowAddTask(vtnMgr, flow.getFlowEntries().get(0));
        addTask.run();
        // when ignored, getResult() return SUCCEEDED
        assertEquals(FlowModResult.SUCCEEDED, addTask.getResult(timeout));
        assertEquals(0, stubObj.getFlowEntries().size());

        events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();

        ClusterFlowRemoveTask removeTask = new ClusterFlowRemoveTask(vtnMgr, flow.getFlowEntries().get(0));
        removeTask.run();
        // when ignored, getResult() return SUCCEEDED
        assertEquals(FlowModResult.SUCCEEDED, removeTask.getResult(timeout));
        assertEquals(0, stubObj.getFlowEntries().size());

        events = getPostedClusterEvent();
        assertEquals(1, events.size());
        clearPostedClusterEvent();
    }

    /**
     * Test method for
     * {@link FlowModTask#getResult(long)},
     * {@link FlowModTask#getResultAbs(long)},
     * {@link FlowModTask#setResult(boolean)}.
     */
    @Test
    public void testGetResult() {
        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();

        // getResult() timeout
        FlowEntry flow = null;
        ClusterFlowAddTask addTask = new ClusterFlowAddTask(vtnMgr, flow);
        long start = System.currentTimeMillis();
        assertEquals(FlowModResult.TIMEDOUT, addTask.getResult(timeout));
        long end = System.currentTimeMillis();
        assertTrue(timeout <= (end - start));

        long timeoutRemote = vtnMgr.getVTNConfig().getRemoteFlowModTimeout();
        ClusterFlowRemoveTask rmTask = new ClusterFlowRemoveTask(vtnMgr, flow);
        start = System.currentTimeMillis();
        assertEquals(FlowModResult.TIMEDOUT, rmTask.getResult(timeoutRemote));
        end = System.currentTimeMillis();
        assertTrue(timeoutRemote <= (end - start));

        // succeeded
        addTask = new ClusterFlowAddTask(vtnMgr, flow);
        addTask.setResult(true);
        assertEquals(FlowModResult.SUCCEEDED, addTask.getResult(timeout));

        rmTask = new ClusterFlowRemoveTask(vtnMgr, flow);
        rmTask.setResult(true);
        assertEquals(FlowModResult.SUCCEEDED, rmTask.getResult(timeoutRemote));

        // failed
        addTask = new ClusterFlowAddTask(vtnMgr, flow);
        addTask.setResult(false);
        assertEquals(FlowModResult.FAILED, addTask.getResult(timeout));

        rmTask = new ClusterFlowRemoveTask(vtnMgr, flow);
        rmTask.setResult(false);
        assertEquals(FlowModResult.FAILED, rmTask.getResult(timeoutRemote));

        // getResult() interrupted
        TimerTask timerTask = new InterruptTask(Thread.currentThread());
        Timer timer = new Timer();

        addTask = new ClusterFlowAddTask(vtnMgr, flow);
        timer.schedule(timerTask, timeout / 2);
        assertEquals(FlowModResult.INTERRUPTED, addTask.getResult(timeout));

        timerTask = new InterruptTask(Thread.currentThread());
        rmTask = new ClusterFlowRemoveTask(vtnMgr, flow);
        timer.schedule(timerTask, timeoutRemote / 2);
        assertEquals(FlowModResult.INTERRUPTED, rmTask.getResult(timeoutRemote));
    }
}
