/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.TcpFactory;

/**
 * {@code SetTcpSrcVerifier} is a utility class used to verify a flow action
 * that modifies the TCP source port.
 */
public abstract class SetTcpSrcVerifier extends SetTpSrcVerifier<TcpFactory> {
    /**
     * Construct a new instance.
     *
     * @param port  The source port number to be set.
     */
    public SetTcpSrcVerifier(short port) {
        super(TcpFactory.class, port);
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public void apply(EthernetFactory fc) {
        TcpFactory tfc = getFactory(fc, TcpFactory.class);
        if (tfc != null) {
            tfc.setSourcePort(getValue());
        }
    }
}
