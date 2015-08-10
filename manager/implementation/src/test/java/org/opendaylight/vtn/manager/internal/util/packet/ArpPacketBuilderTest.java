/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.packet;

import java.net.InetAddress;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.EtherTypes;

/**
 * JUnit test for {@link ArpPacketBuilder}.
 */
public class ArpPacketBuilderTest extends TestBase {
    /**
     * Ensure that an ARP packet can be built without changing parameters.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDefault() throws Exception {
        EtherAddress src = new EtherAddress(0x001122aabbccL);
        EtherAddress dst = EtherAddress.BROADCAST;
        InetAddress target = InetAddress.getByName("10.1.2.3");
        InetAddress ip6 = InetAddress.getByName("::1");
        Ip4Network spa = new Ip4Network(0);
        Ip4Network tpa = new Ip4Network(target);
        ArpPacketBuilder builder = new ArpPacketBuilder();
        Ethernet ether = builder.build(src, target);
        verify(ether, src, dst, spa, tpa, ARP.REQUEST, 0);
        assertEquals(null, builder.build(src, ip6));

        dst = new EtherAddress(0x0123456789abL);
        ether = builder.build(src, dst, target);
        verify(ether, src, dst, spa, tpa, ARP.REQUEST, 0);
        assertEquals(null, builder.build(src, ip6));

        ether = builder.build(src, dst, tpa);
        verify(ether, src, dst, spa, tpa, ARP.REQUEST, 0);
        assertEquals(null, builder.build(src, ip6));
    }

    /**
     * Ensure that an ARP packet can be build with changing parameters.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testParameters() throws Exception {
        EtherAddress src = new EtherAddress(0xa0b1c2d3e4f5L);
        EtherAddress dst = new EtherAddress(0xffffaaaabbbbL);
        EtherAddress bcast = EtherAddress.BROADCAST;
        InetAddress target = InetAddress.getByName("192.168.100.200");
        Ip4Network tpa = new Ip4Network(target);
        Ip4Network spa = new Ip4Network("192.168.100.254");
        Ip4Network spaZero = new Ip4Network(0);
        InetAddress ip6 = InetAddress.getByName("::1");
        int[] vlanIds = {
            0, 1, 2, 4094, 4095,
        };
        short[] opCodes = {
            ARP.REQUEST, ARP.REPLY,
        };
        for (int vid: vlanIds) {
            ArpPacketBuilder builder = new ArpPacketBuilder(vid);
            Ethernet ether = builder.build(src, target);
            verify(ether, src, bcast, spaZero, tpa, ARP.REQUEST, vid);
            assertEquals(null, builder.build(src, ip6));

            ether = builder.build(src, bcast, target);
            verify(ether, src, bcast, spaZero, tpa, ARP.REQUEST, vid);
            assertEquals(null, builder.build(src, ip6));

            ether = builder.build(src, bcast, tpa);
            verify(ether, src, bcast, spaZero, tpa, ARP.REQUEST, vid);
            assertEquals(null, builder.build(src, ip6));

            builder.setSenderProtocolAddress(spa);
            ether = builder.build(src, target);
            verify(ether, src, bcast, spa, tpa, ARP.REQUEST, vid);
            assertEquals(null, builder.build(src, ip6));

            dst = new EtherAddress(0x0123456789abL);
            ether = builder.build(src, dst, target);
            verify(ether, src, dst, spa, tpa, ARP.REQUEST, vid);
            assertEquals(null, builder.build(src, ip6));

            ether = builder.build(src, dst, tpa);
            verify(ether, src, dst, spa, tpa, ARP.REQUEST, vid);
            assertEquals(null, builder.build(src, ip6));

            for (short op: opCodes) {
                builder = new ArpPacketBuilder(vid, op);
                ether = builder.build(src, target);
                verify(ether, src, bcast, spaZero, tpa, op, vid);
                assertEquals(null, builder.build(src, ip6));

                ether = builder.build(src, bcast, target);
                verify(ether, src, bcast, spaZero, tpa, op, vid);
                assertEquals(null, builder.build(src, ip6));

                ether = builder.build(src, bcast, tpa);
                verify(ether, src, bcast, spaZero, tpa, op, vid);
                assertEquals(null, builder.build(src, ip6));

                builder.setSenderProtocolAddress(spa);
                ether = builder.build(src, target);
                verify(ether, src, bcast, spa, tpa, op, vid);
                assertEquals(null, builder.build(src, ip6));

                ether = builder.build(src, dst, target);
                verify(ether, src, dst, spa, tpa, op, vid);
                assertEquals(null, builder.build(src, ip6));

                ether = builder.build(src, dst, tpa);
                verify(ether, src, dst, spa, tpa, op, vid);
                assertEquals(null, builder.build(src, ip6));
            }
        }
    }

    /**
     * Verify the given ARP packet.
     *
     * @param ether   An {@link Ethernet} instance that contains an ARP
     *                message.
     * @param src     The expected source MAC address.
     * @param dst     The expected destination MAC address.
     * @param sender  The expected sender protocol address.
     * @param target  The expected target protocol address.
     * @param op      The expected ARP operation code.
     * @param vid     The expected VLAN ID.
     */
    private void verify(Ethernet ether, EtherAddress src, EtherAddress dst,
                        Ip4Network sender, Ip4Network target, short op,
                        int vid) {
        assertArrayEquals(src.getBytes(), ether.getSourceMACAddress());
        assertArrayEquals(dst.getBytes(), ether.getDestinationMACAddress());
        short ethType = ether.getEtherType();
        Packet payload = ether.getPayload();
        if (vid != 0) {
            assertEquals(EtherTypes.VLANTAGGED.shortValue(), ethType);
            assertTrue(payload instanceof IEEE8021Q);
            IEEE8021Q tag = (IEEE8021Q)payload;
            byte zero = 0;
            assertEquals(zero, tag.getCfi());
            assertEquals(zero, tag.getPcp());
            assertEquals((short)vid, tag.getVid());
            ethType = tag.getEtherType();
            payload = tag.getPayload();
        }

        assertEquals(EtherTypes.ARP.shortValue(), ethType);
        assertTrue(payload instanceof ARP);
        ARP arp = (ARP)payload;

        byte[] tha = (dst.equals(EtherAddress.BROADCAST))
            ? new byte[6] : dst.getBytes();
        assertEquals(ARP.HW_TYPE_ETHERNET, arp.getHardwareType());
        assertEquals(EtherTypes.IPv4.shortValue(), arp.getProtocolType());
        assertEquals((byte)6, arp.getHardwareAddressLength());
        assertEquals((byte)4, arp.getProtocolAddressLength());
        assertEquals(op, arp.getOpCode());
        assertArrayEquals(src.getBytes(), arp.getSenderHardwareAddress());
        assertArrayEquals(tha, arp.getTargetHardwareAddress());
        assertArrayEquals(sender.getBytes(), arp.getSenderProtocolAddress());
        assertArrayEquals(target.getBytes(), arp.getTargetProtocolAddress());
    }
}
