/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import org.opendaylight.controller.sal.packet.ICMP;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code IcmpPacket} class implements a cache for an {@link ICMP} instance.
 */
public final class IcmpPacket implements CachedPacket {
    /**
     * A pseudo short value which indicates the byte value is not specified.
     */
    private static final short  VALUE_NONE = -1;

    /**
     * The ICMP type.
     */
    private short  type = VALUE_NONE;

    /**
     * The ICMP code.
     */
    private short  code = VALUE_NONE;

    /**
     * An {@link ICMP} packet.
     */
    private final ICMP  packet;

    /**
     * Construct a new instance.
     *
     * @param icmp  An {@link ICMP} instance.
     */
    public IcmpPacket(ICMP icmp) {
        packet = icmp;
    }

    /**
     * Return the ICMP type.
     *
     * @return  A short integer value which indicates the ICMP type.
     */
    public short getType() {
        if (type == VALUE_NONE) {
            byte b = packet.getType();
            type = (short)NetUtils.getUnsignedByte(b);
        }

        return type;
    }

    /**
     * Return the ICMP code.
     *
     * @return  A short integer value which indicates the ICMP code.
     */
    public short getCode() {
        if (code == VALUE_NONE) {
            byte b = packet.getCode();
            code = (short)NetUtils.getUnsignedByte(b);
        }

        return code;
    }

    // CachedPacket

    /**
     * Return a {@link ICMP} instance configured in this instance.
     *
     * @return  A {@link ICMP} instance.
     */
    @Override
    public ICMP getPacket() {
        return packet;
    }
}
