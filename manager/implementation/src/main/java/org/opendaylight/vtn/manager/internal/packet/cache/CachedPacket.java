/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.controller.sal.packet.Packet;

/**
 * {@code CachedPacket} defines interfaces that implements cache for a
 * {@link Packet} instance.
 */
public interface CachedPacket extends Cloneable {
    /**
     * Return a {@link Packet} instance.
     *
     * <p>
     *   Note that modification to this instance is not applied to the
     *   returned until {@link #commit(PacketContext)} is called.
     * </p>
     *
     * @return  A {@link Packet} instance.
     */
    Packet getPacket();

    /**
     * Finalize modification to the packet.
     *
     * @param pctx    The context of the received packet.
     * @return  {@code true} only if this packet is modified.
     * @throws VTNException
     *    Failed to copy the packet.
     */
    boolean commit(PacketContext pctx) throws VTNException;

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    CachedPacket clone();
}
