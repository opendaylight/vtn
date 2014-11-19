/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.EtherPacket;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code EthernetMatchImpl} describes the condition to match Ethernet header
 * fields in packet.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class EthernetMatchImpl implements PacketMatch {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -3306965790129460767L;

    /**
     * A pseudo MAC address which indicates every MAC address should match.
     */
    private static final long  MAC_ANY = -1L;

    /**
     * A pseudo Ethernet type value which indicates every Ethernet type
     * should match.
     */
    private static final int  ETHTYPE_ANY = -1;

    /**
     * A pseudo VLAN ID which indicates every VLAN ID should match.
     */
    private static final short  VLAN_ANY = -1;

    /**
     * A pseudo VLAN priority which indicates every VLAN priority should match.
     */
    private static final byte  VLANPRI_ANY = -1;

    /**
     * A mask value which represents valid bits in an Ethernet type.
     */
    private static final int  MASK_TYPE = 0xffff;

    /**
     * Source MAC address to match.
     */
    private final long  sourceAddress;

    /**
     * Destination MAC address to match.
     */
    private final long  destinationAddress;

    /**
     * Ethernet type to match.
     */
    private int  etherType;

    /**
     * VLAN ID to match.
     */
    private final short  vlan;

    /**
     * VLAN priority to match.
     */
    private final byte  vlanPriority;

    /**
     * Construct a new instance which specifies only the condition for the
     * Ethernet type.
     *
     * @param type  An Ethernet type.
     */
    public EthernetMatchImpl(int type) {
        sourceAddress = MAC_ANY;
        destinationAddress = MAC_ANY;
        etherType = type;
        vlan = VLAN_ANY;
        vlanPriority = VLANPRI_ANY;
    }

    /**
     * Construct a new instance.
     *
     * @param match  An {@link EthernetMatch} instance.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws VTNException
     *    {@code match} contains invalid value.
     */
    public EthernetMatchImpl(EthernetMatch match) throws VTNException {
        Status st = match.getValidationStatus();
        if (st != null) {
            throw new VTNException(st);
        }

        sourceAddress = getMacAddress(match.getSourceAddress());
        destinationAddress = getMacAddress(match.getDestinationAddress());

        Integer i = match.getType();
        if (i == null) {
            etherType = ETHTYPE_ANY;
        } else {
            etherType = i.intValue();
            if ((etherType & ~MASK_TYPE) != 0) {
                String msg = "Invalid Ethernet type: " + i;
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
        }

        Short s = match.getVlan();
        if (s == null) {
            vlan = VLAN_ANY;
        } else {
            vlan = s.shortValue();
            MiscUtils.checkVlan(vlan);
        }

        Byte b = match.getVlanPriority();
        if (b == null) {
            vlanPriority = VLANPRI_ANY;
        } else {
            vlanPriority = b.byteValue();
            if (!MiscUtils.isVlanPriorityValid(vlanPriority)) {
                String msg = "Invalid VLAN priority: " + b;
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
            if (vlan <= 0) {
                String msg = "VLAN priority requires a valid VLAN ID.";
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
        }
    }

    /**
     * Return a long value which represents the source MAC address to match
     * against packets.
     *
     * @return  A long value which represents the source MAC address to match.
     *          A negative value is returned if the source MAC address is
     *          not specified.
     */
    public long getSourceAddress() {
        return sourceAddress;
    }

    /**
     * Return a long value which represents the destination MAC address to
     * match against packets.
     *
     * @return  A long value which represents the destination MAC address to
     *          match. A negative value is returned if the source MAC address
     *          is not specified.
     */
    public long getDestinationAddress() {
        return destinationAddress;
    }

    /**
     * Return the Ethernet type to match against packets.
     *
     * @return  An integer which represents the Ethernet type to mach against
     *          packets. A negative value is returned if the Ethernet type is
     *          not specified.
     */
    public int getEtherType() {
        return etherType;
    }

    /**
     * Return the VLAN ID to match against packets.
     *
     * @return  A short integer value which represents the VLAN ID to mach
     *          against packets. A negative value is returned if the VLAN ID
     *          is not specified.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Return the VLAN priority to match against packets.
     *
     * @return  A byte value which represents the VLAN priority to mach
     *          against packets. A negative value is returned if the
     *          VLAN priority is not specified.
     */
    public byte getVlanPriority() {
        return vlanPriority;
    }

    /**
     * Return an {@link EthernetMatch} instance which represents this
     * condition.
     *
     * @return  An {@link EthernetMatch} instance.
     */
    public EthernetMatch getMatch() {
        EthernetAddress src = toEthernetAddress(sourceAddress);
        EthernetAddress dst = toEthernetAddress(destinationAddress);
        Integer ethType = (etherType < 0) ? null : Integer.valueOf(etherType);
        Short vid = (vlan < 0) ? null : Short.valueOf(vlan);
        Byte pri = (vlanPriority < 0) ? null : Byte.valueOf(vlanPriority);

        return new EthernetMatch(src, dst, ethType, vid, pri);
    }

    /**
     * Set the Ethernet type to match against packets.
     *
     * @param type  An Ethernet type.
     * @throws VTNException
     *    The specified type is different from the type in this instance.
     */
    void setEtherType(int type) throws VTNException {
        if (etherType != ETHTYPE_ANY && etherType != type) {
            StringBuilder builder =
                new StringBuilder("Ethernet type conflict: type=0x");
            builder.append(Integer.toHexString(etherType)).
                append(", expected=0x").append(Integer.toHexString(type));
            throw new VTNException(StatusCode.BADREQUEST, builder.toString());
        }

        etherType = type;
    }

    /**
     * Convert an {@link EthernetAddress} instance into a long integer value.
     *
     * @param eaddr  An {@link EthernetAddress} instance or {@code null}.
     * @return  A long integer which represents the specified value.
     *          {@link #MAC_ANY} is returned if {@code null} is specified.
     */
    private long getMacAddress(EthernetAddress eaddr) {
        if (eaddr == null) {
            return MAC_ANY;
        }

        byte[] raw = eaddr.getValue();
        return NetUtils.byteArray6ToLong(raw);
    }

    /**
     * Convert a long integer value into an {@link EthernetAddress} instance.
     *
     * @param mac  A long integer which represents a MAC address.
     * @return  A {@link EthernetAddress} instance converted from the specified
     *          value.
     *          {@code null} is returned if {@link #MAC_ANY} is specified.
     */
    private EthernetAddress toEthernetAddress(long mac) {
        if (mac == MAC_ANY) {
            return null;
        }

        byte[] b = NetUtils.longToByteArray6(mac);
        try {
            return new EthernetAddress(b);
        } catch (Exception e) {
            // This should never happen.
            StringBuilder builder =
                new StringBuilder("Unexpected exception: addr=");
            builder.append(Long.toHexString(mac));
            throw new IllegalStateException(builder.toString(), e);
        }
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
        if (!(o instanceof EthernetMatchImpl)) {
            return false;
        }

        EthernetMatchImpl match = (EthernetMatchImpl)o;
        return (sourceAddress == match.sourceAddress &&
                destinationAddress == match.destinationAddress &&
                etherType == match.etherType && vlan == match.vlan &&
                vlanPriority == match.vlanPriority);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return MiscUtils.hashCode(sourceAddress) +
            (MiscUtils.hashCode(destinationAddress) * 7) +
            (etherType * 13) + ((int)vlan * 17) * (vlanPriority * 31);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("EthernetMatchImpl[");
        String sep = "";
        if (sourceAddress != MAC_ANY) {
            builder.append(sep).append("src=").
                append(MiscUtils.formatMacAddress(sourceAddress));
            sep = ",";
        }
        if (destinationAddress != MAC_ANY) {
            builder.append(sep).append("dst=").
                append(MiscUtils.formatMacAddress(destinationAddress));
            sep = ",";
        }
        if (etherType != ETHTYPE_ANY) {
            builder.append(sep).append("type=0x").
                append(Integer.toHexString(etherType));
            sep = ",";
        }
        if (vlan != VLAN_ANY) {
            builder.append(sep).append("vlan=").append((int)vlan);
            sep = ",";
        }
        if (vlanPriority != VLANPRI_ANY) {
            builder.append(sep).append("pcp=").append((int)vlanPriority);
        }

        builder.append(']');

        return builder.toString();
    }

    // PacketMatch

    /**
     * Determine whether the specified packet matches the condition defined
     * by this instance.
     *
     * @param pctx  The context of the packet to be tested.
     * @return  {@code true} if the specified packet matches the condition.
     *          Otherwise {@code false}.
     */
    @Override
    public boolean match(PacketContext pctx) {
        EtherPacket ether = pctx.getEtherPacket();

        // Test source MAC address.
        if (sourceAddress != MAC_ANY) {
            pctx.addMatchField(MatchType.DL_SRC);
            if (sourceAddress != ether.getSourceMacAddress()) {
                return false;
            }
        }

        // Test destination MAC address.
        if (destinationAddress != MAC_ANY) {
            pctx.addMatchField(MatchType.DL_DST);
            if (destinationAddress != ether.getDestinationMacAddress()) {
                return false;
            }
        }

        // Test Ethernet type.
        if (etherType != ETHTYPE_ANY) {
            pctx.addMatchField(MatchType.DL_TYPE);
            if (etherType != ether.getEtherType()) {
                return false;
            }
        }

        // Test VLAN ID.
        // We don't need to set DL_VLAN field to PacketContext because it is
        // mandatory.
        if (vlan != VLAN_ANY) {
            if (vlan != ether.getVlan()) {
                return false;
            }

            // Test VLAN priority only if a VLAN ID is specified.
            if (vlanPriority != VLANPRI_ANY) {
                pctx.addMatchField(MatchType.DL_VLAN_PR);
                if (vlanPriority != ether.getVlanPriority()) {
                    return false;
                }
            }
        }

        return true;
    }
}
