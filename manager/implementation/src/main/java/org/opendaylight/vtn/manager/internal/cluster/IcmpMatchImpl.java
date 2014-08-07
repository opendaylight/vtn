/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.CachedPacket;
import org.opendaylight.vtn.manager.internal.packet.IcmpPacket;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code IcmpMatchImpl} describes the condition to match ICMP header fields
 * in IPv4 packet.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class IcmpMatchImpl extends L4MatchImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5344880600123638848L;

    /**
     * A value which indicates every ICMP type and code should match.
     */
    private static final short  VALUE_ANY = -1;

    /**
     * A mask value which represents valid bits in ICMP type and code.
     */
    private static final short  MASK_VALUE = 0xff;

    /**
     * ICMP type value to match.
     */
    private final short  type;

    /**
     * ICMP code value to match.
     */
    private final short  code;

    /**
     * Construct a new instance.
     *
     * @param match  An {@link IcmpMatch} instance.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws VTNException
     *    {@code match} contains invalid value.
     */
    public IcmpMatchImpl(IcmpMatch match) throws VTNException {
        type = getValue(match.getType(), "ICMP type.");
        code = getValue(match.getCode(), "ICMP code.");
    }

    /**
     * Return ICMP type value configured in this instance.
     *
     * @return  An ICMP type value.
     *          A negative value is returned if ICMP type is not specified.
     */
    public short getType() {
        return type;
    }

    /**
     * Return ICMP code value configured in this instance.
     *
     * @return  An ICMP code value.
     *          A negative value is returned if ICMP code is not specified.
     */
    public short getCode() {
        return code;
    }

    /**
     * Return a short value in the specified instance.
     *
     * @param s     A {@link Short} instance.
     * @param desc  A brief description about the value.
     * @return   A short integer value in the given instance.
     *           {@link #VALUE_ANY} is returned if {@code null} is specified
     *           to {@code s}.
     * @throws VTNException
     *    An invalid value is configured in {@code s}.
     */
    private short getValue(Short s, String desc) throws VTNException {
        if (s == null) {
            return VALUE_ANY;
        }

        short value = s.shortValue();
        if ((value & ~MASK_VALUE) != 0) {
            throw new VTNException(StatusCode.BADREQUEST,
                                   "Invalid value for " + desc);
        }

        return value;
    }

    /**
     * Return a {@link Short} instance which represents the given value.
     *
     * @param value  A short value.
     * @return  A {@link Short} instance.
     *          {@code null} is returned if a negative value is specified.
     */
    private Short getShort(short value) {
        return (value < 0) ? null : Short.valueOf(value);
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
        if (!(o instanceof IcmpMatchImpl)) {
            return false;
        }

        IcmpMatchImpl match = (IcmpMatchImpl)o;
        return (type == match.type && code == match.code);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return (int)((type << 2) | code);
    }

    // L4PacketMatch

    /**
     * Return an IP protocol number assigned to this protocol.
     *
     * @return  An IP protocol number.
     */
    @Override
    public short getInetProtocol() {
        return IPProtocols.ICMP.shortValue();
    }

    /**
     * Return a {@link L4Match} instance which represents this condition.
     *
     * @return  A {@link L4Match} instance.
     */
    @Override
    public L4Match getMatch() {
        return new IcmpMatch(getShort(type), getShort(code));
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
        CachedPacket packet = pctx.getL4Packet();
        if (!(packet instanceof IcmpPacket)) {
            return false;
        }

        IcmpPacket icmp = (IcmpPacket)packet;
        if (type >= 0) {
            pctx.addMatchField(MatchType.TP_SRC);
            if (type != icmp.getType()) {
                return false;
            }
        }
        if (code >= 0) {
            pctx.addMatchField(MatchType.TP_DST);
            if (code != icmp.getCode()) {
                return false;
            }
        }

        return true;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("IcmpMatchImpl[");
        String sep = "";
        if (type != VALUE_ANY) {
            builder.append("type=").append((int)type);
            sep = ",";
        }
        if (code != VALUE_ANY) {
            builder.append(sep).append("code=").append((int)code);
        }
        builder.append(']');

        return builder.toString();
    }
}
