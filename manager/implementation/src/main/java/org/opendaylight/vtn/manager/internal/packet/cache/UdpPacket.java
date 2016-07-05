/*
 * Copyright (c) 2014, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.UDP;

import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNPortRange;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNUdpMatch;
import org.opendaylight.vtn.manager.internal.util.packet.UdpHeader;

/**
 * {@code UdpPacket} class implements a cache for a {@link UDP} instance.
 */
public final class UdpPacket extends PortProtoPacket<UDP>
    implements UdpHeader {
    /**
     * Byte offset to the checksum field in UDP header.
     */
    private static final int  UDP_OFF_CHECKSUM = 6;

    /**
     * Checksum value which indicates the checksum is disabled.
     */
    private static final short  UDP_CKSUM_DISABLED = 0;

    /**
     * Construct a new instance.
     *
     * @param udp  A {@link UDP} instance.
     */
    public UdpPacket(UDP udp) {
        super(udp);
    }

    /**
     * Construct UDP flow match fields.
     *
     * @param src  A {@link VTNPortRange} instance which specifies the
     *             source port.
     * @param dst  A {@link VTNPortRange} instance which specifies the
     *             destination port.
     * @return  A {@link VTNUdpMatch} instance.
     */
    @Override
    protected VTNUdpMatch createMatch(VTNPortRange src, VTNPortRange dst) {
        return new VTNUdpMatch(src, dst);
    }

    /**
     * Return a flow match type corresponding to the source port.
     *
     * @return  {@link FlowMatchType#UDP_SRC}.
     */
    @Override
    public FlowMatchType getSourceMatchType() {
        return FlowMatchType.UDP_SRC;
    }

    /**
     * Return a flow match type corresponding to the destination port.
     *
     * @return  {@link FlowMatchType#UDP_DST}.
     */
    @Override
    public FlowMatchType getDestinationMatchType() {
        return FlowMatchType.UDP_DST;
    }

    // L4Packet

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean updateChecksum(Inet4Packet ipv4) throws VTNException {
        // Update checksum only if the UDP packet contains a valid checksum.
        boolean mod = false;
        short sum = getPacket().getChecksum();
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
}
