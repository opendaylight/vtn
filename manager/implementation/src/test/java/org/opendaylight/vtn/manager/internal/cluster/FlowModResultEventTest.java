/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import static org.junit.Assert.*;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Hashtable;
import java.util.Iterator;
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
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.ActionList;
import org.opendaylight.vtn.manager.internal.FlowEventTestBase;
import org.opendaylight.vtn.manager.internal.TestStub;
import org.opendaylight.vtn.manager.internal.TestStubCluster;
import org.opendaylight.vtn.manager.internal.VTNFlowDatabase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * test for {@link FlowModResultEvent}
 */
public class FlowModResultEventTest extends FlowEventTestBase {

    /**
     * Test method for {@link FlowModResultEvent#isSingleThreaded(boolean)}.
     */
    @Test
    public void testIsSingleThreaded() {
        FlowModResultEvent event = new FlowModResultEvent("", FlowModResult.SUCCEEDED);

        for (Boolean local : createBooleans(false)) {
            // always return false.
            assertFalse(local.toString(),
                        event.isSingleThreaded(local.booleanValue()));
        }
    }

    /**
     * A timer task used to emulate remote event.
     */
    private class ResultTimerTask extends TimerTask {
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
     * Test method for
     * {@link FlowModResultEvent}.
     */
    @Test
    public void testFlowModResultEvent() {
        setupVTNManagerForRemoteTaskTest(1000L, 1000L);

        long timeout = vtnMgr.getVTNConfig().getFlowModTimeout();
        long remoteTimeout = vtnMgr.getVTNConfig().getRemoteFlowModTimeout();

        // set IClusterGlobalService to stub which work
        // as have multiple cluster nodes.
        TestStubCluster stubNew = new TestStubCluster(2);
        ComponentImpl c = new ComponentImpl(null, null, null);
        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);
        stopVTNManager(true);
        setupResourceManager(c, stubNew);
        startVTNManager(c);

        // create tenant.
        VTenantPath path = new VTenantPath("tenant");
        Status st = vtnMgr.addTenant(path, new VTenantConfig(""));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(path.getTenantName());
        VTNFlow flow = fdb.create(vtnMgr);

        Node node0 = NodeCreator.createOFNode(Long.valueOf(0L));
        Node node1 = NodeCreator.createOFNode(Long.valueOf(1L));

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

        FlowEntry rent = null;
        Iterator<FlowEntry> it = flow.getFlowEntries().iterator();
        while (it.hasNext()) {
            rent = it.next();
            if (rent.getNode().equals(node1)) {
                break;
            }
            rent = null;
        }

       for (FlowModResult result : FlowModResult.values()) {
            for (Boolean local : createBooleans(false)) {
                String emsg = "(FlowModResult)" + result.toString()
                        + ",(local)" + local.toString();

                // FlowModResultEvent is called from this timerTask.
                TimerTask timerTask = new ResultTimerTask(vtnMgr, rent, result,
                                                          local.booleanValue());
                Timer timer = new Timer();

                timer.schedule(timerTask, 100L);
                fdb.install(vtnMgr, flow);
                flushFlowTasks(remoteTimeout * 3);
                timerTask.cancel();

                if (result == FlowModResult.SUCCEEDED && local == Boolean.FALSE) {
                    checkRegsiterdFlowEntry(vtnMgr, 1, flow, flow, 2, emsg);

                    Set<ClusterEvent> events = getPostedClusterEvent();
                    assertEquals(1, events.size());
                    clearPostedClusterEvent();

                    timerTask = new ResultTimerTask(vtnMgr, rent,
                                                    FlowModResult.SUCCEEDED,
                                                    false);
                    timer.schedule(timerTask, 100L);
                    fdb.clear(vtnMgr);
                    flushFlowTasks(remoteTimeout * 3);
                    timer.cancel();
                } else {
                    checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0, emsg);

                    Set<ClusterEvent> events = getPostedClusterEvent();
                    assertEquals(2, events.size());
                    clearPostedClusterEvent();

                    fdb.clear(vtnMgr);
                    flushFlowTasks(timeout);
                }

                fdb.clear(vtnMgr);
                flushFlowTasks(remoteTimeout * 3);
                checkRegsiterdFlowEntry(vtnMgr, 0, flow, null, 0, emsg);
                clearPostedClusterEvent();
            }
        }
       cleanupSetupFile();
    }


    // private methods
    /**
     * check specified Flow Entry is registerd correctly.
     *
     * @param numFlows          The number of Flows.
     * @param registerdFlow     VTNFlow which is registerd.
     * @param numFlowEntries    The number of Flow Entries.
     */
    private void checkRegsiterdFlowEntry(VTNManagerImpl mgr, int numFlows,
                                         VTNFlow registerdFlow, VTNFlow expectedFlow,
                                         int numFlowEntries, String emsg) {
        ConcurrentMap<FlowGroupId, VTNFlow> db = mgr.getFlowDB();
        assertEquals(emsg, numFlows, db.size());
        assertEquals(emsg, expectedFlow, db.get(registerdFlow.getGroupId()));
        assertEquals(emsg, numFlowEntries, stubObj.getFlowEntries().size());
    }
}
