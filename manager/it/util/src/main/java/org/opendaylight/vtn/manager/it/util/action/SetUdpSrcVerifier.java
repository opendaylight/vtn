/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.UdpFactory;

/**
 * {@code SetUdpSrcVerifier} is a utility class used to verify a flow action
 * that modifies the UDP source port.
 */
public abstract class SetUdpSrcVerifier extends SetTpSrcVerifier<UdpFactory> {
    /**
     * Construct a new instance.
     *
     * @param port  The source port number to be set.
     */
    public SetUdpSrcVerifier(int port) {
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
            tfc.setSourcePort(getValue());
        }
    }
}
