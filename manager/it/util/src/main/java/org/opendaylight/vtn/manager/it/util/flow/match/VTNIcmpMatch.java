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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnIcmpMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.vtn.layer4.match.VtnIcmpMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNIcmpMatch} describes the condition for ICMP header to match
 * against packets.
 */
public final class VTNIcmpMatch extends VTNLayer4Match {
    /**
     * The ICMP type to match against packets.
     */
    private Short  icmpType;

    /**
     * The ICMP code to match against packets.
     */
    private Short  icmpCode;

    /**
     * Construct an empty instance.
     */
    public VTNIcmpMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param type  The ICMP type to match.
     *              {@code null} matches every ICMP type.
     * @param code  The ICMP code to match.
     *              {@code null} matches every ICMP code.
     */
    public VTNIcmpMatch(Short type, Short code) {
        icmpType = type;
        icmpCode = code;
    }

    /**
     * Return the ICMP type to match against packets.
     *
     * @return  A {@link Short} instance which indicates the ICMP type to
     *          match. {@code null} if the ICMP type is not specified.
     */
    public Short getIcmpType() {
        return icmpType;
    }

    /**
     * Set the ICMP type to match against packets.
     *
     * @param type  The ICMP type to match against packets.
     * @return  This instance.
     */
    public VTNIcmpMatch setIcmpType(Short type) {
        icmpType = type;
        return this;
    }

    /**
     * Return the ICMP code to match against packets.
     *
     * @return  A {@link Short} instance which indicates the ICMP code to
     *          match. {@code null} if the ICMP code is not specified.
     */
    public Short getIcmpCode() {
        return icmpCode;
    }

    /**
     * Set the ICMP code to match against packets.
     *
     * @param code  The ICMP code to match against packets.
     * @return  This instance.
     */
    public VTNIcmpMatch setIcmpCode(Short code) {
        icmpCode = code;
        return this;
    }

    // VTNLayer4Match

    /**
     * {@inheritDoc}
     */
    @Override
    public short getInetProtocol(IpVersion ver) {
        return InetProtocols.ICMP.shortValue();
    }

    /**
     * Return a {@link VtnIcmpMatch} instance which contains the condition
     * represented by this instance.
     *
     * @return  A {@link VtnIcmpMatch} instance if this instance contains
     *          the condition.
     */
    @Override
    public VtnIcmpMatch toVtnLayer4Match() {
        return new VtnIcmpMatchBuilder().
            setIcmpType(icmpType).
            setIcmpCode(icmpCode).
            build();
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
    public void verify(VtnLayer4Match vl4) {
        assertEquals(true, vl4 instanceof VtnIcmpMatch);
        VtnIcmpMatch vicmp = (VtnIcmpMatch)vl4;

        assertEquals(icmpType, vicmp.getIcmpType());
        assertEquals(icmpCode, vicmp.getIcmpCode());
    }
}
