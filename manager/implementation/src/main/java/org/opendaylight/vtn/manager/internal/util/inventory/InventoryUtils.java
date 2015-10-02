/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.ArrayList;
import java.util.List;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchLink;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortFeatures;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.State;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.LinkKey;

/**
 * {@code InventoryUtils} class is a collection of utility class methods
 * for MD-SAL based inventory management.
 */
public final class InventoryUtils {
    /**
     * 1Kbps link speed.
     */
    private static final long  LINK_SPEED_1K = 1000L;

    /**
     * 10Mbps link speed.
     */
    private static final long  LINK_SPEED_10M = 10000000L;

    /**
     * 100Mbps link speed.
     */
    private static final long  LINK_SPEED_100M = LINK_SPEED_10M * 10L;

    /**
     * 1Gbps link speed.
     */
    private static final long  LINK_SPEED_1G = LINK_SPEED_100M * 10L;

    /**
     * 10Gbps link speed.
     */
    private static final long  LINK_SPEED_10G = LINK_SPEED_1G * 10L;

    /**
     * 40Gbps link speed.
     */
    private static final long  LINK_SPEED_40G = LINK_SPEED_10G * 4L;

    /**
     * 100Gbps link speed.
     */
    private static final long  LINK_SPEED_100G = LINK_SPEED_10G * 10L;

    /**
     * 1Tbps link speed.
     */
    private static final long  LINK_SPEED_1T = LINK_SPEED_100G * 10L;

    /**
     * Base link speed used to calculate the link cost.
     */
    private static final long  LINK_SPEED_BASE = LINK_SPEED_1T * 10L;

    /**
     * The minimum value of the link cost.
     */
    private static final long  LINK_COST_MIN = 1L;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private InventoryUtils() {}

    /**
     * Determine whether the given VTN port has at least one inter-switch link
     * or not.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  {@code true} only if the given port has at least one
     *          inter-switch link.
     */
    public static boolean hasPortLink(VtnPort vport) {
        List<PortLink> plinks = vport.getPortLink();
        return (plinks != null && !plinks.isEmpty());
    }

    /**
     * Determine whether the given VTN port is an edge port or not.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  {@code true} only if the given port is an edge port.
     */
    public static boolean isEdge(VtnPort vport) {
        return !hasPortLink(vport);
    }

    /**
     * Determine whether the given VTN port is enabled or not.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  {@code true} only if the given port is enabled.
     */
    public static boolean isEnabled(VtnPort vport) {
        return Boolean.TRUE.equals(vport.isEnabled());
    }

    /**
     * Determine whether the given VTN port is an enabled edge port or not.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  {@code true} only if the given port is an enabled edge port.
     */
    public static boolean isEnabledEdge(VtnPort vport) {
        return isEnabled(vport) && isEdge(vport);
    }

    /**
     * Determine whether the given node has at least one edge port in up state
     * or not.
     *
     * @param vnode  A {@link VtnNode} instance.
     * @return  {@code true} is returned if the given node has at least one
     *          edge port in up state. Otherwise {@code false} is returned.
     */
    public static boolean hasEdgePort(VtnNode vnode) {
        if (vnode != null) {
            List<VtnPort> ports = vnode.getVtnPort();
            if (ports != null) {
                for (VtnPort vport: ports) {
                    if (isEnabledEdge(vport)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /**
     * Return a string representation of the given VTN port.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  A string representation of the given VTN port.
     */
    public static String toString(VtnPort vport) {
        NodeConnectorId ncid = vport.getId();
        String id = (ncid == null) ? null : ncid.getValue();
        StringBuilder builder = new StringBuilder("{id=");
        builder.append(id).
            append(", name=").append(vport.getName()).
            append(", enabled=").append(vport.isEnabled()).
            append(", cost=").append(vport.getCost()).
            append(", links=");

        List<PortLink> plinks = vport.getPortLink();
        if (plinks == null || plinks.isEmpty()) {
            builder.append("none");
        } else {
            String sep = null;
            builder.append('[');
            for (PortLink plink: plinks) {
                if (sep != null) {
                    builder.append(sep);
                }

                LinkId lid = plink.getLinkId();
                String lidStr = (lid == null) ? null : lid.getValue();
                NodeConnectorId peer = plink.getPeer();
                String peerStr = (peer == null) ? null : peer.getValue();
                builder.append("{id=").append(lidStr).
                    append(", peer=").append(peerStr).append("}");
                sep = ", ";
            }
            builder.append(']');
        }

        return builder.append('}').toString();
    }

    /**
     * Return a string representation of the given link information.
     *
     * @param vslink  A {@link VtnSwitchLink} instance.
     * @return  A string representation of the given link.
     */
    public static String toString(VtnSwitchLink vslink) {
        StringBuilder builder = new StringBuilder("{").
            append(MiscUtils.getValue(vslink.getSource())).
            append(" -> ").
            append(MiscUtils.getValue(vslink.getDestination())).
            append('}');
        return builder.toString();
    }

    /**
     * Return a string representation of the given VTN link information.
     *
     * @param vlink  A {@link VtnLink} instance.
     * @return  A string representation of the given link.
     */
    public static String toString(VtnLink vlink) {
        NodeConnectorId src = vlink.getSource();
        NodeConnectorId dst = vlink.getDestination();
        Boolean st = vlink.isStaticLink();
        if (st == null) {
            st = Boolean.FALSE;
        }
        StringBuilder builder = new StringBuilder("{id=").
            append(MiscUtils.getValue(vlink.getLinkId())).
            append(", src=").append(MiscUtils.getValue(src)).
            append(", dst=").append(MiscUtils.getValue(dst)).
            append(", static=").append(st).
            append('}');
        return builder.toString();
    }

    /**
     * Convert a given topology link ID into an instance identifier of
     * VTN topology link.
     *
     * @param lid  A topology link ID.
     * @return  An instance identifier for a VTN topology link.
     */
    public static InstanceIdentifier<VtnLink> toVtnLinkIdentifier(LinkId lid) {
        return InstanceIdentifier.builder(VtnTopology.class).
            child(VtnLink.class, new VtnLinkKey(lid)).build();
    }

    /**
     * Convert a given topology link ID into an instance identifier of
     * ignored inter-switch link.
     *
     * @param lid  A topology link ID.
     * @return  An instance identifier for a ignored link.
     */
    public static InstanceIdentifier<IgnoredLink> toIgnoredLinkIdentifier(
        LinkId lid) {
        return InstanceIdentifier.builder(IgnoredLinks.class).
            child(IgnoredLink.class, new IgnoredLinkKey(lid)).build();
    }

    /**
     * Convert the given switch port ID into an instance identifier for a
     * static inter-switch link.
     *
     * @param sport  A {@link SalPort} instance.
     * @return  An instance identifier for a static inter-switch link.
     */
    public static InstanceIdentifier<StaticSwitchLink> toStaticSwitchLinkIdentifier(
        SalPort sport) {
        NodeConnectorId ncId = sport.getNodeConnectorId();
        return InstanceIdentifier.builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            child(StaticSwitchLink.class, new StaticSwitchLinkKey(ncId)).
            build();
    }

    /**
     * Convert the given switch port ID into an instance identifier for a
     * static edge port.
     *
     * @param sport  A {@link SalPort} instance.
     * @return  An instance identifier for a static edge port.
     */
    public static InstanceIdentifier<StaticEdgePort> toStaticEdgePortIdentifier(
        SalPort sport) {
        NodeConnectorId ncId = sport.getNodeConnectorId();
        return InstanceIdentifier.builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
            build();
    }

    /**
     * Create a VTN node builder associated with the given MD-SAL node.
     *
     * @param node  A MD-SAL node.
     * @return  A VTN node builder.
     */
    public static VtnNodeBuilder toVtnNodeBuilder(Node node) {
        VtnNodeBuilder builder = new VtnNodeBuilder();
        List<VtnPort> vports = new ArrayList<>();
        VtnOpenflowVersion version = getVtnPorts(vports, node);
        if (version != null) {
            builder.setVtnPort(vports).setOpenflowVersion(version);
        }

        return builder.setId(node.getId());
    }

    /**
     * Create a VTN node builder associated with the given MD-SAL node ID.
     *
     * @param nid  A MD-SAL node ID.
     * @return  A VTN node builder.
     */
    public static VtnNodeBuilder toVtnNodeBuilder(NodeId nid) {
        return new VtnNodeBuilder().setId(nid);
    }

    /**
     * Create a VTN port builder associated with the given MD-SAL node
     * connector ID.
     *
     * @param ncid  A MD-SAL node connector ID.
     * @param fcnc  A {@link FlowCapableNodeConnector} instance.
     * @return  A VTN port builder.
     */
    public static VtnPortBuilder toVtnPortBuilder(
        NodeConnectorId ncid, FlowCapableNodeConnector fcnc) {
        VtnPortBuilder builder = new VtnPortBuilder().setId(ncid);
        String name = fcnc.getName();
        PortFeatures pf = fcnc.getCurrentFeature();
        Long curSpeed = fcnc.getCurrentSpeed();
        PortConfig pcfg = fcnc.getConfiguration();
        Boolean portDown = (pcfg == null) ? null : pcfg.isPORTDOWN();
        State state = fcnc.getState();
        Boolean linkDown = (state == null) ? null : state.isLinkDown();

        boolean enabled = false;
        if (Boolean.FALSE.equals(portDown) && Boolean.FALSE.equals(linkDown)) {
            enabled = true;
        }

        // Determine the cost of the link from the link speed.
        long speed;
        if (curSpeed == null) {
            speed = getLinkSpeed(pf);
        } else {
            speed = curSpeed.longValue() * LINK_SPEED_1K;
            if (speed <= 0) {
                speed = getLinkSpeed(pf);
            }
        }

        long cost = LINK_SPEED_BASE / speed;
        if (cost < LINK_COST_MIN) {
            cost = LINK_COST_MIN;
        }

        if (name == null) {
            // Port name is unavailable.
            // Use node-connector-id instead.
            name = ncid.getValue();
        }

        builder.setName(name).setEnabled(Boolean.valueOf(enabled)).
            setCost(Long.valueOf(cost));

        return builder;
    }

    /**
     * Create a VTN link builder associated with the given inter-switch link.
     *
     * @param lid  A {@link LinkId} instance which represents an inter-switch
     *             link ID.
     * @param src  A {@link SalPort} instance which represents the source
     *             port of the inter-switch link.
     * @param dst  A {@link SalPort} instance which represents the destination
     *             port of the inter-switch link.
     * @return  A VTN link builder.
     */
    public static VtnLinkBuilder toVtnLinkBuilder(LinkId lid, SalPort src,
                                                  SalPort dst) {
        VtnLinkBuilder builder = new VtnLinkBuilder();
        return builder.setLinkId(lid).setSource(src.getNodeConnectorId()).
            setDestination(dst.getNodeConnectorId());
    }

    /**
     * Create an ignored link builder associated with the given inter-switch
     * link.
     *
     * @param lid  A {@link LinkId} instance which represents an inter-switch
     *             link ID.
     * @param src  A {@link SalPort} instance which represents the source
     *             port of the inter-switch link.
     * @param dst  A {@link SalPort} instance which represents the destination
     *             port of the inter-switch link.
     * @return  An ignored link builder.
     */
    public static IgnoredLinkBuilder toIgnoredLinkBuilder(LinkId lid,
                                                          SalPort src,
                                                          SalPort dst) {
        IgnoredLinkBuilder builder = new IgnoredLinkBuilder();
        return builder.setLinkId(lid).setSource(src.getNodeConnectorId()).
            setDestination(dst.getNodeConnectorId());
    }

    /**
     * Create a port link builder associated with the given inter-switch
     * link.
     *
     * @param lid   A {@link LinkId} instance which represents an inter-switch
     *              link ID.
     * @param peer  A {@link SalPort} instance which represents the peer of
     *              the inter-switch link.
     * @return  A port link builder.
     */
    public static PortLinkBuilder toPortLinkBuilder(LinkId lid, SalPort peer) {
        PortLinkBuilder builder = new PortLinkBuilder();
        NodeConnectorId peerId = peer.getNodeConnectorId();
        return builder.setLinkId(lid).setPeer(peerId);
    }


    /**
     * Return a list of VTN ports associated with the switch ports in the
     * given MD-SAL node.
     *
     * @param list  A list of {@link VtnPort} to store results.
     * @param node  A MD-SAL node.
     * @return  Estimated OpenFlow protocol version.
     *          {@code null} is returned if no port is created.
     */
    public static VtnOpenflowVersion getVtnPorts(List<VtnPort> list,
                                                 Node node) {
        List<NodeConnector> connectors = node.getNodeConnector();
        if (connectors == null || connectors.isEmpty()) {
            return null;
        }

        VtnOpenflowVersion version = null;
        for (NodeConnector nc: connectors) {
            NodeConnectorId id = nc.getId();
            SalPort sport = SalPort.create(id);
            if (sport == null) {
                continue;
            }

            FlowCapableNodeConnector fcnc =
                nc.getAugmentation(FlowCapableNodeConnector.class);
            if (fcnc != null) {
                VtnPort vport = toVtnPortBuilder(id, fcnc).build();
                list.add(vport);
                if (version == null) {
                    version = getOpenflowVersion(fcnc);
                }
            }
        }

        return (list.isEmpty()) ? null : version;
    }

    /**
     * Estimate the OpenFlow protocol version from the given
     * {@link NodeConnector} instance.
     *
     * @param fcnc  A {@link FlowCapableNodeConnector} instance.
     * @return  Estimated OpenFlow protocol version number.
     */
    public static VtnOpenflowVersion getOpenflowVersion(
        FlowCapableNodeConnector fcnc) {
        if (fcnc != null && fcnc.getCurrentSpeed() != null) {
            // OpenFlow 1.3 PORT_STATUS message contains the current
            // link speed of the port, but OpenFlow 1.0 does not.
            return VtnOpenflowVersion.OF13;
        }

        return VtnOpenflowVersion.OF10;
    }

    /**
     * Determine the link speed from the current port feature.
     *
     * @param pf  A {@link PortFeatures} instance.
     * @return  A link speed in bps.
     */
    public static long getLinkSpeed(PortFeatures pf) {
        // Use 1Gbps as default.
        long speed = LINK_SPEED_1G;

        if (pf != null) {
            if (Boolean.TRUE.equals(pf.isTenMbHd()) ||
                Boolean.TRUE.equals(pf.isTenMbFd())) {
                speed = LINK_SPEED_10M;
            } else if (Boolean.TRUE.equals(pf.isHundredMbHd()) ||
                       Boolean.TRUE.equals(pf.isHundredMbFd())) {
                speed = LINK_SPEED_100M;
            } else if (Boolean.TRUE.equals(pf.isTenGbFd())) {
                speed = LINK_SPEED_10G;
            } else if (Boolean.TRUE.equals(pf.isFortyGbFd())) {
                speed = LINK_SPEED_40G;
            } else if (Boolean.TRUE.equals(pf.isHundredGbFd())) {
                speed = LINK_SPEED_100G;
            } else if (Boolean.TRUE.equals(pf.isOneTbFd())) {
                speed = LINK_SPEED_1T;
            }
        }

        return speed;
    }

    /**
     * Return a MD-SAL node ID in the given instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  A MD-SAL node ID if found.
     *          {@code null} if not found.
     */
    public static NodeId getNodeId(InstanceIdentifier<?> path) {
        NodeKey key = path.firstKeyOf(Node.class);
        return (key == null) ? null : key.getId();
    }

    /**
     * Return a MD-SAL node connector ID in the given instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  A MD-SAL node connector ID if found.
     *          {@code null} if not found.
     */
    public static NodeConnectorId getNodeConnectorId(
        InstanceIdentifier<?> path) {
        NodeConnectorKey key = path.firstKeyOf(NodeConnector.class);
        return (key == null) ? null : key.getId();
    }

    /**
     * Return a MD-SAL inter-switch link ID in the given instance identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  A MD-SAL inter-switch link ID if found.
     *          {@code null} if not found.
     */
    public static LinkId getLinkId(InstanceIdentifier<?> path) {
        LinkKey key = path.firstKeyOf(Link.class);
        return (key == null) ? null : key.getLinkId();
    }

    /**
     * Remove all VTN links affected by the the removed VTN node.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param snode  A {@link SalNode} instance corresponding to the removed
     *               VTN node.
     * @throws VTNException  An error occurred.
     */
    public static void removeVtnLink(ReadWriteTransaction tx, SalNode snode)
        throws VTNException {
        removeVtnTopologyLink(tx, snode);
        removeIgnoredLink(tx, snode);
    }

    /**
     * Remove all VTN links in vtn-topology affected by the removed VTN node.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param snode  A {@link SalNode} instance corresponding to the removed
     *               VTN node.
     * @throws VTNException  An error occurred.
     */
    public static void removeVtnTopologyLink(ReadWriteTransaction tx,
                                             SalNode snode)
        throws VTNException {
        InstanceIdentifier<VtnTopology> topoPath =
            InstanceIdentifier.create(VtnTopology.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnTopology> opt = DataStoreUtils.read(tx, oper, topoPath);
        if (!opt.isPresent()) {
            return;
        }

        List<VtnLink> links = opt.get().getVtnLink();
        if (links == null) {
            return;
        }

        long dpid = snode.getNodeNumber();
        for (VtnLink vlink: links) {
            LinkId lid = vlink.getLinkId();
            SalPort src = SalPort.create(vlink.getSource());
            SalPort dst = SalPort.create(vlink.getDestination());
            long srcDpid = src.getNodeNumber();
            long dstDpid = dst.getNodeNumber();

            boolean rmLink = false;
            if (srcDpid == dpid) {
                rmLink = true;
                if (dstDpid != dpid) {
                    removePortLink(tx, dst, lid);
                }
            } else if (dstDpid == dpid) {
                rmLink = true;
                removePortLink(tx, src, lid);
            }

            if (rmLink) {
                InstanceIdentifier<VtnLink> lpath = toVtnLinkIdentifier(lid);
                tx.delete(oper, lpath);
            }
        }
    }

    /**
     * Remove all VTN links in ignored-links affected by the removed VTN node.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param snode  A {@link SalNode} instance corresponding to the removed
     *               VTN node.
     * @throws VTNException  An error occurred.
     */
    public static void removeIgnoredLink(ReadWriteTransaction tx,
                                         SalNode snode) throws VTNException {
        InstanceIdentifier<IgnoredLinks> igPath =
            InstanceIdentifier.create(IgnoredLinks.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<IgnoredLinks> opt = DataStoreUtils.read(tx, oper, igPath);
        if (!opt.isPresent()) {
            return;
        }

        List<IgnoredLink> links = opt.get().getIgnoredLink();
        if (links == null) {
            return;
        }

        long dpid = snode.getNodeNumber();
        for (IgnoredLink vlink: links) {
            LinkId lid = vlink.getLinkId();
            SalPort src = SalPort.create(vlink.getSource());
            SalPort dst = SalPort.create(vlink.getDestination());
            long srcDpid = src.getNodeNumber();
            long dstDpid = dst.getNodeNumber();

            if (srcDpid == dpid || dstDpid == dpid) {
                InstanceIdentifier<IgnoredLink> lpath =
                    InventoryUtils.toIgnoredLinkIdentifier(lid);
                tx.delete(LogicalDatastoreType.OPERATIONAL, lpath);
            }
        }
    }

    /**
     * Remove all VTN links affected by the removed VTN port.
     *
     * <p>
     *   Note that this method does not remove port links configured in the
     *   given VTN port.
     * </p>
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param vport  A {@link VtnPort} instance corresponding to the removed
     *               VTN port.
     * @throws VTNException  An error occurred.
     */
    public static void removeVtnLink(ReadWriteTransaction tx, VtnPort vport)
        throws VTNException {
        List<PortLink> links = vport.getPortLink();
        if (links == null) {
            return;
        }

        for (PortLink plink: links) {
            LinkId lid = plink.getLinkId();
            NodeConnectorId peer = plink.getPeer();
            SalPort p = SalPort.create(peer);
            removePortLink(tx, p, lid);

            InstanceIdentifier<VtnLink> lpath = toVtnLinkIdentifier(lid);
            DataStoreUtils.delete(tx, LogicalDatastoreType.OPERATIONAL, lpath);
        }
    }

    /**
     * Remove the specified port link.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param sport  A {@link SalPort} instance corresponding to a VTN port.
     * @param lid    A {@link LinkId} that specifies inter-switch link to be
     *               removed.
     * @throws VTNException  An error occurred.
     */
    public static void removePortLink(ReadWriteTransaction tx, SalPort sport,
                                      LinkId lid) throws VTNException {
        InstanceIdentifier<PortLink> path = sport.getPortLinkIdentifier(lid);
        DataStoreUtils.delete(tx, LogicalDatastoreType.OPERATIONAL, path);
    }


    /**
     * Remove all the VTN links configured in the given VTN port.
     *
     * @param tx     A {@link ReadWriteTransaction} instance.
     * @param sport  A {@link SalPort} instance which specifies the target
     *               VTN port.
     * @param vport  A {@link VtnPort} instance associated with {@code sport}.
     * @throws VTNException  An error occurred.
     */
    public static void clearPortLink(ReadWriteTransaction tx, SalPort sport,
                                     VtnPort vport) throws VTNException {
        List<PortLink> links = vport.getPortLink();
        if (links == null) {
            return;
        }

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        for (PortLink plink: links) {
            LinkId lid = plink.getLinkId();
            InstanceIdentifier<PortLink> path =
                sport.getPortLinkIdentifier(lid);
            tx.delete(oper, path);

            SalPort peer = SalPort.create(plink.getPeer());
            removePortLink(tx, peer, lid);

            InstanceIdentifier<VtnLink> lpath = toVtnLinkIdentifier(lid);
            DataStoreUtils.delete(tx, oper, lpath);
        }
    }
}
