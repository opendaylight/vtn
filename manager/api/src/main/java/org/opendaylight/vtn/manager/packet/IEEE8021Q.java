/*
 * Copyright (c) 2013, 2016 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.packet.EtherTypePacket.ETHT;
import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;

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
public final class IEEE8021Q extends EtherTypePacket<IEEE8021Q> {
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
}
