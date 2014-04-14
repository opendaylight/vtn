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
import java.util.List;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

import org.junit.Test;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * JUnit test for {@link RemoteFlowRequest}.
 */
public class RemoteFlowRequestTest extends TestUseVTNManagerBase {

    /**
     * Construct a new instance.
     */
    public RemoteFlowRequestTest() {
        super(2);
    }

    /**
     * Test method for
     * {@link RemoteFlowRequest#RemoteFlowRequest(List)},
     * {@link RemoteFlowRequest#setResult(String, FlowModResult, int)},
     * {@link RemoteFlowRequest#getResultAbs(long, boolean)}.
     */
    @Test
    public void testRemoteFlowRequest() {
        VTNFlowDatabase fdb = new VTNFlowDatabase("test");
        VTNFlow flow = fdb.create(vtnMgr);

        // create flow entries.
        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
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
        match = new Match();
        match.setField(MatchType.IN_PORT, innc);
        match.setField(MatchType.DL_VLAN, (short) 1);
        actions = new ActionList(outnc.getNode());
        actions.addOutput(outnc);
        flow.addFlow(vtnMgr, match, actions, pri);

        List<FlowEntry> entries = flow.getFlowEntries();

        Set<FlowModResult> resultSet = new HashSet<FlowModResult>();
        resultSet.add(FlowModResult.SUCCEEDED);
        resultSet.add(FlowModResult.FAILED);
        resultSet.add(FlowModResult.IGNORED);

        // removeNode == 1
        for (FlowModResult result0 : resultSet) {
            for (FlowModResult result1 : resultSet) {
                for (Boolean all : createBooleans(false)) {
                    for (int c = 0; c < 3; c++) {
                        RemoteFlowRequest req = new RemoteFlowRequest(entries);
                        if (c < 2) {
                            req.setResult(entries.get(0).getFlowName(),
                                          result0, c);
                            req.setResult(entries.get(1).getFlowName(),
                                          result1, c);
                        } else {
                            // in case c == 2
                            req.setResult(entries.get(0).getFlowName(),
                                          FlowModResult.IGNORED, c);
                            req.setResult(entries.get(0).getFlowName(),
                                          result0, c);
                            req.setResult(entries.get(1).getFlowName(),
                                          result1, c);
                            req.setResult(entries.get(1).getFlowName(),
                                          FlowModResult.IGNORED, c);
                        }

                        if (result0 == FlowModResult.SUCCEEDED
                                && result1 == FlowModResult.SUCCEEDED) {
                            assertTrue(req.getResultAbs(0L, all.booleanValue()));
                            // call again
                            assertTrue(req.getResultAbs(0L, all.booleanValue()));
                        } else {
                            assertFalse(req.getResultAbs(0L, all.booleanValue()));
                            // call again
                            assertFalse(req.getResultAbs(0L, all.booleanValue()));
                        }
                    }
                }
            }
        }

        // one is not set.
        for (FlowModResult result0 : resultSet) {
            for (Boolean all : createBooleans(false)) {
                RemoteFlowRequest req = new RemoteFlowRequest(entries);
                req.setResult(entries.get(0).getFlowName(), result0, 1);
                assertFalse(req.getResultAbs(0L, all.booleanValue()));
            }
        }

        // set result which have invalid flow entry name.
        for (Boolean all : createBooleans(false)) {
            RemoteFlowRequest req = new RemoteFlowRequest(entries);
            req.setResult(entries.get(0).getFlowName(),
                          FlowModResult.SUCCEEDED, 1);
            req.setResult(entries.get(1).getFlowName(),
                          FlowModResult.SUCCEEDED, 1);
            req.setResult("not-match-name", FlowModResult.FAILED, 1);
            assertTrue(req.getResultAbs(0L, all.booleanValue()));
        }

        // in case time out
        for (Boolean all : createBooleans(false)) {
            RemoteFlowRequest req = new RemoteFlowRequest(entries);
            assertFalse(req.getResultAbs(1L, all.booleanValue()));
        }

        // remoteNode == 2
        for (FlowModResult result0 : resultSet) {
            for (FlowModResult result1 : resultSet) {
                for (Boolean all : createBooleans(false)) {
                    RemoteFlowRequest req = new RemoteFlowRequest(entries);
                    req.setResult(entries.get(0).getFlowName(),
                                  FlowModResult.IGNORED, 2);
                    req.setResult(entries.get(0).getFlowName(), result0, 2);
                    req.setResult(entries.get(1).getFlowName(), result1, 2);
                    req.setResult(entries.get(1).getFlowName(),
                                  FlowModResult.IGNORED, 2);

                    if (result0 == FlowModResult.SUCCEEDED
                            && result1 == FlowModResult.SUCCEEDED) {
                        assertTrue(req.getResultAbs(0L, all.booleanValue()));
                        // call again
                        assertTrue(req.getResultAbs(0L, all.booleanValue()));
                    } else {
                        assertFalse(req.getResultAbs(0L, all.booleanValue()));
                        // call again
                        assertFalse(req.getResultAbs(0L, all.booleanValue()));
                    }
                }
            }
        }

        // in case when getResult is waited setResult is called.
        class ResultTimerTask extends TimerTask {
            private RemoteFlowRequest request = null;
            private String flowName = null;
            private FlowModResult result = null;
            private int remoteNodes = 0;

            ResultTimerTask(RemoteFlowRequest req, String name, FlowModResult res,
                            int nodes) {
                request = req;
                flowName = name;
                result = res;
                remoteNodes = nodes;
            }

            @Override
            public void run() {
                request.setResult(flowName, result, remoteNodes);
            }
        }

        Timer timer = new Timer();
        for (FlowModResult result0 : resultSet) {
            for (FlowModResult result1 : resultSet) {
                for (Boolean all : createBooleans(false)) {

                    RemoteFlowRequest req = new RemoteFlowRequest(entries);
                    req.setResult(entries.get(0).getFlowName(), result0, 1);

                    TimerTask task = new ResultTimerTask(req,
                            entries.get(1).getFlowName(), result1, 1);
                    timer.schedule(task, 10L);

                    long resTimeout = 1000L + System.currentTimeMillis();

                    boolean res = req.getResultAbs(resTimeout,
                                                   all.booleanValue());
                    long afterTime = System.currentTimeMillis();

                    assertTrue(resTimeout > afterTime);
                    if (result0 == FlowModResult.SUCCEEDED
                        && result1 == FlowModResult.SUCCEEDED) {
                        assertTrue(res);
                    } else {
                        assertFalse(res);
                    }
                    task.cancel();
                }
            }
        }

        // Timed out test.
        for (FlowModResult result: resultSet) {
            for (Boolean all : createBooleans(false)) {
                RemoteFlowRequest req = new RemoteFlowRequest(entries);
                req.setResult(entries.get(0).getFlowName(), result, 1);

                long resTimeout = 100L + System.currentTimeMillis();
                boolean res = req.getResultAbs(resTimeout, all.booleanValue());
                long afterTime = System.currentTimeMillis();

                assertFalse(res);
                if (!all.booleanValue() && result != FlowModResult.SUCCEEDED) {
                    // getResultAbs() should return without waiting for
                    // completion of all requests.
                    assertTrue(resTimeout > afterTime);
                } else {
                    // Request should timed out.
                    assertTrue(resTimeout <= afterTime);
                }
            }
        }

        class InterruptTask extends TimerTask {
            private Thread targetThread = null;

            InterruptTask(Thread th) {
                targetThread = th;
            }

            @Override
            public void run() {
                targetThread.interrupt();
            }
        }

        for (Boolean all : createBooleans(false)) {
            RemoteFlowRequest req = new RemoteFlowRequest(entries);
            req.setResult(entries.get(0).getFlowName(), FlowModResult.SUCCEEDED, 1);

            TimerTask task = new InterruptTask(Thread.currentThread());
            timer.schedule(task, 500L);

            long resTimeout = 1000L + System.currentTimeMillis();

            boolean res = req.getResultAbs(resTimeout, all.booleanValue());
            assertFalse(res);
            task.cancel();
        }

        timer.cancel();
    }
}
