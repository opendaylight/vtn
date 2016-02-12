/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.opendaylight.vtn.manager.internal.TestBase.createEtherAddress;

import java.util.Random;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code XmlDlAddrAction} describes the configuration of flow action that
 * sets the MAC address into Ethernet header.
 *
 * @param <A>  The type of vtn-action.
 */
public abstract class XmlDlAddrAction<A extends VtnAction>
    extends XmlFlowAction<A> {
    /**
     * The MAC address to set.
     */
    private EtherAddress  address;

    /**
     * Construct an empty instance.
     */
    protected XmlDlAddrAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param addr  The MAC address to set.
     */
    protected XmlDlAddrAction(Integer ord, EtherAddress addr) {
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

    /**
     * Return the MD-SAL MAC address that specifies the MAC address to set.
     *
     * @return  A {@link MacAddress} instance or {@code null}.
     */
    public final MacAddress getMacAddress() {
        return (address == null) ? null : address.getMacAddress();
    }

    // XmlFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        address = createEtherAddress(rand);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setXml(XmlNode xnode) {
        if (address != null) {
            xnode.add(new XmlNode("address", address.getText()));
        }
    }
}
