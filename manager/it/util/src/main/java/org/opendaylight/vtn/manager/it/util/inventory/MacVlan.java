/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.inventory;

import java.util.Locale;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.it.util.VlanDescParser;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowSourceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescListBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code MacVlan} class represents a pair of MAC address and VLAN ID.
 */
public final class MacVlan implements Comparable<MacVlan> {
    /**
     * A pseudo MAC address which represents undefined value.
     */
    public static final long  UNDEFINED = 0;

    /**
     * The number of bits in a valid VLAN ID.
     */
    public static final int  NBITS_VLAN_ID = 12;

    /**
     * The number of bits in a valid MAC address.
     */
    public static final int  NBITS_MAC = 48;

    /**
     * Mask value which represents valid bits in {@link #encodedValue}.
     */
    private static final long  MASK_ENCODED =
        (1L << (NBITS_MAC + NBITS_VLAN_ID)) - 1L;

    /**
     * Mask value which represents a VLAN ID bits in a long integer.
     */
    private static final long MASK_VLAN_ID = (1L << NBITS_VLAN_ID) - 1L;

    /**
     * A string that indicates vlan-host-desc.
     */
    private static final String   DESC_VLAN_HOST_DESC = "vlan-host-desc";

    /**
     * A long value which keeps a MAC address and a VLAN ID.
     */
    private long  encodedValue;

    /**
     * Cache for a string representation.
     */
    private String  stringCache;

    /**
     * Cache for a hash code.
     */
    private int  hash;

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
    public MacVlan(long mac, int vlan) {
        encodedValue = encode(mac, vlan);
    }

    /**
     * Construct a new instance.
     *
     * @param eaddr  An {@link EtherAddress} instance that represents a
     *               MAC address.
     * @param vlan   VLAN ID. Only lower 12 bits in the value is used.
     */
    public MacVlan(EtherAddress eaddr, int vlan) {
        long mac = (eaddr == null) ? UNDEFINED : eaddr.getAddress();
        encodedValue = encode(mac, vlan);
    }

    /**
     * Construct a new instance.
     *
     * @param vh  A {@link VlanHost} instance.
     */
    public MacVlan(VlanHost vh) {
        Integer vid = vh.getVlanId().getValue();
        EtherAddress eaddr = EtherAddress.create(vh.getMacAddress());
        long addr = (eaddr == null) ? UNDEFINED : eaddr.getAddress();
        encodedValue = encode(addr, vid.intValue());
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor is used to create a new instance for MAC mapping
     *   configuration.
     * </p>
     *
     * @param vhd  A {@link VlanHostDesc} instance.
     */
    public MacVlan(VlanHostDesc vhd) {
        initialize(vhd.getValue());
    }

    /**
     * Construct a new instance.
     *
     * @param value  A string which represents a pair of MAC address and a
     *               VLAN ID. The given string must consist of a MAC address
     *               and VLAN ID (decimal) joined with "@".
     *               An empty MAC address is treated as MAC address is not
     *               specified.
     */
    public MacVlan(String value) {
        initialize(value);
    }

    /**
     * Construct a new instance.
     *
     * @param mac   A MAC address.
     *              {@code null} is treated as undefined value.
     * @param vlan  VLAN ID. Only lower 12 bits in the value is used.
     */
    public MacVlan(MacAddress mac, int vlan) {
        EtherAddress eaddr = EtherAddress.create(mac);
        long addr = (eaddr == null) ? UNDEFINED : eaddr.getAddress();
        encodedValue = encode(addr, vlan);
    }

    /**
     * Construct a new instance.
     *
     * @param mac  A byte array which represents a MAC address.
     *             {@code null} and all-zeroed byte array are treated as
     *             undefined value.
     * @param vlan  VLAN ID. Only lower 12 bits in the value is used.
     */
    public MacVlan(byte[] mac, int vlan) {
        this((mac == null) ? UNDEFINED : EtherAddress.toLong(mac), vlan);
    }

    /**
     * Return a long value which represents a MAC address.
     *
     * @return  A long value which represent a MAC address.
     *          {@link #UNDEFINED} is returned if no MAC address is configured
     *          in this instance.
     */
    public long getAddress() {
        return (encodedValue >>> NBITS_VLAN_ID);
    }

    /**
     * Return a MD-SAL MAC address.
     *
     * @return  A {@link MacAddress} instance.
     *          Note that {@code null} is returned if no MAC address is
     *          configured in this instance.
     */
    public MacAddress getMacAddress() {
        long mac = getAddress();
        if (mac == UNDEFINED) {
            return null;
        }

        EtherAddress eaddr = new EtherAddress(mac);
        return eaddr.getMacAddress();
    }

    /**
     * Return a MAC adderss as an {@link EtherAddress} instance.
     *
     * @return  An {@link EtherAddress} instance.
     *          Note that {@code null} is returned if no MAC address is
     *          configured in this instance.
     */
    public EtherAddress getEtherAddress() {
        long mac = getAddress();
        return (mac == UNDEFINED) ? null : new EtherAddress(mac);
    }

    /**
     * Return a VLAN ID.
     *
     * @return  VLAN ID.
     */
    public int getVlanId() {
        return (int)(encodedValue & MASK_VLAN_ID);
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
     * Return a {@link DataFlowSource} instance which represents this instance.
     *
     * @return  A {@link DataFlowSource} instance.
     */
    public DataFlowSource getDataFlowSource() {
        VlanId vid = new VlanId(Integer.valueOf(getVlanId()));
        return new DataFlowSourceBuilder().
            setMacAddress(getMacAddress()).
            setVlanId(vid).
            build();
    }

    /**
     * Return a {@link SourceHostFlowsKey} instance which represents this
     * instance.
     *
     * @return  A {@link SourceHostFlowsKey} instance.
     *          Note that {@code null} is returned if no MAC address is
     *          configured in this instance.
     */
    public SourceHostFlowsKey getSourceHostFlowsKey() {
        VlanId vid = new VlanId(Integer.valueOf(getVlanId()));
        MacAddress mac = getMacAddress();
        return (mac == null) ? null : new SourceHostFlowsKey(mac, vid);
    }

    /**
     * Return a {@link VlanHostDesc} instance which represents this instance.
     *
     * @return  A {@link VlanHostDesc} instance.
     */
    public VlanHostDesc getVlanHostDesc() {
        return new VlanHostDesc(toString());
    }

    /**
     * Return a {@link VlanHostDescList} instance which represents this
     * instance.
     *
     * @return  A {@link VlanHostDescList} instance.
     */
    public VlanHostDescList getVlanHostDescList() {
        return new VlanHostDescListBuilder().
            setHost(getVlanHostDesc()).
            build();
    }

    /**
     * Initialize this instance using the given string.
     *
     * @param value  A string which represents a pair of MAC address and a
     *               VLAN ID. The given string must consist of a MAC address
     *               and VLAN ID (decimal) joined with "@".
     *               An empty MAC address is treated as MAC address is not
     *               specified.
     */
    private void initialize(String value) {
        VlanDescParser parser = new VlanDescParser(value, DESC_VLAN_HOST_DESC);
        String mac = parser.getIdentifier();
        long addr;
        if (mac == null) {
            addr = UNDEFINED;
        } else {
            EtherAddress eaddr = new EtherAddress(mac);
            addr = eaddr.getAddress();
        }

        stringCache = value.toLowerCase(Locale.ENGLISH);
        encodedValue = encode(addr, parser.getVlanId());
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
    private long encode(long mac, int vlan) {
        return (((mac << NBITS_VLAN_ID) | ((long)vlan & MASK_VLAN_ID))
                & MASK_ENCODED);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            MacVlan mvlan = (MacVlan)o;
            ret = (encodedValue == mvlan.encodedValue);
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = hash;
        if (h == 0) {
            h = MacVlan.class.hashCode() + NumberUtils.hashCode(encodedValue);
            hash = h;
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        String value = stringCache;
        if (value == null) {
            StringBuilder builder = new StringBuilder();
            long mac = getAddress();
            if (mac != UNDEFINED) {
                EtherAddress eaddr = new EtherAddress(mac);
                builder.append(eaddr.getText());
            }

            value = builder.append(VlanDescParser.SEPARATOR).
                append(getVlanId()).toString();
            stringCache = value;
        }

        return value;
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
