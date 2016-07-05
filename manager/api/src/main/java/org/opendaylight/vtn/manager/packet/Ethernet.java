/*
 * Copyright (c) 2013, 2016 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.packet.EtherTypePacket.ETHT;

import java.util.Map;

import org.opendaylight.vtn.manager.util.EtherAddress;

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
public class Ethernet extends EtherTypePacket<Ethernet> {
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
     * A map that determines the IPv4 packet header format.
     */
    private static final Map<String, HeaderField>  HEADER_FORMAT;

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
}
