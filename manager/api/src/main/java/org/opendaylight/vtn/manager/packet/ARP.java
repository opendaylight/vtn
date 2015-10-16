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

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

/**
 * {@code ARP} describes an ARP packet.
 *
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public final class ARP extends Packet {
    /**
     * The number of bits in the ARP header.
     */
    private static final int  HEADER_SIZE = 224;

    /**
     * The field name that indicates the hardware type.
     */
    private static final String  HWTYPE = "HardwareType";

    /**
     * The field name that indicates the protocol type.
     */
    private static final String  PTYPE = "ProtocolType";

    /**
     * The field name that indicates the hardware address length.
     */
    private static final String  HWADDRLENGTH = "HardwareAddressLength";

    /**
     * The field name that indicates the protocol address length.
     */
    private static final String  PADDRLENGTH = "ProtocolAddressLength";

    /**
     * The field name that indicates the operation code.
     */
    private static final String  OPCODE = "OpCode";

    /**
     * The field name that indicates the sender hardware address.
     */
    private static final String  SENDERHWADDR = "SenderHardwareAddress";

    /**
     * The field name that indicates the sender protocol address.
     */
    private static final String  SENDERPADDR = "SenderProtocolAddress";

    /**
     * The field name that indicates the target hardware address.
     */
    private static final String  TARGETHWADDR = "TargetHardwareAddress";

    /**
     * The field name that indicates the target protocol address.
     */
    private static final String  TARGETPADDR = "TargetProtocolAddress";

    /**
     * The hardware tyep value that indicates Ethernet.
     */
    public static final short  HW_TYPE_ETHERNET = 0x1;

    /**
     * The ARP operation code that indicates ARP request.
     */
    public static final short  REQUEST = 0x1;

    /**
     * The ARP operation code that indicates ARP reply.
     */
    public static final short  REPLY = 0x2;

    /**
     * A map that determines the ARP packet header format.
     */
    private static final Map<String, HeaderField>  HEADER_FORMAT;

    /**
     * Initialize static fields.
     */
    static {
        int etherBits = EtherAddress.SIZE * Byte.SIZE;
        int ipv4Bits = Ip4Network.SIZE * Byte.SIZE;
        HEADER_FORMAT = new HeaderMapBuilder().
            addNumber(HWTYPE, Short.SIZE).
            addNumber(PTYPE, Short.SIZE).
            addNumber(HWADDRLENGTH, Byte.SIZE).
            addNumber(PADDRLENGTH, Byte.SIZE).
            addNumber(OPCODE, Short.SIZE).
            addByte(SENDERHWADDR, etherBits).
            addByte(SENDERPADDR, ipv4Bits).
            addByte(TARGETHWADDR, etherBits).
            addByte(TARGETPADDR, ipv4Bits).
            build();
    }

    /**
     * Gets the hardware type from the stored ARP header.
     *
     * @return  The hardware type.
     */
    public short getHardwareType() {
        return getShort(HWTYPE);
    }

    /**
     * Gets the protocol type from the stored ARP header.
     *
     * @return  The protocol type.
     */
    public short getProtocolType() {
        return getShort(PTYPE);
    }

    /**
     * Gets the hardware address length from the stored ARP header.
     *
     * @return  The length of the hardware address.
     */
    public byte getHardwareAddressLength() {
        return getByte(HWADDRLENGTH);
    }

    /**
     * Get the protocol address length from Protocol header.
     *
     * @return  The length of the protocol address.
     */
    public byte getProtocolAddressLength() {
        return getByte(PADDRLENGTH);
    }

    /**
     * Gets the opCode from stored ARP header.
     *
     * @return  The operation code.
     */
    public short getOpCode() {
        return getShort(OPCODE);
    }

    /**
     * Gets the sender hardware address from the stored ARP header.
     *
     * @return  The sender hardware address.
     *          {@code null} if not configured.
     */
    public byte[] getSenderHardwareAddress() {
        return getBytes(SENDERHWADDR);
    }

    /**
     * Gets the IP address from the stored ARP header.
     *
     * @return  The sender protocol address.
     *          {@code null} if not configured.
     */
    public byte[] getSenderProtocolAddress() {
        return getBytes(SENDERPADDR);
    }

    /**
     * Gets the hardware address from the stored ARP header.
     *
     * @return  The target hardware address.
     *          {@code null} if not configured.
     */
    public byte[] getTargetHardwareAddress() {
        return getBytes(TARGETHWADDR);
    }

    /**
     * Gets the target protocol address.
     *
     * @return  The target protocol address.
     *          {@code null} if not configured.
     */
    public byte[] getTargetProtocolAddress() {
        return getBytes(TARGETPADDR);
    }

    /**
     * Sets the hardware Type for the current ARP object instance.
     *
     * @param hwtype  The hardwareType to set.
     * @return  This instance.
     */
    public ARP setHardwareType(short hwtype) {
        getHeaderFieldMap().put(HWTYPE, toBytes(hwtype));
        return this;
    }

    /**
     * Sets the protocol Type for the current ARP object instance.
     *
     * @param ptype  The protocol type to set.
     * @return  This instance.
     */
    public ARP setProtocolType(short ptype) {
        getHeaderFieldMap().put(PTYPE, toBytes(ptype));
        return this;
    }

    /**
     * Sets the hardware address length for the current ARP object instance.
     *
     * @param len  The length of the hardware address.
     * @return  This instance.
     */
    public ARP setHardwareAddressLength(byte len) {
        getHeaderFieldMap().put(HWADDRLENGTH, new byte[]{len});
        return this;
    }

    /**
     * Sets the Protocol address for the current ARP object instance.
     *
     * @param len  The length of the protocol address.
     * @return  This instance.
     */
    public ARP setProtocolAddressLength(byte len) {
        getHeaderFieldMap().put(PADDRLENGTH, new byte[]{len});
        return this;
    }

    /**
     * Sets the opCode for the current ARP object instance.
     *
     * @param op  The operation code.
     * @return  This instance.
     */
    public ARP setOpCode(short op) {
        getHeaderFieldMap().put(OPCODE, toBytes(op));
        return this;
    }

    /**
     * Sets the sender hardware address for the current ARP object instance.
     *
     * @param addr  The sender hardware address.
     * @return  This instance.
     */
    public ARP setSenderHardwareAddress(byte[] addr) {
        getHeaderFieldMap().put(SENDERHWADDR, addr.clone());
        return this;
    }

    /**
     * Sets the target hardware address for the current ARP object instance.
     *
     * @param addr  The target hardware address.
     * @return  This instance.
     */
    public ARP setTargetHardwareAddress(byte[] addr) {
        getHeaderFieldMap().put(TARGETHWADDR, addr.clone());
        return this;
    }

    /**
     * Sets the sender protocol address for the current ARP object instance.
     *
     * @param addr  The sender protocol address.
     * @return  This instance.
     */
    public ARP setSenderProtocolAddress(byte[] addr) {
        getHeaderFieldMap().put(SENDERPADDR, addr.clone());
        return this;
    }

    /**
     * Sets the target protocol address for the current ARP object instance.
     *
     * @param addr  The target protocol address.
     * @return  This instance.
     */
    public ARP setTargetProtocolAddress(byte[] addr) {
        getHeaderFieldMap().put(TARGETPADDR, addr.clone());
        return this;
    }

    // Packet

    /**
     * Gets the header size in bits.
     *
     * @return The ARP header size in bits.
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
    public ARP clone() {
        return (ARP)super.clone();
    }
}
