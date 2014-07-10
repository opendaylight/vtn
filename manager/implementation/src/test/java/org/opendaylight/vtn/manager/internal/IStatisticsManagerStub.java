/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.core.NodeTable;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.reader.FlowOnNode;
import org.opendaylight.controller.sal.reader.IReadService;
import org.opendaylight.controller.sal.reader.NodeConnectorStatistics;
import org.opendaylight.controller.sal.reader.NodeDescription;
import org.opendaylight.controller.sal.reader.NodeTableStatistics;
import org.opendaylight.controller.statisticsmanager.IStatisticsManager;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.opendaylight.controller.statisticsmanager.IStatisticsManager;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.reader.FlowOnNode;

/**
 * The Stub implementation for IstaticManager to return the Flows
 * services and provides API to retrieve them.
 */
public class IStatisticsManagerStub implements IStatisticsManager {
  private static final Logger log = LoggerFactory.getLogger(IStatisticsManagerStub.class);
  private IReadService reader;
  //statistics caches
  private ConcurrentMap<Node, List<FlowOnNode>> flowStatistics;
  private ConcurrentMap<Node, List<NodeConnectorStatistics>> nodeConnectorStatistics;
  private ConcurrentMap<Node, List<NodeTableStatistics>> tableStatistics;
  private ConcurrentMap<Node, NodeDescription> descriptionStatistics;

  // data structure for latches
  // this is not a cluster cache
  private ConcurrentMap<Node, CountDownLatch> latches = new ConcurrentHashMap<Node, CountDownLatch>();
  // 30 seconds is the timeout.
  // the value of this can be tweaked based on performance tests.
  private static long latchTimeout = 30;

  // cache for flow stats refresh triggers
  // an entry added to this map triggers the statistics manager
  // to which the node is connected to get the latest flow stats from that node
  // this is a cluster cache
  private ConcurrentMap<Integer, Node> triggers;

  // use an atomic integer for the triggers key
  private AtomicInteger triggerKey = new AtomicInteger();

  // single thread executor for the triggers
  private ExecutorService triggerExecutor;

  static final String TRIGGERS_CACHE = "statisticsmanager.triggers";
  static final String FLOW_STATISTICS_CACHE = "statisticsmanager.flowStatistics";

  @Override
    public List<FlowOnNode> getFlows(Node node) {
      if (node == null) {
        return Collections.emptyList();
      }else{
        return create_flow();
      }
    }
  /**
   * Function will check the node is null and if the node is not null creates the flow.
   */
  @Override
    public List<FlowOnNode> getFlowsNoCache(Node node) {
      if (node == null) {
        return Collections.emptyList();
      }else{
        return create_flow();
      }
    }
  /**
   * Function will create and return flow to the calling functions.
   */

  private  List<FlowOnNode> create_flow(){
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
    return flows;
  }

  @Override
    public Map<Node, List<FlowOnNode>> getFlowStatisticsForFlowList(List<FlowEntry> flowList) {
      Map<Node, List<FlowOnNode>> statMapOutput = new HashMap<Node, List<FlowOnNode>>();

      if (flowList == null || flowList.isEmpty()){
        return statMapOutput;
      }

      Node node;
      // Index FlowEntries' flows by node so we don't traverse entire flow list for each flowEntry
      Map<Node, Set<Flow>> index = new HashMap<Node, Set<Flow>>();
      for (FlowEntry flowEntry : flowList) {
        node = flowEntry.getNode();
        Set<Flow> set = (index.containsKey(node) ? index.get(node) : new HashSet<Flow>());
        set.add(flowEntry.getFlow());
        index.put(node, set);
      }

      // Iterate over flows per indexed node and add to output
      for (Entry<Node, Set<Flow>> indexEntry : index.entrySet()) {
        node = indexEntry.getKey();
        List<FlowOnNode> flowsPerNode = flowStatistics.get(node);

        if (flowsPerNode != null && !flowsPerNode.isEmpty()){
          List<FlowOnNode> filteredFlows = statMapOutput.containsKey(node) ?
            statMapOutput.get(node) : new ArrayList<FlowOnNode>();

          for (FlowOnNode flowOnNode : flowsPerNode) {
            if (indexEntry.getValue().contains(flowOnNode.getFlow())) {
              filteredFlows.add(flowOnNode);
            }
          }
          statMapOutput.put(node, filteredFlows);
        }
      }
      return statMapOutput;
    }

  @Override
    public int getFlowsNumber(Node node) {
      List<FlowOnNode> l;
      if (node == null || (l = flowStatistics.get(node)) == null){
        return -1;
      }
      return l.size();
    }

  @Override
    public NodeDescription getNodeDescription(Node node) {
      if (node == null){
        return null;
      }
      NodeDescription nd = descriptionStatistics.get(node);
      return nd != null? nd.clone() : null;
    }

  @Override
    public NodeConnectorStatistics getNodeConnectorStatistics(NodeConnector nodeConnector) {
      if (nodeConnector == null){
        return null;
      }

      List<NodeConnectorStatistics> statList = nodeConnectorStatistics.get(nodeConnector.getNode());
      if (statList != null){
        for (NodeConnectorStatistics stat : statList) {
          if (stat.getNodeConnector().equals(nodeConnector)){
            return stat;
          }
        }
      }
      return null;
    }

  @Override
    public List<NodeConnectorStatistics> getNodeConnectorStatistics(Node node) {
      if (node == null){
        return Collections.emptyList();
      }

      List<NodeConnectorStatistics> statList = new ArrayList<NodeConnectorStatistics>();
      List<NodeConnectorStatistics> cachedList = nodeConnectorStatistics.get(node);
      if (cachedList != null) {
        statList.addAll(cachedList);
      }
      return statList;
    }

  @Override
    public NodeTableStatistics getNodeTableStatistics(NodeTable nodeTable) {
      if (nodeTable == null){
        return null;
      }
      List<NodeTableStatistics> statList = tableStatistics.get(nodeTable.getNode());
      if (statList != null){
        for (NodeTableStatistics stat : statList) {
          if (stat.getNodeTable().getID().equals(nodeTable.getID())){
            return stat;
          }
        }
      }
      return null;
    }

  @Override
    public List<NodeTableStatistics> getNodeTableStatistics(Node node){
      if (node == null){
        return Collections.emptyList();
      }
      List<NodeTableStatistics> statList = new ArrayList<NodeTableStatistics>();
      List<NodeTableStatistics> cachedList = tableStatistics.get(node);
      if (cachedList != null) {
        statList.addAll(cachedList);
      }
      return statList;
    }
}

