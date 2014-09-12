/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.net.InetAddress;
import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.MiscUtils;
import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.action.SetNwTos;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code Inet4Packet} class implements a cache for an {@link IPv4} instance.
 */
public final class Inet4Packet implements CachedPacket {
    /**
     * A pseudo IP protocol number which indicates the IP protocol number is
     * not specified.
     */
    private static final short  PROTO_NONE = -1;

    /**
     * A pseudo DSCP value which indicates the DSCP value is not specified.
     */
    private static final byte  DSCP_NONE = -1;

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
         * An integer value which indicates the source IP address.
         */
        private int  sourceAddress;

        /**
         * An integer value which indicates the destination IP address.
         */
        private int  destinationAddress;

        /**
         * An {@link InetAddress} instance which indicates the source IP
         * address.
         */
        private InetAddress  sourceInetAddress;

        /**
         * An {@link InetAddress} instance which indicates the destination IP
         * address.
         */
        private InetAddress  destinationInetAddress;

        /**
         * The DSCP field value in the IPv4 packet.
         */
        private byte  dscp = DSCP_NONE;

        /**
         * Constructor.
         */
        private Values() {
        }

        /**
         * Return the source IP address.
         *
         * @return  An integer value which represents the source IP address.
         *          Note that the returned value is unspecified if
         *          {@link #getSourceInetAddress()} returns {@code null}.
         */
        private int getSourceAddress() {
            return sourceAddress;
        }

        /**
         * Return the source IP address.
         *
         * @return  An {@link InetAddress} instance which represents the
         *          source IP address.
         *         {@code null} is returned if not configured.
         */
        private InetAddress getSourceInetAddress() {
            return sourceInetAddress;
        }

        /**
         * Set the source IP address.
         *
         * @param addr  An {@link InetAddress} instance which represents the
         *              source IPv4 address.
         */
        private void setSourceAddress(InetAddress addr) {
            sourceAddress = MiscUtils.toInteger(addr);
            sourceInetAddress = addr;
        }

        /**
         * Set the source IP address.
         *
         * @param addr  An integer value which represents the source IPv4
         *              address.
         * @return  An {@link InetAddress} instance which represents the
         *          given IP address.
         */
        private InetAddress setSourceAddress(int addr) {
            sourceAddress = addr;
            sourceInetAddress = MiscUtils.toInetAddress(addr);
            return sourceInetAddress;
        }

        /**
         * Return the destination IP address.
         *
         * @return  An integer value which represents the destination IP
         *          address.
         *          Note that the returned value is unspecified if
         *          {@link #getDestinationInetAddress()} returns {@code null}.
         */
        private int getDestinationAddress() {
            return destinationAddress;
        }

        /**
         * Return the destination IP address.
         *
         * @return  An {@link InetAddress} instance which represents the
         *          destination IP address.
         *         {@code null} is returned if not configured.
         */
        private InetAddress getDestinationInetAddress() {
            return destinationInetAddress;
        }

        /**
         * Set the destination IP address.
         *
         * @param addr  An {@link InetAddress} instance which represents the
         *              destination IPv4 address.
         */
        private void setDestinationAddress(InetAddress addr) {
            destinationAddress = MiscUtils.toInteger(addr);
            destinationInetAddress = addr;
        }

        /**
         * Set the destination IP address.
         *
         * @param addr  An integer value which represents the destination IPv4
         *              address.
         * @return  An {@link InetAddress} instance which represents the
         *          given IP address.
         */
        private InetAddress setDestinationAddress(int addr) {
            destinationAddress = addr;
            destinationInetAddress = MiscUtils.toInetAddress(addr);
            return destinationInetAddress;
        }

        /**
         * Return the DSCP field value.
         *
         * @return  A byte value which indicates the DSCP field value.
         *          {@link Inet4Packet#DSCP_NONE} is returned if not
         *          configured.
         */
        private byte getDscp() {
            return dscp;
        }

        /**
         * Set the DSCP field value.
         *
         * @param value  A byte value which indicates the DSCP field value.
         */
        private void setDscp(byte value) {
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
            if (sourceInetAddress == null) {
                setSourceAddress(ipv4.getSourceAddress());
            }
            if (destinationInetAddress == null) {
                setDestinationAddress(ipv4.getDestinationAddress());
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
     * Return the source IP address.
     *
     * @return  An integer value which represents the source IP address.
     */
    public int getSourceAddress() {
        Values v = getValues();
        int addr;
        if (v.getSourceInetAddress() == null) {
            addr = packet.getSourceAddress();
            v.setSourceAddress(addr);
        } else {
            addr = v.getSourceAddress();
        }

        return addr;
    }

    /**
     * Return the source IP address.
     *
     * @return  An {@link InetAddress} instance which represents the source
     *          IP address.
     */
    public InetAddress getSourceInetAddress() {
        Values v = getValues();
        InetAddress addr = v.getSourceInetAddress();
        if (addr == null) {
            int address = packet.getSourceAddress();
            addr = v.setSourceAddress(address);
        }

        return addr;
    }

    /**
     * Set the source IP address.
     *
     * @param addr  An {@link InetAddress} instance which represents the
     *              source IPv4 address.
     */
    public void setSourceAddress(InetAddress addr) {
        Values v = getModifiedValues();
        v.setSourceAddress(addr);
    }

    /**
     * Return the destination IP address.
     *
     * @return  An integer value which represents the destination IP address.
     */
    public int getDestinationAddress() {
        Values v = getValues();
        int addr;
        if (v.getDestinationInetAddress() == null) {
            addr = packet.getDestinationAddress();
            v.setDestinationAddress(addr);
        } else {
            addr = v.getDestinationAddress();
        }

        return addr;
    }

    /**
     * Return the destination IP address.
     *
     * @return  An {@link InetAddress} instance which represents the
     *          destination IP address.
     */
    public InetAddress getDestinationInetAddress() {
        Values v = getValues();
        InetAddress addr = v.getDestinationInetAddress();
        if (addr == null) {
            int address = packet.getDestinationAddress();
            addr = v.setDestinationAddress(address);
        }

        return addr;
    }

    /**
     * Set the destination IP address.
     *
     * @param addr  An {@link InetAddress} instance which represents the
     *              destination IPv4 address.
     */
    public void setDestinationAddress(InetAddress addr) {
        Values v = getModifiedValues();
        v.setDestinationAddress(addr);
    }

    /**
     * Return the IP protocol number.
     *
     * @return  A short integer value which indicates the IP protocol number.
     */
    public short getProtocol() {
        if (protocol == PROTO_NONE) {
            protocol = (short)NetUtils.getUnsignedByte(packet.getProtocol());
        }

        return protocol;
    }

    /**
     * Return the DSCP field value.
     *
     * @return  A byte value which indicates the DSCP field value.
     */
    public byte getDscp() {
        Values v = getValues();
        byte dscp = v.getDscp();
        if (dscp == DSCP_NONE) {
            dscp = packet.getDiffServ();
            v.setDscp(dscp);
        }

        return dscp;
    }

    /**
     * Set the DSCP field value.
     *
     * @param dscp  A byte value which indicates the DSCP field value.
     */
    public void setDscp(byte dscp) {
        Values v = getModifiedValues();
        v.setDscp(dscp);
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

        return (values.getSourceAddress() !=
                modifiedValues.getSourceAddress() ||
                values.getDestinationAddress() !=
                modifiedValues.getDestinationAddress());
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

        int src = getSourceAddress();
        int dst = getDestinationAddress();
        MiscUtils.setInt(header, CKSUM_OFF_SRC, src);
        MiscUtils.setInt(header, CKSUM_OFF_DST, dst);
        header[CKSUM_OFF_PROTO] = proto;
        MiscUtils.setShort(header, CKSUM_OFF_LEN, len);

        return header;
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
     * @throws VTNException
     *    Failed to copy the packet.
     */
    private IPv4 getPacketForWrite() throws VTNException {
        if (cloned) {
            // Create a copy of the original packet.
            packet = MiscUtils.copy(packet, new IPv4());
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
     * Configure match fields to test IP header in this packet.
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
    public void setMatch(Match match, Set<MatchType> fields) {
        Values v = values;
        v.fill(packet);

        MatchType type = MatchType.NW_SRC;
        if (fields.contains(type)) {
            // Test source IP address.
            match.setField(type, v.getSourceInetAddress());
        }

        type = MatchType.NW_DST;
        if (fields.contains(type)) {
            // Test destination IP address.
            match.setField(type, v.getDestinationInetAddress());
        }

        type = MatchType.NW_PROTO;
        if (fields.contains(type)) {
            // Test IP protocol number.
            match.setField(type, (byte)getProtocol());
        }

        type = MatchType.NW_TOS;
        if (fields.contains(type)) {
            // Test DSCP field.
            match.setField(type, v.getDscp());
        }
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
            pctx.addMatchField(MatchType.DL_TYPE);

            if (values.getSourceAddress() !=
                modifiedValues.getSourceAddress()) {
                // Source address was modified.
                InetAddress iaddr = modifiedValues.getSourceInetAddress();
                ipv4 = getPacketForWrite();
                ipv4.setSourceAddress(iaddr);
                mod = true;
            } else if (pctx.hasMatchField(MatchType.NW_SRC)) {
                // Source IP address in the original packet is unchanged and
                // it will be specified in flow match. So we don't need to
                // configure SET_NW_SRC action.
                pctx.removeFilterAction(SetNwSrc.class);
            }

            if (values.getDestinationAddress() !=
                modifiedValues.getDestinationAddress()) {
                // Destination address was modified.
                InetAddress iaddr = modifiedValues.getDestinationInetAddress();
                if (ipv4 == null) {
                    ipv4 = getPacketForWrite();
                }
                ipv4.setDestinationAddress(iaddr);
                mod = true;
            } else if (pctx.hasMatchField(MatchType.NW_DST)) {
                // Destination IP address in the original packet is unchanged
                // and it will be specified in flow match. So we don't need to
                // configure SET_NW_DST action.
                pctx.removeFilterAction(SetNwDst.class);
            }

            byte dscp = modifiedValues.getDscp();
            if (values.getDscp() != dscp) {
                // DSCP field was modified.
                if (ipv4 == null) {
                    ipv4 = getPacketForWrite();
                }
                ipv4.setDiffServ(dscp);
                mod = true;
            } else if (pctx.hasMatchField(MatchType.NW_TOS)) {
                // DSCP value in the original packet is unchanged and it will
                // be specified in flow match. So we don't need to configure
                // SET_NW_TOS action.
                pctx.removeFilterAction(SetNwTos.class);
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
}
