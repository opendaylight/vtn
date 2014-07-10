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

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.ICacheUpdateAware;
import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterServices;
import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.IContainer;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.core.NodeTable;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.inventory.IListenInventoryUpdates;
import org.opendaylight.controller.sal.reader.FlowOnNode;
import org.opendaylight.controller.sal.reader.IReadService;
import org.opendaylight.controller.sal.reader.IReadServiceListener;
import org.opendaylight.controller.sal.reader.NodeConnectorStatistics;
import org.opendaylight.controller.sal.reader.NodeDescription;
import org.opendaylight.controller.sal.reader.NodeTableStatistics;
import org.opendaylight.controller.sal.utils.ServiceHelper;
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
  private IContainer container;
  private IClusterContainerServices clusterContainerService;
  private IReadService reader;
  private IConnectionManager connectionManager;
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

  private void nonClusterObjectCreate() {
    flowStatistics = new ConcurrentHashMap<Node, List<FlowOnNode>>();
    nodeConnectorStatistics = new ConcurrentHashMap<Node, List<NodeConnectorStatistics>>();
    tableStatistics = new ConcurrentHashMap<Node, List<NodeTableStatistics>>();
    descriptionStatistics = new ConcurrentHashMap<Node, NodeDescription>();
    triggers = new ConcurrentHashMap<Integer, Node>();
  }

  private void allocateCaches() {
    if (clusterContainerService == null) {
      nonClusterObjectCreate();
      log.error("Clustering service unavailable. Allocated non-cluster statistics manager cache.");
      return;
    }

    try {
      clusterContainerService.createCache(FLOW_STATISTICS_CACHE,
          EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL));
      clusterContainerService.createCache("statisticsmanager.nodeConnectorStatistics",
          EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL));
      clusterContainerService.createCache("statisticsmanager.tableStatistics",
          EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL));
      clusterContainerService.createCache("statisticsmanager.descriptionStatistics",
          EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL));
      clusterContainerService.createCache(TRIGGERS_CACHE,
          EnumSet.of(IClusterServices.cacheMode.NON_TRANSACTIONAL, IClusterServices.cacheMode.ASYNC));
    } catch (CacheConfigException cce) {
      log.error("Statistics cache configuration invalid - check cache mode");
    } catch (CacheExistException ce) {
      log.debug("Skipping statistics cache creation - already present");
    }
  }
  @SuppressWarnings({ "unchecked" })
    private void retrieveCaches() {
      ConcurrentMap<?, ?> map;

      if (this.clusterContainerService == null) {
        log.warn("Can't retrieve statistics manager cache, Clustering service unavailable.");
        return;
      }

      log.debug("Statistics Manager - retrieveCaches for Container {}", container);

      map = clusterContainerService.getCache(FLOW_STATISTICS_CACHE);
      if (map != null) {
        this.flowStatistics = (ConcurrentMap<Node, List<FlowOnNode>>) map;
      } else {
        log.error("Cache allocation failed for statisticsmanager.flowStatistics in container {}", container.getName());
      }

      map = clusterContainerService.getCache("statisticsmanager.nodeConnectorStatistics");
      if (map != null) {
        this.nodeConnectorStatistics = (ConcurrentMap<Node, List<NodeConnectorStatistics>>) map;
      } else {
        log.error("Cache allocation failed for statisticsmanager.nodeConnectorStatistics in container {}", container.getName());
      }

      map = clusterContainerService.getCache("statisticsmanager.tableStatistics");
      if (map != null) {
        this.tableStatistics = (ConcurrentMap<Node, List<NodeTableStatistics>>) map;
      } else {
        log.error("Cache allocation failed for statisticsmanager.tableStatistics in container {}", container.getName());
      }

      map = clusterContainerService.getCache("statisticsmanager.descriptionStatistics");
      if (map != null) {
        this.descriptionStatistics = (ConcurrentMap<Node, NodeDescription>) map;
      } else {
        log.error("Cache allocation failed for statisticsmanager.descriptionStatistics in container {}", container.getName());
      }

      map = clusterContainerService.getCache(TRIGGERS_CACHE);
      if (map != null) {
        this.triggers = (ConcurrentMap<Integer, Node>) map;
      } else {
        log.error("Cache allocation failed for " + TRIGGERS_CACHE +" in container {}", container.getName());
      }
    }

  /**
   * Function called by the dependency manager when all the required
   * dependencies are satisfied
   *
   */
  void init() {
    log.debug("INIT called!");
    allocateCaches();
    retrieveCaches();

  }

  /**
   * Function called by the dependency manager when at least one
   * dependency become unsatisfied or when the component is shutting
   * down because for example bundle is being stopped.
   *
   */
  void destroy() {
    log.debug("DESTROY called!");
  }

  /**
   * Function called by dependency manager after "init ()" is called
   * and after the services provided by the class are registered in
   * the service registry
   *
   */
  void start() {
    log.debug("START called!");
    this.triggerExecutor = Executors.newSingleThreadExecutor();
  }

  /**
   * Function called after registering the service in OSGi service registry.
   */
  void started(){
    // Retrieve current statistics so we don't have to wait for next refresh
    ISwitchManager switchManager = (ISwitchManager) ServiceHelper.getInstance(
        ISwitchManager.class, container.getName(), this);
    if ((reader != null) && (switchManager != null)) {
      Set<Node> nodeSet = switchManager.getNodes();
      for (Node node : nodeSet) {
        List<FlowOnNode> flows = reader.readAllFlows(node);
        if (flows != null) {
          flowStatistics.put(node, flows);
        }
        NodeDescription descr = reader.readDescription(node);
        if (descr != null) {
          descriptionStatistics.put(node, descr);
        }
        List<NodeTableStatistics> tableStats = reader.readNodeTable(node);
        if (tableStats != null) {
          tableStatistics.put(node, tableStats);
        }
        List<NodeConnectorStatistics> ncStats = reader.readNodeConnectors(node);
        if (ncStats != null) {
          nodeConnectorStatistics.put(node, ncStats);
        }
      }

    } else {
      log.trace("Failed to retrieve current statistics. Statistics will not be immediately available!");
    }
  }

  /**
   * Function called by the dependency manager before the services
   * exported by the component are unregistered, this will be
   * followed by a "destroy ()" calls
   *
   */
  void stop() {
    log.debug("STOP called!");
    this.triggerExecutor.shutdownNow();
  }

  void setClusterContainerService(IClusterContainerServices s) {
    log.debug("Cluster Service set for Statistics Mgr");
    this.clusterContainerService = s;
  }

  void unsetClusterContainerService(IClusterContainerServices s) {
    if (this.clusterContainerService == s) {
      log.debug("Cluster Service removed for Statistics Mgr!");
      this.clusterContainerService = null;
    }
  }
  void setIContainer(IContainer c){
    container = c;
  }
  public void unsetIContainer(IContainer s) {
    if (this.container == s) {
      this.container = null;
    }
  }

  public void setReaderService(IReadService service) {
    log.debug("Got inventory service set request {}", service);
    this.reader = service;
  }

  public void unsetReaderService(IReadService service) {
    log.debug("Got a service UNset request {}", service);
    this.reader = null;
  }

  @Override
    public List<FlowOnNode> getFlows(Node node) {
      if (node == null) {
        return Collections.emptyList();
      }else{
        return create_flow();
      }
    }
  /**
   * {@inheritDoc}
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
   * {@THis function will create flow and return to the fuction}
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

