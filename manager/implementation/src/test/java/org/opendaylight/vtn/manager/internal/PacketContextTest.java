/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.TreeMap;

import org.junit.Test;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

public class PacketContextTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short vlans[] = { -10, 0, 1, 100, 4095 };
        Ethernet ether;
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            byte iphost = 1;
            for (short vlan : vlans) {
                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    byte[] bytes = ea.getValue();
                    byte[] src = new byte[] { bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5] };
                    byte[] dst = new byte[] { (byte) 0xff, (byte) 0xff, (byte) 0xff, (byte) 0xff, (byte) 0xff,
                            (byte) 0xff };
                    byte[] sender = new byte[] { (byte) 192, (byte) 168, (byte) 0, iphost };
                    byte[] sender2 = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) (iphost + 100) };
                    byte[] target = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 250 };

                    testPacketContext(src, dst, sender, target, vlan, nc);

                    ether = createARPPacket(src, dst, sender2, target, vlan, ARP.REQUEST);
                    testPacketContext(ether, src, dst, sender2, vlan, nc);
                }
            }
        }
    }

    /**
     * test PacketContext(RawPacket raw, Ethernet ether)
     *
     * @param src
     * @param dst
     * @param sender
     * @param target
     * @param vlan
     * @param nc
     */
    private void testPacketContext(byte[] src, byte[] dst, byte[] sender, byte[] target, short vlan, NodeConnector nc) {
        InetAddress ipaddr = null;
        MacTableEntry me;
        PortVlan pv;
        String desc;

        PacketContext pc = createARPPacketContext(src, dst, sender, target, vlan, nc, ARP.REQUEST);

        if (vlan < 0) {
            assertEquals(0, pc.getVlan());
            pv = new PortVlan(nc, (short) 0);
            me = new MacTableEntry(nc, (short) 0, ipaddr);
        } else {
            assertEquals(vlan, pc.getVlan());
            pv = new PortVlan(nc, vlan);
            me = new MacTableEntry(nc, vlan, ipaddr);
        }

        if (pc.getRawPacket() == null) {
            assertNull(pc.getIncomingNetwork());
            try {
                assertNull(pc.getIncomingNodeConnector());
            } catch (NullPointerException expected) {
                assertEquals(expected.getMessage(), null);
            }
        } else {
            assertNotNull(pc.getIncomingNodeConnector());
            assertEquals(pv, pc.getIncomingNetwork());
        }

        assertNull(pc.getOutgoingNodeConnector());
        assertEquals(src, pc.getSourceAddress());
        assertEquals(dst, pc.getDestinationAddress());

        byte[] sip = pc.getSourceIpAddress();
        assertEquals(sender, sip);

        try {
            ipaddr = InetAddress.getByAddress(sip);
        } catch (UnknownHostException e) {
            // This should never happen.
            fail(e.getMessage());
        }

        Long key = NetUtils.byteArray6ToLong(src);
        pc.addObsoleteEntry(key, me);

        TreeMap<Long, MacTableEntry> map = new TreeMap<Long, MacTableEntry>();
        map.put(key, me);

        assertEquals(map, pc.getObsoleteEntries());

        Ethernet ether = pc.getFrame();

        if (vlan < 0) {
            desc = convertForDescription(ether, nc, (short) 0);
            ether = pc.createFrame((short) 0);
        } else {
            desc = convertForDescription(ether, nc, vlan);
            ether = pc.createFrame(vlan);
        }

        assertEquals(desc, pc.getDescription(nc));
        assertNotNull(ether);
    }

    /**
     * test PacketContext(RawPacket raw, Ethernet ether)
     *
     * @param ether
     * @param src
     * @param dst
     * @param sender
     * @param vlan
     * @param nc
     */
    private void testPacketContext(Ethernet ether, byte[] src, byte[] dst, byte[] sender, short vlan, NodeConnector nc) {
        InetAddress ipaddr = null;
        MacTableEntry me;
        String desc;

        PacketContext pctx = new PacketContext(ether, nc);
        assertEquals(ether, pctx.getFrame());

        if (pctx.getRawPacket() == null) {
            assertNull(pctx.getIncomingNetwork());
            try {
                assertNull(pctx.getIncomingNodeConnector());
            } catch (NullPointerException expected) {
                assertEquals(expected.getMessage(), null);
            }
        } else {
            assertNotNull(pctx.getIncomingNodeConnector());
            assertNotNull(pctx.getIncomingNetwork());
        }

        assertEquals(nc, pctx.getOutgoingNodeConnector());

        assertNotNull(pctx.getPayload());
        assertEquals(src, pctx.getSourceAddress());
        assertEquals(dst, pctx.getDestinationAddress());

        assertEquals(sender, pctx.getSourceIpAddress());
        byte[] sip = pctx.getSourceIpAddress();

        try {
            ipaddr = InetAddress.getByAddress(sip);
        } catch (UnknownHostException e) {
            // This should never happen.
            fail(e.getMessage());
        }

        if (vlan < 0) {
            assertEquals((short) 0, pctx.getVlan());
            me = new MacTableEntry(nc, (short) 0, ipaddr);
        } else {
            assertEquals(vlan, pctx.getVlan());
            me = new MacTableEntry(nc, vlan, ipaddr);
        }

        Long key = NetUtils.byteArray6ToLong(src);
        pctx.addObsoleteEntry(key, me);

        TreeMap<Long, MacTableEntry> map = new TreeMap<Long, MacTableEntry>();
        map.put(key, me);

        assertEquals(map, pctx.getObsoleteEntries());

        if (vlan < 0) {
            desc = convertForDescription(ether, nc, (short) 0);
            ether = pctx.createFrame((short) 0);
        } else {
            desc = convertForDescription(ether, nc, vlan);
            ether = pctx.createFrame(vlan);
        }

        assertEquals(desc, pctx.getDescription(nc));
        assertNotNull(ether);
    }

    /**
     * Create a string to compare description.
     *
     * @param ether
     * @param port
     * @param vlan
     * @return A brief description of the specified ethernet frame.
     */
    private String convertForDescription(Ethernet ether, NodeConnector port, short vlan) {
        String srcmac = HexEncode.bytesToHexStringFormat(ether.getSourceMACAddress());
        String dstmac = HexEncode.bytesToHexStringFormat(ether.getDestinationMACAddress());
        int type = ether.getEtherType() & 0xffff;

        StringBuilder builder = new StringBuilder("src=");
        builder.append(srcmac).append(", dst=").append(dstmac).append(", port=").append(port).append(", type=0x")
                .append(Integer.toHexString(type)).append(", vlan=").append(vlan);

        return builder.toString();
    }
}
