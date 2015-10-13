/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

/**
 * {@code EtherTypes} contains the common Ethernet types.
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
     * The Ethernet type value.
     */
    private final int  value;

    /**
     * Construct a new instance.
     *
     * @param v  The Ethernet type value.
     */
    private EtherTypes(int v) {
        value = v;
    }

    /**
     * Return the Ethernet type value as an integer value.
     *
     * @return  An integer that represents the Ethernet type value.
     */
    public int intValue() {
        return value;
    }

    /**
     * Return the Ethernet type value as a short integer value.
     *
     * @return  A short integer that represents the Ethernet type value.
     */
    public short shortValue() {
        return (short)value;
    }
}
