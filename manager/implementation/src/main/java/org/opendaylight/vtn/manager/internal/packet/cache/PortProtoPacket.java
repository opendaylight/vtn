/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNLayer4PortMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNPortRange;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4PortHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.packet.Packet;

/**
 * {@code PortProtoPacket} class implements a cache for layer 4 protocol
 * header, which identifies the service using 16-bit port number.
 *
 * @param <T>  Type of packet.
 */
public abstract class PortProtoPacket<T extends Packet>
    implements L4Packet, Layer4PortHeader {
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
    private static final int  CKSUM_BYTES = Short.SIZE / Byte.SIZE;

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
            sourcePort = NumberUtils.getUnsigned(port);
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
            destinationPort = NumberUtils.getUnsigned(port);
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
            int v = ((header[i] & NumberUtils.MASK_BYTE) << Byte.SIZE) |
                (header[i + 1] & NumberUtils.MASK_BYTE);
            sum += v;
        }

        int rsize = (data.length & MASK_CLEAR_LSB);
        for (int i = 0; i < rsize; i += CKSUM_BYTES) {
            int v = ((data[i] & NumberUtils.MASK_BYTE) << Byte.SIZE) |
                (data[i + 1] & NumberUtils.MASK_BYTE);
            sum += v;
        }
        if (rsize < data.length) {
            // Zero padding is needed.
            int v = (data[rsize] & NumberUtils.MASK_BYTE) << Byte.SIZE;
            sum += v;
        }

        int carry = (sum >>> Short.SIZE);
        return (sum & NumberUtils.MASK_SHORT) + carry;
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
        NumberUtils.setShort(data, sumOff, (short)0);

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
     * Return the name of the protocol.
     *
     * @return  The protocol name.
     */
    protected abstract String getProtocolName();

    /**
     * Construct flow match fields.
     *
     * @param src  A {@link VTNPortRange} instance which specifies the
     *             source port.
     * @param dst  A {@link VTNPortRange} instance which specifies the
     *             destination port.
     * @return  A {@link VTNLayer4PortMatch} instance.
     * @throws RpcException  Invalid value is specified.
     */
    protected abstract VTNLayer4PortMatch createMatch(VTNPortRange src,
                                                      VTNPortRange dst)
        throws RpcException;

    /**
     * Return a flow match type corresponding to the source port.
     *
     * @return  A {@link FlowMatchType} instance.
     */
    public abstract FlowMatchType getSourceMatchType();

    /**
     * Return a flow match type corresponding to the destination port.
     *
     * @return  A {@link FlowMatchType} instance.
     */
    public abstract FlowMatchType getDestinationMatchType();

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
     * {@inheritDoc}
     */
    @Override
    public final boolean commit(PacketContext pctx) throws VTNException {
        boolean mod = false;
        T pkt = null;
        if (modifiedValues != null) {
            // At least one flow action that modifies TCP or UDP header is
            // configured.
            pctx.addMatchField(FlowMatchType.DL_TYPE);
            pctx.addMatchField(FlowMatchType.IP_PROTO);

            int src = modifiedValues.getSourcePort();
            if (values.getSourcePort() != src) {
                // Source port was modified.
                pkt = getPacketForWrite();
                setRawSourcePort(pkt, (short)src);
                mod = true;
            } else if (pctx.hasMatchField(getSourceMatchType())) {
                // Source port in the original packet is unchanged and it will
                // be specified in flow match. So we don't need to configure
                // SET_TP_SRC action.
                pctx.removeFilterAction(VTNSetPortSrcAction.class);
            }

            int dst = modifiedValues.getDestinationPort();
            if (values.getDestinationPort() != dst) {
                // Destination port was modified.
                if (pkt == null) {
                    pkt = getPacketForWrite();
                }
                setRawDestinationPort(pkt, (short)dst);
                mod = true;
            } else if (pctx.hasMatchField(getDestinationMatchType())) {
                // Destination port in the original packet is unchanged and
                // it will be specified in flow match. So we don't need to
                // configure SET_TP_DST action.
                pctx.removeFilterAction(VTNSetPortDstAction.class);
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

    // L4Packet

    /**
     * {@inheritDoc}
     */
    @Override
    public final VTNLayer4PortMatch createMatch(Set<FlowMatchType> fields)
        throws RpcException {
        Values v = values;
        v.fill(this);

        VTNPortRange src = (fields.contains(getSourceMatchType()))
            ? new VTNPortRange(v.getSourcePort())
            : null;
        VTNPortRange dst = (fields.contains(getDestinationMatchType()))
            ? new VTNPortRange(v.getDestinationPort())
            : null;

        return createMatch(src, dst);
    }

    // Layer4PortHeader

    /**
     * {@inheritDoc}
     */
    @Override
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
     * {@inheritDoc}
     */
    @Override
    public final void setSourcePort(int port) {
        Values v = getModifiedValues();
        v.setSourcePort(port);
    }

    /**
     * {@inheritDoc}
     */
    @Override
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
     * {@inheritDoc}
     */
    @Override
    public final void setDestinationPort(int port) {
        Values v = getModifiedValues();
        v.setDestinationPort(port);
    }

    // ProtocolHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDescription(StringBuilder builder) {
        int src = getSourcePort();
        int dst = getDestinationPort();
        builder.append(getProtocolName()).
            append("[src=").append(src).
            append(",dst=").append(dst).append(']');
    }
}
