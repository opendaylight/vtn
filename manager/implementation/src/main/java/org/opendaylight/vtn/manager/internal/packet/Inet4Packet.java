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

import org.opendaylight.vtn.manager.internal.MiscUtils;

import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code Inet4Packet} class implements a cache for an {@link IPv4} instance.
 */
public final class Inet4Packet implements CachedPacket {
    /**
     * A pseudo IP protocol number which indicates the IP protocol number is
     * not specified.
     */
    private static final short  PROTO_NONE = -1;

    /**
     * A pseudo DSCP value which indicates the DSCP value is not specified.
     */
    private static final byte  DSCP_NONE = -1;

    /**
     * An {@link IPv4} packet.
     */
    private final IPv4  packet;

    /**
     * An integer value which indicates the source IP address.
     */
    private int  sourceAddress;

    /**
     * An integer value which indicates the destination IP address.
     */
    private int  destinationAddress;

    /**
     * A boolean value which determines whether {@link #sourceAddress} is
     * initialized or not.
     */
    private boolean sourceSet;

    /**
     * A boolean value which determines whether {@link #destinationAddress} is
     * initialized or not.
     */
    private boolean destinationSet;

    /**
     * The IP protocol number in the IPv4 packet.
     */
    private short  protocol = PROTO_NONE;

    /**
     * The DSCP field value in the IPv4 packet.
     */
    private byte  dscp = PROTO_NONE;

    /**
     * Construct a new instance.
     *
     * @param ipv4  An {@link IPv4} instance.
     */
    public Inet4Packet(IPv4 ipv4) {
        packet = ipv4;
    }

    /**
     * Return the source IP address.
     *
     * @return  An integer value which represents the source IP address.
     */
    public int getSourceAddress() {
        if (!sourceSet) {
            sourceAddress = packet.getSourceAddress();
            sourceSet = true;
        }

        return sourceAddress;
    }

    /**
     * Return the destination IP address.
     *
     * @return  An integer value which represents the destination IP address.
     */
    public int getDestinationAddress() {
        if (!destinationSet) {
            destinationAddress = packet.getDestinationAddress();
            destinationSet = true;
        }

        return destinationAddress;
    }

    /**
     * Return the IP protocol number.
     *
     * @return  A short integer value which indicates the IP protocol number.
     */
    public short getProtocol() {
        if (protocol == PROTO_NONE) {
            protocol = (short)NetUtils.getUnsignedByte(packet.getProtocol());
        }

        return protocol;
    }

    /**
     * Return the DSCP field value.
     *
     * @return  A byte value which indicates the DSCP field value.
     */
    public byte getDscp() {
        if (dscp == DSCP_NONE) {
            dscp = packet.getDiffServ();
        }

        return dscp;
    }

    // CachedPacket

    /**
     * Return an {@link IPv4} instance configured in this instance.
     *
     * @return  An {@link IPv4} instance.
     */
    @Override
    public IPv4 getPacket() {
        return packet;
    }

    /**
     * Configure match fields to test IP header in this packet.
     *
     * @param match   A {@link Match} instance.
     * @param fields  A set of {@link MatchType} instances corresponding to
     *                match fields to be tested.
     */
    @Override
    public void setMatch(Match match, Set<MatchType> fields) {
        MatchType type = MatchType.NW_SRC;
        if (fields.contains(type)) {
            // Test source IP address.
            match.setField(type, MiscUtils.toInetAddress(getSourceAddress()));
        }

        type = MatchType.NW_DST;
        if (fields.contains(type)) {
            // Test destination IP address.
            match.setField(type,
                           MiscUtils.toInetAddress(getDestinationAddress()));
        }

        type = MatchType.NW_PROTO;
        if (fields.contains(type)) {
            // Test IP protocol number.
            match.setField(type, (byte)getProtocol());
        }

        type = MatchType.NW_TOS;
        if (fields.contains(type)) {
            // Test DSCP field.
            match.setField(type, getDscp());
        }
    }
}
