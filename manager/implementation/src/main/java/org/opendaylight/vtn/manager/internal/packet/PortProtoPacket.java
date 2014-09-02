/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.util.Set;

import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code TcpPacket} class implements a cache for layer 4 protocol header,
 * which identifies the service using 16-bit port number.
 */
public abstract class PortProtoPacket implements CachedPacket {
    /**
     * A pseudo port number which indicates the port number is not specified.
     */
    private static final int  PORT_NONE = -1;

    /**
     * The source port number.
     */
    private int  sourcePort = PORT_NONE;

    /**
     * The destination port number.
     */
    private int  destinationPort = PORT_NONE;

    /**
     * Construct a new instance.
     */
    protected PortProtoPacket() {
    }

    /**
     * Return the source port number.
     *
     * @return  An integer value which represents the source port number.
     */
    public final int getSourcePort() {
        if (sourcePort == PORT_NONE) {
            short port = getRawSourcePort();
            sourcePort = NetUtils.getUnsignedShort(port);
        }

        return sourcePort;
    }

    /**
     * Return the destination port number.
     *
     * @return  An integer value which represents the destination port number.
     */
    public final int getDestinationPort() {
        if (destinationPort == PORT_NONE) {
            short port = getRawDestinationPort();
            destinationPort = NetUtils.getUnsignedShort(port);
        }

        return destinationPort;
    }

    /**
     * Derive the source port number from the packet.
     *
     * @return  A short integer value which represents the source port number.
     */
    public abstract short getRawSourcePort();

    /**
     * Derive the destination port number from the packet.
     *
     * @return  A short integer value which represents the destination port
     *          number.
     */
    public abstract short getRawDestinationPort();

    // CachedPacket

    /**
     * Configure match fields to test TCP/UDP header in this packet.
     *
     * @param match   A {@link Match} instance.
     * @param fields  A set of {@link MatchType} instances corresponding to
     *                match fields to be tested.
     */
    @Override
    public final void setMatch(Match match, Set<MatchType> fields) {
        MatchType type = MatchType.TP_SRC;
        if (fields.contains(type)) {
            // Test source port number.
            match.setField(type, (short)getSourcePort());
        }

        type = MatchType.TP_DST;
        if (fields.contains(type)) {
            // Test destination port number.
            match.setField(type, (short)getDestinationPort());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean commit(PacketContext pctx) {
        // REVISIT: Not yet supported.
        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final PortProtoPacket clone() {
        try {
            return (PortProtoPacket)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
