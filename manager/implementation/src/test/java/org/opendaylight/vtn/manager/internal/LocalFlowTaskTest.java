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
 * JUnit test for {@link LocalFlowAddTask} and {@link LocalFlowRemoveTask}.
 */
@Category(SlowTest.class)
public class LocalFlowTaskTest extends FlowModTaskTestBase {

    @Before
    public void setupTimeoutConfig() {
        setupVTNManagerForRemoteTaskTest(1000L, 1000L, 3000L);
    }

    @After
    public void cleaupTimeoutConfig() {
        cleanupSetupFile();
    }

    /**
     * Test method for
     * {@link LocalFlowAddTask} and {@link LocalFlowRemoveTask}.
     */
    @Test
    public void testLocalFlowTask() {
        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();

        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));

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

        ActionList actions = new ActionList(outnc.getNode());
        actions.addOutput(outnc);
        int pri = 1;
        flow.addFlow(vtnMgr, match, actions, pri);

        // test LocalFlowAddTask
        int i = 0;
        for (FlowEntry ent : flow.getFlowEntries()) {
            LocalFlowAddTask task = new LocalFlowAddTask(vtnMgr, ent);
            assertEquals(ent, task.getFlowEntry());
            assertSame(vtnMgr, task.getVTNManager());

            task.run();
            assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
            assertEquals(++i, stubObj.getFlowEntries().size());
            assertTrue(stubObj.getFlowEntries().contains(ent));
        }

        // add conflict flow entry.
        FlowEntry entConflict = flow.getFlowEntries().iterator().next();
        LocalFlowAddTask taskConflict = new  LocalFlowAddTask(vtnMgr, entConflict);
        taskConflict.run();
        assertEquals(FlowModResult.FAILED, taskConflict.getResult(timeout));
        assertEquals(i, stubObj.getFlowEntries().size());
        assertTrue(stubObj.getFlowEntries().contains(entConflict));

        // test LocalFlowRemoveTask
        for (FlowEntry ent : flow.getFlowEntries()) {
            LocalFlowRemoveTask task = new LocalFlowRemoveTask(vtnMgr, ent);
            assertEquals(ent, task.getFlowEntry());
            assertSame(vtnMgr, task.getVTNManager());

            task.run();
            assertEquals(FlowModResult.SUCCEEDED, task.getResult(timeout));
            assertEquals(--i, stubObj.getFlowEntries().size());
            assertFalse(stubObj.getFlowEntries().contains(ent));
        }

        // add conflict flow entry.
        FlowEntry entNoexist = flow.getFlowEntries().iterator().next();
        LocalFlowRemoveTask taskNoexist
            = new  LocalFlowRemoveTask(vtnMgr, entNoexist);
        taskNoexist.run();
        assertEquals(FlowModResult.FAILED, taskNoexist.getResult(timeout));
        assertEquals(0, stubObj.getFlowEntries().size());

        // timed out
        ForwardingRulesManagerStub frm = new ForwardingRulesManagerStub();
        vtnMgr.setForwardingRuleManager(frm);

        FlowEntry entTimeout = flow.getFlowEntries().iterator().next();
        LocalFlowAddTask taskTimeoutAdd = new  LocalFlowAddTask(vtnMgr, entTimeout);
        taskTimeoutAdd.run();
        assertEquals(FlowModResult.FAILED, taskTimeoutAdd.getResult(timeout));
        assertEquals(0, stubObj.getFlowEntries().size());

        LocalFlowRemoveTask taskTimeoutRemove
            = new LocalFlowRemoveTask(vtnMgr, entTimeout);
        taskTimeoutRemove.run();
        assertEquals(FlowModResult.FAILED, taskTimeoutRemove.getResult(timeout));
        assertEquals(0, stubObj.getFlowEntries().size());

        Set<ClusterEvent> sets = getPostedClusterEvent();
        assertEquals(0, sets.size());
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
        LocalFlowAddTask addTask = new LocalFlowAddTask(vtnMgr, flow);
        long start = System.currentTimeMillis();
        assertEquals(FlowModResult.TIMEDOUT, addTask.getResult(timeout));
        long end = System.currentTimeMillis();
        assertTrue(timeout <= (end - start));

        long timeoutRemote = vtnMgr.getVTNConfig().getRemoteFlowModTimeout();
        LocalFlowRemoveTask rmTask = new LocalFlowRemoveTask(vtnMgr, flow);
        start = System.currentTimeMillis();
        assertEquals(FlowModResult.TIMEDOUT, rmTask.getResult(timeoutRemote));
        end = System.currentTimeMillis();
        assertTrue(timeoutRemote <= (end - start));

        // succeeded
        addTask = new LocalFlowAddTask(vtnMgr, flow);
        addTask.setResult(true);
        assertEquals(FlowModResult.SUCCEEDED, addTask.getResult(timeout));

        rmTask = new LocalFlowRemoveTask(vtnMgr, flow);
        rmTask.setResult(true);
        assertEquals(FlowModResult.SUCCEEDED, rmTask.getResult(timeoutRemote));

        // failed
        addTask = new LocalFlowAddTask(vtnMgr, flow);
        addTask.setResult(false);
        assertEquals(FlowModResult.FAILED, addTask.getResult(timeout));

        rmTask = new LocalFlowRemoveTask(vtnMgr, flow);
        rmTask.setResult(false);
        assertEquals(FlowModResult.FAILED, rmTask.getResult(timeoutRemote));

        // getResult() interrupted
        TimerTask timerTask = new InterruptTask(Thread.currentThread());
        Timer timer = new Timer();

        addTask = new LocalFlowAddTask(vtnMgr, flow);
        timer.schedule(timerTask, timeout / 2);
        assertEquals(FlowModResult.INTERRUPTED, addTask.getResult(timeout));

        timerTask = new InterruptTask(Thread.currentThread());
        rmTask = new LocalFlowRemoveTask(vtnMgr, flow);
        timer.schedule(timerTask, timeoutRemote / 2);
        assertEquals(FlowModResult.INTERRUPTED, rmTask.getResult(timeoutRemote));
    }
}
