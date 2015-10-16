/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.packet;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.Set;

import org.opendaylight.vtn.manager.packet.ICMP;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.it.util.match.FlowMatchType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Icmpv4MatchBuilder;

/**
 * {@code Icmp4Factory} is a utility class used to create or to verify an
 * ICMP version 4 packet.
 */
public final class Icmp4Factory extends PacketFactory {
    /**
     * The ICMP type.
     */
    private byte  icmpType;

    /**
     * The ICMP code.
     */
    private byte  icmpCode;

    /**
     * Construct a new instance.
     *
     * @param i4fc  An {@link Inet4Factory} instance.
     * @return  An {@link Icmp4Factory} instance.
     */
    public static Icmp4Factory newInstance(Inet4Factory i4fc) {
        Icmp4Factory ic4fc = new Icmp4Factory();
        i4fc.setProtocol(InetProtocols.ICMP.byteValue()).setNextFactory(ic4fc);

        return ic4fc;
    }

    /**
     * Construct a new instance.
     *
     * @param i4fc  An {@link Inet4Factory} instance.
     * @param type  The ICMP type.
     * @param code  The ICMP code.
     * @return  An {@link Icmp4Factory} instance.
     */
    public static Icmp4Factory newInstance(Inet4Factory i4fc, byte type,
                                           byte code) {
        Icmp4Factory ic4fc = new Icmp4Factory(type, code);
        i4fc.setProtocol(InetProtocols.ICMP.byteValue()).setNextFactory(ic4fc);

        return ic4fc;
    }

    /**
     * Construct a new instance that indicates an ICMP version 4 packet.
     */
    Icmp4Factory() {}

    /**
     * Construct a new instance that indicates an ICMP version 4 packet.
     *
     * @param type  The ICMP type.
     * @param code  The ICMP code.
     */
    Icmp4Factory(byte type, byte code) {
        icmpType = type;
        icmpCode = code;
    }

    /**
     * Return the ICMP type.
     *
     * @return  An ICMP type value.
     */
    public byte getType() {
        return icmpType;
    }

    /**
     * Return the ICMP code.
     *
     * @return  An ICMP code value.
     */
    public byte getCode() {
        return icmpCode;
    }

    /**
     * Set the ICMP type.
     *
     * @param type  An ICMP type value.
     * @return  This instance.
     */
    public Icmp4Factory setType(byte type) {
        icmpType = type;
        return this;
    }

    /**
     * Set the ICMP code.
     *
     * @param code  An ICMP code value.
     * @return  This instance.
     */
    public Icmp4Factory setCode(byte code) {
        icmpCode = code;
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    Packet createPacket() {
        return new ICMP().setType(icmpType).setCode(icmpCode);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void verify(Packet packet) {
        assertTrue(packet instanceof ICMP);
        ICMP icmp = (ICMP)packet;

        assertEquals(icmpType, icmp.getType());
        assertEquals(icmpCode, icmp.getCode());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    int initMatch(MatchBuilder builder, Set<FlowMatchType> types) {
        Icmpv4MatchBuilder ic4mb = new Icmpv4MatchBuilder();
        int count = 0;

        if (types.contains(FlowMatchType.ICMP4_TYPE)) {
            ic4mb.setIcmpv4Type(toShort(icmpType));
            count++;
        }
        if (types.contains(FlowMatchType.ICMP4_CODE)) {
            ic4mb.setIcmpv4Code(toShort(icmpCode));
            count++;
        }

        if (count > 0) {
            builder.setIcmpv4Match(ic4mb.build());
        }

        return count;
    }

    // Object

    /**
     * {@inheritDoc}
     */
    public Icmp4Factory clone() {
        return (Icmp4Factory)super.clone();
    }
}
