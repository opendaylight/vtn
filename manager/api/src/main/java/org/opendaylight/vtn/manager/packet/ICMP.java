/*
 * Copyright (c) 2013, 2015 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.packet.IPv4.CKSUM_BYTES;
import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_BYTE;
import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_SHORT;
import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;

import java.util.Map;

import org.opendaylight.vtn.manager.util.ByteUtils;

/**
 * {@code ICMP} describes an ICMP packet.
 *
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public final class ICMP extends Packet {
    /**
     * The number of bits in the ICMP header.
     */
    private static final int  HEADER_SIZE = 64;

    /**
     * The field name that indicates the ICMP type.
     */
    private static final String  TYPE = "Type";

    /**
     * The field name that indicates the ICMP code.
     */
    private static final String  CODE = "Code";

    /**
     * The field name that indicates the checksum.
     */
    private static final String  CHECKSUM = "Checksum";

    /**
     * The field name that indicates the ICMP identifier.
     */
    private static final String  IDENTIFIER = "Identifier";

    /**
     * The field name that indicates the sequence number.
     */
    private static final String  SEQNUMBER = "SequenceNumber";

    /**
     * A map that determines the ICMP packet header format.
     */
    private static final Map<String, HeaderField>  HEADER_FORMAT;

    /**
     * Initialize static fields.
     */
    static {
        HEADER_FORMAT = new HeaderMapBuilder().
            addNumber(TYPE, Byte.SIZE).
            addNumber(CODE, Byte.SIZE).
            addNumber(CHECKSUM, Short.SIZE).
            addNumber(IDENTIFIER, Short.SIZE).
            addNumber(SEQNUMBER, Short.SIZE).
            build();
    }

    /**
     * Sets the type for the current ICMP message.
     *
     * @param type  The ICMP message type
     * @return  This ICMP object
     */
    public ICMP setType(byte type) {
        getHeaderFieldMap().put(TYPE, new byte[]{type});
        return this;
    }

    /**
     * Returns the type field of the current ICMP packet.
     *
     * @return  The type code of the current ICMP packet
     */
    public byte getType() {
        return getByte(TYPE);
    }

    /**
     * Sets the ICMP code (type subtype) for the current ICMP object instance.
     *
     * @param code  The ICMP message type subtype
     * @return  This ICMP object
     */
    public ICMP setCode(byte code) {
        getHeaderFieldMap().put(CODE, new byte[]{code});
        return this;
    }

    /**
     * Gets the ICMP code (type subtype) for the current ICMP object instance.
     *
     * @return  The ICMP message type subtype
     */
    public byte getCode() {
        return getByte(CODE);
    }

    /**
     * Sets the ICMP checksum for the current ICMP object instance.
     *
     * @param cksum  The checksum value to be set.
     * @return  This instance.
     */
    public ICMP setChecksum(short cksum) {
        getHeaderFieldMap().put(CHECKSUM, toBytes(cksum));
        return this;
    }

    /**
     * Return the ICMP checksum in this packet.
     *
     * @return  The ICMP checksum value.
     */
    public short getChecksum() {
        return getShort(CHECKSUM);
    }

    /**
     * Sets the ICMP identifier for the current ICMP object instance.
     * @param id  The ICMP identifier to be set.
     * @return  This instance.
     */
    public ICMP setIdentifier(short id) {
        getHeaderFieldMap().put(IDENTIFIER, toBytes(id));
        return this;
    }

    /**
     * Gets the ICMP identifier of the current ICMP object instance.
     *
     * @return  The ICMP identifier.
     */
    public short getIdentifier() {
        return getShort(IDENTIFIER);
    }

    /**
     * Sets the ICMP sequence number for the current ICMP object instance.
     *
     * @param seq  The ICMP sequence number to be set.
     * @return  This instance.
     */
    public ICMP setSequenceNumber(short seq) {
        getHeaderFieldMap().put(SEQNUMBER, toBytes(seq));
        return this;
    }

    /**
     * Gets the ICMP sequence number of the current ICMP object instance.
     *
     * @return  The ICMP sequence number.
     */
    public short getSequenceNumber() {
        return getShort(SEQNUMBER);
    }

    /**
     * Computes the ICMP checksum on the serialized ICMP message.
     *
     * @param data   The data stream.
     * @param start  The byte index on the data stream from which the ICMP
     *               packet starts.
     * @return  The checksum value.
     */
    private short computeChecksum(byte[] data, int start) {
        int sum = 0;
        int wordData;
        int end = (start + HEADER_SIZE / Byte.SIZE) + getRawPayloadSize();
        int checksumStartByte = start + getFieldOffset(CHECKSUM) / Byte.SIZE;
        int even = end & ~1;

        for (int i = start; i < even; i += CKSUM_BYTES) {
            // Skip, if the current bytes are checkSum bytes
            if (i != checksumStartByte) {
                wordData = ((data[i] & MASK_BYTE) << Byte.SIZE) |
                    (data[i + 1] & MASK_BYTE);
                sum = sum + wordData;
            }
        }
        if (even < end) {
            // Add the last octet with zero padding.
            wordData = (data[even] & MASK_BYTE) << Byte.SIZE;
            sum = sum + wordData;
        }

        int carry = sum >>> Short.SIZE;
        int finalSum = (sum & MASK_SHORT) + carry;
        return (short)~((short)finalSum & MASK_SHORT);
    }

    // Packet

    /**
     * Gets the header size in bits.
     *
     * @return The ICMP header size in bits.
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
     * {@inheritDoc}
     */
    @Override
    protected void postSerializeCustomOperation(byte[] myBytes)
        throws PacketException {
        String field = CHECKSUM;
        try {
            HeaderField entry = HEADER_FORMAT.get(field);
            int off = getFieldOffset(field, entry);
            int nbits = getFieldNumBits(field, entry);
            byte[] cksum = toBytes(computeChecksum(myBytes, 0));
            ByteUtils.setBits(myBytes, cksum, off, nbits);
            getHeaderFieldMap().put(CHECKSUM, cksum);
        } catch (RuntimeException e) {
            throw new PacketException("Failed to update checksum.", e);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void postDeserializeCustomOperation(byte[] data, int offset) {
        short computed = computeChecksum(data, offset / Byte.SIZE);
        short actual = getChecksum();
        if (computed != actual) {
            setCorrupted(true);
        }
    }

    // Object

    /**
     * {@inheritDoc}
     */
    @Override
    public ICMP clone() {
        return (ICMP)super.clone();
    }
}
