/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.io.File;
import java.io.FileInputStream;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.UnknownHostException;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Properties;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.DatapathTypeNetdev;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.InterfaceTypeDpdk;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ControllerEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ControllerEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ProtocolEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ProtocolEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ConnectionInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.InterfaceTypeEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.port._interface.attributes.InterfaceExternalIds;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeProtocolBase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeProtocolOpenflow13;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbFailModeBase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbFailModeSecure;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbFailModeStandalone;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ManagedNodeEntry;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.collect.ImmutableBiMap;

/**
 * Handle events from OVSDB Southbound plugin.
 */
public class OVSDBEventHandler  {
    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(OVSDBEventHandler.class);

    /**
     * The name of the configuration directory.
     */
    private static final String  CONFIG_DIR = "configuration";

    /**
     * The name of the configuration file.
     */
    private static final String  VTN_INI_NAME = "vtn.ini";


    /**
     * identifier for the integration bridge name.
     */
    private String integrationBridgeName;

    /**
     * identifier to define failmode of integration bridge.
     */
    private String failmode;

    /**
     * identifier to define protocol of integration bridge.
     */
    private String protocols;

     /**
     * identifier to define portname of the integration bridge.
     */
    private String portname;

    /**
     * Default integration bridge name.
     */
    private static final String DEFAULT_INTEGRATION_BRIDGENAME = "br-int";

    /**
     * identifier for Default failmode.
     */
    private static final String DEFAULT_FAILMODE = "secure";

    /**
     * identifier for Default protocol.
     */
    private static final String DEFAULT_PROTOCOLS = "OpenFlow13";

    /**
     * identifier for Default portname.
     */
    private static final String DEFAULT_PORTNAME = "eth0";

    /**
     * identifier to read for integration bridge name from config file.
     */
    private static final String CONFIG_INTEGRATION_BRIDGENAME = "bridgename";

    /**
     * identifier to read for integration name from config file.
     */
    private static final String EXTERNAL_ID_INTERFACE_ID = "iface-id";

    /**
     * identifier to read failmode from config file.
     */
    private static final String CONFIG_FAILMODE = "failmode";

    /**
     * identifier to read protocol from config file.
     */
    private static final String CONFIG_PROTOCOLS = "protocols";

    /**
     * identifier to openflow port,6653 is official openflow port.
     */
    private static short OPENFLOW_PORT = 6653;

    /**
     * identifier to read open flow protocol from config file.
     */
    public static String OPENFLOW_CONNECTION_PROTOCOL = "tcp";

    /**
     * identifier to read portname from config file.
     */
    private static final String CONFIG_PORTNAME = "portname";

    /**
     * Instance of DataBroker.
     */
    private DataBroker databroker = null;

    /**
     * Instance of mdsalUtils.
     */
    private MdsalUtils mdsalUtils = null;

    /**
     * identifier to read BRIDGE_URI_PREFIX.
     */
    public static final String BRIDGE_URI_PREFIX = "bridge";

    /**
     * Method invoked when the open flow switch is Added.
     * @param dataBroker Instance of DataBroker object to be added.
     */
    public OVSDBEventHandler(DataBroker dataBroker) {
    this.databroker = dataBroker;
    mdsalUtils = new MdsalUtils(dataBroker);
    }

    /**
     * Method invoked when the open flow switch is Added.
     * @param node Instance of Node object to be added.
     * @param resourceAugmentationData DataObject value.
     */
    public void nodeAdded(Node node, DataObject resourceAugmentationData ) {
        LOG.trace("nodeAdded() - {}", node.toString());
        try {
            createInternalNetworkForNeutron(node);
        } catch (Exception e) {
            LOG.warn("Exception occurred while Bridge Creation", e);
        }
    }

    /**
     * Method invoked when the open flow switch is Removed.
     * @param node Instance of Node object to be removed.
     */
    public void nodeRemoved(Node node) {
        LOG.trace("nodeRemoved() - {}", node.toString());
    }

    /**
     * Create InternalNetwork if not already present.
     * @param node Instance of Node object.
     * @throws Exception while Neutron network create.
     */
    public void createInternalNetworkForNeutron(Node node) throws Exception {
        getSystemProperties(node);
        LOG.trace("createInternalNetworkForNeutron() - node ={}, integration bridge ={}", node.toString());

    }

    /**
     * Read the parameters configured in configuration file(vtn.ini).
     * @param node instance of Node
     */
    public void getSystemProperties(Node node) {
        File f = new File(new File(CONFIG_DIR), VTN_INI_NAME);
        FileInputStream in = null;
        Properties prop = new Properties();
        try {
            in = new FileInputStream(f);
            prop.load(in);
            LOG.trace("loaded Integration bridge Configuring details from vtn.ini");
        } catch (Exception e) {
            LOG.debug("Exception occurred while reading SystemProperties", e);
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (Exception e) {
                    LOG.warn("Exception occurred while closing file", e);
                }
                in = null;
            }
        }
        integrationBridgeName =
            prop.getProperty(CONFIG_INTEGRATION_BRIDGENAME,
                             DEFAULT_INTEGRATION_BRIDGENAME);
        failmode = prop.getProperty(CONFIG_FAILMODE, DEFAULT_FAILMODE);
        protocols = prop.getProperty(CONFIG_PROTOCOLS, DEFAULT_PROTOCOLS);
        portname = prop.getProperty(CONFIG_PORTNAME, DEFAULT_PORTNAME);
        LOG.trace("System properties values : {},{},{},{}:",
                  integrationBridgeName, failmode, protocols, portname);
                if (null != integrationBridgeName) {
                   try {
                          if (!isBridgeOnOvsdbNode(node, integrationBridgeName)) {
                             addBridge(node, integrationBridgeName);
                             addPortToBridge(node,integrationBridgeName,portname);
                             } else {
                                LOG.trace("Bridge Already exists in the given Node");
                             }
                   } catch (Exception e) {
                       LOG.warn("Exception occurred while Adding Bridge", e);
                       }
                 }
    }

    /**
     * Read Node from the OVSDB path
     * @param NodeKey is the instance of  nodeKey
     * @param bridgeName
     * @return The {@link InstanceIdentifier} for the manager node.
     */
    private  InstanceIdentifier<Node> createInstanceIdentifier(NodeKey ovsdbNodeKey, String bridgeName) {
       return createInstanceIdentifier(createManagedNodeId(ovsdbNodeKey.getNodeId(), bridgeName));
    }

    /**
     * Create instance identifier
     * @param nodeId is instance of NodeId
     * @return The {@link OvsdbBridgeAugmentation} for the manager node.
     */
    private InstanceIdentifier<Node> createInstanceIdentifier(NodeId nodeId) {
        InstanceIdentifier<Node> nodePath = InstanceIdentifier
                .create(NetworkTopology.class)
                .child(Topology.class, new TopologyKey(new TopologyId(new Uri("ovsdb:1"))))
                .child(Node.class,new NodeKey(nodeId));
        return nodePath;
    }

    /**
     * Get the manager node for this bridge node
     * @param ovsdbNodeId is instance of NodeId
     * @param bridge name
     * @return The {@link OvsdbBridgeAugmentation} for the manager node.
     */
    private  NodeId createManagedNodeId(NodeId ovsdbNodeId, String bridgeName) {
        return new NodeId(ovsdbNodeId.getValue()
                + "/" + BRIDGE_URI_PREFIX + "/" + bridgeName);
    }

    /**
     * Get the connection information of the Node
     * @param ovsdbNode is instance of Node
     * @return connectionInfo.
     */
    private ConnectionInfo getConnectionInfo(Node ovsdbNode) {
        LOG.trace("ConnectionInfo Node {}" ,ovsdbNode);
        ConnectionInfo connectionInfo = null;
        OvsdbNodeAugmentation ovsdbNodeAugmentation = extractOvsdbNode(ovsdbNode);
         LOG.trace("OvsdbNodeAugmentation  Node {}" ,ovsdbNodeAugmentation);
        if (ovsdbNodeAugmentation != null) {
            connectionInfo = ovsdbNodeAugmentation.getConnectionInfo();
             LOG.debug("return connection info  Node {}" ,connectionInfo);
        }
        return connectionInfo;
    }

    /**
     * Add Bridge to the given Node
     * @param ovsdbNode
     * @param bridgeName
     * @return true if the bridge add is successful.
     */
    public boolean addBridge(Node ovsdbNode, String bridgeName) {
        boolean result = false;
        String target = getControllerTarget(ovsdbNode);
        LOG.trace("addBridge: node: {}, bridgeName: {}, target: {}", ovsdbNode, bridgeName, target);
        ConnectionInfo connectionInfo = getConnectionInfo(ovsdbNode);
        if (connectionInfo != null) {
          LOG.debug("Connection info is not null {}", connectionInfo);
          NodeBuilder bridgeNodeBuilder = new NodeBuilder();
          InstanceIdentifier<Node> bridgeIid =
                    this.createInstanceIdentifier(ovsdbNode.getKey(), bridgeName);
          NodeId bridgeNodeId = this.createManagedNodeId(bridgeIid);
          bridgeNodeBuilder.setNodeId(bridgeNodeId);
          OvsdbBridgeAugmentationBuilder ovsdbBridgeAugmentationBuilder = new OvsdbBridgeAugmentationBuilder();
          ovsdbBridgeAugmentationBuilder.setControllerEntry(createControllerEntries(target));
          ovsdbBridgeAugmentationBuilder.setBridgeName(new OvsdbBridgeName(bridgeName));
          ovsdbBridgeAugmentationBuilder.setProtocolEntry(createMdsalProtocols());
          ovsdbBridgeAugmentationBuilder.setFailMode(
                   OVSDB_FAIL_MODE_MAP.inverse().get("secure"));
          this.setManagedByForBridge(ovsdbBridgeAugmentationBuilder, ovsdbNode.getKey());
          bridgeNodeBuilder.addAugmentation(OvsdbBridgeAugmentation.class, ovsdbBridgeAugmentationBuilder.build());
          result = mdsalUtils.put(LogicalDatastoreType.CONFIGURATION, bridgeIid, bridgeNodeBuilder.build());
          LOG.info("Bridge Added: {}", result);
        } else {
            throw new InvalidParameterException("Could not find ConnectionInfo");
        }
        return result;
    }

    /**
     * Returns controller details of the Node.
     * @param  node is instance of Node to be added
     * returns Controller information.
     */
    private String getControllerTarget(Node node) {
        String setControllerStr = null;
        String controllerIpStr = null;
        short openflowPort = OPENFLOW_PORT;
            // Check if ovsdb node has connection info
            OvsdbNodeAugmentation ovsdbNodeAugmentation = this.extractOvsdbNode(node);
            if (ovsdbNodeAugmentation != null) {
                ConnectionInfo connectionInfo = ovsdbNodeAugmentation.getConnectionInfo();
                if (connectionInfo != null && connectionInfo.getLocalIp() != null) {
                    controllerIpStr = new String(connectionInfo.getLocalIp().getValue());
                   } else {
                           LOG.warn("Ovsdb Node does not contains connection info : {}", node);
                }
            }
            if (controllerIpStr != null) {
                LOG.trace("Target OpenFlow Controller found : {}", controllerIpStr);
                setControllerStr = OPENFLOW_CONNECTION_PROTOCOL + ":" + controllerIpStr + ":" + openflowPort;
            } else {
                LOG.warn("Failed to determine OpenFlow controller ip address");
            }
            return setControllerStr;
    }

    /**
     * Return the particular port detail of the given Node.
     * @param the bridgeNode
     * @param portName
     * @return OvsdbTerminationPointAugmentation.
     */
    private OvsdbTerminationPointAugmentation extractTerminationPointAugmentation(Node bridgeNode, String portName) {
        if (bridgeNode.getAugmentation(OvsdbBridgeAugmentation.class) != null) {
            List<OvsdbTerminationPointAugmentation> tpAugmentations = extractTerminationPointAugmentations(bridgeNode);
            for (OvsdbTerminationPointAugmentation ovsdbTerminationPointAugmentation : tpAugmentations) {
                if (ovsdbTerminationPointAugmentation.getName().equals(portName)) {
                    return ovsdbTerminationPointAugmentation;
                }
            }
        }
        return null;
    }

    /**
     * Returns all port details of the given Node.
     * @param the node
     * @return OvsdbTerminationPointAugmentation.
     */
    private List<OvsdbTerminationPointAugmentation> extractTerminationPointAugmentations( Node node ) {
        List<OvsdbTerminationPointAugmentation> tpAugmentations = new ArrayList<OvsdbTerminationPointAugmentation>();
        List<TerminationPoint> terminationPoints = node.getTerminationPoint();
        if (terminationPoints != null && !terminationPoints.isEmpty()) {
            for (TerminationPoint tp : terminationPoints) {
                OvsdbTerminationPointAugmentation ovsdbTerminationPointAugmentation =
                        tp.getAugmentation(OvsdbTerminationPointAugmentation.class);
                if (ovsdbTerminationPointAugmentation != null) {
                    tpAugmentations.add(ovsdbTerminationPointAugmentation);
                }
            }
        }
        return tpAugmentations;
    }

    /**
     * Add a Port to the existing bridge.
     * @params node is instance of Node
     * @params bridgeName
     * @params portName
     * returns true on success
     */
    private boolean addPortToBridge (Node node, String bridgeName, String portName) throws Exception {
        boolean rv = true;

        if (extractTerminationPointAugmentation(node, portName) == null) {
            rv = addTerminationPoint(node, bridgeName, portName);

            if (rv) {
                LOG.trace("addPortToBridge: node: {}, bridge: {}, portname: {} status: success",
                        node.getNodeId().getValue(), bridgeName, portName);
            } else {
                LOG.error("addPortToBridge: node: {}, bridge: {}, portname: {} status: FAILED",
                        node.getNodeId().getValue(), bridgeName, portName);
            }
        } else {
            LOG.trace("addPortToBridge: node: {}, bridge: {}, portname: {} status: not_needed",
                    node.getNodeId().getValue(), bridgeName, portName);
        }

        return rv;
    }

    /**
     * Set the manager node for the node
     * @param ovsdbBridgeAugmentationBuilder is instance of the OvsdbBridgeAugmentationBuilder
     * @param ovsdbNodeKey is instance of NodeKey
     * @return The {@link OvsdbBridgeAugmentation} for the manager node.
     */
    private void setManagedByForBridge(OvsdbBridgeAugmentationBuilder ovsdbBridgeAugmentationBuilder,
                NodeKey ovsdbNodeKey) {
        InstanceIdentifier<Node> connectionNodePath = this.createInstanceIdentifier(ovsdbNodeKey.getNodeId());
        LOG.trace("setManagedByForBridge: connectionNodePath: {}", connectionNodePath);
        ovsdbBridgeAugmentationBuilder.setManagedBy(new OvsdbNodeRef(connectionNodePath));
    }

    /**
     * Add the port to the node, returns true on success
     * @param bridgeNode
     * @param bridgeName
     * @param portName
     * @return true on success.
     */
    private Boolean addTerminationPoint(Node bridgeNode, String bridgeName, String portName) {
        InstanceIdentifier<TerminationPoint> tpIid =
                this.createTerminationPointInstanceIdentifier(bridgeNode, portName);
        OvsdbTerminationPointAugmentationBuilder tpAugmentationBuilder =
                new OvsdbTerminationPointAugmentationBuilder();
        tpAugmentationBuilder.setName(portName);
        TerminationPointBuilder tpBuilder = new TerminationPointBuilder();
        tpBuilder.setKey(InstanceIdentifier.keyOf(tpIid));
        tpBuilder.addAugmentation(OvsdbTerminationPointAugmentation.class, tpAugmentationBuilder.build());
        return mdsalUtils.put(LogicalDatastoreType.CONFIGURATION, tpIid, tpBuilder.build());
    }

    /**
     * @params node instance of Node.
     * @params portName is String value.
     * @return InstanceIdentifier.
     */
    private static InstanceIdentifier<TerminationPoint> createTerminationPointInstanceIdentifier(Node node, String portName) {
        InstanceIdentifier<TerminationPoint> terminationPointPath = InstanceIdentifier
                .create(NetworkTopology.class)
                .child(Topology.class, new TopologyKey(new TopologyId(new Uri("ovsdb:1"))))
                .child(Node.class,node.getKey())
                .child(TerminationPoint.class, new TerminationPointKey(new TpId(portName)));

        LOG.debug("Termination point InstanceIdentifier generated : {}",terminationPointPath);
        return terminationPointPath;
    }

    /**
     * @params targetString is String value.
     * @return ControllerEntry ,the Controller details to be set .
     */
    private List<ControllerEntry> createControllerEntries(String targetString) {
        List<ControllerEntry> controllerEntries = new ArrayList<ControllerEntry>();
        ControllerEntryBuilder controllerEntryBuilder = new ControllerEntryBuilder();
        controllerEntryBuilder.setTarget(new Uri(targetString));
        controllerEntries.add(controllerEntryBuilder.build());
        return controllerEntries;
    }

    /**
     * Returns the supported OpenFlow Protocol Type.
     * @return ProtocolEntry.
     */
    private List<ProtocolEntry> createMdsalProtocols() {
        List<ProtocolEntry> protocolList = new ArrayList<ProtocolEntry>();
        ImmutableBiMap<String, Class<? extends OvsdbBridgeProtocolBase>> mapper =
                OVSDB_PROTOCOL_MAP.inverse();
        protocolList.add(new ProtocolEntryBuilder().
                setProtocol((Class<? extends OvsdbBridgeProtocolBase>) mapper.get("OpenFlow13")).build());
        return protocolList;
    }

    /**
     * Stores the Fail Mode Type.
     */
    private static final ImmutableBiMap<Class<? extends OvsdbFailModeBase>,String> OVSDB_FAIL_MODE_MAP
            = new ImmutableBiMap.Builder<Class<? extends OvsdbFailModeBase>,String>()
            .put(OvsdbFailModeStandalone.class,"standalone")
            .put(OvsdbFailModeSecure.class,"secure")
            .build();

    /**
     * Stores the Supported Openflow Protocol Version.
     */

    private static final ImmutableBiMap<Class<? extends OvsdbBridgeProtocolBase>,String> OVSDB_PROTOCOL_MAP
            = new ImmutableBiMap.Builder<Class<? extends OvsdbBridgeProtocolBase>,String>()
            .put(OvsdbBridgeProtocolOpenflow13.class,"OpenFlow13")
            .build();

    /**
     * Read the node details of the provided node.
     * @params node instance of Node.
     */
    private OvsdbNodeAugmentation extractOvsdbNode(Node node) {
        return node.getAugmentation(OvsdbNodeAugmentation.class);
    }

    /**
     * @params iid instance of InstanceIdentifier.
     * Returns the NodeId of the Controller.
     */
    private  NodeId createManagedNodeId(InstanceIdentifier<Node> iid) {
        NodeKey nodeKey = iid.firstKeyOf(Node.class, NodeKey.class);
        return nodeKey.getNodeId();
    }

    /**
     * @params ovsdbNode Node value
     * @params bridgeName String value
     * Returns false if bridge does not exist on the give node.
     */
    private boolean isBridgeOnOvsdbNode(Node ovsdbNode, String bridgeName) {
        boolean found = false;
        OvsdbNodeAugmentation ovsdbNodeAugmentation = extractOvsdbNode(ovsdbNode);
        if (ovsdbNodeAugmentation != null) {
            List<ManagedNodeEntry> managedNodes = ovsdbNodeAugmentation.getManagedNodeEntry();
            if (managedNodes != null) {
                for (ManagedNodeEntry managedNode : managedNodes) {
                    InstanceIdentifier<?> bridgeIid = managedNode.getBridgeRef().getValue();
                    if (bridgeIid.toString().contains(bridgeName)) {
                        found = true;
                        break;
                    }
                }
            }
        }
        return found;
    }
}
