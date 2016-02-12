/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertTrue;

import static org.opendaylight.vtn.manager.internal.TestBase.createIp4Network;

import java.util.Random;

import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;

/**
 * {@code VTNInetAddrAction} describes the configuration of flow action that
 * sets the IP address into IP header.
 *
 * @param <A>  The type of vtn-action.
 */
public abstract class XmlInetAddrAction<A extends VtnAction>
    extends XmlFlowAction<A> {
    /**
     * The IP address to set.
     */
    private IpNetwork  address;

    /**
     * Construct an empty instance.
     */
    protected XmlInetAddrAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The IP address to set.
     */
    protected XmlInetAddrAction(Integer ord, IpNetwork addr) {
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

    // XmlFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        address = createIp4Network(rand);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setXml(XmlNode xnode) {
        if (address != null) {
            // Only IPv4 address is supported.
            assertTrue(address instanceof Ip4Network);
            xnode.add(new XmlNode("ipv4-address", address.getText()));
        }
    }
}
