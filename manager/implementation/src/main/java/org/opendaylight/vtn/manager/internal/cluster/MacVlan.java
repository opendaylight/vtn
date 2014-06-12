/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code MacVlan} class represents a pair of MAC address and VLAN ID.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class MacVlan implements Serializable, Comparable<MacVlan> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 3174739271911614298L;

    /**
     * A pseudo MAC address which represents undefined value.
     */
    public static final long  UNDEFINED = 0;

    /**
     * The number of bits in a valid MAC address.
     */
    private static final int  NBITS_MAC = 48;

    /**
     * The number of bits in a valid VLAN ID.
     */
    private static final int  NBITS_VLAN_ID = 12;

    /**
     * Mask value which represents a VLAN ID bits in a long integer.
     */
    public static final long MASK_VLAN_ID = (1L << NBITS_VLAN_ID) - 1L;

    /**
     * Mask value which represents valid bits in {@link #encodedValue}.
     */
    private static final long MASK_ENCODED =
        (1L << (NBITS_MAC + NBITS_VLAN_ID)) - 1L;

    /**
     * A long value which keeps a MAC address and a VLAN ID.
     */
    private final long  encodedValue;

    /**
     * Construct a new insttance from a long integer which contains a MAC
     * address and a VLAN ID.
     *
     * <ul>
     *   <li>
     *     Bits from bit 11 to bit 0 (LSB) are treated as a VLAN ID.
     *   </li>
     *   <li>
     *     Bits from bit 59 to bit 12 are treated as a MAC address.
     *   </li>
     * </ul>
     *
     * @param value  A long value which contains a MAC address and a VLAN ID.
     */
    public MacVlan(long value) {
        encodedValue = (value & MASK_ENCODED);
    }

    /**
     * Construct a new instance.
     *
     * @param mac   A long value which represents a MAC address.
     *              Only lower 48 bits in the value is used.
     *              {@link #UNDEFINED} is treated as undefined value.
     * @param vlan  VLAN ID. Only lower 12 bits in the value is used.
     */
    public MacVlan(long mac, short vlan) {
        encodedValue = encode(mac, vlan);
    }

    /**
     * Construct a new instance.
     *
     * @param mac  A byte array which represents a MAC address.
     *             {@code null} and all-zeroed byte array are treated as
     *             undefined value.
     * @param vlan  VLAN ID. Only lower 12 bits in the value is used.
     */
    public MacVlan(byte[] mac, short vlan) {
        this(NetUtils.byteArray6ToLong(mac), vlan);
    }

    /**
     * Construct a new instance which represents the specified data link
     * layer host.
     *
     * <p>
     *   This constructor is used to create a new instance for MAC mapping
     *   configuration.
     * </p>
     *
     * @param dlhost  A {@link DataLinkHost} instance.
     * @throws VTNException
     *    A invalid value is specified to {@code dlhost}.
     */
    public MacVlan(DataLinkHost dlhost) throws VTNException {
        if (dlhost == null) {
            Status s = VTNManagerImpl.argumentIsNull("Data link layer host");
            throw new VTNException(s);
        }

        if (!(dlhost instanceof EthernetHost)) {
            String msg = "Unsupported address type: " + dlhost;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        EthernetHost ehost = (EthernetHost)dlhost;
        EthernetAddress addr = ehost.getAddress();
        long mac;
        if (addr == null) {
            mac = UNDEFINED;
        } else {
            mac = getMacAddress(addr);
        }

        short vlan = ehost.getVlan();
        VTNManagerImpl.checkVlan(vlan);
        encodedValue = encode(mac, vlan);
    }

    /**
     * Return a long value which represents a MAC address.
     *
     * @return  A long value which represent a MAC address.
     *          {@link #UNDEFINED} is returned if no MAC address is configured
     *          in this instance.
     */
    public long getMacAddress() {
        return (encodedValue >>> NBITS_VLAN_ID);
    }

    /**
     * Return a VLAN ID.
     *
     * @return  VLAN ID.
     */
    public short getVlan() {
        return (short)(encodedValue & MASK_VLAN_ID);
    }

    /**
     * Return a long integer value encoded from a MAC address and a VLAN ID.
     *
     * <p>
     *   Lower 48-bits of the returned value keeps a MAC address.
     *   And higher 16-bits of the returned value keeps a VLAN ID.
     * </p>
     *
     * @return  A long integer value encoded from a MAC address and a VLAN ID.
     */
    public long getEncodedValue() {
        return encodedValue;
    }

    /**
     * Return a {@link EthernetHost} instance which represents this instance.
     *
     * @return  A {@link EthernetHost} instance which represents this instance.
     */
    public EthernetHost getEthernetHost() {
        try {
            short vlan = getVlan();
            long mac = getMacAddress();
            EthernetAddress eaddr;
            if (mac == UNDEFINED) {
                eaddr = null;
            } else {
                byte[] raw = NetUtils.longToByteArray6(mac);
                eaddr = new EthernetAddress(raw);
            }

            return new EthernetHost(eaddr, vlan);
        } catch (Exception e) {
            // This should never happen.
            throw new IllegalStateException
                ("Unable to create EthernetHost", e);
        }
    }

    /**
     * Append human readable strings which represents the contents of this
     * object to the specified {@link StringBuilder}.
     *
     * @param builder  A {@link StringBuilder} instance.
     */
    public void appendContents(StringBuilder builder) {
        long mac = getMacAddress();
        short vlan = getVlan();

        if (mac != UNDEFINED) {
            builder.append("addr=").
                append(VTNManagerImpl.formatMacAddress(mac)).append(',');
        }
        builder.append("vlan=").append((int)vlan);
    }

    /**
     * Encode the specified MAC address in long integer and VLAN ID into
     * a long integer.
     *
     * @param mac   A long value which represents a MAC address.
     *              Only lower 48 bits in the value is used.
     *              {@link #UNDEFINED} is treated as undefined value.
     * @param vlan  VLAN ID. Only lower 12 bits in the value is used.
     * @return  A long integer encoded from the specified MAC address and
     *          VLAN ID.
     */
    private long encode(long mac, short vlan) {
        return (((mac << NBITS_VLAN_ID) | ((long)vlan & MASK_VLAN_ID)) &
                MASK_ENCODED);
    }

    /**
     * Convert a {@link EthernetAddress eth} into a long integer which
     * represents a MAC address.
     *
     * @param eth  A {@link EthernetAddress} instance.
     * @return     A long integer which represents a MAC address.
     * @throws VTNException
     *    A invalid address is specified to {@code eth}.
     */
    private long getMacAddress(EthernetAddress eth)
        throws VTNException {
        byte[] raw = eth.getValue();
        long mac = NetUtils.byteArray6ToLong(raw);
        String badaddr = null;
        if (NetUtils.isBroadcastMACAddr(raw)) {
            badaddr = "Broadcast address";
        } else if (!NetUtils.isUnicastMACAddr(raw)) {
            badaddr = "Multicast address";
        } else if (mac == UNDEFINED) {
            badaddr = "Zero";
        } else {
            return mac;
        }

        StringBuilder builder = new StringBuilder(badaddr);
        builder.append(" cannot be specified: ").
            append(VTNManagerImpl.formatMacAddress(mac));
        throw new VTNException(StatusCode.BADREQUEST, builder.toString());
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
        if (!(o instanceof MacVlan)) {
            return false;
        }

        MacVlan mvlan = (MacVlan)o;
        return (encodedValue == mvlan.encodedValue);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return VTNManagerImpl.hashCode(encodedValue);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("MacVlan[");
        appendContents(builder);
        builder.append(']');

        return builder.toString();
    }

    // Comparable

    /**
     * Compare two {@code MacVlan} instances numerically.
     *
     * <p>
     *   This method compares MAC addresses in both objects first.
     *   If the same MAC address is configured in both objects, compares
     *   VLAN IDs in both objects.
     * </p>
     *
     * @param  mvlan  A {@code MacVlan} instance to be compared.
     * @return   {@code 0} is returned if this instance is equal to
     *           the specified instance.
     *           A value less than {@code 0} is returned if this instance is
     *           numerically less than the specified instance.
     *           A value greater than {@code 0} is returned if this instance is
     *           numerically greater than the specified instance.
     */
    @Override
    public int compareTo(MacVlan mvlan) {
        return Long.compare(encodedValue, mvlan.encodedValue);
    }
}
