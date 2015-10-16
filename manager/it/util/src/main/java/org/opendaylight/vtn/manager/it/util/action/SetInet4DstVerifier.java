/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.ListIterator;

import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.dst.action._case.SetNwDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;

/**
 * {@code SetDscpVerifier} is a utility class used to verify a SET_NW_DST
 * action.
 */
public final class SetInet4DstVerifier extends ActionVerifier {
    /**
     * The destination IP address to be set.
     */
    private final IpNetwork  address;

    /**
     * Ensure that the specified action is an expected SET_NW_DST action.
     *
     * @param it  Action list iterator.
     * @param ip  Expected destination IP address.
     */
    public static void verify(ListIterator<Action> it, IpNetwork ip) {
        SetNwDstActionCase act = verify(it, SetNwDstActionCase.class);
        SetNwDstAction snda = act.getSetNwDstAction();
        assertEquals(ip.getMdAddress(), snda.getAddress());
    }

    /**
     * Construct a new instance.
     *
     * @param ip  The destination IP address to be set.
     */
    public SetInet4DstVerifier(IpNetwork ip) {
        assertTrue(ip instanceof Ip4Network);
        address = ip;
    }

    /**
     * Return the destination IP address to be set.
     *
     * @return  The destination IP address to be set.
     */
    public IpNetwork getAddress() {
        return address;
    }

    // ActionVerifier

    /**
     * {@inheritDoc}
     */
    @Override
    public void verify(EthernetFactory efc, ListIterator<Action> it) {
        Inet4Factory i4fc = getFactory(efc, Inet4Factory.class);
        if (i4fc != null) {
            verify(it, address);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void apply(EthernetFactory efc) {
        Inet4Factory i4fc = getFactory(efc, Inet4Factory.class);
        if (i4fc != null) {
            i4fc.setDestinationAddress(address);
        }
    }
}
