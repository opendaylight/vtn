/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.TCP;

import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNPortRange;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNTcpMatch;
import org.opendaylight.vtn.manager.internal.util.packet.TcpHeader;

/**
 * {@code TcpPacket} class implements a cache for a {@link TCP} instance.
 */
public final class TcpPacket extends PortProtoPacket<TCP>
    implements TcpHeader {
    /**
     * Byte offset to the checksum field in TCP header.
     */
    private static final int  TCP_OFF_CHECKSUM = 16;

    /**
     * A {@link TCP} packet.
     */
    private TCP  packet;

    /**
     * Construct a new instance.
     *
     * @param tcp  A {@link TCP} instance.
     */
    public TcpPacket(TCP tcp) {
        packet = tcp;
    }

    // PortProtoPacket

    /**
     * Derive the source port number from the packet.
     *
     * @return  A short integer value which represents the source port number.
     */
    @Override
    protected short getRawSourcePort() {
        return packet.getSourcePort();
    }

    /**
     * Derive the destination port number from the packet.
     *
     * @return  A short integer value which represents the destination port
     *          number.
     */
    @Override
    protected short getRawDestinationPort() {
        return packet.getDestinationPort();
    }

    /**
     * Set the source port number to the given packet.
     *
     * @param pkt   A {@link TCP} instance.
     * @param port  A short integer value which indicates the source port.
     */
    @Override
    protected void setRawSourcePort(TCP pkt, short port) {
        pkt.setSourcePort(port);
    }

    /**
     * Set the destination port number to the given packet.
     *
     * @param pkt   A {@link TCP} instance.
     * @param port  A short integer value which indicates the destination port.
     */
    @Override
    protected void setRawDestinationPort(TCP pkt, short port) {
        pkt.setDestinationPort(port);
    }

    /**
     * Return a {@link TCP} instance to set modified values.
     *
     * @param doCopy {@code true} is passed if the packet configured in this
     *               instance needs to be copied.
     * @return  A {@link TCP} instance.
     */
    @Override
    protected TCP getPacketForWrite(boolean doCopy) {
        TCP pkt;
        if (doCopy) {
            pkt = packet.clone();
            packet = pkt;
        } else {
            pkt = packet;
        }

        return pkt;
    }

    /**
     * Return the name of the protocol.
     *
     * @return  {@code "TCP"}.
     */
    @Override
    protected String getProtocolName() {
        return "TCP";
    }

    /**
     * Construct TCP flow match fields.
     *
     * @param src  A {@link VTNPortRange} instance which specifies the
     *             source port.
     * @param dst  A {@link VTNPortRange} instance which specifies the
     *             destination port.
     * @return  A {@link VTNTcpMatch} instance.
     */
    @Override
    protected VTNTcpMatch createMatch(VTNPortRange src, VTNPortRange dst) {
        return new VTNTcpMatch(src, dst);
    }

    /**
     * Return a flow match type corresponding to the source port.
     *
     * @return  {@link FlowMatchType#TCP_SRC}.
     */
    @Override
    public FlowMatchType getSourceMatchType() {
        return FlowMatchType.TCP_SRC;
    }

    /**
     * Return a flow match type corresponding to the destination port.
     *
     * @return  {@link FlowMatchType#TCP_DST}.
     */
    @Override
    public FlowMatchType getDestinationMatchType() {
        return FlowMatchType.TCP_DST;
    }

    // L4Packet

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean updateChecksum(Inet4Packet ipv4) throws VTNException {
        short sum = packet.getChecksum();
        TCP pkt = getPacketForWrite();
        short newSum = computeChecksum(ipv4, pkt, TCP_OFF_CHECKSUM);
        boolean mod;
        if (sum != newSum) {
            pkt.setChecksum(newSum);
            mod = true;
        } else {
            mod = false;
        }

        return mod;
    }

    // CachedPacket

    /**
     * Return a {@link TCP} instance configured in this instance.
     *
     * @return  A {@link TCP} instance.
     */
    @Override
    public TCP getPacket() {
        return packet;
    }
}
