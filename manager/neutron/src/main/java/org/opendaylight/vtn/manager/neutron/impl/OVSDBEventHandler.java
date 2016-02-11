/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_OK;

import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.convertUUIDToKey;

import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.collect.ImmutableBiMap;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInputBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.DatapathId;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ControllerEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ControllerEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ProtocolEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ProtocolEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ConnectionInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ManagedNodeEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.port._interface.attributes.InterfaceExternalIds;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.PortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointKey;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * Handle events from OVSDB Southbound plugin.
 */
public final class OVSDBEventHandler {
    /**
     * Logger instance.
     */
    private static final Logger LOG =
        LoggerFactory.getLogger(OVSDBEventHandler.class);

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
     * identifier to read for integration name from config file.
     */
    private static final String EXTERNAL_ID_INTERFACE_ID = "iface-id";

    /**
     * identifier to read failmode from config file.
     */
    private static final String CONFIG_FAILMODE = "failmode";

    /**
     * identifier to openflow port,6653 is official openflow port.
     */
    private static final short OPENFLOW_PORT = 6653;

    /**
     * identifier to read open flow protocol from config file.
     */
    public static final String OPENFLOW_CONNECTION_PROTOCOL = "tcp";

    /**
     * identifier to get Action of OVSDB Ports
     */
    private static final String ACTION_PORT = "delete";

    /**
     * Instance of {@link MdsalUtils}.
     */
    private final MdsalUtils  mdsalUtils;

    /**
     * VTN Manager service.
     */
    private final VTNManagerService  vtnManager;

    /**
     * identifier to read BRIDGE_URI_PREFIX.
     */
    public static final String BRIDGE_URI_PREFIX = "bridge";

    /**
     * VTN identifiers in neutron port object.
     */
    private static final int VTN_IDENTIFIERS_IN_PORT = 3;

    /**
     * identifiers for OVSDB Port Name.
     */
    public static String ovsdbPortName;

    /**
     * identifiers for OVSDB Bridge Name.
     */
    public static String ovsdbBridgeName;

    /**
     * identifiers for OVSDB Protocol.
     */
    public static String ovsdbProtocol;

    /**
     * identifiers for OVSDB Failmode.
     */
    public static String ovsdbFailMode;

    /**
     * Convert the given DPID into an {@link OfNode} instance.
     *
     * @param dpid  A {@link DatapathId} instance.
     * @return  An {@link OfNode} instance on success.
     *          {@code null} on return.
     */
    static OfNode toOfNode(DatapathId dpid) {
        try {
            return new OfNode(dpid);
        } catch (RuntimeException e) {
            LOG.info("Ignore invalid DPID: " + dpid, e);
            return null;
        }
    }

    /**
     * Method invoked when the open flow switch is Added.
     *
     * @param md   A {@link MdsalUtils} instance.
     * @param vtn  A {@link VTNManagerService} instance.
     */
    public OVSDBEventHandler(MdsalUtils md, VTNManagerService vtn) {
        mdsalUtils = md;
        vtnManager = vtn;
    }

    /**
     * Method invoked when the open flow switch is Added.
     * @param node Instance of Node object to be added.
     * @param resourceAugmentationData DataObject value.
     */
    public void nodeAdded(Node node, DataObject resourceAugmentationData) {
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
     * Read the parameters configured in configuration file(default.config).
     * @param node instance of Node
     */
    public void getSystemProperties(Node node) {
        LOG.trace("System properties from default config : {},{},{},{}:",
                  ovsdbBridgeName, ovsdbPortName, ovsdbProtocol, ovsdbFailMode);
        integrationBridgeName = ovsdbBridgeName;
        if (integrationBridgeName == null) {
            integrationBridgeName = DEFAULT_INTEGRATION_BRIDGENAME;
        }
        portname = ovsdbPortName;
        if (portname == null) {
            portname = DEFAULT_PORTNAME;
        }
        failmode = ovsdbFailMode;
        if (failmode == null) {
            failmode = DEFAULT_FAILMODE;
        }
        protocols = ovsdbProtocol;
        if (protocols == null) {
            protocols = DEFAULT_PROTOCOLS;
        }
        LOG.trace("System properties values : {},{},{},{}:",
                  integrationBridgeName, failmode, protocols, portname);
        if (null != integrationBridgeName) {
            try {
                if (!isBridgeOnOvsdbNode(node, integrationBridgeName)) {
                    addBridge(node, integrationBridgeName);
                    addPortToBridge(node, integrationBridgeName, portname);
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
     * @param ovsdbNodeKey  A {@link NodeKey} instance.
     * @param bridgeName    The name of the bridge.
     * @return The {@link InstanceIdentifier} for the manager node.
     */
    private  InstanceIdentifier<Node> createInstanceIdentifier(NodeKey ovsdbNodeKey, String bridgeName) {
        return createInstanceIdentifier(
            createManagedNodeId(ovsdbNodeKey.getNodeId(), bridgeName));
    }

    /**
     * Create instance identifier
     * @param nodeId is instance of NodeId
     * @return The {@link OvsdbBridgeAugmentation} for the manager node.
     */
    private InstanceIdentifier<Node> createInstanceIdentifier(NodeId nodeId) {
        return InstanceIdentifier.builder(NetworkTopology.class).
            child(Topology.class,
                  new TopologyKey(new TopologyId(new Uri("ovsdb:1")))).
            child(Node.class, new NodeKey(nodeId)).
            build();
    }

    /**
     * Get the manager node for this bridge node
     * @param ovsdbNodeId  A {@link NodeId} instance.
     * @param bridgeName   The name of the bridge.
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
        LOG.trace("ConnectionInfo Node {}", ovsdbNode);
        ConnectionInfo connectionInfo = null;
        OvsdbNodeAugmentation ovsdbNodeAugmentation = extractOvsdbNode(ovsdbNode);
        LOG.trace("OvsdbNodeAugmentation  Node {}", ovsdbNodeAugmentation);
        if (ovsdbNodeAugmentation != null) {
            connectionInfo = ovsdbNodeAugmentation.getConnectionInfo();
            LOG.debug("return connection info  Node {}", connectionInfo);
        }
        return connectionInfo;
    }

    /**
     * Add Bridge to the given Node
     * @param ovsdbNode the node object of the OVS instance
     * @param bridgeName the bridgename of the bridge to be added
     * @return true if the bridge add is successful.
     */
    public boolean addBridge(Node ovsdbNode, String bridgeName) {
        boolean result = false;
        String target = getControllerTarget(ovsdbNode);
        LOG.trace("addBridge: node: {}, bridgeName: {}, target: {}",
                  ovsdbNode, bridgeName, target);
        ConnectionInfo connectionInfo = getConnectionInfo(ovsdbNode);
        if (connectionInfo != null) {
            LOG.debug("Connection info is not null {}", connectionInfo);
            NodeBuilder bridgeNodeBuilder = new NodeBuilder();
            InstanceIdentifier<Node> bridgeIid =
                this.createInstanceIdentifier(ovsdbNode.getKey(), bridgeName);
            NodeId bridgeNodeId = this.createManagedNodeId(bridgeIid);
            bridgeNodeBuilder.setNodeId(bridgeNodeId);
            OvsdbBridgeAugmentationBuilder ovsdbBridgeAugmentationBuilder =
                new OvsdbBridgeAugmentationBuilder();
            ovsdbBridgeAugmentationBuilder.
                setControllerEntry(createControllerEntries(target));
            ovsdbBridgeAugmentationBuilder.
                setBridgeName(new OvsdbBridgeName(bridgeName));
            ovsdbBridgeAugmentationBuilder.
                setProtocolEntry(createMdsalProtocols());
            ovsdbBridgeAugmentationBuilder.setFailMode(
                OVSDB_FAIL_MODE_MAP.inverse().get("secure"));
            this.setManagedByForBridge(ovsdbBridgeAugmentationBuilder,
                                       ovsdbNode.getKey());
            bridgeNodeBuilder.
                addAugmentation(OvsdbBridgeAugmentation.class,
                                ovsdbBridgeAugmentationBuilder.build());
            result = mdsalUtils.put(LogicalDatastoreType.CONFIGURATION,
                                    bridgeIid, bridgeNodeBuilder.build());
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
     * @param bridgeNode  A {@link Node} instance.
     * @param portName    The name of the port.
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
     * @param node  A {@link Node} instance.
     * @return A list of {@link OvsdbTerminationPointAugmentation} instances.
     */
    private List<OvsdbTerminationPointAugmentation> extractTerminationPointAugmentations(Node node) {
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
      * Get the bridge node for a particular bridge from config DS
      * @param node  A {@link Node} instance.
      * @param bridgeName   The name of the bridge.
      * @return The {@link Node} for the manager node.
      */
    private Node getBridgeConfigNode(Node node, String bridgeName) {
        InstanceIdentifier<Node> bridgeIid =
                 this.createInstanceIdentifier(node.getKey(), bridgeName);
        Node bridgeNode = mdsalUtils.
                 read(LogicalDatastoreType.CONFIGURATION,
                      bridgeIid).orNull();
        return bridgeNode;
    }

    /**
     * Add a Port to the existing bridge.
     * @param parentNode        A {@link Node} instance.
     * @param bridgeName  The name of the bridge.
     * @param portName    The name of the port.
     * @return true on success
     */
    private boolean addPortToBridge(Node parentNode, String bridgeName, String portName) {
        boolean rv = true;
        Node bridgeNode = getBridgeConfigNode(parentNode, bridgeName);

        // This should never occur
        if (bridgeNode == null) {
            LOG.error("addPortToBridge: node: {}, bridge: {}, status: bridge not configured in OVS",
                   parentNode.getNodeId(), bridgeName);
            return false;
        }

        if (extractTerminationPointAugmentation(bridgeNode, portName) == null) {
            rv = addTerminationPoint(bridgeNode, bridgeName, portName);

            if (rv) {
                LOG.trace("addPortToBridge: node: {}, bridge: {}, portname: {} status: success",
                        bridgeNode.getNodeId().getValue(), bridgeName, portName);
            } else {
                LOG.error("addPortToBridge: node: {}, bridge: {}, portname: {} status: failed",
                        bridgeNode.getNodeId().getValue(), bridgeName, portName);
            }
        } else {
            LOG.trace("addPortToBridge: node: {}, bridge: {}, portname: {} status: not_needed",
                    bridgeNode.getNodeId().getValue(), bridgeName, portName);
        }
        return rv;
    }

    /**
     * Set the manager node for the node
     * @param ovsdbBridgeAugmentationBuilder
     *    A {@link OvsdbBridgeAugmentationBuilder} instance.
     * @param ovsdbNodeKey  A {@link NodeKey} instance.
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
     * @param node      A {@link Node} instance.
     * @param portName  The name of the port.
     * @return InstanceIdentifier.
     */
    private static InstanceIdentifier<TerminationPoint> createTerminationPointInstanceIdentifier(Node node, String portName) {
        InstanceIdentifier<TerminationPoint> terminationPointPath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class,
                  new TopologyKey(new TopologyId(new Uri("ovsdb:1")))).
            child(Node.class, node.getKey()).
            child(TerminationPoint.class,
                  new TerminationPointKey(new TpId(portName))).
            build();

        LOG.debug("Termination point InstanceIdentifier generated : {}", terminationPointPath);
        return terminationPointPath;
    }

    /**
     * @param targetString is String value.
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
                setProtocol((Class<? extends OvsdbBridgeProtocolBase>)mapper.get("OpenFlow13")).build());
        return protocolList;
    }

    /**
     * Stores the Fail Mode Type.
     */
    private static final ImmutableBiMap<Class<? extends OvsdbFailModeBase>, String> OVSDB_FAIL_MODE_MAP
            = new ImmutableBiMap.Builder<Class<? extends OvsdbFailModeBase>, String>()
            .put(OvsdbFailModeStandalone.class, "standalone")
            .put(OvsdbFailModeSecure.class, "secure")
            .build();

    /**
     * Stores the Supported Openflow Protocol Version.
     */

    private static final ImmutableBiMap<Class<? extends OvsdbBridgeProtocolBase>, String> OVSDB_PROTOCOL_MAP
            = new ImmutableBiMap.Builder<Class<? extends OvsdbBridgeProtocolBase>, String>()
            .put(OvsdbBridgeProtocolOpenflow13.class, "OpenFlow13")
            .build();

    /**
     * Read the node details of the provided node.
     * @param node instance of Node.
     */
    private OvsdbNodeAugmentation extractOvsdbNode(Node node) {
        return node.getAugmentation(OvsdbNodeAugmentation.class);
    }

    /**
     * @param iid instance of InstanceIdentifier.
     * Returns the NodeId of the Controller.
     */
    private NodeId createManagedNodeId(InstanceIdentifier<Node> iid) {
        NodeKey nodeKey = iid.firstKeyOf(Node.class);
        return nodeKey.getNodeId();
    }

    /**
     * @param ovsdbNode Node value
     * @param bridgeName String value
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

    /**
     * Get OVSDB Ports.
     * @param node the OVS node on which the ports are to be read
     * @param action denodes the type of action to read for
     */
    public void readOVSDBPorts(Node node, String action) {
        InstanceIdentifier<Node> bridgeNodeIid = this
                .createInstanceIdentifier(node.getNodeId());
        Node operNode = mdsalUtils.
            read(LogicalDatastoreType.OPERATIONAL, bridgeNodeIid).orNull();
        String value = "";
        if (operNode != null) {
            List<OvsdbTerminationPointAugmentation> ports = extractTerminationPointAugmentations(operNode);
            OvsdbBridgeAugmentation bridgeNode = getBridgeNode(node,
                    integrationBridgeName);
            for (OvsdbTerminationPointAugmentation port : ports) {
                List<InterfaceExternalIds> pairs = port.getInterfaceExternalIds();
                String ovsPortName = port.getName();
                if (pairs != null && !pairs.isEmpty()) {
                    for (InterfaceExternalIds pair : pairs) {
                        if (pair.getExternalIdKey().equalsIgnoreCase(EXTERNAL_ID_INTERFACE_ID)) {
                            value = pair.getExternalIdValue();
                            Port neutronPort = readNeutronPort(value);
                            if ((action.equalsIgnoreCase(ACTION_PORT)) && ((neutronPort != null))) {
                                deletePortMapForInterface(neutronPort);
                            }

                            OfNode ofNode = toOfNode(
                                bridgeNode.getDatapathId());
                            Long ofPort = port.getOfport();
                            if (neutronPort != null && ofNode != null) {
                                setPortMapForInterface(
                                    neutronPort, ofNode, ofPort, ovsPortName);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    /**
     * Set PortMap for an Interface.
     *
     * @param neutronPort  A {@link Port} instance.
     * @param ofNode       An {@link OfNode} instance that specifies the
     *                     OpenFlow switch.
     * @param ofPort       A long value that indicates the physical switch port
     *                     number.
     * @param portName     The name of the switch port.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    private int setPortMapForInterface(
        Port neutronPort, OfNode ofNode, Long ofPort, String portName) {
        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        int result = getVTNIdentifiers(neutronPort, vtnIDs);
        if (result != HTTP_OK) {
            LOG.error("processRowUpdated getVTNIdentifiers failed, result - {}",
                      result);
        } else {
            // Map the untagged network on the specified port.
            VlanId vlanId = new VlanId(0);
            String tenantID = vtnIDs[0];
            String bridgeID = vtnIDs[1];
            String portID = vtnIDs[2];
            if (ofPort != null) {
                SetPortMapInput input = new SetPortMapInputBuilder().
                        setTenantName(tenantID).
                        setBridgeName(bridgeID).
                        setInterfaceName(portID).
                        setNode(ofNode.getNodeId()).
                        setPortId(ofPort.toString()).
                        setVlanId(vlanId).
                        build();
                result = vtnManager.setPortMap(input);
            } else {
                if (portName != null) {
                    SetPortMapInput input = new SetPortMapInputBuilder().
                            setTenantName(tenantID).
                            setBridgeName(bridgeID).
                            setInterfaceName(portID).
                            setNode(ofNode.getNodeId()).
                            setPortName(portName).
                            setVlanId(vlanId).
                            build();
                    result = vtnManager.setPortMap(input);
                }
            }
        }

        return result;
    }

    /**
     * Validate and Return VTN identifiers for the given NeutronPort object.
     *
     * @param port
     *            An instance of Port object.
     * @param vtnIDs
     *            VTN identifiers.
     * @return A HTTP status code to the validation request.
     */
    private int getVTNIdentifiers(Port port, String[] vtnIDs) {
        int result = HTTP_BAD_REQUEST;
        /**
         * To basic validation of the request
         */
        if (port == null) {
            LOG.error("port object not specified");
            return result;
        }

        String tenantUUID = port.getTenantId().getValue();
        String bridgeUUID = port.getNetworkId().getValue();
        String portUUID = port.getUuid().getValue();


        if ((tenantUUID == null) || (bridgeUUID == null) || portUUID == null) {
            LOG.error("neutron identifiers not specified");
            return result;
        }

        String tenantID = convertUUIDToKey(tenantUUID);
        if (tenantID == null) {
            LOG.error("Invalid tenant identifier");
            return result;
        }

        String bridgeID = convertUUIDToKey(bridgeUUID);
        if (bridgeID == null) {
            LOG.error("Invalid bridge identifier");
            return result;
        }

        String portID = convertUUIDToKey(portUUID);
        if (portID == null) {
            LOG.error("Invalid port identifier");
            return result;
        }

        vtnIDs[0] = tenantID;
        vtnIDs[1] = bridgeID;
        vtnIDs[2] = portID;

        return HTTP_OK;
    }

    /**
     * Delete PortMap set for a NeutronPort.
     *
     * @param neutronPort
     *            An instance of NeutronPort object.
     * @return A HTTP status code for delete PortMap request.
     */
    private int deletePortMapForInterface(Port neutronPort) {
        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        int result = getVTNIdentifiers(neutronPort, vtnIDs);
        if (result != HTTP_OK) {
            LOG.error("processRowUpdated getVTNIdentifiers failed, result - {}",
                      result);
        } else {
            String tenantID = vtnIDs[0];
            String bridgeID = vtnIDs[1];
            String portID = vtnIDs[2];
            RemovePortMapInput input = new RemovePortMapInputBuilder().
                setTenantName(tenantID).
                setBridgeName(bridgeID).
                setInterfaceName(portID).
                build();
            result = vtnManager.removePortMap(input);
        }

        return result;
    }

    /**
     * Read NeutronPort.
     * @param uuid
     * @return  Neutron Port.
     */
    private Port readNeutronPort(String uuid) {
        try {
            Uuid uid = new Uuid(uuid);
            InstanceIdentifier<Port> portpath = InstanceIdentifier.builder(Neutron.class).child(Ports.class).child(Port.class ,   new PortKey(uid)).build();
            Port port = mdsalUtils.
                read(LogicalDatastoreType.CONFIGURATION, portpath).orNull();
            return port;
        } catch (Exception ex) {
            LOG.error("exception obtained {}", ex);
        }
        return null;
    }

    /**
     * Get all BridgeDetails.
     * @param node
     * @return OvsdbBridgeAugmentation.
     */
    private  OvsdbBridgeAugmentation extractBridgeAugmentation(Node node) {
        if (node == null) {
            return null;
        }
        return node.getAugmentation(OvsdbBridgeAugmentation.class);
    }

    /**
     * Get the Bridge Details for specified bridgeName.
     * @param node
     * @param bridgeName
     * @return OvsdbBridgeAugmentation.
     */
    private  OvsdbBridgeAugmentation getBridgeNode(Node node, String bridgeName) {
        OvsdbBridgeAugmentation bridgeNode;
        OvsdbBridgeAugmentation bridge = extractBridgeAugmentation(node);
        if (bridge != null
                && bridge.getBridgeName().getValue().equals(bridgeName)) {
            bridgeNode = bridge;
        } else {
            bridgeNode = null;
        }

        return bridgeNode;
    }

}
