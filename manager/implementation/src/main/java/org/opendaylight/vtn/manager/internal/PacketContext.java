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
import java.util.EnumSet;
import java.util.Set;
import java.util.HashSet;
import java.util.Map;
import java.util.LinkedHashMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.packet.CachedPacket;
import org.opendaylight.vtn.manager.internal.packet.EtherPacket;
import org.opendaylight.vtn.manager.internal.packet.IcmpPacket;
import org.opendaylight.vtn.manager.internal.packet.Inet4Packet;
import org.opendaylight.vtn.manager.internal.packet.TcpPacket;
import org.opendaylight.vtn.manager.internal.packet.UdpPacket;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.ICMP;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.TCP;
import org.opendaylight.controller.sal.packet.UDP;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code PacketContext} class describes the context of received packet.
 *
 * <p>
 *   This class is designed to be used by a single thread.
 * </p>
 */
public class PacketContext {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PacketContext.class);

    /**
     * Bitmask which represents valid ethernet type.
     */
    private static final int  ETHER_TYPE_MASK = 0xffff;

    /**
     * A received raw packet.
     */
    private final RawPacket  rawPacket;

    /**
     * Decoded ethernet frame.
     */
    private final EtherPacket  etherFrame;

    /**
     * Source IP address.
     */
    private byte[] sourceInetAddress;

    /**
     * Obsolete layer 2 host entries.
     */
    private final Set<ObjectPair<MacVlan, NodeConnector>>  obsoleteHosts =
        new HashSet<ObjectPair<MacVlan, NodeConnector>>();

    /**
     * Outgoing node connector.
     */
    private NodeConnector  outgoing;

    /**
     * Set of virtual node paths to be associated with the data flow.
     */
    private final Set<VTenantPath>  virtualNodes = new HashSet<VTenantPath>();

    /**
     * A sequence of virtual packet routing.
     */
    private final Map<VNodePath, VNodeRoute>  virtualRoute =
        new LinkedHashMap<VNodePath, VNodeRoute>();

    /**
     * A set of {@link MatchType} instances which represents match fields
     * to be configured.
     */
    private final EnumSet<MatchType>  matchFields =
        EnumSet.noneOf(MatchType.class);

    /**
     * A path to the virtual node that established the egress flow.
     */
    private VNodePath  egressNodePath;

    /**
     * An {@link Inet4Packet} instance which represents the IPv4 packet in the
     * payload.
     */
    private Inet4Packet  inet4Packet;

    /**
     * A {@link CachedPacket} instance which represents the layer 4 protocol
     * data.
     */
    private CachedPacket  l4Packet;

    /**
     * Route resolver for this packet.
     */
    private RouteResolver  routeResolver;

    /**
     * Idle timeout for this flow.
     */
    private int  idleTimeout;

    /**
     * Hard timeout for this flow.
     */
    private int  hardTimeout;

    /**
     * Construct a new packet context.
     *
     * @param raw    A received raw packet.
     * @param ether  Decoded ethernet frame.
     */
    PacketContext(RawPacket raw, Ethernet ether) {
        rawPacket = raw;
        etherFrame = new EtherPacket(ether);
    }

    /**
     * Construct a new packet context from the specified ethernet frame.
     *
     * @param ether     An ethernet frame.
     * @param out       Outgoing node connector.
     */
    PacketContext(Ethernet ether, NodeConnector out) {
        this(null, ether);
        outgoing = out;
    }

    /**
     * Return an ethernet frame in this context.
     *
     * @return  An ethernet frame.
     */
    public Ethernet getFrame() {
        return etherFrame.getPacket();
    }

    /**
     * Return an {@link EtherPacket} instance which represents the Ethernet
     * frame.
     *
     * @return  An {@link EtherPacket} instance.
     */
    public EtherPacket getEtherPacket() {
        return etherFrame;
    }

    /**
     * Return the source MAC address.
     *
     * @return  The source MAC address.
     */
    public byte[] getSourceAddress() {
        return etherFrame.getSourceAddress();
    }

    /**
     * Return the destination MAC address.
     *
     * @return  The destination MAC address.
     */
    public byte[] getDestinationAddress() {
        return etherFrame.getDestinationAddress();
    }

    /**
     * Return the IP address associated with the source MAC address.
     *
     * @return  The source IP address.
     *          {@code null} is returned if no IP header was found.
     */
    public byte[] getSourceIpAddress() {
        byte[] sip = sourceInetAddress;
        if (sip == null) {
            Packet payload = etherFrame.getPayload();
            if (payload instanceof ARP) {
                ARP arp = (ARP)payload;
                if (arp.getProtocolType() == EtherTypes.IPv4.shortValue()) {
                    // Ignore sender protocol address if it is zero.
                    byte[] sender = arp.getSenderProtocolAddress();
                    if (NetUtils.byteArray4ToInt(sender) != 0) {
                        sourceInetAddress = sender;
                        sip = sender;
                    }
                }
            }

            if (sip == null) {
                sourceInetAddress = new byte[0];
            }
        } else if (sip.length == 0) {
            // Negative cache.
            sip = null;
        }

        return sip;
    }

    /**
     * Return a received raw packet.
     *
     * @return  A raw packet.
     */
    public RawPacket getRawPacket() {
        return rawPacket;
    }

    /**
     * Return the payload in the received ethernet frame.
     *
     * @return  A payload.
     */
    public Packet getPayload() {
        return etherFrame.getPayload();
    }

    /**
     * Return the node connector where the packet was received.
     *
     * @return  A incoming node connector.
     */
    public NodeConnector getIncomingNodeConnector() {
        return rawPacket.getIncomingNodeConnector();
    }

    /**
     * Return a pair of switch port and VLAN ID which indicates the incoming
     * network.
     *
     * @return  A {@link PortVlan} which indicates the incoming network where
     *          the packet was received from.
     */
    public PortVlan getIncomingNetwork() {
        if (rawPacket == null) {
            return null;
        }

        NodeConnector nc = rawPacket.getIncomingNodeConnector();
        return new PortVlan(nc, getVlan());
    }

    /**
     * Return a node connector associated with the switch port to which the
     * packet sends.
     *
     * @return  Outgoing node connector.
     */
    public NodeConnector getOutgoingNodeConnector() {
        return outgoing;
    }

    /**
     * Return VLAN ID of this packet.
     *
     * @return  VLAN ID. Zero is returned if no VLAN tag was found in the
     *          packet.
     */
    public short getVlan() {
        return etherFrame.getVlan();
    }

    /**
     * Create a new ethernet frame which forwards the received packet.
     *
     * @param vlan  VLAN ID for a new frame.
     *              Zero means that the VLAN tag should not be added.
     * @return  A new ethernet frame.
     */
    public Ethernet createFrame(short vlan) {
        Ethernet ether = new Ethernet();
        ether.setSourceMACAddress(etherFrame.getSourceAddress()).
            setDestinationMACAddress(etherFrame.getDestinationAddress());

        short ethType = (short)etherFrame.getEtherType();
        IEEE8021Q vlanTag = etherFrame.getVlanTag();
        Packet payload = etherFrame.getPayload();
        if (vlan != 0 || (vlanTag != null && vlanTag.getVid() == 0)) {
            // Add a VLAN tag.
            // We don't strip VLAN tag with zero VLAN ID because PCP field
            // in the VLAN tag should affect even if the VLAN ID is zero.
            IEEE8021Q tag = new IEEE8021Q();
            byte cfi, pcp;
            if (vlanTag != null) {
                cfi = vlanTag.getCfi();
                pcp = vlanTag.getPcp();
            } else {
                cfi = (byte)0;
                pcp = (byte)0;
            }
            tag.setCfi(cfi).setPcp(pcp).setVid(vlan).setEtherType(ethType);
            ether.setEtherType(EtherTypes.VLANTAGGED.shortValue());

            // Set payload to IEEE 802.1Q header.
            if (payload != null) {
                tag.setPayload(payload);
            } else {
                byte[] rawPayload = etherFrame.getRawPayload();
                if (rawPayload != null) {
                    tag.setRawPayload(rawPayload);
                }
            }

            // Set IEEE 802.1Q header as payload.
            ether.setPayload(tag);
        } else {
            ether.setEtherType(ethType);
            if (payload != null) {
                ether.setPayload(payload);
            } else {
                byte[] rawPayload = etherFrame.getRawPayload();
                if (rawPayload != null) {
                    ether.setRawPayload(rawPayload);
                }
            }
        }

        return ether;
    }

    /**
     * Add an obsolete layer 2 host entry specified by the MAC address table
     * entry.
     *
     * @param tent  An obsolete MAC address table entry.
     */
    public void addObsoleteEntry(MacTableEntry tent) {
        long mac = tent.getMacAddress();
        MacVlan mvlan = new MacVlan(mac, tent.getVlan());
        obsoleteHosts.add(new ObjectPair<MacVlan, NodeConnector>
                          (mvlan, tent.getPort()));
    }

    /**
     * Return a set of obsolete layer 2 host entries.
     *
     * @return  A set of obsolete layer 2 host entries.
     */
    public Set<ObjectPair<MacVlan, NodeConnector>> getObsoleteEntries() {
        return obsoleteHosts;
    }


    /**
     * Purge VTN flows relevant to obsolete layer 2 host entries.
     *
     * @param mgr         VTN manager service.
     * @param tenantName  Name of the virtual tenant.
     */
    public void purgeObsoleteFlow(VTNManagerImpl mgr, String tenantName) {
        VTNFlowDatabase fdb = mgr.getTenantFlowDB(tenantName);
        if (fdb != null) {
            purgeObsoleteFlow(mgr, fdb);
        }
    }

    /**
     * Purge VTN flows relevant to obsolete layer 2 host entries.
     *
     * <p>
     *   This method removes obsolte layer 2 host entries added by
     *   {@link #addObsoleteEntry(MacTableEntry)}.
     * </p>
     *
     * @param mgr   VTN manager service.
     * @param fdb   VTN flow database.
     */
    public void purgeObsoleteFlow(VTNManagerImpl mgr, VTNFlowDatabase fdb) {
        for (ObjectPair<MacVlan, NodeConnector> host: obsoleteHosts) {
            fdb.removeFlows(mgr, host.getLeft(), host.getRight());
        }
        obsoleteHosts.clear();
    }

    /**
     * Create a brief description of the ethernet frame in this context.
     *
     * @return  A brief description of the ethernet frame in ths context.
     */
    public String getDescription() {
        return getDescription((NodeConnector)null);
    }

    /**
     * Create a brief description of the ethernet frame in this context.
     *
     * @param port  A node connector associated with the ethernet frame.
     * @return  A brief description of the ethernet frame in ths context.
     */
    public String getDescription(NodeConnector port) {
        return getDescription(etherFrame.getSourceAddress(),
                              etherFrame.getDestinationAddress(),
                              etherFrame.getEtherType(), port,
                              etherFrame.getVlan());
    }

    /**
     * Create a brief description of the ethernet frame.
     *
     * @param ether  An ethernet frame.
     * @param port   A node connector associated with the ethernet frame.
     * @param vlan   VLAN ID.
     * @return  A brief description of the specified ethernet frame.
     */
    public String getDescription(Ethernet ether, NodeConnector port,
                                 short vlan) {
        return getDescription(ether.getSourceMACAddress(),
                              ether.getDestinationMACAddress(),
                              (int)ether.getEtherType() & ETHER_TYPE_MASK,
                              port, vlan);
    }

    /**
     * Create a brief description of the ethernet frame.
     *
     * @param src    The source MAC address.
     * @param dst    The destination MAC address.
     * @param type   The ethernet type.
     * @param port   A node connector associated with the ethernet frame.
     * @param vlan   VLAN ID.
     * @return  A brief description of the specified ethernet frame.
     */
    public String getDescription(byte[] src, byte[] dst, int type,
                                 NodeConnector port, short vlan) {
        String srcmac = HexEncode.bytesToHexStringFormat(src);
        String dstmac = HexEncode.bytesToHexStringFormat(dst);

        StringBuilder builder = new StringBuilder("src=");
        builder.append(srcmac).
            append(", dst=").append(dstmac);
        if (port != null) {
            builder.append(", port=").append(port);
        }
        builder.append(", type=0x").append(Integer.toHexString(type)).
            append(", vlan=").append((int)vlan);

        return builder.toString();
    }

    /**
     * Return an {@link Inet4Packet} instance which represents the IPv4 packet
     * in the payload.
     *
     * @return  An {@link Inet4Packet} instance if the Ethernet frame contains
     *          an IPv4 paclet. Otherwise {@code null}.
     */
    public Inet4Packet getInet4Packet() {
        if (inet4Packet == null && isIPv4()) {
            Packet packet = etherFrame.getPayload();
            if (packet instanceof IPv4) {
                inet4Packet = new Inet4Packet((IPv4)packet);
            }
        }

        return inet4Packet;
    }

    /**
     * Return a {@link CachedPacket} instance which represents layer 4
     * protocol data.
     *
     * @return  A {@link CachedPacket} instance if found.
     *          {@code null} if not found.
     */
    public CachedPacket getL4Packet() {
        if (l4Packet == null) {
            Inet4Packet ipv4 = getInet4Packet();
            if (ipv4 != null) {
                Packet payload = ipv4.getPacket().getPayload();
                if (payload instanceof TCP) {
                    l4Packet = new TcpPacket((TCP)payload);
                } else if (payload instanceof UDP) {
                    l4Packet = new UdpPacket((UDP)payload);
                } else if (payload instanceof ICMP) {
                    l4Packet = new IcmpPacket((ICMP)payload);
                }
            }
        }

        return l4Packet;
    }

    /**
     * Determine whether this packet is an IPv4 packet or not.
     *
     * @return  {@code true} is returned only if this packet is an IPv4 packet.
     */
    public boolean isIPv4() {
        return (etherFrame.getEtherType() == EtherTypes.IPv4.intValue());
    }

    /**
     * Try to probe IP address of the source address of this packet.
     *
     * @param mgr  VTN Manager service.
     */
    public void probeInetAddress(VTNManagerImpl mgr) {
        Inet4Packet ipv4 = getInet4Packet();
        if (ipv4 != null) {
            // Send an ARP request to the source address of this packet.
            int srcIp = ipv4.getSourceAddress();
            byte[] dst = etherFrame.getSourceAddress();
            byte[] tpa = NetUtils.intToByteArray4(srcIp);
            short vlan = getVlan();
            Ethernet ether = mgr.createArpRequest(dst, tpa, vlan);
            NodeConnector port = getIncomingNodeConnector();

            if (LOG.isTraceEnabled()) {
                String dstmac = HexEncode.bytesToHexStringFormat(dst);
                String target;
                try {
                    InetAddress ia = InetAddress.getByAddress(tpa);
                    target = ia.getHostAddress();
                } catch (Exception e) {
                    target = HexEncode.bytesToHexStringFormat(tpa);
                }
                LOG.trace("{}: Send an ARP request to detect IP address: " +
                          "dst={}, tpa={}, vlan={}, port={}",
                          mgr.getContainerName(), dstmac, target, vlan, port);
            }

            mgr.transmit(port, ether);
        }
    }

    /**
     * Add match field to be configured into a flow entry.
     *
     * @param type  A match type to be added.
     */
    public void addMatchField(MatchType type) {
        matchFields.add(type);
    }

    /**
     * Create match for a flow entry.
     *
     * @param inPort  A node connector associated with incoming switch port.
     * @return  A match object that matches the packet.
     */
    public Match createMatch(NodeConnector inPort) {
        Match match = new Match();

        // Incoming port field is mandatory.
        match.setField(MatchType.IN_PORT, inPort);

        etherFrame.setMatch(match, matchFields);
        Inet4Packet ipv4 = getInet4Packet();
        if (ipv4 != null) {
            ipv4.setMatch(match, matchFields);
            CachedPacket l4 = getL4Packet();
            if (l4 != null) {
                l4.setMatch(match, matchFields);
            }
        }

        return match;
    }

    /**
     * Append the specified virtual node hop to the virtual packet route.
     *
     * @param vroute  A {@link VNodeRoute} instance which represents a
     *                routing to the virtual node.
     */
    public void addNodeRoute(VNodeRoute vroute) {
        virtualRoute.put(vroute.getPath(), vroute);
    }

    /**
     * Set the location of the egress node.
     *
     * @param path    A {@link VNodePath} instance which represents the
     *                location of the egress node.
     */
    public void setEgressVNodePath(VNodePath path) {
        egressNodePath = path;
    }

    /**
     * Record a virtual node to be associated with the data flow.
     *
     * <p>
     *   Node that the virtual node on the packet routing path does not need
     *   to be associated with the data flow by this method.
     * </p>
     *
     * @param path  A virtual node path.
     */
    public void addNodePath(VTenantPath path) {
        virtualNodes.add(path);
    }

    /**
     * Fix up the VTN flow for installation.
     *
     * @param vflow  A VTN flow.
     */
    public void fixUp(VTNFlow vflow) {
        // Set the virtual packet routing path.
        vflow.addVirtualRoute(virtualRoute.values());
        vflow.setEgressVNodePath(egressNodePath);

        // Set additional dependencies.
        vflow.addDependency(virtualNodes);

        // Set flow timeout.
        vflow.setTimeout(idleTimeout, hardTimeout);
    }

    /**
     * Set route resolver for this packet.
     *
     * @param rr  A {@link RouteResolver} instance.
     */
    public void setRouteResolver(RouteResolver rr) {
        routeResolver = rr;
    }

    /**
     * Return route resolver for this packet.
     *
     * @return  A {@link RouteResolver} instance.
     */
    public RouteResolver getRouteResolver() {
        return routeResolver;
    }

    /**
     * Set timeout for an ingress flow.
     *
     * <p>
     *   This method does nothing if no flow entry is added.
     * </p>
     *
     * @param idle  An idle timeout for an ingress flow.
     * @param hard  A hard timeout for an ingress flow.
     */
    public void setFlowTimeout(int idle, int hard) {
        idleTimeout = idle;
        hardTimeout = hard;
    }

    /**
     * Return a priority value for flow entries.
     *
     * @param mgr  VTN Manager service.
     * @return  A flow priority value.
     */
    public int getFlowPriority(VTNManagerImpl mgr) {
        int pri = mgr.getVTNConfig().getL2FlowPriority();
        return (pri + matchFields.size());
    }
}
