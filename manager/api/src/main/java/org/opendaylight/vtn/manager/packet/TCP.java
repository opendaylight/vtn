/*
 * Copyright (c) 2013, 2015 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;

import java.util.Map;

/**
 * {@code TCP} describes an TCP packet.
 *
 * <p>
 *   TCP option is not supported. TCP option is treated as a part of TCP
 *   payload.e
 * </p>
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public final class TCP extends Packet {
    /**
     * The number of bits in the TCP header, excluding options.
     */
    private static final int  HEADER_SIZE = 160;

    /**
     * The field name that indicates the source port number.
     */
    private static final String  SRCPORT = "SourcePort";

    /**
     * The field name that indicates the destination port number.
     */
    private static final String  DESTPORT = "DestinationPort";

    /**
     * The field name that indicates the TCP sequence number.
     */
    private static final String  SEQNUMBER = "SequenceNumber";

    /**
     * The field name that indicates the TCP acknowledgement number.
     */
    private static final String  ACKNUMBER = "AcknowledgementNumber";

    /**
     * The field name that indicates the data offset.
     */
    private static final String  DATAOFFSET = "DataOffset";

    /**
     * The field name that indicates the reserved field.
     */
    private static final String  RESERVED = "Reserved";

    /**
     * The field name that indicates the header length and TCP flags.
     */
    private static final String  HEADERLENFLAGS = "HeaderLenFlags";

    /**
     * The field name that indicates the TCP window size.
     */
    private static final String  WINDOWSIZE = "WindowSize";

    /**
     * The field name that indicates the TCP checksum.
     */
    private static final String  CHECKSUM = "Checksum";

    /**
     * The field name that indicates the urgent pointer.
     */
    private static final String  URGENTPOINTER = "UrgentPointer";

    /**
     * The number of bits in the data offset field.
     */
    private static final int  BITS_DATAOFFSET = 4;

    /**
     * The number of bits in the reserved field.
     */
    private static final int  BITS_RESERVED = 3;

    /**
     * The number of bits in the header length and TCP flags field.
     */
    private static final int  BITS_HEADERLENFLAGS = 9;

    /**
     * A map that determines the TCP packet header format.
     */
    private static final Map<String, HeaderField>  HEADER_FORMAT;

    /**
     * Initialize static fields.
     */
    static {
        HEADER_FORMAT = new HeaderMapBuilder().
            addNumber(SRCPORT, Short.SIZE).
            addNumber(DESTPORT, Short.SIZE).
            addNumber(SEQNUMBER, Integer.SIZE).
            addNumber(ACKNUMBER, Integer.SIZE).
            addNumber(DATAOFFSET, BITS_DATAOFFSET).
            addNumber(RESERVED, BITS_RESERVED).
            addNumber(HEADERLENFLAGS, BITS_HEADERLENFLAGS).
            addNumber(WINDOWSIZE, Short.SIZE).
            addNumber(CHECKSUM, Short.SIZE).
            addNumber(URGENTPOINTER, Short.SIZE).
            build();
    }

    /**
     * Sets the TCP source port for the current TCP object instance.
     *
     * @param port  The source port number.
     * @return  This instance.
     */
    public TCP setSourcePort(short port) {
        getHeaderFieldMap().put(SRCPORT, toBytes(port));
        return this;
    }

    /**
     * Sets the TCP destination port for the current TCP object instance.
     *
     * @param port  The destination port number.
     * @return  This instance.
     */
    public TCP setDestinationPort(short port) {
        getHeaderFieldMap().put(DESTPORT, toBytes(port));
        return this;
    }

    /**
     * Sets the TCP sequence number for the current TCP object instance.
     *
     * @param seq  The TCP sequence number.
     * @return  This instance.
     */
    public TCP setSequenceNumber(int seq) {
        getHeaderFieldMap().put(SEQNUMBER, toBytes(seq));
        return this;
    }

    /**
     * Sets the TCP data offset for the current TCP object instance.
     *
     * @param off  The TCP data offset.
     * @return  This instance.
     */
    public TCP setDataOffset(byte off) {
        getHeaderFieldMap().put(DATAOFFSET, new byte[]{off});
        return this;
    }

    /**
     * Sets the TCP reserved bits for the current TCP object instance.
     *
     * @param resv  The reserved field value.
     * @return  This instance.
     */
    public TCP setReserved(byte resv) {
        getHeaderFieldMap().put(RESERVED, new byte[]{resv});
        return this;
    }

    /**
     * Sets the TCP Ack number for the current TCP object instance.
     *
     * @param acq  The TCP acknowledgement number.
     * @return  This instance.
     */
    public TCP setAckNumber(int acq) {
        getHeaderFieldMap().put(ACKNUMBER, toBytes(acq));
        return this;
    }

    /**
     * Sets the TCP flags for the current TCP object instance.
     *
     * @param flags  The TCP flags.
     * @return  This instance.
     */
    public TCP setHeaderLenFlags(short flags) {
        getHeaderFieldMap().put(HEADERLENFLAGS, toBytes(flags));
        return this;
    }

    /**
     * Sets the TCP window size for the current TCP object instance.
     *
     * @param wsize  The TCP window size.
     * @return  This instance.
     */
    public TCP setWindowSize(short wsize) {
        getHeaderFieldMap().put(WINDOWSIZE, toBytes(wsize));
        return this;
    }

    /**
     * Sets the TCP checksum for the current TCP object instance.
     *
     * @param cksum  The TCP checksum.
     * @return  This instance.
     */
    public TCP setChecksum(short cksum) {
        getHeaderFieldMap().put(CHECKSUM, toBytes(cksum));
        return this;
    }

    /**
     * Sets the TCP Urgent Pointer for the current TCP object instance.
     *
     * @param urg  The TCP urgent pointer.
     * @return  This instance.
     */
    public TCP setUrgentPointer(short urg) {
        getHeaderFieldMap().put(URGENTPOINTER, toBytes(urg));
        return this;
    }

    /**
     * Gets the stored source port value of TCP header.
     *
     * @return  The source port number.
     */
    public short getSourcePort() {
        return getShort(SRCPORT);
    }

    /**
     * Gets the stored destination port value of TCP header.
     *
     * @return  The destination port number.
     */
    public short getDestinationPort() {
        return getShort(DESTPORT);
    }

    /**
     * Return the TCP sequence number.
     *
     * @return  The TCP sequence number.
     */
    public int getSequenceNumber() {
        return getInt(SEQNUMBER);
    }

    /**
     * Return the TCP data offset.
     *
     * @return  The TCP data offset.
     */
    public byte getDataOffset() {
        return getByte(DATAOFFSET);
    }

    /**
     * Return the reserved field value.
     *
     * @return  The value configured in the reserved field.
     */
    public byte getReserved() {
        return getByte(RESERVED);
    }

    /**
     * Return the TCP acknowledgement number.
     *
     * @return  The TCP acknowledgement number.
     */
    public int getAckNumber() {
        return getInt(ACKNUMBER);
    }

    /**
     * Return the TCP header length and flags.
     *
     * @return  The value of TCP header length and flags.
     */
    public short getHeaderLenFlags() {
        return getShort(HEADERLENFLAGS);
    }

    /**
     * Return the TCP window size.
     *
     * @return  The TCP window size.
     */
    public short getWindowSize() {
        return getShort(WINDOWSIZE);
    }

    /**
     * Get the stored checksum value of the TCP header.
     *
     * @return  The TCP checksum.
     */
    public short getChecksum() {
        return getShort(CHECKSUM);
    }

    /**
     * Return the TCP urgent pointer.
     *
     * @return  The TCP urgent pointer.
     */
    public short getUrgentPointer() {
        return getShort(URGENTPOINTER);
    }

    // Packet

    /**
     * Gets the header size in bits.
     *
     * @return The TCP header size in bits.
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

    // Object

    /**
     * {@inheritDoc}
     */
    @Override
    public TCP clone() {
        return (TCP)super.clone();
    }
}
