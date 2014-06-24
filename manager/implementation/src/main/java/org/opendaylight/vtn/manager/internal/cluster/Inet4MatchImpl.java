/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.Objects;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.Inet4Packet;

import org.opendaylight.controller.sal.utils.EtherTypes;

/**
 * {@code Inet4MatchImpl} describes the condition to match IPv4 header fields
 * in packet.
 *
 * <p>
 *   Although this interface is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class Inet4MatchImpl extends InetMatchImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2950598600572887764L;

    /**
     * A source IP address to match against packets.
     */
    private final Inet4AddressMatch  source;

    /**
     * A destination IP address to match against packets.
     */
    private final Inet4AddressMatch  destination;

    /**
     * Construct a new instance that contains only the condition for the
     * IP protocol number.
     *
     * @param proto  An IP protocol number to match.
     */
    public Inet4MatchImpl(short proto) {
        super(proto);
        source = null;
        destination = null;
    }

    /**
     * Construct a new instance.
     *
     * @param match  An {@link InetMatch} instance.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws VTNException
     *    {@code match} contains invalid value.
     */
    public Inet4MatchImpl(InetMatch match) throws VTNException {
        super(match);

        InetAddress addr = match.getSourceAddress();
        if (addr == null) {
            source = null;
        } else {
            Short suffix = match.getSourceSuffix();
            source = new Inet4AddressMatch(addr, suffix);
        }

        addr = match.getDestinationAddress();
        if (addr == null) {
            destination = null;
        } else {
            Short suffix = match.getDestinationSuffix();
            destination = new Inet4AddressMatch(addr, suffix);
        }
    }

    /**
     * Return an {@link Inet4AddressMatch} instance which represents the source
     * IP address to match against packets.
     *
     * @return  An {@link Inet4AddressMatch} instance which represents a
     *          source IP address. {@code null} is returned if this condition
     *          does not specify the source IP address.
     */
    public Inet4AddressMatch getSource() {
        return source;
    }

    /**
     * Return an {@link Inet4AddressMatch} instance which represents the
     * destination IP address to match against packets.
     *
     * @return  An {@link Inet4AddressMatch} instance which represents the
     *          destination IP address. {@code null} is returned if this
     *          condition does not specify the destination IP address.
     */
    public Inet4AddressMatch getDestination() {
        return destination;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!super.equals(o)) {
            return false;
        }

        Inet4MatchImpl match = (Inet4MatchImpl)o;
        return (Objects.equals(source, match.source) &&
                Objects.equals(destination, match.destination));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(source, destination) + (super.hashCode() * 31);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("Inet4MatchImpl[");
        String sep = "";
        if (source != null) {
            builder.append("src=").append(source.toString());
            sep = ",";
        }
        if (destination != null) {
            builder.append(sep).append("dst=").append(destination.toString());
            sep = ",";
        }

        short proto = getProtocol();
        if (proto >= 0) {
            builder.append(sep).append("proto=").append((int)proto);
            sep = ",";
        }

        byte dscp = getDscp();
        if (dscp >= 0) {
            builder.append(sep).append("dscp=").append((int)dscp);
        }
        builder.append(']');

        return builder.toString();
    }

    // PacketMatch

    /**
     * Determine whether the specified packet matches the condition defined
     * by this instance.
     *
     * @param pctx  The context of the packet to be tested.
     * @return  {@code true} if the specified packet matches the condition.
     *          Otherwise {@code false}.
     */
    @Override
    public boolean match(PacketContext pctx) {
        Inet4Packet ipv4 = pctx.getInet4Packet();
        if (ipv4 == null) {
            return false;
        }

        // Test source IP address.
        if (source != null && !source.match(ipv4.getSourceAddress())) {
            return false;
        }

        // Test destination IP address.
        if (destination != null &&
            !destination.match(ipv4.getDestinationAddress())) {
            return false;
        }

        return match(ipv4.getProtocol(), ipv4.getDscp());
    }

    // InetMatchImpl

    /**
     * Return an Ethernet protocol type assigned to this protocol.
     *
     * @return  An Ethernet protocol type for IPv4.
     */
    @Override
    public int getEtherType() {
        return EtherTypes.IPv4.intValue();
    }

    /**
     * Return an {@link InetMatch} instance which represents this condition.
     *
     * @return  An {@link InetMatch} instance.
     */
    @Override
    public InetMatch getMatch() {
        InetAddress src, dst;
        Short srcSuff, dstSuff;
        if (source == null) {
            src = null;
            srcSuff = null;
        } else {
            src = source.getInetAddress();
            srcSuff = source.getCidrSuffix();
        }

        if (destination == null) {
            dst = null;
            dstSuff = null;
        } else {
            dst = destination.getInetAddress();
            dstSuff = destination.getCidrSuffix();
        }

        return new Inet4Match(src, srcSuff, dst, dstSuff, getProtocolShort(),
                              getDscpByte());
    }
}
