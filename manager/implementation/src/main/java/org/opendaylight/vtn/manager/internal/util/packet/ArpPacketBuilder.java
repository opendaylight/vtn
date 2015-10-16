/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.packet;

import java.net.Inet4Address;
import java.net.InetAddress;

import org.opendaylight.vtn.manager.packet.ARP;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.packet.IEEE8021Q;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.Ip4Network;

/**
 * {@code ArpPacketBuilder} is a utility to build an ARP packet.
 */
public final class ArpPacketBuilder {
    /**
     * The sender protocol address.
     */
    private Ip4Network  senderProtocolAddress;

    /**
     * ARP operation code.
     */
    private short  operation;

    /**
     * VLAN ID for an ARP packet.
     */
    private int  vlanId;

    /**
     * Construct a new instance that builds an ARP request to be sent to
     * the untagged network.
     */
    public ArpPacketBuilder() {
        this(EtherHeader.VLAN_NONE);
    }

    /**
     * Construct a new instance that builds an ARP request to be sent to the
     * specified VLAN.
     *
     * @param vid  The VLAN ID that specifies the VLAN.
     *             {@link EtherHeader#VLAN_NONE} indicates the untagged
     *             network.
     */
    public ArpPacketBuilder(int vid) {
        this(vid, ARP.REQUEST);
    }

    /**
     * Construct a new instance that builds an ARP packet.
     *
     * @param vid  The VLAN ID that specifies the VLAN.
     *             {@link EtherHeader#VLAN_NONE} indicates the untagged
     *             network.
     * @param op   ARP operation code.
     */
    public ArpPacketBuilder(int vid, short op) {
        vlanId = vid;
        operation = op;
    }

    /**
     * Set the sender protocol address.
     *
     * @param ip4  An {@link Ip4Network} instance which indicates the
     *             sender protocol address. Note that the prefix length is
     *             always ignored.
     * @return  This instance.
     */
    public ArpPacketBuilder setSenderProtocolAddress(Ip4Network ip4) {
        senderProtocolAddress = ip4;
        return this;
    }

    /**
     * Create a broadcast Ethernet frame which contains an ARP request message
     * to probe the given IP address.
     *
     * @param src   An {@link EtherAddress} instance to be used as the source
     *              MAC address.
     * @param addr  The target IP address.
     * @return  An Ethernet frame. {@code null} is returned if {@code addr}
     *          is invalid.
     */
    public Ethernet build(EtherAddress src, InetAddress addr) {
        return build(src, EtherAddress.BROADCAST, addr);
    }

    /**
     * Create an Ethernet frame which contains an ARP request message to
     * probe the given IP address.
     *
     * @param src   An {@link EtherAddress} instance to be used as the source
     *              MAC address.
     * @param dst   The destination MAC address.
     * @param addr  The target IP address.
     * @return  An Ethernet frame. {@code null} is returned if {@code addr}
     *          is invalid.
     */
    public Ethernet build(EtherAddress src, EtherAddress dst,
                          InetAddress addr) {
        // IP address must be an IPv4 address.
        return (addr instanceof Inet4Address)
            ? build(src, dst, new Ip4Network(addr))
            : null;
    }

    /**
     * Create an Ethernet frame which contains an ARP message.
     *
     * @param src     Source MAC address.
     * @param dst     Destination MAC address.
     * @param target  Target IP address.
     * @return  An Ethernet frame.
     */
    public Ethernet build(EtherAddress src, EtherAddress dst,
                          Ip4Network target) {
        byte[] sha = src.getBytes();
        byte[] dstMac = dst.getBytes();
        byte[] tha = (dst.isBroadcast())
            ? new byte[EtherAddress.SIZE] : dstMac;
        Ip4Network sender = senderProtocolAddress;
        byte[] spa = (sender == null)
            ? new byte[Ip4Network.SIZE] : sender.getBytes();
        byte[] tpa = target.getBytes();

        // Create an ARP request message.
        ARP arp = new ARP();
        arp.setHardwareType(ARP.HW_TYPE_ETHERNET).
            setProtocolType(EtherTypes.IPV4.shortValue()).
            setHardwareAddressLength((byte)EtherAddress.SIZE).
            setProtocolAddressLength((byte)Ip4Network.SIZE).
            setOpCode(operation).
            setSenderHardwareAddress(sha).setSenderProtocolAddress(spa).
            setTargetHardwareAddress(tha).setTargetProtocolAddress(tpa);

        short ethType = EtherTypes.ARP.shortValue();
        Packet payload = arp;
        if (vlanId != EtherHeader.VLAN_NONE) {
            // Add a VLAN tag.
            IEEE8021Q tag = new IEEE8021Q();
            tag.setCfi((byte)0).setPcp((byte)0).setVid((short)vlanId).
                setEtherType(ethType).setPayload(arp);
            ethType = EtherTypes.VLAN.shortValue();
            payload = tag;
        }

        // Create an ethernet frame.
        Ethernet ether = new Ethernet();
        ether.setSourceMACAddress(sha).setDestinationMACAddress(dstMac).
            setEtherType(ethType).setPayload(payload);

        return ether;
    }
}
