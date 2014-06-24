/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import org.opendaylight.controller.sal.packet.Packet;

/**
 * {@code CachedPacket} defines interfaces that implements cache for a
 * {@link Packet} instance.
 */
public interface CachedPacket {
    /**
     * Return a {@link Packet} instance.
     *
     * @return  A {@link Packet} instance.
     */
    Packet getPacket();
}
