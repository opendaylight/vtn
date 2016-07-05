/*
 * Copyright (c) 2014, 2016 NEC Corporation. All rights reserved.
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
     * Construct a new instance.
     *
     * @param tcp  A {@link TCP} instance.
     */
    public TcpPacket(TCP tcp) {
        super(tcp);
    }

    // PortProtoPacket

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
        short sum = getPacket().getChecksum();
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
}
