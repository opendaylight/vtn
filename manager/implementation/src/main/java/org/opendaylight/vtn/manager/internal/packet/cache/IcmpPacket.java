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

import org.opendaylight.vtn.manager.packet.ICMP;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpCodeAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpTypeAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNIcmpMatch;
import org.opendaylight.vtn.manager.internal.util.packet.IcmpHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * {@code IcmpPacket} class implements a cache for an {@link ICMP} instance.
 */
public final class IcmpPacket implements L4Packet, IcmpHeader {
    /**
     * A pseudo short value which indicates the byte value is not specified.
     */
    private static final short  VALUE_NONE = -1;

    /**
     * An {@link ICMP} packet.
     */
    private ICMP  packet;

    /**
     * Cached values in ICMP header.
     */
    private Values  values = new Values();

    /**
     * ICMP header values to be set.
     */
    private Values  modifiedValues;

    /**
     * Set {@code true} if this instance is created by {@link #clone()}.
     */
    private boolean  cloned;

    /**
     * This class describes modifiable fields in ICMPv4 hedaer.
     */
    private static final class Values implements Cloneable {
        /**
         * The ICMP type.
         */
        private short  type = VALUE_NONE;

        /**
         * The ICMP code.
         */
        private short  code = VALUE_NONE;

        /**
         * Constructor.
         */
        private Values() {
        }

        /**
         * Return the ICMP type.
         *
         * @return  A short integer value which indicates the ICMP type.
         *          {@link IcmpPacket#VALUE_NONE} is returned if not
         *           configured.
         */
        private short getType() {
            return type;
        }

        /**
         * Set the ICMP type.
         *
         * @param value  A short integer value which indicates the ICMP type.
         */
        private void setType(short value) {
            type = value;
        }

        /**
         * Set the ICMP type.
         *
         * @param value  A byte integer value which indicates the ICMP type.
         * @return  A short value which represents the given ICMP type.
         */
        private short setType(byte value) {
            type = (short)NumberUtils.getUnsigned(value);
            return type;
        }

        /**
         * Return the ICMP code.
         *
         * @return  A short integer value which indicates the ICMP code.
         *          {@link IcmpPacket#VALUE_NONE} is returned if not
         *           configured.
         */
        private short getCode() {
            return code;
        }

        /**
         * Set the ICMP code.
         *
         * @param value  A short integer value which indicates the ICMP code.
         */
        private void setCode(short value) {
            code = value;
        }

        /**
         * Set the ICMP code.
         *
         * @param value  A byte integer value which indicates the ICMP code.
         * @return  A short value which represents the given ICMP code.
         */
        private short setCode(byte value) {
            code = (short)NumberUtils.getUnsigned(value);
            return code;
        }

        /**
         * Fetch all modifiable field values from the given packet.
         *
         * <p>
         *   Field values already cached in this instance are preserved.
         * </p>
         *
         * @param icmp  An {@link ICMP} instance.
         */
        private void fill(ICMP icmp) {
            if (type == VALUE_NONE) {
                setType(icmp.getType());
            }
            if (code == VALUE_NONE) {
                setCode(icmp.getCode());
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
     * @param icmp  An {@link ICMP} instance.
     */
    public IcmpPacket(ICMP icmp) {
        packet = icmp;
    }

    /**
     * Return a {@link Values} instance that keeps current values for
     * ICMP header fields.
     *
     * @return  A {@link Values} instance.
     */
    private Values getValues() {
        return (modifiedValues == null) ? values : modifiedValues;
    }

    /**
     * Return a {@link Values} instance that keeps ICMP header field values
     * to be set.
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
     * Return an {@link ICMP} instance to set modified values.
     *
     * @return  An {@link ICMP} instance.
     */
    private ICMP getPacketForWrite() {
        if (cloned) {
            // Create a copy of the original packet.
            packet = packet.clone();
            cloned = false;
        }

        return packet;
    }

    // CachedPacket

    /**
     * Return a {@link ICMP} instance configured in this instance.
     *
     * <p>
     *   Note that modification to the ICMP header is not applied to the
     *   returned until {@link #commit(PacketContext)} is called.
     * </p>
     *
     * @return  A {@link ICMP} instance.
     */
    @Override
    public ICMP getPacket() {
        return packet;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean commit(PacketContext pctx) throws VTNException {
        boolean mod = false;
        ICMP icmp = null;
        if (modifiedValues != null) {
            // At least one flow action that modifies ICMP header is
            // configured.
            pctx.addMatchField(FlowMatchType.DL_TYPE);
            pctx.addMatchField(FlowMatchType.IP_PROTO);

            short type = modifiedValues.getType();
            if (values.getType() != type) {
                // ICMP type was modified.
                icmp = getPacketForWrite();
                icmp.setType((byte)type);
                mod = true;
            } else if (pctx.hasMatchField(FlowMatchType.ICMP_TYPE)) {
                // ICMP type in the original packet is unchanged and it will be
                // specified in flow match. So we don't need to configure
                // SET_TP_SRC action.
                pctx.removeFilterAction(VTNSetIcmpTypeAction.class);
            }

            short code = modifiedValues.getCode();
            if (values.getCode() != code) {
                // ICMP code was modifled.
                if (icmp == null) {
                    icmp = getPacketForWrite();
                }
                icmp.setCode((byte)code);
                mod = true;
            } else if (pctx.hasMatchField(FlowMatchType.ICMP_CODE)) {
                // ICMP code in the original packet is unchanged and it will be
                // specified in flow match. So we don't need to configure
                // SET_TP_DST action.
                pctx.removeFilterAction(VTNSetIcmpCodeAction.class);
            }
        }

        return mod;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IcmpPacket clone() {
        try {
            IcmpPacket icmp = (IcmpPacket)super.clone();
            Values v = icmp.values;
            icmp.values = v.clone();

            v = icmp.modifiedValues;
            if (v != null) {
                icmp.modifiedValues = v.clone();
            }
            icmp.cloned = true;

            return icmp;
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
    public VTNIcmpMatch createMatch(Set<FlowMatchType> fields)
        throws RpcException {
        Values v = values;
        v.fill(packet);

        Short type = (fields.contains(FlowMatchType.ICMP_TYPE))
            ? Short.valueOf(v.getType())
            : null;
        Short code = (fields.contains(FlowMatchType.ICMP_CODE))
            ? Short.valueOf(v.getCode())
            : null;

        return new VTNIcmpMatch(type, code);
    }

    /**
     * Calculate the checksum of the packet.
     *
     * <p>
     *   This method does nothing because the ICMP checksum is computed by
     *   {@link ICMP} class.
     * </p>
     *
     * @param ipv4  Never used.
     * @return  {@code false}.
     */
    @Override
    public boolean updateChecksum(Inet4Packet ipv4) {
        return false;
    }

    // IcmpHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public short getIcmpType() {
        Values v = getValues();
        short type = v.getType();
        if (type == VALUE_NONE) {
            byte b = packet.getType();
            type = v.setType(b);
        }

        return type;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setIcmpType(short type) {
        Values v = getModifiedValues();
        v.setType(type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public short getIcmpCode() {
        Values v = getValues();
        short code = v.getCode();
        if (code == VALUE_NONE) {
            byte b = packet.getCode();
            code = v.setCode(b);
        }

        return code;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setIcmpCode(short code) {
        Values v = getModifiedValues();
        v.setCode(code);
    }


    // ProtocolHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDescription(StringBuilder builder) {
        int type = (int)getIcmpType();
        int code = (int)getIcmpCode();
        builder.append("ICMP[type=").append(type).
            append(",code=").append(code).append(']');
    }
}
