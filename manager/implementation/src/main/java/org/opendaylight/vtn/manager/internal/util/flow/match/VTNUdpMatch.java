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

import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.internal.util.packet.UdpHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnUdpMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnUdpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnUdpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpDestinationRangeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpSourceRangeBuilder;

import  org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._4.match.UdpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.UdpMatchFields;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNUdpMatch} describes the condition for UDP header to match against
 * packets.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-udp-match")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNUdpMatch extends VTNLayer4PortMatch<UdpHeader> {
    /**
     * Construct a new instance that matches every UDP packet.
     */
    public VTNUdpMatch() {
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
    public VTNUdpMatch(VTNPortRange src, VTNPortRange dst) {
        super(src, dst);
    }

    /**
     * Construct a new instance from the given {@link VtnUdpMatchFields}
     * instance.
     *
     * @param vumatch  A {@link VtnUdpMatchFields} instance.
     * @throws NullPointerException
     *    {@code vumatch} is {@code null}.
     * @throws RpcException
     *    {@code vumatch} contains invalid value.
     */
    public VTNUdpMatch(VtnUdpMatchFields vumatch) throws RpcException {
        super(vumatch.getUdpSourceRange(), vumatch.getUdpDestinationRange());
    }

    /**
     * Construct a new instance from the given {@link UdpMatchFields} instance.
     *
     * @param umatch  A {@link UdpMatchFields} instance.
     * @throws NullPointerException
     *    {@code umatch} is {@code null}.
     * @throws RpcException
     *    {@code umatch} contains invalid value.
     */
    public VTNUdpMatch(UdpMatchFields umatch) throws RpcException {
        super(umatch.getUdpSourcePort(), umatch.getUdpDestinationPort());
    }

    /**
     * Create an {@link VtnUdpMatchBuilder} instance.
     *
     * @param vumatch  An {@link VtnUdpMatchBuilder} instance.
     *                 {@code null} must be specified on the first call.
     * @return  If {@code vumatch} is {@code null}, this method creates a new
     *          {@link VtnUdpMatchBuilder} instance and returns it.
     *          Otherwise a {@link VtnUdpMatchBuilder} instance passed to
     *          {@code vumatch} is returned.
     */
    private VtnUdpMatchBuilder create(VtnUdpMatchBuilder vumatch) {
        return (vumatch == null) ? new VtnUdpMatchBuilder() : vumatch;
    }

    /**
     * Create an {@link UdpMatchBuilder} instance.
     *
     * @param umatch  An {@link UdpMatchBuilder} instance.
     *                {@code null} must be specified on the first call.
     * @return  If {@code umatch} is {@code null}, this method creates a new
     *          {@link UdpMatchBuilder} instance and returns it.
     *          Otherwise a {@link UdpMatchBuilder} instance passed to
     *          {@code umatch} is returned.
     */
    private UdpMatchBuilder create(UdpMatchBuilder umatch) {
        return (umatch == null) ? new UdpMatchBuilder() : umatch;
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
        return InetProtocols.UDP.shortValue();
    }

    /**
     * Return a {@link VtnUdpMatch} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnUdpMatch} instance if this instance contains
     *          the condition. {@code null} if this instance does not contain
     *          any condition.
     */
    @Override
    public VtnUdpMatch toVtnLayer4Match() {
        VtnUdpMatchBuilder vumatch;
        VTNPortRange src = getSourcePort();
        if (src == null) {
            vumatch = null;
        } else {
            UdpSourceRangeBuilder sb = new UdpSourceRangeBuilder().
                setPortFrom(src.getPortNumberFrom()).
                setPortTo(src.getPortNumberTo());
            vumatch = create((VtnUdpMatchBuilder)null).
                setUdpSourceRange(sb.build());
        }

        VTNPortRange dst = getDestinationPort();
        if (dst != null) {
            UdpDestinationRangeBuilder db = new UdpDestinationRangeBuilder().
                setPortFrom(dst.getPortNumberFrom()).
                setPortTo(dst.getPortNumberTo());
            vumatch = create(vumatch).setUdpDestinationRange(db.build());
        }

        return (vumatch == null) ? null : vumatch.build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setMatch(MatchBuilder builder, IpVersion ver) {
        UdpMatchBuilder umatch;
        VTNPortRange src = getSourcePort();
        if (src == null) {
            umatch = null;
        } else {
            umatch = create((UdpMatchBuilder)null).
                setUdpSourcePort(src.getPortNumberFrom());
        }

        VTNPortRange dst = getDestinationPort();
        if (dst != null) {
            umatch = create(umatch).
                setUdpDestinationPort(dst.getPortNumberFrom());
        }

        if (umatch != null) {
            builder.setLayer4Match(umatch.build());
        }
    }

    // VTNLayer4PortMatch

    /**
     * Return a class which represents the packet header type.
     *
     * @return  A class for {@link UdpHeader}.
     */
    @Override
    public Class<UdpHeader> getHeaderType() {
        return UdpHeader.class;
    }

    /**
     * Return a flow match type corresponding to the source port.
     *
     * @return  {@link FlowMatchType#UDP_SRC}.
     */
    @Override
    public FlowMatchType getSourceMatchType() {
        return FlowMatchType.UDP_SRC;
    }

    /**
     * Return a flow match type corresponding to the destination port.
     *
     * @return  {@link FlowMatchType#UDP_DST}.
     */
    @Override
    public FlowMatchType getDestinationMatchType() {
        return FlowMatchType.UDP_DST;
    }
}
