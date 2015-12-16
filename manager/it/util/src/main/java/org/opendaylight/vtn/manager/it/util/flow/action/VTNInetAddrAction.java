/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import static org.opendaylight.vtn.manager.it.util.TestBase.createIp4Network;

import java.util.Random;

import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4;

/**
 * {@code VTNInetAddrAction} describes the configuration of flow action that
 * sets the IP address into IP header.
 *
 * @param <A>  The type of vtn-action.
 */
public abstract class VTNInetAddrAction<A extends VtnAction>
    extends FlowAction<A> {
    /**
     * The IP address to set.
     */
    private IpNetwork  address;

    /**
     * Construct an empty instance.
     */
    protected VTNInetAddrAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The IP address to set.
     */
    protected VTNInetAddrAction(Integer ord, IpNetwork addr) {
        super(ord);
        address = addr;
    }

    /**
     * Return the IP address to set.
     *
     * @return  An {@link IpNetwork} instance.
     */
    public final IpNetwork getAddress() {
        return address;
    }

    /**
     * Return the MD-SAL IP address that specifies the IP address to set.
     *
     * @return  An {@link Address} instance.
     */
    public final Address getMdAddress() {
        return (address == null) ? null : address.getMdAddress();
    }

    /**
     * Verify the IP address to set.
     *
     * @param maddr  An {@link Address} instance to be verified.
     */
    protected void verifyAddress(Address maddr) {
        // Only IPv4 address is supported.
        assertTrue(address instanceof Ip4Network);
        Ip4Network ip4 = (Ip4Network)address;

        assertTrue(maddr instanceof Ipv4);
        Ipv4 ipv4 = (Ipv4)maddr;
        assertEquals(ip4.getIpPrefix().getIpv4Prefix(), ipv4.getIpv4Address());
    }

    // FlowAction

    /**
     * {@inheritDoc}
     */
    protected void setImpl(Random rand) {
        address = createIp4Network(rand);
    }
}
