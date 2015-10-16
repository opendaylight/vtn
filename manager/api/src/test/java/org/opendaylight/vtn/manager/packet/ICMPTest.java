/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import java.util.Arrays;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link ICMP}.
 */
public class ICMPTest extends TestBase {
    /**
     * Test case for ICMP type.
     *
     * <ul>
     *   <li>{@link ICMP#getType()}</li>
     *   <li>{@link ICMP#setType(byte)}</li>
     * </ul>
     */
    @Test
    public void testSetType() {
        ICMP icmp = new ICMP();
        assertEquals((byte)0, icmp.getType());

        byte[] values = {0, 1, 9, 57, 94, 127, -128, -127, -15, -2, -1};
        for (byte type: values) {
            assertSame(icmp, icmp.setType(type));
            assertEquals(type, icmp.getType());
        }
    }

    /**
     * Test case for ICMP code.
     *
     * <ul>
     *   <li>{@link ICMP#getCode()}</li>
     *   <li>{@link ICMP#setCode(byte)}</li>
     * </ul>
     */
    @Test
    public void testSetCode() {
        ICMP icmp = new ICMP();
        assertEquals((byte)0, icmp.getCode());

        byte[] values = {0, 1, 9, 57, 94, 127, -128, -127, -15, -2, -1};
        for (byte code: values) {
            assertSame(icmp, icmp.setCode(code));
            assertEquals(code, icmp.getCode());
        }
    }

    /**
     * Test case for ICMP checksum.
     *
     * <ul>
     *   <li>{@link ICMP#getChecksum()}</li>
     *   <li>{@link ICMP#setChecksum(short)}</li>
     * </ul>
     */
    @Test
    public void testSetChecksum() {
        ICMP icmp = new ICMP();
        assertEquals((short)0, icmp.getChecksum());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short cksum: values) {
            assertSame(icmp, icmp.setChecksum(cksum));
            assertEquals(cksum, icmp.getChecksum());
        }
    }

    /**
     * Test case for ICMP identifier.
     *
     * <ul>
     *   <li>{@link ICMP#getIdentifier()}</li>
     *   <li>{@link ICMP#setIdentifier(short)}</li>
     * </ul>
     */
    @Test
    public void testSetIdentifier() {
        ICMP icmp = new ICMP();
        assertEquals((short)0, icmp.getChecksum());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short id: values) {
            assertSame(icmp, icmp.setIdentifier(id));
            assertEquals(id, icmp.getIdentifier());
        }
    }

    /**
     * Test case for ICMP sequence number.
     *
     * <ul>
     *   <li>{@link ICMP#getSequenceNumber()}</li>
     *   <li>{@link ICMP#setSequenceNumber(short)}</li>
     * </ul>
     */
    @Test
    public void testSetSequenceNumber() {
        ICMP icmp = new ICMP();
        assertEquals((short)0, icmp.getSequenceNumber());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short seq: values) {
            assertSame(icmp, icmp.setSequenceNumber(seq));
            assertEquals(seq, icmp.getSequenceNumber());
        }
    }

    /**
     * Test case for {@link ICMP#clone()}.
     */
    @Test
    public void testClone() {
        byte type = 12;
        byte code = 45;
        short cksum = 18341;
        short id = -29234;
        short seq = 31721;
        ICMP icmp = new ICMP().
            setType(type).
            setCode(code).
            setChecksum(cksum).
            setIdentifier(id).
            setSequenceNumber(seq);

        ICMP copy = icmp.clone();
        assertNotSame(icmp, copy);
        assertEquals(icmp, copy);
        assertEquals(icmp.hashCode(), copy.hashCode());

        // Modifying the source packet should never affect a deep copy.
        byte type1 = 0;
        byte code1 = 23;
        short cksum1 = -29216;
        short id1 = 3445;
        short seq1 = 9163;
        icmp.setType(type1).
            setCode(code1).
            setChecksum(cksum1).
            setIdentifier(id1).
            setSequenceNumber(seq1);

        assertEquals(type, copy.getType());
        assertEquals(code, copy.getCode());
        assertEquals(cksum, copy.getChecksum());
        assertEquals(id, copy.getIdentifier());
        assertEquals(seq, copy.getSequenceNumber());

        assertEquals(type1, icmp.getType());
        assertEquals(code1, icmp.getCode());
        assertEquals(cksum1, icmp.getChecksum());
        assertEquals(id1, icmp.getIdentifier());
        assertEquals(seq1, icmp.getSequenceNumber());
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
        byte[] icmpRawPayload = {
            (byte)0x38, (byte)0x26, (byte)0x9e, (byte)0x51,
            (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x2e, (byte)0x6a, (byte)0x08, (byte)0x00,
            (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x10, (byte)0x11, (byte)0x12, (byte)0x13,
            (byte)0x14, (byte)0x15, (byte)0x16, (byte)0x17,
            (byte)0x18, (byte)0x19, (byte)0x1a, (byte)0x1b,
            (byte)0x1c, (byte)0x1d, (byte)0x1e, (byte)0x1f,
            (byte)0x20, (byte)0x21, (byte)0x22, (byte)0x23,
            (byte)0x24, (byte)0x25, (byte)0x26, (byte)0x27,
            (byte)0x28, (byte)0x29, (byte)0x2a, (byte)0x2b,
            (byte)0x2c, (byte)0x2d, (byte)0x2e, (byte)0x2f,
            (byte)0x30, (byte)0x31, (byte)0x32, (byte)0x33,
            (byte)0x34, (byte)0x35, (byte)0x36, (byte)0x37,
        };
        serializeTest(icmpRawPayload, (short)0xe553);

        serializeTest(null, (short)0xb108);
        serializeTest(new byte[0], (short)0xb108);

        byte[] odd = {
            (byte)0xba, (byte)0xd4, (byte)0xc7, (byte)0x53,
            (byte)0xf8, (byte)0x59, (byte)0x68, (byte)0x77,
            (byte)0xfd, (byte)0x27, (byte)0xe0, (byte)0x5b,
            (byte)0xd0, (byte)0x2e, (byte)0x28, (byte)0x41,
            (byte)0xa3, (byte)0x48, (byte)0x5d, (byte)0x2e,
            (byte)0x7d, (byte)0x5b, (byte)0xd3, (byte)0x60,
            (byte)0xb3, (byte)0x88, (byte)0x8d, (byte)0x0f,
            (byte)0x1d, (byte)0x87, (byte)0x51, (byte)0x0f,
            (byte)0x6a, (byte)0xff, (byte)0xf7, (byte)0xd4,
            (byte)0x40, (byte)0x35, (byte)0x4e, (byte)0x01,
            (byte)0x36,
        };
        serializeTest(odd, (short)0xd0ad);

        // Large payload that causes 16-bit checksum overflow more than
        // 255 times.
        byte[] largeEven = new byte[1024];
        Arrays.fill(largeEven, (byte)0xff);
        serializeTest(largeEven, (short)0xb108);

        byte[] largeOdd = new byte[1021];
        Arrays.fill(largeOdd, (byte)0xff);
        serializeTest(largeOdd, (short)0xb207);

        // Serialize an empty packet.
        ICMP icmp = new ICMP();
        byte[] expected = new byte[8];

        // Checksum should be updated.
        expected[2] = (byte)0xff;
        expected[3] = (byte)0xff;
        assertArrayEquals(expected, icmp.serialize());
    }

    /**
     * Ensure that the ICMP packet is serializable and deserializable.
     *
     * @param payload   The raw packet of the ICMP packet.
     * @param checksum  The expected ICMP checksum.
     * @throws Exception  An error occurred.
     */
    private void serializeTest(byte[] payload, short checksum)
        throws Exception {
        ICMP icmp = new ICMP();
        icmp.setType((byte)8).setCode((byte)0).
            setIdentifier((short)0x46f5).setSequenceNumber((short)2);
        int payloadSize = 0;
        if (payload != null) {
            icmp.setRawPayload(payload);
            payloadSize = payload.length;
        }

        // Serialize
        byte[] data = icmp.serialize();
        assertEquals(payloadSize + 8, data.length);

        // Deserialize
        ICMP icmpDes = new ICMP();
        icmpDes.deserialize(data, 0, data.length);

        assertFalse(icmpDes.isCorrupted());
        assertEquals(checksum, icmpDes.getChecksum());
        assertEquals(icmp, icmpDes);
        assertEquals(false, icmpDes.isCorrupted());

        // Ensure that data corruption can be detected.
        data[0] = (byte)~data[0];
        icmpDes = new ICMP();
        icmpDes.deserialize(data, 0, data.length);
        assertEquals(true, icmpDes.isCorrupted());
    }
}
