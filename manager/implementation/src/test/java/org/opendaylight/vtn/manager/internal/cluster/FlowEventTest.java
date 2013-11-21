/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.Test;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.ActionList;
import org.opendaylight.vtn.manager.internal.FlowModTaskTestBase;
import org.opendaylight.vtn.manager.internal.TestStubCluster;
import org.opendaylight.vtn.manager.internal.VTNFlowDatabase;


/**
 * JUnit Test for  {@link FlowAddEvent} and {@link FlowRemoveEvent}
 */
public class FlowEventTest extends FlowModTaskTestBase {

    /**
     * Test method for
     * {@link FlowModEvent#isSingleThreaded(boolean)}.
     */
    @Test
    public void testIsSingleThreaded() {
        List<FlowEntry> entries = new ArrayList<FlowEntry>();
        FlowAddEvent addEvent = new FlowAddEvent(entries);
        FlowRemoveEvent removeEvent = new FlowRemoveEvent(entries);

        for (Boolean local : createBooleans(false)) {
            // always return false.
            assertFalse(local.toString(),
                        addEvent.isSingleThreaded(local.booleanValue()));
            assertFalse(local.toString(),
                        removeEvent.isSingleThreaded(local.booleanValue()));
        }
    }

    /**
     * Test method for
     * {@link FlowAddEvent},
     * {@link FlowRemoveEvent}.
     */
    @Test
    public void testFlowEvent() {
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

        for (Boolean local : createBooleans(false)) {
            String emsg = local.toString();

            FlowAddEvent addEvent = new FlowAddEvent(flow.getFlowEntries());
            addEvent.received(vtnMgr, local.booleanValue());

            if (local) {
                assertEquals(emsg, 0, stubObj.getFlowEntries().size());
            } else {
                flushAsyncTask(remoteTimeout);

                assertEquals(emsg, 2, stubObj.getFlowEntries().size());
            }

            FlowRemoveEvent removeEvent
                    = new FlowRemoveEvent(flow.getFlowEntries());
            removeEvent.received(vtnMgr, local.booleanValue());

            flushAsyncTask(remoteTimeout);

            assertEquals(emsg, 0, stubObj.getFlowEntries().size());
        }
    }
}
