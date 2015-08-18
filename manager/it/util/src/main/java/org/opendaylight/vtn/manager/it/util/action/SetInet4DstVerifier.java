/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.action;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.ListIterator;

import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.dst.action._case.SetNwDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;

/**
 * {@code SetDscpVerifier} is a utility class used to verify a SET_NW_DST
 * action.
 */
public final class SetInet4DstVerifier extends ActionVerifier {
    /**
     * The destination IP address to be set.
     */
    private final InetAddress  address;

    /**
     * Ensure that the specified action is an expected SET_NW_DST action.
     *
     * @param it  Action list iterator.
     * @param ip  Expected destination IP address.
     */
    public static void verify(ListIterator<Action> it, InetAddress ip) {
        SetNwDstActionCase act = verify(it, SetNwDstActionCase.class);
        SetNwDstAction snda = act.getSetNwDstAction();
        Address addr = snda.getAddress();
        assertTrue(addr instanceof Ipv4);
        Ipv4 v4 = (Ipv4)addr;
        Ipv4Prefix v4p = v4.getIpv4Address();
        assertNotNull(v4p);
        assertEquals(ip.getHostAddress(), v4p.getValue());
    }

    /**
     * Construct a new instance.
     *
     * @param ip  The destination IP address to be set.
     */
    public SetInet4DstVerifier(InetAddress ip) {
        assertTrue(ip instanceof Inet4Address);
        address = ip;
    }

    /**
     * Return the destination IP address to be set.
     *
     * @return  The destination IP address to be set.
     */
    public InetAddress getAddress() {
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
