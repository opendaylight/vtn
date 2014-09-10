/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.net.Inet4Address;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;

import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code MiscUtils} class is a collection of miscellaneous utility class
 * methods.
 */
public final class MiscUtils {
    /**
     * A mask value which represents all bits in a byte value.
     */
    public static final int  MASK_BYTE = (1 << Byte.SIZE) - 1;

    /**
     * A mask value which represents all bits in a short value.
     */
    public static final int  MASK_SHORT = (1 << Short.SIZE) - 1;

    /**
     * Maximum length of the resource name.
     */
    private static final int RESOURCE_NAME_MAXLEN = 31;

    /**
     * Regular expression that matches valid resource name.
     */
    private static final Pattern RESOURCE_NAME_REGEX =
        Pattern.compile("^\\p{Alnum}[\\p{Alnum}_]*$");

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
     * The number of bits to be shifted to get the first octet in an integer.
     */
    private static final int  INT_SHIFT_OCTET1 = 24;

    /**
     * The number of bits to be shifted to get the second octet in an integer.
     */
    private static final int  INT_SHIFT_OCTET2 = 16;

    /**
     * The number of bits to be shifted to get the third octet in an integer.
     */
    private static final int  INT_SHIFT_OCTET3 = 8;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private MiscUtils() {}

    /**
     * Return a hash code of the given long integer value.
     *
     * @param value  A long integer value.
     * @return  A hash code of the given value.
     */
    public static int hashCode(long value) {
        return (int)(value ^ (value >>> Integer.SIZE));
    }

    /**
     * Convert a long value which represents a MAC address into a string.
     *
     * @param mac  A long value which represents a MAC address.
     * @return  A string representation of MAC address.
     */
    public static String formatMacAddress(long mac) {
        byte[] addr = NetUtils.longToByteArray6(mac);
        return HexEncode.bytesToHexStringFormat(addr);
    }

    /**
     * Check the specified resource name.
     *
     * @param desc  Brief description of the resource.
     * @param name  The name of the resource.
     * @throws VTNException  The specified name is invalid.
     */
    public static void checkName(String desc, String name)
        throws VTNException {
        if (name == null) {
            Status status = argumentIsNull(desc + " name");
            throw new VTNException(status);
        }

        if (name.isEmpty()) {
            Status status = new Status(StatusCode.BADREQUEST,
                                       desc + " name cannot be empty");
            throw new VTNException(status);
        }

        int len = name.length();
        if (len > RESOURCE_NAME_MAXLEN) {
            Status status = new Status(StatusCode.BADREQUEST,
                                       desc + " name is too long");
            throw new VTNException(status);
        }

        Matcher m = RESOURCE_NAME_REGEX.matcher(name);
        if (!m.matches()) {
            Status status = new Status(StatusCode.BADREQUEST, desc +
                                       " name contains invalid character");
            throw new VTNException(status);
        }
    }

    /**
     * Check the specified VLAN ID.
     *
     * @param vlan  VLAN ID.
     * @throws VTNException  The specified VLAN ID is invalid.
     */
    public static void checkVlan(short vlan) throws VTNException {
        if (((long)vlan & ~MacVlan.MASK_VLAN_ID) != 0L) {
            String msg = "Invalid VLAN ID: " + vlan;
            throw new VTNException(StatusCode.BADREQUEST, msg);
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

    /**
     * Return a failure status which represents a {@code null} is specified
     * unexpectedly.
     *
     * @param desc  Brief description of the argument.
     * @return  A failure reason.
     */
    public static Status argumentIsNull(String desc) {
        String msg = desc + " cannot be null";
        return new Status(StatusCode.BADREQUEST, msg);
    }

    /**
     * Convert an integer into an IPv4 address.
     *
     * @param address  An integer value which represents an IPv4 address.
     * @return  An {@link InetAddress} instance.
     * @throws IllegalStateException
     *    An error occurred.
     */
    public static InetAddress toInetAddress(int address) {
        byte[] addr = NetUtils.intToByteArray4(address);
        try {
            return InetAddress.getByAddress(addr);
        } catch (Exception e) {
            // This should never happen.
            StringBuilder builder =
                new StringBuilder("Unexpected exception: addr=");
            builder.append(Integer.toHexString(address));
            throw new IllegalStateException(builder.toString(), e);
        }
    }

    /**
     * Convert an IPv4 address into an integer.
     *
     * @param addr  A {@link InetAddress} instance which represents an IPv4
     *              address.
     * @return  An integer value.
     * @throws IllegalStateException
     *    An error occurred.
     */
    public static int toInteger(InetAddress addr) {
        if (addr instanceof Inet4Address) {
            byte[] bytes = addr.getAddress();
            return NetUtils.byteArray4ToInt(bytes);
        }

        StringBuilder builder =
            new StringBuilder("Unexpected InetAddress: addr=");
        builder.append(addr);
        throw new IllegalStateException(builder.toString());
    }

    /**
     * Copy the contents of the given packet.
     *
     * @param src  The source {@link Packet} instance.
     * @param dst  The destination {@link Packet} instance.
     * @param <T>  Type of packet.
     * @return  {@code dst}.
     * @throws VTNException
     *    Failed to copy the packet.
     */
    public static <T extends Packet> T copy(T src, T dst) throws VTNException {
        try {
            byte[] raw = src.serialize();
            int nbits = raw.length * NetUtils.NumBitsInAByte;
            dst.deserialize(raw, 0, nbits);
            return dst;
        } catch (Exception e) {
            // This should never happen.
            throw new VTNException("Failed to copy the packet.", e);
        }
    }

    /**
     * Set an integer value into the given byte array in network byte order.
     *
     * @param array  A byte array.
     * @param off    Index of {@code array} to store value.
     * @param value  An integer value.
     */
    public static void setInt(byte[] array, int off, int value) {
        int index = off;
        array[index++] = (byte)(value >>> INT_SHIFT_OCTET1);
        array[index++] = (byte)(value >>> INT_SHIFT_OCTET2);
        array[index++] = (byte)(value >>> INT_SHIFT_OCTET3);
        array[index] = (byte)value;
    }

    /**
     * Set a short integer value into the given byte array in network byte
     * order.
     *
     * @param array  A byte array.
     * @param off    Index of {@code array} to store value.
     * @param value  A short integer value.
     */
    public static void setShort(byte[] array, int off, short value) {
        array[off] = (byte)(value >>> Byte.SIZE);
        array[off + 1] = (byte)value;
    }
}
