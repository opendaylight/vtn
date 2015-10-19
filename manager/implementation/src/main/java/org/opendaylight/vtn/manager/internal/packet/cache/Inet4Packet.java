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
import org.opendaylight.vtn.manager.packet.IPv4;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNInet4Match;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * {@code Inet4Packet} class implements a cache for an {@link IPv4} instance.
 */
public final class Inet4Packet implements CachedPacket, InetHeader {
    /**
     * A pseudo IP protocol number which indicates the IP protocol number is
     * not specified.
     */
    private static final short  PROTO_NONE = -1;

    /**
     * A pseudo DSCP value which indicates the DSCP value is not specified.
     */
    private static final short  DSCP_NONE = -1;

    /**
     * Byte offset to the source address in a pseudo IPv4 header used for
     * computing TCP/UDP checksum.
     */
    private static final int CKSUM_OFF_SRC = 0;

    /**
     * Byte offset to the destination address in a pseudo IPv4 header used for
     * computing TCP/UDP checksum.
     */
    private static final int CKSUM_OFF_DST = 4;

    /**
     * Byte offset to the IP protocol number in a pseudo IPv4 header used for
     * computing TCP/UDP checksum.
     */
    private static final int CKSUM_OFF_PROTO = 9;

    /**
     * Byte offset to the payload length in a pseudo IPv4 header used for
     * computing TCP/UDP checksum.
     */
    private static final int CKSUM_OFF_LEN = 10;

    /**
     * The number of bytes in a pseudo IPv4 header used for computing
     * TCP/UDP checksum.
     */
    private static final int CKSUM_HEADER_SIZE = 12;

    /**
     * An {@link IPv4} packet.
     */
    private IPv4  packet;

    /**
     * The IP protocol number in the IPv4 packet.
     */
    private short  protocol = PROTO_NONE;

    /**
     * Cached values in IPv4 header.
     */
    private Values  values = new Values();

    /**
     * IPv4 header values to be set.
     */
    private Values  modifiedValues;

    /**
     * Set {@code true} if this instance is created by {@link #clone()}.
     */
    private boolean  cloned;

    /**
     * This class describes modifiable fields in IPv4 header.
     */
    private static final class Values implements Cloneable {
        /**
         * The source IP address.
         */
        private Ip4Network  sourceAddress;

        /**
         * The destination IP address.
         */
        private Ip4Network  destinationAddress;

        /**
         * The DSCP field value in the IPv4 packet.
         */
        private short  dscp = DSCP_NONE;

        /**
         * Constructor.
         */
        private Values() {
        }

        /**
         * Return the source IP address.
         *
         * @return  An {@link Ip4Network} instance which represents the
         *          source IP address.
         *          {@code null} is returned if not configured.
         */
        private Ip4Network getSourceAddress() {
            return sourceAddress;
        }

        /**
         * Set the source IP address.
         *
         * @param addr  An {@link Ip4Network} instance which represents the
         *              source IPv4 address.
         */
        private void setSourceAddress(Ip4Network addr) {
            sourceAddress = addr;
        }

        /**
         * Return the destination IP address.
         *
         * @return  An {@link Ip4Network} instance which represents the
         *          destination IP address.
         *          {@code null} is returned if not configured.
         */
        private Ip4Network getDestinationAddress() {
            return destinationAddress;
        }

        /**
         * Set the destination IP address.
         *
         * @param addr  An {@link Ip4Network} instance which represents the
         *              destination IPv4 address.
         */
        private void setDestinationAddress(Ip4Network addr) {
            destinationAddress = addr;
        }

        /**
         * Return the DSCP field value.
         *
         * @return  A short value which indicates the DSCP field value.
         *          {@link Inet4Packet#DSCP_NONE} is returned if not
         *          configured.
         */
        private short getDscp() {
            return dscp;
        }

        /**
         * Set the DSCP field value.
         *
         * @param value  A short value which indicates the DSCP field value.
         */
        private void setDscp(short value) {
            dscp = value;
        }

        /**
         * Fetch all modifiable field values from the given packet.
         *
         * <p>
         *   Field values already cached in this instance are preserved.
         * </p>
         *
         * @param ipv4  An {@link IPv4} instance.
         */
        private void fill(IPv4 ipv4) {
            if (sourceAddress == null) {
                sourceAddress = ipv4.getSourceAddress();
            }
            if (destinationAddress == null) {
                destinationAddress = ipv4.getDestinationAddress();
            }
            if (dscp == DSCP_NONE) {
                dscp = ipv4.getDiffServ();
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
     * Construct a new instance.
     *
     * @param ipv4  An {@link IPv4} instance.
     */
    public Inet4Packet(IPv4 ipv4) {
        packet = ipv4;
    }

    /**
     * Determine whether the source or destination address is modified or not.
     *
     * @return  {@code true} only if the source or destination address is
     *          modified.
     */
    public boolean isAddressModified() {
        if (modifiedValues == null) {
            return false;
        }

        Ip4Network oldIp = values.getSourceAddress();
        Ip4Network newIp = modifiedValues.getSourceAddress();
        if (oldIp.getAddress() != newIp.getAddress()) {
            return true;
        }

        oldIp = values.getDestinationAddress();
        newIp = modifiedValues.getDestinationAddress();
        return (oldIp.getAddress() != newIp.getAddress());
    }

    /**
     * Create a pseudo IPv4 header used for computing TCP/UDP checksum.
     *
     * @param proto  An IP protocol number.
     * @param len    The number of octets in a payload.
     * @return  A byte array which represents the pseudo IPv4 header.
     */
    public byte[] getHeaderForChecksum(byte proto, short len) {
        byte[] header = new byte[CKSUM_HEADER_SIZE];

        int src = getSourceAddress().getAddress();
        int dst = getDestinationAddress().getAddress();
        NumberUtils.setInt(header, CKSUM_OFF_SRC, src);
        NumberUtils.setInt(header, CKSUM_OFF_DST, dst);
        header[CKSUM_OFF_PROTO] = proto;
        NumberUtils.setShort(header, CKSUM_OFF_LEN, len);

        return header;
    }

    /**
     * Construct match fields to test IP header in this packet.
     *
     * <p>
     *   Note that this method creates match fields that matches the original
     *   packet. Any modification to the packet is ignored.
     * </p>
     *
     * @param fields  A set of {@link FlowMatchType} instances corresponding to
     *                match fields to be tested.
     * @return  A {@link VTNInet4Match} instance.
     * @throws RpcException  This packet is broken.
     */
    public VTNInet4Match createMatch(Set<FlowMatchType> fields)
        throws RpcException {
        Values v = values;
        v.fill(packet);

        Ip4Network src = (fields.contains(FlowMatchType.IP_SRC))
            ? v.getSourceAddress()
            : null;
        Ip4Network dst = (fields.contains(FlowMatchType.IP_DST))
            ? v.getDestinationAddress()
            : null;
        Short proto = (fields.contains(FlowMatchType.IP_PROTO))
            ? Short.valueOf(getProtocol())
            : null;
        Short dscp = (fields.contains(FlowMatchType.IP_DSCP))
            ? Short.valueOf(v.getDscp())
            : null;

        return new VTNInet4Match(src, dst, proto, dscp);
    }

    /**
     * Return a {@link Values} instance that keeps current values for
     * IPv4 header fields.
     *
     * @return  A {@link Values} instance.
     */
    private Values getValues() {
        return (modifiedValues == null) ? values : modifiedValues;
    }

    /**
     * Return a {@link Values} instance that keeps IPv4 header field values to
     * be set.
     *
     * @return  A {@link Values} instance.
     */
    private Values getModifiedValues() {
        if (modifiedValues == null) {
            values.fill(packet);
            modifiedValues = values.clone();
        }

        return modifiedValues;
    }

    /**
     * Return an {@link IPv4} instance to set modified values.
     *
     * @return  An {@link IPv4} instance.
     */
    private IPv4 getPacketForWrite() {
        if (cloned) {
            // Create a copy of the original packet.
            packet = packet.clone();
            cloned = false;
        }

        return packet;
    }

    // CachedPacket

    /**
     * Return an {@link IPv4} instance configured in this instance.
     *
     * <p>
     *   Note that modification to the IPv4 header is not applied to the
     *   returned until {@link #commit(PacketContext)} is called.
     * </p>
     *
     * @return  An {@link IPv4} instance.
     */
    @Override
    public IPv4 getPacket() {
        return packet;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean commit(PacketContext pctx) throws VTNException {
        boolean mod = false;
        IPv4 ipv4 = null;
        if (modifiedValues != null) {
            // At least one flow action that modifies IPv4 header is
            // configured.
            pctx.addMatchField(FlowMatchType.DL_TYPE);

            Ip4Network oldIp = values.getSourceAddress();
            Ip4Network newIp = modifiedValues.getSourceAddress();
            if (oldIp.getAddress() != newIp.getAddress()) {
                // Source address was modified.
                ipv4 = getPacketForWrite();
                ipv4.setSourceAddress(newIp);
                mod = true;
            } else if (pctx.hasMatchField(FlowMatchType.IP_SRC)) {
                // Source IP address in the original packet is unchanged and
                // it will be specified in flow match. So we don't need to
                // configure SET_NW_SRC action.
                pctx.removeFilterAction(VTNSetInetSrcAction.class);
            }

            oldIp = values.getDestinationAddress();
            newIp = modifiedValues.getDestinationAddress();
            if (oldIp.getAddress() != newIp.getAddress()) {
                // Destination address was modified.
                if (ipv4 == null) {
                    ipv4 = getPacketForWrite();
                }
                ipv4.setDestinationAddress(newIp);
                mod = true;
            } else if (pctx.hasMatchField(FlowMatchType.IP_DST)) {
                // Destination IP address in the original packet is unchanged
                // and it will be specified in flow match. So we don't need to
                // configure SET_NW_DST action.
                pctx.removeFilterAction(VTNSetInetDstAction.class);
            }

            short dscp = modifiedValues.getDscp();
            if (values.getDscp() != dscp) {
                // DSCP field was modified.
                if (ipv4 == null) {
                    ipv4 = getPacketForWrite();
                }
                ipv4.setDiffServ((byte)dscp);
                mod = true;
            } else if (pctx.hasMatchField(FlowMatchType.IP_DSCP)) {
                // DSCP value in the original packet is unchanged and it will
                // be specified in flow match. So we don't need to configure
                // SET_NW_TOS action.
                pctx.removeFilterAction(VTNSetInetDscpAction.class);
            }
        }

        return mod;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Inet4Packet clone() {
        try {
            Inet4Packet ip = (Inet4Packet)super.clone();
            Values v = ip.values;
            ip.values = v.clone();

            v = ip.modifiedValues;
            if (v != null) {
                ip.modifiedValues = v.clone();
            }
            ip.cloned = true;

            return ip;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }

    // InetHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public Ip4Network getSourceAddress() {
        Values v = getValues();
        Ip4Network ipn = v.getSourceAddress();
        if (ipn == null) {
            ipn = packet.getSourceAddress();
            v.setSourceAddress(ipn);
        }

        return ipn;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean setSourceAddress(IpNetwork ipn) {
        boolean result;
        Ip4Network ip4 = Ip4Network.toIp4Address(ipn);
        if (ip4 == null) {
            result = false;
        } else {
            Values v = getModifiedValues();
            v.setSourceAddress(ip4);
            result = true;
        }

        return result;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Ip4Network getDestinationAddress() {
        Values v = getValues();
        Ip4Network ipn = v.getDestinationAddress();
        if (ipn == null) {
            ipn = packet.getDestinationAddress();
            v.setDestinationAddress(ipn);
        }

        return ipn;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean setDestinationAddress(IpNetwork ipn) {
        boolean result;
        Ip4Network ip4 = Ip4Network.toIp4Address(ipn);
        if (ip4 == null) {
            result = false;
        } else {
            Values v = getModifiedValues();
            v.setDestinationAddress(ip4);
            result = true;
        }

        return result;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public short getProtocol() {
        if (protocol == PROTO_NONE) {
            protocol = (short)NumberUtils.getUnsigned(packet.getProtocol());
        }

        return protocol;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public short getDscp() {
        Values v = getValues();
        short dscp = v.getDscp();
        if (dscp == DSCP_NONE) {
            dscp = (short)NumberUtils.getUnsigned(packet.getDiffServ());
            v.setDscp(dscp);
        }

        return dscp;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDscp(short dscp) {
        Values v = getModifiedValues();
        v.setDscp(dscp);
    }

    // ProtocolHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDescription(StringBuilder builder) {
        Ip4Network src = getSourceAddress();
        Ip4Network dst = getDestinationAddress();
        int proto = (int)getProtocol();
        int dscp = (int)getDscp();
        builder.append("Inet4[src=").append(src.getText()).
            append(",dst=").append(dst.getText()).
            append(",proto=").append(proto).
            append(",dscp=").append(dscp).append(']');
    }
}
