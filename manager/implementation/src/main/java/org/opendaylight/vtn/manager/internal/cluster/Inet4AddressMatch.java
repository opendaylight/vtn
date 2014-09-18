/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.io.Serializable;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.MiscUtils;

import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code Inet4AddressMatch} describes an IPv4 address and netmask.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class Inet4AddressMatch implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4629368100388947618L;

    /**
     * An IP address mask value which indicates all bits in an IPv4 address.
     */
    public static final int  MASK_ALL = -1;

    /**
     * The number of bits in an IPv4 address.
     */
    private static final int  LENGTH = 32;

    /**
     * The minimum value of the CIDR suffix.
     */
    protected static final int  CIDR_SUFFIX_MIN = 1;

    /**
     * The maximum value of the CIDR suffix.
     */
    protected static final int  CIDR_SUFFIX_MAX = LENGTH - 1;

    /**
     * An IPv4 address.
     */
    private final int  address;

    /**
     * A netmask used to test IP address.
     */
    private final int  mask;

    /**
     * Construct a new instance.
     *
     * @param addr  An {@link InetAddress} instance which represents an
     *              IPv4 address.
     * @param suff  A {@link Short} instance which represents a CIDR suffix
     *              which represents netmask.
     * @throws NullPointerException
     *    {@code addr} is {@code null}.
     * @throws VTNException
     *    Invalid value is specified to {@code suff}.
     */
    public Inet4AddressMatch(InetAddress addr, Short suff)
        throws VTNException {
        mask = toNetMask(suff);
        address = NetUtils.byteArray4ToInt(addr.getAddress()) & mask;
    }

    /**
     * Return an integer value which represents an IPv4 address configured in
     * this instance.
     *
     * @return  An integer value which represents an IPv4 address.
     */
    public int getAddress() {
        return address;
    }

    /**
     * Return an {@link InetAddress} instance configured in this instance.
     *
     * @return  An {@link InetAddress} instance.
     * @throws IllegalStateException
     *    An error occurred.
     */
    public InetAddress getInetAddress() {
        return MiscUtils.toInetAddress(address);
    }

    /**
     * Return a netmask configured in this instance.
     *
     * @return  A netmask configured in this instance.
     *          {@link #MASK_ALL} is returned if no netmask is configured.
     */
    public int getMask() {
        return mask;
    }

    /**
     * Return a CIDR suffix configured in this instance.
     *
     * @return  A {@link Short} instance which represents a CIDR suffix.
     *          {@code null} is returned if no CIDR suffix is configured.
     */
    public Short getCidrSuffix() {
        if (mask == MASK_ALL) {
            return null;
        }

        int nbits = Integer.SIZE - Integer.numberOfTrailingZeros(mask);
        return Short.valueOf((short)nbits);
    }

    /**
     * Determine whether the specified IPv4 address match the condition
     * described by this instance.
     *
     * @param addr  An integer value which represents an IPv4 address.
     * @return  {@code true} is returned if the specified IPv4 address matches
     *          the condition. Otherwise {@code false} is returned.
     */
    public boolean match(int addr) {
        return (address == (addr & mask));
    }

    /**
     * Convert a CIDR suffix into netmask.
     *
     * @param suffix  A {@link Short} instance which represents a CIDR suffix.
     * @return  A netmask converted from {@code suffix}.
     * @throws VTNException
     *    An invalid CIDR suffix is specified.
     */
    private int toNetMask(Short suffix) throws VTNException {
        if (suffix == null) {
            return MASK_ALL;
        }

        int suff = suffix.intValue();
        if (suff < CIDR_SUFFIX_MIN || suff > CIDR_SUFFIX_MAX) {
            String msg = "Invalid CIDR suffix: " + suff;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        int nzeroes = LENGTH - suff;
        return (MASK_ALL << nzeroes);
    }

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
        if (!(o instanceof Inet4AddressMatch)) {
            return false;
        }

        Inet4AddressMatch cond = (Inet4AddressMatch)o;

        return (address == cond.address && mask == cond.mask);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return address;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        byte[] addr = NetUtils.intToByteArray4(address);
        String sep = "";
        for (byte b: addr) {
            int i = NetUtils.getUnsignedByte(b);
            builder.append(sep).append(i);
            sep = ".";
        }

        if (mask != MASK_ALL) {
            int suff = Integer.SIZE - Integer.numberOfTrailingZeros(mask);
            builder.append('/').append(suff);
        }

        return builder.toString();
    }
}
