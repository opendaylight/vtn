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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnTcpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnTcpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpDestinationRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpSourceRange;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNTcpMatch} describes the condition for TCP header to match against
 * packets.
 */
public final class VTNTcpMatch extends Layer4PortMatch {
    /**
     * Construct an empty instance.
     */
    public VTNTcpMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param src  The source port number to match against packet.
     *             {@code null} matches every source port number.
     * @param dst  The destination port number to match against packet.
     *             {@code null} matches every destination port number.
     */
    public VTNTcpMatch(Integer src, Integer dst) {
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
    public VTNTcpMatch(PortRange src, PortRange dst) {
        super(src, dst);
    }

    /**
     * Set the range of the source port number to match against packet.
     *
     * @param range  The range of the source port number.
     *               {@code null} matches every source port number.
     * @return  This instance.
     */
    public VTNTcpMatch setSourcePort(PortRange range) {
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
    public VTNTcpMatch setDestinationPort(PortRange range) {
        setDestination(range);
        return this;
    }

    // VTNLayer4Match

    /**
     * {@inheritDoc}
     */
    @Override
    public short getInetProtocol(IpVersion ver) {
        return InetProtocols.TCP.shortValue();
    }

    /**
     * Return a {@link VtnTcpMatch} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnTcpMatch} instance if this instance contains
     *          the condition.
     */
    @Override
    public VtnTcpMatch toVtnLayer4Match() {
        PortRange range = getSourcePort();
        TcpSourceRange src = (range == null)
            ? null : range.getTcpSourceRange();

        range = getDestinationPort();
        TcpDestinationRange dst = (range == null)
            ? null : range.getTcpDestinationRange();

        return new VtnTcpMatchBuilder().
            setTcpSourceRange(src).
            setTcpDestinationRange(dst).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VtnLayer4Match vl4) {
        assertEquals(true, vl4 instanceof VtnTcpMatch);
        VtnTcpMatch vtcp = (VtnTcpMatch)vl4;

        PortRange range = getSourcePort();
        TcpSourceRange src = vtcp.getTcpSourceRange();
        if (range == null) {
            assertEquals(null, src);
        } else {
            range.verify(src);
        }

        range = getDestinationPort();
        TcpDestinationRange dst = vtcp.getTcpDestinationRange();
        if (range == null) {
            assertEquals(null, dst);
        } else {
            range.verify(dst);
        }
    }
}
