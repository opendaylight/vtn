/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.opendaylight.vtn.manager.flow.cond.PortMatch;

import org.opendaylight.vtn.manager.internal.util.packet.TcpHeader;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.controller.sal.utils.IPProtocols;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnTcpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnTcpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpDestinationRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpSourceRange;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Layer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._4.match.TcpMatch;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code TcpMatchParams} describes parameters for conditions to match against
 * TCP header.
 */
public final class TcpMatchParams extends Layer4PortMatchParams<TcpMatchParams>
    implements TcpHeader {
    // Layer4MatchParams

    /**
     * {@inheritDoc}
     */
    @Override
    public org.opendaylight.vtn.manager.flow.cond.TcpMatch toL4Match() {
        return new org.opendaylight.vtn.manager.flow.cond.TcpMatch(
            getSourcePortMatch(), getDestinationPortMatch());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnTcpMatch toVtnLayer4Match(boolean comp) {
        PortRangeParams r = getSourcePortParams();
        TcpSourceRange src = (r == null) ? null : r.toTcpSourceRange(comp);
        r = getDestinationPortParams();
        TcpDestinationRange dst = (r == null)
            ? null : r.toTcpDestinationRange(comp);

        return new VtnTcpMatchBuilder().setTcpSourceRange(src).
            setTcpDestinationRange(dst).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNTcpMatch toVTNLayer4Match() throws Exception {
        return new VTNTcpMatch(toL4Match());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public XmlNode toXmlNode() {
        return toXmlNode("vtn-tcp-match");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNTcpMatch verifyValues(VTNLayer4Match l4m) {
        assertTrue(l4m instanceof VTNTcpMatch);
        VTNTcpMatch tmatch = (VTNTcpMatch)l4m;
        PortRangeParams src = getSourcePortParams();
        PortRangeParams dst = getDestinationPortParams();
        VTNPortRange srcRange = tmatch.getSourcePort();
        VTNPortRange dstRange = tmatch.getDestinationPort();

        if (src == null) {
            assertEquals(null, srcRange);
        } else {
            Integer from = src.getPortFrom();
            Integer to = src.getPortTo();
            if (to == null) {
                to = from;
            }
            assertEquals(from, srcRange.getPortFrom());
            assertEquals(to, srcRange.getPortTo());
        }

        if (dst == null) {
            assertEquals(null, dstRange);
        } else {
            Integer from = dst.getPortFrom();
            Integer to = dst.getPortTo();
            if (to == null) {
                to = from;
            }
            assertEquals(from, dstRange.getPortFrom());
            assertEquals(to, dstRange.getPortTo());
        }

        assertEquals(isEmpty(), tmatch.isEmpty());

        return tmatch;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VTNLayer4Match l4m) throws Exception {
        assertTrue(l4m instanceof VTNTcpMatch);
        VTNTcpMatch tmatch = (VTNTcpMatch)l4m;
        PortMatch srcMatch = getSourcePortMatch();
        PortMatch dstMatch = getDestinationPortMatch();
        VTNPortRange src = VTNPortRange.create(srcMatch);
        VTNPortRange dst = VTNPortRange.create(dstMatch);
        assertEquals(src, tmatch.getSourcePort());
        assertEquals(dst, tmatch.getDestinationPort());
        assertEquals(IPProtocols.TCP.shortValue(),
                     tmatch.getInetProtocol(IpVersion.Ipv4));
        assertEquals(IPProtocols.TCP.shortValue(),
                     tmatch.getInetProtocol(IpVersion.Ipv6));
        assertEquals(TcpHeader.class, tmatch.getHeaderType());
        assertEquals(FlowMatchType.TCP_SRC, tmatch.getSourceMatchType());
        assertEquals(FlowMatchType.TCP_DST, tmatch.getDestinationMatchType());

        org.opendaylight.vtn.manager.flow.cond.TcpMatch tm =
            tmatch.toL4Match();
        assertEquals(srcMatch, tm.getSourcePort());
        assertEquals(dstMatch, tm.getDestinationPort());

        VtnLayer4Match vl4 = tmatch.toVtnLayer4Match();
        if (vl4 == null) {
            assertEquals(null, srcMatch);
            assertEquals(null, dstMatch);
            assertEquals(null, VTNLayer4Match.create(vl4));
        } else {
            assertTrue(vl4 instanceof VtnTcpMatch);
            VtnTcpMatch vtm = (VtnTcpMatch)vl4;
            TcpSourceRange vsrc = vtm.getTcpSourceRange();
            if (srcMatch == null) {
                assertEquals(null, vsrc);
            } else {
                assertEquals(srcMatch.getPortFrom(),
                             vsrc.getPortFrom().getValue());
                assertEquals(srcMatch.getPortTo(),
                             vsrc.getPortTo().getValue());
            }
            TcpDestinationRange vdst = vtm.getTcpDestinationRange();
            if (dstMatch == null) {
                assertEquals(null, vdst);
            } else {
                assertEquals(dstMatch.getPortFrom(),
                             vdst.getPortFrom().getValue());
                assertEquals(dstMatch.getPortTo(),
                             vdst.getPortTo().getValue());
            }
            assertEquals(tmatch, VTNLayer4Match.create(vl4));
        }

        MatchBuilder mb = new MatchBuilder();
        tmatch.setMatch(mb, IpVersion.Ipv4);
        boolean hasValue = verify(mb);

        VTNLayer4Match l4 = VTNLayer4Match.create(mb.build());
        if (hasValue) {
            assertTrue(l4 instanceof VTNTcpMatch);
            VTNTcpMatch tmatch1 = (VTNTcpMatch)l4;

            // MD-SAL port match cannot represent the range of port numbers.
            src = tmatch1.getSourcePort();
            if (srcMatch == null) {
                assertEquals(null, src);
            } else {
                Integer from = srcMatch.getPortFrom();
                assertEquals(from, src.getPortFrom());
                assertEquals(from, src.getPortTo());
            }

            dst = tmatch1.getDestinationPort();
            if (dstMatch == null) {
                assertEquals(null, dst);
            } else {
                Integer from = dstMatch.getPortFrom();
                assertEquals(from, dst.getPortFrom());
                assertEquals(from, dst.getPortTo());
            }
        } else {
            assertEquals(null, l4);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean verify(MatchBuilder builder) {
        Layer4Match l4 = builder.getLayer4Match();
        PortRangeParams src = getSourcePortParams();
        PortRangeParams dst = getDestinationPortParams();
        if (l4 == null) {
            assertEquals(null, src);
            assertEquals(null, dst);
            return false;
        }

        assertTrue(l4 instanceof TcpMatch);
        TcpMatch tm = (TcpMatch)l4;
        boolean hasValue = false;
        if (src == null) {
            assertEquals(null, tm.getTcpSourcePort());
        } else {
            assertEquals(src.getPortFrom(), tm.getTcpSourcePort().getValue());
            hasValue = true;
        }

        if (dst == null) {
            assertEquals(null, tm.getTcpDestinationPort());
        } else {
            assertEquals(dst.getPortFrom(),
                         tm.getTcpDestinationPort().getValue());
            hasValue = true;
        }

        assertTrue(hasValue);
        return hasValue;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verifyMd(VTNLayer4Match l4m) throws Exception {
        assertTrue(l4m instanceof VTNTcpMatch);
        VTNTcpMatch tmatch = (VTNTcpMatch)l4m;
        PortRangeParams src = getSourcePortParams();
        PortRangeParams dst = getDestinationPortParams();
        VTNPortRange srcRange = tmatch.getSourcePort();
        VTNPortRange dstRange = tmatch.getDestinationPort();
        if (src == null) {
            assertEquals(null, srcRange);
        } else {
            Integer port = src.getPortFrom();
            assertEquals(port, srcRange.getPortFrom());
            assertEquals(port, srcRange.getPortTo());
        }

        if (dst == null) {
            assertEquals(null, dstRange);
        } else {
            Integer port = dst.getPortFrom();
            assertEquals(port, dstRange.getPortFrom());
            assertEquals(port, dstRange.getPortTo());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Class<TcpMatchParams> getMatchType() {
        return TcpMatchParams.class;
    }

    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public TcpMatchParams clone() {
        return (TcpMatchParams)super.clone();
    }
}
