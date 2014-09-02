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
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;

import org.opendaylight.vtn.manager.internal.MiscUtils;
import org.opendaylight.vtn.manager.internal.PacketContext;

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
     * An {@link IPv4} packet.
     */
    private IPv4  packet;

    /**
     * An integer value which indicates the source IP address.
     */
    private int  sourceAddress;

    /**
     * An integer value which indicates the destination IP address.
     */
    private int  destinationAddress;

    /**
     * A boolean value which determines whether {@link #sourceAddress} is
     * initialized or not.
     */
    private boolean sourceSet;

    /**
     * A boolean value which determines whether {@link #destinationAddress} is
     * initialized or not.
     */
    private boolean destinationSet;

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
         * The DSCP field value in the IPv4 packet.
         */
        private byte  dscp = DSCP_NONE;

        // REVISIT: Source and destination IP address are not yet supported.

        /**
         * Constructor.
         */
        private Values() {
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
            if (dscp != DSCP_NONE) {
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
        if (!sourceSet) {
            sourceAddress = packet.getSourceAddress();
            sourceSet = true;
        }

        return sourceAddress;
    }

    /**
     * Return the destination IP address.
     *
     * @return  An integer value which represents the destination IP address.
     */
    public int getDestinationAddress() {
        if (!destinationSet) {
            destinationAddress = packet.getDestinationAddress();
            destinationSet = true;
        }

        return destinationAddress;
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
            try {
                byte[] raw = packet.serialize();
                int nbits = raw.length * NetUtils.NumBitsInAByte;
                packet = new IPv4();
                packet.deserialize(raw, 0, nbits);
            } catch (Exception e) {
                // This should never happen.
                throw new VTNException("Failed to copy the packet.", e);
            }

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
            match.setField(type, MiscUtils.toInetAddress(getSourceAddress()));
        }

        type = MatchType.NW_DST;
        if (fields.contains(type)) {
            // Test destination IP address.
            match.setField(type,
                           MiscUtils.toInetAddress(getDestinationAddress()));
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
        if (modifiedValues != null) {
            byte dscp = modifiedValues.getDscp();
            if (values.getDscp() != dscp) {
                // DSCP field was modified.
                // Note that this action must be applied to only IPv4 packets.
                int tos = (int)SetDscpAction.dscpToTos(dscp);
                pctx.addFilterAction(new SetNwTos(tos));
                pctx.addMatchField(MatchType.DL_TYPE);
                IPv4 ipv4 = getPacketForWrite();
                ipv4.setDiffServ(dscp);
                mod = true;
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
