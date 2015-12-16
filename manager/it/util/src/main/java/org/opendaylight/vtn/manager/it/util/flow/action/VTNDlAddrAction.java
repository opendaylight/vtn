/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.opendaylight.vtn.manager.it.util.TestBase.createEtherAddress;

import java.util.Random;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;

/**
 * {@code VTNDlAddrAction} describes the configuration of flow action that
 * sets the MAC address into Ethernet header.
 *
 * @param <A>  The type of vtn-action.
 */
public abstract class VTNDlAddrAction<A extends VtnAction>
    extends FlowAction<A> {
    /**
     * The MAC address to set.
     */
    private EtherAddress  address;

    /**
     * Construct an empty instance.
     */
    protected VTNDlAddrAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The MAC address to set.
     */
    protected VTNDlAddrAction(Integer ord, EtherAddress addr) {
        super(ord);
        address = addr;
    }

    /**
     * Return the MAC address to set.
     *
     * @return  An {@link EtherAddress} instance.
     */
    public final EtherAddress getAddress() {
        return address;
    }

    // FlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        address = createEtherAddress(rand);
    }
}
