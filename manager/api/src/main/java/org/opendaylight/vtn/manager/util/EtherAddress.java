/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.io.Serializable;
import java.util.Arrays;
import java.util.Locale;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlValue;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;

import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.MacAddressFilter;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code EtherAddress} describes an ethernet address, aka MAC address.
 *
 * @since  Lithium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "ether-address")
@XmlAccessorType(XmlAccessType.NONE)
public final class EtherAddress implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5338324896227836920L;

    /**
     * The number of octets in an ethernet address.
     */
    public static final int  SIZE = 6;

    /**
     * Multicast bit in the first octet.
     */
    public static final int  BIT_MULTICAST = 0x1;

    /**
     * A mask value that indicates the multicast bit in the first octet.
     */
    public static final long  MASK_MULTICAST =
        ((long)BIT_MULTICAST << ((SIZE - 1) * Byte.SIZE));

    /**
     * A {@link EtherAddress} instance which represents a broadcast address.
     */
    public static final EtherAddress  BROADCAST =
        new EtherAddress(0xffffffffffffL);

    /**
     * A long integer value which represents an ethernet address.
     */
    private long  address;

    /**
     * An byte array which represents an ethernet address.
     */
    private byte[]  byteAddress;

    /**
     * A {@link MacAddress} instance which represents an ethernet address.
     */
    private MacAddress  macAddress;

    /**
     * Convert the given byte array that represents an ethernet address into
     * a long integer number.
     *
     * @param b  A byte array.
     * @return  A long integer number.
     * @throws NullPointerException
     *    {@code b} is {@code null}.
     * @throws IllegalArgumentException
     *    The length of {@code b} is not 6.
     */
    public static long toLong(byte[] b) {
        checkAddress(b);

        long num = 0L;
        int i = 0;
        do {
            num <<= Byte.SIZE;
            num |= b[i] & NumberUtils.MASK_BYTE;
            i++;
        } while (i < SIZE);

        return num;
    }

    /**
     * Convert a string representation of an ethernet address into a long
     * integer number.
     *
     * @param hex  A hex string which represents an ethernet address.
     * @return  A long integer number.
     * @throws NullPointerException
     *    {@code hex} is {@code null}.
     * @throws NumberFormatException
     *    The given string is not a hex string.
     * @throws IllegalArgumentException
     *    The given string is not a string representation of an ethernet
     *    address.
     */
    public static long toLong(String hex) {
        long value = 0L;
        int count = 1;
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < hex.length(); i++) {
            char c = hex.charAt(i);
            if (c == ByteUtils.HEX_SEPARATOR_CHAR) {
                count++;
                int octet = ByteUtils.parseHexOctet(builder.toString());
                value = (value << Byte.SIZE) | (long)octet;
                builder = new StringBuilder();
            } else {
                builder.append(c);
            }
        }

        if (count != SIZE) {
            throw new IllegalArgumentException("Too many octets: " + hex);
        }

        int octet = ByteUtils.parseHexOctet(builder.toString());
        return (value << Byte.SIZE) | (long)octet;
    }

    /**
     * Convert a long number which represents an ethernet address into a byte
     * array.
     *
     * @param addr  A long integer which represents an byte array.
     * @return  A byte array that contains 6 octets.
     */
    public static byte[] toBytes(long addr) {
        long value = addr;
        byte[] mac = new byte[SIZE];
        int i = SIZE - 1;
        do {
            mac[i] = (byte)value;
            value >>>= Byte.SIZE;
            i--;
        } while (i >= 0);

        return mac;
    }

    /**
     * Convert a string representation of an ethernet address into a byte
     * array.
     *
     * @param hex  A hex string which represents an ethernet address.
     * @return  A byte array that contains 6 octets if a valid hex string is
     *          passed. {@code null} is returned if {@code null} is passed.
     * @throws IllegalArgumentException
     *    The given string is not a string representation of an ethernet
     *    address.
     */
    public static byte[] toBytes(String hex) {
        byte[] bytes = ByteUtils.toBytes(hex);
        if (bytes != null) {
            checkAddress(bytes);
        }
        return bytes;
    }

    /**
     * Determine whether the given byte array represents a broadcast address
     * or not.
     *
     * @param bytes  A byte array to be tested.
     * @return  {@code true} only if the given byte array represents a
     *          broadcast address.
     */
    public static boolean isBroadcast(byte[] bytes) {
        return Arrays.equals(bytes, BROADCAST.getBytesImpl());
    }

    /**
     * Determine whether the given byte array represents an unicast address
     * or not.
     *
     * @param bytes  A byte array to be tested.
     * @return  {@code true} only if the given byte array represents an
     *          unicast address.
     */
    public static boolean isUnicast(byte[] bytes) {
        return (bytes.length == SIZE && (bytes[0] & BIT_MULTICAST) == 0);
    }

    /**
     * Create a new {@link EtherAddress} instance.
     *
     * @param bytes  A byte array which represents an ethernet address.
     * @return  An {@link EtherAddress} instance if {@code bytes} is not
     *          {@code null}. {@code null} if {@code bytes} is {@code null}.
     * @throws IllegalArgumentException
     *    The length of {@code bytes} is not 6.
     */
    public static EtherAddress create(byte[] bytes) {
        return (bytes == null) ? null : new EtherAddress(bytes);
    }

    /**
     * Create a new {@link EtherAddress} instance.
     *
     * @param hex  A hex string which represents an ethernet address.
     * @return  An {@link EtherAddress} instance if {@code hex} is not
     *          {@code null}. {@code null} if {@code hex} is {@code null}.
     * @throws IllegalArgumentException
     *    The given string is not a string representation of an ethernet
     *    address.
     */
    public static EtherAddress create(String hex) {
        return (hex == null) ? null : new EtherAddress(hex);
    }

    /**
     * Create a new {@link EtherAddress} instance.
     *
     * @param mac  A {@link MacAddress} instance.
     * @return  An {@link EtherAddress} instance if {@code mac} is not
     *          {@code null}. {@code null} if {@code mac} is {@code null}.
     * @throws IllegalArgumentException
     *    The given instance contains an invalid value.
     */
    public static EtherAddress create(MacAddress mac) {
        return (mac == null) ? null : new EtherAddress(mac);
    }

    /**
     * Create a new {@link EtherAddress} instance.
     *
     * @param mf  A {@link MacAddressFilter} instance.
     *            Note that MAC address mask is always ignored.
     * @return  An {@link EtherAddress} instance if a valid MAC address is
     *          configured in {@code mf}. Otherwise {@code null}.
     * @throws IllegalArgumentException
     *    The given instance contains an invalid value.
     */
    public static EtherAddress create(MacAddressFilter mf) {
        return (mf == null) ? null : create(mf.getAddress());
    }

    /**
     * Create a new {@link EtherAddress} instance.
     *
     * @param eaddr  An {@link EthernetAddress} instance.
     * @return  An {@link EtherAddress} instance if {@code eaddr} is not
     *          {@code null}. {@code null} if {@code eaddr} is {@code null}.
     * @throws IllegalArgumentException
     *    The given instance contains an invalid value.
     */
    public static EtherAddress create(EthernetAddress eaddr) {
        return (eaddr == null) ? null : new EtherAddress(eaddr);
    }

    /**
     * Verify the given byte array which represents an ethernet address.
     *
     * @param b  A byte array to be tested.
     * @throws IllegalArgumentException
     *    The length of {@code b} is not 6.
     */
    private static void checkAddress(byte[] b) {
        if (b.length != SIZE) {
            throw new IllegalArgumentException(
                "Invalid byte array length: " + b.length);
        }
    }

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private EtherAddress() {
    }

    /**
     * Construct a new instance.
     *
     * @param addr  A long integer value which represents an ethernet address.
     */
    public EtherAddress(long addr) {
        address = addr;
    }

    /**
     * Construct a new instance.
     *
     * @param bytes  A byte array which represents an ethernet address.
     * @throws NullPointerException
     *    {@code bytes} is {@code null}.
     * @throws IllegalArgumentException
     *    The length of {@code bytes} is not 6.
     */
    public EtherAddress(byte[] bytes) {
        byteAddress = bytes.clone();
        address = toLong(byteAddress);
    }

    /**
     * Construct a new instance.
     *
     * @param hex  A hex string which represents an ethernet address.
     * @throws NullPointerException
     *    {@code hex} is {@code null}.
     * @throws IllegalArgumentException
     *    The given string is not a string representation of an ethernet
     *    address.
     */
    public EtherAddress(String hex) {
        // Enforce lower case.
        String lhex = hex.toLowerCase(Locale.ENGLISH);
        macAddress = new MacAddress(lhex);
        address = toLong(lhex);
    }

    /**
     * Construct a new instance.
     *
     * @param mac  A {@link MacAddress} instance.
     * @throws NullPointerException
     *    {@code mac} is {@code null}.
     * @throws IllegalArgumentException
     *    The given instance contains an invalid value.
     */
    public EtherAddress(MacAddress mac) {
        // Enforce lower case.
        String hex = mac.getValue();
        String lhex = hex.toLowerCase(Locale.ENGLISH);
        if (hex.equals(lhex)) {
            macAddress = mac;
        } else {
            macAddress = new MacAddress(lhex);
        }
        address = toLong(lhex);
    }

    /**
     * Construct a new instance.
     *
     * @param eaddr  An {@link EthernetAddress} instance.
     * @throws NullPointerException
     *    {@code eaddr} is {@code null}.
     * @throws IllegalArgumentException
     *    The given instance contains an invalid value.
     */
    public EtherAddress(EthernetAddress eaddr) {
        this(eaddr.getValue());
    }

    /**
     * Return a long number which represents this ethernet address.
     *
     * @return  A long number.
     */
    public long getAddress() {
        return address;
    }

    /**
     * Return a byte array which represents this ethernet address.
     *
     * @return  A byte array.
     */
    public byte[] getBytes() {
        return getBytesImpl().clone();
    }

    /**
     * Return a {@link MacAddress} instance which represents this ethernet
     * address.
     *
     * @return  A {@link MacAddress} instance.
     */
    public MacAddress getMacAddress() {
        MacAddress mac = macAddress;
        if (mac == null) {
            byte[] bytes = getBytesImpl();
            String hex = ByteUtils.toHexString(bytes);
            mac = new MacAddress(hex);
            macAddress = mac;
        }

        return mac;
    }

    /**
     * Return a {@link EthernetAddress} instance which represents this ethernet
     * address.
     *
     * @return  A {@link EthernetAddress} instance.
     */
    public EthernetAddress getEthernetAddress() {
        // EthernetAddress has a vulnerability that can corrupt internal byte
        // array. So we don't cache it.
        byte[] bytes = getBytesImpl();
        try {
            return new EthernetAddress(bytes);
        } catch (Exception e) {
            // This should never happen.
            String msg = "Failed to create EthernetAddress: " + getText();
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Return a hex string which represents this ethernet address.
     *
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   A string representation of a MAC address.
     *
     *   <p>
     *     A MAC address is represented by hexadecimal notation with
     *     {@code ':'} inserted between octets.
     *     (e.g. {@code "11:22:33:aa:bb:cc"})
     *   </p>
     */
    @XmlValue
    public String getText() {
        return getMacAddress().getValue();
    }

    /**
     * Determine whether this instance represents a broadcast address or not.
     *
     * @return  {@code true} only if this instance represents a broadcast
     *          address.
     */
    public boolean isBroadcast() {
        return (address == BROADCAST.address);
    }

    /**
     * Determine whether this instance represents an unicast address or not.
     *
     * @return  {@code true} only if this instance represents an unicast
     *          address.
     */
    public boolean isUnicast() {
        return ((address & MASK_MULTICAST) == 0);
    }

    /**
     * Create a byte array that represents this ethernet address, and cache it
     * in this instance.
     *
     * @return  A byte array that represents this ethernet address.
     */
    private byte[] getBytesImpl() {
        byte[] bytes = byteAddress;
        if (bytes == null) {
            bytes = toBytes(address);
            byteAddress = bytes;
        }

        return bytes;
    }

    /**
     * Set a hex string which represents an ethernet address.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param hex  A hex strin which represents an ethernet address.
     */
    @SuppressWarnings("unused")
    private void setText(String hex) {
        address = toLong(hex.trim());
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        EtherAddress ea = (EtherAddress)o;
        return (address == ea.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return NumberUtils.hashCode(address);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("EtherAddress[");
        return builder.append(getText()).append(']').toString();
    }
}
