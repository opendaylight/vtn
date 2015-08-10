/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.unicast;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;
import org.opendaylight.vtn.manager.it.util.packet.TcpFactory;

import org.opendaylight.controller.sal.utils.IPProtocols;

/**
 * {@code Tcp4FlowFactory} describes a test environment for configuring an
 * TCP/IPv4 unicast flow.
 */
public final class Tcp4FlowFactory extends Inet4PortFlowFactory {
    /**
     * Raw payload for test packet.
     */
    private static final byte[]  PAYLOAD = {
        (byte)0x12, (byte)0x34, (byte)0x56, (byte)0x78,
    };

    /**
     * Construct a new instance.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param src     The source port.
     * @param dst     The destination port.
     */
    public Tcp4FlowFactory(OfMockService ofmock, short src, short dst) {
        super(ofmock, IPProtocols.TCP.byteValue(), src, dst);
    }

    // UnicastFlowFactory

    /**
     * {@inheritDoc}
     */
    @Override
    public EthernetFactory createPacketFactory(TestHost src, TestHost dst) {
        EthernetFactory efc = super.createPacketFactory(src, dst);
        Inet4Factory i4fc = efc.getNextFactory(Inet4Factory.class);
        TcpFactory ufc = TcpFactory.newInstance(i4fc).
            setSourcePort(getSourcePort()).
            setDestinationPort(getDestinationPort());
        ufc.setRawPayload(PAYLOAD);

        return efc;
    }
}
