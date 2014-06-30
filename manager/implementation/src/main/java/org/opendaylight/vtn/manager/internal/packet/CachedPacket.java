/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.util.Set;

import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
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

    /**
     * Configure match fields to test protocol header in this packet.
     *
     * @param match   A {@link Match} instance.
     * @param fields  A set of {@link MatchType} instances corresponding to
     *                match fields to be tested.
     */
    void setMatch(Match match, Set<MatchType> fields);
}
