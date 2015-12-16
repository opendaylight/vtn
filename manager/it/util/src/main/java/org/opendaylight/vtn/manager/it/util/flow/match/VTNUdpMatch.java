/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnUdpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnUdpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpDestinationRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpSourceRange;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNUdpMatch} describes the condition for UDP header to match against
 * packets.
 */
public final class VTNUdpMatch extends Layer4PortMatch {
    /**
     * Construct an empty instance.
     */
    public VTNUdpMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param src  The source port number to match against packet.
     *             {@code null} matches every source port number.
     * @param dst  The destination port number to match against packet.
     *             {@code null} matches every destination port number.
     */
    public VTNUdpMatch(Integer src, Integer dst) {
        super(src, dst);
    }

    /**
     * Construct a new instance.
     *
     * @param src  A {@link PortRange} instance which specifies the range
     *             of the source port number. {@code null} matches every
     *             source port number.
     * @param dst  A {@link PortRange} instance which specifies the range
     *             of the destination port number. {@code null} matches every
     *             destination port number.
     */
    public VTNUdpMatch(PortRange src, PortRange dst) {
        super(src, dst);
    }

    /**
     * Set the range of the source port number to match against packet.
     *
     * @param range  The range of the source port number.
     *               {@code null} matches every source port number.
     * @return  This instance.
     */
    public VTNUdpMatch setSourcePort(PortRange range) {
        setSource(range);
        return this;
    }

    /**
     * Set the range of the destination port number to match against packet.
     *
     * @param range  The range of the destination port number.
     *               {@code null} matches every destination port number.
     * @return  This instance.
     */
    public VTNUdpMatch setDestinationPort(PortRange range) {
        setDestination(range);
        return this;
    }

    // VTNLayer4Match

    /**
     * {@inheritDoc}
     */
    @Override
    public short getInetProtocol(IpVersion ver) {
        return InetProtocols.UDP.shortValue();
    }

    /**
     * Return a {@link VtnUdpMatch} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnUdpMatch} instance if this instance contains
     *          the condition.
     */
    @Override
    public VtnUdpMatch toVtnLayer4Match() {
        PortRange range = getSourcePort();
        UdpSourceRange src = (range == null)
            ? null : range.getUdpSourceRange();

        range = getDestinationPort();
        UdpDestinationRange dst = (range == null)
            ? null : range.getUdpDestinationRange();

        return new VtnUdpMatchBuilder().
            setUdpSourceRange(src).
            setUdpDestinationRange(dst).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnLayer4Match vl4) {
        assertEquals(true, vl4 instanceof VtnUdpMatch);
        VtnUdpMatch vudp = (VtnUdpMatch)vl4;

        PortRange range = getSourcePort();
        UdpSourceRange src = vudp.getUdpSourceRange();
        if (range == null) {
            assertEquals(null, src);
        } else {
            range.verify(src);
        }

        range = getDestinationPort();
        UdpDestinationRange dst = vudp.getUdpDestinationRange();
        if (range == null) {
            assertEquals(null, dst);
        } else {
            range.verify(dst);
        }
    }
}
