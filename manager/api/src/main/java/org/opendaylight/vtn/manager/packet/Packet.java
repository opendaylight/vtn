/*
 * Copyright (c) 2013, 2015 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;
import static org.opendaylight.vtn.manager.util.NumberUtils.toInteger;
import static org.opendaylight.vtn.manager.util.NumberUtils.toShort;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.collect.ImmutableMap;

import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.Ip4Network;

/**
 * {@code Packet} is an abstract class which represents the generic network
 * packet object.
 *
 * <p>
 *   It provides the basic methods which are common for all the packets,
 *   like serialize and deserialize.
 * </p>
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public abstract class Packet implements Cloneable {
    /**
     * Logger instance.
     */
    private static final Logger  LOG = LoggerFactory.getLogger(Packet.class);

    /**
     * Determine whether this packet is corrupted or not.
     */
    private boolean  corrupted;

    /**
     * The packet encapsulated by this packet.
     */
    private Packet  payload;

    /**
     * The unparsed raw payload carried by this packet.
     */
    private byte[]  rawPayload;

    /**
     * Header fields values.
     */
    private Map<String, byte[]>  hdrFieldsMap = new HashMap<>();

    /**
     * The class of the encapsulated packet object.
     */
    private Class<? extends Packet>  payloadClass;

    /**
     * {@code HeaderField} describes the location of the header field in the
     * packet header.
     */
    protected static final class HeaderField {
        /**
         * The start bit offset for the header field.
         */
        private final int  offset;

        /**
         * The number of bits in the header field.
         */
        private final int  numBits;

        /**
         * The number of bytes in the buffer used to keep the value.
         */
        private final int  size;

        /**
         * Construct a new instance.
         *
         * @param off     The start bit offset for the header field.
         * @param nbits   The number of bits in the header field.
         * @param nbytes  The number of bytes in the buffer used to keep
         *                the value.
         */
        private HeaderField(int off, int nbits, int nbytes) {
            offset = off;
            numBits = nbits;
            size = nbytes;
        }

        /**
         * Return the start bit offset for the header field.
         *
         * @return  The start offset bit for the header field.
         */
        int getOffset() {
            return offset;
        }

        /**
         * Return the number of bits in the header field.
         *
         * @return  The number of bits in the header field.
         */
        int getNumBits() {
            return numBits;
        }

        /**
         * Return the size of the buffer for the header field.
         *
         * @return  The number of bytes in the buffer used to keep the value.
         */
        int getSize() {
            return size;
        }

        /**
         * Return the hash code of the given value.
         *
         * @param value  A value associated with this header field.
         * @return  The hash code of the given value.
         */
        int hash(byte[] value) {
            byte[] v = value;
            assert v == null || v.length == size;

            if (v == null && size != 0) {
                // Compute the hash code of the default value.
                v = new byte[size];
            }

            return Arrays.hashCode(v);
        }
    }

    /**
     * Internal utility class to generate header format map.
     *
     * <p>
     *   The order of the header fields iteration is the order in which its
     *   entries were added.
     * </p>
     */
    static final class HeaderMapBuilder {
        /**
         * A immutable map builder.
         */
        private final ImmutableMap.Builder<String, HeaderField> builder =
            ImmutableMap.<String, HeaderField>builder();

        /**
         * The bit offset fot the next field.
         */
        private int  offset;

        /**
         * Add a new header field that keeps a byte array.
         *
         * @param name   The name of the header field.
         * @param nbits  The number of bits in the header field.
         * @return  This instance.
         */
        HeaderMapBuilder addByte(String name, int nbits) {
            int size = (nbits + Byte.SIZE - 1) / Byte.SIZE;
            return add(name, nbits, size);
        }

        /**
         * Add a new header field that keeps a number.
         *
         * @param name   The name of the header field.
         * @param nbits  The number of bits in the header field.
         * @return  This instance.
         */
        HeaderMapBuilder addNumber(String name, int nbits) {
            int size = (nbits + Byte.SIZE - 1) / Byte.SIZE;
            if (size != 0) {
                // Round up the size to a power of 2.
                int leading = Integer.numberOfLeadingZeros(size);
                int trailing = Integer.numberOfTrailingZeros(size);
                if (leading + trailing < Integer.SIZE - 1) {
                    size = 1 << (Integer.SIZE - leading);
                }
            }

            return add(name, nbits, size);
        }

        /**
         * Add a new header field.
         *
         * @param name   The name of the header field.
         * @param nbits  The number of bits in the header field.
         * @param size   The number of bytes in the buffer used to keep the
         *               value.
         * @return  This instance.
         */
        private HeaderMapBuilder add(String name, int nbits, int size) {
            HeaderField field = new HeaderField(offset, nbits, size);
            builder.put(name, field);
            offset += nbits;
            return this;
        }

        /**
         * Create a new header field map.
         *
         * @return  A map that determines the format of the header fields.
         */
        Map<String, HeaderField> build() {
            return builder.build();
        }
    }

    /**
     * Construct a new instance.
     */
    Packet() {
    }

    /**
     * Return the packet encapsulated by this packet.
     *
     * @return  The packet encapsulated by this packet if present.
     *          {@code null} if not present.
     */
    public final Packet getPayload() {
        return payload;
    }

    /**
     * Set the packet encapsulated by this packet.
     *
     * @param p  The packet encapsulated by this packet.
     */
    public void setPayload(Packet p) {
        payload = p;
    }

    /**
     * This method deserializes the data bits obtained from the wire into the
     * respective header and payload which are of type Packet.
     *
     * @param data       Data from wire to deserialize.
     * @param bitOffset  Bit position where packet header starts in data array.
     * @param size       Size of packet in bits.
     * @return  This instance.
     * @throws PacketException  An error occurred.
     */
    public Packet deserialize(byte[] data, int bitOffset, int size)
        throws PacketException {
        // Deserialize the header fields one by one.
        int startOffset = 0;
        int numBits = 0;
        payloadClass = null;
        Map<String, HeaderField> fmtMap = getHeaderFormat();
        for (Entry<String, HeaderField> entry: fmtMap.entrySet()) {
            String hdrField = entry.getKey();
            HeaderField hent = entry.getValue();
            startOffset = bitOffset + getFieldOffset(hdrField, hent);
            numBits = getFieldNumBits(hdrField, hent);

            byte[] hdrFieldBytes;
            try {
                hdrFieldBytes = ByteUtils.getBits(data, startOffset, numBits);
            } catch (RuntimeException e) {
                String msg = "Failed to deserialize field: " + hdrField +
                    ": off=" + startOffset + ", size=" + numBits +
                    ", total=" + size;
                throw new PacketException(msg, e);
            }

            /*
             * Store the raw read value, checks the payload type and set the
             * payloadClass accordingly
             */
            setHeaderField(hdrField, hdrFieldBytes);

            if (LOG.isTraceEnabled()) {
                LOG.trace("Deserializing: {}: {}: {} (offset {} bitsize {})",
                          getClass().getSimpleName(), hdrField,
                          ByteUtils.toHexString(hdrFieldBytes),
                          startOffset, numBits);
            }
        }

        // Deserialize the payload now
        int payloadStart = startOffset + numBits;
        int payloadSize = data.length * Byte.SIZE - payloadStart;

        if (payloadClass != null) {
            try {
                payload = payloadClass.newInstance();
            } catch (Exception e) {
                throw new PacketException(
                    "Error parsing payload for Ethernet packet", e);
            }
            payload.deserialize(data, payloadStart, payloadSize);
        } else if (payloadSize > 0) {
            /*
             *  The payload class was not set, it means no class for parsing
             *  this payload is present. Let's store the raw payload if any.
             */
            int start = payloadStart / Byte.SIZE;
            int stop = start + payloadSize / Byte.SIZE;
            rawPayload = Arrays.copyOfRange(data, start, stop);
        }

        // Take care of computation that can be done only after deserialization
        postDeserializeCustomOperation(data, payloadStart - getHeaderSize());

        return this;
    }

    /**
     * This method serializes the header and payload from the respective
     * packet class, into a single stream of bytes to be sent on the wire.
     *
     * @return  The byte array representing the serialized Packet.
     * @throws PacketException  An error occurred.
     */
    public byte[] serialize() throws PacketException {
        // Acquire or compute the serialized payload
        byte[] payloadBytes = null;
        if (payload != null) {
            payloadBytes = payload.serialize();
        } else if (rawPayload != null) {
            payloadBytes = rawPayload;
        }

        int payloadSize = (payloadBytes == null) ? 0 : payloadBytes.length;

        // Allocate the buffer to contain the full (header + payload) packet
        int headerSize = getHeaderSize() / Byte.SIZE;
        byte[] packetBytes = new byte[headerSize + payloadSize];
        if (payloadSize != 0) {
            System.arraycopy(payloadBytes, 0, packetBytes, headerSize,
                             payloadSize);
        }

        // Serialize this packet header, field by field
        Map<String, HeaderField> fmtMap = getHeaderFormat();
        for (Entry<String, HeaderField> entry: fmtMap.entrySet()) {
            String field = entry.getKey();
            HeaderField hent = entry.getValue();
            byte[] fieldBytes = hdrFieldsMap.get(field);
            // Let's skip optional fields when not set
            if (fieldBytes != null) {
                int off = getFieldOffset(field, hent);
                int nbits = getFieldNumBits(field, hent);
                try {
                    ByteUtils.setBits(packetBytes, fieldBytes, off, nbits);
                } catch (RuntimeException e) {
                    String msg = "Failed to serialize field: " + field +
                        ": off=" + off + ", size=" + nbits +
                        ", total=" + fieldBytes.length;
                    throw new PacketException(msg, e);
                }
            }
        }

        // Perform post serialize operations (like checksum computation)
        postSerializeCustomOperation(packetBytes);

        if (LOG.isTraceEnabled()) {
            LOG.trace("Serialized: {}: {}", getClass().getSimpleName(),
                      ByteUtils.toHexString(packetBytes));
        }

        return packetBytes;
    }

    /**
     * This method gets called at the end of the serialization process.
     *
     * It is intended for the child packets to insert some custom data into the
     * output byte stream which cannot be done or cannot be done efficiently
     * during the normal Packet.serialize() path. An example is the checksum
     * computation for IPv4.
     *
     * @param myBytes  Serialized bytes.
     * @throws PacketException  An error occurred.
     */
    protected void postSerializeCustomOperation(byte[] myBytes)
        throws PacketException {
    }

    /**
     * This method re-computes the checksum of the bits received on the wire
     * and validates it with the checksum in the bits received.
     *
     * Since the computation of checksum varies based on the protocol,
     * this method is overridden. Currently only IPv4 and ICMP do checksum
     * computation and validation. TCP and UDP need to implement these if
     * required.
     *
     * @param data   The byte stream representing the Ethernet frame.
     * @param offset The bit offset from where the byte array corresponding to
     *               this Packet starts in the frame
     * @throws PacketException  An error occurred.
     */
    protected void postDeserializeCustomOperation(byte[] data, int offset)
        throws PacketException {
    }

    /**
     * Set a class that specifies the type of packets encapsulated by this
     * packet.
     *
     * @param cls  A class that specifies the type of the payload packet.
     */
    protected final void setPayloadClass(Class<? extends Packet> cls) {
        payloadClass = cls;
    }

    /**
     * Associate the specified value with the specified header field.
     *
     * @param name   The name of the header field.
     * @param value  The value to be associated with the specified header
     *               field. {@code null} cannot be specified.
     */
    protected void setHeaderField(String name, byte[] value) {
        hdrFieldsMap.put(name, value);
    }

    /**
     * Return a map that keeps packet header fields.
     *
     * @return  A map that keeps packet header fields.
     */
    protected final Map<String, byte[]> getHeaderFieldMap() {
        return hdrFieldsMap;
    }

    /**
     * Gets the header length in bits.
     *
     * @return  The header length in bits.
     */
    public int getHeaderSize() {
        int size = 0;

        /*
         * We need to iterate over the fields that were read in the frame
         * (hdrFieldsMap) not all the possible ones described in the map
         * returned by getHeaderFormat().
         * For ex, 802.1Q may or may not be there
         */
        Map<String, HeaderField> fmtMap = getHeaderFormat();
        for (Entry<String, byte[]> fieldEntry: hdrFieldsMap.entrySet()) {
            if (fieldEntry.getValue() != null) {
                String field = fieldEntry.getKey();
                size += getFieldNumBits(field, fmtMap.get(field));
            }
        }
        return size;
    }

    /**
     * This method fetches the start bit offset for header field specified by
     * {@code name}.
     *
     * @param name   The name of the header field.
     * @return  The offset of the requested field.
     */
    public final int getFieldOffset(String name) {
        return getFieldOffset(name, getHeaderFormat().get(name));
    }

    /**
     * This method fetches the number of bits for header field specified by
     * {@code name}.
     *
     * @param name  The name of the header field.
     * @return  The number of bits of the requested field.
     */
    public final int getFieldNumBits(String name) {
        return getFieldNumBits(name, getHeaderFormat().get(name));
    }

    /**
     * Returns the raw payload carried by this packet in case payload was not
     * parsed. Caller can call this function in case the getPaylod() returns
     * {@code null}.
     *
     * @return  The raw payload if not parsable as an array of bytes.
     *         {@code null} otherwise
     */
    public byte[] getRawPayload() {
        return (rawPayload == null) ? null : rawPayload.clone();
    }

    /**
     * Set a raw payload in the packet class.
     *
     * @param bytes  The raw payload as byte array.
     */
    public void setRawPayload(byte[] bytes) {
        rawPayload = (bytes == null || bytes.length == 0)
            ? null : bytes.clone();
    }

    /**
     * Return whether the deserialized packet is to be considered corrupted.
     * This is the case when the checksum computed after reconstructing the
     * packet received from wire is not equal to the checksum read from the
     * stream. For the Packet class which do not have a checksum field, this
     * function will always return false.
     *
     * @return true if the deserialized packet's recomputed checksum is not
     *         equal to the packet carried checksum
     */
    public final boolean isCorrupted() {
        return corrupted;
    }

    /**
     * Set a boolean value which determines whether this packet is corrupted
     * or not.
     *
     * @param c  {@code true} indicates that this packet is corrupted.
     */
    protected final void setCorrupted(boolean c) {
        corrupted = c;
    }

    /**
     * Return the size of the raw payload in this packet.
     *
     * @return  The number of bytes in the raw payload.
     */
    protected final int getRawPayloadSize() {
        return (rawPayload == null) ? 0 : rawPayload.length;
    }

    /**
     * Return the offset of the header field configured in the given instance.
     *
     * @param name   The name of the header field.
     * @param entry  The header field entry associated with {@code name}.
     * @return  The offset of the requested field.
     */
    protected int getFieldOffset(String name, HeaderField entry) {
        return entry.getOffset();
    }

    /**
     * Return the number of bits in the header field configured in the given
     * instance.
     *
     * @param name   The name of the header field.
     * @param entry  The header field entry associated with {@code name}.
     * @return  The number of bits of the requested field.
     */
    protected int getFieldNumBits(String name, HeaderField entry) {
        return entry.getNumBits();
    }

    /**
     * Compare the given header field values.
     *
     * @param name    The name of the header field.
     * @param value1  The first header field value to be compared.
     * @param value2  The second header field value to be compared.
     * @return  {@code true} only if the given two header field values are
     *          identical.
     */
    protected boolean equalsField(String name, byte[] value1, byte[] value2) {
        boolean ret = (value1 == value2);
        if (!ret) {
            // null is identical to zero.
            byte[] v1 = value1;
            byte[] v2 = value2;
            if (v1 == null) {
                v1 = new byte[v2.length];
            } else if (v2 == null) {
                v2 = new byte[v1.length];
            }
            ret = Arrays.equals(v1, v2);
        }

        return ret;
    }

    /**
     * Return the value of the specified field as a byte array.
     *
     * @param name  The name of the field.
     * @return  A byte array.
     */
    protected final byte[] getBytes(String name) {
        byte[] value = hdrFieldsMap.get(name);
        return (value == null) ? null : value.clone();
    }

    /**
     * Return the value of the specified field as a byte.
     *
     * @param name  The name of the field.
     * @return  A byte value.
     */
    protected final byte getByte(String name) {
        byte[] value = hdrFieldsMap.get(name);
        return (value == null) ? 0 : value[0];
    }

    /**
     * Return the value of the specified field as an integer.
     *
     * @param name  The name of the field.
     * @return  An integer value.
     */
    protected final int getInt(String name) {
        byte[] value = hdrFieldsMap.get(name);
        return (value == null) ? 0 : toInteger(value);
    }

    /**
     * Return the value of the specified field as a short integer.
     *
     * @param name  The name of the field.
     * @return  A short integer value.
     */
    protected final short getShort(String name) {
        byte[] value = hdrFieldsMap.get(name);
        return (value == null) ? 0 : toShort(value);
    }

    /**
     * Return the value of the specified field as an IPv4 address.
     *
     * @param name  The name of the field.
     * @return  An {@link Ip4Network} instance.
     */
    protected final Ip4Network getIp4Network(String name) {
        byte[] value = hdrFieldsMap.get(name);
        return (value == null) ? new Ip4Network(0) : new Ip4Network(value);
    }

    /**
     * Determine whether the given value represents a zero short value or not.
     *
     * @param value  A byte array that contains a short integer.
     * @return  {@code true} only if the given value represents a zero.
     */
    protected final boolean isZeroShort(byte[] value) {
        return (value == null || toShort(value) == 0);
    }

    /**
     * Return a map that determines the format of the packet.
     *
     * @return  A map that determines the format of the packet.
     */
    protected abstract Map<String, HeaderField> getHeaderFormat();

    /**
     * Compute the hash code of the header field map.
     *
     * @return  The hash code of the header field map.
     */
    private int headerHashCode() {
        int h = 0;
        Map<String, HeaderField> fmtMap = getHeaderFormat();
        for (Entry<String, HeaderField> entry: fmtMap.entrySet()) {
            String field = entry.getKey();
            HeaderField hf = entry.getValue();
            h += (field.hashCode() ^ hf.hash(hdrFieldsMap.get(field)));
        }

        return h;
    }

    /**
     * Determine whether the given header field map is equal to the map
     * configured in this instance or not.
     *
     * @param map  The header field map to be compared.
     *             Specifying {@code null} results in undefined behavior.
     * @return  {@code true} only if the given map is equal to the header
     *          field map in this instance.
     */
    private boolean headerEquals(Map<String, byte[]> map) {
        for (String field: getHeaderFormat().keySet()) {
            byte[] myData = hdrFieldsMap.get(field);
            byte[] data = map.get(field);
            if (!equalsField(field, myData, data)) {
                return false;
            }
        }

        return true;
    }

    /**
     * Create a deep copy of the header field map in this packet.
     *
     * @return  A deep copy of the header field map in this packet.
     */
    private Map<String, byte[]> headerClone() {
        Map<String, byte[]> copy = new HashMap<>();
        for (Entry<String, byte[]> entry: hdrFieldsMap.entrySet()) {
            String field = entry.getKey();
            byte[] data = entry.getValue();
            copy.put(field, data.clone());
        }

        return copy;
    }

    // Object

    /**
     * Return the hash code of this object.
     *
     * <p>
     *   Note that this method never uses the payload to compute the hash code.
     * </p>
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return getClass().hashCode() * HASH_PRIME + headerHashCode();
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   Note that this method never compares the payload encapsulated by
     *   the packet.
     * </p>
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            Packet pkt = (Packet)o;
            ret = headerEquals(pkt.hdrFieldsMap);
        }

        return ret;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder().
            append(getClass().getSimpleName()).append('{');
        String sep = "";
        for (String field: getHeaderFormat().keySet()) {
            byte[] value = hdrFieldsMap.get(field);
            builder.append(sep).append(field).append('=').
                append(ByteUtils.toHexString(value));
            sep = ", ";
        }

        return builder.append('}').toString();
    }

    /**
     * Return a deep copy of this packet.
     *
     * @return  A deep copy of this packet.
     */
    @Override
    public Packet clone() {
        try {
            Packet copy = (Packet)super.clone();
            copy.hdrFieldsMap = headerClone();

            // Copy the payload.
            // Note that we don't need to copy raw payload because it is
            // never modified.
            if (payload != null) {
                copy.payload = payload.clone();
            }

            return copy;
        } catch (CloneNotSupportedException e) {
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
