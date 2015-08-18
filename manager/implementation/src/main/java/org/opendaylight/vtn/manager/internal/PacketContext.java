/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.Collection;
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
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgeHostFlowRemover;
import org.opendaylight.vtn.manager.internal.packet.PacketInEvent;
import org.opendaylight.vtn.manager.internal.packet.cache.EtherPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.IcmpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.Inet4Packet;
import org.opendaylight.vtn.manager.internal.packet.cache.L4Packet;
import org.opendaylight.vtn.manager.internal.packet.cache.TcpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.UdpPacket;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchContext;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNInet4Match;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNLayer4Match;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.packet.ArpPacketBuilder;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.core.NodeConnector;
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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;

/**
 * {@code PacketContext} class describes the context of received packet.
 *
 * <p>
 *   This class is designed to be used by a single thread.
 * </p>
 */
public class PacketContext implements Cloneable, FlowMatchContext {
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
     * PACKET_IN event.
     */
    private final PacketInEvent  packetIn;

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
    private Set<L2Host>  obsoleteHosts = new HashSet<>();

    /**
     * The egress switch port.
     */
    private SalPort  egressPort;

    /**
     * A sequence of virtual packet routing.
     */
    private List<VNodeRoute>  virtualRoute = new ArrayList<VNodeRoute>();

    /**
     * A set of {@link FlowMatchType} instances which represents match fields
     * to be configured.
     */
    private EnumSet<FlowMatchType>  matchFields =
        EnumSet.noneOf(FlowMatchType.class);

    /**
     * A {@link VNodeRoute} instance which represents the hop to the egress
     * virtual node.
     */
    private VNodeRoute  egressNodeRoute;

    /**
     * An {@link Inet4Packet} instance which represents the IPv4 packet in the
     * payload.
     */
    private Inet4Packet  inet4Packet;

    /**
     * A {@link L4Packet} instance which represents the layer 4 protocol
     * data.
     */
    private L4Packet  l4Packet;

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
     * A map that keeps flow actions created by flow filters.
     */
    private Map<Class<? extends FlowFilterAction>, FlowFilterAction>  filterActions;

    /**
     * The number of virtual node hops caused by REDIRECT flow filter.
     */
    private int  virtualNodeHops;

    /**
     * A {@link RedirectFlowException} instance which represents the first
     * packet redirection in a flow.
     */
    private RedirectFlowException  firstRedirection;

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
     * Set {@code true} at least one flow filter is evaluated.
     */
    private boolean  filtered;

    /**
     * Determine whether the destination MAC address of this packet is equal to
     * the controller's MAC address or not.
     */
    private boolean  toController;

    /**
     * A MD-SAL datastore transaction context.
     */
    private final TxContext txContext;

    /**
     * Construct a new packet context to handle PACKET_IN message.
     *
     * @param ev  A {@link PacketInEvent} instance.
     */
    PacketContext(PacketInEvent ev) {
        rawPacket = ev.getPayload();
        etherFrame = new EtherPacket(ev.getEthernet());
        txContext = ev.getTxContext();
        packetIn = ev;
    }

    /**
     * Construct a new packet context from the specified ethernet frame.
     *
     * <p>
     *   This constructor is used to transmit self-originated packet.
     * </p>
     *
     * @param ether  An ethernet frame.
     * @param out    A {@link SalPort} instance corresponding to the egress
     *               switch port.
     * @param ctx    A MD-SAL datastore transaction only for read.
     */
    PacketContext(Ethernet ether, SalPort out, TxContext ctx) {
        rawPacket = null;
        etherFrame = new EtherPacket(ether);
        txContext = ctx;
        egressPort = out;
        packetIn = null;

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
    public EtherAddress getSourceAddress() {
        return etherFrame.getSourceAddress();
    }

    /**
     * Return the destination MAC address.
     *
     * @return  The destination MAC address.
     */
    public EtherAddress getDestinationAddress() {
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
                    if (NumberUtils.toInteger(sender) != 0) {
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
     * Return a {@link SalPort} instance corresponding to the ingress switch
     * port.
     *
     * @return  A {@link SalPort} instance or {@code null}.
     */
    public SalPort getIngressPort() {
        return (packetIn == null) ? null : packetIn.getIngressPort();
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
        return new PortVlan(nc, (short)etherFrame.getOriginalVlan());
    }

    /**
     * Return a {@link SalPort} instance corresponding to the egress switch
     * port to which the packet sends.
     *
     * @return  A {@link SalPort} instance if the egress switch port is
     *          configured. Otherwise {@code null}.
     */
    public SalPort getEgressPort() {
        return egressPort;
    }

    /**
     * Return VLAN ID of this packet.
     *
     * @return  VLAN ID. Zero is returned if no VLAN tag was found in the
     *          packet.
     */
    public int getVlan() {
        return etherFrame.getVlanId();
    }

    /**
     * Set VLAN ID used for packet matching.
     *
     * @param vid  A VLAN ID.
     */
    public void setVlan(int vid) {
        etherFrame.setVlanId(vid);
    }

    /**
     * Create a new ethernet frame which forwards the received packet.
     *
     * @param vlan  VLAN ID for a new frame.
     *              Zero means that the VLAN tag should not be added.
     * @return  A new ethernet frame.
     * @throws VTNException
     *    Failed to commit packet modification.
     */
    public Ethernet createFrame(int vlan) throws VTNException {
        // Configure VLAN ID for outgoing packet. It is used to determine
        // flow actions to be configured.
        etherFrame.setVlanId(vlan);

        // Commit changes made by flow filters to the packet.
        commit();

        EtherAddress src = etherFrame.getSourceAddress();
        EtherAddress dst = etherFrame.getDestinationAddress();
        Ethernet ether = new Ethernet();
        ether.setSourceMACAddress(src.getBytes()).
            setDestinationMACAddress(dst.getBytes());

        short ethType = (short)etherFrame.getEtherType();
        IEEE8021Q vlanTag = etherFrame.getVlanTag();
        Packet payload = etherFrame.getPayload();
        if (vlan != EtherHeader.VLAN_NONE ||
            (vlanTag != null && vlanTag.getVid() == EtherHeader.VLAN_NONE)) {
            // Add a VLAN tag.
            // We don't strip VLAN tag with zero VLAN ID because PCP field
            // in the VLAN tag should affect even if the VLAN ID is zero.
            IEEE8021Q tag = new IEEE8021Q();
            byte pcp = (byte)etherFrame.getVlanPriority();
            if (pcp < 0) {
                pcp = (byte)0;
            }

            byte cfi;
            if (vlanTag != null) {
                cfi = vlanTag.getCfi();
            } else {
                cfi = (byte)0;
            }
            tag.setCfi(cfi).setPcp(pcp).setVid((short)vlan).
                setEtherType(ethType);
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
        obsoleteHosts.add(new L2Host(mac, tent.getVlan(), tent.getPort()));
    }

    /**
     * Return a set of obsolete layer 2 host entries.
     *
     * @return  A set of obsolete layer 2 host entries.
     */
    public Set<L2Host> getObsoleteEntries() {
        return obsoleteHosts;
    }

    /**
     * Purge VTN flows relevant to obsolete layer 2 host entries.
     */
    public void purgeObsoleteFlow() {
        purgeObsoleteFlow(txContext.getProvider());
    }

    /**
     * Purge VTN flows relevant to obsolete layer 2 host entries.
     *
     * @param provider    VTN manager provider service.
     */
    public void purgeObsoleteFlow(VTNManagerProvider provider) {
        String tname = mapReference.getPath().getTenantName();
        for (L2Host host: obsoleteHosts) {
            EdgeHostFlowRemover remover = new EdgeHostFlowRemover(tname, host);
            provider.removeFlows(remover);
        }
        obsoleteHosts.clear();
    }

    /**
     * Create a brief description of the ethernet frame in this context.
     *
     * @return  A brief description of the ethernet frame in ths context.
     */
    public String getDescription() {
        return getDescription(getIngressPort());
    }

    /**
     * Create a brief description of the ethernet frame in this context.
     *
     * @param sport  A {@link SalPort} instance associated with the ethernet
     *               frame.
     * @return  A brief description of the ethernet frame in ths context.
     */
    public String getDescription(SalPort sport) {
        return getDescription(etherFrame.getSourceAddress(),
                              etherFrame.getDestinationAddress(),
                              etherFrame.getEtherType(), sport,
                              etherFrame.getVlanId());
    }

    /**
     * Create a brief description of the ethernet frame.
     *
     * @param ether  An ethernet frame.
     * @param sport  A {@link SalPort} instance associated with the ethernet
     *               frame.
     * @param vlan   VLAN ID.
     * @return  A brief description of the specified ethernet frame.
     */
    public String getDescription(Ethernet ether, SalPort sport, int vlan) {
        EtherAddress src = new EtherAddress(ether.getSourceMACAddress());
        EtherAddress dst = new EtherAddress(ether.getDestinationMACAddress());
        return getDescription(src, dst,
                              (int)ether.getEtherType() & ETHER_TYPE_MASK,
                              sport, vlan);
    }

    /**
     * Create a brief description of the ethernet frame.
     *
     * @param src    The source MAC address.
     * @param dst    The destination MAC address.
     * @param type   The ethernet type.
     * @param sport  A {@link SalPort} instance associated with the ethernet
     *               frame.
     * @param vlan   VLAN ID.
     * @return  A brief description of the specified ethernet frame.
     */
    public String getDescription(EtherAddress src, EtherAddress dst, int type,
                                 SalPort sport, int vlan) {
        StringBuilder builder = new StringBuilder("src=");
        builder.append(src.getText()).
            append(", dst=").append(dst.getText());
        if (sport != null) {
            builder.append(", port=").append(sport);
        }
        builder.append(", type=0x").append(Integer.toHexString(type)).
            append(", vlan=").append(vlan);

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
     * Return a {@link L4Packet} instance which represents layer 4 protocol
     * data.
     *
     * @return  A {@link L4Packet} instance if found.
     *          {@code null} if not found.
     */
    public L4Packet getL4Packet() {
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
        return etherFrame.getDestinationAddress().isUnicast();
    }

    /**
     * Set a {@link MapReference} instance that determines the ingress virtual
     * node.
     *
     * @param ref  A {@link MapReference} instance.
     */
    public void setMapReference(MapReference ref) {
        mapReference = ref;
        FlowMatchType mtype = ref.getMapType().getMatchType();
        if (mtype != null) {
            matchFields.add(mtype);
        }
    }

    /**
     * Try to probe IP address of the source address of this packet.
     */
    public void probeInetAddress() {
        Inet4Packet ipv4 = getInet4Packet();
        if (ipv4 != null) {
            // Send an ARP request to the source address of this packet.
            VTNManagerProvider provider = txContext.getProvider();
            Ip4Network tpa = ipv4.getSourceAddress();
            EtherAddress src =
                provider.getVTNConfig().getControllerMacAddress();
            EtherAddress dst = etherFrame.getSourceAddress();
            int vlan = etherFrame.getOriginalVlan();
            Ethernet ether = new ArpPacketBuilder(vlan).build(src, dst, tpa);
            SalPort sport = getIngressPort();

            if (LOG.isTraceEnabled()) {
                String dstmac = dst.getText();
                String target = tpa.getText();
                LOG.trace("Send an ARP request to detect IP address: " +
                          "dst={}, tpa={}, vlan={}, port={}",
                          dstmac, target, vlan, sport);
            }

            provider.transmit(sport, ether);
        }
    }

    /**
     * Append the specified virtual node hop to the virtual packet route.
     *
     * @param vroute  A {@link VNodeRoute} instance which represents a
     *                routing to the virtual node.
     */
    public void addVNodeRoute(VNodeRoute vroute) {
        virtualRoute.add(vroute);
    }

    /**
     * Set the virtual node hop to the egress node.
     *
     * @param vroute  A {@link VNodeRoute} instance which represents the hop
     *                to the egress node.
     */
    public void setEgressVNodeRoute(VNodeRoute vroute) {
        VNodeRoute old = egressNodeRoute;
        if (old != null) {
            if (vroute.getReason() == VirtualRouteReason.FORWARDED &&
                old.getPath().equals(vroute.getPath())) {
                // No need to record the specified hop because the hop to the
                // specified virtual node is already recorded.
                return;
            }

            // Push the old hop into the virtual node route.
            virtualRoute.add(old);
        }

        egressNodeRoute = vroute;
    }

    /**
     * Return a {@link VNodeRoute} instance which represents the virtual node
     * hop to the egress node.
     *
     * @return  A {@link VNodeRoute} instance.
     *          {@code null} is returned if not configured.
     */
    public VNodeRoute getEgressVNodeRoute() {
        return egressNodeRoute;
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
     * @return  A flow priority value.
     */
    public int getFlowPriority() {
        VTNManagerProvider provider = txContext.getProvider();
        int pri = provider.getVTNConfig().getL2FlowPriority();
        int nmatches = matchFields.size() -
            FlowMatchType.getUnicastTypeCount(matchFields);
        return (pri + nmatches);
    }

    /**
     * Install a VTN data flow for the received packet.
     *
     * @param egress  A {@link SalPort} corresponding to the egress switch
     *                port.
     * @param outVid  A VLAN ID to be set to the outgoing packet.
     * @param path    A list of {@link LinkEdge} instances which represents
     *                packet route to the destination address of the received
     *                packet.
     */
    public void installFlow(SalPort egress, int outVid, List<LinkEdge> path) {
        // Purge obsolete flows before installing new flow.
        purgeObsoleteFlow();

        // Prepare to install an unicast flow entry.
        addUnicastMatchFields();

        VTNFlowBuilder builder = createFlowBuilder();
        if (builder == null) {
            return;
        }

        // Add flow entries for inter-switch links.
        SalPort src = packetIn.getIngressPort();
        for (LinkEdge le: path) {
            SalPort dst = le.getSourcePort();
            builder.addInternalFlow(src, dst);
            src = le.getDestinationPort();
        }

        // Create the egress flow entry.
        assert src.getNodeNumber() == egress.getNodeNumber();
        int inVid = etherFrame.getOriginalVlan();
        builder.addEgressFlow(src, egress, inVid, outVid, getFilterActions());

        // Install flow entries.
        txContext.getProvider().addFlow(builder);
    }

    /**
     * Install a flow entry that discards the packet.
     */
    public void installDropFlow() {
        if (flooding) {
            // Never install flow entry on packet flooding.
            return;
        }

        if (isUnicast()) {
            // The source and destination MAC address must be specified in a
            // drop flow entry if this packet is an unicast packet.
            addUnicastMatchFields();
        }

        // Create a flow entry that discards the given packet.
        VTNFlowBuilder builder = createFlowBuilder();
        if (builder == null) {
            return;
        }

        if (egressNodeRoute != null) {
            builder.setEgressVNodeRoute(null);
        }

        builder.addDropFlow(packetIn.getIngressPort());
        txContext.getProvider().addFlow(builder);
    }

    /**
     * Append a SAL action to the flow filter action list.
     *
     * @param act  A flow filter action.
     */
    public void addFilterAction(FlowFilterAction act) {
        if (!flooding) {
            if (filterActions == null) {
                filterActions = new LinkedHashMap<>();
            }
            filterActions.put(act.getClass(), act);
        }
    }

    /**
     * Remove the specified flow action from the flow filter action list.
     *
     * @param actClass  A class of flow filter action to be removed.
     */
    public void removeFilterAction(Class<? extends FlowFilterAction> actClass) {
        if (!flooding && filterActions != null) {
            filterActions.remove(actClass);
        }
    }

    /**
     * Return a list of SAL actions created by flow filters.
     *
     * @return  A collection of flow filter actions.
     *          {@code null} is returned if no flow action was created by
     *          flow filter.
     */
    public Collection<FlowFilterAction> getFilterActions() {
        return (filterActions == null) ? null : filterActions.values();
    }

    /**
     * Commit all modifications to the packet.
     *
     * @throws VTNException
     *    Failed to copy the packet.
     */
    public void commit() throws VTNException {
        // Commit modification to the Ethernet header.
        etherFrame.commit(this);

        // Commit modification to layer 4 protocol header.
        L4Packet l4 = l4Packet;
        Inet4Packet inet4 = inet4Packet;
        boolean l4Changed;
        if (l4 == null) {
            l4Changed = false;
        } else {
            l4Changed = l4.commit(this);
            // Update checksum if needed.
            if ((l4Changed || inet4.isAddressModified()) &&
                l4.updateChecksum(inet4)) {
                l4Changed = true;
            }
        }

        // Commit modification to IPv4 header.
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
     * Determine whether at least one flow filter is evaluated with this packet
     * or not.
     *
     * @return  {@code true} only if at least one flow filter is evaluated.
     */
    public boolean isFiltered() {
        return filtered;
    }

    /**
     * Set a boolean value which determines whetehr at least one flow filter
     * is evaluated with this packet or not.
     *
     * @param b  {@code true} means that at least one flow filter is evaluated
     *           with this packet.
     */
    public void setFiltered(boolean b) {
        filtered = b;
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
     * Record the packet redirection.
     *
     * @param path  The location of the destination interface.
     * @return  The number of virtual node hops.
     */
    public int redirect(VNodePath path) {
        fixIngressNode();

        // Record the packet redirection as the hop to the egress node.
        setEgressVNodeRoute(new VNodeRoute(path,
                                           VirtualRouteReason.REDIRECTED));

        // Increment the virtual node hops.
        int ret = virtualNodeHops + 1;
        virtualNodeHops = ret;

        return ret;
    }

    /**
     * Set a {@link RedirectFlowException} which represents the first packet
     * redirection in a flow.
     *
     * @param rex  A {@link RedirectFlowException} instance.
     * @return  {@code true} is returned if the given instance represents the
     *          first packet redirection in a flow.
     *          Otherwise {@code false} is returned.
     */
    public boolean setFirstRedirection(RedirectFlowException rex) {
        boolean first = (firstRedirection == null);
        if (first) {
            firstRedirection = rex;
        }

        return first;
    }

    /**
     * Return a {@link RedirectFlowException} which represents the first packet
     * redirection in a flow.
     *
     * @return  A {@link RedirectFlowException} instance.
     *          {@code null} is returned if the packet was not redirected.
     */
    public RedirectFlowException getFirstRedirection() {
        return firstRedirection;
    }

    /**
     * Return a MD-SAL datastore transaction context.
     *
     * @return  A {@link TxContext} instance.
     */
    public TxContext getTxContext() {
        return txContext;
    }

    /**
     * Fix up the ingress virtual node.
     */
    private void fixIngressNode() {
        if (virtualRoute.isEmpty() && mapReference != null) {
            // This can happen if the packet was discarded by the VTN flow
            // filter. In this case we need to estimate ingress virtual node
            // route from mapping reference.
            virtualRoute.add(mapReference.getIngressRoute());
        }
    }

    /**
     * Create a new VTN data flow builder.
     *
     * @return  A {@link VTNFlowBuilder} on success.
     *          {@code null} on failure.
     */
    private VTNFlowBuilder createFlowBuilder() {
        VTNMatch vmatch = createMatch();
        if (vmatch == null) {
            return null;
        }

        fixIngressNode();
        assert !virtualRoute.isEmpty();
        VNodeRoute vnr = virtualRoute.get(0);
        String tname = vnr.getPath().getTenantName();

        VTNManagerProvider provider = txContext.getProvider();
        VTNConfig vcfg = provider.getVTNConfig();
        EtherAddress mac = vcfg.getControllerMacAddress();
        int nmatches = matchFields.size() -
            FlowMatchType.getUnicastTypeCount(matchFields);
        int pri = vcfg.getL2FlowPriority() + nmatches;

        VTNFlowBuilder builder = new VTNFlowBuilder(
            tname, mac, vmatch, pri, idleTimeout, hardTimeout);

        // Set the virtual packet routing path and path policy identifier.
        return builder.addVirtualRoute(virtualRoute).
            setEgressVNodeRoute(egressNodeRoute).
            setPathPolicyId(routeResolver.getPathPolicyId());
    }

    /**
     * Create a new flow match.
     *
     * @return  A {@link VTNMatch} instance.
     *          {@code null} if this packet is broken.
     */
    private VTNMatch createMatch() {
        assert !flooding;

        try {
            VTNEtherMatch ematch = etherFrame.createMatch(matchFields);
            VTNInet4Match imatch = null;
            VTNLayer4Match l4match = null;

            Inet4Packet ipv4 = getInet4Packet();
            if (ipv4 != null) {
                imatch = ipv4.createMatch(matchFields);
                if (imatch.isEmpty()) {
                    imatch = null;
                }

                L4Packet l4 = getL4Packet();
                if (l4 != null) {
                    l4match = l4.createMatch(matchFields);
                    if (l4match.isEmpty()) {
                        l4match = null;
                    }
                }
            }

            return new VTNMatch(ematch, imatch, l4match);
        } catch (RpcException | RuntimeException e) {
            // This should never happen.
            LOG.error("Failed to create VTNMatch.", e);
            return null;
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

            L4Packet l4 = pctx.l4Packet;
            if (l4 != null) {
                pctx.l4Packet = l4.clone();
            }

            return pctx;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }

    // FlowMatchContext

    /**
     * {@inheritDoc}
     */
    @Override
    public void addMatchField(FlowMatchType type) {
        if (!flooding) {
            matchFields.add(type);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean hasMatchField(FlowMatchType type) {
        return matchFields.contains(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addUnicastMatchFields() {
        if (!flooding) {
            FlowMatchType.addUnicastTypes(matchFields);
        }
    }

    // PacketHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public EtherHeader getEtherHeader() {
        return etherFrame;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InetHeader getInetHeader() {
        return getInet4Packet();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Layer4Header getLayer4Header() {
        return getL4Packet();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getHeaderDescription() {
        StringBuilder builder = new StringBuilder();
        getEtherHeader().setDescription(builder);

        InetHeader inet = getInetHeader();
        if (inet != null) {
            builder.append(',');
            inet.setDescription(builder);
            Layer4Header l4 = getLayer4Header();
            if (l4 != null) {
                builder.append(',');
                l4.setDescription(builder);
            }
        }

        return builder.toString();
    }
}
