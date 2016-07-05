/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.util.NumberUtils.toBytes;

/**
 * Abstract class that describes protocol packet which identifies the service
 * using 16-bit port number.
 *
 * @param <T>  The actual type of this packet.
 * @since      Boron
 */
public abstract class PortPacket<T extends PortPacket> extends Packet {
    /**
     * The field name that indicates the source port number.
     */
    static final String  SRCPORT = "SourcePort";

    /**
     * The field name that indicates the destination port number.
     */
    static final String  DESTPORT = "DestinationPort";

    /**
     * Construct a new instance.
     */
    PortPacket() {
    }

    /**
     * Return the source port number configured in this instance.
     *
     * @return  The source port number.
     */
    public final short getSourcePort() {
        return getShort(SRCPORT);
    }

    /**
     * Return the destination port number configured in this instance.
     *
     * @return  The destination port number.
     */
    public final short getDestinationPort() {
        return getShort(DESTPORT);
    }

    /**
     * Set the source port number into this instance.
     *
     * @param port  The source port number.
     * @return  This instance.
     */
    public final T setSourcePort(short port) {
        return setPort(SRCPORT, port);
    }

    /**
     * Set the destination port number into this instance.
     *
     * @param port  The destination port number.
     * @return  This instance.
     */
    public final T setDestinationPort(short port) {
        return setPort(DESTPORT, port);
    }

    /**
     * Set the port number into this instance.
     *
     * @param field  The name of the field.
     * @param port   The port number to set.
     * @return  This instance.
     */
    private T setPort(String field, short port) {
        getHeaderFieldMap().put(field, toBytes(port));

        @SuppressWarnings("unchecked")
        T packet = (T)this;
        return packet;
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
