/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.unicast;

import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;
import org.opendaylight.vtn.manager.it.util.packet.UdpFactory;

/**
 * {@code Udp4FlowFactory} describes a test environment for configuring an
 * Udp/IPv4 unicast flow.
 */
public final class Udp4FlowFactory extends Inet4PortFlowFactory {
    /**
     * Raw payload for test packet.
     */
    private static final byte[]  PAYLOAD = {
        (byte)0xab, (byte)0xcd, (byte)0xef, (byte)0x01,
        (byte)0x23, (byte)0x45,
    };

    /**
     * Construct a new instance.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param src     The source port.
     * @param dst     The destination port.
     */
    public Udp4FlowFactory(OfMockService ofmock, int src, int dst) {
        super(ofmock, InetProtocols.UDP.shortValue(), src, dst);
    }

    // UnicastFlowFactory

    /**
     * {@inheritDoc}
     */
    @Override
    public EthernetFactory createPacketFactory(TestHost src, TestHost dst) {
        EthernetFactory efc = super.createPacketFactory(src, dst);
        Inet4Factory i4fc = efc.getNextFactory(Inet4Factory.class);
        UdpFactory ufc = UdpFactory.newInstance(i4fc).
            setSourcePort(getSourcePort()).
            setDestinationPort(getDestinationPort());
        ufc.setRawPayload(PAYLOAD);

        return efc;
    }
}
