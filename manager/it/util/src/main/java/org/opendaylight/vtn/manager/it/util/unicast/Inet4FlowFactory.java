/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.unicast;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;

/**
 * {@code Inet4FlowFactory} describes a test environment for configuring an
 * IPv4 unicast flow.
 */
public class Inet4FlowFactory extends UnicastFlowFactory {
    /**
     * Default IP protocl number.
     */
    private static final short  DEFAULT_PROTOCOL = (short)200;

    /**
     * Raw payload for test packet.
     */
    private static final byte[]  PAYLOAD = {
        (byte)0xab, (byte)0xcd, (byte)0xef, (byte)0x11,
        (byte)0x22, (byte)0x33,
    };

    /**
     * Protocol number.
     */
    private short  protocol = DEFAULT_PROTOCOL;

    /**
     * IPv4 DSCP value.
     */
    private Short  inet4Dscp;

    /**
     * Construct a new instance.
     *
     * @param ofmock   openflowplugin mock-up service.
     * @param service  A {@link VTNServices} instance.
     */
    public Inet4FlowFactory(OfMockService ofmock, VTNServices service) {
        super(ofmock, service);
    }

    /**
     * Return the IP protocol number.
     *
     * @return  The IP protocol number to be used for test.
     */
    public short getProtocol() {
        return protocol;
    }

    /**
     * Return the IPv4 DSCP value to be used for test.
     *
     * @return  A {@link Short} instance or {@code null}.
     */
    public Short getDscp() {
        return inet4Dscp;
    }

    /**
     * Set the IP protocol number to be used for test.
     *
     * @param proto  IP protocol number.
     * @return  This instance.
     */
    public Inet4FlowFactory setProtocol(short proto) {
        protocol = proto;
        return this;
    }

    /**
     * Set the IPv4 DSCP value to be used for test.
     *
     * @param dscp  DSCP value.
     * @return  This instance.
     */
    public Inet4FlowFactory setDscp(short dscp) {
        inet4Dscp = Short.valueOf(dscp);
        return this;
    }

    /**
     * Set raw payload for IPv4 packet.
     *
     * @param i4fc  An {@link Inet4Factory} instance.
     */
    protected void setRawPayload(Inet4Factory i4fc) {
        i4fc.setRawPayload(PAYLOAD);
    }

    // UnicastFlowFactory

    /**
     * {@inheritDoc}
     */
    @Override
    public EthernetFactory createPacketFactory(TestHost src, TestHost dst) {
        EthernetFactory efc = createEthernetFactory(src, dst);
        Inet4Factory i4fc = Inet4Factory.newInstance(efc).
            setProtocol(protocol).
            setSourceAddress(src.getInetAddress()).
            setDestinationAddress(dst.getInetAddress());
        setRawPayload(i4fc);

        if (inet4Dscp != null) {
            i4fc.setDscp(inet4Dscp.shortValue());
        }

        return efc;
    }
}
