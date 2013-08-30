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
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.LLDP;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketException;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
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
                    byte[] dst = new byte[] { (byte) 0xff, (byte) 0xff, (byte) 0xff,
                                              (byte) 0xff, (byte) 0xff, (byte) 0xff };
                    byte[] sender = new byte[] { (byte) 192, (byte) 168, (byte) 0, iphost };
                    byte[] sender2 = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) (iphost + 100) };
                    byte[] target = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 250 };

                    testPacketContext(src, dst, sender, target, vlan, nc);

                    ether = createARPPacket(src, dst, sender2, target, vlan, ARP.REQUEST);
                    testPacketContext(ether, src, dst, sender2, target, vlan, nc);
                }
            }
        }
    }

    /**
     * Test case for PacketContext(RawPacket raw, Ethernet ether)
     *
     * @param src   src MAC address.
     * @param dst   dst MAC address.
     * @param sender    sender IP address.
     * @param target    target IP address.
     * @param vlan  VLAN ID
     * @param nc    Node Connector.
     */
    private void testPacketContext(byte[] src, byte[] dst, byte[] sender, byte[] target, short vlan, NodeConnector nc) {
        InetAddress ipaddr = null;
        MacTableEntry me;
        PortVlan pv;
        String desc;
        String msg = "(src)" + src + ",(dst)" + dst +
                     ",(sender)" + sender + ",(target)" + target;

        // createARPPacketContext create PacketContext
        // by using {@link PacketContext(RawPacket, Ethernet)}.
        PacketContext pc = createARPPacketContext(src, dst, sender, target, vlan, nc, ARP.REQUEST);

        if (vlan < 0) {
            assertEquals(msg, 0, pc.getVlan());
            pv = new PortVlan(nc, (short) 0);
            me = new MacTableEntry(nc, (short) 0, ipaddr);
        } else {
            assertEquals(msg, vlan, pc.getVlan());
            pv = new PortVlan(nc, vlan);
            me = new MacTableEntry(nc, vlan, ipaddr);
        }

        assertNotNull(msg, pc.getRawPacket());
        assertNotNull(msg, pc.getIncomingNodeConnector());
        assertEquals(msg, pv, pc.getIncomingNetwork());

        assertNull(msg, pc.getOutgoingNodeConnector());
        assertEquals(msg, src, pc.getSourceAddress());
        assertEquals(msg, dst, pc.getDestinationAddress());

        byte[] sip = pc.getSourceIpAddress();
        assertEquals(msg, sender, sip);

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

        assertEquals(msg, map, pc.getObsoleteEntries());

        // getFrame()
        Ethernet ether = pc.getFrame();
        checkOutEthernetPacket("", ether, EtherTypes.ARP, src, dst, vlan, EtherTypes.IPv4,
                ARP.REQUEST, src, dst, sender, target);

        // createFrame()
        Ethernet newether;
        if (vlan < 0) {
            desc = convertForDescription(ether, nc, (short) 0);
            newether = pc.createFrame((short) 0);
        } else {
            desc = convertForDescription(ether, nc, vlan);
            newether = pc.createFrame(vlan);
        }
        assertEquals(msg, desc, pc.getDescription(nc));
        assertNotNull(msg, newether);
        assertEquals(msg, ether, newether);
        assertTrue(msg, ether != newether);

        // in case payload is null && RawPayload is null
        PacketContext pctxp = null;
        short newvlan = 0;
        if (vlan < 0) {
            ether.setPayload(null);
            pctxp = new PacketContext(null, ether);
        } else {
            Packet tag = ether.getPayload();
            tag.setPayload(null);
            pctxp = new PacketContext(null, ether);
            newvlan = vlan;
        }
        desc = convertForDescription(ether, nc, newvlan);
        newether = pctxp.createFrame(newvlan);
        assertEquals(msg, desc, pctxp.getDescription(nc));
        assertEquals(msg, newether, pctxp.getFrame());
        checkOutEthernetPacket(msg, newether, EtherTypes.ARP, src, dst, vlan, null,
                (short)-1, null, null, null, null);
        assertNotNull(msg, newether);

        // in case payload is null && RawPayload is not null
        RawPacket raw = null;
        try {
            raw = new RawPacket(ether.serialize());
            ether.setRawPayload(ether.serialize());
        } catch (ConstructionException e) {
            unexpected(e);
        } catch (PacketException e) {
            unexpected(e);
        }

        if (vlan < 0) {
            ether.setPayload(null);
            pctxp = new PacketContext(raw, ether);
            assertEquals(msg, ether, pctxp.getFrame());
        } else {
            Packet tag = ether.getPayload();
            tag.setPayload(null);
            tag.setRawPayload(new byte[]{0});
            pctxp = new PacketContext(raw, ether);
            assertEquals(msg, ether, pctxp.getFrame());
            newvlan = vlan;
        }
        desc = convertForDescription(ether, nc, newvlan);
        newether = pctxp.createFrame(newvlan);
        assertEquals(msg, desc, pctxp.getDescription(nc));
        checkOutEthernetPacket(msg, newether, EtherTypes.ARP, src, dst, vlan, null,
                (short)-1, null, null, null, null);
        assertNotNull(msg, newether);
    }

    /**
     * Test case for PacketContext(RawPacket raw, Ethernet ether)
     *
     * @param ether Ethernet frame
     * @param src   src MAC address
     * @param dst   dst MAC address
     * @param sender    sender IP address
     * @param vlan  VLAN ID
     * @param nc    a Node Connector.
     */
    private void testPacketContext(Ethernet ether, byte[] src, byte[] dst, byte[] sender, byte[] target, short vlan, NodeConnector nc) {
        InetAddress ipaddr = null;
        MacTableEntry me;
        String desc;
        String msg = "(ether)" + ether.toString() + ",(src)" + src + ",(dst)" + dst +
                     ",(sender)" + sender + ",(target)" + target;

        PacketContext pctx = new PacketContext(ether, nc);

        assertNull(msg, pctx.getRawPacket());
        assertNull(msg, pctx.getIncomingNetwork());
        try {
            assertNull(msg, pctx.getIncomingNodeConnector());
        } catch (NullPointerException expected) {
            assertEquals(msg, expected.getMessage(), null);
        }

        assertEquals(msg, nc, pctx.getOutgoingNodeConnector());

        assertNotNull(msg, pctx.getPayload());
        assertEquals(msg, src, pctx.getSourceAddress());
        assertEquals(msg, dst, pctx.getDestinationAddress());

        assertEquals(msg, sender, pctx.getSourceIpAddress());
        byte[] sip = pctx.getSourceIpAddress();

        try {
            ipaddr = InetAddress.getByAddress(sip);
        } catch (UnknownHostException e) {
            // This should never happen.
            fail(e.getMessage());
        }

        if (vlan < 0) {
            assertEquals(msg, (short)0, pctx.getVlan());
            me = new MacTableEntry(nc, (short) 0, ipaddr);
        } else {
            assertEquals(vlan, pctx.getVlan());
            me = new MacTableEntry(nc, vlan, ipaddr);
        }

        Long key = NetUtils.byteArray6ToLong(src);
        pctx.addObsoleteEntry(key, me);

        TreeMap<Long, MacTableEntry> map = new TreeMap<Long, MacTableEntry>();
        map.put(key, me);

        assertEquals(msg, map, pctx.getObsoleteEntries());

        // getFrame()
        assertEquals(ether, pctx.getFrame());
        checkOutEthernetPacket(msg, ether, EtherTypes.ARP, src, dst, vlan, EtherTypes.IPv4,
                ARP.REQUEST, src, dst, sender, target);

        // createFrame()
        Ethernet newether;
        if (vlan < 0) {
            desc = convertForDescription(ether, nc, (short) 0);
            newether = pctx.createFrame((short) 0);
        } else {
            desc = convertForDescription(ether, nc, vlan);
            newether = pctx.createFrame(vlan);
        }
        assertEquals(msg, desc, pctx.getDescription(nc));
        assertNotNull(msg, newether);
        assertEquals(msg, ether, newether);
        assertTrue(msg, ether != newether);

        if (vlan <= 0) {
            // set invalid sender ip address
            ARP arp = (ARP)newether.getPayload();
            arp.setProtocolType(EtherTypes.IPv4.shortValue());
            arp.setSenderProtocolAddress(new byte[] {0});
            PacketContext pctxl = new PacketContext(newether, nc);
            assertEquals(msg, null, pctxl.getSourceIpAddress());
            assertEquals(msg, null, pctxl.getSourceIpAddress());



        } else {
            // case of invalid source address
            IEEE8021Q tag = (IEEE8021Q)newether.getPayload();
            ARP arp = (ARP)tag.getPayload();
            arp.setProtocolType(EtherTypes.LLDP.shortValue());
            PacketContext pctxl = new PacketContext(newether, nc);
            assertEquals(msg, null, pctxl.getSourceIpAddress());

            // set invalid sender ip address
            arp.setProtocolType(EtherTypes.IPv4.shortValue());
            arp.setSenderProtocolAddress(new byte[] {0});
            pctxl = new PacketContext(newether, nc);
            assertEquals(msg, null, pctxl.getSourceIpAddress());
            assertEquals(msg, null, pctxl.getSourceIpAddress());
        }
    }

    /**
     * Create a string to compare description.
     *
     * @param ether a Ethernet Object
     * @param port  a NodeConnector
     * @param vlan  VLAN ID
     * @return A brief description of the specified ethernet frame.
     */
    private String convertForDescription(Ethernet ether, NodeConnector port, short vlan) {
        String srcmac = HexEncode.bytesToHexStringFormat(ether.getSourceMACAddress());
        String dstmac = HexEncode.bytesToHexStringFormat(ether.getDestinationMACAddress());
        int type = ether.getEtherType() & 0xffff;

        StringBuilder builder = new StringBuilder("src=");
        builder.append(srcmac).append(", dst=")
               .append(dstmac).append(", port=")
               .append(port).append(", type=0x")
               .append(Integer.toHexString(type))
               .append(", vlan=").append(vlan);

        return builder.toString();
    }
}