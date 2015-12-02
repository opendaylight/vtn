/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnIcmpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnTcpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnUdpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Icmpv4MatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.TcpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.UdpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Layer4Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNLayer4Match} describes the condition for layer 4 protocol header
 * to match against packets.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-layer4-match")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({VTNTcpMatch.class, VTNUdpMatch.class, VTNIcmpMatch.class})
public abstract class VTNLayer4Match {
    /**
     * Create a new instance from the given {@link VtnLayer4Match} instance.
     *
     * @param vl4  A {@link VtnLayer4Match} instance.
     * @return  A {@link VTNLayer4Match} instance created from the given
     *          {@link VtnLayer4Match} instance. Note that {@code null} is
     *          returned if {@code vl4} is {@code null}.
     * @throws RpcException
     *    The given {@link VtnLayer4Match} instance is invalid.
     */
    public static final VTNLayer4Match create(VtnLayer4Match vl4)
        throws RpcException {
        if (vl4 == null) {
            return null;
        }
        if (vl4 instanceof VtnTcpMatchFields) {
            return new VTNTcpMatch((VtnTcpMatchFields)vl4);
        }
        if (vl4 instanceof VtnUdpMatchFields) {
            return new VTNUdpMatch((VtnUdpMatchFields)vl4);
        }
        if (vl4 instanceof VtnIcmpMatchFields) {
            return new VTNIcmpMatch((VtnIcmpMatchFields)vl4);
        }

        // This should never hanppen.
        String msg = "Unexpected VTN L4 match instance: " + vl4;
        throw RpcException.getBadArgumentException(msg);
    }

    /**
     * Create a new instance from the given MD-SAL flow match.
     *
     * @param match  A MD-SAL flow match.
     * @return  A {@link VTNLayer4Match} instance or {@code null}.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws RpcException
     *    {@code match} contains invalid condition against layer 4 protocol
     *    header.
     */
    public static final VTNLayer4Match create(Match match)
        throws RpcException {
        Icmpv4MatchFields icmpv4 = match.getIcmpv4Match();
        if (icmpv4 != null) {
            return new VTNIcmpMatch(icmpv4);
        }

        Layer4Match l4 = match.getLayer4Match();
        if (l4 == null) {
            return null;
        }
        if (l4 instanceof TcpMatchFields) {
            return new VTNTcpMatch((TcpMatchFields)l4);
        }
        if (l4 instanceof UdpMatchFields) {
            return new VTNUdpMatch((UdpMatchFields)l4);
        }

        // This should never hanppen.
        String msg = "Unsupported MD-SAL L4 match instance: " + l4;
        throw RpcException.getBadArgumentException(msg);
    }

    /**
     * Construct a new instance that matches every layer 4 packet.
     */
    VTNLayer4Match() {
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    public abstract void verify() throws RpcException;

    /**
     * Return an IP protocol number assigned to this protocol.
     *
     * @param ver  An {@link IpVersion} instance which describes the
     *             IP version.
     * @return  An IP protocol number.
     * @throws IllegalStateException
     *    This layer 4 protocol is unavailable on the given IP version.
     */
    public abstract short getInetProtocol(IpVersion ver);

    /**
     * Return a {@link VtnLayer4Match} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnLayer4Match} instance if this instance contains
     *          the condition. {@code null} if this instance does not contain
     *          any condition.
     */
    public abstract VtnLayer4Match toVtnLayer4Match();

    /**
     * Configure the condition represented by this instance into the given
     * MD-SAL flow match builder.
     *
     * @param builder  A {@link MatchBuilder} instance.
     * @param ver      An {@link IpVersion} instance which describes the
     *                 IP version.
     */
    public abstract void setMatch(MatchBuilder builder, IpVersion ver);

    /**
     * Determine whether the given layer 4 protocol header matches the
     * condition configured in this instance.
     *
     * @param ctx   A {@link FlowMatchContext} instance.
     * @return  {@code true} only if the given header matches all the
     *          conditions configured in this instance.
     */
    public abstract boolean match(FlowMatchContext ctx);

    /**
     * Store strings used to construct flow condition key.
     *
     * @param builder  A {@link StringBuilder} instance which contains strings
     *                 used to construct flow condition key.
     */
    public abstract void setConditionKey(StringBuilder builder);

    /**
     * Determine whether this instance does not specify any header field
     * or not.
     *
     * @return  {@code true} only if this instance is empty.
     */
    public abstract boolean isEmpty();
}
