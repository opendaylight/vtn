/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.opendaylight.vtn.manager.flow.cond.PortMatch;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.internal.util.packet.UdpHeader;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnUdpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnUdpMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpDestinationRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpSourceRange;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Layer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._4.match.UdpMatch;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code UdpMatchParams} describes parameters for conditions to match against
 * UDP header.
 */
public final class UdpMatchParams extends Layer4PortMatchParams<UdpMatchParams>
    implements UdpHeader {
    // Layer4MatchParams

    /**
     * {@inheritDoc}
     */
    @Override
    public org.opendaylight.vtn.manager.flow.cond.UdpMatch toL4Match() {
        return new org.opendaylight.vtn.manager.flow.cond.UdpMatch(
            getSourcePortMatch(), getDestinationPortMatch());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnUdpMatch toVtnLayer4Match(boolean comp) {
        PortRangeParams r = getSourcePortParams();
        UdpSourceRange src = (r == null) ? null : r.toUdpSourceRange(comp);
        r = getDestinationPortParams();
        UdpDestinationRange dst = (r == null)
            ? null : r.toUdpDestinationRange(comp);

        return new VtnUdpMatchBuilder().setUdpSourceRange(src).
            setUdpDestinationRange(dst).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNUdpMatch toVTNLayer4Match() throws Exception {
        return new VTNUdpMatch(toL4Match());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public XmlNode toXmlNode() {
        return toXmlNode("vtn-udp-match");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNUdpMatch verifyValues(VTNLayer4Match l4m) {
        assertTrue(l4m instanceof VTNUdpMatch);
        VTNUdpMatch umatch = (VTNUdpMatch)l4m;
        PortRangeParams src = getSourcePortParams();
        PortRangeParams dst = getDestinationPortParams();
        VTNPortRange srcRange = umatch.getSourcePort();
        VTNPortRange dstRange = umatch.getDestinationPort();

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

        assertEquals(isEmpty(), umatch.isEmpty());

        return umatch;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VTNLayer4Match l4m) throws Exception {
        assertTrue(l4m instanceof VTNUdpMatch);
        VTNUdpMatch umatch = (VTNUdpMatch)l4m;
        PortMatch srcMatch = getSourcePortMatch();
        PortMatch dstMatch = getDestinationPortMatch();
        VTNPortRange src = VTNPortRange.create(srcMatch);
        VTNPortRange dst = VTNPortRange.create(dstMatch);
        assertEquals(src, umatch.getSourcePort());
        assertEquals(dst, umatch.getDestinationPort());
        assertEquals(InetProtocols.UDP.shortValue(),
                     umatch.getInetProtocol(IpVersion.Ipv4));
        assertEquals(InetProtocols.UDP.shortValue(),
                     umatch.getInetProtocol(IpVersion.Ipv6));
        assertEquals(UdpHeader.class, umatch.getHeaderType());
        assertEquals(FlowMatchType.UDP_SRC, umatch.getSourceMatchType());
        assertEquals(FlowMatchType.UDP_DST, umatch.getDestinationMatchType());

        org.opendaylight.vtn.manager.flow.cond.UdpMatch um =
            umatch.toL4Match();
        assertEquals(srcMatch, um.getSourcePort());
        assertEquals(dstMatch, um.getDestinationPort());

        VtnLayer4Match vl4 = umatch.toVtnLayer4Match();
        if (vl4 == null) {
            assertEquals(null, srcMatch);
            assertEquals(null, dstMatch);
            assertEquals(null, VTNLayer4Match.create(vl4));
        } else {
            assertTrue(vl4 instanceof VtnUdpMatch);
            VtnUdpMatch vum = (VtnUdpMatch)vl4;
            UdpSourceRange vsrc = vum.getUdpSourceRange();
            if (srcMatch == null) {
                assertEquals(null, vsrc);
            } else {
                assertEquals(srcMatch.getPortFrom(),
                             vsrc.getPortFrom().getValue());
                assertEquals(srcMatch.getPortTo(),
                             vsrc.getPortTo().getValue());
            }
            UdpDestinationRange vdst = vum.getUdpDestinationRange();
            if (dstMatch == null) {
                assertEquals(null, vdst);
            } else {
                assertEquals(dstMatch.getPortFrom(),
                             vdst.getPortFrom().getValue());
                assertEquals(dstMatch.getPortTo(),
                             vdst.getPortTo().getValue());
            }
            assertEquals(umatch, VTNLayer4Match.create(vl4));
        }

        MatchBuilder mb = new MatchBuilder();
        umatch.setMatch(mb, IpVersion.Ipv4);
        boolean hasValue = verify(mb);

        VTNLayer4Match l4 = VTNLayer4Match.create(mb.build());
        if (hasValue) {
            assertTrue(l4 instanceof VTNUdpMatch);
            VTNUdpMatch umatch1 = (VTNUdpMatch)l4;

            // MD-SAL port match cannot represent the range of port numbers.
            src = umatch1.getSourcePort();
            if (srcMatch == null) {
                assertEquals(null, src);
            } else {
                Integer from = srcMatch.getPortFrom();
                assertEquals(from, src.getPortFrom());
                assertEquals(from, src.getPortTo());
            }

            dst = umatch1.getDestinationPort();
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

        assertTrue(l4 instanceof UdpMatch);
        UdpMatch um = (UdpMatch)l4;
        boolean hasValue = false;
        if (src == null) {
            assertEquals(null, um.getUdpSourcePort());
        } else {
            assertEquals(src.getPortFrom(), um.getUdpSourcePort().getValue());
            hasValue = true;
        }

        if (dst == null) {
            assertEquals(null, um.getUdpDestinationPort());
        } else {
            assertEquals(dst.getPortFrom(),
                         um.getUdpDestinationPort().getValue());
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
        assertTrue(l4m instanceof VTNUdpMatch);
        VTNUdpMatch umatch = (VTNUdpMatch)l4m;
        PortRangeParams src = getSourcePortParams();
        PortRangeParams dst = getDestinationPortParams();
        VTNPortRange srcRange = umatch.getSourcePort();
        VTNPortRange dstRange = umatch.getDestinationPort();
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
    protected Class<UdpMatchParams> getMatchType() {
        return UdpMatchParams.class;
    }

    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public UdpMatchParams clone() {
        return (UdpMatchParams)super.clone();
    }
}
