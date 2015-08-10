/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.Set;

/**
 * {@code FlowMatchType} describes the type of flow match field supported
 * by the VTN Manager.
 */
public enum FlowMatchType {
    /**
     * Indicates the ingress switch port.
     */
    IN_PORT,

    /**
     * Indicates the source MAC address in ethernet header.
     */
    DL_SRC,

    /**
     * Indicates the destination MAC address in ethernet header.
     */
    DL_DST,

    /**
     * Indicates the ethernet type in ethernet header.
     */
    DL_TYPE,

    /**
     * Indicates the VLAN ID in IEEE 802.1Q VLAN tag.
     */
    DL_VLAN,

    /**
     * Indicates the VLAN priority value in IEEE 802.1Q VLAN tag.
     */
    DL_VLAN_PCP,

    /**
     * Indicates the source IP address in IP header.
     */
    IP_SRC,

    /**
     * Indicates the destination IP address in IP header.
     */
    IP_DST,

    /**
     * Indicates the IP protocol number in IP header.
     */
    IP_PROTO,

    /**
     * Indicates the IP DSCP value in IP header.
     */
    IP_DSCP,

    /**
     * Indicates the source port number in TCP header.
     */
    TCP_SRC,

    /**
     * Indicates the destination port number in TCP header.
     */
    TCP_DST,

    /**
     * Indicates the source port number in UDP header.
     */
    UDP_SRC,

    /**
     * Indicates the destination port number in UDP header.
     */
    UDP_DST,

    /**
     * Indicates the ICMP type in ICMP header.
     */
    ICMP_TYPE,

    /**
     * Indicates the ICMP code in ICMP header.
     */
    ICMP_CODE;

    /**
     * Flow match fields to be configured in every unicast flow entry.
     */
    private static final FlowMatchType[]  UNICAST_MATCHES = {
        DL_SRC, DL_DST,
    };

    /**
     * Add match fields mandatory for unicast flow entries to the given set.
     *
     * @param set  A set to store match fields mandatory for unicast flow
     *             entry.
     */
    public static void addUnicastTypes(Set<FlowMatchType> set) {
        for (FlowMatchType type: UNICAST_MATCHES) {
            set.add(type);
        }
    }

    /**
     * Return the number of match types mandatory for unicast flow entries
     * in the given set.
     *
     * @param set  A set of {@link FlowMatchType}.
     * @return  The number of match types mandatory for unicast flow entries.
     */
    public static int getUnicastTypeCount(Set<FlowMatchType> set) {
        int count = 0;
        for (FlowMatchType type: UNICAST_MATCHES) {
            if (set.contains(type)) {
                count++;
            }
        }

        return count;
    }
}
