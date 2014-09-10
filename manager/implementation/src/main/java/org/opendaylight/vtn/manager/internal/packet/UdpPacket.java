/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.MiscUtils;

import org.opendaylight.controller.sal.packet.UDP;

/**
 * {@code UdpPacket} class implements a cache for a {@link UDP} instance.
 */
public final class UdpPacket extends PortProtoPacket<UDP> {
    /**
     * Byte offset to the checksum field in UDP header.
     */
    private static final int  UDP_OFF_CHECKSUM = 6;

    /**
     * Checksum value which indicates the checksum is disabled.
     */
    private static final short  UDP_CKSUM_DISABLED = 0;

    /**
     * A {@link UDP} packet.
     */
    private UDP  packet;

    /**
     * Construct a new instance.
     *
     * @param udp  A {@link UDP} instance.
     */
    public UdpPacket(UDP udp) {
        packet = udp;
    }

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
     * @param pkt   A {@link UDP} instance.
     * @param port  A short integer value which indicates the source port.
     */
    @Override
    protected void setRawSourcePort(UDP pkt, short port) {
        pkt.setSourcePort(port);
    }

    /**
     * Set the destination port number to the given packet.
     *
     * @param pkt   A {@link UDP} instance.
     * @param port  A short integer value which indicates the destination port.
     */
    @Override
    protected void setRawDestinationPort(UDP pkt, short port) {
        pkt.setDestinationPort(port);
    }

    /**
     * Return a {@link UDP} instance to set modified values.
     *
     * @param doCopy {@code true} is passed if the packet configured in this
     *               instance needs to be copied.
     * @return  A {@link UDP} instance.
     * @throws VTNException
     *    Failed to copy the packet.
     */
    @Override
    protected UDP getPacketForWrite(boolean doCopy) throws VTNException {
        UDP pkt;
        if (doCopy) {
            pkt = MiscUtils.copy(packet, new UDP());
            packet = pkt;
        } else {
            pkt = packet;
        }

        return pkt;
    }

    // L4Packet

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean updateChecksum(Inet4Packet ipv4) throws VTNException {
        // Update checksum only if the UDP packet contains a valid checksum.
        boolean mod = false;
        short sum = packet.getChecksum();
        if (sum != 0) {
            UDP pkt = getPacketForWrite();
            short newSum = computeChecksum(ipv4, pkt, UDP_OFF_CHECKSUM);
            if (newSum == UDP_CKSUM_DISABLED) {
                // Set all bits in the checksum field instead of zero.
                newSum = (short)~newSum;
            }
            if (sum != newSum) {
                pkt.setChecksum(newSum);
                mod = true;
            }
        }

        return mod;
    }

    // CachedPacket

    /**
     * Return a {@link UDP} instance configured in this instance.
     *
     * @return  A {@link UDP} instance.
     */
    @Override
    public UDP getPacket() {
        return packet;
    }
}
