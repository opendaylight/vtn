/*
 * Copyright (c) 2013, 2016 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;

import java.util.Map;

/**
 * {@code UDP} describes an UDP packet.
 *
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public final class UDP extends PortPacket<UDP> {
    /**
     * The number of bits in the UDP header.
     */
    private static final int  HEADER_SIZE = 64;

    /**
     * The field name that indicates the packet length.
     */
    private static final String  LENGTH = "Length";

    /**
     * The field name that indicates the UDP checksum.
     */
    private static final String  CHECKSUM = "Checksum";

    /**
     * A map that determines the UDP packet header format.
     */
    private static final Map<String, HeaderField>  HEADER_FORMAT;

    /**
     * Initialize static fields.
     */
    static {
        HEADER_FORMAT = new HeaderMapBuilder().
            addNumber(SRCPORT, Short.SIZE).
            addNumber(DESTPORT, Short.SIZE).
            addNumber(LENGTH, Short.SIZE).
            addNumber(CHECKSUM, Short.SIZE).
            build();
    }

    /**
     * Gets the stored length of UDP packet.
     *
     * @return  The number of bytes in the UDP datagram.
     */
    public short getLength() {
        return getShort(LENGTH);
    }

    /**
     * Get the stored checksum value of the UDP packet.
     *
     * @return  The UDP checksum.
     */
    public short getChecksum() {
        return getShort(CHECKSUM);
    }

    /**
     * Set the UDP header length value for the current UDP object instance.
     *
     * @param len  The number of bytes in the UDP datagram.
     * @return  This instance.
     */
    public UDP setLength(short len) {
        getHeaderFieldMap().put(LENGTH, toBytes(len));
        return this;
    }

    /**
     * Set the checksum for the current UDP object instance.
     *
     * @param cksum  The UDP checksum.
     * @return  This instance.
     */
    public UDP setChecksum(short cksum) {
        getHeaderFieldMap().put(CHECKSUM, toBytes(cksum));
        return this;
    }

    // Packet

    /**
     * Gets the header size in bits.
     *
     * @return  The UDP header size in bits.
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
