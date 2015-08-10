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
import org.opendaylight.vtn.manager.it.util.packet.ArpFactory;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;

/**
 * {@code ArpFlowFactory} describes a test environment for configuring an
 * ARP unicast flow.
 */
public final class ArpFlowFactory extends UnicastFlowFactory {
    /**
     * ARP operation code.
     */
    private final Short  operation;

    /**
     * Construct a new instance.
     *
     * @param ofmock  openflowplugin mock-up service.
     */
    public ArpFlowFactory(OfMockService ofmock) {
        super(ofmock);
        operation = null;
    }

    /**
     * Construct a new instance.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param op      ARP operation code to be used for test.
     */
    public ArpFlowFactory(OfMockService ofmock, short op) {
        super(ofmock);
        operation = Short.valueOf(op);
    }

    // UnicastFlowFactory

    /**
     * {@inheritDoc}
     */
    @Override
    public EthernetFactory createPacketFactory(TestHost src, TestHost dst) {
        EthernetFactory efc = createEthernetFactory(src, dst);

        byte[] sha = src.getMacAddress();
        byte[] tha = dst.getMacAddress();
        byte[] spa = src.getRawInetAddress();
        byte[] tpa = src.getRawInetAddress();
        ArpFactory afc = ArpFactory.newInstance(efc).
            setSenderHardwareAddress(sha).
            setTargetHardwareAddress(tha).
            setSenderProtocolAddress(spa).
            setTargetProtocolAddress(tpa);
        if (operation != null) {
            afc.setOperation(operation.shortValue());
        }

        return efc;
    }
}
