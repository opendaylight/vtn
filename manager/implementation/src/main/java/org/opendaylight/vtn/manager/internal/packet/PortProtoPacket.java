/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.controller.sal.action.SetTpDst;
import org.opendaylight.controller.sal.action.SetTpSrc;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code PortProtoPacket} class implements a cache for layer 4 protocol
 * header, which identifies the service using 16-bit port number.
 *
 * @param <T>  Type of packet.
 */
public abstract class PortProtoPacket<T extends Packet> implements L4Packet {
    /**
     * A pseudo port number which indicates the port number is not specified.
     */
    private static final int  PORT_NONE = -1;

    /**
     * Computed checksum that indicates the packet verification succeeded.
     */
    private static final int  CKSUM_OK = 0xffff;

    /**
     * The number of octets in TCP/UDP checksum.
     */
    private static final int  CKSUM_BYTES =
        Short.SIZE / NetUtils.NumBitsInAByte;

    /**
     * A mask value used to clear LSB.
     */
    private static final int  MASK_CLEAR_LSB = ~1;

    /**
     * Cached values in a protocol header.
     */
    private Values  values = new Values();

    /**
     * Protocol header values to be set.
     */
    private Values  modifiedValues;

    /**
     * Set {@code true} if this instance is created by {@link #clone()}.
     */
    private boolean  cloned;

    /**
     * This class describes modifiable fields in a protocol hedaer.
     */
    private static final class Values implements Cloneable {
        /**
         * The source port number.
         */
        private int  sourcePort = PORT_NONE;

        /**
         * The destination port number.
         */
        private int  destinationPort = PORT_NONE;

        /**
         * Constructor.
         */
        private Values() {
        }

        /**
         * Return the source port number.
         *
         * @return  An integer value which indicates the source port.
         *          {@link PortProtoPacket#PORT_NONE} is returned if not
         *          configured.
         */
        private int getSourcePort() {
            return sourcePort;
        }

        /**
         * Set the source port number.
         *
         * @param port  An integer value which indicates the source port.
         */
        private void setSourcePort(int port) {
            sourcePort = port;
        }

        /**
         * Set the source port number.
         *
         * @param port  A short integer value which indicates the source port.
         * @return  An integer value which indicates the source port.
         */
        private int setSourcePort(short port) {
            sourcePort = NetUtils.getUnsignedShort(port);
            return sourcePort;
        }

        /**
         * Return the destination port number.
         *
         * @return  An integer value which indicates the destination port.
         *          {@link PortProtoPacket#PORT_NONE} is returned if not
         *          configured.
         */
        private int getDestinationPort() {
            return destinationPort;
        }

        /**
         * Set the destination port number.
         *
         * @param port  An integer value which indicates the destination port.
         */
        private void setDestinationPort(int port) {
            destinationPort = port;
        }

        /**
         * Set the destination port number.
         *
         * @param port  A short integer value which indicates the destination
         *              port.
         * @return  An integer value which indicates the destination port.
         */
        private int setDestinationPort(short port) {
            destinationPort = NetUtils.getUnsignedShort(port);
            return destinationPort;
        }

        /**
         * Fetch all modifiable field values from the given packet.
         *
         * <p>
         *   Field values already cached in this instance are preserved.
         * </p>
         *
         * @param packet  A {@link PortProtoPacket} instance.
         */
        private void fill(PortProtoPacket packet) {
            if (sourcePort == PORT_NONE) {
                setSourcePort(packet.getRawSourcePort());
            }
            if (destinationPort == PORT_NONE) {
                setDestinationPort(packet.getRawDestinationPort());
            }
        }

        /**
         * Return a shallow copy of this instance.
         *
         * @return  A shallow copy of this instance.
         */
        @Override
        public Values clone() {
            try {
                return (Values)super.clone();
            } catch (CloneNotSupportedException e) {
                // This should never happen.
                throw new IllegalStateException("clone() failed", e);
            }
        }
    }

    /**
     * Compute the one's complement sum of all 16-bit words in the given
     * packet.
     *
     * @param ipv4  An {@link Inet4Packet} instance that contains the given
     *              packet.
     * @param data  A serialized packet data.
     * @return  A computed checksum.
     */
    private static int computeChecksum(Inet4Packet ipv4, byte[] data) {
        // Create a pseudo IPv4 header.
        byte proto = (byte)ipv4.getProtocol();
        byte[] header = ipv4.getHeaderForChecksum(proto, (short)data.length);
        int sum = 0;
        for (int i = 0; i < header.length; i += CKSUM_BYTES) {
            int v = ((header[i] & MiscUtils.MASK_BYTE) << Byte.SIZE) |
                (header[i + 1] & MiscUtils.MASK_BYTE);
            sum += v;
        }

        int rsize = (data.length & MASK_CLEAR_LSB);
        for (int i = 0; i < rsize; i += CKSUM_BYTES) {
            int v = ((data[i] & MiscUtils.MASK_BYTE) << Byte.SIZE) |
                (data[i + 1] & MiscUtils.MASK_BYTE);
            sum += v;
        }
        if (rsize < data.length) {
            // Zero padding is needed.
            int v = (data[rsize] & MiscUtils.MASK_BYTE) << Byte.SIZE;
            sum += v;
        }

        int carry = (sum >>> Short.SIZE);
        return (sum & MiscUtils.MASK_SHORT) + carry;
    }

    /**
     * Compute the checksum of the given packet.
     *
     * @param ipv4    An {@link Inet4Packet} instance that contains the given
     *                packet.
     * @param packet  A {@link Packet} instance.
     * @param sumOff  Offset in bytes to the checksum field.
     * @return  A computed checksum.
     * @throws VTNException
     *    An error occurred.
     */
    protected static final short computeChecksum(Inet4Packet ipv4,
                                                 Packet packet, int sumOff)
        throws VTNException {
        // Serialize the given packet.
        byte[] data;
        try {
            data = packet.serialize();
        } catch (Exception e) {
            // This should never happen.
            throw new VTNException("Failed to serialize the packet.", e);
        }

        // Clear checksum field.
        MiscUtils.setShort(data, sumOff, (short)0);

        // Compute checksum.
        return (short)~computeChecksum(ipv4, data);
    }

    /**
     * Verify the packet using the checksum.
     *
     * @param ipv4    An {@link Inet4Packet} instance that contains the given
     *                packet.
     * @param packet  A {@link Packet} to be verified.
     * @return  {@code true} is returned only if the verification succeeded.
     * @throws VTNException
     *    An error occurred.
     */
    protected static final boolean verifyChecksum(Inet4Packet ipv4,
                                                  Packet packet)
        throws VTNException {
        // Serialize the given packet.
        byte[] data;
        try {
            data = packet.serialize();
        } catch (Exception e) {
            // This should never happen.
            throw new VTNException("Failed to serialize the packet.", e);
        }

        // Compute checksum.
        int sum = computeChecksum(ipv4, data);
        return (sum == CKSUM_OK);
    }

    /**
     * Construct a new instance.
     */
    protected PortProtoPacket() {
    }

    /**
     * Return the source port number.
     *
     * @return  An integer value which represents the source port number.
     */
    public final int getSourcePort() {
        Values v = getValues();
        int port = v.getSourcePort();
        if (port == PORT_NONE) {
            short p = getRawSourcePort();
            port = v.setSourcePort(p);
        }

        return port;
    }

    /**
     * Set the source port number.
     *
     * @param port  An integer value which indicates the source port.
     */
    public final void setSourcePort(int port) {
        Values v = getModifiedValues();
        v.setSourcePort(port);
    }

    /**
     * Return the destination port number.
     *
     * @return  An integer value which represents the destination port number.
     */
    public final int getDestinationPort() {
        Values v = getValues();
        int port = v.getDestinationPort();
        if (port == PORT_NONE) {
            short p = getRawDestinationPort();
            port = v.setDestinationPort(p);
        }

        return port;
    }

    /**
     * Set the destination port number.
     *
     * @param port  An integer value which indicates the destination port.
     */
    public final void setDestinationPort(int port) {
        Values v = getModifiedValues();
        v.setDestinationPort(port);
    }

    /**
     * Return a {@link Packet} instance to set modified values.
     *
     * @return  A {@link Packet} instance.
     * @throws VTNException
     *    Failed to copy the packet.
     */
    protected final T getPacketForWrite() throws VTNException {
        T pkt = getPacketForWrite(cloned);
        cloned = false;
        return pkt;
    }

    /**
     * Derive the source port number from the packet.
     *
     * @return  A short integer value which represents the source port number.
     */
    protected abstract short getRawSourcePort();

    /**
     * Derive the destination port number from the packet.
     *
     * @return  A short integer value which represents the destination port
     *          number.
     */
    protected abstract short getRawDestinationPort();

    /**
     * Set the source port number to the given packet.
     *
     * @param pkt   A {@link Packet} instance.
     * @param port  A short integer value which indicates the source port.
     */
    protected abstract void setRawSourcePort(T pkt, short port);

    /**
     * Set the destination port number to the given packet.
     *
     * @param pkt   A {@link Packet} instance.
     * @param port  A short integer value which indicates the destination port.
     */
    protected abstract void setRawDestinationPort(T pkt, short port);

    /**
     * Return a {@link Packet} instance to set modified values.
     *
     * @param doCopy {@code true} is passed if the packet configured in this
     *               instance needs to be copied.
     * @return  A {@link Packet} instance.
     * @throws VTNException
     *    Failed to copy the packet.
     */
    protected abstract T getPacketForWrite(boolean doCopy) throws VTNException;

    /**
     * Return a {@link Values} instance that keeps current values for
     * protocol header fields.
     *
     * @return  A {@link Values} instance.
     */
    private Values getValues() {
        return (modifiedValues == null) ? values : modifiedValues;
    }

    /**
     * Return a {@link Values} instance that keeps protocol header field values
     * to be set.
     *
     * @return  A {@link Values} instance.
     */
    private Values getModifiedValues() {
        if (modifiedValues == null) {
            values.fill(this);
            modifiedValues = values.clone();
        }

        return modifiedValues;
    }

    // CachedPacket

    /**
     * Configure match fields to test TCP/UDP header in this packet.
     *
     * <p>
     *   Note that this method creates match fields that matches the original
     *   packet. Any modification to the packet is ignored.
     * </p>
     *
     * @param match   A {@link Match} instance.
     * @param fields  A set of {@link MatchType} instances corresponding to
     *                match fields to be tested.
     */
    @Override
    public final void setMatch(Match match, Set<MatchType> fields) {
        Values v = values;
        v.fill(this);

        MatchType type = MatchType.TP_SRC;
        if (fields.contains(type)) {
            // Test source port number.
            match.setField(type, (short)v.getSourcePort());
        }

        type = MatchType.TP_DST;
        if (fields.contains(type)) {
            // Test destination port number.
            match.setField(type, (short)v.getDestinationPort());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean commit(PacketContext pctx) throws VTNException {
        boolean mod = false;
        T pkt = null;
        if (modifiedValues != null) {
            // At least one flow action that modifies TCP or UDP header is
            // configured.
            pctx.addMatchField(MatchType.DL_TYPE);
            pctx.addMatchField(MatchType.NW_PROTO);

            int src = modifiedValues.getSourcePort();
            if (values.getSourcePort() != src) {
                // Source port was modified.
                pkt = getPacketForWrite();
                setRawSourcePort(pkt, (short)src);
                mod = true;
            } else if (pctx.hasMatchField(MatchType.TP_SRC)) {
                // Source port in the original packet is unchanged and it will
                // be specified in flow match. So we don't need to configure
                // SET_TP_SRC action.
                pctx.removeFilterAction(SetTpSrc.class);
            }

            int dst = modifiedValues.getDestinationPort();
            if (values.getDestinationPort() != dst) {
                // Destination port was modified.
                if (pkt == null) {
                    pkt = getPacketForWrite();
                }
                setRawDestinationPort(pkt, (short)dst);
                mod = true;
            } else if (pctx.hasMatchField(MatchType.TP_DST)) {
                // Destination port in the original packet is unchanged and
                // it will be specified in flow match. So we don't need to
                // configure SET_TP_DST action.
                pctx.removeFilterAction(SetTpDst.class);
            }
        }

        return mod;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final PortProtoPacket clone() {
        try {
            PortProtoPacket p = (PortProtoPacket)super.clone();
            Values v = p.values;
            p.values = v.clone();

            v = p.modifiedValues;
            if (v != null) {
                p.modifiedValues = v.clone();
            }
            p.cloned = true;

            return p;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
