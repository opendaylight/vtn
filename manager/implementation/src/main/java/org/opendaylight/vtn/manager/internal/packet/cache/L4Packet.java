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

import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNLayer4Match;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * {@code L4Packet} defines interfaces that implements cache for layer 4
 * protocol header.
 */
public interface L4Packet extends CachedPacket, Layer4Header {
    /**
     * Construct match fields to test layer 4 protocol header in this packet.
     *
     * <p>
     *   Note that this method creates match fields that matches the original
     *   packet. Any modification to the packet is ignored.
     * </p>
     *
     * @param fields  A set of {@link FlowMatchType} instances corresponding to
     *                match fields to be tested.
     * @return  A {@link VTNLayer4Match} instance.
     * @throws RpcException  This packet is broken.
     */
    VTNLayer4Match createMatch(Set<FlowMatchType> fields) throws RpcException;

    /**
     * Calculate the checksum of the packet, and set the computed checksum
     * into a {@link org.opendaylight.vtn.manager.packet.Packet} instance
     * configured in this instance.
     *
     * @param ipv4  An {@link Inet4Packet} instance that contains this
     *              packet.
     * @return  {@code true} is returned if the checksum field was updated.
     *          {@code false} is returned if the packet was not modified.
     * @throws VTNException  An error occurred.
     */
    boolean updateChecksum(Inet4Packet ipv4) throws VTNException;

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    L4Packet clone();
}
