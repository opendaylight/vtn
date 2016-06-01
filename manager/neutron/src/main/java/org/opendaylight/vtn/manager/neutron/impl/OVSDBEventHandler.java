/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_OK;

import static org.opendaylight.vtn.manager.neutron.impl.NeutronConfig.FAILMODE_SECURE;
import static org.opendaylight.vtn.manager.neutron.impl.NeutronConfig.FAILMODE_STANDALONE;
import static org.opendaylight.vtn.manager.neutron.impl.NeutronConfig.PROTO_OF13;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getBridgeId;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getInterfaceId;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getTenantId;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getUuid;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.collect.ImmutableBiMap;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInputBuilder;

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

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;

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
     * identifier to openflow port,6653 is official openflow port.
     */
    private static final short OPENFLOW_PORT = 6653;

    /**
     * identifier to read open flow protocol from config file.
     */
    private static final String OPENFLOW_CONNECTION_PROTOCOL = "tcp";

    /**
     * Stores the Fail Mode Type.
     */
    private static final ImmutableBiMap<Class<? extends OvsdbFailModeBase>, String> OVSDB_FAIL_MODE_MAP =
        new ImmutableBiMap.Builder<Class<? extends OvsdbFailModeBase>, String>().
        put(OvsdbFailModeStandalone.class, FAILMODE_STANDALONE).
        put(OvsdbFailModeSecure.class, FAILMODE_SECURE).
        build();

    /**
     * Stores the Supported Openflow Protocol Version.
     */
    private static final ImmutableBiMap<Class<? extends OvsdbBridgeProtocolBase>, String> OVSDB_PROTOCOL_MAP =
        new ImmutableBiMap.Builder<Class<? extends OvsdbBridgeProtocolBase>, String>().
        put(OvsdbBridgeProtocolOpenflow13.class, PROTO_OF13).
        build();

    /**
     * The configuration of the manager.neutron bundle.
     */
    private final NeutronConfig  bundleConfig;

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
     * Method invoked when the open flow switch is Added.
     *
     * @param cfg  The configuration for the manager.neutron bundle.
     * @param md   A {@link MdsalUtils} instance.
     * @param vtn  A {@link VTNManagerService} instance.
     */
    public OVSDBEventHandler(NeutronConfig cfg, MdsalUtils md,
                             VTNManagerService vtn) {
        bundleConfig = cfg;
        mdsalUtils = md;
        vtnManager = vtn;
    }

    /**
     * Method invoked when the open flow switch is Added.
     *
     * @param node    The created node.
     * @param ovnode  The created OVSDB node augmentation.
     */
    public void nodeAdded(@Nonnull Node node,
                          @Nonnull OvsdbNodeAugmentation ovnode) {
        NodeKey key = node.getKey();
        LOG.trace("nodeAdded() - key={}, ovnode={}", key, ovnode);
        if (key != null) {
            String bname = bundleConfig.getOvsdbBridgeName();
            if (!isBridgeOnOvsdbNode(ovnode, bname)) {
                if (addBridge(key, ovnode, bname)) {
                    addPortToBridge(key, bname,
                                    bundleConfig.getOvsdbPortName());
                }
            } else {
                LOG.trace("Bridge already exists in the given node: {}",
                          key.getNodeId());
            }
        } else {
            // This should never happen.
            LOG.warn("Ignore null node key: node={}", node);
        }
    }

    /**
     * Method invoked when the open flow switch is Removed.
     * @param node Instance of Node object to be removed.
     */
    public void nodeRemoved(Node node) {
        LOG.trace("nodeRemoved() - {}", node);
        boolean result = deleteBridge(node, bundleConfig.getOvsdbBridgeName());
        if (!result) {
            LOG.error("Failure in delete the bridge node entry from Config Datastore");
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
     */
    public void setPortMap(@Nonnull Port neutronPort, @Nonnull OfNode ofNode,
                           Long ofPort, String portName) {
        // Map the untagged network on the specified port.
        LOG.info("Configuring port mapping: neutron-port={}, of-port=[{}, {}]",
                 getUuid(neutronPort.getUuid()), ofPort, portName);
        VlanId vlanId = new VlanId(0);
        SetPortMapInputBuilder builder = new SetPortMapInputBuilder().
            setTenantName(getTenantId(neutronPort)).
            setBridgeName(getBridgeId(neutronPort)).
            setInterfaceName(getInterfaceId(neutronPort)).
            setNode(ofNode.getNodeId()).
            setVlanId(vlanId);
        if (ofPort != null) {
            builder.setPortId(ofPort.toString());
        } else if (portName != null) {
            builder.setPortName(portName);
        } else {
            LOG.error("No physical port is specified: port={}, node={}",
                      neutronPort, ofNode);
            return;
        }

        int result = vtnManager.setPortMap(builder.build());
        if (result != HTTP_OK) {
            LOG.error("Failed to map physical port: port={}, node={}, " +
                      "id={}, name={}", neutronPort, ofNode, ofPort,
                      portName);
        }
    }

    /**
     * Delete PortMap set for a NeutronPort.
     *
     * @param neutronPort
     *            An instance of NeutronPort object.
     */
    public void deletePortMap(@Nonnull Port neutronPort) {
        LOG.info("Unconfiguring port mapping: neutron-port={}",
                 getUuid(neutronPort.getUuid()));
        RemovePortMapInput input = new RemovePortMapInputBuilder().
            setTenantName(getTenantId(neutronPort)).
            setBridgeName(getBridgeId(neutronPort)).
            setInterfaceName(getInterfaceId(neutronPort)).
            build();
        int result = vtnManager.removePortMap(input);
        if (result != HTTP_OK && result != HTTP_NOT_FOUND) {
            LOG.error("Failed to unmap physical port: port={}", neutronPort);
        }
    }

    /**
     * Return the name of the interface bridge in the OVS.
     *
     * @return  The name of the interface bridge in the OVS.
     */
    @Nonnull
    public String getOvsdbBridgeName() {
        return bundleConfig.getOvsdbBridgeName();
    }

    /**
     * Read Node from the OVSDB path.
     *
     * @param ovsdbNodeKey  A {@link NodeKey} instance.
     * @param bridgeName    The name of the bridge.
     * @return The {@link InstanceIdentifier} for the manager node.
     */
    private  InstanceIdentifier<Node> createInstanceIdentifier(NodeKey ovsdbNodeKey, String bridgeName) {
        return createInstanceIdentifier(
            getManagedNodeId(ovsdbNodeKey.getNodeId(), bridgeName));
    }

    /**
     * Create instance identifier.
     *
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
     * Get the manager node for this bridge node.
     *
     * @param ovsdbNodeId  A {@link NodeId} instance.
     * @param bridgeName   The name of the bridge.
     * @return The {@link OvsdbBridgeAugmentation} for the manager node.
     */
    private NodeId getManagedNodeId(NodeId ovsdbNodeId, String bridgeName) {
        return new NodeId(ovsdbNodeId.getValue()
                + "/" + BRIDGE_URI_PREFIX + "/" + bridgeName);
    }

    /**
     * Add Bridge to the given Node.
     *
     * @param key         The key of the target node.
     * @param ovnode      OVSDB node augmentation.
     * @param bridgeName  The name of the bridge to be added
     * @return true if the bridge add is successful.
     */
    private boolean addBridge(@Nonnull NodeKey key,
                              @Nonnull OvsdbNodeAugmentation ovnode,
                              @Nonnull String bridgeName) {
        boolean result = false;
        String target = getControllerTarget(ovnode);
        LOG.trace("addBridge: node: {}, bridgeName: {}, target: {}",
                  ovnode, bridgeName, target);
        ConnectionInfo connectionInfo = ovnode.getConnectionInfo();
        if (connectionInfo != null) {
            LOG.debug("Connection info is not null {}", connectionInfo);
            NodeBuilder bridgeNodeBuilder = new NodeBuilder();
            InstanceIdentifier<Node> bridgeIid =
                this.createInstanceIdentifier(key, bridgeName);
            NodeId bridgeNodeId = getNodeId(bridgeIid);
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
                OVSDB_FAIL_MODE_MAP.inverse().get(FAILMODE_SECURE));
            setManagedByForBridge(ovsdbBridgeAugmentationBuilder, key);
            bridgeNodeBuilder.
                addAugmentation(OvsdbBridgeAugmentation.class,
                                ovsdbBridgeAugmentationBuilder.build());
            result = mdsalUtils.put(LogicalDatastoreType.CONFIGURATION,
                                    bridgeIid, bridgeNodeBuilder.build());
            LOG.info("Bridge Added: {}", result);
        } else {
            LOG.error("Could not find ConnectionInfo: ovnode={}", ovnode);
        }
        return result;
    }

    /**
     * Delete Bridge on the given Node.
     *
     * @param ovsdbNode the node object of the OVS instance
     * @param bridgeName the bridgename of the bridge to be removed
     * @return true if the bridge deleted is successful.
     */
    private boolean deleteBridge(Node ovsdbNode, String bridgeName) {
        InstanceIdentifier<Node> bridgeIid =
            this.createInstanceIdentifier(ovsdbNode.getKey(), bridgeName);
        return mdsalUtils.delete(LogicalDatastoreType.CONFIGURATION, bridgeIid);
    }

    /**
     * Returns controller details of the Node.
     *
     * @param ovnode  OVSDB node augmentation.
     * @return  Controller information.
     */
    private String getControllerTarget(@Nonnull OvsdbNodeAugmentation ovnode) {
        String setControllerStr = null;
        String controllerIpStr = null;
        short openflowPort = OPENFLOW_PORT;

        // Check if ovsdb node has connection info
        ConnectionInfo connectionInfo = ovnode.getConnectionInfo();
        if (connectionInfo != null && connectionInfo.getLocalIp() != null) {
            controllerIpStr =
                new String(connectionInfo.getLocalIp().getValue());
        } else {
            LOG.warn("Ovsdb Node does not contains connection info : {}",
                     ovnode);
        }
        if (controllerIpStr != null) {
            LOG.trace("Target OpenFlow Controller found : {}", controllerIpStr);
            setControllerStr = OPENFLOW_CONNECTION_PROTOCOL + ":" +
                controllerIpStr + ":" + openflowPort;
        } else {
            LOG.warn("Failed to determine OpenFlow controller ip address");
        }
        return setControllerStr;
    }

    /**
     * Return the particular port detail of the given Node.
     *
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
     *
     * @param node  A {@link Node} instance.
     * @return A list of {@link OvsdbTerminationPointAugmentation} instances.
     */
    private List<OvsdbTerminationPointAugmentation> extractTerminationPointAugmentations(Node node) {
        List<OvsdbTerminationPointAugmentation> tpAugmentations = new ArrayList<OvsdbTerminationPointAugmentation>();
        List<TerminationPoint> terminationPoints = node.getTerminationPoint();
        if (terminationPoints != null) {
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
     *
     * @param key         The key of the targe node.
     * @param bridgeName  The name of the bridge.
     * @param portName    The name of the port.
     * @return true on success
     */
    private boolean addPortToBridge(@Nonnull NodeKey key,
                                    @Nonnull String bridgeName,
                                    @Nonnull String portName) {
        boolean rv = true;
        InstanceIdentifier<Node> bridgeIid =
            createInstanceIdentifier(key, bridgeName);
        Node bridgeNode = mdsalUtils.read(
            LogicalDatastoreType.CONFIGURATION, bridgeIid).orNull();

        // This should never occur
        if (bridgeNode == null) {
            LOG.error("addPortToBridge: node: {}, bridge: {}, status: bridge not configured in OVS",
                      key.getNodeId(), bridgeName);
            return false;
        }

        if (extractTerminationPointAugmentation(bridgeNode, portName) == null) {
            rv = addTerminationPoint(bridgeNode, portName);

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
     * Set the manager node for the node.
     *
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
     * Add the port to the node, returns true on success.
     *
     * @param bridgeNode  The target bridge node.
     * @param portName    The name of the porrt.
     * @return true on success.
     */
    private boolean addTerminationPoint(Node bridgeNode, String portName) {
        InstanceIdentifier<TerminationPoint> tpIid =
            createTerminationPointInstanceIdentifier(bridgeNode, portName);
        OvsdbTerminationPointAugmentationBuilder tpAugmentationBuilder =
            new OvsdbTerminationPointAugmentationBuilder();
        tpAugmentationBuilder.setName(portName);
        TerminationPointBuilder tpBuilder = new TerminationPointBuilder();
        tpBuilder.setKey(InstanceIdentifier.keyOf(tpIid));
        tpBuilder.addAugmentation(OvsdbTerminationPointAugmentation.class,
                                  tpAugmentationBuilder.build());
        return mdsalUtils.put(LogicalDatastoreType.CONFIGURATION, tpIid,
                              tpBuilder.build());
    }

    /**
     * Create instance identifier for the specified termination point.
     *
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
     * Create a list of controller entries.
     *
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
     *
     * @return ProtocolEntry.
     */
    private List<ProtocolEntry> createMdsalProtocols() {
        List<ProtocolEntry> protocolList = new ArrayList<ProtocolEntry>();
        ImmutableBiMap<String, Class<? extends OvsdbBridgeProtocolBase>> mapper =
                OVSDB_PROTOCOL_MAP.inverse();
        protocolList.add(new ProtocolEntryBuilder().
                setProtocol((Class<? extends OvsdbBridgeProtocolBase>)mapper.get(PROTO_OF13)).build());
        return protocolList;
    }

    /**
     * Return the node ID in the specified instance identifier.
     *
     * @param iid instance of InstanceIdentifier.
     * @return  NodeId of the Controller.
     */
    private NodeId getNodeId(InstanceIdentifier<Node> iid) {
        NodeKey nodeKey = iid.firstKeyOf(Node.class);
        return nodeKey.getNodeId();
    }

    /**
     * @param ovnode  OVSDB node augmentation.
     * @param bridgeName  String value
     * @return  false if bridge does not exist on the give node.
     */
    private boolean isBridgeOnOvsdbNode(
        @Nonnull OvsdbNodeAugmentation ovnode, String bridgeName) {
        boolean found = false;
        List<ManagedNodeEntry> managedNodes = ovnode.getManagedNodeEntry();
        if (managedNodes != null) {
            for (ManagedNodeEntry managedNode : managedNodes) {
                InstanceIdentifier<?> bridgeIid =
                    managedNode.getBridgeRef().getValue();
                if (bridgeIid.toString().contains(bridgeName)) {
                    found = true;
                    break;
                }
            }
        }
        return found;
    }
}
