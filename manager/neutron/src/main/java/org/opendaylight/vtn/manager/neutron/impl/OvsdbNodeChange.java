/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.opendaylight.vtn.manager.neutron.impl.OvsdbDataChangeListener.LOG;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.annotation.Nonnull;

import com.google.common.base.Optional;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.DatapathId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.port._interface.attributes.InterfaceExternalIds;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.PortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;

/**
 * {@code OvsdbNodeChange} describes changes made to the node in the
 * OVSDB topology.
 */
public final class OvsdbNodeChange {
    /**
     * The OVSDB external ID associated with the system identifier of the
     * interface.
     */
    private static final String  EXTID_INTERFACE_ID = "iface-id";

    /**
     * OVSDBEventHandler instance.
     */
    @Nonnull
    private final OVSDBEventHandler  eventHandler;

    /**
     * MD-SAL read-only transaction holder.
     */
    @Nonnull
    private final ReadTransactionHolder  txHolder;

    /**
     * The target node.
     */
    @Nonnull
    private final Node  targetNode;

    /**
     * The OVSDB node augmentation that has been newly created.
     */
    private OvsdbNodeAugmentation  createdOvsdbNode;

    /**
     * Set {@code true} if the OVSDB node augmentation has been removed.
     */
    private boolean  ovsdbNodeRemoved;

    /**
     * A list of port mapping configurations to be applied.
     */
    private List<PortMapAttr>  mapPorts = new ArrayList<>();

    /**
     * A list of neutron ports to deconfigure port mapping.
     */
    private List<Port>  unmapPorts = new ArrayList<>();

    /**
     * The identifier of the OpenFlow node that contains switch ports to be
     * mapped.
     */
    private OfNode  openflowNode;

    /**
     * {@code OvsdbPortAttr} describes attributes of the OVSDB port to be
     * mapped to the virtual interface.
     */
    private static final class PortMapAttr {
        /**
         * The target neutron port.
         */
        @Nonnull
        private final Port  targetPort;

        /**
         * The port identifier that specifies the port in the OpenFlow node.
         */
        private final Long  portId;

        /**
         * The name of the port.
         */
        private final String  portName;

        /**
         * Construct a new instance.
         *
         * @param port  The target neutron port.
         * @param ovtp  The OVSDB termination point augmentation.
         */
        private PortMapAttr(@Nonnull Port port,
                            @Nonnull OvsdbTerminationPointAugmentation ovtp) {
            targetPort = port;
            portId = ovtp.getOfport();
            portName = ovtp.getName();
        }

        /**
         * Apply the port mapping configuration.
         *
         * @param ovh    An {@link OVSDBEventHandler} instance.
         * @param onode  An {@link OfNode} instance that specifies the
         *               OpenFlow switch.
         */
        private void apply(@Nonnull OVSDBEventHandler ovh,
                           @Nonnull OfNode onode) {
            ovh.setPortMap(targetPort, onode, portId, portName);
        }
    }

    /**
     * Create information about the created node.
     *
     * @param ovh   An {@link OVSDBEventHandler} instance.
     * @param txh   The read-only transaction holder.
     * @param node  The created node.
     * @return  An {@link OvsdbNodeChange} instance that contains information
     *          about the created node.
     *          {@code null} if no change to be applied.
     */
    public static OvsdbNodeChange nodeCreated(
        @Nonnull OVSDBEventHandler ovh, @Nonnull ReadTransactionHolder txh,
        @Nonnull Node node) {
        OvsdbNodeChange ovchg;
        OvsdbNodeAugmentation ovnode =
            node.getAugmentation(OvsdbNodeAugmentation.class);
        if (ovnode != null) {
            // The OVSDB node augmentation has been created.
            ovchg = new OvsdbNodeChange(ovh, txh, node);
            ovchg.setCreatedOvsdbNode(node, ovnode);
        } else {
            ovchg = null;
        }

        return ovchg;
    }

    /**
     * Create information about the updated node.
     *
     * @param ovh   An {@link OVSDBEventHandler} instance.
     * @param txh     The read-only transaction holder.
     * @param mod     The data object modification that represents changes
     *                to the node subtree.
     * @param before  The target node before modification.
     * @param after   The target node after modification.
     * @return  An {@link OvsdbNodeChange} instance that contains information
     *          about the removed node.
     *          {@code null} if no change to be applied.
     */
    public static OvsdbNodeChange nodeUpdated(
        @Nonnull OVSDBEventHandler ovh, @Nonnull ReadTransactionHolder txh,
        @Nonnull DataObjectModification<Node> mod, @Nonnull Node before,
        @Nonnull Node after) {
        OvsdbNodeChange ovchg;
        OvsdbNodeAugmentation oldOvnode =
            before.getAugmentation(OvsdbNodeAugmentation.class);
        OvsdbNodeAugmentation newOvnode =
            after.getAugmentation(OvsdbNodeAugmentation.class);
        if (newOvnode != null) {
            ovchg = new OvsdbNodeChange(ovh, txh, after);
            if (oldOvnode == null) {
                // The OVSDB node augmentation has been created.
                ovchg.setCreatedOvsdbNode(after, newOvnode);
            } else {
                ovchg.nodeUpdated(mod, before);
            }
        } else if (oldOvnode != null) {
            // The OVSDB node augmentation has been removed.
            ovchg = new OvsdbNodeChange(ovh, txh, before);
            ovchg.setRemovedOvsdbNode(before);
        } else {
            ovchg = null;
        }

        return ovchg;
    }

    /**
     * Create information about the removed node.
     *
     * @param ovh   An {@link OVSDBEventHandler} instance.
     * @param txh   The read-only transaction holder.
     * @param node  The removed node.
     * @return  An {@link OvsdbNodeChange} instance that contains information
     *          about the removed node.
     *          {@code null} if no change to be applied.
     */
    public static OvsdbNodeChange nodeRemoved(
        @Nonnull OVSDBEventHandler ovh, @Nonnull ReadTransactionHolder txh,
        @Nonnull Node node) {
        OvsdbNodeChange ovchg;
        OvsdbNodeAugmentation ovnode =
            node.getAugmentation(OvsdbNodeAugmentation.class);
        if (ovnode != null) {
            // The OVSDB node augmentation has been removed.
            ovchg = new OvsdbNodeChange(ovh, txh, node);
            ovchg.setRemovedOvsdbNode(node);
        } else {
            ovchg = null;
        }

        return ovchg;
    }

    /**
     * Return the neutron port ID configured in the given OVSDB termination
     * point augmentation.
     *
     * @param ovtp  The OVSDB termination point augmentation.
     * @return  The neutron port ID if found.
     *          {@code null} if not found.
     */
    static String getPortId(@Nonnull OvsdbTerminationPointAugmentation ovtp) {
        List<InterfaceExternalIds> extIds = ovtp.getInterfaceExternalIds();
        String portId = null;
        if (extIds != null) {
            for (InterfaceExternalIds extId: extIds) {
                String key = extId.getExternalIdKey();
                if (EXTID_INTERFACE_ID.equals(key)) {
                    portId = extId.getExternalIdValue();
                    break;
                }
            }
        }

        return portId;
    }

    /**
     * Determine whether the specified OVSDB termination point has been changed
     * or not.
     *
     * <p>
     *   This method compares fields in the OVSDB termination point that affect
     *   port mapping.
     * </p>
     *
     * @param before  The OVSDB termination point before modification.
     * @param after   The OVSDB termination point after modification.
     * @return  {@code true} if the specified OVSDB termination point is
     *          changed. {@code null} otherwise.
     */
    static boolean isChanged(@Nonnull OvsdbTerminationPointAugmentation before,
                             @Nonnull OvsdbTerminationPointAugmentation after) {
        // Compare external interface ID.
        boolean changed = !Objects.equals(getPortId(before), getPortId(after));
        if (!changed) {
            // Compare OpenFlow port ID and port name.
            changed = !(Objects.equals(before.getOfport(), after.getOfport()) &&
                        Objects.equals(before.getName(), after.getName()));
        }

        return changed;
    }

    /**
     * Return the OpenFlow node associated with the given OVSDB topology node.
     *
     * @param ovh   An {@link OVSDBEventHandler} instance.
     * @param node  A {@link Node} instance.
     * @return  An {@link OfNode} instance on success.
     *          {@code null} on error.
     */
    static OfNode getOfNode(
        @Nonnull OVSDBEventHandler ovh, @Nonnull Node node) {
        OfNode ofnode = null;
        OvsdbBridgeAugmentation ovbr = getBridgeNode(ovh, node);
        if (ovbr != null) {
            DatapathId dpid = ovbr.getDatapathId();
            try {
                ofnode = new OfNode(dpid);
            } catch (RuntimeException e) {
                String msg = "Ignore invalid DPID: node=" +
                    node.getNodeId() + ", dpid=" + dpid;
                LOG.warn(msg, e);
            }
        }

        return ofnode;
    }

    /**
     * Return the OVSDB bridge augmentation in the given node.
     *
     * @param ovh   An {@link OVSDBEventHandler} instance.
     * @param node  A {@link Node} instance.
     * @return  The OVSDB bridge augmentation if found.
     *          {@code null} if not found.
     */
    static OvsdbBridgeAugmentation getBridgeNode(
        @Nonnull OVSDBEventHandler ovh, @Nonnull Node node) {
        OvsdbBridgeAugmentation ovbr =
            node.getAugmentation(OvsdbBridgeAugmentation.class);
        OvsdbBridgeAugmentation result = null;
        if (ovbr != null) {
            OvsdbBridgeName name = ovbr.getBridgeName();
            if (name != null &&
                ovh.getOvsdbBridgeName().equals(name.getValue())) {
                result = ovbr;
            } else {
                LOG.trace("Ignore OVSDB bridge: node={}, bridge={}",
                          node.getNodeId(), ovbr);
            }
        }

        return result;
    }

    /**
     * Construct a new instance.
     *
     * @param ovh   An {@link OVSDBEventHandler} instance.
     * @param txh   The read-only transaction holder.
     * @param node  The target node.
     */
    private OvsdbNodeChange(@Nonnull OVSDBEventHandler ovh,
                            @Nonnull ReadTransactionHolder txh,
                            @Nonnull Node node) {
        eventHandler = ovh;
        txHolder = txh;
        targetNode = node;
    }

    /**
     * Apply all the changes to the virtual network configuration.
     */
    public void apply() {
        // Apply the created OVSDB node augmentation.
        if (createdOvsdbNode != null) {
            eventHandler.nodeAdded(targetNode, createdOvsdbNode);
        }

        // Apply the port mappings to be deconfigured.
        for (Port port: unmapPorts) {
            eventHandler.deletePortMap(port);
        }

        // Apply the port mappings to be configured.
        if (openflowNode != null) {
            for (PortMapAttr attr: mapPorts) {
                attr.apply(eventHandler, openflowNode);
            }
        }

        // Apply the removed OVSDB node augmentation.
        if (ovsdbNodeRemoved) {
            eventHandler.nodeRemoved(targetNode);
        }
    }

    /**
     * Set up information about the created OVSDB node.
     *
     * @param node    A topology node that contains the given OVSDB node
     *                augmentation.
     * @param ovnode  The created OVSDB node augmentation.
     */
    private void setCreatedOvsdbNode(@Nonnull Node node,
                                     @Nonnull OvsdbNodeAugmentation ovnode) {
        createdOvsdbNode = ovnode;
        openflowNode = getOfNode(eventHandler, targetNode);
        LOG.info("OVSDB node has been created: id={}, ofnode={}",
                 node.getNodeId().getValue(), openflowNode);
        if (openflowNode != null) {
            // Establish port mappings between existing neutron ports and
            // OVSDB termination points.
            setMapPorts(targetNode);
        }
    }

    /**
     * Turn on the flag that indicates the OVSDB node augmentation is removed.
     *
     * @param node  A topology node that contained the removed OVSDB node
     *              augmentation.
     */
    private void setRemovedOvsdbNode(@Nonnull Node node) {
        LOG.info("OVSDB node has been removed: id={}",
                 node.getNodeId().getValue());
        ovsdbNodeRemoved = true;

        // Remove all port mappings in the target node.
        setUnmapPorts(targetNode);
    }

    /**
     * Schedule all the neutron ports associated with the given node to
     * configure port mapping.
     *
     * @param node  The target node.
     */
    private void setMapPorts(@Nonnull Node node) {
        List<TerminationPoint> tps = node.getTerminationPoint();
        if (tps != null) {
            for (TerminationPoint tp: tps) {
                OvsdbTerminationPointAugmentation ovtp = tp.getAugmentation(
                    OvsdbTerminationPointAugmentation.class);
                if (ovtp != null) {
                    setMapPort(ovtp);
                }
            }
        }
    }

    /**
     * Schedule all the neutron ports associated with the given node to
     * deconfigure port mapping.
     *
     * @param node  The target node.
     */
    private void setUnmapPorts(@Nonnull Node node) {
        List<TerminationPoint> tps = node.getTerminationPoint();
        if (tps != null) {
            for (TerminationPoint tp: tps) {
                OvsdbTerminationPointAugmentation ovtp = tp.getAugmentation(
                    OvsdbTerminationPointAugmentation.class);
                if (ovtp != null) {
                    setUnmapPort(ovtp);
                }
            }
        }
    }

    /**
     * Set up information about the neutron port to configure port mapping.
     *
     * @param ovtp  The OVSDB termination point augmentation associated with
     *              the neutron port to configure port mapping.
     */
    private void setMapPort(
        @Nonnull OvsdbTerminationPointAugmentation ovtp) {
        String id = getPortId(ovtp);
        if (id != null) {
            setMapPort(ovtp, id);
        }
    }

    /**
     * Set up information about the neutron port to configure port mapping.
     *
     * @param ovtp  The OVSDB termination point augmentation associated with
     *              the neutron port to configure port mapping.
     * @param id    The identifier of the neutron port associated with the
     *              termination point.
     */
    private void setMapPort(
        @Nonnull OvsdbTerminationPointAugmentation ovtp, @Nonnull String id) {
        Port port = readPort(id);
        if (port != null) {
            mapPorts.add(new PortMapAttr(port, ovtp));
        }
    }

    /**
     * Set up information about the neutron port to deconfigure port mapping.
     *
     * @param ovtp  The OVSDB termination point augmentation associated with
     *              the neutron port to deconfigure port mapping.
     */
    private void setUnmapPort(@Nonnull OvsdbTerminationPointAugmentation ovtp) {
        String id = getPortId(ovtp);
        if (id != null) {
            setUnmapPort(id);
        }
    }

    /**
     * Set up information about the neutron port to deconfigured port mapping.
     *
     * @param id  The UUID associated with the neutron port to deconfigure
     *            port mapping.
     */
    private void setUnmapPort(@Nonnull String id) {
        Port port = readPort(id);
        if (port != null) {
            unmapPorts.add(port);
        }
    }

    /**
     * Create information about the updated node.
     *
     * @param mod     The data object modification that represents changes
     *                to the node subtree.
     * @param before  The target node before modification.
     */
    private void nodeUpdated(
        @Nonnull DataObjectModification<Node> mod, @Nonnull Node before) {
        // Check to see if the target OpenFlow node is changed.
        OfNode oldOfnode = getOfNode(eventHandler, before);
        OfNode newOfnode = getOfNode(eventHandler, targetNode);
        openflowNode = newOfnode;
        if (!Objects.equals(oldOfnode, newOfnode)) {
            LOG.info("OpenFlow node has been changed: {} -> {}",
                     oldOfnode, newOfnode);

            // All the port mappings need to be removed.
            if (oldOfnode != null) {
                setUnmapPorts(before);
            }

            if (newOfnode != null) {
                // Establish port mappings for all the OVSDB termination
                // points.
                setMapPorts(targetNode);
            }
        } else if (newOfnode != null) {
            // Check to see if termination points are updated.
            scanTerminationPoints(mod);
        }
    }

    /**
     * Detect modifications of OVSDB termination points in the given node.
     *
     * @param mod  Modification of node subtree.
     */
    private void scanTerminationPoints(
        @Nonnull DataObjectModification<Node> mod) {
        for (DataObjectModification<?> child: mod.getModifiedChildren()) {
            Class<?> type = child.getDataType();
            if (TerminationPoint.class.equals(type)) {
                @SuppressWarnings("unchecked")
                DataObjectModification<TerminationPoint> tmod =
                    (DataObjectModification<TerminationPoint>)child;
                DataObjectModification<OvsdbTerminationPointAugmentation> omod =
                    tmod.getModifiedAugmentation(
                        OvsdbTerminationPointAugmentation.class);
                if (omod != null) {
                    setModifiedTerminationPoint(omod);
                }
            }
        }
    }

    /**
     * Set up information about the modified OVSDB termination point
     * augmentation.
     *
     * @param mod  Modification of OVSDB termination point augmentation.
     */
    private void setModifiedTerminationPoint(
        @Nonnull DataObjectModification<OvsdbTerminationPointAugmentation> mod) {
        ModificationType modType = mod.getModificationType();
        OvsdbTerminationPointAugmentation before = mod.getDataBefore();
        if (modType == ModificationType.DELETE) {
            if (before == null) {
                LOG.warn("Ignore null deleted OVSDB termination point.");
            } else {
                // The OVSDB termination point has been removed.
                LOG.trace("OVSDB termination point has been removed: {}",
                          before);
                setUnmapPort(before);
            }
        } else {
            OvsdbTerminationPointAugmentation after = mod.getDataAfter();
            if (after == null) {
                LOG.warn("Ignore null updated OVSDB termination point.");
            } else if (before == null) {
                LOG.trace("OVSDB termination point has been created: {}",
                          after);
                setMapPort(after);
            } else if (isChanged(before, after)) {
                LOG.trace("OVSDB termination point has been changed: " +
                          "before={}, after={}", before, after);
                setUnmapPort(before);
                setMapPort(after);
            }
        }
    }

    /**
     * Read the neutron port associated with the given UUID in the
     * configuration datastore.
     *
     * @param id  The UUID for the neutron port.
     * @return  The neutron port if fonud.
     *          {@code null} if not fonud.
     */
    private Port readPort(String id) {
        try {
            Uuid uuid = new Uuid(id);
            InstanceIdentifier<Port> path = InstanceIdentifier.
                builder(Neutron.class).
                child(Ports.class).
                child(Port.class, new PortKey(uuid)).
                build();
            Optional<Port> opt = MdsalUtils.read(
                txHolder.get(), LogicalDatastoreType.CONFIGURATION, path);
            if (opt.isPresent()) {
                return opt.get();
            }
            LOG.debug("Neutron port not found: id={}", id);
        } catch (Exception e) {
            LOG.error("Failed to read neutron port: id=" + id, e);
        }

        return null;
    }
}
