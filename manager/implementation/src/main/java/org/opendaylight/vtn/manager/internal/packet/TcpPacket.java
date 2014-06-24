/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import org.opendaylight.controller.sal.packet.TCP;

/**
 * {@code TcpPacket} class implements a cache for a {@link TCP} instance.
 */
public final class TcpPacket extends PortProtoPacket {
    /**
     * A {@link TCP} packet.
     */
    private final TCP  packet;

    /**
     * Construct a new instance.
     *
     * @param tcp  A {@link TCP} instance.
     */
    public TcpPacket(TCP tcp) {
        packet = tcp;
    }

    /**
     * Derive the source port number from the packet.
     *
     * @return  A short integer value which represents the source port number.
     */
    @Override
    public short getRawSourcePort() {
        return packet.getSourcePort();
    }

    /**
     * Derive the destination port number from the packet.
     *
     * @return  A short integer value which represents the destination port
     *          number.
     */
    @Override
    public short getRawDestinationPort() {
        return packet.getDestinationPort();
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
