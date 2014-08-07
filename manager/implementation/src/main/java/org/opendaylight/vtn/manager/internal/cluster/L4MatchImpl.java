/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;
import org.opendaylight.vtn.manager.flow.cond.UdpMatch;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code L4MatchImpl} describes the condition to match layer 4 protocol
 * header fields in packet.
 *
 * <ul>
 *   <li>
 *     This class assumes that all supported layer 4 protocols are implemented
 *     on Internet Protocol.
 *   </li>
 *   <li>
 *     Although this class is public to other packages, this class does not
 *     provide any API. Applications other than VTN Manager must not use this
 *     class.
 *   </li>
 * </ul>
 */
public abstract class L4MatchImpl implements PacketMatch {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -8133751263574088210L;

    /**
     * Create a new {@code L4MatchImpl} instance from the given
     * {@link L4Match} instance.
     *
     * @param match  A {@link L4Match} instance.
     * @return  A {@link L4MatchImpl} instance constructed from {@code match}.
     * @throws VTNException
     *   An invalid instance is specified to {@code match}.
     */
    public static final L4MatchImpl create(L4Match match)
        throws VTNException {
        if (match instanceof TcpMatch) {
            return new TcpMatchImpl((TcpMatch)match);
        }
        if (match instanceof UdpMatch) {
            return new UdpMatchImpl((UdpMatch)match);
        }
        if (match instanceof IcmpMatch) {
            return new IcmpMatchImpl((IcmpMatch)match);
        }

        // This should never happen.
        throw new VTNException(StatusCode.BADREQUEST,
                               "Unexpected L4 match instance: " + match);
    }

    /**
     * Construct a new instance.
     */
    protected L4MatchImpl() {
    }

    /**
     * Return an IP protocol number assigned to this protocol.
     *
     * @return  An IP protocol number.
     */
    public abstract short getInetProtocol();

    /**
     * Return a {@link L4Match} instance which represents this condition.
     *
     * @return  A {@link L4Match} instance.
     */
    public abstract L4Match getMatch();
}
