/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.net.InetAddress;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4Builder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpPrefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;

/**
 * {@code Ip4Network} describes an IPv4 network.
 * IPv4 network is specified by an IPv4 address and prefix length.
 *
 * @since  Lithium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "ip4-network")
@XmlAccessorType(XmlAccessType.NONE)
public final class Ip4Network extends IpNetwork {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -7431660855016600442L;

    /**
     * The number of octets in an IPv4 address.
     */
    public static final int  SIZE = 4;

    /**
     * An IPv4 netmask which indicates all bits in an IPv4 address are set.
     */
    private static final int  NETMASK_ALL = -1;

    /**
     * An integer value which represents an IPv4 network address.
     */
    private int  address;

    /**
     * An integer value which represents a network mask.
     */
    private int  netMask;

    /**
     * A byte array which represents an IPv4 network address.
     */
    private byte[]  byteAddress;

    /**
     * A string representation of an IPv4 network address.
     * Note that this text does not contain prefix length.
     */
    private String  networkAddress;

    /**
     * An {@link IpPrefix} instance which represents this IPv4 network.
     */
    private IpPrefix  ipPrefix;

    /**
     * Return an IPv4 prefix length represented by the given byte array.
     *
     * <p>
     *   Note that this method returns 32 if all the bits in the given array
     *   are not set.
     * </p>
     *
     * @param bytes  A byte array which represents IPv4 network mask.
     * @return  The IPv4 prefix length represented by the given byte array.
     * @throws NullPointerException
     *    {@code bytes} is {@code null}.
     * @throws IllegalArgumentException
     *    The given byte array does not represent an IPv4 network mask.
     */
    public static int getPrefixLength(byte[] bytes) {
        // Verify the given network mask.
        int mask = NumberUtils.toInteger(bytes);
        if (mask == 0) {
            return Integer.SIZE;
        }

        int inv = ~mask;
        int p2 = inv + 1;
        if ((p2 & inv) != 0) {
            throw new IllegalArgumentException(
                "Invalid IPv4 netmask: " + Integer.toHexString(mask));
        }

        return Integer.numberOfLeadingZeros(inv);
    }

    /**
     * Return an integer value which represents the IPv4 netmask specified by
     * the given prefix length.
     *
     * @param length  The IPv4 prefix length.
     *                Note that zero means "no mask". So zero is treated as if
     *                the maximum prefix length is specified.
     * @return  An integer value.
     * @throws IllegalArgumentException
     *    The given prefix length is invalid.
     */
    public static int getNetMask(int length) {
        if (length < 0 || length > Integer.SIZE) {
            throw new IllegalArgumentException(
                "Invalid prefix length: " + length);
        }

        int nzeroes = Integer.SIZE - length;
        return (NETMASK_ALL << nzeroes);
    }

    /**
     * Return an {@link InetAddress} instance which represents the IPv4 netmask
     * specified by the given prefix length.
     *
     * @param length  The IPv4 prefix length.
     *                Note that zero means "no mask". So zero is treated as if
     *                the maximum prefix length is specified.
     * @return  An {@link InetAddress} instance.
     * @throws IllegalArgumentException
     *    The given prefix length is invalid.
     */
    public static InetAddress getInetMask(int length) {
        return getInetAddress(getNetMask(length));
    }

    /**
     * Create an IPv4 network address from the given IPv4 address and
     * prefix length.
     *
     * @param iaddr   An {@link InetAddress} instance.
     * @param length  The IPv4 prefix length.
     *                Note that zero means "no mask". So zero is treated as if
     *                the maximum prefix length is specified.
     * @return  An {@link InetAddress} instance which represents the IPv4
     *          network address specified by the given pair of IPv4 address and
     *          prefix length.
     * @throws NullPointerException
     *    {@code iaddr} is {@code null}.
     * @throws IllegalArgumentException
     *    The given IPv4 address or prefix length is invalid.
     */
    public static InetAddress getNetworkAddress(InetAddress iaddr,
                                                int length) {
        int addr = NumberUtils.toInteger(iaddr.getAddress());
        int mask = getNetMask(length);
        return getInetAddress(addr & mask);
    }

    /**
     * Create an {@link InetAddress} instance which represents the IPv4 address
     * specified by the given integer value.
     *
     * @param addr  An integer value which represents the IPv4 address.
     * @return  An {@link InetAddress} instance.
     */
    public static InetAddress getInetAddress(int addr) {
        return getInetAddress(NumberUtils.toBytes(addr));
    }

    /**
     * Ensure that the given {@link IpNetwork} instance represents an
     * IPv4 address.
     *
     * @param ipn  An {@link IpNetwork} instance.
     * @return  {@code ipn} casted as {@link Ip4Network} if {@code ipn}
     *          represents an IPv4 address. Otherwise {@code null}.
     */
    public static Ip4Network toIp4Address(IpNetwork ipn) {
        return (ipn instanceof Ip4Network && ipn.isAddress())
            ? (Ip4Network)ipn : null;
    }

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private Ip4Network() {
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor specifies 32 as CIDR prefix length.
     * </p>
     *
     * @param addr  An integer value which represents an IPv4 address.
     */
    public Ip4Network(int addr) {
        this(addr, Integer.SIZE);
    }

    /**
     * Construct a new instance.
     *
     * @param addr    An integer value which represents an IPv4 address.
     * @param prefix  Prefix length that specifies network range.
     *                Note that zero means "no mask". So zero is treated as if
     *                the maximum prefix length is specified.
     * @throws IllegalArgumentException
     *    The given prefix length is invalid.
     */
    public Ip4Network(int addr, int prefix) {
        super(prefix);
        int mask = getNetMask(getPrefixLength());
        address = addr & mask;
        netMask = mask;
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor specifies 32 as CIDR prefix length.
     * </p>
     *
     * @param bytes  A byte array which represents an IPv4 address.
     * @throws NullPointerException
     *    {@code bytes} is {@code null}.
     * @throws IllegalArgumentException
     *    The given byte address does not represent an IPv4 address.
     */
    public Ip4Network(byte[] bytes) {
        this(bytes, Integer.SIZE);
    }

    /**
     * Construct a new instance.
     *
     * @param bytes  A byte array which represents an IPv4 address.
     * @param prefix  Prefix length that specifies network range.
     *                Note that zero means "no mask". So zero is treated as if
     *                the maximum prefix length is specified.
     * @throws NullPointerException
     *    {@code bytes} is {@code null}.
     * @throws IllegalArgumentException
     *    The given prefix length is invalid.
     * @throws IllegalArgumentException
     *    The given byte address does not represent an IPv4 address.
     */
    public Ip4Network(byte[] bytes, int prefix) {
        super(prefix);
        int addr = NumberUtils.toInteger(bytes);
        int plen = getPrefixLength();
        int mask = getNetMask(plen);
        netMask = mask;
        if (plen == Integer.SIZE) {
            address = addr;
            byteAddress = bytes.clone();
        } else {
            address = addr & mask;
        }
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor specifies 32 as CIDR prefix length.
     * </p>
     *
     * @param iaddr  An {@link InetAddress} instance which represents an IPv4
     *               address.
     * @throws NullPointerException
     *    {@code iaddr} is {@code null}.
     * @throws IllegalArgumentException
     *    The given {@link InetAddress} instance does not represent an IPv4
     *    address.
     */
    public Ip4Network(InetAddress iaddr) {
        super(iaddr, Integer.SIZE);
    }

    /**
     * Construct a new instance.
     *
     * @param iaddr  An {@link InetAddress} instance which represents an IPv4
     *               address.
     * @param prefix  Prefix length that specifies network range.
     *                Note that zero means "no mask". So zero is treated as if
     *                the maximum prefix length is specified.
     * @throws NullPointerException
     *    {@code iaddr} is {@code null}.
     * @throws IllegalArgumentException
     *    The given prefix length is invalid.
     * @throws IllegalArgumentException
     *    The given {@link InetAddress} instance does not represent an IPv4
     *    address.
     */
    public Ip4Network(InetAddress iaddr, int prefix) {
        super(iaddr, prefix);
    }

    /**
     * Construct a new instance.
     *
     * @param cidr  A string representation of the IP network in CIDR notation.
     *              Note that zero prefix means "no mask". So zero prefix is
     *              treated as if the maximum prefix length is specified.
     * @throws NullPointerException
     *    {@code cidr} is {@code null}.
     * @throws IllegalArgumentException
     *    The given network address is invalid.
     */
    public Ip4Network(String cidr) {
        super(cidr);
    }

    /**
     * Return an integer value which represents the host address.
     *
     * @return  An integer value which represents the host address.
     */
    public int getAddress() {
        return address;
    }

    /**
     * Return an integer value which represents the network mask.
     *
     * @return  An integer value which represents the network mask.
     */
    public int getNetMask() {
        return netMask;
    }

    // IpNetwork

    /**
     * {@inheritDoc}
     */
    @Override
    public int getMaxPrefix() {
        return Integer.SIZE;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IpPrefix getIpPrefix() {
        IpPrefix ip = ipPrefix;
        if (ip == null) {
            StringBuilder builder = new StringBuilder();
            builder.append(getHostAddress()).append(CIDR_SEPARATOR).
                append(getPrefixLength());
            ip = new IpPrefix(new Ipv4Prefix(builder.toString()));
            ipPrefix = ip;
        }

        return ip;
    }

    /**
     * Return a MD-SAL IP address which represents this network address.
     *
     * @return  An {@link Ipv4} instance.
     */
    @Override
    public Ipv4 getMdAddress() {
        IpPrefix ipp = getIpPrefix();
        return new Ipv4Builder().setIpv4Address(ipp.getIpv4Prefix()).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getCidrText() {
        return getIpPrefix().getIpv4Prefix().getValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getHostAddress() {
        String addr = networkAddress;
        if (addr == null) {
            addr = getInetAddress().getHostAddress();
            networkAddress = addr;
        }

        return addr;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public byte[] getBytes() {
        byte[] bytes = byteAddress;
        if (bytes == null) {
            bytes = NumberUtils.toBytes(address);
            byteAddress = bytes;
        }

        return bytes.clone();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean contains(IpNetwork inw) {
        if (!inw.getClass().equals(Ip4Network.class)) {
            throw new IllegalArgumentException("Unexpected IpNetwork: " + inw);
        }

        Ip4Network ip4 = (Ip4Network)inw;
        return (address == (ip4.address & netMask));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    InetAddress init(InetAddress iaddr, int prefix) {
        int addr = NumberUtils.toInteger(iaddr.getAddress());
        int mask = getNetMask(prefix);
        int maskedAddr = addr & mask;
        address = maskedAddr;
        netMask = mask;
        return getInetAddress(maskedAddr);
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
        if (o == null || !Ip4Network.class.equals(o.getClass())) {
            return false;
        }

        Ip4Network ip4 = (Ip4Network)o;
        return (address == ip4.address &&
                getPrefixLength() == ip4.getPrefixLength());
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Ip4Network.class.getName().hashCode() * address;
    }
}
