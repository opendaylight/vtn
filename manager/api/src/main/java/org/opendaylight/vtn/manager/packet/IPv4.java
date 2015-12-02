/*
 * Copyright (c) 2013, 2015 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_BYTE;
import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_SHORT;
import static org.opendaylight.vtn.manager.util.NumberUtils.NUM_OCTETS_SHORT;
import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;

import java.util.Arrays;
import java.util.EnumMap;
import java.util.Map;

import com.google.common.collect.ImmutableMap;

import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.InetProtocols;
import org.opendaylight.vtn.manager.util.Ip4Network;

/**
 * {@code IPv4} describes an IP version 4 packet.
 *
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public final class IPv4 extends Packet {
    /**
     * The IPv4 version number.
     */
    public static final byte  IPV4_VERSION = 4;

    /**
     * The number of octets in IP checksum.
     */
    public static final int  CKSUM_BYTES = NUM_OCTETS_SHORT;

    /**
     * The field name that indicates the Inetnet Protocol version.
     */
    private static final String  VERSION = "Version";

    /**
     * The field name that indicates the IP header length.
     */
    private static final String  HEADERLENGTH = "HeaderLength";

    /**
     * The field name that indicates the IP differential service.
     */
    private static final String  DIFFSERV = "DiffServ";

    /**
     * The field name that indicates the ECN bits.
     */
    private static final String  ECN = "ECN";

    /**
     * The field name that indicates the total length.
     */
    private static final String  TOTLENGTH = "TotalLength";

    /**
     * The field name that indicates the IPv4 identification.
     */
    private static final String  IDENTIFICATION = "Identification";

    /**
     * The field name that indicates the IPv4 flags.
     */
    private static final String  FLAGS = "Flags";

    /**
     * The field name that indicates the IPv4 fragment offset.
     */
    private static final String  FRAGOFFSET = "FragmentOffset";

    /**
     * The field name that indicates the IPv4 Time-To-Live.
     */
    private static final String  TTL = "TTL";

    /**
     * The field name that indicates the IP protocol number.
     */
    private static final String  PROTOCOL = "Protocol";

    /**
     * The field name that indicates the IP checksum.
     */
    private static final String  CHECKSUM = "Checksum";

    /**
     * The field name that indicates the source IP address.
     */
    private static final String  SIP = "SourceIPAddress";

    /**
     * The field name that indicates the destination IP address.
     */
    private static final String  DIP = "DestinationIPAddress";

    /**
     * The field name that indicates the IPv4 options.
     */
    private static final String  OPTIONS = "Options";

    /**
     * The number of bits in the IP protocol version field.
     */
    private static final int  BITS_VERSION = 4;

    /**
     * The number of bits in the IP header length field.
     */
    private static final int  BITS_HEADERLENGTH = 4;

    /**
     * The number of bits in the IP differential service field.
     */
    private static final int  BITS_DIFFSERV = 6;

    /**
     * The number of bits in the ECN field.
     */
    private static final int  BITS_ECN = 2;

    /**
     * The number of bits in the IPv4 flags field.
     */
    private static final int  BITS_FLAGS = 3;

    /**
     * The number of bits in the IPv4 fragment offset field.
     */
    private static final int  BITS_FRAGOFFSET = 13;

    /**
     * The number of bits to calculate IPv4 unit size.
     */
    private static final int  UNIT_SIZE_SHIFT = 2;

    /**
     * The number of bytes in an IPv4 unit value.
     */
    private static final int  UNIT_SIZE = (1 << UNIT_SIZE_SHIFT);

    /**
     * The minimum size of the IPv4 header in bytes.
     */
    private static final int  MIN_HEADER_SIZE = 20;

    /**
     * The default value of the header length field.
     */
    private static final byte  DEFAULT_HEADER_LENGTH =
        (byte)(MIN_HEADER_SIZE >>> UNIT_SIZE_SHIFT);

    /**
     * The default value of the IPv4 flags.
     */
    private static final byte  DEFAULT_FLAGS = 2;

    /**
     * A map that determines the IPv4 packet header format.
     */
    private static final Map<String, HeaderField>  HEADER_FORMAT;

    /**
     * A set of supported payload types.
     */
    private static final Map<InetProtocols, Class<? extends Packet>>  PAYLOAD_TYPES;

    /**
     * Initialize static fields.
     */
    static {
        // Initialize the IPv4 header format.
        int addrSize = Ip4Network.SIZE * Byte.SIZE;
        HEADER_FORMAT = new HeaderMapBuilder().
            addNumber(VERSION, BITS_VERSION).
            addNumber(HEADERLENGTH, BITS_HEADERLENGTH).
            addNumber(DIFFSERV, BITS_DIFFSERV).
            addNumber(ECN, BITS_ECN).
            addNumber(TOTLENGTH, Short.SIZE).
            addNumber(IDENTIFICATION, Short.SIZE).
            addNumber(FLAGS, BITS_FLAGS).
            addNumber(FRAGOFFSET, BITS_FRAGOFFSET).
            addNumber(TTL, Byte.SIZE).
            addNumber(PROTOCOL, Byte.SIZE).
            addNumber(CHECKSUM, Short.SIZE).
            addByte(SIP, addrSize).
            addByte(DIP, addrSize).
            addByte(OPTIONS, 0).
            build();

        // Initialize the payload types.
        EnumMap<InetProtocols, Class<? extends Packet>> typeMap =
            new EnumMap<>(InetProtocols.class);
        typeMap.put(InetProtocols.ICMP, ICMP.class);
        typeMap.put(InetProtocols.TCP, TCP.class);
        typeMap.put(InetProtocols.UDP, UDP.class);
        PAYLOAD_TYPES = ImmutableMap.copyOf(typeMap);
    }

    /**
     * Determine the payload type for the given IP protocol number.
     *
     * @param proto  The IP protocol number.
     * @return  A class for the payload type.
     *          {@code null} if no payload type is defined for the given
     *          IP protocol number.
     */
    static Class<? extends Packet> getPayloadClass(byte proto) {
        InetProtocols ipproto = InetProtocols.forValue(proto);
        return (ipproto == null) ? null : PAYLOAD_TYPES.get(ipproto);
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor sets the version to 4, headerLength to 5,
     *   and flags to 2.
     * </p>
     */
    public IPv4() {
        setVersion(IPV4_VERSION);
        setHeaderLength(DEFAULT_HEADER_LENGTH);
        setFlags(DEFAULT_FLAGS);
    }

    /**
     * Gets the IP version stored.
     *
     * @return  The IP version.
     */
    public byte getVersion() {
        return getByte(VERSION);
    }

    /**
     * Gets the IP header length stored.
     *
     * @return  The IP header length in bytes.
     */
    public int getHeaderLen() {
        return (getByte(HEADERLENGTH) << UNIT_SIZE_SHIFT);
    }

    /**
     * Gets the differential services value stored.
     *
     * @return  The differential services value.
     */
    public byte getDiffServ() {
        return getByte(DIFFSERV);
    }

    /**
     * Gets the ECN bits stored.
     *
     * @return  The ECN bits.
     */
    public byte getECN() {
        return getByte(ECN);
    }

    /**
     * Gets the total length of the IPv4 packe in bytes.
     *
     * @return  The total length of the IPv4 packet.
     */
    public short getTotalLength() {
        return getShort(TOTLENGTH);
    }

    /**
     * Gets the identification value stored.
     *
     * @return  The IP identification value.
     */
    public short getIdentification() {
        return getShort(IDENTIFICATION);
    }

    /**
     * Gets the flag values stored.
     *
     * @return  The IPv4 flags.
     */
    public byte getFlags() {
        return getByte(FLAGS);
    }

    /**
     * Gets the TTL value stored.
     *
     * @return  The TTL value.
     */
    public byte getTtl() {
        return getByte(TTL);
    }

    /**
     * Gets the protocol value stored.
     *
     * @return  The IP protocol number.
     */
    public byte getProtocol() {
        return getByte(PROTOCOL);
    }

    /**
     * Gets the checksum value stored.
     *
     * @return  The IP header checksum.
     */
    public short getChecksum() {
        return getShort(CHECKSUM);
    }

    /**
     * Gets the fragment offset stored.
     *
     * @return  The IP fragment offset.
     */
    public short getFragmentOffset() {
        return getShort(FRAGOFFSET);
    }

    /**
     * Gets the source IP address stored.
     *
     * @return  An {@link Ip4Network} instance that represents the source
     *          IP address.
     */
    public Ip4Network getSourceAddress() {
        return getIp4Network(SIP);
    }

    /**
     * Gets the destination IP address stored.
     *
     * @return  An {@link Ip4Network} instance that represents the destination
     *          IP address.
     */
    public Ip4Network getDestinationAddress() {
        return getIp4Network(DIP);
    }

    /**
     * Gets the Options stored.
     *
     * @return  The IPv4 options. {@code null} if no option is present.
     */
    public byte[] getOptions() {
        byte[] opts = getHeaderFieldMap().get(OPTIONS);
        return (opts == null || opts.length == 0) ? null : opts.clone();
    }

    /**
     * Stores the IP version from the header.
     *
     * @param version  The version to set
     * @return  This instance.
     */
    public IPv4 setVersion(byte version) {
        getHeaderFieldMap().put(VERSION, new byte[]{version});
        return this;
    }

    /**
     * Stores the length of IP header in words (4 bytes).
     *
     * @param len  The headerLength to set.
     * @return  This instance.
     */
    public IPv4 setHeaderLength(byte len) {
        getHeaderFieldMap().put(HEADERLENGTH, new byte[]{len});
        return this;
    }

    /**
     * Stores the differential services value from the IP header.
     *
     * @param  dserv  The differential services value.
     * @return  This instance.
     */
    public IPv4 setDiffServ(byte dserv) {
        getHeaderFieldMap().put(DIFFSERV, new byte[]{dserv});
        return this;
    }

    /**
     * Stores the ECN bits from the header.
     *
     * @param ecn  The ECN bits to set.
     * @return  This instance.
     */
    public IPv4 setECN(byte ecn) {
        getHeaderFieldMap().put(ECN, new byte[]{ecn});
        return this;
    }

    /**
     * Stores the total length of IPv4 packet in bytes.
     *
     * @param len  The total length of the IPv4 packet.
     * @return  This instance.
     */
    public IPv4 setTotalLength(short len) {
        getHeaderFieldMap().put(TOTLENGTH, toBytes(len));
        return this;
    }

    /**
     * Stores the identification number from the header.
     *
     * @param id  The identification to set.
     * @return  This instance.
     */
    public IPv4 setIdentification(short id) {
        getHeaderFieldMap().put(IDENTIFICATION, toBytes(id));
        return this;
    }

    /**
     * Stores the IP flags value.
     *
     * @param flags  The flags to set.
     * @return  This instance.
     */
    public IPv4 setFlags(byte flags) {
        getHeaderFieldMap().put(FLAGS, new byte[]{flags});
        return this;
    }

    /**
     * Stores the IP fragmentation offset value.
     *
     * @param off  The fragmentation offset.
     * @return  This instance.
     */
    public IPv4 setFragmentOffset(short off) {
        getHeaderFieldMap().put(FRAGOFFSET, toBytes(off));
        return this;
    }

    /**
     * Stores the TTL value.
     *
     * @param ttl  The ttl to set.
     * @return  This instance.
     */
    public IPv4 setTtl(byte ttl) {
        getHeaderFieldMap().put(TTL, new byte[]{ttl});
        return this;
    }

    /**
     * Stores the protocol value of the IP payload.
     *
     * @param proto  The protocol to set.
     * @return  This instance.
     */
    public IPv4 setProtocol(byte proto) {
        getHeaderFieldMap().put(PROTOCOL, new byte[]{proto});
        return this;
    }

    /**
     * Set the IP checksum value.
     *
     * <p>
     *   This method is only for testing.
     *   IP checksum will be updated on serialization.
     * </p>
     *
     * @param cksum  The IP checksum.
     * @return  This instance.
     */
    public IPv4 setChecksum(short cksum) {
        getHeaderFieldMap().put(CHECKSUM, toBytes(cksum));
        return this;
    }

    /**
     * Stores the IP source address from the header.
     *
     * <p>
     *   The network prefix in the given paramter is always ignored.
     * </p>
     *
     * @param addr  The source IP address.
     * @return  This instance.
     */
    public IPv4 setSourceAddress(Ip4Network addr) {
        getHeaderFieldMap().put(SIP, addr.getBytes());
        return this;
    }

    /**
     * Stores the IP destination address from the header.
     *
     * <p>
     *   The network prefix in the given paramter is always ignored.
     * </p>
     *
     * @param addr  The destination IP address.
     * @return  This instance.
     */
    public IPv4 setDestinationAddress(Ip4Network addr) {
        getHeaderFieldMap().put(DIP, addr.getBytes());
        return this;
    }

    /**
     * Store the options from IP header.
     *
     * @param options  The IP options.
     * @return  This instance.
     */
    public IPv4 setOptions(byte[] options) {
        Map<String, byte[]> map = getHeaderFieldMap();
        byte newIHL = DEFAULT_HEADER_LENGTH;
        if (options == null || options.length == 0) {
            map.remove(OPTIONS);
        } else {
            int len = options.length;
            int mask = UNIT_SIZE - 1;
            int rlen = (len + mask) & ~mask;
            byte[] newopt;
            if (rlen > len) {
                // Padding is required.
                newopt = new byte[rlen];
                System.arraycopy(options, 0, newopt, 0, len);
                len = rlen;
            } else {
                newopt = options.clone();
            }
            map.put(OPTIONS, newopt);
            newIHL += ((len & MASK_BYTE) >>> UNIT_SIZE_SHIFT);
        }

        map.put(HEADERLENGTH, new byte[]{newIHL});
        return this;
    }

    /**
     * Computes the IPv4 header checksum on the passed stream of bytes
     * representing the packet.
     *
     * @param data    The byte stream.
     * @param start  The byte offset from where the IPv4 packet starts.
     * @return The computed checksum
     */
    short computeChecksum(byte[] data, int start) {
        int end = start + getHeaderLen() - 1;
        int sum = 0;
        int wordData;
        int checksumStart = start + (getFieldOffset(CHECKSUM) / Byte.SIZE);

        for (int i = start; i <= end; i += CKSUM_BYTES) {
            // Skip, if the current bytes are checkSum bytes
            if (i != checksumStart) {
                wordData = ((data[i] & MASK_BYTE) << Byte.SIZE) |
                    (data[i + 1] & MASK_BYTE);
                sum = sum + wordData;
            }
        }

        int carry = sum >>> Short.SIZE;
        int finalSum = (sum & MASK_SHORT) + carry;
        return (short)~((short)finalSum & MASK_SHORT);
    }

    // Packet

    /**
     * Gets the header size in bits.
     *
     * @return  The number of bits constituting the header.
     */
    @Override
    public int getHeaderSize() {
        int len = getHeaderLen();
        if (len == 0) {
            len = MIN_HEADER_SIZE;
        }

        return len * Byte.SIZE;
    }

    /**
     * Gets the number of bits for the specified field.
     *
     * <p>
     *   If the fieldname has variable length like "Options", then this value
     *   is computed using the header length.
     * </p>
     *
     * @param name   The name of the header field.
     * @param entry  The header field entry associated with {@code name}.
     * @return  The number of bits of the requested field.
     */
    @Override
    protected int getFieldNumBits(String name, HeaderField entry) {
        return (name.equals(OPTIONS))
            ? (getHeaderLen() - MIN_HEADER_SIZE) * Byte.SIZE
            : entry.getNumBits();
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
     * <p>
     *   Variable header value like payload protocol, is stored here.
     * </p>
     *
     * @param name   The name of the header field.
     * @param value  The value to be associated with the specified header
     *               field. {@code null} cannot be specified.
     */
    @Override
    protected void setHeaderField(String name, byte[] value) {
        Map<String, byte[]> map = getHeaderFieldMap();
        if (name.equals(PROTOCOL)) {
            // Don't set payloadClass if framgment offset is not zero.
            byte[] fragoff = map.get(FRAGOFFSET);
            if (isZeroShort(fragoff)) {
                setPayloadClass(getPayloadClass(value[0]));
            }
        } else if (name.equals(FRAGOFFSET)) {
            if (!isZeroShort(value)) {
                // Clear payloadClass because protocol header is not present
                // in this packet.
                setPayloadClass(null);
            }
        } else if (name.equals(OPTIONS) &&
                   (value == null || value.length == 0)) {
            map.remove(name);
            return;
        }
        map.put(name, value);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean equalsField(String name, byte[] value1, byte[] value2) {
        return (name.equals(OPTIONS))
            ? Arrays.equals(value1, value2)
            : super.equalsField(name, value1, value2);
    }

    /**
     * Set the packet encapsulated by this IPv4 packet.
     *
     * <p>
     *   This method updates the total length of the IPv4 packet.
     * </p>
     *
     * @param p  The packet encapsulated by this IPv4 packet.
     */
    @Override
    public void setPayload(Packet p) {
        super.setPayload(p);

        if (p != null) {
            // Determine the total length.
            int plen;
            try {
                plen = p.serialize().length;
            } catch (PacketException e) {
                throw new IllegalStateException(
                    "Failed to serialize the specified payload.", e);
            }

            setTotalLength((short)(getHeaderLen() + plen));
        }
    }

    /**
     * This method gets called at the end of the serialization process.
     *
     * <p>
     *   This method computes the IP checksum and stores it into the IP header.
     * </p>
     *
     * @param myBytes  Serialized bytes.
     * @throws PacketException  An error occurred.
     */
    @Override
    protected void postSerializeCustomOperation(byte[] myBytes)
        throws PacketException {
        try {
            Map<String, byte[]> map = getHeaderFieldMap();
            String field = TOTLENGTH;
            HeaderField entry = HEADER_FORMAT.get(field);
            int off = getFieldOffset(field, entry);
            int nbits = getFieldNumBits(field, entry);

            // Recompute the total length field here.
            byte[] total = toBytes((short)myBytes.length);
            ByteUtils.setBits(myBytes, total, off, nbits);
            map.put(TOTLENGTH, total);

            // Compute the header checksum.
            field = CHECKSUM;
            entry = HEADER_FORMAT.get(field);
            off = getFieldOffset(field, entry);
            nbits = getFieldNumBits(field, entry);
            byte[] cksum = toBytes(computeChecksum(myBytes, 0));
            ByteUtils.setBits(myBytes, cksum, off, nbits);
            map.put(CHECKSUM, cksum);
        } catch (RuntimeException e) {
            throw new PacketException("Failed to update checksum.", e);
        }
    }

    /**
     * This method re-computes the checksum of the bits received on the wire
     * and validates it with the checksum in the bits received.
     *
     * @param data   The byte stream representing the Ethernet frame.
     * @param offset The bit offset from where the byte array corresponding to
     *               this Packet starts in the frame
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
    public IPv4 clone() {
        return (IPv4)super.clone();
    }
}
