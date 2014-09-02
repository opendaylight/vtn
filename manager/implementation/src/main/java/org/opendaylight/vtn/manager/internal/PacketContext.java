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
import java.util.EnumSet;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.packet.CachedPacket;
import org.opendaylight.vtn.manager.internal.packet.EtherPacket;
import org.opendaylight.vtn.manager.internal.packet.IcmpPacket;
import org.opendaylight.vtn.manager.internal.packet.Inet4Packet;
import org.opendaylight.vtn.manager.internal.packet.TcpPacket;
import org.opendaylight.vtn.manager.internal.packet.UdpPacket;

import org.opendaylight.controller.sal.action.Action;
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
public class PacketContext implements Cloneable {
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
     * Flow match fields to be configured in an unicast flow entry.
     */
    private static final MatchType[] UNICAST_MATCHES = {
        MatchType.DL_SRC,
        MatchType.DL_DST,
    };

    /**
     * A received raw packet.
     */
    private final RawPacket  rawPacket;

    /**
     * Decoded ethernet frame.
     */
    private EtherPacket  etherFrame;

    /**
     * Source IP address.
     */
    private byte[] sourceInetAddress;

    /**
     * Obsolete layer 2 host entries.
     */
    private Set<ObjectPair<MacVlan, NodeConnector>>  obsoleteHosts =
        new HashSet<ObjectPair<MacVlan, NodeConnector>>();

    /**
     * Outgoing node connector.
     */
    private NodeConnector  outgoing;

    /**
     * Set of virtual node paths to be associated with the data flow.
     */
    private Set<VTenantPath>  virtualNodes = new HashSet<VTenantPath>();

    /**
     * A sequence of virtual packet routing.
     */
    private Map<VNodePath, VNodeRoute>  virtualRoute =
        new LinkedHashMap<VNodePath, VNodeRoute>();

    /**
     * A set of {@link MatchType} instances which represents match fields
     * to be configured.
     */
    private EnumSet<MatchType>  matchFields =
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
     * A {@link MapReference} instance which represents the virtual mapping
     * that maps the incoming packet.
     */
    private MapReference  mapReference;

    /**
     * A list of SAL actions created by flow filters.
     */
    private List<Action>  filterActions;

    /**
     * Set {@code true} if this packet is goint to be broadcasted in the
     * vBridge.
     */
    private boolean  flooding;

    /**
     * Determine whether the flow filter should be disabled or not.
     */
    private boolean  filterDisabled;

    /**
     * Determine whether the destination MAC address of this packet is equal to
     * the controller's MAC address or not.
     */
    private boolean  toController;

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
     * <p>
     *   This constructor is used to transmit self-originated packet.
     * </p>
     *
     * @param ether     An ethernet frame.
     * @param out       Outgoing node connector.
     */
    PacketContext(Ethernet ether, NodeConnector out) {
        this(null, ether);
        outgoing = out;

        // Flow filter must not affect self-originated packet.
        filterDisabled = true;
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
        return new PortVlan(nc, etherFrame.getOriginalVlan());
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
     * Set VLAN ID used for packet matching.
     *
     * @param vid  A VLAN ID.
     */
    public void setVlan(short vid) {
        etherFrame.setVlan(vid);
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
            byte pcp = etherFrame.getVlanPriority();
            if (pcp < 0) {
                pcp = (byte)0;
            }

            byte cfi;
            if (vlanTag != null) {
                cfi = vlanTag.getCfi();
            } else {
                cfi = (byte)0;
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
        obsoleteHosts.add(new ObjectPair<MacVlan, NodeConnector>(
                              mvlan, tent.getPort()));
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
        NodeConnector incoming = (rawPacket == null)
            ? null : rawPacket.getIncomingNodeConnector();
        return getDescription(incoming);
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
     * Determine whether this packet is an unicast packet or not.
     *
     * @return  {@code true} is returned only if this packet is an unicast
     *          packet.
     */
    public boolean isUnicast() {
        byte[] dst = etherFrame.getDestinationAddress();
        return NetUtils.isUnicastMACAddr(dst);
    }

    /**
     * Set a {@link MapReference} instance that determines the ingress virtual
     * node.
     *
     * @param ref  A {@link MapReference} instance.
     */
    public void setMapReference(MapReference ref) {
        mapReference = ref;
        MatchType mtype = ref.getMapType().getMatchType();
        if (mtype != null) {
            matchFields.add(mtype);
        }
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
            short vlan = etherFrame.getOriginalVlan();
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
     * Add a match field to be configured into a flow entry.
     *
     * @param type  A match type to be added.
     */
    public void addMatchField(MatchType type) {
        if (!flooding) {
            matchFields.add(type);
        }
    }

    /**
     * Add match fields to be configured into an unicast flow entry.
     */
    public void addUnicastMatchFields() {
        if (!flooding) {
            for (MatchType type: UNICAST_MATCHES) {
                matchFields.add(type);
            }
        }
    }

    /**
     * Create match for a flow entry.
     *
     * @param inPort  A node connector associated with incoming switch port.
     * @return  A match object that matches the packet.
     */
    public Match createMatch(NodeConnector inPort) {
        assert !flooding;
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
    public void addVNodeRoute(VNodeRoute vroute) {
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
     *   Note that the virtual node on the packet routing path does not need
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
        if (virtualRoute.isEmpty() && mapReference != null) {
            // This can happen if the packet was discarded by the VTN flow
            // filter. In this case we need to estimate ingress virtual node
            // route from mapping reference.
            vflow.addVirtualRoute(mapReference.getIngressRoute());
        }

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
     * Return the idle timeout for a flow entry.
     *
     * @return  The idle timeout for a flow entry.
     */
    public int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Return the hard timeout for a flow entry.
     *
     * @return  The hard timeout for a flow entry.
     */
    public int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Return a priority value for flow entries.
     *
     * @param mgr  VTN Manager service.
     * @return  A flow priority value.
     */
    public int getFlowPriority(VTNManagerImpl mgr) {
        int pri = mgr.getVTNConfig().getL2FlowPriority();
        int nmatches = matchFields.size();
        for (MatchType type: UNICAST_MATCHES) {
            if (matchFields.contains(type)) {
                nmatches--;
            }
        }

        return (pri + nmatches);
    }

    /**
     * Install a flow entry that discards the packet.
     *
     * @param mgr  VTN Manager service.
     * @param fdb  VTN flow database.
     */
    public void installDropFlow(VTNManagerImpl mgr, VTNFlowDatabase fdb) {
        if (flooding) {
            // Never install flow entry on packet flooding.
            return;
        }

        // Create a flow entry that discards the given packet.
        if (isUnicast()) {
            addUnicastMatchFields();
        }

        NodeConnector incoming = getIncomingNodeConnector();
        Match match = createMatch(incoming);
        int pri = getFlowPriority(mgr);
        VTNFlow vflow = fdb.create(mgr);
        vflow.addFlow(mgr, match, pri);

        // Set the virtual packet routing.
        fixUp(vflow);
        vflow.setEgressVNodePath(null);

        // Install a flow entry.
        fdb.install(mgr, vflow);
    }

    /**
     * Append a SAL action to the flow filter action list.
     *
     * @param act  A SAL action.
     */
    public void addFilterAction(Action act) {
        if (!flooding) {
            if (filterActions == null) {
                filterActions = new ArrayList<Action>();
            }
            filterActions.add(act);
        }
    }

    /**
     * Return a list of SAL actions created by flow filters.
     *
     * @return  A list of SAL actions.
     *          {@code null} is returned if no SAL action was created by
     *          flow filter.
     */
    public List<Action> getFilterActions() {
        return filterActions;
    }

    /**
     * Commit all modifications to the packet.
     *
     * @throws VTNException
     *    Failed to copy the packet.
     */
    public void commit() throws VTNException {
        etherFrame.commit(this);
        CachedPacket l4 = l4Packet;
        Inet4Packet inet4 = inet4Packet;
        boolean l4Changed = (l4 != null) ? l4.commit(this) : false;
        boolean inet4Changed = (inet4 != null) ? inet4.commit(this) : false;
        if (l4Changed) {
            Packet payload = l4.getPacket();
            inet4.getPacket().setPayload(payload);
            inet4Changed = true;
        }
        if (inet4Changed) {
            IPv4 payload = inet4.getPacket();
            etherFrame.setPayload(payload);
        }
    }

    /**
     * Determine whether the packet is going to be broadcasted in the vBridge
     * or not.
     *
     * @return  {@code true} is returned only if the packet is going to be
     *          broadcasted in the vBridge.
     */
    public boolean isFlooding() {
        return flooding;
    }

    /**
     * Set a boolean value which indicates whether the packet is going to
     * be broadcasted in the vBridge.
     *
     * @param b  {@code true} means that the packet is going to be broadcasted
     *           in the vBridge.
     */
    public void setFlooding(boolean b) {
        flooding = b;
    }

    /**
     * Determine whether this packet should be handled without flow filters
     * or not.
     *
     * @return  {@code true} is returned only if flow filters should be
     *          disabled.
     */
    public boolean isFilterDisabled() {
        return filterDisabled;
    }

    /**
     * Set a boolean value which indicates whether this packet should be
     * handled without flow filters or not.
     *
     * @param b  {@code true} means that this packet should be handled without
     *           flow filter.
     *           {@code false} means that flow filters should be applied to
     *           this packet.
     */
    public void setFilterDisabled(boolean b) {
        filterDisabled = b;
    }

    /**
     * Determine whether the destination address of this packet is equal to
     * the controller address or not.
     *
     * @return  {@code true} is returned if this packet is sent to the
     *          controller. Otherwise {@code false} is returned.
     */
    public boolean isToController() {
        return toController;
    }

    /**
     * Set a boolean value which indicates whether the packet is sent to the
     * controller or not.
     *
     * @param b  {@code true} means that the packet is sent to the controller.
     *           Pass {@code false} otherwise.
     */
    public void setToController(boolean b) {
        toController = b;
        if (b) {
            // Disable flow filter.
            filterDisabled = true;
        }
    }

    /**
     * Return a copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public PacketContext clone() {
        // Currently this method is expected to be called only if the packet
        // is flooding in the vBridge.
        assert flooding;

        try {
            PacketContext pctx = (PacketContext)super.clone();
            pctx.etherFrame = pctx.etherFrame.clone();

            Inet4Packet inet4 = pctx.inet4Packet;
            if (inet4 != null) {
                pctx.inet4Packet = inet4.clone();
            }

            CachedPacket l4 = pctx.l4Packet;
            if (l4 != null) {
                pctx.l4Packet = l4.clone();
            }

            return pctx;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
