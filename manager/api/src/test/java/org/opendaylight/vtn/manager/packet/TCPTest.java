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
 * JUnit test for {@link TCP}.
 */
public class TCPTest extends TestBase {
    /**
     * Test case for TCP source port.
     *
     * <ul>
     *   <li>{@link TCP#getSourcePort()}</li>
     *   <li>{@link TCP#setSourcePort(short)}</li>
     * </ul>
     */
    @Test
    public void testGetSourcePort() {
        TCP tcp = new TCP();
        assertEquals((short)0, tcp.getSourcePort());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short port: values) {
            assertSame(tcp, tcp.setSourcePort(port));
            assertEquals(port, tcp.getSourcePort());
        }
    }

    /**
     * Test case for TCP destination port.
     *
     * <ul>
     *   <li>{@link TCP#getDestinationPort()}</li>
     *   <li>{@link TCP#setDestinationPort(short)}</li>
     * </ul>
     */
    @Test
    public void testGetDestinationPort() {
        TCP tcp = new TCP();
        assertEquals((short)0, tcp.getDestinationPort());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short port: values) {
            assertSame(tcp, tcp.setDestinationPort(port));
            assertEquals(port, tcp.getDestinationPort());
        }
    }

    /**
     * Test case for TCP sequence number.
     *
     * <ul>
     *   <li>{@link TCP#getSequenceNumber()}</li>
     *   <li>{@link TCP#setSequenceNumber(int)}</li>
     * </ul>
     */
    @Test
    public void testGetSequenceNumber() {
        TCP tcp = new TCP();
        assertEquals((short)0, tcp.getSequenceNumber());

        int[] values = {
            1, 123, 4678, 0x12345678, 0x7fffffff,
            -1, -12934, -91287345, Integer.MIN_VALUE,
        };
        for (int seq: values) {
            assertSame(tcp, tcp.setSequenceNumber(seq));
            assertEquals(seq, tcp.getSequenceNumber());
        }
    }

    /**
     * Test case for TCP acknowledgement number.
     *
     * <ul>
     *   <li>{@link TCP#getAckNumber()}</li>
     *   <li>{@link TCP#setAckNumber(int)}</li>
     * </ul>
     */
    @Test
    public void testGetAckNumber() {
        TCP tcp = new TCP();
        assertEquals((short)0, tcp.getAckNumber());

        int[] values = {
            1, 123, 4678, 0x12345678, 0x7fffffff,
            -1, -12934, -91287345, Integer.MIN_VALUE,
        };
        for (int ack: values) {
            assertSame(tcp, tcp.setAckNumber(ack));
            assertEquals(ack, tcp.getAckNumber());
        }
    }

    /**
     * Test case for TCP data offset.
     *
     * <ul>
     *   <li>{@link TCP#getDataOffset()}</li>
     *   <li>{@link TCP#setDataOffset(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetDataOffset() {
        TCP tcp = new TCP();
        assertEquals((byte)0, tcp.getDataOffset());

        for (byte off = 0; off <= 15; off++) {
            assertSame(tcp, tcp.setDataOffset(off));
            assertEquals(off, tcp.getDataOffset());
        }
    }

    /**
     * Test case for reserved field.
     *
     * <ul>
     *   <li>{@link TCP#getReserved()}</li>
     *   <li>{@link TCP#setReserved(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetReserved() {
        TCP tcp = new TCP();
        assertEquals((byte)0, tcp.getReserved());

        for (byte resv = 0; resv <= 7; resv++) {
            assertSame(tcp, tcp.setReserved(resv));
            assertEquals(resv, tcp.getReserved());
        }
    }

    /**
     * Test case for TCP header length and TCP flags.
     *
     * <ul>
     *   <li>{@link TCP#getHeaderLenFlags()}</li>
     *   <li>{@link TCP#setHeaderLenFlags(short)}</li>
     * </ul>
     */
    @Test
    public void testHeaderLenFlags() {
        TCP tcp = new TCP();
        assertEquals((byte)0, tcp.getHeaderLenFlags());

        short[] values = {0, 1, 33, 123, 312, 456, 509, 510, 511};
        for (short flags: values) {
            assertSame(tcp, tcp.setHeaderLenFlags(flags));
            assertEquals(flags, tcp.getHeaderLenFlags());
        }
    }

    /**
     * Test case for TCP window size.
     *
     * <ul>
     *   <li>{@link TCP#getWindowSize()}</li>
     *   <li>{@link TCP#setWindowSize(short)}</li>
     * </ul>
     */
    @Test
    public void testGetWindowSize() {
        TCP tcp = new TCP();
        assertEquals((short)0, tcp.getWindowSize());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short wsize: values) {
            assertSame(tcp, tcp.setWindowSize(wsize));
            assertEquals(wsize, tcp.getWindowSize());
        }
    }

    /**
     * Test case for TCP checksum.
     *
     * <ul>
     *   <li>{@link TCP#getChecksum()}</li>
     *   <li>{@link TCP#setChecksum(short)}</li>
     * </ul>
     */
    @Test
    public void testGetChecksum() {
        TCP tcp = new TCP();
        assertEquals((short)0, tcp.getChecksum());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short cksum: values) {
            assertSame(tcp, tcp.setChecksum(cksum));
            assertEquals(cksum, tcp.getChecksum());
        }
    }

    /**
     * Test case for TCP urgent pointer.
     *
     * <ul>
     *   <li>{@link TCP#getUrgentPointer()}</li>
     *   <li>{@link TCP#setUrgentPointer(short)}</li>
     * </ul>
     */
    @Test
    public void testGetUrgentPointer() {
        TCP tcp = new TCP();
        assertEquals((short)0, tcp.getUrgentPointer());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short urg: values) {
            assertSame(tcp, tcp.setUrgentPointer(urg));
            assertEquals(urg, tcp.getUrgentPointer());
        }
    }

    /**
     * Test case for {@link TCP#clone()}.
     */
    @Test
    public void testClone() {
        short src = 16721;
        short dst = 999;
        int seq = 0x761231da;
        int ack = 0x7319efc1;
        byte off = 12;
        byte resv = 2;
        short flags = 503;
        short wsize = 31982;
        short cksum = -30981;
        short urg = 7788;
        TCP tcp = new TCP().
            setSourcePort(src).
            setDestinationPort(dst).
            setSequenceNumber(seq).
            setAckNumber(ack).
            setDataOffset(off).
            setReserved(resv).
            setHeaderLenFlags(flags).
            setWindowSize(wsize).
            setChecksum(cksum).
            setUrgentPointer(urg);

        TCP copy = tcp.clone();
        assertNotSame(tcp, copy);
        assertEquals(tcp, copy);
        assertEquals(tcp.hashCode(), copy.hashCode());
        assertEquals(null, copy.getRawPayload());

        // Modifying the source packet should never affect a deep copy.
        short src1 = 29871;
        short dst1 = 17;
        int seq1 = (int)0xabcdef00;
        int ack1 = (int)0x891d4fcb;
        byte off1 = 3;
        byte resv1 = 0;
        byte flags1 = 81;
        short wsize1 = 3600;
        short cksum1 = 19826;
        short urg1 = 4;
        tcp.setSourcePort(src1).
            setDestinationPort(dst1).
            setSequenceNumber(seq1).
            setAckNumber(ack1).
            setDataOffset(off1).
            setReserved(resv1).
            setHeaderLenFlags(flags1).
            setWindowSize(wsize1).
            setChecksum(cksum1).
            setUrgentPointer(urg1);
        byte[] payload = {
            (byte)0x9b, (byte)0x6d, (byte)0x30, (byte)0x0e,
        };

        assertEquals(src, copy.getSourcePort());
        assertEquals(dst, copy.getDestinationPort());
        assertEquals(seq, copy.getSequenceNumber());
        assertEquals(ack, copy.getAckNumber());
        assertEquals(off, copy.getDataOffset());
        assertEquals(resv, copy.getReserved());
        assertEquals(flags, copy.getHeaderLenFlags());
        assertEquals(wsize, copy.getWindowSize());
        assertEquals(cksum, copy.getChecksum());
        assertEquals(urg, copy.getUrgentPointer());

        assertEquals(src1, tcp.getSourcePort());
        assertEquals(dst1, tcp.getDestinationPort());
        assertEquals(seq1, tcp.getSequenceNumber());
        assertEquals(ack1, tcp.getAckNumber());
        assertEquals(off1, tcp.getDataOffset());
        assertEquals(resv1, tcp.getReserved());
        assertEquals(flags1, tcp.getHeaderLenFlags());
        assertEquals(wsize1, tcp.getWindowSize());
        assertEquals(cksum1, tcp.getChecksum());
        assertEquals(urg1, tcp.getUrgentPointer());
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
            (byte)0x41, (byte)0x9c,

            // Destination port.
            (byte)0x00, (byte)0x35,

            // Sequence number.
            (byte)0x41, (byte)0xdb, (byte)0x3a, (byte)0x91,

            // Acknowledgement number.
            (byte)0xb7, (byte)0x05, (byte)0xb1, (byte)0x9b,

            // Data offset, reserved, header length, and flags.
            (byte)0x9c, (byte)0xc1,

            // Window size.
            (byte)0x52, (byte)0xd2,

            // Checksum.
            (byte)0xd9, (byte)0x04,

            // Urgent pointer.
            (byte)0x68, (byte)0xde,
        };

        short src = (short)0x419c;
        short dst = (short)0x0035;
        int seq = (int)0x41db3a91;
        int ack = (int)0xb705b19b;
        byte off = 0x9;
        byte resv = 6;
        short flags = 193;
        short wsize = (short)0x52d2;
        short cksum = (short)0xd904;
        short urg = (short)0x68de;

        // Deserialize raw packet.
        TCP tcp = new TCP();
        tcp.deserialize(raw, 0, raw.length * Byte.SIZE);
        assertEquals(src, tcp.getSourcePort());
        assertEquals(dst, tcp.getDestinationPort());
        assertEquals(seq, tcp.getSequenceNumber());
        assertEquals(ack, tcp.getAckNumber());
        assertEquals(off, tcp.getDataOffset());
        assertEquals(resv, tcp.getReserved());
        assertEquals(flags, tcp.getHeaderLenFlags());
        assertEquals(wsize, tcp.getWindowSize());
        assertEquals(cksum, tcp.getChecksum());
        assertEquals(urg, tcp.getUrgentPointer());
        assertEquals(null, tcp.getPayload());
        assertEquals(null, tcp.getRawPayload());
        assertEquals(false, tcp.isCorrupted());

        // Serialize packet.
        assertArrayEquals(raw, tcp.serialize());

        // Deserialize packet with TCP payload.
        raw = new byte[]{
            // Source port.
            (byte)0xb0, (byte)0xd9,

            // Destination port.
            (byte)0x2e, (byte)0x8c,

            // Sequence number.
            (byte)0xe6, (byte)0x0d, (byte)0x2a, (byte)0xdb,

            // Acknowledgement number.
            (byte)0x5f, (byte)0x9c, (byte)0x76, (byte)0xf6,

            // Data offset, reserved, header length, and flags.
            (byte)0xc5, (byte)0x37,

            // Window size.
            (byte)0x49, (byte)0x6c,

            // Checksum.
            (byte)0x36, (byte)0xad,

            // Urgent pointer.
            (byte)0xd7, (byte)0xbd,

            // Payload.
            (byte)0x7d, (byte)0xc9, (byte)0x39, (byte)0xf2,
            (byte)0xf1, (byte)0x4f, (byte)0xc4, (byte)0x73,
            (byte)0x66,
        };

        src = (short)0xb0d9;
        dst = (short)0x2e8c;
        seq = (int)0xe60d2adb;
        ack = (int)0x5f9c76f6;
        off = 0xc;
        resv = 2;
        flags = 311;
        wsize = (short)0x496c;
        cksum = (short)0x36ad;
        urg = (short)0xd7bd;
        byte[] payload = {
            (byte)0x7d, (byte)0xc9, (byte)0x39, (byte)0xf2,
            (byte)0xf1, (byte)0x4f, (byte)0xc4, (byte)0x73,
            (byte)0x66,
        };

        tcp = new TCP();
        tcp.deserialize(raw, 0, raw.length * Byte.SIZE);
        assertEquals(src, tcp.getSourcePort());
        assertEquals(dst, tcp.getDestinationPort());
        assertEquals(seq, tcp.getSequenceNumber());
        assertEquals(ack, tcp.getAckNumber());
        assertEquals(off, tcp.getDataOffset());
        assertEquals(resv, tcp.getReserved());
        assertEquals(flags, tcp.getHeaderLenFlags());
        assertEquals(wsize, tcp.getWindowSize());
        assertEquals(cksum, tcp.getChecksum());
        assertEquals(urg, tcp.getUrgentPointer());
        assertEquals(null, tcp.getPayload());
        assertArrayEquals(payload, tcp.getRawPayload());
        assertEquals(false, tcp.isCorrupted());

        // Serialize packet.
        assertArrayEquals(raw, tcp.serialize());

        // Serialize an empty packet.
        tcp = new TCP();
        byte[] expected = new byte[20];
        assertArrayEquals(expected, tcp.serialize());
    }
}
