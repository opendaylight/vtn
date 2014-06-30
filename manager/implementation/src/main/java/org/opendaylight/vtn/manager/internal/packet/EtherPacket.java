/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.util.Set;

import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code EtherPacket} class implements a cache for a {@link Ethernet}
 * instance including VLAN tag.
 */
public final class EtherPacket implements CachedPacket {
    /**
     * A pseudo MAC address which represents the MAC address is not specified.
     */
    private static final long  MAC_NONE = -1L;

    /**
     * A pseudo VLAN ID which represents the VLAN ID is not specified.
     */
    private static final short  VLAN_NONE = -1;

    /**
     * A pseudo VLAN priority which represents the VLAN priority is not
     * specified.
     */
    private static final byte VLANPRI_NONE = -1;

    /**
     * A pseudo VLAN priority which represents the VLAN priority is not
     * configured in Ethernet frame.
     */
    private static final byte VLANPRI_UNDEF = -2;

    /**
     * An {@link Ethernet} instance.
     */
    private final Ethernet  packet;

    /**
     * An {@link IEEE8021Q} instance which represents a VLAN tag.
     */
    private final IEEE8021Q  vlanTag;

    /**
     * A byte array which represents the source MAC address.
     */
    private byte[]  sourceAddress;

    /**
     * A byte array which represents the destination MAC address.
     */
    private byte[]  destinationAddress;

    /**
     * A long value which represents the source MAC address.
     */
    private long  sourceMac = MAC_NONE;

    /**
     * A long value which represents the destination MAC address.
     */
    private long  destinationMac = MAC_NONE;

    /**
     * The ethernet type.
     */
    private final int  etherType;

    /**
     * The VLAN ID.
     */
    private short  vlan = VLAN_NONE;

    /**
     * The VLAN priority.
     */
    private byte  vlanPriority = VLANPRI_NONE;

    /**
     * Payload of the Ethernet frame.
     */
    private final Packet  payload;

    /**
     * Unparsed payload of the Ethernet frame.
     */
    private final byte[]  rawPayload;

    /**
     * Construct a new instance.
     *
     * @param ether  An {@link Ethernet} instance.
     */
    public EtherPacket(Ethernet ether) {
        packet = ether;

        Packet parent = ether;
        Packet pld = ether.getPayload();
        short ethType;
        if (pld instanceof IEEE8021Q) {
            // This packet has a VLAN tag.
            vlanTag = (IEEE8021Q)pld;
            pld = vlanTag.getPayload();
            ethType = vlanTag.getEtherType();
            parent = vlanTag;
        } else {
            ethType = ether.getEtherType();
            vlanTag = null;
        }

        etherType = NetUtils.getUnsignedShort(ethType);
        payload = pld;
        rawPayload = parent.getRawPayload();
    }

    /**
     * Return the source MAC address as a byte array.
     *
     * @return  The source MAC address.
     */
    public byte[] getSourceAddress() {
        if (sourceAddress == null) {
            sourceAddress = packet.getSourceMACAddress();
        }

        return sourceAddress;
    }

    /**
     * Return the destination MAC address as a byte array.
     *
     * @return  The destination MAC address.
     */
    public byte[] getDestinationAddress() {
        if (destinationAddress == null) {
            destinationAddress = packet.getDestinationMACAddress();
        }

        return destinationAddress;
    }

    /**
     * Return the source MAC address as a long integer.
     *
     * @return  The source MAC address.
     */
    public long getSourceMacAddress() {
        if (sourceMac == MAC_NONE) {
            byte[] addr = getSourceAddress();
            sourceMac = NetUtils.byteArray6ToLong(addr);
        }

        return sourceMac;
    }

    /**
     * Return the destination MAC address as a long integer.
     *
     * @return  The destination MAC address.
     */
    public long getDestinationMacAddress() {
        if (destinationMac == MAC_NONE) {
            byte[] addr = getDestinationAddress();
            destinationMac = NetUtils.byteArray6ToLong(addr);
        }

        return destinationMac;
    }

    /**
     * Return the Ethernet type.
     *
     * @return  An integer value which represents the Ethernet type.
     */
    public int getEtherType() {
        return etherType;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  A short value which represents the VLAN ID.
     *          Zero is returned if no VLAN tag was found in the Ethernet
     *          frame.
     */
    public short getVlan() {
        if (vlan == VLAN_NONE) {
            vlan = (vlanTag == null) ? 0 : vlanTag.getVid();
        }

        return vlan;
    }

    /**
     * Return the VLAN priority.
     *
     * @return  A byte value which represents the VLAN priority.
     *          A negative value is returned if no VLAN tag was found in
     *          the Ethernet frame.
     */
    public byte getVlanPriority() {
        if (vlanPriority == VLANPRI_NONE) {
            vlanPriority = (vlanTag == null)
                ? VLANPRI_UNDEF : vlanTag.getPcp();
        }

        return vlanPriority;
    }

    /**
     * Return the VLAN tag in the Ethernet frame.
     *
     * @return  An {@link IEEE8021Q} instance which represents the VLAN tag.
     *          {@code null} is returned if no VLAN tag was found in the
     *          Ethernet frame.
     */
    public IEEE8021Q getVlanTag() {
        return vlanTag;
    }

    /**
     * Return the payload in the Ethernet frame.
     *
     * @return  A {@link Packet} instance which represents the payload.
     */
    public Packet getPayload() {
        return payload;
    }

    /**
     * Return the unparsed payload in the Ethernet frame.
     *
     * @return  A byte array which represents the payload in the Ethernet
     *          frame.
     */
    public byte[] getRawPayload() {
        return rawPayload;
    }

    // CachedPacket

    /**
     * Return an {@link Ethernet} instance configured in this instance.
     *
     * @return  An {@link Ethernet} instance.
     */
    @Override
    public Ethernet getPacket() {
        return packet;
    }

    /**
     * Configure match fields to test Ethernet header in this packet.
     *
     * @param match   A {@link Match} instance.
     * @param fields  A set of {@link MatchType} instances corresponding to
     *                match fields to be tested.
     */
    @Override
    public void setMatch(Match match, Set<MatchType> fields) {
        // Source and destination MAC address, and VLAN ID fields are
        // mandatory.
        match.setField(MatchType.DL_SRC, getSourceAddress());
        match.setField(MatchType.DL_DST, getDestinationAddress());

        // This code expects MatchType.DL_VLAN_NONE is zero.
        short vid = getVlan();
        match.setField(MatchType.DL_VLAN, vid);

        // Test VLAN priority only if this packet has a VLAN tag.
        if (vid != MatchType.DL_VLAN_NONE) {
            MatchType type = MatchType.DL_VLAN_PR;
            if (fields.contains(type)) {
                match.setField(type, getVlanPriority());
            }
        }

        MatchType type = MatchType.DL_TYPE;
        if (fields.contains(type)) {
            // Test Ethernet type.
            match.setField(type, (short)etherType);
        }
    }
}
