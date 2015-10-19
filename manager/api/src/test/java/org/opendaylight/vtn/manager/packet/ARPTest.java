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
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link ARP}.
 */
public class ARPTest extends TestBase {
    /**
     * Test case for hardware type.
     *
     * <ul>
     *   <li>{@link ARP#getHardwareType()}</li>
     *   <li>{@link ARP#setHardwareType(short)}</li>
     * </ul>
     */
    @Test
    public void testGetHardwareType() {
        ARP arp = new ARP();
        assertEquals((short)0, arp.getHardwareType());

        short[] types = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short hwtype: types) {
            assertSame(arp, arp.setHardwareType(hwtype));
            assertEquals(hwtype, arp.getHardwareType());
        }
    }

    /**
     * Test case for protocol type.
     *
     * <ul>
     *   <li>{@link ARP#getProtocolType()}</li>
     *   <li>{@link ARP#setProtocolType(short)}</li>
     * </ul>
     */
    @Test
    public void testGetProtocolType() {
        ARP arp = new ARP();
        assertEquals((short)0, arp.getProtocolType());

        short[] types = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short ptype: types) {
            assertSame(arp, arp.setProtocolType(ptype));
            assertEquals(ptype, arp.getProtocolType());
        }
    }

    /**
     * Test case for the hardware address length.
     *
     * <ul>
     *   <li>{@link ARP#getHardwareAddressLength()}</li>
     *   <li>{@link ARP#setHardwareAddressLength(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetHardwareAddressLength() {
        ARP arp = new ARP();
        assertEquals((byte)0, arp.getHardwareAddressLength());

        byte[] lengths = {0, 1, 9, 57, 127, -128, -127, -15, -2, -1};
        for (byte len: lengths) {
            assertSame(arp, arp.setHardwareAddressLength(len));
            assertEquals(len, arp.getHardwareAddressLength());
        }
    }

    /**
     * Test cast for the protocol address length.
     *
     * <ul>
     *   <li>{@link ARP#getProtocolAddressLength()}</li>
     *   <li>{@link ARP#setProtocolAddressLength(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetProtocolAddressLength() {
        ARP arp = new ARP();
        assertEquals((byte)0, arp.getProtocolAddressLength());

        byte[] lengths = {0, 1, 9, 57, 127, -128, -127, -15, -2, -1};
        for (byte len: lengths) {
            assertSame(arp, arp.setProtocolAddressLength(len));
            assertEquals(len, arp.getProtocolAddressLength());
        }
    }

    /**
     * Test case for the operation code.
     *
     * <ul>
     *   <li>{@link ARP#getOpCode()}</li>
     *   <li>{@link ARP#setOpCode(short)}</li>
     * </ul>
     */
    @Test
    public void testGetOpCode() {
        ARP arp = new ARP();
        assertEquals((short)0, arp.getOpCode());

        short[] codes = {0, 1, 2, 100, 127, 128, 30123, 32767, -30000, -2, -1};
        for (short op: codes) {
            assertSame(arp, arp.setOpCode(op));
            assertEquals(op, arp.getOpCode());
        }
    }

    /**
     * Test case for the sender hardware address.
     *
     * <ul>
     *   <li>{@link ARP#getSenderHardwareAddress()}</li>
     *   <li>{@link ARP#setSenderHardwareAddress(byte[])}</li>
     * </ul>
     */
    @Test
    public void testGetSenderHardwareAddress() {
        ARP arp = new ARP();
        assertEquals(null, arp.getSenderHardwareAddress());

        EtherAddress[] addrs = {
            new EtherAddress(0x123456789abcL),
            new EtherAddress(0x001122334455L),
            new EtherAddress(0x182cf31fa341L),
        };

        for (EtherAddress eaddr: addrs) {
            byte[] baddr = eaddr.getBytes();
            assertSame(arp, arp.setSenderHardwareAddress(baddr));

            // The specified array should be copied.
            baddr[0] = 0;
            baddr[1] = 0;
            assertArrayEquals(eaddr.getBytes(),
                              arp.getSenderHardwareAddress());
        }
    }

    /**
     * Test case for the sender protocol address.
     *
     * <ul>
     *   <li>{@link ARP#getSenderProtocolAddress()}</li>
     *   <li>{@link ARP#setSenderProtocolAddress(byte[])}</li>
     * </ul>
     */
    @Test
    public void testGetSenderProtocolAddress() {
        ARP arp = new ARP();
        assertEquals(null, arp.getSenderProtocolAddress());

        Ip4Network[] addrs = {
            new Ip4Network("192.168.123.234"),
            new Ip4Network("10.20.30.40"),
            new Ip4Network("220.39.195.254"),
        };
        for (Ip4Network addr: addrs) {
            byte[] baddr = addr.getBytes();
            assertSame(arp, arp.setSenderProtocolAddress(baddr));

            // The specified array should be copied.
            baddr[0] = 0;
            baddr[1] = 0;
            assertArrayEquals(addr.getBytes(), arp.getSenderProtocolAddress());
        }
    }

    /**
     * Test case for the target hardware address.
     *
     * <ul>
     *   <li>{@link ARP#getSenderHardwareAddress()}</li>
     *   <li>{@link ARP#setSenderHardwareAddress(byte[])}</li>
     * </ul>
     */
    @Test
    public void testGetTargetHardwareAddress() {
        ARP arp = new ARP();
        assertEquals(null, arp.getTargetHardwareAddress());

        EtherAddress[] addrs = {
            new EtherAddress(0x123456789abcL),
            new EtherAddress(0x001122334455L),
            new EtherAddress(0x182cf31fa341L),
        };

        for (EtherAddress eaddr: addrs) {
            byte[] baddr = eaddr.getBytes();
            assertSame(arp, arp.setTargetHardwareAddress(baddr));

            // The specified array should be copied.
            baddr[0] = 0;
            baddr[1] = 0;
            assertArrayEquals(eaddr.getBytes(),
                              arp.getTargetHardwareAddress());
        }
    }

    /**
     * Test case for the target protocol address.
     *
     * <ul>
     *   <li>{@link ARP#getTargetProtocolAddress()}</li>
     *   <li>{@link ARP#setTargetProtocolAddress(byte[])}</li>
     * </ul>
     */
    @Test
    public void testGetTargetProtocolAddress() {
        ARP arp = new ARP();
        assertEquals(null, arp.getTargetProtocolAddress());

        Ip4Network[] addrs = {
            new Ip4Network("192.168.123.234"),
            new Ip4Network("10.20.30.40"),
            new Ip4Network("220.39.195.254"),
        };
        for (Ip4Network addr: addrs) {
            byte[] baddr = addr.getBytes();
            assertSame(arp, arp.setTargetProtocolAddress(baddr));

            // The specified array should be copied.
            baddr[0] = 0;
            baddr[1] = 0;
            assertArrayEquals(addr.getBytes(), arp.getTargetProtocolAddress());
        }
    }

    /**
     * Test case for {@link ARP#clone()}.
     */
    @Test
    public void testClone() {
        short hwtype = 1;
        short ptype = 123;
        byte hlen = 6;
        byte plen = 4;
        short op = 7;
        EtherAddress sha = new EtherAddress(0x001122334455L);
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

        ARP copy = arp.clone();
        assertNotSame(arp, copy);
        assertEquals(arp, copy);
        assertEquals(arp.hashCode(), copy.hashCode());

        // Modifying the source packet should never affect a deep copy.
        short hwtype1 = 3;
        short ptype1 = 45;
        byte hlen1 = 4;
        byte plen1 = 8;
        short op1 = 1;
        EtherAddress sha1 = new EtherAddress(0xaabbccddeeffL);
        EtherAddress tha1 = new EtherAddress(0x000011112222L);
        Ip4Network spa1 = new Ip4Network("10.20.30.40");
        Ip4Network tpa1 = new Ip4Network("10.20.30.1");
        arp.setHardwareType(hwtype1).
            setProtocolType(ptype1).
            setHardwareAddressLength(hlen1).
            setProtocolAddressLength(plen1).
            setOpCode(op1).
            setSenderHardwareAddress(sha1.getBytes()).
            setTargetHardwareAddress(tha1.getBytes()).
            setSenderProtocolAddress(spa1.getBytes()).
            setTargetProtocolAddress(tpa1.getBytes());

        assertEquals(hwtype, copy.getHardwareType());
        assertEquals(ptype, copy.getProtocolType());
        assertEquals(hlen, copy.getHardwareAddressLength());
        assertEquals(plen, copy.getProtocolAddressLength());
        assertEquals(op, copy.getOpCode());
        assertArrayEquals(sha.getBytes(), copy.getSenderHardwareAddress());
        assertArrayEquals(tha.getBytes(), copy.getTargetHardwareAddress());
        assertArrayEquals(spa.getBytes(), copy.getSenderProtocolAddress());
        assertArrayEquals(tpa.getBytes(), copy.getTargetProtocolAddress());

        assertEquals(hwtype1, arp.getHardwareType());
        assertEquals(ptype1, arp.getProtocolType());
        assertEquals(hlen1, arp.getHardwareAddressLength());
        assertEquals(plen1, arp.getProtocolAddressLength());
        assertEquals(op1, arp.getOpCode());
        assertArrayEquals(sha1.getBytes(), arp.getSenderHardwareAddress());
        assertArrayEquals(tha1.getBytes(), arp.getTargetHardwareAddress());
        assertArrayEquals(spa1.getBytes(), arp.getSenderProtocolAddress());
        assertArrayEquals(tpa1.getBytes(), arp.getTargetProtocolAddress());
    }

    /**
     * Test for serialization and deserialization.
     *
     * <ul>
     *   <li>{@link Packet#serialize()}</li>
     *   <li>{@link Packet#deserialize(byte[], int, int)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        byte[] raw = {
            // Hardware type.
            (byte)0x00, (byte)0x01,

            // Protocol type.
            (byte)0x08, (byte)0x00,

            // Hardware address length.
            (byte)0x06,

            // Protocol address length.
            (byte)0x04,

            // Operation code.
            (byte)0x00, 0x01,

            // Sender hardware address.
            (byte)0x01, (byte)0x23, (byte)0x45, (byte)0x67,
            (byte)0x89, (byte)0xab,

            // Sender protocol address.
            (byte)10, (byte)20, (byte)30, (byte)40,

            // Target hardware address.
            (byte)0xff, (byte)0xff, (byte)0xff, (byte)0xff,
            (byte)0xff, (byte)0xff,

            // Target protocol address.
            (byte)192, (byte)168, (byte)123, (byte)254,
        };

        EtherAddress sha = new EtherAddress(0x0123456789abL);
        EtherAddress tha = EtherAddress.BROADCAST;
        Ip4Network spa = new Ip4Network("10.20.30.40");
        Ip4Network tpa = new Ip4Network("192.168.123.254");

        // Deserialize raw packet.
        ARP arp = new ARP();
        arp.deserialize(raw, 0, raw.length * Byte.SIZE);
        assertEquals((short)1, arp.getHardwareType());
        assertEquals((short)0x800, arp.getProtocolType());
        assertEquals((byte)6, arp.getHardwareAddressLength());
        assertEquals((byte)4, arp.getProtocolAddressLength());
        assertEquals((short)1, arp.getOpCode());
        assertArrayEquals(sha.getBytes(), arp.getSenderHardwareAddress());
        assertArrayEquals(tha.getBytes(), arp.getTargetHardwareAddress());
        assertArrayEquals(spa.getBytes(), arp.getSenderProtocolAddress());
        assertArrayEquals(tpa.getBytes(), arp.getTargetProtocolAddress());
        assertEquals(null, arp.getPayload());
        assertEquals(null, arp.getRawPayload());
        assertEquals(false, arp.isCorrupted());

        // Serialize packet.
        assertArrayEquals(raw, arp.serialize());

        // Serialize an empty packet.
        arp = new ARP();
        byte[] expected = new byte[28];
        assertArrayEquals(expected, arp.serialize());
    }
}
