/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.junit.Test;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.TimeUnit;
import java.net.InetAddress;

import org.opendaylight.vtn.manager.flow.FlowStats;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.reader.FlowOnNode;
import org.opendaylight.controller.statisticsmanager.IStatisticsManager;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;

import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.action.ActionType;
import org.opendaylight.controller.sal.action.Controller;
import org.opendaylight.controller.sal.action.Flood;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.core.ContainerFlow;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.IPProtocols;

/**
 * JUnit test for {@link StatsReaderTest}.
 */

public class StatsReaderTest extends TestBase {
  /**
   * Test case for getter methods.
   */
  @Test
    public void testGetter() {
      try{
        IStatisticsManager mgr = new IStatisticsManagerStub();
        /*Creating object for stats reader mgr- IStatisticsManager object, @update, @cache*/
        StatsReader statsreader = new StatsReader(mgr,true,false);
        StatsReader statsreader1 = new StatsReader(mgr,false,false);
        StatsReader statsreader2 = new StatsReader(mgr,true,true);
        StatsReader statsreader3 = new StatsReader(mgr,false,true);
        Node node1 = NodeCreator.createOFNode(1L);
        Node node2 = NodeCreator.createOFNode(25L);
        FlowEntry fe1 = new FlowEntry("Junit", "flow1", getSampleFlowV6(node1), node1);
        FlowEntry fe2 = new FlowEntry("Junit", "flow2", getSampleFlowV6(node2), node2);
        assertNotNull(statsreader.get(fe1));
        assertNotNull(statsreader1.get(fe1));
        assertNotNull(statsreader2.get(fe1));
        assertNotNull(statsreader3.get(fe2));
      }catch(Exception e){
      }
    }

  /**
   * Method to generate flows
   */
  private Flow getSampleFlowV6(Node node) throws Exception {
    Match match = new Match();
    NodeConnector inNC = NodeConnectorCreator.createNodeConnector((short)10, NodeCreator.createOFNode((long)10));
    NodeConnector outNC = NodeConnectorCreator.createNodeConnector((short)20, NodeCreator.createOFNode((long)20));

    match.setField(MatchType.DL_TYPE, EtherTypes.IPv4.shortValue());
    match.setField(MatchType.IN_PORT, inNC);

    Output output = new Output(outNC);
    ArrayList<Action> action = new ArrayList<Action>();
    action.add(output);

    Flow flow = new Flow (match, action);
    FlowOnNode flownode = new FlowOnNode(flow);
    flownode.setPacketCount(5L);
    flownode.setByteCount(10L);
    flownode.setTableId((byte) 0x0);
    flownode.setDurationSeconds(10);
    flownode.setDurationNanoseconds(23);
    List<FlowOnNode> flows = new ArrayList<FlowOnNode>();
    flows.add(flownode);
    return flow;
  }
}

