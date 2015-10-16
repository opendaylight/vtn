/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import java.util.Set;

import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.packet.IEEE8021Q;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * {@code EtherPacket} class implements a cache for a {@link Ethernet}
 * instance including VLAN tag.
 */
public final class EtherPacket implements CachedPacket, EtherHeader {
    /**
     * A pseudo VLAN priority which represents the VLAN priority is not
     * specified.
     */
    private static final short VLANPRI_NONE = -1;

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
         * The source MAC address.
         */
        private EtherAddress  sourceAddress;

        /**
         * The destination MAC address.
         */
        private EtherAddress  destinationAddress;

        /**
         * The VLAN ID.
         */
        private int  vlan;

        /**
         * The VLAN priority.
         */
        private short  vlanPriority = VLANPRI_NONE;

        /**
         * Constructor.
         *
         * @param vid  VLAN ID in the original Ethernet frame.
         */
        private Values(short vid) {
            vlan = vid;
        }

        /**
         * Return the source MAC address .
         *
         * @return  An {@link EtherAddress} instance.
         *          {@code null} is returned if not configured.
         */
        private EtherAddress getSourceAddress() {
            return sourceAddress;
        }

        /**
         * Set the source MAC address.
         *
         * @param addr  An {@link EtherAddress} instance.
         */
        private void setSourceAddress(EtherAddress addr) {
            sourceAddress = addr;
        }

        /**
         * Return the destination MAC address.
         *
         * @return  An {@link EtherAddress} instance.
         *          {@code null} is returned if not configured.
         */
        private EtherAddress getDestinationAddress() {
            return destinationAddress;
        }

        /**
         * Set the destination MAC address.
         *
         * @param addr  An {@link EtherAddress} instance.
         */
        private void setDestinationAddress(EtherAddress addr) {
            destinationAddress = addr;
        }

        /**
         * Return the VLAN ID.
         *
         * @return  A VLAN ID.
         */
        private int getVlan() {
            return vlan;
        }

        /**
         * Set the VLAN ID.
         *
         * @param vid  A VLAN ID.
         */
        private void setVlan(int vid) {
            vlan = vid;
        }

        /**
         * Return the VLAN priority.
         *
         * @return  A byte value which represents the VLAN priority.
         *          {@link EtherPacket#VLANPRI_NONE} is returned if not
         *          configured.
         */
        private short getVlanPriority() {
            return vlanPriority;
        }

        /**
         * Set the VLAN priority.
         *
         * @param pri  A VLAN priority.
         */
        private void setVlanPriority(short pri) {
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
                sourceAddress = new EtherAddress(ether.getSourceMACAddress());
            }
            if (destinationAddress == null) {
                destinationAddress =
                    new EtherAddress(ether.getDestinationMACAddress());
            }
            if (vlanPriority == VLANPRI_NONE && tag != null) {
                vlanPriority = (short)NumberUtils.getUnsigned(tag.getPcp());
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
            vid = VLAN_NONE;
        }

        values = new Values(vid);
        etherType = NumberUtils.getUnsigned(ethType);
        payload = pld;
        rawPayload = parent.getRawPayload();
    }

    /**
     * Return the VLAN ID in the original Ethernet frame.
     *
     * @return  A short value which represents the VLAN ID.
     *          Zero is returned if no VLAN tag was found in the original
     *          Ethernet frame.
     */
    public int getOriginalVlan() {
        return values.getVlan();
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
     * Construct match fields to test Ethernet header in this packet.
     *
     * <p>
     *   Note that this method creates match fields that matches the original
     *   packet. Any modification to the packet is ignored.
     * </p>
     *
     * @param fields  A set of {@link FlowMatchType} instances corresponding to
     *                match fields to be tested.
     * @return  A {@link VTNEtherMatch} instance.
     * @throws RpcException  This packet is broken.
     */
    public VTNEtherMatch createMatch(Set<FlowMatchType> fields)
        throws RpcException {
        Values v = values;
        v.fill(packet, vlanTag);

        // VLAN ID field is mandatory.
        int vid = v.getVlan();

        EtherAddress src = (fields.contains(FlowMatchType.DL_SRC))
            ? v.getSourceAddress()
            : null;
        EtherAddress dst = (fields.contains(FlowMatchType.DL_DST))
            ? v.getDestinationAddress()
            : null;
        Integer type = (fields.contains(FlowMatchType.DL_TYPE))
            ? Integer.valueOf(etherType)
            : null;

        // Test VLAN priority only if this packet has a VLAN tag.
        Short pcp = (vid != VLAN_NONE &&
                     fields.contains(FlowMatchType.DL_VLAN_PCP))
            ? v.getVlanPriority()
            : null;

        return new VTNEtherMatch(src, dst, type, vid, pcp);
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
     * {@inheritDoc}
     */
    @Override
    public boolean commit(PacketContext pctx) {
        // We don't need to set modified values to Ethernet and IEEE8021Q
        // instances because PacketContext always creates Ethernet header and
        // VLAN tag from scratch.
        boolean mod = false;
        if (modifiedValues != null) {
            EtherAddress src = modifiedValues.getSourceAddress();
            if (!src.equals(values.getSourceAddress())) {
                // Source MAC address was modified.
                mod = true;
            } else if (pctx.hasMatchField(FlowMatchType.DL_SRC)) {
                // Source MAC address is not modified, and it will be specified
                // in flow match. So we don't need to configure SET_DL_SRC
                // action.
                pctx.removeFilterAction(VTNSetDlSrcAction.class);
            }

            EtherAddress dst = modifiedValues.getDestinationAddress();
            if (!dst.equals(values.getDestinationAddress())) {
                // Destination MAC address was modified.
                mod = true;
            } else if (pctx.hasMatchField(FlowMatchType.DL_DST)) {
                // Destination MAC address is not modified, and it will be
                // specified in flow match. So we don't need to configure
                // SET_DL_DST action.
                pctx.removeFilterAction(VTNSetDlDstAction.class);
            }

            int vlan = modifiedValues.getVlan();
            if (vlan == VLAN_NONE) {
                // SET_VLAN_PCP should never be applied to untagged frame.
                pctx.removeFilterAction(VTNSetVlanPcpAction.class);
            } else {
                short pri = modifiedValues.getVlanPriority();
                if (values.getVlanPriority() != pri) {
                    // VLAN priority was modified.
                    mod = true;
                } else if (pctx.hasMatchField(FlowMatchType.DL_VLAN_PCP)) {
                    // VLAN priority is not modified, and it will be specified
                    // in flow match. So we don't need to configure
                    // SET_VLAN_PCP action.
                    pctx.removeFilterAction(VTNSetVlanPcpAction.class);
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

    // EtherHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public EtherAddress getSourceAddress() {
        Values v = getValues();
        EtherAddress addr = v.getSourceAddress();
        if (addr == null) {
            addr = new EtherAddress(packet.getSourceMACAddress());
            v.setSourceAddress(addr);
        }

        return addr;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setSourceAddress(EtherAddress mac) {
        getModifiedValues().setSourceAddress(mac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public EtherAddress getDestinationAddress() {
        Values v = getValues();
        EtherAddress addr = v.getDestinationAddress();
        if (addr == null) {
            addr = new EtherAddress(packet.getDestinationMACAddress());
            v.setDestinationAddress(addr);
        }

        return addr;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDestinationAddress(EtherAddress mac) {
        getModifiedValues().setDestinationAddress(mac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getEtherType() {
        return etherType;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getVlanId() {
        // VLAN ID is always cached.
        return getValues().getVlan();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setVlanId(int vid) {
        getModifiedValues().setVlan(vid);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public short getVlanPriority() {
        Values v = getValues();
        short pri = v.getVlanPriority();
        if (pri == VLANPRI_NONE && vlanTag != null) {
            pri = (short)NumberUtils.getUnsigned(vlanTag.getPcp());
            v.setVlanPriority(pri);
        }

        return pri;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setVlanPriority(short pcp) {
        getModifiedValues().setVlanPriority(pcp);
    }

    // ProtocolHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDescription(StringBuilder builder) {
        EtherAddress src = getSourceAddress();
        EtherAddress dst = getDestinationAddress();
        builder.append("Ether[src=").append(src.getText()).
            append(",dst=").append(dst.getText()).
            append(",type=0x").append(Integer.toHexString(etherType));

        int vid = getVlanId();
        if (vid != VLAN_NONE) {
            builder.append(",vlan={id=").append(vid).
                append(",pcp=").append((int)getVlanPriority()).append('}');
        }
        builder.append(']');
    }
}
