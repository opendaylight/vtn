/*
 * Copyright (c) 2013, 2015 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.packet.Ethernet.ETHT;
import static org.opendaylight.vtn.manager.packet.Ethernet.getPayloadClass;
import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;
import static org.opendaylight.vtn.manager.util.NumberUtils.toShort;

import java.util.Map;

/**
 * {@code IEEE8021Q} describes an IEEE 802.1Q VLAN tag.
 *
 * <p>
 *   This class is provided only for VTN internal use.
 *   This class may be changed without any notice.
 * </p>
 *
 * @since  Beryllium
 */
public final class IEEE8021Q extends Packet {
    /**
     * The number of bits in the VLAN tag.
     */
    private static final int  HEADER_SIZE = 32;

    /**
     * The field name that indicates the priority code point.
     */
    private static final String  PCP = "PriorityCodePoint";

    /**
     * The field name that indicates the canonical format indicator.
     */
    private static final String  CFI = "CanonicalFormatIndicator";

    /**
     * The field name that indicates the VLAN ID.
     */
    private static final String  VID = "VlanIdentifier";

    /**
     * The number of bits in the prioriy code point field.
     */
    private static final int  BITS_PCP = 3;

    /**
     * The number of bits in the canonical format indicator field.
     */
    private static final int  BITS_CFI = 1;

    /**
     * The number of bits in the VLAN ID field.
     */
    private static final int  BITS_VID = 12;

    /**
     * A map that determines the IEEE 802.1Q header format.
     */
    private static final Map<String, HeaderField>  HEADER_FORMAT;

    /**
     * Initialize static fields.
     */
    static {
        HEADER_FORMAT = new HeaderMapBuilder().
            addNumber(PCP, BITS_PCP).
            addNumber(CFI, BITS_CFI).
            addNumber(VID, BITS_VID).
            addNumber(ETHT, Short.SIZE).
            build();
    }

    /**
     * Gets the priority code point(PCP) stored.
     *
     * @return  The priority code point value.
     */
    public byte getPcp() {
        return getByte(PCP);
    }

    /**
     * Gets the canonical format indicator(CFI) stored.
     *
     * @return  The canonical format indicator value.
     */
    public byte getCfi() {
        return getByte(CFI);
    }

    /**
     * Gets the VLAN identifier(VID) stored.
     *
     * @return  The VLAN ID.
     */
    public short getVid() {
        return getShort(VID);
    }

    /**
     * Gets the Ethernet type stored.
     *
     * @return  The Ethernet type.
     */
    public short getEtherType() {
        return getShort(ETHT);
    }

    /**
     * Sets the priority code point(PCP) for the current IEEE 802.1Q object
     * instance.
     *
     * @param pcp  The priority code point value.
     * @return  This instance.
     */
    public IEEE8021Q setPcp(byte pcp) {
        getHeaderFieldMap().put(PCP, new byte[]{pcp});
        return this;
    }

    /**
     * Sets the canonical format indicator(CFI) for the current IEEE 802.1Q
     * object instance.
     *
     * @param cfi  The canonical format indicator value.
     * @return  This instance.
     */
    public IEEE8021Q setCfi(byte cfi) {
        getHeaderFieldMap().put(CFI, new byte[]{cfi});
        return this;
    }

    /**
     * Sets the VLAN identifier(VID) for the current IEEE 802.1Q instance.
     *
     * @param vid  The VLAN ID.
     * @return  This instance.
     */
    public IEEE8021Q setVid(short vid) {
        getHeaderFieldMap().put(VID, toBytes(vid));
        return this;
    }

    /**
     * Sets the etherType for the current IEEE 802.1Q object instance.
     *
     * @param type  The Ethernet type.
     * @return  This instance.
     */
    public IEEE8021Q setEtherType(short type) {
        getHeaderFieldMap().put(ETHT, toBytes(type));
        return this;
    }

    // Packet

    /**
     * Gets the header size in bits.
     *
     * @return  The IEEE 802.1Q header size in bits.
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
    public IEEE8021Q clone() {
        return (IEEE8021Q)super.clone();
    }
}
