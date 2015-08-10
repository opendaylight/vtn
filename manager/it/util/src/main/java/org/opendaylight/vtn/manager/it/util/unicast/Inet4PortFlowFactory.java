/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.unicast;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;

/**
 * {@code Inet4PortFlowFactory} describes a test environment for configuring
 * unicast flow using IPv4 transport layer protocol.
 */
public abstract class Inet4PortFlowFactory extends Inet4FlowFactory {
    /**
     * The source port.
     */
    private short  sourcePort;

    /**
     * The destination port.
     */
    private short  destinationPort;

    /**
     * Construct a new instance.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param proto   IP protocol number.
     * @param src     The source port.
     * @param dst     The destination port.
     */
    public Inet4PortFlowFactory(OfMockService ofmock, byte proto, short src,
                                short dst) {
        super(ofmock);
        setProtocol(proto);
        sourcePort = src;
        destinationPort = dst;
    }

    /**
     * Return the source port to be used for test.
     *
     * @return  The source port number.
     */
    public final short getSourcePort() {
        return sourcePort;
    }

    /**
     * Return the destination port to be used for test.
     *
     * @return  The destination port number.
     */
    public final short getDestinationPort() {
        return destinationPort;
    }

    /**
     * Set the source port to be used for test.
     *
     * @param port  The source port number to be used for test.
     * @return  This instance.
     */
    public final Inet4PortFlowFactory setSourcePort(short port) {
        sourcePort = port;
        return this;
    }

    /**
     * Set the destination port to be used for test.
     *
     * @param port  The destination port number to be used for test.
     * @return  This instance.
     */
    public final Inet4PortFlowFactory setDestinationPort(short port) {
        destinationPort = port;
        return this;
    }

    // Inet4FlowFactory

    /**
     * Override this method to prevent IPv4 packet from setting raw payload.
     *
     * @param i4fc  Unused.
     */
    @Override
    protected final void setRawPayload(Inet4Factory i4fc) {
    }
}
