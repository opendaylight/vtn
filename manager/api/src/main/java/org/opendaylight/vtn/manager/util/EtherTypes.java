/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.util.Map;

import com.google.common.collect.ImmutableMap;

/**
 * {@code EtherTypes} contains the common Ethernet types.
 *
 * @since  Beryllium
 */
public enum EtherTypes {
    /**
     * Indicates the IPv4.
     */
    IPV4(0x800),

    /**
     * Indicates the Address Resolution Protocol.
     */
    ARP(0x806),

    /**
     * Indicates the IEEE 802.1Q VLAN tag.
     */
    VLAN(0x8100),

    /**
     * Indicates the IEEE 802.1ad QinQ VLAN.
     */
    QINQ(0x88a8),

    /**
     * Indicates the Link Layer Discovery Protocol.
     */
    LLDP(0x88cc);

    /**
     * A map which keeps pairs of Ethernet types and {@link EtherTypes}
     * instances.
     */
    private static final Map<Short, EtherTypes>  TYPE_MAP;

    /**
     * The Ethernet type value.
     */
    private final short  value;

    /**
     * Initialize static fields.
     */
    static {
        ImmutableMap.Builder<Short, EtherTypes> builder =
            ImmutableMap.<Short, EtherTypes>builder();
        for (EtherTypes type: EtherTypes.values()) {
            builder.put(type.value, type);
        }

        TYPE_MAP = builder.build();
    }

    /**
     * Convert the given Ethernet type vlaue into a {@link EtherTypes}
     * instance.
     *
     * @param value  An Ethernet type value.
     * @return  An {@link EtherTypes} instance if found.
     *          {@code null} if not found.
     */
    public static EtherTypes forValue(short value) {
        return TYPE_MAP.get(value);
    }

    /**
     * Construct a new instance.
     *
     * @param v  The Ethernet type value.
     */
    EtherTypes(int v) {
        value = (short)v;
    }

    /**
     * Return the Ethernet type value as an integer value.
     *
     * @return  An integer that represents the Ethernet type value.
     */
    public int intValue() {
        return (value & NumberUtils.MASK_SHORT);
    }

    /**
     * Return the Ethernet type value as a short integer value.
     *
     * @return  A short integer that represents the Ethernet type value.
     */
    public short shortValue() {
        return value;
    }
}
