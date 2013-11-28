/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import org.junit.Test;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.vtn.manager.internal.VNodeEventTestBase;

/**
 * JUnit Test for {@link RawPacketEvent}.
 */
public class RawPacketEventTest extends VNodeEventTestBase {

    /**
     * Test method for
     * {@link ClusterEvent#received(VTNManagerImpl, boolean)},
     * {@link RawPacketEvent#isSingleThreaded(boolean)}.
     */
    @Test
    public void testReceived() {
        byte[] src = new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        byte[] dst = new byte[] {(byte) 0xff, (byte) 0xff, (byte) 0xff,
                                 (byte) 0xff, (byte) 0xff, (byte) 0xff};
        byte[] sender = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 1};
        byte[] target = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 10};
        Node node = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 10), node);
        NodeConnector outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 11), node);
        RawPacket pkt = createARPRawPacket(src, dst, sender, target, (short) 0, innc, ARP.REQUEST);

        RawPacketEvent ev = new RawPacketEvent(pkt, outnc);

        for (Boolean local : createBooleans(false)) {
            ev.received(vtnMgr, local.booleanValue());
            flushTasks();
            if (local == Boolean.FALSE) {
                assertEquals(1, stubObj.getTransmittedDataPacket().size());
            } else {
                assertEquals(0, stubObj.getTransmittedDataPacket().size());
            }
            assertFalse(local.toString(), ev.isSingleThreaded(local.booleanValue()));
        }
    }

    /**
     * Ensure that {@link RawPacketEvent} is serializable.
     */
    @Test
    public void testSerialize() {
        byte[] src = new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        byte[] dst = new byte[] {(byte) 0xff, (byte) 0xff, (byte) 0xff,
                                 (byte) 0xff, (byte) 0xff, (byte) 0xff};
        byte[] sender = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 1};
        byte[] target = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 10};
        Node node = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector innc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 10), node);
        NodeConnector outnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 11), node);
        RawPacket pkt = createARPRawPacket(src, dst, sender, target, (short) 0, innc, ARP.REQUEST);

        RawPacketEvent ev = new RawPacketEvent(pkt, outnc);
        eventSerializeTest(ev);

        // Because RawPacketEvent has no getter method,
        // equality of deserialized object isn't checked.
    }
}