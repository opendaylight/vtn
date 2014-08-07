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
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code InetMatchImpl} describes the condition to match IP protocol header
 * fields in packet.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class InetMatchImpl implements PacketMatch {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 3917984759372518420L;

    /**
     * A pseudo IP protocol number which indicates every protocol number
     * shold match.
     */
    protected static final short  PROTO_ANY = -1;

    /**
     * A pseudo DSCP field value which indicates every DSCP value should match.
     */
    protected static final byte  DSCP_ANY = -1;

    /**
     * A mask value which represents valid bits in an IP protocol number.
     */
    private static final short  MASK_PROTO = 0xff;

    /**
     * A mask value which represents valid bits in an DSCP value.
     */
    private static final byte  MASK_DSCP = 0x3f;

    /**
     * An IP protocol type value to match against packets.
     */
    private short  protocol;

    /**
     * A DSCP field value to match against packets.
     */
    private final byte  dscp;

    /**
     * Create a new {@code InetMatchImpl} instance from the given
     * {@link InetMatch} instance.
     *
     * @param match  A {@link InetMatch} instance.
     * @return  A {@link InetMatch} instance constructed from {@code match}.
     * @throws VTNException
     *   An invalid instance is specified to {@code match}.
     */
    public static final InetMatchImpl create(InetMatch match)
        throws VTNException {
        if (match instanceof Inet4Match) {
            return new Inet4MatchImpl((Inet4Match)match);
        }

        // This should never happen.
        throw new VTNException(StatusCode.BADREQUEST,
                               "Unexpected inet match instance: " + match);
    }

    /**
     * Construct a new instance that contains only the condition for the
     * IP protocol number.
     *
     * @param proto  An IP protocol number to match.
     */
    protected InetMatchImpl(short proto) {
        protocol = proto;
        dscp = DSCP_ANY;
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
    protected InetMatchImpl(InetMatch match) throws VTNException {
        Status st = match.getValidationStatus();
        if (st != null) {
            throw new VTNException(st);
        }

        Short proto = match.getProtocol();
        if (proto == null) {
            protocol = PROTO_ANY;
        } else {
            protocol = proto.shortValue();
            if ((protocol & ~MASK_PROTO) != 0) {
                String msg = "Invalid IP protocol number: " + proto;
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
        }

        Byte d = match.getDscp();
        if (d == null) {
            dscp = DSCP_ANY;
        } else {
            dscp = d.byteValue();
            if ((dscp & ~MASK_DSCP) != 0) {
                String msg = "Invalid DSCP field value: " + d;
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
        }
    }

    /**
     * Return the IP protocol type to match against packets.
     *
     * @return  A short integer value which represents the IP protocol type
     *          to match against packets.
     *          A negative value is returned if this instance does not
     *          specify the IP protocol type to match.
     */
    public final short getProtocol() {
        return protocol;
    }

    /**
     * Return the IP protocol type to match against packets.
     *
     * @return  A {@link Short} instance which represents the IP protocol type
     *          to match against packets.
     *          {@code null} is returned if this instance does not
     *          specify the IP protocol type to match.
     */
    public final Short getProtocolShort() {
        return (protocol < 0) ? null : Short.valueOf(protocol);
    }

    /**
     * Return the DSCP field value to match against packets.
     *
     * @return  A byte value which represents the DSCP field value to match
     *          against packets.
     *          A negative value is returned if this instance does not
     *          specify the DSCP field value to match.
     */
    public final byte getDscp() {
        return dscp;
    }

    /**
     * Return the DSCP field value to match against packets.
     *
     * @return  A {@link Byte} instance which represents the DSCP field value
     *          to match against packets.
     *          {@code null} is returned if this instance does not specify
     *          the DSCP field value to match.
     */
    public final Byte getDscpByte() {
        return (dscp < 0) ? null : Byte.valueOf(dscp);
    }

    /**
     * Determine whether the IP protocol number and DSCP field match the
     * condition described by this instance.
     *
     * @param pctx   The context of the packet to be tested.
     * @param proto  An IP protocol number to be tested.
     * @param ds     A DSCP field value to be tested.
     * @return  {@code true} is returned if the specified arguments match the
     *          condition. Otherwise {@code false} is returned.
     */
    public final boolean match(PacketContext pctx, short proto, byte ds) {
        if (protocol >= 0) {
            pctx.addMatchField(MatchType.NW_PROTO);
            if (protocol != proto) {
                return false;
            }
        }

        if (dscp >= 0) {
            pctx.addMatchField(MatchType.NW_TOS);
            if (dscp != ds) {
                return false;
            }
        }

        return true;
    }

    /**
     * Set the IP protocol number to match against packets.
     *
     * @param proto  An IP protocol number.
     * @throws VTNException
     *    The specified protocol number is different from the number configured
     *    in this instance.
     */
    void setProtocol(short proto) throws VTNException {
        if (protocol != PROTO_ANY && protocol != proto) {
            StringBuilder builder =
                new StringBuilder("IP protocol conflict: proto=");
            builder.append((int)protocol).append(", expected=").
                append((int)proto);
            throw new VTNException(StatusCode.BADREQUEST, builder.toString());
        }

        protocol = proto;
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        InetMatchImpl match = (InetMatchImpl)o;
        return (protocol == match.protocol && dscp == match.dscp);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return (int)(((int)protocol << Byte.SIZE) | dscp);
    }

    /**
     * Return an Ethernet protocol type assigned to this protocol.
     *
     * @return  An Ethernet protocol type.
     */
    public abstract int getEtherType();

    /**
     * Return an {@link InetMatch} instance which represents this condition.
     *
     * @return  An {@link InetMatch} instance.
     */
    public abstract InetMatch getMatch();
}
