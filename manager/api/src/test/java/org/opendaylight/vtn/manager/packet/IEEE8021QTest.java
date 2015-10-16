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
 * JUnit test for {@link IEEE8021Q}.
 */
public class IEEE8021QTest extends TestBase {
    /**
     * Test case for the priority code point.
     *
     * <ul>
     *   <li>{@link IEEE8021Q#getPcp()}</li>
     *   <li>{@link IEEE8021Q#setPcp(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetPcp() {
        IEEE8021Q vlan = new IEEE8021Q();
        assertEquals((byte)0, vlan.getPcp());

        for (byte v = 0; v <= 7; v++) {
            assertSame(vlan, vlan.setPcp(v));
            assertEquals(v, vlan.getPcp());
        }
    }

    /**
     * Test case for the canonical format indicator.
     *
     * <ul>
     *   <li>{@link IEEE8021Q#getCfi()}</li>
     *   <li>{@link IEEE8021Q#setCfi(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetCfi() throws Exception {
        IEEE8021Q vlan = new IEEE8021Q();
        assertEquals((byte)0, vlan.getCfi());

        for (byte v = 0; v <= 1; v++) {
            assertSame(vlan, vlan.setCfi(v));
            assertEquals(v, vlan.getCfi());
        }
    }

    /**
     * Test case for the VLAN ID.
     *
     * <ul>
     *   <li>{@link IEEE8021Q#getVid()}</li>
     *   <li>{@link IEEE8021Q#setVid(short)}</li>
     * </ul>
     */
    @Test
    public void testGetVid() throws Exception {
        IEEE8021Q vlan = new IEEE8021Q();
        assertEquals((short)0, vlan.getVid());

        short[] values = {
            0, 1, 4, 21, 127, 128, 254, 255, 256, 257, 1023, 1024, 1025,
            2000, 3000, 4000, 4093, 4094, 4095,
        };
        for (short v: values) {
            assertSame(vlan, vlan.setVid(v));
            assertEquals(v, vlan.getVid());
        }
    }

    /**
     * Test case for the Ethernet type.
     *
     * <ul>
     *   <li>{@link IEEE8021Q#getEtherType()}</li>
     *   <li>{@link IEEE8021Q#setEtherType(short)}</li>
     * </ul>
     */
    @Test
    public void testGetEthertype() throws Exception {
        IEEE8021Q vlan = new IEEE8021Q();
        assertEquals((short)0, vlan.getVid());

        short[] values = {
            (short)0x0000, (short)0x0001, (short)0x0020, (short)0x03ff,
            (short)0x07ff, (short)0x0800, (short)0x0999, (short)0x1abc,
            (short)0x5678, (short)0x7fff, (short)0x8000, (short)0x8888,
            (short)0xabcd, (short)0xcdef, (short)0xff00, (short)0xffff,
        };
        for (short v: values) {
            assertSame(vlan, vlan.setEtherType(v));
            assertEquals(v, vlan.getEtherType());
        }
    }

    /**
     * Ensure that the IEEE8021Q packet is deserializable.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeserialize() throws Exception {
        byte[] data = {
            // Destination MAC address.
            (byte)0x0a, (byte)0x0c, (byte)0x0e, (byte)0x14,
            (byte)0x37, (byte)0x45,

            // Source MAC address.
            (byte)0xa6, (byte)0xec, (byte)0x9c, (byte)0xae,
            (byte)0xb2, (byte)0x9f,

            // Ether type.
            (byte)0x81, (byte)0x00,

            // VLAN tag
            // PCP, CFI, and VLAN ID.
            (byte)0xaf, (byte)0xfe,

            // Ethernet type.
            0x08, 0x06,

            // ARP

            // Hardware type.
            0x00, 0x01,

            // Protocol type.
            0x08, 0x00,

            // Hardware address length.
            0x06,

            // Protocol address length.
            0x04,

            // ARP operation code.
            0x00, 0x01,

            // Sender hardware address.
            (byte)0xa6, (byte)0xec, (byte)0x9c, (byte)0xae,
            (byte)0xb2, (byte)0x9f,

            // Sender protocol address.
            (byte)0x09, (byte)0x09, (byte)0x09, (byte)0x01,

            // Target hardware address.
            (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x00, (byte)0x00,

            // Target protocol address.
            (byte)0x09, (byte)0x09, (byte)0x09, (byte)0xfe,
        };

        short startOffset = 0;
        short numBits = (short)(data.length * 8);
        Ethernet eth = new Ethernet();
        eth.deserialize(data, startOffset, numBits);

        assertEquals((short)0x8100, eth.getEtherType());

        IEEE8021Q vlan = (IEEE8021Q)eth.getPayload();
        assertEquals((byte)0, vlan.getCfi());
        assertEquals((byte)5, vlan.getPcp());
        assertEquals((short)4094, vlan.getVid());
        assertEquals((short)0x0806, vlan.getEtherType());

        ARP arp = (ARP)vlan.getPayload();
        assertEquals((short)0x1, arp.getHardwareType());
        assertEquals((short)0x0800, arp.getProtocolType());
        assertEquals((byte)0x06, arp.getHardwareAddressLength());
        assertEquals((byte)0x04, arp.getProtocolAddressLength());
        assertEquals((short)1, arp.getOpCode());

        byte[] sha = {
            (byte)0xa6, (byte)0xec, (byte)0x9c, (byte)0xae,
            (byte)0xb2, (byte)0x9f,
        };
        byte[] spa = {
            (byte)0x09, (byte)0x09, (byte)0x09, (byte)0x01,
        };
        byte[] tha = {
            (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x00, (byte)0x00,
        };
        byte[] tpa = {
            (byte)0x09, (byte)0x09, (byte)0x09, (byte)0xfe,
        };
        assertArrayEquals(sha, arp.getSenderHardwareAddress());
        assertArrayEquals(spa, arp.getSenderProtocolAddress());
        assertArrayEquals(tha, arp.getTargetHardwareAddress());
        assertArrayEquals(tpa, arp.getTargetProtocolAddress());
    }

    /**
     * Enshre that the IEEE8021Q packet is serializable.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSerialize() throws Exception {

        byte[] dMac = {
            (byte)0xa, (byte)0xc, (byte)0xe, (byte)0x14,
            (byte)0x37, (byte)0x45,
        };
        byte[] sMac = {
            (byte)0xa6, (byte)0xec, (byte)0x9c, (byte)0xae,
            (byte)0xb2, (byte)0x9f,
        };

        Ethernet eth = new Ethernet().
            setDestinationMACAddress(dMac).
            setSourceMACAddress(sMac).
            setEtherType((short)0x8100);

        IEEE8021Q vlan = new IEEE8021Q().
            setCfi((byte)0x0).
            setPcp((byte)0x5).
            setVid((short)4094).
            setEtherType((short)0x806);

        eth.setPayload(vlan);

        ARP arp = new ARP().
            setHardwareType((short)1).
            setProtocolType((short)0x800).
            setHardwareAddressLength((byte)0x6).
            setProtocolAddressLength((byte)0x4).
            setOpCode((byte)0x1);

        byte[] sha = {
            (byte)0xa6, (byte)0xec, (byte)0x9c, (byte)0xae,
            (byte)0xb2, (byte)0x9f,
        };
        byte[] tha = {
            (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x00, (byte)0x00,
        };
        byte[] spa = {(byte)0x9, (byte)0x9, (byte)0x9, (byte)0x01};
        byte[] tpa = {(byte)0x9, (byte)0x9, (byte)0x9, (byte)0xfe};
        arp.setSenderHardwareAddress(sha);
        arp.setSenderProtocolAddress(spa);
        arp.setTargetHardwareAddress(tha);
        arp.setTargetProtocolAddress(tpa);

        vlan.setPayload(arp);

        byte[] data = eth.serialize();

        // Ethernet header

         // Destination MAC adress.
        assertEquals((byte)0x0a, data[0]);
        assertEquals((byte)0x0c, data[1]);
        assertEquals((byte)0x0e, data[2]);
        assertEquals((byte)0x14, data[3]);
        assertEquals((byte)0x37, data[4]);
        assertEquals((byte)0x45, data[5]);

         // Source MAC address.
        assertEquals((byte)0xa6, data[6]);
        assertEquals((byte)0xec, data[7]);
        assertEquals((byte)0x9c, data[8]);
        assertEquals((byte)0xae, data[9]);
        assertEquals((byte)0xb2, data[10]);
        assertEquals((byte)0x9f, data[11]);

        // Ethernet type.
        assertEquals((byte)0x81, data[12]);
        assertEquals((byte)0x00, data[13]);

        // VLAN Tag

        // PCP, CFI, VLAN ID
        assertEquals((byte)0xaf, data[14]);
        assertEquals((byte)0xfe, data[15]);

        // Ethernet type.
        assertEquals((byte)0x08, data[16]);
        assertEquals((byte)0x06, data[17]);

        // ARP

        // Hardware type.
        assertEquals((byte)0x00, data[18]);
        assertEquals((byte)0x01, data[19]);

        // Protocol type.
        assertEquals((byte)0x08, data[20]);
        assertEquals((byte)0x00, data[21]);

        // Hardware address length.
        assertEquals((byte)0x06, data[22]);

        // Protocol address length.
        assertEquals((byte)0x04, data[23]);

        // ARP operation code.
        assertEquals((byte)0x00, data[24]);
        assertEquals((byte)0x01, data[25]);

        // Sender hardware address.
        assertEquals((byte)0xa6, data[26]);
        assertEquals((byte)0xec, data[27]);
        assertEquals((byte)0x9c, data[28]);
        assertEquals((byte)0xae, data[29]);
        assertEquals((byte)0xb2, data[30]);
        assertEquals((byte)0x9f, data[31]);

        // Sender protocol address.
        assertEquals((byte)0x09, data[32]);
        assertEquals((byte)0x09, data[33]);
        assertEquals((byte)0x09, data[34]);
        assertEquals((byte)0x01, data[35]);

        // Target hardware address.
        assertEquals((byte)0x00, data[36]);
        assertEquals((byte)0x00, data[37]);
        assertEquals((byte)0x00, data[38]);
        assertEquals((byte)0x00, data[39]);
        assertEquals((byte)0x00, data[40]);
        assertEquals((byte)0x00, data[41]);

        // Target protocol address
        assertEquals((byte)0x09, data[42]);
        assertEquals((byte)0x09, data[43]);
        assertEquals((byte)0x09, data[44]);
        assertEquals((byte)0xfe, data[45]);
    }

    /**
     * Test case for {@link IEEE8021Q#clone()}.
     */
    @Test
    public void testClone() {
        byte pcp = 5;
        byte cfi = 0;
        short vid = 104;
        short etype = 0x806;
        IEEE8021Q vlan = new IEEE8021Q().
            setPcp(pcp).
            setCfi(cfi).
            setVid(vid).
            setEtherType(etype);

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

        vlan.setPayload(arp);

        IEEE8021Q copy = vlan.clone();
        assertNotSame(vlan, copy);
        assertEquals(vlan, copy);
        assertEquals(vlan.hashCode(), copy.hashCode());
        assertEquals(null, copy.getRawPayload());

        Packet payload = copy.getPayload();
        assertNotSame(arp, payload);
        assertEquals(arp, payload);
        assertEquals(null, payload.getRawPayload());

        // Modifying the source packet should never affect a deep copy.
        byte pcp1 = 7;
        byte cfi1 = 1;
        short vid1 = 4095;
        short etype1 = 0x811;
        vlan.setPcp(pcp1).
            setCfi(cfi1).
            setVid(vid1).
            setEtherType(etype1);
        vlan.setPayload(null);

        assertEquals(cfi, copy.getCfi());
        assertEquals(pcp, copy.getPcp());
        assertEquals(vid, copy.getVid());
        assertEquals(etype, copy.getEtherType());

        assertEquals(cfi1, vlan.getCfi());
        assertEquals(pcp1, vlan.getPcp());
        assertEquals(vid1, vlan.getVid());
        assertEquals(etype1, vlan.getEtherType());

        assertEquals(null, vlan.getPayload());
        assertEquals(payload, copy.getPayload());
    }
}
