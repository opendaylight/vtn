/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;
import static org.opendaylight.vtn.manager.util.NumberUtils.toShort;

import java.util.EnumMap;
import java.util.Map;

import com.google.common.collect.ImmutableMap;

import org.opendaylight.vtn.manager.util.EtherTypes;

/**
 * {@code EtherTypePacket} describes an layer 2 packet that contains Ethernet
 * type and layer 3 packet.
 *
 * @param <T>  The actual type of class that extends this class.
 * @since      Boron
 */
public abstract class EtherTypePacket<T extends EtherTypePacket>
    extends Packet {
    /**
     * A set of supported payload types.
     */
    private static final Map<EtherTypes, Class<? extends Packet>> PAYLOAD_TYPES;

    /**
     * The field name that indicates the Ethernet type.
     */
    static final String  ETHT = "EtherType";

    /**
     * Initialize static fields.
     */
    static {
        // Initialize the payload types.
        Map<EtherTypes, Class<? extends Packet>> typeMap =
            new EnumMap<>(EtherTypes.class);
        typeMap.put(EtherTypes.IPV4, IPv4.class);
        typeMap.put(EtherTypes.ARP, ARP.class);
        typeMap.put(EtherTypes.VLAN, IEEE8021Q.class);
        PAYLOAD_TYPES = ImmutableMap.copyOf(typeMap);
    }

    /**
     * Determine the payload type for the given Ethernet type.
     *
     * @param type  The Ethernet type value.
     * @return  A class for the payload type.
     *          {@code null} if no payload type is defined for the given
     *          Ethernet type.
     */
    static final Class<? extends Packet> getPayloadClass(short type) {
        EtherTypes etype = EtherTypes.forValue(type);
        return (etype == null) ? null : PAYLOAD_TYPES.get(etype);
    }

    /**
     * Return the Ethernet type configured in this instance.
     *
     * @return  The Ethernet type.
     */
    public final short getEtherType() {
        return getShort(ETHT);
    }

    /**
     * Set the Ethernet type that determines the type of the payload for
     * this instance.
     *
     * @param type  The Ethernet type.
     * @return  This instance.
     */
    public final T setEtherType(short type) {
        getHeaderFieldMap().put(ETHT, toBytes(type));

        @SuppressWarnings("unchecked")
        T packet = (T)this;
        return packet;
    }

    /**
     * Store the value of fields read from data stream.
     *
     * @param name   The name of the header field.
     * @param value  The value to be associated with the specified header
     *               field. {@code null} cannot be specified.
     */
    @Override
    protected final void setHeaderField(String name, byte[] value) {
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
    public final T clone() {
        @SuppressWarnings("unchecked")
        T packet = (T)super.clone();
        return packet;
    }
}
