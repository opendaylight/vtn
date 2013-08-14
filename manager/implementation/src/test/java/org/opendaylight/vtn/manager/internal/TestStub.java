/*
 * Copyright (c) 2013 NEC Corporation
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

import javax.transaction.HeuristicMixedException;
import javax.transaction.HeuristicRollbackException;
import javax.transaction.NotSupportedException;
import javax.transaction.RollbackException;
import javax.transaction.SystemException;
import javax.transaction.Transaction;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.IClusterServices.cacheMode;
import org.opendaylight.controller.forwardingrulesmanager.FlowConfig;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.forwardingrulesmanager.IForwardingRulesManager;
import org.opendaylight.controller.forwardingrulesmanager.PortGroupConfig;
import org.opendaylight.controller.forwardingrulesmanager.PortGroupProvider;
import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Host;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.core.Property;
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
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.switchmanager.SpanConfig;
import org.opendaylight.controller.switchmanager.Subnet;
import org.opendaylight.controller.switchmanager.SubnetConfig;
import org.opendaylight.controller.switchmanager.Switch;
import org.opendaylight.controller.switchmanager.SwitchConfig;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.controller.topologymanager.TopologyUserLinkConfig;
import org.opendaylight.vtn.manager.SwitchPort;

class TestStub implements IClusterGlobalServices, IClusterContainerServices,
    ISwitchManager, ITopologyManager, IDataPacketService, IRouting, IForwardingRulesManager,
    IfIptoHost {

    /*
     * stub mode
     *   0 : no nodes
     *   1 : 2 nodes and each node have 5 nodeconnectors.
     *   2 :
     */
    private int stubmode = 0;

    private Set<Node> nodes = null;
    private ConcurrentMap<Node, Set<NodeConnector>> nodeConnectors = null;
    private ConcurrentMap<Node, Map<String, NodeConnector>> nodeConnectorNames = null;
    private Set<SwitchPort> ports = null;
    private ConcurrentMap<Node, Map<Node, List<Edge>>> nodeEdges = null;
    private ConcurrentMap<NodeConnector, Map<String, Property>> nodeConnectorProps = null;

    private List<RawPacket> transmittedData = null;

    private SubnetConfig savedSubnetConfig = null;


    /**
     * Constractor of TestStub
     */
    public TestStub() {
        stubmode = 0;
    }

    /**
     * Constractor of TestStub
     * @param mode  stubmode
     */
    public TestStub(int mode) {
        stubmode = mode;
        setup();
    }

    private void setup() {
        transmittedData = new ArrayList<RawPacket>();
        if (stubmode == 1) {
            // create nodes
            nodes = new HashSet<Node>();
            Node node = NodeCreator.createOFNode(Long.valueOf(0));
            nodes.add(node);
            node = NodeCreator.createOFNode(Long.valueOf(1));
            nodes.add(node);

            // create nodeconnectors
            nodeConnectors = new ConcurrentHashMap<Node, Set<NodeConnector>>();
            nodeConnectorNames = new ConcurrentHashMap<Node, Map<String, NodeConnector>>();
            nodeConnectorProps = new ConcurrentHashMap<NodeConnector, Map<String, Property>>();
            for (Node addnode : nodes) {
                Map<String, NodeConnector> map = new HashMap<String, NodeConnector>();
                Set<NodeConnector> ncs = new HashSet<NodeConnector>();
                for (short i = 10; i < 15; i++) {
                    NodeConnector nc =
                        NodeConnectorCreator.createOFNodeConnector(Short.valueOf(i), addnode);
                    ncs.add(nc);
                    String mapname = "port-" + i;
                    map.put(mapname, nc);

                    Map<String, Property> propmap = nodeConnectorProps.get(nc);
                    if (propmap == null) {
                        propmap = new HashMap<String, Property>();
                    }
                    Name prop = new Name(mapname);
                    propmap.put("name", prop);

                    nodeConnectorProps.put(nc, propmap);
                }
                nodeConnectors.put(addnode, ncs);
                nodeConnectorNames.put(addnode, map);
            }

            // create edges
            nodeEdges = new ConcurrentHashMap<Node, Map<Node, List<Edge>>>();
            for (Node srcnode: nodes) {
                for (Node dstnode: nodes) {
                    if(srcnode.equals(dstnode)) {
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

                    NodeConnector tail = nodeConnectorNames.get(dstnode).get("port-15");
                    NodeConnector head = nodeConnectorNames.get(srcnode).get("port-15");
                    Edge edge = null;
                    try {
                        edge = new Edge(tail, head);
                    } catch (ConstructionException e) {
                        edge = null;
                    }
                    edglist.add(edge);
                    map.put(dstnode, edglist);
                    nodeEdges.put(srcnode, map);
                }
            }
        }
    }

    // IClusterGlobalServices, IClusterContainerServices
    private ConcurrentMap<String, ConcurrentMap<?, ?>> caches = new ConcurrentHashMap<String, ConcurrentMap<?, ?>>();

    @Override
    public ConcurrentMap<?, ?> createCache(String cacheName, Set<cacheMode> cMode) throws CacheExistException,
            CacheConfigException {

        ConcurrentMap<?, ?> res = this.caches.get(cacheName);
        if (res == null) {
            res = new ConcurrentHashMap<Object, Object>();
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

    }

    @Override
    public void tcommit() throws RollbackException, HeuristicMixedException, HeuristicRollbackException,
            SecurityException, IllegalStateException, SystemException {

    }

    @Override
    public void trollback() throws IllegalStateException, SecurityException, SystemException {

    }

    @Override
    public Transaction tgetTransaction() throws SystemException {
        return null;
    }

    @Override
    public InetAddress getCoordinatorAddress() {
        return null;
    }

    @Override
    public List<InetAddress> getClusteredControllers() {
        return null;
    }

    @Override
    public InetAddress getMyAddress() {
        return null;
    }

    @Override
    public boolean amICoordinator() {
        return false;
    }

    // ISwitchManager
    @Override
    public Status addSubnet(SubnetConfig configObject) {
        savedSubnetConfig = configObject;
        return null;
    }

    @Override
    public Status removeSubnet(SubnetConfig configObject) {
        return null;
    }

    @Override
    public Status removeSubnet(String name) {
        return null;
    }

    @Override
    public List<Switch> getNetworkDevices() {
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

        if (stubmode == 1) {
            Subnet sub = new Subnet(savedSubnetConfig);
            return sub;
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
    public Status addPortsToSubnet(String name, String nodeConnectors) {
        return null;
    }

    @Override
    public Status removePortsFromSubnet(String name, String nodeConnectors) {
        return null;
    }

    @Override
    public Set<Node> getNodes() {
        if (stubmode == 1) {
            return nodes;
        }

        return null;
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
        if (stubmode == 1) {
            return nodeConnectors.get(node);
        }
        return null;
    }

    @Override
    public Set<NodeConnector> getNodeConnectors(Node node) {
        if (stubmode == 1) {
            return nodeConnectors.get(node);
        }
        return null;
    }

    @Override
    public Set<NodeConnector> getPhysicalNodeConnectors(Node node) {
        if (stubmode == 1) {
            return nodeConnectors.get(node);
        }

        return null;
    }

    @Override
    public Map<String, Property> getNodeConnectorProps(NodeConnector nodeConnector) {
        return null;
    }

    @Override
    public Property getNodeConnectorProp(NodeConnector nodeConnector, String propName) {
        if (stubmode == 1) {
            Map<String, Property> map = nodeConnectorProps.get(nodeConnector);
            Property name = map.get(propName);
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
        if (stubmode == 1) {
            Map<String, NodeConnector> map = nodeConnectorNames.get(node);
            return map.get(nodeConnectorName);
        }
        return null;
    }

    @Override
    public boolean isSpecial(NodeConnector p) {
        if (stubmode == 1) {
            return false;
        }
        return false;
    }

    @Override
    public Boolean isNodeConnectorEnabled(NodeConnector nodeConnector) {
        if (stubmode == 1) {
            return true;
        }
        return null;
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
    public boolean isHostRefreshEnabled() {
        return false;
    }

    @Override
    public int getHostRetryCount() {
        return 0;
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
        if (stubmode == 1) {
            return false;
        }
        return false;
    }

    @Override
    public Map<Edge, Set<Property>> getEdges() {
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
                System.out.println("deserialize failed.");
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
        if (stubmode == 1) {
            List<Edge> edges = nodeEdges.get(src).get(dst);
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
    public Status installFlowEntry(FlowEntry flow) {
        return null;
    }

    @Override
    public Status uninstallFlowEntry(FlowEntry flow) {
        return null;
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

    // IfIptoHost

    @Override
    public HostNodeConnector hostFind(InetAddress networkAddress) {
        return null;
    }

    @Override
    public HostNodeConnector hostQuery(InetAddress networkAddress) {
        if (stubmode == 1) {
            HostNodeConnector hnode = null;
            byte [] tgt = networkAddress.getAddress();
            byte [] ip = new byte[] {(byte)192, (byte)168, (byte)0, (byte)251};
            if (Arrays.equals(tgt, ip)) {

                byte [] mac = new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
                Node node = NodeCreator.createOFNode(Long.valueOf(0));
                NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node);
                try {
                    hnode = new HostNodeConnector(mac, networkAddress, nc, (short)0);
                } catch (ConstructionException e) {
                    return null;
                }

            }
            return  hnode;
        }
        return null;
    }

    @Override
    public Future<HostNodeConnector> discoverHost(InetAddress networkAddress) {
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

    // additional method
    public List<RawPacket> getTransmittedDataPacket() {
        List<RawPacket> ret = new ArrayList<RawPacket>();
        ret.addAll(transmittedData);
        transmittedData.clear();
        return ret;
    }


}
