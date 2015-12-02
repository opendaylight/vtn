/*
 * Copyright (c) 2013, 2015 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;
import static org.opendaylight.vtn.manager.util.NumberUtils.toShort;

import java.util.EnumMap;
import java.util.Map;

import com.google.common.collect.ImmutableMap;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;

/**
 * {@code Ethernet} describes an Ethernet frame.
 *
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public class Ethernet extends Packet {
    /**
     * The number of bits in the Ethernet header.
     */
    private static final int  HEADER_SIZE = 112;

    /**
     * The field name that indicates the destination MAC address.
     */
    private static final String  DMAC = "DestinationMACAddress";

    /**
     * The field name that indicates the source MAC address.
     */
    private static final String  SMAC = "SourceMACAddress";

    /**
     * The field name that indicates the Ethernet type.
     */
    static final String  ETHT = "EtherType";

    /**
     * A map that determines the IPv4 packet header format.
     */
    private static final Map<String, HeaderField>  HEADER_FORMAT;

    /**
     * A set of supported payload types.
     */
    private static final Map<EtherTypes, Class<? extends Packet>> PAYLOAD_TYPES;

    /**
     * Initialize static fields.
     */
    static {
        // Initialize the Ethernet header format.
        int addrSize = EtherAddress.SIZE * Byte.SIZE;
        HEADER_FORMAT = new HeaderMapBuilder().
            addByte(DMAC, addrSize).
            addByte(SMAC, addrSize).
            addNumber(ETHT, Short.SIZE).
            build();

        // Initialize the payload types.
        EnumMap<EtherTypes, Class<? extends Packet>> typeMap =
            new EnumMap<>(EtherTypes.class);
        typeMap.put(EtherTypes.IPV4, IPv4.class);
        typeMap.put(EtherTypes.ARP, ARP.class);
        typeMap.put(EtherTypes.VLAN, IEEE8021Q.class);
        PAYLOAD_TYPES = ImmutableMap.copyOf(typeMap);
    }

    /**
     * Determine the payload type for the given Ethernet type.
     *
     * @param type  The Ethernet type value.
     * @return  A class for the payload type.
     *          {@code null} if no payload type is defined for the given
     *          Ethernet type.
     */
    static Class<? extends Packet> getPayloadClass(short type) {
        EtherTypes etype = EtherTypes.forValue(type);
        return (etype == null) ? null : PAYLOAD_TYPES.get(etype);
    }

    /**
     * Gets the destination MAC address stored.
     *
     * @return  The destination MAC address.
     *          {@code null} if not configured.
     */
    public byte[] getDestinationMACAddress() {
        return getHeaderFieldMap().get(DMAC);
    }

    /**
     * Gets the source MAC address stored.
     *
     * @return  The source MAC address.
     *          {@code null} if not configured.
     */
    public byte[] getSourceMACAddress() {
        return getHeaderFieldMap().get(SMAC);
    }

    /**
     * Gets the etherType stored.
     *
     * @return  The Ethernet type.
     */
    public short getEtherType() {
        return getShort(ETHT);
    }

    /**
     * Sets the destination MAC address for the current Ethernet object.
     *
     * @param mac  The destination MAC address.
     *             {@code null} cannot be specified.
     * @return  This instance.
     */
    public Ethernet setDestinationMACAddress(byte[] mac) {
        getHeaderFieldMap().put(DMAC, mac.clone());
        return this;
    }

    /**
     * Sets the source MAC address for the current Ethernet object.
     *
     * @param mac  The source MAC address.
     *             {@code null} cannot be specified.
     * @return  This instance.
     */
    public Ethernet setSourceMACAddress(byte[] mac) {
        getHeaderFieldMap().put(SMAC, mac.clone());
        return this;
    }

    /**
     * Sets the etherType for the current Ethernet object.
     *
     * @param type  The Ethernet type.
     * @return  This instance.
     */
    public Ethernet setEtherType(short type) {
        getHeaderFieldMap().put(ETHT, toBytes(type));
        return this;
    }

    // Packet

    /**
     * Return the number of bits in the Ethernet header.
     *
     * @return  The number of bits in the Ethernet header.
     */
    @Override
    public int getHeaderSize() {
        return HEADER_SIZE;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Map<String, HeaderField> getHeaderFormat() {
        return HEADER_FORMAT;
    }

    /**
     * Stores the value of fields read from data stream.
     *
     * @param name   The name of the header field.
     * @param value  The value to be associated with the specified header
     *               field. {@code null} cannot be specified.
     */
    @Override
    protected void setHeaderField(String name, byte[] value) {
        if (name.equals(ETHT)) {
            short etype = toShort(value);
            setPayloadClass(getPayloadClass(etype));
        }

        super.setHeaderField(name, value);
    }

    // Object

    /**
     * {@inheritDoc}
     */
    @Override
    public Ethernet clone() {
        return (Ethernet)super.clone();
    }
}
