/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * {@code ProtocolUtils} class is a collection of utilities for network
 * protocol handling.
 */
public final class ProtocolUtils {
    /**
     * Mask value which represents  valid bits in an ethernet type.
     */
    public static final int  MASK_ETHER_TYPE = 0xffff;

    /**
     * The number of bits in a valid VLAN ID.
     */
    public static final int  NBITS_VLAN_ID = 12;

    /**
     * Mask value which represents a VLAN ID bits in a long integer.
     */
    public static final long MASK_VLAN_ID = (1L << NBITS_VLAN_ID) - 1L;

    /**
     * A mask value which represents valid bits in an VLAN priority.
     */
    private static final byte  MASK_VLAN_PRI = 0x7;

    /**
     * A mask value which represents valid bits in an DSCP value for
     * IP protocol.
     */
    private static final byte  MASK_IP_DSCP = 0x3f;

    /**
     * A mask value which represents valid bits in ICMP type and code.
     */
    private static final short  MASK_ICMP_VALUE = 0xff;

    /**
     * A mask value which represents valid bits in port number of transport
     * layer protocol.
     */
    private static final int  MASK_TP_PORT = 0xffff;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private ProtocolUtils() {}

    /**
     * Check the given ethernet type.
     *
     * @param type  The ethernet type to be checked.
     * @throws VTNException  The given ethernet type is invalid.
     */
    public static void checkEtherType(int type) throws VTNException {
        if ((type & ~MASK_ETHER_TYPE) != 0) {
            throw invalidEtherType((long)type);
        }
    }

    /**
     * Return a {@link VTNException} that notifies an invalid ethernet type.
     *
     * @param type  The invalid ethernet type.
     * @return  A {@link VTNException}.
     */
    private static VTNException invalidEtherType(long type) {
        String msg = "Invalid Ethernet type: " + type;
        return RpcException.getBadArgumentException(msg);
    }

    /**
     * Check the specified VLAN ID.
     *
     * @param vlan  VLAN ID.
     * @throws VTNException  The specified VLAN ID is invalid.
     */
    public static void checkVlan(short vlan) throws VTNException {
        if (((long)vlan & ~MASK_VLAN_ID) != 0L) {
            String msg = "Invalid VLAN ID: " + vlan;
            throw RpcException.getBadArgumentException(msg);
        }
    }

    /**
     * Determine whether the specified VLAN priority value is valid or not.
     *
     * @param pri  A VLAN priority.
     * @return  {@code true} only if the given VLAN priority is valid.
     */
    public static boolean isVlanPriorityValid(byte pri) {
        return ((pri & ~MASK_VLAN_PRI) == 0);
    }

    /**
     * Determine whether the specified IP DSCP value is valid or not.
     *
     * @param dscp  A DSCP value.
     * @return  {@code true} only if the given DSCP value is valid.
     */
    public static boolean isDscpValid(byte dscp) {
        return ((dscp & ~MASK_IP_DSCP) == 0);
    }

    /**
     * Determine whether the specified ICMP type or code value is valid or not.
     *
     * @param value  An ICMP type or code.
     * @return  {@code true} only if the given value is valid.
     */
    public static boolean isIcmpValueValid(short value) {
        return ((value & ~MASK_ICMP_VALUE) == 0);
    }

    /**
     * Determine whether the specified port number of transport layer protocol
     * is valid or not.
     *
     * @param port  A port number.
     * @return  {@code true} only if the given port number is valid.
     */
    public static boolean isPortNumberValid(int port) {
        return ((port & ~MASK_TP_PORT) == 0);
    }
}
