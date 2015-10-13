/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.ArrayList;

import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.internal.util.packet.IcmpHeader;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnIcmpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnIcmpMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Icmpv4Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code IcmpMatchParams} describes parameters for conditions to match against
 * ICMP header.
 */
public final class IcmpMatchParams extends Layer4MatchParams<IcmpMatchParams>
    implements IcmpHeader {
    /**
     * The ICMP type.
     */
    private Short  icmpType;

    /**
     * The ICMP code.
     */
    private Short  icmpCode;

    /**
     * Test case for {@link VTNIcmpMatch#getInetProtocol(IpVersion)}.
     *
     * @param imatch  A {@link VTNIcmpMatch} instance.
     */
    public static void checkInetProtocol(VTNIcmpMatch imatch) {
        ArrayList<IpVersion> vers = new ArrayList<>();
        vers.add(null);
        for (IpVersion ver: IpVersion.values()) {
            vers.add(ver);
        }
        for (IpVersion ver: vers) {
            if (ver == IpVersion.Ipv4) {
                assertEquals(InetProtocols.ICMP.shortValue(),
                             imatch.getInetProtocol(ver));
            } else {
                try {
                    imatch.getInetProtocol(ver);
                    unexpected();
                } catch (IllegalStateException e) {
                    assertEquals("Unsupported IP version: " + ver,
                                 e.getMessage());
                }
            }
        }
    }

    /**
     * Construct an empty instance.
     */
    public IcmpMatchParams() {
    }

    /**
     * Construct a new instance.
     *
     * @param type  The ICMP type.
     * @param code  The ICMP code.
     */
    public IcmpMatchParams(Short type, Short code) {
        icmpType = type;
        icmpCode = code;
    }

    /**
     * Return the ICMP type.
     *
     * @return  The ICMP type.
     */
    public Short getType() {
        return icmpType;
    }

    /**
     * Set the ICMP type.
     *
     * @param type  The ICMP type.
     * @return  This instance.
     */
    public IcmpMatchParams setType(Short type) {
        icmpType = type;
        return this;
    }

    /**
     * Return the ICMP code.
     *
     * @return  The ICMP code.
     */
    public Short getCode() {
        return icmpCode;
    }

    /**
     * Set the ICMP code.
     *
     * @param code  The ICMP code.
     * @return  This instance.
     */
    public IcmpMatchParams setCode(Short code) {
        icmpCode = code;
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public IcmpMatchParams reset() {
        icmpType = null;
        icmpCode = null;
        return this;
    }

    // IcmpHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public short getIcmpType() {
        return icmpType.shortValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setIcmpType(short type) {
        icmpType = Short.valueOf(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public short getIcmpCode() {
        return icmpCode.shortValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setIcmpCode(short code) {
        icmpCode = Short.valueOf(code);
    }

    // Layer4MatchParams

    /**
     * {@inheritDoc}
     */
    @Override
    public IcmpMatch toL4Match() {
        return new IcmpMatch(icmpType, icmpCode);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnIcmpMatch toVtnLayer4Match(boolean comp) {
        return new VtnIcmpMatchBuilder().
            setIcmpType(icmpType).
            setIcmpCode(icmpCode).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNIcmpMatch toVTNLayer4Match() throws Exception {
        return new VTNIcmpMatch(toL4Match());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public XmlNode toXmlNode() {
        return toXmlNode("vtn-icmp-match");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public XmlNode toXmlNode(String name) {
        XmlNode root = new XmlNode(name);
        if (icmpType != null) {
            root.add(new XmlNode("type", icmpType));
        }
        if (icmpCode != null) {
            root.add(new XmlNode("code", icmpCode));
        }

        return root;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isEmpty() {
        return (icmpType == null && icmpCode == null);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNIcmpMatch verifyValues(VTNLayer4Match l4m) {
        assertTrue(l4m instanceof VTNIcmpMatch);
        VTNIcmpMatch imatch = (VTNIcmpMatch)l4m;
        assertEquals(icmpType, imatch.getIcmpType());
        assertEquals(icmpCode, imatch.getIcmpCode());
        assertEquals(isEmpty(), imatch.isEmpty());
        return imatch;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(VTNLayer4Match l4m) throws Exception {
        VTNIcmpMatch imatch = verifyValues(l4m);
        checkInetProtocol(imatch);

        IcmpMatch im = imatch.toL4Match();
        assertEquals(icmpType, im.getType());
        assertEquals(icmpCode, im.getCode());

        VtnLayer4Match vl4 = imatch.toVtnLayer4Match();
        if (vl4 == null) {
            assertEquals(null, icmpType);
            assertEquals(null, icmpCode);
            assertEquals(null, VTNLayer4Match.create(vl4));
        } else {
            assertTrue(vl4 instanceof VtnIcmpMatch);
            VtnIcmpMatch vim = (VtnIcmpMatch)vl4;
            assertEquals(icmpType, vim.getIcmpType());
            assertEquals(icmpCode, vim.getIcmpCode());
            assertEquals(imatch, VTNLayer4Match.create(vl4));
        }

        MatchBuilder mb = new MatchBuilder();
        imatch.setMatch(mb, IpVersion.Ipv4);
        boolean hasValue = verify(mb);

        VTNLayer4Match l4 = VTNLayer4Match.create(mb.build());
        if (hasValue) {
            assertTrue(l4 instanceof VTNIcmpMatch);
            VTNIcmpMatch imatch1 = (VTNIcmpMatch)l4;
            assertEquals(icmpType, imatch1.getIcmpType());
            assertEquals(icmpCode, imatch1.getIcmpCode());
        } else {
            assertEquals(null, l4);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean verify(MatchBuilder builder) {
        assertEquals(null, builder.getIcmpv6Match());
        Icmpv4Match i4match = builder.getIcmpv4Match();
        if (i4match == null) {
            assertEquals(null, icmpType);
            assertEquals(null, icmpCode);
            return false;
        }

        boolean hasValue = false;
        assertEquals(icmpType, i4match.getIcmpv4Type());
        if (icmpType != null) {
            hasValue = true;
        }

        assertEquals(icmpCode, i4match.getIcmpv4Code());
        if (icmpCode != null) {
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
        assertTrue(l4m instanceof VTNIcmpMatch);
        VTNIcmpMatch imatch = (VTNIcmpMatch)l4m;
        assertEquals(icmpType, imatch.getIcmpType());
        assertEquals(icmpCode, imatch.getIcmpCode());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Class<IcmpMatchParams> getMatchType() {
        return IcmpMatchParams.class;
    }

    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public IcmpMatchParams clone() {
        return (IcmpMatchParams)super.clone();
    }
}
