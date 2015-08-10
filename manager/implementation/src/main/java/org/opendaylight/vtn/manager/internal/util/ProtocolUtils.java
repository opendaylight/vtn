/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetType;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Dscp;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.EtherType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * {@code ProtocolUtils} class is a collection of utilities for network
 * protocol handling.
 */
public final class ProtocolUtils {
    /**
     * The number of bits in a valid VLAN ID.
     */
    public static final int  NBITS_VLAN_ID = 12;

    /**
     * The number of bits in IP ECN field.
     */
    private static final int  NBITS_IP_ECN = 2;

    /**
     * Mask value which represents  valid bits in an ethernet type.
     */
    public static final long  MASK_ETHER_TYPE = 0xffffL;

    /**
     * Mask value which represents a VLAN ID bits in a long integer.
     */
    public static final long MASK_VLAN_ID = (1L << NBITS_VLAN_ID) - 1L;

    /**
     * A mask value which represents valid bits in an VLAN priority.
     */
    private static final short  MASK_VLAN_PRI = 0x7;

    /**
     * A mask value which represents valid bits in an IP protocol number.
     */
    private static final short  MASK_IP_PROTO = 0xff;

    /**
     * A mask value which represents valid bits in an DSCP value for
     * IP protocol.
     */
    private static final short  MASK_IP_DSCP = 0x3f;

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
     * @throws RpcException  The given ethernet type is invalid.
     */
    public static void checkEtherType(int type) throws RpcException {
        if (((long)type & ~MASK_ETHER_TYPE) != 0) {
            throw invalidEtherType((long)type);
        }
    }

    /**
     * Return the ethernet type in the given {@link EtherType} instance.
     *
     * @param etype  An {@link EtherType} instance.
     * @return  An {@link Integer} instance if the ethernet type is present
     *          in the given {@link EtherType} instance.
     *          Otherwise {@code null}.
     * @throws RpcException  The given ethernet type is invalid.
     */
    public static Integer getEtherType(EtherType etype) throws RpcException {
        if (etype != null) {
            Long value = etype.getValue();
            if (value != null) {
                long type = value.longValue();
                if ((type & ~MASK_ETHER_TYPE) != 0) {
                    throw invalidEtherType(type);
                }

                return Integer.valueOf((int)type);
            }
        }

        return null;
    }

    /**
     * Return the ethernet type in the given {@link EthernetType} instance.
     *
     * @param etype  An {@link EthernetType} instance.
     * @return  An {@link Integer} instance if the ethernet type is present
     *          in the given {@link EthernetType} instance.
     *          Otherwise {@code null}.
     * @throws RpcException  The given ethernet type is invalid.
     */
    public static Integer getEtherType(EthernetType etype)
        throws RpcException {
        return (etype == null) ? null : getEtherType(etype.getType());
    }

    /**
     * Return a {@link RpcException} that notifies an invalid ethernet type.
     *
     * @param type  The invalid ethernet type.
     * @return  A {@link RpcException}.
     */
    private static RpcException invalidEtherType(long type) {
        String msg = "Invalid Ethernet type: " + type;
        return RpcException.getBadArgumentException(msg);
    }

    /**
     * Check the specified VLAN ID.
     *
     * @param vlan  VLAN ID.
     * @throws RpcException  The specified VLAN ID is invalid.
     */
    public static void checkVlan(short vlan) throws RpcException {
        if (((long)vlan & ~MASK_VLAN_ID) != 0L) {
            throw invalidVlanId((int)vlan);
        }
    }

    /**
     * Check the specified VLAN ID.
     *
     * @param vlan  VLAN ID.
     * @throws RpcException  The specified VLAN ID is invalid.
     */
    public static void checkVlan(int vlan) throws RpcException {
        if (((long)vlan & ~MASK_VLAN_ID) != 0L) {
            throw invalidVlanId(vlan);
        }
    }

    /**
     * Return the VLAN ID in the given {@link VlanId} instance.
     *
     * @param vid  A {@link VlanId} instance.
     * @return  An {@link Integer} instance if the VLAN ID is present in the
     *          given {@link VlanId} instance.
     *          Otherwise {@code null}.
     * @throws RpcException  The given VLAN ID is invalid.
     */
    public static Integer getVlanId(VlanId vid) throws RpcException {
        Integer value = null;
        if (vid != null) {
            value = vid.getValue();
            if (value != null) {
                // This check can be removed when RESTCONF implements the
                // restriction check.
                long vlan = value.longValue();
                if ((vlan & ~MASK_VLAN_ID) != 0) {
                    throw invalidVlanId(vlan);
                }
            }
        }

        return value;
    }

    /**
     * Return a {@link RpcException} that notifies an invalid VLAN ID.
     *
     * @param vid  The invalid VLAN ID.
     * @return  A {@link RpcException}.
     */
    private static RpcException invalidVlanId(long vid) {
        return RpcException.getBadArgumentException("Invalid VLAN ID: " + vid);
    }

    /**
     * Determine whether the specified VLAN priority value is valid or not.
     *
     * @param pri  A VLAN priority.
     * @return  {@code true} only if the given VLAN priority is valid.
     */
    public static boolean isVlanPriorityValid(short pri) {
        return ((pri & ~MASK_VLAN_PRI) == 0);
    }

    /**
     * Check the specified VLAN priority.
     *
     * @param pri  A VLAN priority.
     * @throws RpcException  The specified VLAN priority is invalid.
     */
    public static void checkVlanPriority(short pri) throws RpcException {
        if ((pri & ~MASK_VLAN_PRI) != 0) {
            throw RpcException.getBadArgumentException(
                "Invalid VLAN priority: " + pri);
        }
    }

    /**
     * Return the VLAN priority in the given {@link VlanPcp} instance.
     *
     * @param pcp  A {@link VlanPcp} instance.
     * @return  An {@link Short} instance if the VLAN priority is present in
     *          the given {@link VlanPcp} instance.
     *          Otherwise {@code null}.
     * @throws RpcException  The given VLAN priority is invalid.
     */
    public static Short getVlanPriority(VlanPcp pcp) throws RpcException {
        Short value = null;
        if (pcp != null) {
            value = pcp.getValue();
            if (value != null) {
                // This check can be removed when RESTCONF implements the
                // restriction check.
                short pri = value.shortValue();
                checkVlanPriority(pri);
            }
        }

        return value;
    }

    /**
     * Check the specified IP protocol number.
     *
     * @param proto  An IP protocol number.
     * @throws RpcException  The specified IP protocol number is invalid.
     */
    public static void checkIpProtocol(short proto) throws RpcException {
        if ((proto & ~MASK_IP_PROTO) != 0) {
            throw invalidIpProtocol(proto);
        }
    }

    /**
     * Return a {@link RpcException} that notifies an invalid IP protocol
     * number.
     *
     * @param proto  The invalid IP protocol number.
     * @return  A {@link RpcException}.
     */
    private static RpcException invalidIpProtocol(short proto) {
        return RpcException.getBadArgumentException(
            "Invalid IP protocol number: " + proto);
    }

    /**
     * Determine whether the specified IP DSCP value is valid or not.
     *
     * @param dscp  A DSCP value.
     * @return  {@code true} only if the given DSCP value is valid.
     */
    public static boolean isDscpValid(short dscp) {
        return ((dscp & ~MASK_IP_DSCP) == 0);
    }

    /**
     * Check the specified IP DSCP field value.
     *
     * @param dscp  An IP DSCP field value.
     * @throws RpcException  The specified IP DSCP value is invalid.
     */
    public static void checkIpDscp(short dscp) throws RpcException {
        if ((dscp & ~MASK_IP_DSCP) != 0) {
            throw invalidIpDscp(dscp);
        }
    }

    /**
     * Return the IP DSCP field value configured in the given {@link Dscp}
     * instance.
     *
     * @param dscp  A {@link Dscp} instance.
     * @return  A {@link Short} instance if the DSCP value is present in the
     *          given {@link Dscp} instance.
     *          Otherwise {@code null}.
     * @throws RpcException  The given DSCP value is invalid.
     */
    public static Short getIpDscp(Dscp dscp) throws RpcException {
        Short value = null;
        if (dscp != null) {
            value = dscp.getValue();
            if (value != null) {
                // This check can be removed when RESTCONF implements the
                // restriction check.
                checkIpDscp(value.shortValue());
            }
        }

        return value;
    }

    /**
     * Return a {@link RpcException} that notifies an invalid IP DSCP value.
     *
     * @param dscp  The invalid IP DSCP value.
     * @return  A {@link RpcException}.
     */
    private static RpcException invalidIpDscp(short dscp) {
        return RpcException.getBadArgumentException(
            "Invalid IP DSCP field value: " + dscp);
    }

    /**
     * Convert the given IP DSCP value into a TOS value.
     *
     * @param dscp  A DSCP value.
     * @return  A TOS value.
     */
    public static int dscpToTos(short dscp) {
        return ((int)(dscp & MASK_IP_DSCP) << NBITS_IP_ECN);
    }

    /**
     * Convert the given IP TOS value into a DSCP value.
     *
     * @param tos  A TOS value.
     * @return  A DSCP value.
     */
    public static short tosToDscp(int tos) {
        return (short)((tos >>> NBITS_IP_ECN) & MASK_IP_DSCP);
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
     * Check the specified ICMP type or code value.
     *
     * @param value  An ICMP type or code.
     * @param desc   A brief description about the given value.
     * @throws RpcException  The specified value is invalid.
     */
    public static void checkIcmpValue(Short value, String desc)
        throws RpcException {
        if (value != null && !isIcmpValueValid(value.shortValue())) {
            StringBuilder builder = new StringBuilder("Invalid ICMP ").
                append(desc).append(": ").append(value);
            throw RpcException.getBadArgumentException(builder.toString());
        }
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

    /**
     * Check the specified port number of IP transport layer protocol.
     *
     * @param port  A port number.
     * @throws RpcException  The specified port number is invalid.
     */
    public static void checkPortNumber(int port) throws RpcException {
        if (!isPortNumberValid(port)) {
            throw invalidPortNumber(port);
        }
    }

    /**
     * Return the port number configured in the given {@link PortNumber}
     * instance.
     *
     * @param port  A {@link PortNumber} instance.
     * @return  An {@link Integer} instance if the port number is present in
     *          the given {@link PortNumber} instance.
     *          Otherwise {@code null}.
     * @throws RpcException  The given port number is invalid.
     */
    public static Integer getPortNumber(PortNumber port) throws RpcException {
        Integer value = null;
        if (port != null) {
            value = port.getValue();
            if (value != null) {
                // This check can be removed when RESTCONF implements the
                // restriction check.
                checkPortNumber(value.intValue());
            }
        }

        return value;
    }

    /**
     * Return a {@link RpcException} that notifies an invalid port number.
     *
     * @param port  The invalid port number.
     * @return  A {@link RpcException}.
     */
    private static RpcException invalidPortNumber(int port) {
        return RpcException.getBadArgumentException(
            "Invalid port number: " + port);
    }
}
