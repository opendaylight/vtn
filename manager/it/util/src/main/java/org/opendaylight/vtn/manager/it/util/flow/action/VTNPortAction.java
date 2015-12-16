/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.action;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.it.util.TestBase.createUnsignedShort;

import java.util.Random;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code VTNPortAction} describes the configuration of flow action that sets
 * the port number for IP transport layer protocol into packet.
 *
 * @param <A>  The type of vtn-action.
 */
public abstract class VTNPortAction<A extends VtnAction>
    extends FlowAction<A> {
    /**
     * The default value of the port number.
     */
    public static final Integer  DEFAULT_PORT = 0;

    /**
     * The port number to set.
     */
    private Integer  port;

    /**
     * Construct an empty instance.
     */
    protected VTNPortAction() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ord   The order of the flow action.
     * @param pnum  The port number to set.
     */
    protected VTNPortAction(Integer ord, Integer pnum) {
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

    /**
     * Verify the target port number.
     *
     * @param pnum  The port number to be verified.
     */
    protected void verifyPort(PortNumber pnum) {
        Integer expected = (port == null) ? DEFAULT_PORT : port;
        assertEquals(expected, pnum.getValue());
    }

    // FlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected void setImpl(Random rand) {
        port = createUnsignedShort(rand);
    }
}
