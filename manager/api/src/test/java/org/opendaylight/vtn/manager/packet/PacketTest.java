/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import java.util.Map;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link Packet}.
 */
public class PacketTest extends TestBase {
    /**
     * Test case for {@link Packet#deserialize(byte[], int, int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeserialize() throws Exception {
        Ethernet eth = new Ethernet();
        byte[] data = {
            // Destination MAC address
            10, 12, 14, 20, 55, 69,

            // Source MAC address
            -90, -20, -100, -82, -78, -97,

            // Ethernet type (ARP)
            0x08, 0x06,

            // ARP

            // Hardware type
            0, 1,

            // Protocol type (IPv4)
            0x08, 0x00,

            // Hardware address length
            6,

            // Protocol address length
            4,

            // ARP operation code
            0, 1,

            // Sender hardware address
            -90, -20, -100, -82, -78, -97,

            // Sender protocol address
            10, 20, 30, 55,

            // Target hardware address
            0, 0, 0, 0, 0, 0,

            // Target protocol address
            10, 20, 30, -2,
        };

        eth.deserialize(data, 0, data.length * Byte.SIZE);

        byte[] dstMac = eth.getDestinationMACAddress();
        byte[] srcMac = eth.getSourceMACAddress();
        short etype = eth.getEtherType();

        assertEquals(10, dstMac[0]);
        assertEquals(12, dstMac[1]);
        assertEquals(14, dstMac[2]);
        assertEquals(20, dstMac[3]);
        assertEquals(55, dstMac[4]);
        assertEquals(69, dstMac[5]);

        assertEquals(-90, srcMac[0]);
        assertEquals(-20, srcMac[1]);
        assertEquals(-100, srcMac[2]);
        assertEquals(-82, srcMac[3]);
        assertEquals(-78, srcMac[4]);
        assertEquals(-97, srcMac[5]);

        assertEquals(0x806, etype);

        ARP arp = (ARP)eth.getPayload();

        assertEquals((byte)0x1, arp.getHardwareType());
        assertEquals(2048, arp.getProtocolType());
        assertEquals((byte)0x6, arp.getHardwareAddressLength());
        assertEquals((byte)0x4, arp.getProtocolAddressLength());
        assertEquals(1, arp.getOpCode());

        byte[] sha = arp.getSenderHardwareAddress();
        byte[] spa = arp.getSenderProtocolAddress();
        byte[] tha = arp.getTargetHardwareAddress();
        byte[] tpa = arp.getTargetProtocolAddress();

        assertEquals((byte)0xA6, sha[0]);
        assertEquals((byte)0xEC, sha[1]);
        assertEquals((byte)0x9C, sha[2]);
        assertEquals((byte)0xAE, sha[3]);
        assertEquals((byte)0xB2, sha[4]);
        assertEquals((byte)0x9F, sha[5]);

        assertEquals((byte)10, spa[0]);
        assertEquals((byte)20, spa[1]);
        assertEquals((byte)30, spa[2]);
        assertEquals((byte)55, spa[3]);

        assertEquals((byte)0x0, tha[0]);
        assertEquals((byte)0x0, tha[1]);
        assertEquals((byte)0x0, tha[2]);
        assertEquals((byte)0x0, tha[3]);
        assertEquals((byte)0x0, tha[4]);
        assertEquals((byte)0x0, tha[5]);

        assertEquals((byte)10, tpa[0]);
        assertEquals((byte)20, tpa[1]);
        assertEquals((byte)30, tpa[2]);
        assertEquals((byte)-2, tpa[3]);
    }

    /**
     * Test case for {@link Packet#serialize()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        Ethernet eth = new Ethernet();
        Map<String, byte[]> header = eth.getHeaderFieldMap();

        byte[] dstMac = {10, 12, 14, 20, 55, 69};
        byte[] srcMac = {82, 97, 109, 117, 127, -50};
        short etype = 0x0806;

        eth.setDestinationMACAddress(dstMac);
        eth.setSourceMACAddress(srcMac);
        eth.setEtherType(etype);

        assertArrayEquals(dstMac, header.get("DestinationMACAddress"));
        assertArrayEquals(srcMac, header.get("SourceMACAddress"));

        byte[] etypeData = {0x08, 0x06};
        assertArrayEquals(etypeData, header.get("EtherType"));

        byte[] data = eth.serialize();
        assertEquals(10, data[0]);
        assertEquals(12, data[1]);
        assertEquals(14, data[2]);
        assertEquals(20, data[3]);
        assertEquals(55, data[4]);
        assertEquals(69, data[5]);

        assertEquals(82, data[6]);
        assertEquals(97, data[7]);
        assertEquals(109, data[8]);
        assertEquals(117, data[9]);
        assertEquals(127, data[10]);
        assertEquals(-50, data[11]);

        assertEquals(8, data[12]);
        assertEquals(6, data[13]);
    }
}
