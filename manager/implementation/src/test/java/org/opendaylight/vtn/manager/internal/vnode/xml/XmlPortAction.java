/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.opendaylight.vtn.manager.internal.TestBase.createUnsignedShort;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code XmlPortAction} describes the configuration of flow action that sets
 * the port number for IP transport layer protocol into packet.
 *
 * @param <A>  The type of vtn-action.
 */
public abstract class XmlPortAction<A extends VtnAction>
    extends XmlFlowAction<A> {
    /**
     * The port number to set.
     */
    private Integer  port;

    /**
     * Construct an empty instance.
     */
    protected XmlPortAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param pnum  The port number to set.
     */
    protected XmlPortAction(Integer ord, Integer pnum) {
        super(ord);
        port = pnum;
    }

    /**
     * Return the port number to set.
     *
     * @return  The port number to set.
     */
    public final Integer getPort() {
        return port;
    }

    /**
     * Return a {@link PortNumber} instance that specifies the port number
     * to set.
     *
     * @return  A {@link PortNumber} instance if the port number is configured.
     *          {@code null} if not configured.
     */
    public final PortNumber getPortNumber() {
        return (port == null) ? null : new PortNumber(port);
    }

    // XmlFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        port = createUnsignedShort(rand);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setXml(XmlNode xnode) {
        if (port != null) {
            xnode.add(new XmlNode("port", port));
        }
    }
}
