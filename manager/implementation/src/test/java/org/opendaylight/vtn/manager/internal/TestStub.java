/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import javax.transaction.HeuristicMixedException;
import javax.transaction.HeuristicRollbackException;
import javax.transaction.NotSupportedException;
import javax.transaction.RollbackException;
import javax.transaction.SystemException;
import javax.transaction.Transaction;

import org.junit.Assert;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.IClusterServices.cacheMode;
import org.opendaylight.controller.connectionmanager.ConnectionMgmtScheme;
import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowConfig;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.forwardingrulesmanager.IForwardingRulesManager;
import org.opendaylight.controller.forwardingrulesmanager.PortGroupConfig;
import org.opendaylight.controller.forwardingrulesmanager.PortGroupProvider;
import org.opendaylight.controller.hosttracker.HostIdFactory;
import org.opendaylight.controller.hosttracker.IHostId;
import org.opendaylight.controller.hosttracker.IPHostId;
import org.opendaylight.controller.hosttracker.IPMacHostId;
import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.connection.ConnectionConstants;
import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Host;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IDataPacketService;
import org.opendaylight.controller.sal.packet.LinkEncap;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.routing.IRouting;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.switchmanager.SpanConfig;
import org.opendaylight.controller.switchmanager.Subnet;
import org.opendaylight.controller.switchmanager.SubnetConfig;
import org.opendaylight.controller.switchmanager.Switch;
import org.opendaylight.controller.switchmanager.SwitchConfig;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.controller.topologymanager.TopologyUserLinkConfig;

/**
 * Stub module for Unit test of VTNManager.
 * This stub provides APIs implemented in Bundle in controller project.
 *
 * <p>
 *   Note that stubMode can be set to 0 or 2 or 3 only.
 *   (other is not implemented yet.)
 * </p>
 */
public class TestStub implements IClusterGlobalServices, IClusterContainerServices,
    ISwitchManager, ITopologyManager, IDataPacketService, IRouting,
    IForwardingRulesManager, IfIptoHost, IConnectionManager {

    /**
     * The name of the cluster cache which keeps revision identifier of
     * configuration per mapping type.
     */
    private static final String CACHE_CONFREVISION = "vtn.confrev";

    /**
     * Active cluster cache transactions per thread.
     */
    private static final ThreadLocal<TestTransaction>  CLUSTER_TRANSACTION =
        new ThreadLocal<TestTransaction>();

    /**
     * Mode for cluster cache transaction test.
     */
    private TestTransaction.Mode  transactionTestMode = null;

    /*
     * Mode of {@link TestStub}.
     * Each mode corresponds to following configuration.
     *
     *   0 : no nodes
     *   1 : 1 node (note: not implemented yet)
     *   2 : 2 nodes and each node have 6 nodeconnectors
     *       ("port-15" is used to connect nodes).
     *   3 : 3 nodes and each node have 7 nodeconnectors
     *       (node0 and node1 connect with port-15, node0 and node2 connect with port-16)
     */
    private int stubMode = 0;

    /**
     * Set of existing node.
     */
    private Set<Node> nodes = null;

    /**
     * Map between Node and NodeConnector.
     */
    private ConcurrentMap<Node, Set<NodeConnector>> nodeConnectors = null;

    /**
     * Map between a name of NodeConnector and NodeConnector in each node.
     */
    private ConcurrentMap<Node, Map<String, NodeConnector>> nodeConnectorNames = null;

    /**
     * Edges exist in between each nodes.
     */
    private ConcurrentMap<Node, Map<Node, List<Edge>>> nodeEdges = null;

    /**
     * Map between NodeConnector and NodeConnector properties.
     */
    private ConcurrentMap<NodeConnector, Map<String, Property>> nodeConnectorProps = null;

    /**
     * Map between NodeConnector and property of internal node connector.
     */
    private ConcurrentMap<NodeConnector, Set<Property>> nodeConnectorsISL = null;

    /**
     * List of data transmitted from controller.
     */
    private List<RawPacket> transmittedData = null;

    /**
     * saved Subnet object
     */
    private Subnet savedSubnet = null;

    /**
     * saved SubnetConfig object.
     */
    private SubnetConfig savedSubnetConfig = null;

    /**
     * Installed flow entries.
     */
    private HashSet<FlowEntry>  flowEntries;

    /**
     * List of InetAddress of cluster nodes
     */
    List<InetAddress> nodeInetAddresses = null;

    /**
     * Constructor of TestStub
     */
    public TestStub() {
        stubMode = 0;
    }

    /**
     * Constructor of TestStub
     * @param mode  stubMode
     */
    public TestStub(int mode) {
        stubMode = mode;
        setup();
    }

    /**
     * setup datas.
     */
    private void setup() {
        transmittedData = new ArrayList<RawPacket>();
        flowEntries = new HashSet<FlowEntry>();
        if (stubMode >= 2) {
            // create nodes
            nodes = new HashSet<Node>();
            nodeConnectors = new ConcurrentHashMap<Node, Set<NodeConnector>>();
            nodeConnectorNames = new ConcurrentHashMap<Node, Map<String, NodeConnector>>();
            nodeConnectorProps = new ConcurrentHashMap<NodeConnector, Map<String, Property>>();

            for (short i = 0; i < stubMode; i++) {
                Node node = NodeCreator.createOFNode(Long.valueOf(i));
                nodes.add(node);

                // create nodeconnectors
                short ncnum = (short)(4 + stubMode);

                for (Node addnode : nodes) {
                    Map<String, NodeConnector> map = new HashMap<String, NodeConnector>();
                    Set<NodeConnector> ncs = new HashSet<NodeConnector>();
                    for (short inc = 10; inc < ((short)10 + ncnum); inc++) {
                        NodeConnector nc =
                                NodeConnectorCreator.createOFNodeConnector(Short.valueOf(inc), addnode);
                        ncs.add(nc);
                        String mapname = "port-" + inc;
                        map.put(mapname, nc);

                        Map<String, Property> propmap = nodeConnectorProps.get(nc);
                        if (propmap == null) {
                            propmap = new HashMap<String, Property>();
                        }
                        Name nm = new Name(mapname);
                        propmap.put(Name.NamePropName, nm);
                        Config cf = new Config(Config.ADMIN_UP);
                        propmap.put(Config.ConfigPropName, cf);
                        State st = new State(State.EDGE_UP);
                        propmap.put(State.StatePropName, st);

                        nodeConnectorProps.put(nc, propmap);
                    }
                    nodeConnectors.put(addnode, ncs);
                    nodeConnectorNames.put(addnode, map);
                }
            }

            // create edges
            nodeEdges = new ConcurrentHashMap<Node, Map<Node, List<Edge>>>();
            nodeConnectorsISL = new ConcurrentHashMap<NodeConnector, Set<Property>>();
            Node node0 = NodeCreator.createOFNode(Long.valueOf(0));
            for (Node srcnode: nodes) {
                for (Node dstnode: nodes) {
                    if (srcnode.equals(dstnode)) {
                        continue;
                    }
                    if (stubMode == 3 &&
                        ((srcnode.getID().equals(Long.valueOf(1)) &&
                          dstnode.getID().equals(Long.valueOf(2))) ||
                         (srcnode.getID().equals(Long.valueOf(2)) &&
                          dstnode.getID().equals(Long.valueOf(1))))
                       ) {
                        continue;
                    }

                    Map<Node, List<Edge>> map = nodeEdges.get(srcnode);
                    List<Edge> edglist = null;
                    if (map == null) {
                        map = new HashMap<Node, List<Edge>>();
                    } else {
                        edglist = map.get(dstnode);
                    }
                    if (edglist == null) {
                        edglist = new ArrayList<Edge>();
                    }

                    NodeConnector tail = null;
                    NodeConnector head = null;
                    if (((srcnode.getID().equals(Long.valueOf(0)) &&
                          dstnode.getID().equals(Long.valueOf(1))) ||
                         (srcnode.getID().equals(Long.valueOf(1)) &&
                          dstnode.getID().equals(Long.valueOf(0))))) {
                        tail = nodeConnectorNames.get(srcnode).get("port-15");
                        head = nodeConnectorNames.get(dstnode).get("port-15");
                        Edge edge = null;
                        try {
                            edge = new Edge(tail, head);
                        } catch (ConstructionException e) {
                            Assert.fail("Bad edge: tail=" + tail + ", head =" +
                                        head);
                        }
                        edglist.add(edge);
                    } else if (stubMode == 3 &&
                            ((srcnode.getID().equals(Long.valueOf(0)) &&
                              dstnode.getID().equals(Long.valueOf(2))) ||
                             (srcnode.getID().equals(Long.valueOf(2)) &&
                              dstnode.getID().equals(Long.valueOf(0))))) {
                        tail = nodeConnectorNames.get(srcnode).get("port-16");
                        head = nodeConnectorNames.get(dstnode).get("port-16");
                        Edge edge = null;
                        try {
                            edge = new Edge(tail, head);
                        } catch (ConstructionException e) {
                            Assert.fail("Bad edge: tail=" + tail + ", head =" +
                                        head);
                        }
                        edglist.add(edge);
                    } else if (stubMode == 3 &&
                               (srcnode.getID().equals(Long.valueOf(1)) &&
                                 dstnode.getID().equals(Long.valueOf(2)))) {
                        tail = nodeConnectorNames.get(srcnode).get("port-15");
                        head = nodeConnectorNames.get(node0).get("port-15");
                        Edge edge = null;
                        try {
                            edge = new Edge(tail, head);
                        } catch (ConstructionException e) {
                            Assert.fail("Bad edge: tail=" + tail + ", head =" +
                                        head);
                        }
                        edglist.add(edge);

                        tail = nodeConnectorNames.get(node0).get("port-16");
                        head = nodeConnectorNames.get(dstnode).get("port-16");
                        try {
                            edge = new Edge(tail, head);
                        } catch (ConstructionException e) {
                            Assert.fail("Bad edge: tail=" + tail + ", head =" +
                                        head);
                        }
                        edglist.add(edge);
                    } else if (stubMode == 3 &&
                               (srcnode.getID().equals(Long.valueOf(2)) &&
                                dstnode.getID().equals(Long.valueOf(1)))) {
                           tail = nodeConnectorNames.get(srcnode).get("port-16");
                           head = nodeConnectorNames.get(node0).get("port-16");
                           Edge edge = null;
                           try {
                               edge = new Edge(tail, head);
                           } catch (ConstructionException e) {
                               Assert.fail("Bad edge: tail=" + tail +
                                           ", head =" + head);
                           }
                           edglist.add(edge);

                           tail = nodeConnectorNames.get(node0).get("port-15");
                           head = nodeConnectorNames.get(dstnode).get("port-15");
                           try {
                               edge = new Edge(tail, head);
                           } catch (ConstructionException e) {
                               Assert.fail("Bad edge: tail=" + tail +
                                           ", head =" + head);
                           }
                           edglist.add(edge);
                    }
                    map.put(dstnode, edglist);
                    nodeEdges.put(srcnode, map);

                    nodeConnectorsISL.put(tail, new HashSet<Property>(1));
                    nodeConnectorsISL.put(head, new HashSet<Property>(1));
                }
            }
        }
    }

    private Set<Node> getNodeSet(Set<Node> nodeSet) {
        if (nodeSet == null) {
            return new HashSet<Node>();
        }
        return new HashSet<Node>(nodeSet);
    }

    private Set<NodeConnector> getPortSet(Set<NodeConnector> ncSet) {
        if (ncSet == null) {
            return null;
        }
        return new HashSet<NodeConnector>(ncSet);
    }

    private Map<String, Property> getProperty(Map<String, Property> prop) {
        if (prop == null) {
            return new HashMap<String, Property>();
        }
        return new HashMap<String, Property>(prop);
    }

    /**
     * All cluster caches.
     */
    private ConcurrentMap<String, ConcurrentMap<?, ?>> caches =
        new ConcurrentHashMap<String, ConcurrentMap<?, ?>>();

    // IClusterGlobalServices, IClusterContainerServices

    @Override
    public void removeContainerCaches(String containerName) {

    }

    @Override
    public ConcurrentMap<?, ?> createCache(String cacheName, Set<cacheMode> cMode) throws CacheExistException,
            CacheConfigException {

        ConcurrentMap<?, ?> res = this.caches.get(cacheName);
        if (res == null) {
            res = (CACHE_CONFREVISION.equals(cacheName))
                ? new ConfRevisionMap()
                : new ConcurrentHashMap<Object, Object>();
            this.caches.put(cacheName, res);
            return res;
        }
        throw new CacheExistException();
    }

    @Override
    public ConcurrentMap<?, ?> getCache(String cacheName) {
        return this.caches.get(cacheName);
    }

    @Override
    public void destroyCache(String cacheName) {
        this.caches.remove(cacheName);
    }

    @Override
    public boolean existCache(String cacheName) {
        return (this.caches.get(cacheName) != null);
    }

    @Override
    public Set<String> getCacheList() {
        return this.caches.keySet();
    }

    @Override
    public Properties getCacheProperties(String cacheName) {
        return null;
    }

    @Override
    public void tbegin() throws NotSupportedException, SystemException {
        tbegin(0, TimeUnit.SECONDS);
    }

    @Override
    public void tbegin(long timeout, TimeUnit unit)
        throws NotSupportedException, SystemException {
        TestTransaction t = CLUSTER_TRANSACTION.get();
        if (t != null) {
            throw new SystemException("Cache transaction is already active.");
        }

        t = new TestTransaction(transactionTestMode, timeout, unit, caches);
        CLUSTER_TRANSACTION.set(t);

    }

    @Override
    public void tcommit() throws RollbackException, HeuristicMixedException, HeuristicRollbackException,
            SecurityException, IllegalStateException, SystemException {
        TestTransaction t = CLUSTER_TRANSACTION.get();
        if (t == null) {
            throw new SystemException("Cache transaction is not active.");
        }

        t.commit();
        CLUSTER_TRANSACTION.remove();
    }

    @Override
    public void trollback() throws IllegalStateException, SecurityException, SystemException {
        TestTransaction t = CLUSTER_TRANSACTION.get();
        if (t == null) {
            throw new SystemException("Cache transaction is not active.");
        }

        t.rollback();
        t.restore(caches);
        CLUSTER_TRANSACTION.remove();
    }

    @Override
    public Transaction tgetTransaction() {
        return CLUSTER_TRANSACTION.get();
    }

    @Override
    public InetAddress getCoordinatorAddress() {
        return null;
    }

    @Override
    public List<InetAddress> getClusteredControllers() {
        List<InetAddress> list = new ArrayList<InetAddress>();
        InetAddress loopbackAddress = InetAddress.getLoopbackAddress();
        list.add(loopbackAddress);
        return list;
    }

    @Override
    public InetAddress getMyAddress() {
        return null;
    }

    @Override
    public boolean amICoordinator() {
      return true;
    }

    // ISwitchManager
    @Override
    public Status addSubnet(SubnetConfig configObject) {
        savedSubnetConfig = configObject;
        return null;
    }

    @Override
    public Status removeSubnet(SubnetConfig configObject) {
        savedSubnet = null;
        savedSubnetConfig = null;
        return null;
    }

    @Override
    public Status modifySubnet(SubnetConfig configObject) {
        return null;
    }

    @Override
    public Status removeSubnet(String name) {
        savedSubnet = null;
        savedSubnetConfig = null;
        return null;
    }

    @Override
    public List<Switch> getNetworkDevices() {
       return null;
    }

    @Override
    public Set<Switch> getConfiguredNotConnectedSwitches() {
        return null;
    }

    @Override
    public List<SubnetConfig> getSubnetsConfigList() {
        return null;
    }

    @Override
    public SubnetConfig getSubnetConfig(String subnet) {
        return null;
    }

    @Override
    public Subnet getSubnetByNetworkAddress(InetAddress networkAddress) {
        if (savedSubnetConfig != null) {
            if (savedSubnet == null) {
                savedSubnet = new Subnet(savedSubnetConfig);
            }
            return savedSubnet;
        }

        return null;
    }

    @Override
    public Status saveSwitchConfig() {
        return null;
    }

    @Override
    public Status addSpanConfig(SpanConfig configObject) {
        return null;
    }

    @Override
    public Status removeSpanConfig(SpanConfig cfgObject) {
        return null;
    }

    @Override
    public List<SpanConfig> getSpanConfigList() {
        return null;
    }

    @Override
    public List<NodeConnector> getSpanPorts(Node node) {
        return null;
    }

    @Override
    public void updateSwitchConfig(SwitchConfig cfgObject) {

    }

    @Override
    public SwitchConfig getSwitchConfig(String nodeId) {
        return null;
    }

    @Override
    public Status addPortsToSubnet(String name, List<String> nodeConnectors) {
        return null;
    }

    @Override
    public Status removePortsFromSubnet(String name, List<String> nodeConnectors) {
        return null;
    }

    @Override
    public Set<Node> getNodes() {
        if (stubMode >= 1) {
            return getNodeSet(nodes);
        }

        return getNodeSet(null);
    }

    @Override
    public Map<String, Property> getNodeProps(Node node) {
        return null;
    }

    @Override
    public Property getNodeProp(Node node, String propName) {
        return null;
    }

    @Override
    public void setNodeProp(Node node, Property prop) {

    }

    @Override
    public Status removeNodeProp(Node node, String propName) {
        return null;
    }

    @Override
    public Status removeNodeAllProps(Node node) {
        return null;
    }

    @Override
    public Set<NodeConnector> getUpNodeConnectors(Node node) {
        if (stubMode >= 1) {
            return getPortSet(nodeConnectors.get(node));
        }
        return getPortSet(null);
    }

    @Override
    public Set<NodeConnector> getNodeConnectors(Node node) {
        if (stubMode >= 1) {
            return getPortSet(nodeConnectors.get(node));
        }
        return getPortSet(null);
    }

    @Override
    public Set<NodeConnector> getPhysicalNodeConnectors(Node node) {
        if (stubMode >= 1) {
            return getPortSet(nodeConnectors.get(node));
        }
        return getPortSet(null);
    }

    @Override
    public Map<String, Property> getNodeConnectorProps(NodeConnector nodeConnector) {
        if (stubMode >= 1) {
            return getProperty(nodeConnectorProps.get(nodeConnector));
        }
        return getProperty(null);
    }

    @Override
    public Property getNodeConnectorProp(NodeConnector nodeConnector, String propName) {
        if (stubMode >= 1) {
            Map<String, Property> map = nodeConnectorProps.get(nodeConnector);
            Property name = null;
            if (map != null) {
                name = map.get(propName);
            }
            return name;
        }
        return null;
    }

    @Override
    public Status addNodeConnectorProp(NodeConnector nodeConnector, Property prop) {
        return null;
    }

    @Override
    public Status removeNodeConnectorProp(NodeConnector nc, String propName) {
        return null;
    }

    @Override
    public Status removeNodeConnectorAllProps(NodeConnector nodeConnector) {
        return null;
    }

    @Override
    public NodeConnector getNodeConnector(Node node, String nodeConnectorName) {
        if (stubMode >= 1) {
            Map<String, NodeConnector> map = nodeConnectorNames.get(node);
            return map.get(nodeConnectorName);
        }
        return null;
    }

    @Override
    public boolean isSpecial(NodeConnector p) {
        if (stubMode >= 1) {
            if (p.getType().equals(NodeConnectorIDType.CONTROLLER)
                || p.getType().equals(NodeConnectorIDType.ALL)
                || p.getType().equals(NodeConnectorIDType.SWSTACK)
                || p.getType().equals(NodeConnectorIDType.HWPATH)) {
                return true;
            }
            return false;
        }
        return false;
    }

    @Override
    public Boolean isNodeConnectorEnabled(NodeConnector nodeConnector) {
        if (stubMode >= 1) {
            return true;
        }
        return null;
    }

    @Override
    public boolean doesNodeConnectorExist(NodeConnector nc) {
        return false;
    }

    @Override
    public byte[] getControllerMAC() {
        return new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                            (byte)0xCC, (byte)0xCC, (byte)0xCC};
    }

    @Override
    public byte[] getNodeMAC(Node node) {
        return null;
    }

    @Override
    public Property createProperty(String propName, String propValue) {
        return null;
    }

    @Override
    public String getNodeDescription(Node node) {
        return null;
    }

    @Override
    public Map<String, Property> getControllerProperties() {
        return null;
    }

    @Override
    public Property getControllerProperty(String propertyName) {
        return null;
    }

    @Override
    public Status setControllerProperty(Property property) {
        return null;
    }

    @Override
    public Status removeControllerProperty(String propertyName) {
        return null;
    }

    @Override
    public Status updateNodeConfig(SwitchConfig switchConfig) {
        return null;
    }

    @Override
    public Status removeNodeConfig(String nodeId) {
        return null;
    }

    // ITopologyManager
    @Override
    public boolean isInternal(NodeConnector p) {
        if (stubMode >= 1) {
            return (nodeConnectorsISL.get(p) != null);
        }
        return false;
    }

    @Override
    public Map<Edge, Set<Property>> getEdges() {
        if (stubMode >= 1) {
            Map<Edge, Set<Property>> edges = new HashMap<Edge, Set<Property>>();

            for (Node srcNode : nodeEdges.keySet()) {
                Map<Node, List<Edge>> srcmap = nodeEdges.get(srcNode);
                for (Node dstNode : srcmap.keySet()) {
                    for (Edge edge : srcmap.get(dstNode)) {
                        edges.put(edge, new HashSet<Property>(1));
                    }
                }
            }
            return edges;
        }

        return null;
    }

    @Override
    public Map<Node, Set<Edge>> getNodeEdges() {
        return null;
    }

    @Override
    public void updateHostLink(NodeConnector p, Host h, UpdateType t, Set<Property> props) {

    }

    @Override
    public Set<NodeConnector> getNodeConnectorWithHost() {
        return null;
    }

    @Override
    public Host getHostAttachedToNodeConnector(NodeConnector p) {
        return null;
    }

    @Override
    public Map<Node, Set<NodeConnector>> getNodesWithNodeConnectorHost() {
        return null;
    }

    @Override
    public Status addUserLink(TopologyUserLinkConfig link) {
        return null;
    }

    @Override
    public Status deleteUserLink(String linkName) {
        return null;
    }

    @Override
    public Status saveConfig() {
        return null;
    }

    @Override
    public ConcurrentMap<String, TopologyUserLinkConfig> getUserLinks() {
        return null;
    }

    @Override
    public List<Host> getHostsAttachedToNodeConnector(NodeConnector p) {
        return null;
    }

    // IDataPacketService
    @Override
    public void transmitDataPacket(RawPacket outPkt) {
        transmittedData.add(outPkt);
    }

    @Override
    public Packet decodeDataPacket(RawPacket pkt) {
        if (pkt == null) {
            return null;
        }
        byte[] data = pkt.getPacketData();
        if (data.length <= 0) {
            return null;
        }
        if (pkt.getEncap().equals(LinkEncap.ETHERNET)) {
            Ethernet res = new Ethernet();
            try {
                res.deserialize(data, 0, data.length * NetUtils.NumBitsInAByte);
            } catch (Exception e) {
                Assert.fail("deserialize failed: " + e);
                return null;
            }
            return res;
        }
        return null;
    }

    @Override
    public RawPacket encodeDataPacket(Packet pkt) {
        if (pkt == null) {
            return null;
        }
        byte[] data;
        try {
            data = pkt.serialize();
        } catch (Exception e) {
            return null;
        }
        if (data.length <= 0) {
            return null;
        }
        try {
            RawPacket res = new RawPacket(data);
            return res;
        } catch (ConstructionException cex) {
        }

        return null;
    }

    // IRouting

    @Override
    public Path getRoute(Node src, Node dst) {
        if (stubMode >= 1) {
            Map<Node, List<Edge>> nodeEdgeMap = nodeEdges.get(src);
            if (nodeEdgeMap == null) {
                return null;
            }
            List<Edge> edges = nodeEdgeMap.get(dst);
            if (edges == null) {
                return null;
            }
            Path result = null;
            try {
                result = new Path(edges);
            } catch (ConstructionException e) {
                result = null;
            }
            return result;
        }

        return null;
    }

    @Override
    public Path getMaxThroughputRoute(Node src, Node dst) {
        return null;
    }

    @Override
    public Path getRoute(Node src, Node dst, Short Bw) {
        return null;
    }

    @Override
    public void clear() {

    }

    @Override
    public void clearMaxThroughput() {

    }

    @Override
    public void initMaxThroughput(Map<Edge, Number> EdgeWeightMap) {

    }


    // IForwardingRulesManager

    @Override
    public synchronized Status installFlowEntry(FlowEntry flow) {
        if (flow == null) {
            return new Status(StatusCode.NOTACCEPTABLE, null);
        }
        if (flowEntries.add(flow)) {
            return new Status(StatusCode.SUCCESS, null);
        }
        return new Status(StatusCode.CONFLICT, "Already installed");
    }

    @Override
    public synchronized Status uninstallFlowEntry(FlowEntry flow) {
        if (flow == null) {
            return new Status(StatusCode.NOTACCEPTABLE, null);
        }
        if (flowEntries.remove(flow)) {
            return new Status(StatusCode.SUCCESS, null);
        }
        return new Status(null, null);
    }

    @Override
    public Status uninstallFlowEntryGroup(String groupName) {
        return null;
    }

    @Override
    public Status modifyFlowEntry(FlowEntry current, FlowEntry newone) {
        return null;
    }

    @Override
    public Status modifyOrAddFlowEntry(FlowEntry newone) {
        return null;
    }

    @Override
    public Status installFlowEntryAsync(FlowEntry flow) {
        return null;
    }

    @Override
    public Status uninstallFlowEntryAsync(FlowEntry flow) {
        return null;
    }

    @Override
    public Status uninstallFlowEntryGroupAsync(String groupName) {
        return null;
    }

    @Override
    public Status modifyFlowEntryAsync(FlowEntry current, FlowEntry newone) {
        return null;
    }

    @Override
    public Status modifyOrAddFlowEntryAsync(FlowEntry newone) {
        return null;
    }

    @Override
    public Status solicitStatusResponse(Node node, boolean blocking) {
        return null;
    }

    @Override
    public boolean checkFlowEntryConflict(FlowEntry flow) {
        return false;
    }

    @Override
    public List<FlowEntry> getInstalledFlowEntriesForGroup(String policyName) {
        return null;
    }

    @Override
    public List<FlowEntry> getFlowEntriesForGroup(String group) {
        return null;
    }

    @Override
    public void addOutputPort(Node node, String flowName, List<NodeConnector> dstPort) {

    }

    @Override
    public void removeOutputPort(Node node, String flowName, List<NodeConnector> dstPort) {

    }

    @Override
    public void replaceOutputPort(Node node, String flowName, NodeConnector outPort) {

    }

    @Override
    public NodeConnector getOutputPort(Node node, String flowName) {
        return null;
    }

    @Override
    public Map<String, Object> getTSPolicyData() {
        return null;
    }

    @Override
    public void setTSPolicyData(String policyName, Object o, boolean add) {

    }

    @Override
    public Object getTSPolicyData(String policyName) {
        return null;
    }

    @Override
    public List<FlowConfig> getStaticFlows() {
        return null;
    }

    @Override
    public List<FlowConfig> getStaticFlows(Node node) {
        return null;
    }

    @Override
    public FlowConfig getStaticFlow(String name, Node n) {
        return null;
    }

    @Override
    public List<String> getStaticFlowNamesForNode(Node node) {
        return null;
    }

    @Override
    public List<Node> getListNodeWithConfiguredFlows() {
        return null;
    }

    @Override
    public Status addStaticFlow(FlowConfig config) {
        return null;
    }

    @Override
    public Status removeStaticFlow(FlowConfig config) {
        return null;
    }

    @Override
    public Status modifyStaticFlow(FlowConfig config) {
        return null;
    }

    @Override
    public Status removeStaticFlow(String name, Node node) {
        return null;
    }

    @Override
    public Status toggleStaticFlowStatus(FlowConfig configObject) {
        return null;
    }

    @Override
    public Status toggleStaticFlowStatus(String name, Node node) {
        return null;
    }

    @Override
    public Map<String, PortGroupConfig> getPortGroupConfigs() {
        return null;
    }

    @Override
    public boolean addPortGroupConfig(String name, String regex, boolean load) {
        return false;
    }

    @Override
    public boolean delPortGroupConfig(String name) {
        return false;
    }

    @Override
    public PortGroupProvider getPortGroupProvider() {
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

    // IfIptoHost

    @Override
    public HostNodeConnector hostFind(IHostId id) {
        return null;
    }

    @Override
    public HostNodeConnector hostFind(InetAddress networkAddress) {
        return null;
    }

    @Override
    public HostNodeConnector hostQuery(IHostId id) {
        if (stubMode >= 1) {
            HostNodeConnector hnode = null;
            InetAddress networkAddress = getIpAddress(id);
            if (networkAddress == null) {
                return null;
            }
            byte [] tgt = networkAddress.getAddress();
            byte [] ip = new byte[] {(byte)192, (byte)168, (byte)0, (byte)251};
            if (Arrays.equals(tgt, ip)) {

                byte [] mac = new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
                Node node = NodeCreator.createOFNode(Long.valueOf(0));
                NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node);
                try {
                    hnode = new HostNodeConnector(mac, networkAddress, nc, (short) 0);
                } catch (ConstructionException e) {
                    return null;
                }

            }
            return  hnode;
        }
        return null;
    }

    @Override
    public HostNodeConnector hostQuery(InetAddress networkAddress) {
        IHostId id = HostIdFactory.create(networkAddress, null);
        return hostQuery(id);
    }

    @Override
    public Future<HostNodeConnector> discoverHost(IHostId id) {
        return null;
    }

    @Override
    public Future<HostNodeConnector> discoverHost(InetAddress networkAddress) {
        return null;
    }

    @Override
    public List<List<String>> getHostNetworkHierarchy(IHostId id) {
        return null;
    }

    @Override
    public List<List<String>> getHostNetworkHierarchy(InetAddress hostAddress) {
        return null;
    }

    @Override
    public Set<HostNodeConnector> getAllHosts() {
        return null;
    }

    @Override
    public Set<HostNodeConnector> getActiveStaticHosts() {
        return null;
    }

    @Override
    public Set<HostNodeConnector> getInactiveStaticHosts() {
        return null;
    }

    @Override
    public Status addStaticHost(String networkAddress, String dataLayerAddress, NodeConnector nc, String vlan) {
        return null;
    }

    @Override
    public Status removeStaticHost(String networkAddress) {
        return null;
    }

    @Override
    public Status removeStaticHostUsingIPAndMac(String networkAddress,
                                                String macAddress) {
        return null;
    }

    // IConnectionManager

    @Override
    public ConnectionMgmtScheme getActiveScheme() {
        return ConnectionMgmtScheme.ANY_CONTROLLER_ONE_MASTER;
    }

    @Override
    public Set<Node> getNodes(InetAddress controller) {
        return null;
    }

    @Override
    public Set<Node> getLocalNodes() {
        return null;
    }

    @Override
    public boolean isLocal(Node node) {
        return true;
    }

    @Override
    public ConnectionLocality getLocalityStatus(Node node) {
        return ConnectionLocality.LOCAL;
    }

    @Override
    public Status disconnect(Node node) {
        return new Status(StatusCode.SUCCESS, null);
    }

    @Override
    public Node connect (String connectionIdentifier,
                         Map<ConnectionConstants, String> params) {
        return null;
    }

    @Override
    public Node connect(String type, String connectionIdentifier,
                        Map<ConnectionConstants, String> params) {
        return null;
    }

    @Override
    public Set<InetAddress> getControllers(Node node) {
        return null;
    }

    // additional method for control stub
    public void addEdge(Edge edge) {
        Map<Node, List<Edge>> map = nodeEdges.get(edge.getTailNodeConnector().getNode());
        List<Edge> list = map.get(edge.getHeadNodeConnector().getNode());
        list.add(edge);
        map.put(edge.getHeadNodeConnector().getNode(), list);
        nodeEdges.put(edge.getTailNodeConnector().getNode(), map);

        nodeConnectorsISL.put(edge.getTailNodeConnector(), new HashSet<Property>(1));
        nodeConnectorsISL.put(edge.getHeadNodeConnector(), new HashSet<Property>(1));
    }

    public void deleteEdge(Edge edge) {
        Map<Node, List<Edge>> map = nodeEdges.get(edge.getTailNodeConnector().getNode());
        List<Edge> list = map.get(edge.getHeadNodeConnector().getNode());
        list.remove(edge);
        map.put(edge.getHeadNodeConnector().getNode(), list);
        nodeEdges.put(edge.getTailNodeConnector().getNode(), map);

        nodeConnectorsISL.remove(edge.getTailNodeConnector());
        nodeConnectorsISL.remove(edge.getHeadNodeConnector());
    }

    public List<RawPacket> getTransmittedDataPacket() {
        List<RawPacket> ret = new ArrayList<RawPacket>();
        ret.addAll(transmittedData);
        transmittedData.clear();
        return ret;
    }

    /**
     * Return a set of installed flow entries.
     *
     * @return  A set of installed flow entries.
     */
    public synchronized Set<FlowEntry> getFlowEntries() {
        return new HashSet<FlowEntry>(flowEntries);
    }

    /**
     * Return an IP address in the given host ID.
     *
     * @param id  A host ID.
     * @return    An IP address in the given host ID.
     *            {@code null} is returned if not set.
     */
    public InetAddress getIpAddress(IHostId id) {
        if (id instanceof IPHostId) {
            return ((IPHostId)id).getIpAddress();
        }
        if (id instanceof IPMacHostId) {
            return ((IPMacHostId)id).getIpAddress();
        }

        return null;
    }

    /**
     * Set mode for cluster cache transaction test.
     *
     * @param mode  Test mode.
     */
    public void setTransactionTestMode(TestTransaction.Mode mode) {
        transactionTestMode = mode;
    }
}
