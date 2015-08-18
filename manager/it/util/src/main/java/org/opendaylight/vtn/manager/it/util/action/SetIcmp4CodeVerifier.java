/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import org.opendaylight.vtn.manager.it.util.packet.Icmp4Factory;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;

/**
 * {@code SetTcpSrcVerifier} is a utility class used to verify a flow action
 * that modifies the ICMP code in a ICMP version 4 packet.
 */
public abstract class SetIcmp4CodeVerifier
    extends SetTpSrcVerifier<Icmp4Factory> {
    /**
     * Construct a new instance.
     *
     * @param code  The ICMP code to be set.
     */
    public SetIcmp4CodeVerifier(short code) {
        super(Icmp4Factory.class, code);
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public void apply(EthernetFactory fc) {
        Icmp4Factory ic4fc = getFactory(fc, Icmp4Factory.class);
        if (ic4fc != null) {
            ic4fc.setCode((byte)getValue());
        }
    }
}
