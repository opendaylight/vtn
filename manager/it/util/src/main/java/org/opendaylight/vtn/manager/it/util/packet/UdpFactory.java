/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.packet;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.Set;

import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.packet.UDP;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.it.util.flow.match.FlowMatchType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._4.match.UdpMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code UdpFactory} is a utility class used to create or to verify an
 * UDP packet.
 */
public final class UdpFactory extends PacketFactory {
    /**
     * The size of UDP header.
     */
    private static final short  UDP_HEADER_SIZE = (short)8;

    /**
     * Default UDP checksum.
     */
    private static final short  DEFAULT_CHECKSUM = (short)0;

    /**
     * The source port number.
     */
    private int  sourcePort;

    /**
     * The destination port number.
     */
    private int  destinationPort;

    /**
     * Construct a new instance.
     *
     * @param i4fc  An {@link Inet4Factory} instance.
     * @return  An {@link UdpFactory} instance.
     */
    public static UdpFactory newInstance(Inet4Factory i4fc) {
        UdpFactory ufc = new UdpFactory();
        i4fc.setProtocol(InetProtocols.UDP.shortValue()).setNextFactory(ufc);

        return ufc;
    }

    /**
     * Construct a new instance.
     *
     * @param i4fc  An {@link Inet4Factory} instance.
     * @param src   The source port number.
     * @param dst   The destination port number.
     * @return  An {@link UdpFactory} instance.
     */
    public static UdpFactory newInstance(Inet4Factory i4fc, int src, int dst) {
        UdpFactory ufc = new UdpFactory(src, dst);
        i4fc.setProtocol(InetProtocols.UDP.shortValue()).setNextFactory(ufc);

        return ufc;
    }

    /**
     * Construct a new instance that indicates a UDP packet.
     */
    UdpFactory() {}

    /**
     * Construct a new instance that indicates a UDP packet.
     *
     * @param src  The source port number.
     * @param dst  The destination port number.
     */
    UdpFactory(int src, int dst) {
        sourcePort = src;
        destinationPort = dst;
    }

    /**
     * Return the source port number.
     *
     * @return  The source port number.
     */
    public int getSourcePort() {
        return sourcePort;
    }

    /**
     * Return the destination port number.
     *
     * @return  The destination port number.
     */
    public int getDestinationPort() {
        return destinationPort;
    }

    /**
     * Set the source port number.
     *
     * @param port  The source port number.
     * @return  This instance.
     */
    public UdpFactory setSourcePort(int port) {
        sourcePort = port;
        return this;
    }

    /**
     * Set the destination port number.
     *
     * @param port  The destination port number.
     * @return  This instance.
     */
    public UdpFactory setDestinationPort(int port) {
        destinationPort = port;
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    Packet createPacket() {
        UDP udp = new UDP().setSourcePort((short)sourcePort).
            setDestinationPort((short)destinationPort).
            setChecksum(DEFAULT_CHECKSUM);

        short size = UDP_HEADER_SIZE;
        byte[] payload = getRawPayload();
        if (payload != null) {
            size += (short)payload.length;
        }

        return udp.setLength(size);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void verify(Packet packet) {
        assertTrue(packet instanceof UDP);
        UDP udp = (UDP)packet;

        // Checksum is not supported.
        assertEquals((short)sourcePort, udp.getSourcePort());
        assertEquals((short)destinationPort, udp.getDestinationPort());

        short size = UDP_HEADER_SIZE;
        byte[] payload = udp.getRawPayload();
        if (payload != null) {
            size += (short)payload.length;
        }
        assertEquals(size, udp.getLength());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    int initMatch(MatchBuilder builder, Set<FlowMatchType> types) {
        UdpMatchBuilder umb = new UdpMatchBuilder();
        int count = 0;

        if (types.contains(FlowMatchType.UDP_SRC)) {
            umb.setUdpSourcePort(new PortNumber(sourcePort));
            count++;
        }
        if (types.contains(FlowMatchType.UDP_DST)) {
            umb.setUdpDestinationPort(new PortNumber(destinationPort));
            count++;
        }

        if (count > 0) {
            builder.setLayer4Match(umb.build());
        }

        return count;
    }

    // Object

    /**
     * {@inheritDoc}
     */
    public UdpFactory clone() {
        return (UdpFactory)super.clone();
    }
}
