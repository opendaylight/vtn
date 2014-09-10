/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import org.opendaylight.vtn.manager.VTNException;

/**
 * {@code L4Packet} defines interfaces that implements cache for layer 4
 * protocol header.
 */
public interface L4Packet extends CachedPacket {
    /**
     * Calculate the checksum of the packet, and set the computed checksum
     * into a {@link org.opendaylight.controller.sal.packet.Packet} instance
     * configured in this instance.
     *
     * @param ipv4  An {@link Inet4Packet} instance that contains this
     *              packet.
     * @return  {@code true} is returned if the checksum field was updated.
     *          {@code false} is returned if the packet was not modified.
     * @throws VTNException
     *    An error occurred.
     */
    boolean updateChecksum(Inet4Packet ipv4) throws VTNException;

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    L4Packet clone();
}
