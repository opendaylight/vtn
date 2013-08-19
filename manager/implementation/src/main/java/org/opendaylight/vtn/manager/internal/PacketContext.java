/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.Map;
import java.util.TreeMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.switchmanager.ISwitchManager;

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
    private final static Logger  LOG =
        LoggerFactory.getLogger(PacketContext.class);

    /**
     * A received raw packet.
     */
    private final RawPacket  rawPacket;

    /**
     * IEEE 802.1Q header.
     */
    private final IEEE8021Q  vlanTag;

    /**
     * Decoded ethernet frame.
     */
    private final Ethernet  etherFrame;

    /**
     * Payload of the packet.
     */
    private final Packet  payload;

    /**
     * Unparsed payload.
     */
    private final byte[]  rawPayload;

    /**
     * Source MAC address.
     */
    private byte[]  sourceAddress;

    /**
     * Destination MAC address.
     */
    private byte[]  destinationAddress;

    /**
     * Source IP address.
     */
    private byte[] sourceInetAddress;

    /**
     * Obsolete MAC address table entries.
     */
    private TreeMap<Long, MacTableEntry>  obsoleteEntries =
        new TreeMap<Long, MacTableEntry>();

    /**
     * Outgoing node connector.
     */
    private NodeConnector  outgoing;

    /**
     * Construct a new packet context.
     *
     * @param raw    A received raw packet.
     * @param ether  Decoded ethernet frame.
     */
    PacketContext(RawPacket raw, Ethernet ether) {
        rawPacket = raw;
        etherFrame = ether;

        Packet parent = ether;
        Packet pld = ether.getPayload();
        if (pld instanceof IEEE8021Q) {
            // This packet has a VLAN tag.
            vlanTag = (IEEE8021Q)pld;
            pld = vlanTag.getPayload();
            parent = vlanTag;
        } else {
            vlanTag = null;
        }

        payload = pld;
        rawPayload = parent.getRawPayload();
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
        return etherFrame;
    }

    /**
     * Return the source MAC address.
     *
     * @return  The source MAC address.
     */
    public byte[] getSourceAddress() {
        if (sourceAddress == null) {
            sourceAddress = etherFrame.getSourceMACAddress();
        }
        return sourceAddress;
    }

    /**
     * Return the destination MAC address.
     *
     * @return  The destination MAC address.
     */
    public byte[] getDestinationAddress() {
        if (destinationAddress == null) {
            destinationAddress = etherFrame.getDestinationMACAddress();
        }
        return destinationAddress;
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
        return payload;
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
        return (vlanTag == null) ? 0 : vlanTag.getVid();
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
        ether.setSourceMACAddress(getSourceAddress()).
            setDestinationMACAddress(getDestinationAddress());

        short ethType;
        if (vlan != 0) {
            // Add a VLAN tag.
            IEEE8021Q tag = new IEEE8021Q();
            byte cfi, pcp;
            if (vlanTag != null) {
                cfi = vlanTag.getCfi();
                pcp = vlanTag.getPcp();
                ethType = vlanTag.getEtherType();
            } else {
                cfi = (byte)0;
                pcp = (byte)0;
                ethType = etherFrame.getEtherType();
            }
            tag.setCfi(cfi).setPcp(pcp).setVid(vlan).setEtherType(ethType);
            ether.setEtherType(EtherTypes.VLANTAGGED.shortValue());

            // Set payload to IEEE 802.1Q header.
            if (payload != null) {
                tag.setPayload(payload);
            } else if (rawPacket != null) {
                tag.setRawPayload(rawPayload);
            }

            // Set IEEE 802.1Q header as payload.
            ether.setPayload(tag);
        } else {
            ethType = (vlanTag == null)
                ? etherFrame.getEtherType()
                : vlanTag.getEtherType();
            ether.setEtherType(ethType);
            if (payload != null) {
                ether.setPayload(payload);
            } else if (rawPayload != null) {
                ether.setRawPayload(rawPayload);
            }
        }

        return ether;
    }

    /**
     * Add the specified MAC address table entry to obsolete entry map.
     * packet.
     *
     * @param key   A long value which represents a MAC address.
     * @param tent  An obsolete MAC address table entry.
     */
    public void addObsoleteEntry(Long key, MacTableEntry tent) {
        obsoleteEntries.put(key, tent);
    }

    /**
     * Return a set of obsolete MAC address table entries.
     *
     * @return  A set of obsolete MAC address table entries.
     */
    public Map<Long, MacTableEntry> getObsoleteEntries() {
        return obsoleteEntries;
    }

    /**
     * Create a brief description of the ethernet frame in this context.
     *
     * @param port  A node connector associated with the ethernet frame.
     * @return  A brief description of the ethernet frame in ths context.
     */
    public String getDescription(NodeConnector port) {
        return getDescription(etherFrame, port, getVlan());
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
        String srcmac = HexEncode.
            bytesToHexStringFormat(ether.getSourceMACAddress());
        String dstmac = HexEncode.
            bytesToHexStringFormat(ether.getDestinationMACAddress());
        int type = (int)ether.getEtherType() & 0xffff;

        StringBuilder builder = new StringBuilder("src=");
        builder.append(srcmac).
            append(", dst=").append(dstmac).
            append(", port=").append(port).
            append(", type=0x").append(Integer.toHexString(type)).
            append(", vlan=").append((int)vlan);

        return builder.toString();
    }

    /**
     * Try to probe IP address of the source address of this packet.
     *
     * @param mgr  VTN Manager service.
     */
    public void probeInetAddress(VTNManagerImpl mgr) {
        if (payload instanceof IPv4) {
            // Send an ARP request to the source address of this packet.
            ISwitchManager swMgr = mgr.getSwitchManager();
            IPv4 ipv4 = (IPv4)payload;
            int srcIp = ipv4.getSourceAddress();
            byte[] dst = getSourceAddress();
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
}
