/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.cond.PortMatch;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.internal.util.packet.TcpHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnTcpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnTcpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnTcpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpDestinationRangeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpSourceRangeBuilder;

import  org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._4.match.TcpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.TcpMatchFields;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNTcpMatch} describes the condition for TCP header to match against
 * packets.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-tcp-match")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNTcpMatch extends VTNLayer4PortMatch<TcpHeader> {
    /**
     * Construct a new instance that matches every TCP packet.
     */
    public VTNTcpMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param src  A {@link VTNPortRange} instance which specifies the range
     *             of the source port number. {@code null} matches every
     *             source port number.
     * @param dst  A {@link VTNPortRange} instance which specifies the range
     *             of the destination port number. {@code null} matches every
     *             destination port number.
     */
    public VTNTcpMatch(VTNPortRange src, VTNPortRange dst) {
        super(src, dst);
    }

    /**
     * Construct a new instance from the given {@link TcpMatch} instance.
     *
     * @param tmatch   A {@link TcpMatch} instance.
     * @throws NullPointerException
     *    {@code tmatch} is {@code null}.
     * @throws RpcException
     *    {@code tmatch} contains invalid value.
     */
    public VTNTcpMatch(TcpMatch tmatch) throws RpcException {
        super(tmatch);
    }

    /**
     * Construct a new instance from the given {@link VtnTcpMatchFields}
     * instance.
     *
     * @param vtmatch  A {@link VtnTcpMatchFields} instance.
     * @throws NullPointerException
     *    {@code vtmatch} is {@code null}.
     * @throws RpcException
     *    {@code vtmatch} contains invalid value.
     */
    public VTNTcpMatch(VtnTcpMatchFields vtmatch) throws RpcException {
        super(vtmatch.getTcpSourceRange(), vtmatch.getTcpDestinationRange());
    }

    /**
     * Construct a new instance from the given {@link TcpMatchFields} instance.
     *
     * @param tmatch  A {@link TcpMatchFields} instance.
     * @throws NullPointerException
     *    {@code tmatch} is {@code null}.
     * @throws RpcException
     *    {@code tmatch} contains invalid value.
     */
    public VTNTcpMatch(TcpMatchFields tmatch) throws RpcException {
        super(tmatch.getTcpSourcePort(), tmatch.getTcpDestinationPort());
    }

    /**
     * Create an {@link VtnTcpMatchBuilder} instance.
     *
     * @param vtmatch  An {@link VtnTcpMatchBuilder} instance.
     *                 {@code null} must be specified on the first call.
     * @return  If {@code vtmatch} is {@code null}, this method creates a new
     *          {@link VtnTcpMatchBuilder} instance and returns it.
     *          Otherwise a {@link VtnTcpMatchBuilder} instance passed to
     *          {@code vtmatch} is returned.
     */
    private VtnTcpMatchBuilder create(VtnTcpMatchBuilder vtmatch) {
        return (vtmatch == null) ? new VtnTcpMatchBuilder() : vtmatch;
    }

    /**
     * Create an {@link TcpMatchBuilder} instance.
     *
     * @param tmatch  An {@link TcpMatchBuilder} instance.
     *                {@code null} must be specified on the first call.
     * @return  If {@code tmatch} is {@code null}, this method creates a new
     *          {@link TcpMatchBuilder} instance and returns it.
     *          Otherwise a {@link TcpMatchBuilder} instance passed to
     *          {@code tmatch} is returned.
     */
    private TcpMatchBuilder create(TcpMatchBuilder tmatch) {
        return (tmatch == null) ? new TcpMatchBuilder() : tmatch;
    }

    // VTNLayer4PortMatch

    /**
     * Return an IP protocol number assigned to this protocol.
     *
     * @param ver  An {@link IpVersion} instance which describes the
     *             IP version.
     * @return  6 is always returned.
     */
    public short getInetProtocol(IpVersion ver) {
        return InetProtocols.TCP.shortValue();
    }

    /**
     * Return a {@link TcpMatch} instance which represents this condition.
     *
     * @return  A {@link TcpMatch} instance.
     */
    public TcpMatch toL4Match() {
        PortMatch src = VTNPortRange.toPortMatch(getSourcePort());
        PortMatch dst = VTNPortRange.toPortMatch(getDestinationPort());
        return new TcpMatch(src, dst);
    }

    /**
     * Return a {@link VtnTcpMatch} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnTcpMatch} instance if this instance contains
     *          the condition. {@code null} if this instance does not contain
     *          any condition.
     */
    @Override
    public VtnTcpMatch toVtnLayer4Match() {
        VtnTcpMatchBuilder vtmatch = null;
        VTNPortRange src = getSourcePort();
        if (src != null) {
            TcpSourceRangeBuilder sb = new TcpSourceRangeBuilder().
                setPortFrom(src.getPortNumberFrom()).
                setPortTo(src.getPortNumberTo());
            vtmatch = create(vtmatch).setTcpSourceRange(sb.build());
        }

        VTNPortRange dst = getDestinationPort();
        if (dst != null) {
            TcpDestinationRangeBuilder db = new TcpDestinationRangeBuilder().
                setPortFrom(dst.getPortNumberFrom()).
                setPortTo(dst.getPortNumberTo());
            vtmatch = create(vtmatch).setTcpDestinationRange(db.build());
        }

        return (vtmatch == null) ? null : vtmatch.build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setMatch(MatchBuilder builder, IpVersion ver) {
        TcpMatchBuilder tmatch = null;
        VTNPortRange src = getSourcePort();
        if (src != null) {
            tmatch = create(tmatch).setTcpSourcePort(src.getPortNumberFrom());
        }

        VTNPortRange dst = getDestinationPort();
        if (dst != null) {
            tmatch = create(tmatch).
                setTcpDestinationPort(dst.getPortNumberFrom());
        }

        if (tmatch != null) {
            builder.setLayer4Match(tmatch.build());
        }
    }

    // VTNLayer4PortMatch

    /**
     * Return a class which represents the packet header type.
     *
     * @return  A class for {@link TcpHeader}.
     */
    @Override
    public Class<TcpHeader> getHeaderType() {
        return TcpHeader.class;
    }

    /**
     * Return a flow match type corresponding to the source port.
     *
     * @return  {@link FlowMatchType#TCP_SRC}.
     */
    @Override
    public FlowMatchType getSourceMatchType() {
        return FlowMatchType.TCP_SRC;
    }

    /**
     * Return a flow match type corresponding to the destination port.
     *
     * @return  {@link FlowMatchType#TCP_DST}.
     */
    @Override
    public FlowMatchType getDestinationMatchType() {
        return FlowMatchType.TCP_DST;
    }
}
