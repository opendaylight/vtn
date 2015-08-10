/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.UdpFactory;

/**
 * {@code SetUdpDstVerifier} is a utility class used to verify a flow action
 * that modifies the UDP destination port.
 */
public abstract class SetUdpDstVerifier extends SetTpDstVerifier<UdpFactory> {
    /**
     * Construct a new instance.
     *
     * @param port  The destination port number to be set.
     */
    public SetUdpDstVerifier(short port) {
        super(UdpFactory.class, port);
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public void apply(EthernetFactory fc) {
        UdpFactory tfc = getFactory(fc, UdpFactory.class);
        if (tfc != null) {
            tfc.setDestinationPort(getValue());
        }
    }
}
