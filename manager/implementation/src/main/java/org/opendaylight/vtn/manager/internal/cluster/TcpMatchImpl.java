/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.CachedPacket;
import org.opendaylight.vtn.manager.internal.packet.TcpPacket;

import org.opendaylight.controller.sal.utils.IPProtocols;

/**
 * {@code TcpMatchImpl} describes the condition to match TCP header fields
 * in IP packet.
 *
 * <p>
 *   Although this interface is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class TcpMatchImpl extends PortProtoMatchImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 4196379454588474512L;

    /**
     * Construct a new instance.
     *
     * @param match  A {@link TcpMatch} instance.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws VTNException
     *    {@code match} contains invalid value.
     */
    public TcpMatchImpl(TcpMatch match) throws VTNException {
        super(match);
    }

    // L4MatchImpl

    /**
     * Return an IP protocol number assigned to this protocol.
     *
     * @return  An IP protocol number.
     */
    @Override
    public short getInetProtocol() {
        return IPProtocols.TCP.shortValue();
    }


    /**
     * Return a {@link L4Match} instance which represents this condition.
     *
     * @return  A {@link L4Match} instance.
     */
    @Override
    public L4Match getMatch() {
        L4PortMatch src = getSourcePort();
        L4PortMatch dst = getDestinationPort();
        return new TcpMatch(getPortMatch(src), getPortMatch(dst));
    }

    // PacketMatch

    /**
     * Determine whether the specified packet matches the condition defined
     * by this instance.
     *
     * @param pctx  The context of the packet to be tested.
     * @return  {@code true} if the specified packet matches the condition.
     *          Otherwise {@code false}.
     */
    @Override
    public boolean match(PacketContext pctx) {
        CachedPacket packet = pctx.getL4Packet();
        if (!(packet instanceof TcpPacket)) {
            return false;
        }

        TcpPacket tcp = (TcpPacket)packet;
        L4PortMatch src = getSourcePort();
        L4PortMatch dst = getDestinationPort();

        if (src != null) {
            int port = tcp.getSourcePort();
            if (!src.match(port)) {
                return false;
            }
        }

        if (dst != null) {
            int port = tcp.getDestinationPort();
            if (!dst.match(port)) {
                return false;
            }
        }

        return true;
    }
}
