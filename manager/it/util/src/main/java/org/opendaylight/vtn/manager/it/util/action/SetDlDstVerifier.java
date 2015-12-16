/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import static org.junit.Assert.assertEquals;

import java.util.ListIterator;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;

/**
 * {@code SetDlDstVerifier} is a utility class used to verify a SET_DL_DST
 * action.
 */
public final class SetDlDstVerifier extends ActionVerifier {
    /**
     * The destination MAC address to be set.
     */
    private final EtherAddress  macAddress;

    /**
     * Ensure that the specified action is an expected SET_DL_DST action.
     *
     * @param it   Action list iterator.
     * @param mac  Expected MAC address.
     */
    public static void verify(ListIterator<Action> it, EtherAddress mac) {
        SetDlDstActionCase act = verify(it, SetDlDstActionCase.class);
        SetDlDstAction sdda = act.getSetDlDstAction();
        assertEquals(mac.getMacAddress(), sdda.getAddress());
    }

    /**
     * Construct a new instance.
     *
     * @param mac  The MAC address to be set as the destination MAC address.
     */
    public SetDlDstVerifier(EtherAddress mac) {
        macAddress = mac;
    }

    /**
     * Return the destination MAC address.
     *
     * @return  The destination MAC address.
     */
    public EtherAddress getAddress() {
        return macAddress;
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(EthernetFactory efc, ListIterator<Action> it) {
        verify(it, macAddress);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void apply(EthernetFactory efc) {
        efc.setDestinationAddress(macAddress);
    }
}
