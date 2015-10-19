/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link Ethernet}.
 */
public class EthernetTest extends TestBase {
    /**
     * Test case for {@link Ethernet#getPayloadClass(short)}.
     */
    @Test
    public void testGetPayloadClass() {
        short[] badTypes = {
            (short)0x0000, (short)0x0011, (short)0x07ff, (short)0x0805,
            (short)0x0899, (short)0x8001, (short)0x8101, (short)0x88a8,
            (short)0x88cc, (short)0x9999, (short)0xabcd, (short)0xcdef,
            (short)0xcef0, (short)0xf000, (short)0xfff0, (short)0xffff,
        };

        for (short type: badTypes) {
            assertEquals(null, Ethernet.getPayloadClass(type));
        }

        assertEquals(IPv4.class, Ethernet.getPayloadClass((short)0x0800));
        assertEquals(ARP.class, Ethernet.getPayloadClass((short)0x0806));
        assertEquals(IEEE8021Q.class, Ethernet.getPayloadClass((short)0x8100));
    }

    /**
     * Test case for the destination MAC address.
     *
     * <ul>
     *   <li>{@link Ethernet#getDestinationMACAddress()}</li>
     *   <li>{@link Ethernet#setDestinationMACAddress(byte[])}</li>
     * </ul>
     */
    @Test
    public void testGetDestinationMACAddress() {
        Ethernet eth = new Ethernet();
        assertEquals(null, eth.getDestinationMACAddress());

        EtherAddress[] addrs = {
            new EtherAddress(0x123456789abcL),
            new EtherAddress(0),
            EtherAddress.BROADCAST,
            new EtherAddress(0x00abcdef1122L),
        };
        for (EtherAddress eaddr: addrs) {
            byte[] mac = eaddr.getBytes();
            assertSame(eth, eth.setDestinationMACAddress(mac));

            // Ensure the specified byte array is copied.
            mac[0] = (byte)0xae;
            assertArrayEquals(eaddr.getBytes(),
                              eth.getDestinationMACAddress());
        }
    }

    /**
     * Test case for the source MAC address.
     *
     * <ul>
     *   <li>{@link Ethernet#getSourceMACAddress()}</li>
     *   <li>{@link Ethernet#setSourceMACAddress(byte[])}</li>
     * </ul>
     */
    @Test
    public void testSourceMACAddress() {
        Ethernet eth = new Ethernet();
        assertEquals(null, eth.getSourceMACAddress());

        EtherAddress[] addrs = {
            new EtherAddress(0x123456789abcL),
            new EtherAddress(0),
            EtherAddress.BROADCAST,
            new EtherAddress(0x00abcdef1122L),
        };
        for (EtherAddress eaddr: addrs) {
            byte[] mac = eaddr.getBytes();
            assertSame(eth, eth.setSourceMACAddress(mac));

            // Ensure the specified byte array is copied.
            mac[0] = (byte)0xae;
            assertArrayEquals(eaddr.getBytes(),
                              eth.getSourceMACAddress());
        }
    }

    /**
     * Test case for the Ethernet type.
     *
     * <ul>
     *   <li>{@link Ethernet#getEtherType()}</li>
     *   <li>{@link Ethernet#setEtherType(short)}</li>
     * </ul>
     */
    @Test
    public void testGetEthertype() throws Exception {
        Ethernet eth = new Ethernet();
        assertEquals((short)0, eth.getEtherType());


        short[] values = {
            (short)0x0000, (short)0x0001, (short)0x0020, (short)0x03ff,
            (short)0x07ff, (short)0x0800, (short)0x0999, (short)0x1abc,
            (short)0x5678, (short)0x7fff, (short)0x8000, (short)0x8888,
            (short)0xabcd, (short)0xcdef, (short)0xff00, (short)0xffff,
        };
        for (short v: values) {
            assertSame(eth, eth.setEtherType(v));
            assertEquals(v, eth.getEtherType());
        }
    }

    /**
     * Test case for {@link Ethernet#clone()}.
     */
    @Test
    public void testClone() {
        EtherAddress src = new EtherAddress(0x001122334455L);
        EtherAddress dst = new EtherAddress(0x0abbccddeeffL);
        short etype = EtherTypes.VLAN.shortValue();
        Ethernet eth = new Ethernet().
            setSourceMACAddress(src.getBytes()).
            setDestinationMACAddress(dst.getBytes()).
            setEtherType(etype);

        byte pcp = 0;
        byte cfi = 0;
        short vid = 4095;
        short vetype = EtherTypes.ARP.shortValue();
        IEEE8021Q vlan = new IEEE8021Q().
            setPcp(pcp).
            setCfi(cfi).
            setVid(vid).
            setEtherType(vetype);

        short hwtype = 1;
        short ptype = 0x800;
        byte hlen = 6;
        byte plen = 4;
        short op = 7;
        EtherAddress sha = new EtherAddress(0x010203040506L);
        EtherAddress tha = new EtherAddress(0xa0b0c0d0e0f0L);
        Ip4Network spa = new Ip4Network("1.2.3.4");
        Ip4Network tpa = new Ip4Network("192.168.34.56");
        ARP arp = new ARP().
            setHardwareType(hwtype).
            setProtocolType(ptype).
            setHardwareAddressLength(hlen).
            setProtocolAddressLength(plen).
            setOpCode(op).
            setSenderHardwareAddress(sha.getBytes()).
            setTargetHardwareAddress(tha.getBytes()).
            setSenderProtocolAddress(spa.getBytes()).
            setTargetProtocolAddress(tpa.getBytes());

        vlan.setPayload(arp);
        eth.setPayload(vlan);

        Ethernet copy = eth.clone();
        assertNotSame(eth, copy);
        assertEquals(eth, copy);
        assertEquals(eth.hashCode(), copy.hashCode());
        assertEquals(null, copy.getRawPayload());

        Packet payload1 = copy.getPayload();
        assertNotSame(vlan, payload1);
        assertEquals(vlan, payload1);
        assertEquals(null, payload1.getRawPayload());

        Packet payload2 = payload1.getPayload();
        assertNotSame(arp, payload2);
        assertEquals(arp, payload2);
        assertEquals(null, payload2.getPayload());
        assertEquals(null, payload2.getRawPayload());

        // Modifying the source packet should never affect a deep copy.
        EtherAddress src1 = new EtherAddress(0x0L);
        EtherAddress dst1 = EtherAddress.BROADCAST;
        short etype1 = 0x123;
        eth.setSourceMACAddress(src1.getBytes()).
            setDestinationMACAddress(dst1.getBytes()).
            setEtherType(etype1);
        eth.setPayload(null);

        assertArrayEquals(src.getBytes(), copy.getSourceMACAddress());
        assertArrayEquals(dst.getBytes(), copy.getDestinationMACAddress());
        assertEquals(etype, copy.getEtherType());
        assertEquals(payload1, copy.getPayload());
        assertEquals(payload2, payload1.getPayload());
        assertEquals(null, payload2.getPayload());

        assertArrayEquals(src1.getBytes(), eth.getSourceMACAddress());
        assertArrayEquals(dst1.getBytes(), eth.getDestinationMACAddress());
        assertEquals(etype1, eth.getEtherType());
        assertEquals(null, eth.getPayload());
    }
}
