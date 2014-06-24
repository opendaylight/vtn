/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

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
}
