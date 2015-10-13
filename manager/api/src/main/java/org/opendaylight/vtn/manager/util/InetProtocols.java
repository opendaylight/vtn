/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

/**
 * {@code InetProtocols} contains the common IP protocol numbers.
 */
public enum InetProtocols {
    /**
     * Indicates the Internet Control Message Protocol (IPv4).
     */
    ICMP(1),

    /**
     * Indicates the Transmission Control Protocol.
     */
    TCP(6),

    /**
     * Indicates the User Datagram Protocol.
     */
    UDP(17);

    /**
     * The IP protocol number.
     */
    private final int  value;

    /**
     * Construct a new instance.
     *
     * @param v  The IP protocol number.
     */
    private InetProtocols(int v) {
        value = v;
    }

    /**
     * Return the IP protocol number as an integer value.
     *
     * @return  An integer that represents the IP protocol number.
     */
    public int intValue() {
        return value;
    }

    /**
     * Return the IP protocol number as a short integer value.
     *
     * @return  A short integer that represents the IP protocol number.
     */
    public short shortValue() {
        return (short)value;
    }

    /**
     * Return the IP protocol number as a byte value.
     *
     * @return  A byte that represents the IP protocol number.
     */
    public byte byteValue() {
        return (byte)value;
    }
}
