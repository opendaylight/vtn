/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link UDP}.
 */
public class UDPTest extends TestBase {
    /**
     * Test case for UDP source port.
     *
     * <ul>
     *   <li>{@link UDP#getSourcePort()}</li>
     *   <li>{@link UDP#setSourcePort(short)}</li>
     * </ul>
     */
    @Test
    public void testGetSourcePort() {
        UDP udp = new UDP();
        assertEquals((short)0, udp.getSourcePort());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short port: values) {
            assertSame(udp, udp.setSourcePort(port));
            assertEquals(port, udp.getSourcePort());
        }
    }

    /**
     * Test case for UDP destination port.
     *
     * <ul>
     *   <li>{@link UDP#getDestinationPort()}</li>
     *   <li>{@link UDP#setDestinationPort(short)}</li>
     * </ul>
     */
    @Test
    public void testGetDestinationPort() {
        UDP udp = new UDP();
        assertEquals((short)0, udp.getDestinationPort());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short port: values) {
            assertSame(udp, udp.setDestinationPort(port));
            assertEquals(port, udp.getDestinationPort());
        }
    }

    /**
     * Test case for UDP datagram length.
     *
     * <ul>
     *   <li>{@link UDP#getLength()}</li>
     *   <li>{@link UDP#setLength(short)}</li>
     * </ul>
     */
    @Test
    public void testGetLength() {
        UDP udp = new UDP();
        assertEquals((short)0, udp.getLength());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short len: values) {
            assertSame(udp, udp.setLength(len));
            assertEquals(len, udp.getLength());
        }
    }

    /**
     * Test case for UDP checksum.
     *
     * <ul>
     *   <li>{@link UDP#getChecksum()}</li>
     *   <li>{@link UDP#setChecksum(short)}</li>
     * </ul>
     */
    @Test
    public void testGetChecksum() {
        UDP udp = new UDP();
        assertEquals((short)0, udp.getLength());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short cksum: values) {
            assertSame(udp, udp.setChecksum(cksum));
            assertEquals(cksum, udp.getChecksum());
        }
    }

    /**
     * Test case for {@link UDP#clone()}.
     */
    @Test
    public void testClone() {
        short src = 32144;
        short dst = 53;
        short len = 3128;
        short cksum = -28714;
        byte[] payload = {
            (byte)0x4e, (byte)0x99, (byte)0xe2, (byte)0x52,
            (byte)0xc3, (byte)0x4f, (byte)0x3e, (byte)0xac,
        };

        UDP udp = new UDP().
            setSourcePort(src).
            setDestinationPort(dst).
            setLength(len).
            setChecksum(cksum);
        udp.setRawPayload(payload);

        UDP copy = udp.clone();
        assertNotSame(udp, copy);
        assertEquals(udp, copy);
        assertEquals(udp.hashCode(), copy.hashCode());
        assertArrayEquals(payload, copy.getRawPayload());

        // Modifying the source packet should never affect a deep copy.
        short src1 = 4567;
        short dst1 = 80;
        short len1 = 7811;
        short cksum1 = 31985;
        udp.setSourcePort(src1).
            setDestinationPort(dst1).
            setLength(len1).
            setChecksum(cksum1);
        udp.setRawPayload(null);

        assertEquals(src, copy.getSourcePort());
        assertEquals(dst, copy.getDestinationPort());
        assertEquals(len, copy.getLength());
        assertEquals(cksum, copy.getChecksum());
        assertArrayEquals(payload, copy.getRawPayload());

        assertEquals(src1, udp.getSourcePort());
        assertEquals(dst1, udp.getDestinationPort());
        assertEquals(len1, udp.getLength());
        assertEquals(cksum1, udp.getChecksum());
        assertEquals(null, udp.getRawPayload());
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
    public void testSerialization() throws Exception {
        byte[] raw = {
            // Source port.
            (byte)0x01, (byte)0x35,

            // Destination port.
            (byte)0x31, (byte)0x9d,

            // Length
            (byte)0x01, (byte)0x16,

            // Checksum
            (byte)0xd4, (byte)0x41,
        };

        short src = (short)0x0135;
        short dst = (short)0x319d;
        short len = (short)0x0116;
        short cksum = (short)0xd441;

        // Deserialize raw packet.
        UDP udp = new UDP();
        udp.deserialize(raw, 0, raw.length * Byte.SIZE);
        assertEquals(src, udp.getSourcePort());
        assertEquals(dst, udp.getDestinationPort());
        assertEquals(len, udp.getLength());
        assertEquals(cksum, udp.getChecksum());
        assertEquals(null, udp.getPayload());
        assertEquals(null, udp.getRawPayload());
        assertEquals(false, udp.isCorrupted());

        // Serialize packet.
        assertArrayEquals(raw, udp.serialize());

        // Deserialize packet with UDP payload.
        raw = new byte[]{
            // Source port.
            (byte)0x74, (byte)0x1d,

            // Destination port.
            (byte)0x00, (byte)0x31,

            // Length
            (byte)0x12, (byte)0x34,

            // Checksum
            (byte)0x91, (byte)0xdf,

            // Payload
            (byte)0x26, (byte)0xa6, (byte)0xdf, (byte)0x80,
            (byte)0x06, (byte)0xb5, (byte)0xd6, (byte)0xa0,
            (byte)0x66, (byte)0xf2, (byte)0x38, (byte)0x6e,
            (byte)0x36, (byte)0x19, (byte)0xa9,
        };

        src = (short)0x741d;
        dst = (short)0x0031;
        len = (short)0x1234;
        cksum = (short)0x91df;
        byte[] payload = {
            (byte)0x26, (byte)0xa6, (byte)0xdf, (byte)0x80,
            (byte)0x06, (byte)0xb5, (byte)0xd6, (byte)0xa0,
            (byte)0x66, (byte)0xf2, (byte)0x38, (byte)0x6e,
            (byte)0x36, (byte)0x19, (byte)0xa9,
        };

        // Deserialize raw packet.
        udp = new UDP();
        udp.deserialize(raw, 0, raw.length * Byte.SIZE);
        assertEquals(src, udp.getSourcePort());
        assertEquals(dst, udp.getDestinationPort());
        assertEquals(len, udp.getLength());
        assertEquals(cksum, udp.getChecksum());
        assertEquals(null, udp.getPayload());
        assertArrayEquals(payload, udp.getRawPayload());
        assertEquals(false, udp.isCorrupted());

        // Serialize packet.
        assertArrayEquals(raw, udp.serialize());

        // Serialize an empty packet.
        udp = new UDP();
        byte[] expected = new byte[8];
        assertArrayEquals(expected, udp.serialize());
    }
}
