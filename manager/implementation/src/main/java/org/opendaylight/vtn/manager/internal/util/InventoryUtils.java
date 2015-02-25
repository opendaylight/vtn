/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.ArrayList;
import java.util.List;

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

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortFeatures;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.State;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

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
     * Create a VTN port builder associated with the given MD-SAL node
     * connector ID.
     *
     * @param nc  A {@link NodeConnector} instance.
     * @return  A VTN port builder.
     */
    public static VtnPortBuilder toVtnPortBuilder(NodeConnector nc) {
        VtnPortBuilder builder = new VtnPortBuilder();
        builder.setId(nc.getId());

        FlowCapableNodeConnector fcnc =
            nc.getAugmentation(FlowCapableNodeConnector.class);
        String name = null;
        boolean enabled = false;
        PortFeatures pf = null;
        Long curSpeed = null;
        if (fcnc != null) {
            name = fcnc.getName();
            PortConfig pcfg = fcnc.getConfiguration();
            Boolean portDown = null;
            if (pcfg != null) {
                portDown = pcfg.isPORTDOWN();
            }

            State state = fcnc.getState();
            Boolean linkDown = null;
            if (state != null) {
                linkDown = state.isLinkDown();
            }

            if (Boolean.FALSE.equals(portDown) &&
                Boolean.FALSE.equals(linkDown)) {
                enabled = true;
            }

            pf = fcnc.getCurrentFeature();
            curSpeed = fcnc.getCurrentSpeed();
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
            name = nc.getId().getValue();
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
            if (sport != null) {
                VtnPort vport = toVtnPortBuilder(nc).build();
                list.add(vport);
                if (version == null) {
                    version = getOpenflowVersion(nc);
                }
            }
        }

        return (list.isEmpty()) ? null : version;
    }

    /**
     * Estimate the OpenFlow protocol version from the given
     * {@link NodeConnector} instance.
     *
     * @param nc  A {@link NodeConnector} instance.
     * @return  Estimated OpenFlow protocol version number.
     */
    public static VtnOpenflowVersion getOpenflowVersion(NodeConnector nc) {
        FlowCapableNodeConnector fcnc =
            nc.getAugmentation(FlowCapableNodeConnector.class);
        if (fcnc != null) {
            if (fcnc.getCurrentSpeed() != null) {
                // OpenFlow 1.3 PORT_STATUS message contains the current
                // link speed of the port, but OpenFlow 1.0 does not.
                return VtnOpenflowVersion.OF13;
            }
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
        if (pf != null) {
            if (Boolean.TRUE.equals(pf.isTenMbHd()) ||
                Boolean.TRUE.equals(pf.isTenMbFd())) {
                return LINK_SPEED_10M;
            }

            if (Boolean.TRUE.equals(pf.isHundredMbHd()) ||
                Boolean.TRUE.equals(pf.isHundredMbFd())) {
                return LINK_SPEED_100M;
            }

            if (Boolean.TRUE.equals(pf.isTenGbFd())) {
                return LINK_SPEED_10G;
            }

            if (Boolean.TRUE.equals(pf.isFortyGbFd())) {
                return LINK_SPEED_40G;
            }

            if (Boolean.TRUE.equals(pf.isHundredGbFd())) {
                return LINK_SPEED_100G;
            }

            if (Boolean.TRUE.equals(pf.isOneTbFd())) {
                return LINK_SPEED_1T;
            }
        }

        // Use 1Gbps as default.
        return LINK_SPEED_1G;
    }
}
