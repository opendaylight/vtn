/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.action.SetVlanPcp;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;

/**
 * {@code EtherPacket} class implements a cache for a {@link Ethernet}
 * instance including VLAN tag.
 */
public final class EtherPacket implements CachedPacket {
    /**
     * A pseudo MAC address which represents the MAC address is not specified.
     */
    private static final long  MAC_NONE = -1L;

    /**
     * A pseudo VLAN priority which represents the VLAN priority is not
     * specified.
     */
    private static final byte VLANPRI_NONE = -1;

    /**
     * An {@link Ethernet} instance.
     */
    private final Ethernet  packet;

    /**
     * An {@link IEEE8021Q} instance which represents a VLAN tag.
     */
    private final IEEE8021Q  vlanTag;

    /**
     * The ethernet type.
     */
    private final int  etherType;

    /**
     * Payload of the Ethernet frame.
     */
    private Packet  payload;

    /**
     * Unparsed payload of the Ethernet frame.
     */
    private final byte[]  rawPayload;

    /**
     * Cached values in Ethernet header.
     */
    private Values  values;

    /**
     * Ethernet header values to be set.
     */
    private Values  modifiedValues;

    /**
     * This class describes modifiable fields in Ethernet header.
     */
    private static final class Values implements Cloneable {
        /**
         * A byte array which represents the source MAC address.
         */
        private byte[]  sourceAddress;

        /**
         * A byte array which represents the destination MAC address.
         */
        private byte[]  destinationAddress;

        /**
         * A long value which represents the source MAC address.
         */
        private long  sourceMac = MAC_NONE;

        /**
         * A long value which represents the destination MAC address.
         */
        private long  destinationMac = MAC_NONE;

        /**
         * The VLAN ID.
         */
        private short  vlan;

        /**
         * The VLAN priority.
         */
        private byte  vlanPriority = VLANPRI_NONE;

        /**
         * Constructor.
         *
         * @param vid  VLAN ID in the original Ethernet frame.
         */
        private Values(short vid) {
            vlan = vid;
        }

        /**
         * Return the source MAC address as a byte array.
         *
         * @return  The source MAC address.
         *          {@code null} is returned if not configured.
         */
        private byte[] getSourceAddress() {
            return sourceAddress;
        }

        /**
         * Set the source MAC address.
         *
         * @param addr  A byte array that represents the source MAC address.
         */
        private void setSourceAddress(byte[] addr) {
            sourceAddress = addr.clone();
            sourceMac = EtherAddress.toLong(sourceAddress);
        }

        /**
         * Return the destination MAC address as a byte array.
         *
         * @return  The destination MAC address.
         *          {@code null} is returned if not configured.
         */
        private byte[] getDestinationAddress() {
            return destinationAddress;
        }

        /**
         * Set the destination MAC address.
         *
         * @param addr  A byte array that represents the destination
         *              MAC address.
         */
        private void setDestinationAddress(byte[] addr) {
            destinationAddress = addr.clone();
            destinationMac = EtherAddress.toLong(destinationAddress);
        }

        /**
         * Return the source MAC address as a long integer.
         *
         * @return  The source MAC address.
         *          {@link EtherPacket#MAC_NONE} is returned if not configured.
         */
        private long getSourceMacAddress() {
            return sourceMac;
        }

        /**
         * Return the destination MAC address as a long integer.
         *
         * @return  The destination MAC address.
         *          {@link EtherPacket#MAC_NONE} is returned if not configured.
         */
        private long getDestinationMacAddress() {
            return destinationMac;
        }

        /**
         * Return the VLAN ID.
         *
         * @return  A VLAN ID.
         */
        private short getVlan() {
            return vlan;
        }

        /**
         * Set the VLAN ID.
         *
         * @param vid  A VLAN ID.
         */
        private void setVlan(short vid) {
            vlan = vid;
        }

        /**
         * Return the VLAN priority.
         *
         * @return  A byte value which represents the VLAN priority.
         *          {@link EtherPacket#VLANPRI_NONE} is returned if not
         *          configured.
         */
        private byte getVlanPriority() {
            return vlanPriority;
        }

        /**
         * Set the VLAN priority.
         *
         * @param pri  A VLAN priority.
         */
        private void setVlanPriority(byte pri) {
            vlanPriority = pri;
        }

        /**
         * Fetch all modifiable field values from the given packet.
         *
         * <p>
         *   Field values already cached in this instance are preserved.
         * </p>
         *
         * @param ether  An {@link Ethernet} instance.
         * @param tag    An {@link IEEE8021Q} instance.
         */
        private void fill(Ethernet ether, IEEE8021Q tag) {
            if (sourceAddress == null) {
                setSourceAddress(ether.getSourceMACAddress());
            }
            if (destinationAddress == null) {
                setDestinationAddress(ether.getDestinationMACAddress());
            }
            if (vlanPriority == VLANPRI_NONE && tag != null) {
                vlanPriority = tag.getPcp();
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
     * @param ether  An {@link Ethernet} instance.
     */
    public EtherPacket(Ethernet ether) {
        packet = ether;

        Packet parent = ether;
        Packet pld = ether.getPayload();
        short ethType;
        short vid;
        if (pld instanceof IEEE8021Q) {
            // This packet has a VLAN tag.
            vlanTag = (IEEE8021Q)pld;
            pld = vlanTag.getPayload();
            ethType = vlanTag.getEtherType();
            parent = vlanTag;
            vid = vlanTag.getVid();
        } else {
            ethType = ether.getEtherType();
            vlanTag = null;
            vid = MatchType.DL_VLAN_NONE;
        }

        values = new Values(vid);
        etherType = NumberUtils.getUnsigned(ethType);
        payload = pld;
        rawPayload = parent.getRawPayload();
    }

    /**
     * Return the source MAC address as a byte array.
     *
     * @return  The source MAC address.
     */
    public byte[] getSourceAddress() {
        Values v = getValues();
        byte[] addr = v.getSourceAddress();
        if (addr == null) {
            addr = packet.getSourceMACAddress();
            v.setSourceAddress(addr);
        }

        return addr;
    }

    /**
     * Set the source MAC address.
     *
     * @param addr  A byte array that represents the source MAC address.
     */
    public void setSourceAddress(byte[] addr) {
        getModifiedValues().setSourceAddress(addr);
    }

    /**
     * Return the destination MAC address as a byte array.
     *
     * @return  The destination MAC address.
     */
    public byte[] getDestinationAddress() {
        Values v = getValues();
        byte[] addr = v.getDestinationAddress();
        if (addr == null) {
            addr = packet.getDestinationMACAddress();
            v.setDestinationAddress(addr);
        }

        return addr;
    }

    /**
     * Set the source MAC address.
     *
     * @param addr  A byte array that represents the source MAC address.
     */
    public void setDestinationAddress(byte[] addr) {
        getModifiedValues().setDestinationAddress(addr);
    }

    /**
     * Return the source MAC address as a long integer.
     *
     * @return  The source MAC address.
     */
    public long getSourceMacAddress() {
        Values v = getValues();
        long mac = v.getSourceMacAddress();
        if (v.getSourceAddress() == null) {
            byte[] addr = packet.getSourceMACAddress();
            v.setSourceAddress(addr);
            mac = v.getSourceMacAddress();
        }

        return mac;
    }

    /**
     * Return the destination MAC address as a long integer.
     *
     * @return  The destination MAC address.
     */
    public long getDestinationMacAddress() {
        Values v = getValues();
        long mac = v.getDestinationMacAddress();
        if (v.getDestinationAddress() == null) {
            byte[] addr = packet.getDestinationMACAddress();
            v.setDestinationAddress(addr);
            mac = v.getDestinationMacAddress();
        }

        return mac;
    }

    /**
     * Return the Ethernet type.
     *
     * @return  An integer value which represents the Ethernet type.
     */
    public int getEtherType() {
        return etherType;
    }

    /**
     * Return the VLAN ID in the original Ethernet frame.
     *
     * @return  A short value which represents the VLAN ID.
     *          Zero is returned if no VLAN tag was found in the original
     *          Ethernet frame.
     */
    public short getOriginalVlan() {
        return values.getVlan();
    }

    /**
     * Return the VLAN ID.
     *
     * @return  A short value which represents the VLAN ID.
     *          Zero is returned if no VLAN tag was found in the Ethernet
     *          frame.
     */
    public short getVlan() {
        // VLAN ID is always cached.
        return getValues().getVlan();
    }

    /**
     * Set the VLAN ID.
     *
     * @param vid  A VLAN ID.
     */
    public void setVlan(short vid) {
        getModifiedValues().setVlan(vid);
    }

    /**
     * Return the VLAN priority.
     *
     * @return  A byte value which represents the VLAN priority.
     *          A negative value is returned if no VLAN tag was found in
     *          the Ethernet frame and no VLAN priority is set by the call of
     *          {@link #setVlanPriority(byte)}.
     */
    public byte getVlanPriority() {
        Values v = getValues();
        byte pri = v.getVlanPriority();
        if (pri == VLANPRI_NONE && vlanTag != null) {
            pri = vlanTag.getPcp();
            v.setVlanPriority(pri);
        }

        return pri;
    }

    /**
     * Set the VLAN priority.
     *
     * @param pri  A VLAN priority.
     */
    public void setVlanPriority(byte pri) {
        getModifiedValues().setVlanPriority(pri);
    }

    /**
     * Return the VLAN tag in the Ethernet frame.
     *
     * <p>
     *   Note that this method returns the VLAN tag in the original Ethernet
     *   frame. Any modification to VLAN tag is never applied to the returned
     *   value even if {@link #commit(PacketContext)} is called.
     * </p>
     *
     * @return  An {@link IEEE8021Q} instance which represents the VLAN tag.
     *          {@code null} is returned if no VLAN tag was found in the
     *          Ethernet frame.
     */
    public IEEE8021Q getVlanTag() {
        return vlanTag;
    }

    /**
     * Return the payload in the Ethernet frame.
     *
     * @return  A {@link Packet} instance which represents the payload.
     */
    public Packet getPayload() {
        return payload;
    }

    /**
     * Set the payload of the Ethernet frame.
     *
     * @param packet  The payload of the Ethernet frame.
     */
    public void setPayload(Packet packet) {
        payload = packet;
    }

    /**
     * Return the unparsed payload in the Ethernet frame.
     *
     * @return  A byte array which represents the payload in the Ethernet
     *          frame.
     */
    public byte[] getRawPayload() {
        return rawPayload;
    }

    /**
     * Return a {@link Values} instance that keeps current values for
     * Ethernet header fields.
     *
     * @return  A {@link Values} instance.
     */
    private Values getValues() {
        return (modifiedValues == null) ? values : modifiedValues;
    }

    /**
     * Return a {@link Values} instance that keeps Ethernet header field values
     * to be set.
     *
     * @return  A {@link Values} instance.
     */
    private Values getModifiedValues() {
        if (modifiedValues == null) {
            values.fill(packet, vlanTag);
            modifiedValues = values.clone();
        }

        return modifiedValues;
    }

    // CachedPacket

    /**
     * Return an {@link Ethernet} instance configured in this instance.
     *
     * <p>
     *   Note that this method returns the original ethernet frame.
     *   Any modification to Ethernet header field is never applied to the
     *   returned value even if {@link #commit(PacketContext)} is called.
     * </p>
     *
     * @return  An {@link Ethernet} instance.
     */
    @Override
    public Ethernet getPacket() {
        return packet;
    }

    /**
     * Configure match fields to test Ethernet header in this packet.
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
        v.fill(packet, vlanTag);

        // VLAN ID field is mandatory.
        // Note that this code expects MatchType.DL_VLAN_NONE is zero.
        short vid = v.getVlan();
        match.setField(MatchType.DL_VLAN, vid);

        MatchType type = MatchType.DL_SRC;
        if (fields.contains(type)) {
            // Test source MAC address.
            match.setField(type, v.getSourceAddress());
        }

        type = MatchType.DL_DST;
        if (fields.contains(type)) {
            // Test destination MAC address.
            match.setField(type, v.getDestinationAddress());
        }

        // Test VLAN priority only if this packet has a VLAN tag.
        if (vid != MatchType.DL_VLAN_NONE) {
            type = MatchType.DL_VLAN_PR;
            if (fields.contains(type)) {
                match.setField(type, v.getVlanPriority());
            }
        }

        type = MatchType.DL_TYPE;
        if (fields.contains(type)) {
            // Test Ethernet type.
            match.setField(type, (short)etherType);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean commit(PacketContext pctx) {
        // We don't need to set modified values to Ethernet and IEEE8021Q
        // instances because PacketContext always creates Ethernet header and
        // VLAN tag from scratch.
        boolean mod = false;
        if (modifiedValues != null) {
            if (values.getSourceMacAddress() !=
                modifiedValues.getSourceMacAddress()) {
                // Source MAC address was modified.
                mod = true;
            } else if (pctx.hasMatchField(MatchType.DL_SRC)) {
                // Source MAC address is not modified, and it will be specified
                // in flow match. So we don't need to configure SET_DL_SRC
                // action.
                pctx.removeFilterAction(SetDlSrc.class);
            }

            if (values.getDestinationMacAddress() !=
                modifiedValues.getDestinationMacAddress()) {
                // Destination MAC address was modified.
                mod = true;
            } else if (pctx.hasMatchField(MatchType.DL_DST)) {
                // Destination MAC address is not modified, and it will be
                // specified in flow match. So we don't need to configure
                // SET_DL_DST action.
                pctx.removeFilterAction(SetDlDst.class);
            }

            short vlan = modifiedValues.getVlan();
            if (vlan == MatchType.DL_VLAN_NONE) {
                // SET_VLAN_PCP should never be applied to untagged frame.
                pctx.removeFilterAction(SetVlanPcp.class);
            } else {
                byte pri = modifiedValues.getVlanPriority();
                if (values.getVlanPriority() != pri) {
                    // VLAN priority was modified.
                    mod = true;
                } else if (pctx.hasMatchField(MatchType.DL_VLAN_PR)) {
                    // VLAN priority is not modified, and it will be specified
                    // in flow match. So we don't need to configure
                    // SET_VLAN_PCP action.
                    pctx.removeFilterAction(SetVlanPcp.class);
                }
            }
        }

        return mod;
    }

    /**
     * Return a deep copy of this instance.
     *
     * <p>
     *   Note that this method does not copy the original Ethernet header and
     *   VLAN tag because this class never modifies them.
     * </p>
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public EtherPacket clone() {
        try {
            EtherPacket ether = (EtherPacket)super.clone();
            Values v = ether.values;
            ether.values = v.clone();

            v = ether.modifiedValues;
            if (v != null) {
                ether.modifiedValues = v.clone();
            }

            return ether;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
