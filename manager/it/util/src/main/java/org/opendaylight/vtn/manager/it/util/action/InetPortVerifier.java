/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import org.opendaylight.vtn.manager.it.util.packet.PacketFactory;

/**
 * {@code InetProtoVerifier} is a utility class used to verify a SET_TP_SRC or
 * SET_TP_DST action.
 *
 * @param <T>  The type of packet factory corresponding to the target protocol.
 */
public abstract class InetPortVerifier<T extends PacketFactory>
    extends ActionVerifier {
    /**
     * Packet factory class corresponding to the target packet.
     */
    private final Class<T>  factoryType;

    /**
     * The value to be set.
     */
    private final int  value;

    /**
     * Construct a new instance.
     *
     * @param cls  Packet factory class corresponding to the target packet.
     * @param v    The value to be set.
     */
    InetPortVerifier(Class<T> cls, int v) {
        factoryType = cls;
        value = v;
    }

    /**
     * Return the packet factory class corresponding to the target packet.
     *
     * @return  The packet factory class.
     */
    public final Class<T> getFactoryType() {
        return factoryType;
    }

    /**
     * Return the short integer value to be set.
     *
     * @return  The short integer value to be set.
     */
    public final int getValue() {
        return value;
    }
}
