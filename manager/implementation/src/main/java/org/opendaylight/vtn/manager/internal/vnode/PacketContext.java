/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.ProtocolUtils.MASK_ETHER_TYPE;

import java.util.ArrayList;
import java.util.Collection;
import java.util.EnumMap;
import java.util.EnumSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.collect.ImmutableMap;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.ARP;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.packet.ICMP;
import org.opendaylight.vtn.manager.packet.IEEE8021Q;
import org.opendaylight.vtn.manager.packet.IPv4;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.packet.TCP;
import org.opendaylight.vtn.manager.packet.UDP;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.add.FlowAddQueue;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgeHostFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.packet.PacketInEvent;
import org.opendaylight.vtn.manager.internal.packet.PacketOutQueue;
import org.opendaylight.vtn.manager.internal.packet.cache.CachedPacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.EtherPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.IcmpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.Inet4Packet;
import org.opendaylight.vtn.manager.internal.packet.cache.L4Packet;
import org.opendaylight.vtn.manager.internal.packet.cache.TcpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.UdpPacket;
import org.opendaylight.vtn.manager.internal.routing.PathMapContext;
import org.opendaylight.vtn.manager.internal.routing.PathMapEvaluator;
import org.opendaylight.vtn.manager.internal.util.flow.VNodeHop;
import org.opendaylight.vtn.manager.internal.util.flow.VTNFlowBuilder;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.flow.filter.DropFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterContext;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterListId;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNInet4Match;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNLayer4Match;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.L2Host;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.packet.ArpPacketBuilder;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.packet.UnsupportedPacketException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.MacEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;

/**
 * {@code PacketContext} class describes the context of received packet.
 *
 * <p>
 *   This class is designed to be used by a single thread.
 * </p>
 */
public class PacketContext
    implements Cloneable, CachedPacketContext, PathMapContext,
               FlowFilterContext {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PacketContext.class);

    /**
     * A map that associates the virtual node type with the reason of
     * virtual node hop.
     */
    private static final Map<VNodeType, VirtualRouteReason>  ROUTE_REASONS;

    /**
     * A {@link SalPort} instance which specifies the ingress switch port.
     */
    private final SalPort  ingressPort;

    /**
     * Decoded ethernet frame.
     */
    private EtherPacket  etherFrame;

    /**
     * The IPv4 address associated with the sender MAC address.
     */
    private Ip4Network  senderIp4Address;

    /**
     * A sequence of virtual packet routing.
     */
    private final List<VNodeHop>  virtualRoute = new ArrayList<>();

    /**
     * A set of {@link FlowMatchType} instances which represents match fields
     * to be configured.
     */
    private final Set<FlowMatchType>  matchFields =
        EnumSet.noneOf(FlowMatchType.class);

    /**
     * A {@link VNodeHop} instance which represents the hop to the egress
     * virtual node.
     */
    private VNodeHop  egressNodeHop;

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
     * A {@link TenantNodeIdentifier} instance which represents the virtual
     * mapping that maps the incoming packet.
     */
    private TenantNodeIdentifier<?, ?>  mapReference;

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
     * Controller's MAC address.
     */
    private EtherAddress  controllerAddress;

    /**
     * Packet transmission queue.
     */
    private PacketOutQueue  outQueue;

    /**
     * Inventory reader.
     */
    private InventoryReader  inventoryReader;

    /**
     * Flow condition reader.
     */
    private FlowCondReader  condReader;

    /**
     * Virtual node reader.
     */
    private VNodeReader  vnodeReader;

    /**
     * The base flow priority.
     */
    private int  baseFlowPriority;

    /**
     * The maximum number of flow redirections.
     */
    private int  maxRedirections;

    /**
     * Initialize static fields.
     */
    static {
        Map<VNodeType, VirtualRouteReason> reasons =
            new EnumMap<>(VNodeType.class);
        reasons.put(VNodeType.VBRIDGE_IF, VirtualRouteReason.PORTMAPPED);
        reasons.put(VNodeType.VTERMINAL_IF, VirtualRouteReason.PORTMAPPED);
        reasons.put(VNodeType.VLANMAP, VirtualRouteReason.VLANMAPPED);
        reasons.put(VNodeType.MACMAP, VirtualRouteReason.MACMAPPED);
        reasons.put(VNodeType.MACMAP_HOST, VirtualRouteReason.MACMAPPED);

        ROUTE_REASONS = ImmutableMap.copyOf(reasons);
    }

    /**
     * Construct a new packet context to handle PACKET_IN message.
     *
     * @param ev  A {@link PacketInEvent} instance.
     */
    public PacketContext(PacketInEvent ev) {
        ingressPort = ev.getIngressPort();

        EtherPacket ether = new EtherPacket(ev.getEthernet());
        etherFrame = ether;

        TxContext ctx = ev.getTxContext();
        txContext = ctx;

        // Check to see if the given packet is destinated to the controller.
        VTNConfig vcfg = ctx.getProvider().getVTNConfig();
        EtherAddress ctlr = vcfg.getControllerMacAddress();
        controllerAddress = ctlr;
        setToController(ether.getDestinationAddress().equals(ctlr));

        // Cache configurations in the vtn-config container.
        baseFlowPriority = vcfg.getL2FlowPriority();
        maxRedirections = vcfg.getMaxRedirections();
    }

    /**
     * Construct a new packet context.
     *
     * <p>
     *   This constructor is only for unit test.
     * </p>
     *
     * @param ctx      A runtime context for transaction task.
     * @param ether    The ethernet frame.
     * @param ingress  The ingress switch port.
     */
    public PacketContext(TxContext ctx, Ethernet ether, SalPort ingress) {
        txContext = ctx;
        etherFrame = new EtherPacket(ether);
        ingressPort = ingress;
    }

    /**
     * Initialize the received packet information.
     *
     * @param ref  A {@link TenantNodeIdentifier} instance that specifies the
     *             virtual mapping that maps the packet.
     */
    public void initPacketIn(TenantNodeIdentifier<?, ?> ref) {
        mapReference = ref;
        if (ref.getType().isMacMap()) {
            // The source MAC address must be specified in the ingress flow
            // entry if the packet is mapped by MAC mapping.
            matchFields.add(FlowMatchType.DL_SRC);
        }

        // Evaluate path maps.
        routeResolver = new PathMapEvaluator(this).evaluate(ref);
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
     * Return the IPv4 address associated with the source MAC address.
     *
     * @return  An {@link Ip4Network} instance that indicates the sender
     *          IPv4 address if found. {@code null} if not found.
     */
    public Ip4Network getSenderIp4Address() {
        Ip4Network src = senderIp4Address;
        if (src == null) {
            Packet payload = etherFrame.getPayload();
            if (payload instanceof ARP) {
                ARP arp = (ARP)payload;
                if (arp.getProtocolType() == EtherTypes.IPV4.shortValue()) {
                    byte[] sender = arp.getSenderProtocolAddress();
                    // Ignore sender protocol address if it is zero.
                    int addr = NumberUtils.toInteger(sender);
                    if (addr != 0) {
                        src = new Ip4Network(addr);
                        senderIp4Address = src;
                    }
                }
            }

            if (src == null) {
                senderIp4Address = new Ip4Network(0);
            }
        } else if (src.getAddress() == 0) {
            // Negative cache.
            src = null;
        }

        return src;
    }

    /**
     * Return a {@link SalPort} instance corresponding to the ingress switch
     * port.
     *
     * @return  A {@link SalPort} instance.
     */
    public SalPort getIngressPort() {
        return ingressPort;
    }

    /**
     * Return the name of the ingress switch port.
     *
     * @return  The name of the ingress switch port.
     *          {@code null} if not available.
     * @throws VTNException  An error occurred.
     */
    public String getIngressPortName() throws VTNException {
        // VtnPort should be found.
        return getInventoryReader().get(ingressPort).getName();
    }

    /**
     * Return a pair of switch port and VLAN ID which indicates the incoming
     * network.
     *
     * @return  A {@link PortVlan} which indicates the incoming network where
     *          the packet was received from.
     */
    public PortVlan getIncomingNetwork() {
        return new PortVlan(ingressPort, etherFrame.getOriginalVlan());
    }

    /**
     * Return VLAN ID of this packet.
     *
     * @return  VLAN ID. Zero is returned if no VLAN tag was found in the
     *          packet.
     */
    public int getVlanId() {
        return etherFrame.getVlanId();
    }

    /**
     * Create a new ethernet frame that forwards the received packet.
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
            ether.setEtherType(EtherTypes.VLAN.shortValue());

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
     * Purge VTN flows relevant to the given obsolete layer 2 host.
     *
     * @param ment  An obsolete MAC address table entry.
     */
    public void removeFlows(MacEntry ment) {
        long mac = ment.getEtherAddress().getAddress();
        L2Host host = new L2Host(mac, ment.getVlanId(), ment.getPort());
        String tname = mapReference.getTenantNameString();
        txContext.getSpecific(FlowRemoverQueue.class).
            enqueue(new EdgeHostFlowRemover(tname, host));
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
     * @param vid    VLAN ID.
     * @return  A brief description of the specified ethernet frame.
     */
    public String getDescription(Ethernet ether, SalPort sport, int vid) {
        EtherAddress src = new EtherAddress(ether.getSourceMACAddress());
        EtherAddress dst = new EtherAddress(ether.getDestinationMACAddress());
        return getDescription(src, dst,
                              (int)(ether.getEtherType() & MASK_ETHER_TYPE),
                              sport, vid);
    }

    /**
     * Create a brief description of the ethernet frame.
     *
     * @param src    The source MAC address.
     * @param dst    The destination MAC address.
     * @param type   The ethernet type.
     * @param sport  A {@link SalPort} instance associated with the ethernet
     *               frame.
     * @param vid    VLAN ID.
     * @return  A brief description of the specified ethernet frame.
     */
    public String getDescription(EtherAddress src, EtherAddress dst, int type,
                                 SalPort sport, int vid) {
        StringBuilder builder = new StringBuilder("src=");
        builder.append(src.getText()).
            append(", dst=").append(dst.getText());
        if (sport != null) {
            builder.append(", port=").append(sport);
        }
        builder.append(", type=0x").append(Integer.toHexString(type)).
            append(", vlan=").append(vid);

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
        return (etherFrame.getEtherType() == EtherTypes.IPV4.intValue());
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
     * Try to probe IP address of the source address of this packet.
     */
    public void probeInetAddress() {
        Inet4Packet ipv4 = getInet4Packet();
        if (ipv4 != null) {
            // Send an ARP request to the source address of this packet.
            EtherAddress src = getControllerAddress();
            Ip4Network tpa = ipv4.getSourceAddress();
            EtherAddress dst = etherFrame.getSourceAddress();
            int vlan = etherFrame.getOriginalVlan();
            Ethernet ether = new ArpPacketBuilder(vlan).build(src, dst, tpa);
            SalPort sport = ingressPort;

            if (LOG.isTraceEnabled()) {
                String dstmac = dst.getText();
                String target = tpa.getText();
                LOG.trace("Sending an ARP request to detect IP address: " +
                          "dst={}, tpa={}, vlan={}, port={}",
                          dstmac, target, vlan, sport);
            }

            transmit(sport, ether);
        }
    }

    /**
     * Append the specified virtual node hop to the virtual packet route.
     *
     * @param vhop  A {@link VNodeHop} instance which represents a routing
     *                to the virtual node.
     */
    public void addVNodeHop(VNodeHop vhop) {
        virtualRoute.add(vhop);
    }

    /**
     * Set the virtual node hop to the egress node.
     *
     * @param vhop  A {@link VNodeHop} instance which represents the hop to
     *              the egress node.
     */
    public void setEgressVNodeHop(VNodeHop vhop) {
        VNodeHop old = egressNodeHop;
        if (old != null) {
            if (vhop.getReason() == VirtualRouteReason.FORWARDED &&
                old.getPath().equals(vhop.getPath())) {
                // No need to record the specified hop because the hop to the
                // specified virtual node is already recorded.
                return;
            }

            // Push the old hop into the virtual node route.
            virtualRoute.add(old);
        }

        egressNodeHop = vhop;
    }

    /**
     * Return a {@link VNodeHop} instance which represents the virtual node
     * hop to the egress node.
     *
     * @return  A {@link VNodeHop} instance.
     *          {@code null} is returned if not configured.
     */
    public VNodeHop getEgressVNodeHop() {
        return egressNodeHop;
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
     * Return the base flow priority.
     *
     * @return  The base flow priority.
     */
    public int getBaseFlowPriority() {
        return baseFlowPriority;
    }

    /**
     * Return the maximum number of flow redirections.
     *
     * @return  The maximum number of flow redirections.
     */
    public int getMaxRedirections() {
        return maxRedirections;
    }

    /**
     * Return a priority value for flow entries.
     *
     * @return  A flow priority value.
     */
    public int getFlowPriority() {
        int nmatches = matchFields.size() -
            FlowMatchType.getUnicastTypeCount(matchFields);
        return (baseFlowPriority + nmatches);
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
        // Prepare to install an unicast flow entry.
        addUnicastMatchFields();

        VTNFlowBuilder builder = createFlowBuilder();
        if (builder != null) {
            // Add flow entries for inter-switch links.
            SalPort src = ingressPort;
            for (LinkEdge le: path) {
                SalPort dst = le.getSourcePort();
                builder.addInternalFlow(src, dst);
                src = le.getDestinationPort();
            }

            // Create the egress flow entry.
            assert src.getNodeNumber() == egress.getNodeNumber();
            int inVid = etherFrame.getOriginalVlan();
            builder.addEgressFlow(src, egress, inVid, outVid,
                                  getFilterActions());

            // Install flow entries.
            addFlow(builder);
        }
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
     * Determine whether at least one flow filter is evaluated with this packet
     * or not.
     *
     * @return  {@code true} only if at least one flow filter is evaluated.
     */
    public boolean isFiltered() {
        return filtered;
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
     * @param rex   A {@link RedirectFlowException} instance that keeps
     *              information about packet redirection.
     * @return  {@code true} if the packet should be forwarded to the
     *          destination interface.
     *          {@code false} if the packet should be discarded.
     */
    public boolean redirect(VInterfaceIdentifier<?> path,
                            RedirectFlowException rex) {
        fixIngressNode();

        // Record the packet redirection as the hop to the egress node.
        setEgressVNodeHop(new VNodeHop(path, VirtualRouteReason.REDIRECTED));

        // Increment the virtual node hops.
        int hops = virtualNodeHops + 1;
        virtualNodeHops = hops;

        boolean result = (hops <= maxRedirections);
        if (!result) {
            txContext.log(LOG, VTNLogLevel.ERROR,
                          "{}: Discard packet: Too many hops: to={}, " +
                          "cond={}, hops={}, packet={}", rex.getFilterPath(),
                          rex.getDestination(), rex.getCondition(), hops,
                          getDescription());
            installDropFlow();
        }

        return result;
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
     * Return the MAC address of the controller.
     *
     * @return  An {@link EtherAddress} instance that indicates the MAC address
     *          of the controller.
     */
    public EtherAddress getControllerAddress() {
        EtherAddress eaddr = controllerAddress;
        if (eaddr == null) {
            eaddr = txContext.getProvider().getVTNConfig().
                getControllerMacAddress();
            controllerAddress = eaddr;
        }

        return eaddr;
    }

    /**
     * Transmit the given packet.
     *
     * <p>
     *   The given packet is enqueued to the packet transmission queue, and
     *   will be transmitted when the MD-SAL datastore transaction completes
     *   successfully.
     * </p>
     *
     * @param egress  A {@link SalPort} instance which specifies the egress
     *                switch port.
     * @param packet  A {@link Packet} instance to transmit.
     */
    public void transmit(SalPort egress, Packet packet) {
        PacketOutQueue outq = outQueue;
        if (outq == null) {
            outq = txContext.getSpecific(PacketOutQueue.class);
            outQueue = outq;
        }

        outq.enqueue(egress, packet);
    }

    /**
     * Install the given data flow.
     *
     * <p>
     *   The given data flow is enqueued to the flow installation request
     *   queue, and it will be installed when the MD-SAL datastore transaction
     *   completes successfully.
     * </p>
     *
     * @param builder  A {@link VTNFlowBuilder} instance which contains
     *                 data flow to be installed.
     */
    public void addFlow(VTNFlowBuilder builder) {
        txContext.getSpecific(FlowAddQueue.class).enqueue(builder);
    }

    /**
     * Return the virtual node reader associated with the current MD-SAL
     * datastore transaction.
     *
     * @return  A {@link VNodeReader} instance.
     */
    public VNodeReader getVNodeReader() {
        VNodeReader reader = vnodeReader;
        if (reader == null) {
            reader = txContext.getSpecific(VNodeReader.class);
            vnodeReader = reader;
        }

        return vnodeReader;
    }

    /**
     * Evaluate the given flow filter list against the packet.
     *
     * @param fflist  A {@link FlowFilterList} instance.
     * @param vid     A VLAN ID to be used for packet matching.
     *                A VLAN ID configured in the given packet is used if a
     *                negative value is specified.
     * @return  A {@link PacketContext} to be used for succeeding packet
     *          processing.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter configured in
     *    this instance.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter configured in
     *    this instance.
     */
    public PacketContext evaluate(FlowFilterList fflist, int vid)
        throws DropFlowException, RedirectFlowException {
        PacketContext pc = this;
        if (!fflist.isEmpty()) {
            filtered = true;
            if (filterDisabled) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}: Flow filter is disabled: {}",
                              fflist.getListId(), getDescription());
                }
            } else {
                FlowFilterListId flid = fflist.getListId();
                if (flooding && flid.isOutput()) {
                    // This packet is going to be broadcasted.
                    // So we need to preserve the original packet for
                    // succeeding transmission.
                    pc = clone();
                }

                // Evaluate the given flow filters.
                fflist.evaluate(pc, vid);
            }
        }

        return pc;
    }

    /**
     * Return the inventory reader associated with the current MD-SAL datastoren
     * transaction.
     *
     * @return  An {@link InventoryReader} instance.
     */
    public InventoryReader getInventoryReader() {
        InventoryReader reader = inventoryReader;
        if (reader == null) {
            reader = txContext.getReadSpecific(InventoryReader.class);
            inventoryReader = reader;
        }

        return reader;
    }

    /**
     * Create a new VTN flow builder with specifying the flow match.
     *
     * <p>
     *   Note that this method configures uses the base flow priority.
     * </p>
     *
     * @param tname   The name of the VTN.
     * @param vmatch  A {@link VTNMatch} instance.
     * @return  A {@link VTNFlowBuilder} instance.
     */
    public VTNFlowBuilder createFlowBuilder(String tname, VTNMatch vmatch) {
        return new VTNFlowBuilder(
            tname, vmatch, baseFlowPriority, idleTimeout, hardTimeout);
    }

    /**
     * Fix up the ingress virtual node.
     */
    private void fixIngressNode() {
        if (virtualRoute.isEmpty() && mapReference != null) {
            // This can happen if the packet was discarded by the VTN flow
            // filter. In this case we need to estimate ingress virtual node
            // route from mapping reference.
            VNodeType vtype = mapReference.getType();
            VirtualRouteReason reason = ROUTE_REASONS.get(vtype);
            assert reason != null;
            virtualRoute.add(new VNodeHop(mapReference, reason));
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
        VNodeHop vhop = virtualRoute.get(0);
        String tname = vhop.getPath().getTenantNameString();
        int pri = getFlowPriority();
        VTNFlowBuilder builder = new VTNFlowBuilder(
            tname, vmatch, pri, idleTimeout, hardTimeout);

        // Set the virtual packet routing path and path policy identifier.
        return builder.addVirtualRoute(virtualRoute).
            setEgressVNodeHop(egressNodeHop).
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
            txContext.log(LOG, VTNLogLevel.ERROR,
                          "Failed to create VTNMatch.", e);
            return null;
        }
    }

    /**
     * Read the flow condition specified by the given condition name.
     *
     * @param cond  The name of the flow condition.
     * @return  A {@link VTNFlowCondition} instance if found.
     *          {@code null} if not found.
     */
    private VTNFlowCondition getFlowCondition(String cond) {
        return getFlowCondReader().get(cond);
    }

    /**
     * Invoked when the destination virtual interface of the packet redirection
     * was not found.
     *
     * @param rex  A {@link RedirectFlowException} instance.
     * @param msg  An error message.
     */
    public void destinationNotFound(RedirectFlowException rex, String msg) {
        txContext.log(LOG, VTNLogLevel.ERROR,
                      "{}: Discard packet: {}: to={}, cond={}, packet={}",
                      rex.getFilterPath(), msg, rex.getDestination(),
                      rex.getCondition(), getDescription());
        installDropFlow();
    }

    /**
     * Invoked when the destination virtual interface of the packet redirection
     * was disabled.
     *
     * @param rex  A {@link RedirectFlowException} instance.
     */
    public void destinationDisabled(RedirectFlowException rex) {
        txContext.log(LOG, VTNLogLevel.ERROR,
                      "{}: Discard packet: Destination is disabled: to={}, " +
                      "cond={}, packet={}", rex.getFilterPath(),
                      rex.getDestination(), rex.getCondition(),
                      getDescription());
    }

    /**
     * Invoked when the physical switch port is not mapped to the destination
     * virtual interface.
     *
     * @param rex  A {@link RedirectFlowException} instance.
     */
    public void notMapped(RedirectFlowException rex) {
        txContext.log(LOG, VTNLogLevel.ERROR,
                      "{}: Discard packet: Switch port is not mapped: " +
                      "to={}, cond={}, packet={}", rex.getFilterPath(),
                      rex.getDestination(), rex.getCondition(),
                      getDescription());
        installDropFlow();
    }

    /**
     * Invoked when the final destination of the unicast packet redirection is
     * determined.
     *
     * @param rex    A {@link RedirectFlowException} instance.
     * @param to     The identifier for the egress virtual node.
     * @param sport  A {@link SalPort} corresponding to the outgoing physical
     *               switch port.
     * @param vid    A VLAN ID to be set to the outgoing packet.
     */
    public void forwarded(RedirectFlowException rex, VNodeIdentifier<?> to,
                          SalPort sport, int vid) {
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: Packet was redirected to {}: port={}, vlan={}, " +
                      "packet={}", rex.getFilterPath(), to, sport, vid,
                      getDescription());
        }
    }

    /**
     * Invoked when the redirected unicast packet was broadcasted to the
     * bridge.
     *
     * @param rex    A {@link RedirectFlowException} instance.
     * @param ident  The identifier for the virtual node where the packet
     *               was broadcasted.
     */
    public void broadcasted(RedirectFlowException rex,
                            VNodeIdentifier<?> ident) {
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: Packet was broadcasted into {}: packet={}",
                      rex.getFilterPath(), ident, getDescription());
        }
    }

    // Object

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

    // FlowActionContext

    /**
     * {@inheritDoc}
     */
    @Override
    public void addFilterAction(FlowFilterAction act) {
        if (!flooding) {
            if (filterActions == null) {
                filterActions = new LinkedHashMap<>();
            }
            filterActions.put(act.getClass(), act);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeFilterAction(Class<? extends FlowFilterAction> actClass) {
        if (!flooding && filterActions != null) {
            filterActions.remove(actClass);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<FlowFilterAction> getFilterActions() {
        return (filterActions == null) ? null : filterActions.values();
    }

    // PathMapContext

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNManagerProvider getProvider() {
        return txContext.getProvider();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowCondReader getFlowCondReader() {
        FlowCondReader reader = condReader;
        if (reader == null) {
            reader = txContext.getReadSpecific(FlowCondReader.class);
            condReader = reader;
        }

        return reader;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setFlowTimeout(int idle, int hard) {
        idleTimeout = idle;
        hardTimeout = hard;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return getDescription(ingressPort);
    }

    // FlowFilterContext

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isFlooding() {
        return flooding;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setVlanId(int vid) {
        etherFrame.setVlanId(vid);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void installDropFlow() {
        // Never install flow entry on packet flooding.
        if (!flooding) {
            if (isUnicast()) {
                // The source and destination MAC address must be specified in
                // a drop flow entry if this packet is an unicast packet.
                FlowMatchType.addUnicastTypes(matchFields);
            }

            // Create a flow entry that discards the given packet.
            VTNFlowBuilder builder = createFlowBuilder();
            if (builder != null) {
                if (egressNodeHop != null) {
                    builder.setEgressVNodeHop(null);
                }

                addFlow(builder.addDropFlow(ingressPort));
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNFlowCondition getFlowCondition(VTNFlowFilter ff)
        throws UnsupportedPacketException {
        if (!isUnicast() && !ff.isMulticastSupported()) {
            throw new UnsupportedPacketException(
                "multicast packet is not supported");
        }

        if (isFlooding() && !ff.isFloodingSuppoted()) {
            throw new UnsupportedPacketException(
                "flooding packet is not supported");
        }

        String cond = ff.getCondition();
        VTNFlowCondition vfcond = getFlowCondition(cond);
        if (vfcond == null) {
            throw new UnsupportedPacketException(
                "Flow condition not found: " + cond);
        }

        return vfcond;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean setFirstRedirection(RedirectFlowException rex) {
        boolean first = (firstRedirection == null);
        if (first) {
            firstRedirection = rex;
        }

        return first;
    }
}
