/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

/**
 * {@code FlowMatchType} is a enum for additional flow match types suppored by
 * the VTN Manager.
 */
public enum FlowMatchType {
    /**
     * Ethernet type.
     */
    ETH_TYPE,

    /**
     * VLAN priority.
     */
    VLAN_PCP,

    /**
     * IP source address.
     */
    IP_SRC,

    /**
     * IP destination address.
     */
    IP_DST,

    /**
     * IP protocol number.
     */
    IP_PROTO,

    /**
     * IP DSCP field.
     */
    IP_DSCP,

    /**
     * TCP source port.
     */
    TCP_SRC,

    /**
     * TCP destination port.
     */
    TCP_DST,

    /**
     * UDP source port.
     */
    UDP_SRC,

    /**
     * UDP destination port.
     */
    UDP_DST,

    /**
     * ICMPv4 type.
     */
    ICMP4_TYPE,

    /**
     * ICMPv4 code.
     */
    ICMP4_CODE;
}
