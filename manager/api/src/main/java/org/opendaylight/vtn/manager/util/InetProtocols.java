/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_BYTE;

import java.util.Map;

import com.google.common.collect.ImmutableMap;

/**
 * {@code InetProtocols} contains the common IP protocol numbers.
 *
 * @since  Beryllium
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
     * A map which keeps pairs of IP protocol value and {@link InetProtocols}
     * instances.
     */
    private static final Map<Byte, InetProtocols>  PROTO_MAP;

    /**
     * The IP protocol number.
     */
    private final byte  value;

    /**
     * Initialize static fields.
     */
    static {
        ImmutableMap.Builder<Byte, InetProtocols> builder =
            ImmutableMap.<Byte, InetProtocols>builder();
        for (InetProtocols proto: InetProtocols.values()) {
            builder.put(proto.value, proto);
        }

        PROTO_MAP = builder.build();
    }

    /**
     * Convert the given IP protocol number into a {@link InetProtocols}
     * instance.
     *
     * @param proto  An IP protocol number.
     * @return  An {@link InetProtocols} instance if found.
     *          {@code null} if not found.
     */
    public static InetProtocols forValue(byte proto) {
        return PROTO_MAP.get(proto);
    }

    /**
     * Construct a new instance.
     *
     * @param v  The IP protocol number.
     */
    private InetProtocols(int v) {
        value = (byte)v;
    }

    /**
     * Return the IP protocol number as an integer value.
     *
     * @return  An integer that represents the IP protocol number.
     */
    public int intValue() {
        return (int)(value & MASK_BYTE);
    }

    /**
     * Return the IP protocol number as a short integer value.
     *
     * @return  A short integer that represents the IP protocol number.
     */
    public short shortValue() {
        return (short)(value & MASK_BYTE);
    }

    /**
     * Return the IP protocol number as a byte value.
     *
     * @return  A byte that represents the IP protocol number.
     */
    public byte byteValue() {
        return value;
    }
}
