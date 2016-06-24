/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
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
import org.opendaylight.vtn.manager.packet.TCP;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.it.util.flow.match.FlowMatchType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._4.match.TcpMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev130715.PortNumber;

/**
 * {@code TcpFactory} is a utility class used to create or to verify a
 * TCP packet.
 */
public final class TcpFactory extends PacketFactory {
    /**
     * Default TCP sequence number.
     */
    private static final int  DEFAULT_SEQ = 0xdeadbeef;

    /**
     * Defautl ACK number.
     */
    private static final int  DEFAULT_ACK = 0xbaddcafe;

    /**
     * Default data offset.
     */
    private static final byte  DEFAULT_OFF = (byte)5;

    /**
     * Default TCP flags.
     */
    private static final short  DEFAULT_FLAGS = (short)0x18;

    /**
     * Default TCP window size.
     */
    private static final short  DEFAULT_WIN = (short)1264;

    /**
     * Default checksum.
     */
    private static final short  DEFAULT_CHECKSUM = (short)0;

    /**
     * Default urgent pointer.
     */
    private static final short  DEFAULT_URGENT = (short)0;

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
     * @return  An {@link TcpFactory} instance.
     */
    public static TcpFactory newInstance(Inet4Factory i4fc) {
        TcpFactory tfc = new TcpFactory();
        i4fc.setProtocol(InetProtocols.TCP.shortValue()).setNextFactory(tfc);

        return tfc;
    }

    /**
     * Construct a new instance.
     *
     * @param i4fc  An {@link Inet4Factory} instance.
     * @param src   The source port number.
     * @param dst   The destination port number.
     * @return  An {@link TcpFactory} instance.
     */
    public static TcpFactory newInstance(Inet4Factory i4fc, int src, int dst) {
        TcpFactory tfc = new TcpFactory(src, dst);
        i4fc.setProtocol(InetProtocols.TCP.shortValue()).setNextFactory(tfc);

        return tfc;
    }

    /**
     * Construct a new instance that indicates a TCP packet.
     */
    TcpFactory() {}

    /**
     * Construct a new instance that indicates a TCP packet.
     *
     * @param src  The source port number.
     * @param dst  The destination port number.
     */
    TcpFactory(int src, int dst) {
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
    public TcpFactory setSourcePort(int port) {
        sourcePort = port;
        return this;
    }

    /**
     * Set the destination port number.
     *
     * @param port  The destination port number.
     * @return  This instance.
     */
    public TcpFactory setDestinationPort(int port) {
        destinationPort = port;
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    Packet createPacket() {
        TCP tcp = new TCP().setSourcePort((short)sourcePort).
            setDestinationPort((short)destinationPort).
            setSequenceNumber(DEFAULT_SEQ).
            setAckNumber(DEFAULT_ACK).
            setDataOffset(DEFAULT_OFF).
            setHeaderLenFlags(DEFAULT_FLAGS).
            setWindowSize(DEFAULT_WIN).
            setChecksum(DEFAULT_CHECKSUM).
            setUrgentPointer(DEFAULT_URGENT);

        return tcp;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void verify(Packet packet) {
        assertTrue(packet instanceof TCP);
        TCP tcp = (TCP)packet;

        // Checksum is not supported.
        assertEquals((short)sourcePort, tcp.getSourcePort());
        assertEquals((short)destinationPort, tcp.getDestinationPort());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    int initMatch(MatchBuilder builder, Set<FlowMatchType> types) {
        TcpMatchBuilder tmb = new TcpMatchBuilder();
        int count = 0;

        if (types.contains(FlowMatchType.TCP_SRC)) {
            tmb.setTcpSourcePort(new PortNumber(sourcePort));
            count++;
        }
        if (types.contains(FlowMatchType.TCP_DST)) {
            tmb.setTcpDestinationPort(new PortNumber(destinationPort));
            count++;
        }

        if (count > 0) {
            builder.setLayer4Match(tmb.build());
        }

        return count;
    }

    // Object

    /**
     * {@inheritDoc}
     */
    public TcpFactory clone() {
        return (TcpFactory)super.clone();
    }
}
