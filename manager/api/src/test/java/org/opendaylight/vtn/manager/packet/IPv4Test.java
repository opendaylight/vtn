/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.packet;

import static org.opendaylight.vtn.manager.util.NumberUtils.getUnsigned;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.InetProtocols;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link IPv4}.
 */
public class IPv4Test extends TestBase {
    /**
     * Test case for {@link IPv4#getPayloadClass(byte)}.
     */
    @Test
    public void testGetPayloadClass() {
        for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
            Class<?> expected;
            if (i == 1) {
                expected = ICMP.class;
            } else if (i == 6) {
                expected = TCP.class;
            } else if (i == 17) {
                expected = UDP.class;
            } else {
                expected = null;
            }

            assertEquals(expected, IPv4.getPayloadClass((byte)i));
        }
    }

    /**
     * Test case for IP version.
     *
     * <ul>
     *   <li>{@link IPv4#getVersion()}</li>
     *   <li>{@link IPv4#setVersion(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetVersion() {
        IPv4 ip = new IPv4();
        assertEquals((byte)4, ip.getVersion());

        for (byte v = 0; v <= 15; v++) {
            assertSame(ip, ip.setVersion(v));
            assertEquals(v, ip.getVersion());
        }
    }

    /**
     * Test case for IP header length.
     *
     * <ul>
     *   <li>{@link IPv4#getHeaderLen()}</li>
     *   <li>{@link IPv4#setHeaderLength(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetHeaderLength() {
        IPv4 ip = new IPv4();
        assertEquals(20, ip.getHeaderLen());

        for (byte v = 0; v <= 15; v++) {
            assertSame(ip, ip.setHeaderLength(v));
            assertEquals(v * 4, ip.getHeaderLen());
        }
    }

    /**
     * Test case for IP differential services.
     *
     * <ul>
     *   <li>{@link IPv4#getDiffServ()}</li>
     *   <li>{@link IPv4#setDiffServ(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetDiffServ() {
        IPv4 ip = new IPv4();
        assertEquals((byte)0, ip.getDiffServ());

        for (byte v = 0; v <= 63; v++) {
            assertSame(ip, ip.setDiffServ(v));
            assertEquals(v, ip.getDiffServ());
        }
    }

    /**
     * Test case for IP ECN bits.
     *
     * <ul>
     *   <li>{@link IPv4#getECN()}</li>
     *   <li>{@link IPv4#setECN(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetECN() {
        IPv4 ip = new IPv4();
        assertEquals((byte)0, ip.getECN());

        for (byte v = 0; v <= 3; v++) {
            assertSame(ip, ip.setECN(v));
            assertEquals(v, ip.getECN());
        }
    }

    /**
     * Test case for the IP total length.
     *
     * <ul>
     *   <li>{@link IPv4#getTotalLength()}</li>
     *   <li>{@link IPv4#setTotalLength(short)}</li>
     * </ul>
     */
    @Test
    public void testGetTotalLength() {
        IPv4 ip = new IPv4();
        assertEquals((byte)0, ip.getTotalLength());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short v: values) {
            assertSame(ip, ip.setTotalLength(v));
            assertEquals(v, ip.getTotalLength());
        }
    }

    /**
     * Test case for the IP identification.
     *
     * <ul>
     *   <li>{@link IPv4#getIdentification()}</li>
     *   <li>{@link IPv4#setIdentification(short)}</li>
     * </ul>
     */
    @Test
    public void testGetIdentification() {
        IPv4 ip = new IPv4();
        assertEquals((byte)0, ip.getIdentification());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short v: values) {
            assertSame(ip, ip.setIdentification(v));
            assertEquals(v, ip.getIdentification());
        }
    }

    /**
     * Test case for the IPv4 flags.
     *
     * <ul>
     *   <li>{@link IPv4#getFlags()}</li>
     *   <li>{@link IPv4#setFlags(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetFlags() {
        IPv4 ip = new IPv4();
        assertEquals((byte)2, ip.getFlags());

        byte[] values = {0, 1, 9, 57, 94, 127, -128, -127, -15, -2, -1};
        for (byte v: values) {
            assertSame(ip, ip.setFlags(v));
            assertEquals(v, ip.getFlags());
        }
    }

    /**
     * Test case for TTL.
     *
     * <ul>
     *   <li>{@link IPv4#getTtl()}</li>
     *   <li>{@link IPv4#setTtl(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetTtl() {
        IPv4 ip = new IPv4();
        assertEquals((byte)0, ip.getTtl());

        byte[] values = {0, 1, 9, 57, 94, 127, -128, -127, -15, -2, -1};
        for (byte v: values) {
            assertSame(ip, ip.setTtl(v));
            assertEquals(v, ip.getTtl());
        }
    }

    /**
     * Test case for the IP protocol number.
     *
     * <ul>
     *   <li>{@link IPv4#getProtocol()}</li>
     *   <li>{@link IPv4#setProtocol(byte)}</li>
     * </ul>
     */
    @Test
    public void testGetProtocol() {
        IPv4 ip = new IPv4();
        assertEquals((byte)0, ip.getProtocol());

        byte[] values = {0, 1, 9, 57, 94, 127, -128, -127, -15, -2, -1};
        for (byte v: values) {
            assertSame(ip, ip.setProtocol(v));
            assertEquals(v, ip.getProtocol());
        }
    }

    /**
     * Test case for the IPv4 checksum.
     *
     * <ul>
     *   <li>{@link IPv4#getChecksum()}</li>
     *   <li>{@link IPv4#setChecksum(short)}</li>
     * </ul>
     */
    @Test
    public void testGetChecksum() {
        IPv4 ip = new IPv4();
        assertEquals((byte)0, ip.getChecksum());

        short[] values = {1, 4, 99, 2054, 30000, 32767, -32768, -2, -1};
        for (short v: values) {
            assertSame(ip, ip.setChecksum(v));
            assertEquals(v, ip.getChecksum());
        }
    }

    /**
     * Test case for the IP fragmentation offset.
     *
     * <ul>
     *   <li>{@link IPv4#getFragmentOffset()}</li>
     *   <li>{@link IPv4#setFragmentOffset(short)}</li>
     * </ul>
     */
    @Test
    public void testGetFragmentOffset() {
        IPv4 ip = new IPv4();
        assertEquals((byte)0, ip.getFragmentOffset());

        short[] values = {0, 1, 19, 456, 719, 1024, 2491, 6182, 8190, 8191};
        for (short v: values) {
            assertSame(ip, ip.setFragmentOffset(v));
            assertEquals(v, ip.getFragmentOffset());
        }
    }

    /**
     * Test case for the source IP address.
     *
     * <ul>
     *   <li>{@link IPv4#getSourceAddress()}</li>
     *   <li>{@link IPv4#setSourceAddress(Ip4Network)}</li>
     * </ul>
     */
    @Test
    public void testGetSourceAddress() {
        IPv4 ip = new IPv4();
        assertEquals(new Ip4Network(0), ip.getSourceAddress());

        Ip4Network[] addrs = {
            new Ip4Network("11.22.33.44"),
            new Ip4Network("127.0.0.1"),
            new Ip4Network("192.168.45.254"),
            new Ip4Network("200.35.87.176"),
        };
        for (Ip4Network addr: addrs) {
            assertSame(ip, ip.setSourceAddress(addr));
            assertEquals(addr, ip.getSourceAddress());
        }
    }

    /**
     * Test case for the destination IP address.
     *
     * <ul>
     *   <li>{@link IPv4#getDestinationAddress()}</li>
     *   <li>{@link IPv4#setDestinationAddress(Ip4Network)}</li>
     * </ul>
     */
    @Test
    public void testGetDestinationAddress() {
        IPv4 ip = new IPv4();
        assertEquals(new Ip4Network(0), ip.getDestinationAddress());

        Ip4Network[] addrs = {
            new Ip4Network("11.22.33.44"),
            new Ip4Network("127.0.0.1"),
            new Ip4Network("192.168.45.254"),
            new Ip4Network("200.35.87.176"),
        };
        for (Ip4Network addr: addrs) {
            assertSame(ip, ip.setDestinationAddress(addr));
            assertEquals(addr, ip.getDestinationAddress());
        }
    }


    /**
     * Test case for the IPv4 options.
     *
     * <ul>
     *   <li>{@link IPv4#getOptions()}</li>
     *   <li>{@link IPv4#setOptions(byte[])}</li>
     * </ul>
     */
    @Test
    public void testOptions() throws Exception {
        IPv4 ip = new IPv4();
        assertEquals(20, ip.getHeaderLen());
        assertEquals(160, ip.getHeaderSize());
        assertEquals(0, ip.getFieldNumBits("Options"));

        byte[][] options = {
            new byte[] {
                (byte)0x01,
            },
            new byte[] {
                (byte)0x01, (byte)0x02,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
            },
            null,
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
                (byte)0x05,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
                (byte)0x05, (byte)0x06,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
                (byte)0x05, (byte)0x06, (byte)0x07,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
                (byte)0x05, (byte)0x06, (byte)0x07, (byte)0x08,
            },
            new byte[0],
        };

        byte[][] expected = {
            new byte[] {
                (byte)0x01, (byte)0x00, (byte)0x00, (byte)0x00,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x00, (byte)0x00,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x00,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
            },
            null,
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
                (byte)0x05, (byte)0x00, (byte)0x00, (byte)0x00,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
                (byte)0x05, (byte)0x06, (byte)0x00, (byte)0x00,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
                (byte)0x05, (byte)0x06, (byte)0x07, (byte)0x00,
            },
            new byte[] {
                (byte)0x01, (byte)0x02, (byte)0x03, (byte)0x04,
                (byte)0x05, (byte)0x06, (byte)0x07, (byte)0x08,
            },
            null,
        };

        byte[] echo = {
            (byte)0x11, (byte)0x22, (byte)0x33, (byte)0x44,
            (byte)0x55, (byte)0x66, (byte)0x77, (byte)0x88,
            (byte)0x99, (byte)0xaa,
        };
        ICMP icmp = new ICMP();
        icmp.setType((byte)8);
        icmp.setCode((byte)0);
        icmp.setIdentifier((short)0xabcd);
        icmp.setSequenceNumber((short)7777);
        icmp.setRawPayload(echo);

        ip.setSourceAddress(new Ip4Network("192.168.10.20"));
        ip.setDestinationAddress(new Ip4Network("192.168.30.40"));
        ip.setProtocol(InetProtocols.ICMP.byteValue());

        for (int i = 0; i < options.length; i++) {
            byte[] opts = options[i];
            byte[] exp = expected[i];

            // Set IPv4 options.
            int hlen = 20;
            int optlen;
            if (exp != null) {
                optlen = exp.length;
                hlen += optlen;
            } else {
                optlen = 0;
            }
            ip.setOptions(opts);
            assertArrayEquals(exp, ip.getOptions());
            assertEquals(hlen, ip.getHeaderLen());
            assertEquals(hlen * 8, ip.getHeaderSize());
            assertEquals(optlen * 8, ip.getFieldNumBits("Options"));

            // Serialize/Deserialize test.
            ip.setPayload(icmp);

            byte[] raw = ip.serialize();
            IPv4 newip = new IPv4();
            newip.deserialize(raw, 0, raw.length * 8);
            assertEquals(ip, newip);
            assertEquals(icmp, newip.getPayload());
            assertArrayEquals(exp, newip.getOptions());
        }
    }

    /**
     * Test case for the IPv4 checksum computation.
     */
    @Test
    public void testChecksum() {
        byte[] header = {
            (byte)0x45, 00, 00, (byte)0x3c, (byte)0x1c,
            (byte)0x46, (byte)0x40, 00, (byte)0x40, 06, (byte)0xb1,
            (byte)0xe6, (byte)0xac, (byte)0x10, (byte)0x0a,
            (byte)0x63, (byte)0xac, (byte)0x10, (byte)0x0a, (byte)0x0c
        };
        byte[] header2 = {
            (byte)0x45, 00, 00, (byte)0x73, 00, 00,
            (byte)0x40, 00, (byte)0x40, (byte)0x11, (byte)0xb8,
            (byte)0x61, (byte)0xc0, (byte)0xa8, 00, 01, (byte)0xc0,
            (byte)0xa8, 00, (byte)0xc7
        };
        byte[] header3 = {
            (byte)0x45, 00, 00, (byte)0x47, (byte)0x73,
            (byte)0x88, (byte)0x40, 00, (byte)0x40, 06, (byte)0xA2,
            (byte)0xC4, (byte)0x83, (byte)0x9F, (byte)0x0E,
            (byte)0x85, (byte)0x83, (byte)0x9F, (byte)0x0E, (byte)0xA1
        };
        byte[] header4 = {
            (byte)0x45, 00, 00, (byte)0x54, 00, 00,
            (byte)0x40, 00, (byte)0x40, 01, (byte)0xf0, (byte)0x8e,
            (byte)0xc0, (byte)0xa8, (byte)0x64, (byte)0x65,
            (byte)0xc0, (byte)0xa8, (byte)0x64, (byte)0x64
        };
        byte[] header5 = {
            (byte)0x45, 00, 00, (byte)0x54, 00, 00,
            (byte)0x40, 00, (byte)0x40, 01, (byte)0xef, (byte)0x8d,
            (byte)0xc0, (byte)0xa8, (byte)0x64, (byte)0x65,
            (byte)0xc0, (byte)0xa8, (byte)0x65, (byte)0x65
        };
        byte[] header6 = {
            (byte)0x45, 00, 00, (byte)0x54, 00, 00,
            (byte)0x40, 00, (byte)0x40, 01, (byte)0x0b, (byte)0x92,
            (byte)0xc0, (byte)0xa8, (byte)0x64, (byte)0x65, (byte)0x9,
            (byte)0x9, (byte)0x1, (byte)0x1
        };
        byte[] header7 = {
            (byte)0x45, 00, 00, (byte)0x54, 00, 00,
            (byte)0x40, 00, (byte)0x40, 01, (byte)0, (byte)0,
            (byte)0xc0, (byte)0xa8, (byte)0x64, (byte)0x65, (byte)0x9,
            (byte)0x9, (byte)0x2, (byte)0x2
        };

        IPv4 ip = new IPv4();
        assertEquals(0xb1e6, getUnsigned(ip.computeChecksum(header, 0)));
        assertEquals(0xb861, getUnsigned(ip.computeChecksum(header2, 0)));
        assertEquals(0xa2c4, getUnsigned(ip.computeChecksum(header3, 0)));
        assertEquals(0xf08e, getUnsigned(ip.computeChecksum(header4, 0)));
        assertEquals(0xef8d, getUnsigned(ip.computeChecksum(header5, 0)));
        assertEquals(0x0b92, getUnsigned(ip.computeChecksum(header6, 0)));
        assertEquals(0x0a91, getUnsigned(ip.computeChecksum(header7, 0)));
    }

    /**
     * Test case for serialization.
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

        ICMP icmp = new ICMP();
        icmp.setType((byte)8);
        icmp.setCode((byte)0);
        icmp.setIdentifier((short)0x46f5);
        icmp.setSequenceNumber((short)2);
        icmp.setRawPayload(icmpRawPayload);

        IPv4 ip = new IPv4();
        ip.setVersion((byte)4);
        ip.setIdentification((short)5);
        ip.setDiffServ((byte)0);
        ip.setECN((byte)0);
        ip.setTotalLength((short)84);
        ip.setFlags((byte)2);
        ip.setFragmentOffset((short)0);
        ip.setTtl((byte)64);
        ip.setProtocol(InetProtocols.ICMP.byteValue());
        ip.setDestinationAddress(new Ip4Network("192.168.100.100"));
        ip.setSourceAddress(new Ip4Network("192.168.100.101"));
        ip.setPayload(icmp);

        Ethernet eth = new Ethernet();
        byte[] dstMac = {
            (byte)0x98, (byte)0xfc, (byte)0x11, (byte)0x93,
            (byte)0x5c, (byte)0xb8,
        };
        byte[] srcMac = {
            (byte)0x00, (byte)0x24, (byte)0xd7, (byte)0xa9,
            (byte)0xa3, (byte)0x50,
        };
        eth.setDestinationMACAddress(dstMac);
        eth.setSourceMACAddress(srcMac);
        eth.setEtherType(EtherTypes.IPV4.shortValue());
        eth.setPayload(ip);

        byte[] stream = eth.serialize();
        Ethernet decEth = new Ethernet();
        decEth.deserialize(stream, 0, stream.length * Byte.SIZE);

        IPv4 decIp = (IPv4)decEth.getPayload();
        assertFalse(decIp.isCorrupted());
        assertTrue(ip.equals(decIp));

        ICMP decIcmp = (ICMP)decIp.getPayload();
        assertFalse(decIcmp.isCorrupted());
        assertArrayEquals(icmpRawPayload, decIcmp.getRawPayload());
    }

    /**
     * Test case for IPv4 header fragmentation.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFragment()throws Exception {
        byte[] payload1 = new byte[0];
        byte[] payload2 = {
            (byte)0x61, (byte)0xd1, (byte)0x3d, (byte)0x51,
            (byte)0x1b, (byte)0x75, (byte)0xa7, (byte)0x83,
        };
        byte[] payload3 = {
            (byte)0xe7, (byte)0x0f, (byte)0x2d, (byte)0x7e,
            (byte)0x15, (byte)0xba, (byte)0xe7, (byte)0x6d,
            (byte)0xb5, (byte)0xc5, (byte)0xb5, (byte)0x37,
            (byte)0x59, (byte)0xbc, (byte)0x91, (byte)0x43,
            (byte)0xb5, (byte)0xb7, (byte)0xe4, (byte)0x28,
            (byte)0xec, (byte)0x62, (byte)0x6b, (byte)0x6a,
            (byte)0xd1, (byte)0xcb, (byte)0x79, (byte)0x1e,
            (byte)0xfc, (byte)0x82, (byte)0xf5, (byte)0xb4,
        };

        // Ensure that the payload is not deserialized if the fragment offset
        // is not zero.
        byte proto = InetProtocols.TCP.byteValue();
        fragmentTest(payload1, proto, (short)0xf250);
        fragmentTest(payload2, proto, (short)0xf248);
        fragmentTest(payload3, proto, (short)0xf230);

        proto = InetProtocols.UDP.byteValue();
        fragmentTest(payload1, proto, (short)0xf245);
        fragmentTest(payload2, proto, (short)0xf23d);
        fragmentTest(payload3, proto, (short)0xf225);

        proto = InetProtocols.ICMP.byteValue();
        fragmentTest(payload1, proto, (short)0xf255);
        fragmentTest(payload2, proto, (short)0xf24d);
        fragmentTest(payload3, proto, (short)0xf235);

        // Ensure that the protocol header in the first fragment is
        // deserialized.
        proto = InetProtocols.TCP.byteValue();
        TCP tcp = new TCP();
        tcp.setSourcePort((short)1234).setDestinationPort((short)32000).
            setSequenceNumber((int)0xd541f5f8).setAckNumber((int)0x58da787d).
            setDataOffset((byte)5).setReserved((byte)0).
            setHeaderLenFlags((short)0x18).setWindowSize((short)0x40e8).
            setUrgentPointer((short)0x15f7).setChecksum((short)0x0d4e);
        firstFragmentTest(tcp, payload1, proto, (short)0xdfe6);
        tcp.setChecksum((short)0xab2a);
        firstFragmentTest(tcp, payload2, proto, (short)0xdfde);
        tcp.setChecksum((short)0x1c75);
        firstFragmentTest(tcp, payload3, proto, (short)0xdfc6);

        proto = InetProtocols.UDP.byteValue();
        UDP udp = new UDP();
        udp.setSourcePort((short)53).setDestinationPort((short)45383).
            setLength((short)(payload1.length + 8)).setChecksum((short)0);
        firstFragmentTest(udp, payload1, proto, (short)0xdfe7);
        udp.setLength((short)(payload2.length + 8));
        firstFragmentTest(udp, payload2, proto, (short)0xdfdf);
        udp.setLength((short)(payload3.length + 8));
        firstFragmentTest(udp, payload3, proto, (short)0xdfc7);

        proto = InetProtocols.ICMP.byteValue();
        ICMP icmp = new ICMP();
        icmp.setType((byte)8).setCode((byte)0).setIdentifier((short)0x3d1e).
            setSequenceNumber((short)1);
        firstFragmentTest(icmp, payload1, proto, (short)0xdff7);
        firstFragmentTest(icmp, payload2, proto, (short)0xdfef);
        firstFragmentTest(icmp, payload3, proto, (short)0xdfd7);
    }

    /**
     * Test case for {@link IPv4#clone()}.
     */
    @Test
    public void testClone() {
        byte dserv = 34;
        byte ecn = 2;
        short id = 4981;
        byte flag = 5;
        byte ttl = 116;
        byte proto = InetProtocols.UDP.byteValue();
        short fragoff = 45;
        short cksum = 12345;
        Ip4Network src = new Ip4Network("10.20.30.40");
        Ip4Network dst = new Ip4Network("192.168.100.200");
        IPv4 ip = new IPv4().
            setDiffServ(dserv).
            setECN(ecn).
            setIdentification(id).
            setFlags(flag).
            setTtl(ttl).
            setProtocol(proto).
            setFragmentOffset(fragoff).
            setChecksum(cksum).
            setSourceAddress(src).
            setDestinationAddress(dst);

        UDP udp = new UDP().
            setSourcePort((short)451).
            setDestinationPort((short)17).
            setLength((short)567).
            setChecksum((short)0);

        byte[] raw = {
            (byte)0x8b, (byte)0xc5, (byte)0x0b, (byte)0x3a,
            (byte)0xc2, (byte)0x3d, (byte)0xb2, (byte)0x65,
        };
        udp.setRawPayload(raw);
        ip.setPayload(udp);

        IPv4 copy = ip.clone();
        assertNotSame(ip, copy);
        assertEquals(ip, copy);
        assertEquals(ip.hashCode(), copy.hashCode());
        assertEquals(null, copy.getRawPayload());

        Packet payload = copy.getPayload();
        assertNotSame(udp, payload);
        assertEquals(udp, payload);
        assertArrayEquals(raw, payload.getRawPayload());

        // Modifying the source packet should never affect a deep copy.
        byte dserv1 = 3;
        byte ecn1 = 0;
        short id1 = -31984;
        byte flag1 = 0;
        byte ttl1 = 12;
        short fragoff1 = 0;
        short cksum1 = -32315;
        Ip4Network src1 = new Ip4Network("11.22.33.44");
        Ip4Network dst1 = new Ip4Network("192.168.100.253");
        ip.setDiffServ(dserv1).
            setECN(ecn1).
            setIdentification(id1).
            setFlags(flag1).
            setTtl(ttl1).
            setFragmentOffset(fragoff1).
            setChecksum(cksum1).
            setSourceAddress(src1).
            setDestinationAddress(dst1);

        UDP udp1 = new UDP().
            setSourcePort((short)64120).
            setDestinationPort((short)45).
            setLength((short)333).
            setChecksum((short)12345);
        ip.setPayload(udp1);

        assertEquals(dserv, copy.getDiffServ());
        assertEquals(ecn, copy.getECN());
        assertEquals(id, copy.getIdentification());
        assertEquals(flag, copy.getFlags());
        assertEquals(ttl, copy.getTtl());
        assertEquals(fragoff, copy.getFragmentOffset());
        assertEquals(cksum, copy.getChecksum());
        assertEquals(src, copy.getSourceAddress());
        assertEquals(dst, copy.getDestinationAddress());

        assertEquals(dserv1, ip.getDiffServ());
        assertEquals(ecn1, ip.getECN());
        assertEquals(id1, ip.getIdentification());
        assertEquals(flag1, ip.getFlags());
        assertEquals(ttl1, ip.getTtl());
        assertEquals(fragoff1, ip.getFragmentOffset());
        assertEquals(cksum1, ip.getChecksum());
        assertEquals(src1, ip.getSourceAddress());
        assertEquals(dst1, ip.getDestinationAddress());

        assertEquals(udp, copy.getPayload());
        assertArrayEquals(raw, copy.getPayload().getRawPayload());
        assertEquals(udp1, ip.getPayload());
        assertEquals(null, ip.getPayload().getRawPayload());
    }

    /**
     * Run the IPv4 fragmentation test.
     *
     * @param payload   The payload of the IPv4 packet.
     * @param proto     The IP protocol number.
     * @param checksum  The expected IPv4 checksum value.
     * @throws Exception  An error occurred.
     */
    private void fragmentTest(byte[] payload, byte proto, short checksum)
        throws Exception {
        // Construct a fragmented raw IPv4 packet.
        int ipv4Len = 20;
        byte[] rawIp = new byte[ipv4Len + payload.length];

        byte ipVersion = 4;
        byte dscp = 35;
        byte ecn = 2;
        byte tos = (byte)((dscp << 2) | ecn);
        short totalLen = (short)rawIp.length;
        short id = 22143;
        short offset = 0xb9;
        byte ttl = 64;
        byte[] srcIp = {(byte)0x0a, (byte)0x00, (byte)0x00, (byte)0x01};
        byte[] dstIp = {(byte)0xc0, (byte)0xa9, (byte)0x66, (byte)0x23};

        rawIp[0] = (byte)((ipVersion << 4) | (ipv4Len >> 2));
        rawIp[1] = tos;
        rawIp[2] = (byte)(totalLen >>> Byte.SIZE);
        rawIp[3] = (byte)totalLen;
        rawIp[4] = (byte)(id >>> Byte.SIZE);
        rawIp[5] = (byte)id;
        rawIp[6] = (byte)(offset >>> Byte.SIZE);
        rawIp[7] = (byte)offset;
        rawIp[8] = ttl;
        rawIp[9] = proto;
        rawIp[10] = (byte)(checksum >>> Byte.SIZE);
        rawIp[11] = (byte)checksum;
        System.arraycopy(srcIp, 0, rawIp, 12, srcIp.length);
        System.arraycopy(dstIp, 0, rawIp, 16, srcIp.length);
        System.arraycopy(payload, 0, rawIp, ipv4Len, payload.length);

        // Deserialize.
        IPv4 ipv4 = new IPv4();
        ipv4.deserialize(rawIp, 0, rawIp.length * Byte.SIZE);

        assertEquals(ipVersion, ipv4.getVersion());
        assertEquals(ipv4Len, ipv4.getHeaderLen());
        assertEquals(dscp, ipv4.getDiffServ());
        assertEquals(ecn, ipv4.getECN());
        assertEquals(totalLen, ipv4.getTotalLength());
        assertEquals(id, ipv4.getIdentification());
        assertEquals((byte)0, ipv4.getFlags());
        assertEquals(offset, ipv4.getFragmentOffset());
        assertEquals(ttl, ipv4.getTtl());
        assertEquals(proto, ipv4.getProtocol());
        assertEquals(checksum, ipv4.getChecksum());
        assertArrayEquals(srcIp, ipv4.getSourceAddress().getBytes());
        assertArrayEquals(dstIp, ipv4.getDestinationAddress().getBytes());
        assertEquals(false, ipv4.isCorrupted());

        // payloadClass should not be set if fragment offset is not zero.
        assertEquals(null, ipv4.getPayload());
        checkRawPayload(payload, ipv4.getRawPayload());

        // Ensure that data corruption can be detected.
        rawIp[1] = (byte)~rawIp[1];
        ipv4 = new IPv4();
        ipv4.deserialize(rawIp, 0, rawIp.length * Byte.SIZE);
        assertEquals(true, ipv4.isCorrupted());
    }

    /**
     * Ensure that the protocol header in the first fragment is deserialized.
     *
     * @param payload     The payload of the IPv4 packet.
     * @param rawPayload  The raw payload of the IPv4 packet.
     * @param proto       The IP protocol number.
     * @param checksum    The expected IPv4 checksum value.
     * @throws Exception  An error occurred.
     */
    private void firstFragmentTest(Packet payload, byte[] rawPayload,
                                   byte proto, short checksum)
        throws Exception {
        // Construct a raw IPv4 packet with MF flag.
        int ipv4Len = 20;
        payload.setRawPayload(rawPayload);
        byte[] payloadBytes = payload.serialize();
        byte[] rawIp = new byte[ipv4Len + payloadBytes.length];

        byte ipVersion = 4;
        byte dscp = 13;
        byte ecn = 1;
        byte tos = (byte)((dscp << 2) | ecn);
        short totalLen = (short)rawIp.length;
        short id = 19834;
        byte flags = 0x1;
        short offset = 0;
        short off = (short)(((short)flags << 13) | offset);
        byte ttl = 64;
        byte[] srcIp = {(byte)0xac, (byte)0x23, (byte)0x5b, (byte)0xfd};
        byte[] dstIp = {(byte)0xc0, (byte)0xa8, (byte)0x64, (byte)0x71};

        rawIp[0] = (byte)((ipVersion << 4) | (ipv4Len >> 2));
        rawIp[1] = tos;
        rawIp[2] = (byte)(totalLen >>> Byte.SIZE);
        rawIp[3] = (byte)totalLen;
        rawIp[4] = (byte)(id >>> Byte.SIZE);
        rawIp[5] = (byte)id;
        rawIp[6] = (byte)(off >>> Byte.SIZE);
        rawIp[7] = (byte)off;
        rawIp[8] = ttl;
        rawIp[9] = proto;
        rawIp[10] = (byte)(checksum >>> Byte.SIZE);
        rawIp[11] = (byte)checksum;
        System.arraycopy(srcIp, 0, rawIp, 12, srcIp.length);
        System.arraycopy(dstIp, 0, rawIp, 16, srcIp.length);
        System.arraycopy(payloadBytes, 0, rawIp, ipv4Len, payloadBytes.length);

        // Deserialize.
        IPv4 ipv4 = new IPv4();
        ipv4.deserialize(rawIp, 0, rawIp.length * Byte.SIZE);

        assertEquals(ipVersion, ipv4.getVersion());
        assertEquals(ipv4Len, ipv4.getHeaderLen());
        assertEquals(dscp, ipv4.getDiffServ());
        assertEquals(ecn, ipv4.getECN());
        assertEquals(totalLen, ipv4.getTotalLength());
        assertEquals(id, ipv4.getIdentification());
        assertEquals(flags, ipv4.getFlags());
        assertEquals(offset, ipv4.getFragmentOffset());
        assertEquals(ttl, ipv4.getTtl());
        assertEquals(proto, ipv4.getProtocol());
        assertEquals(checksum, ipv4.getChecksum());
        assertArrayEquals(srcIp, ipv4.getSourceAddress().getBytes());
        assertArrayEquals(dstIp, ipv4.getDestinationAddress().getBytes());
        assertEquals(false, ipv4.isCorrupted());

        // Protocol header in the first fragment should be deserialized.
        assertEquals(null, ipv4.getRawPayload());

        Packet desPayload = ipv4.getPayload();
        assertEquals(payload, desPayload);
        assertEquals(false, desPayload.isCorrupted());
        checkRawPayload(rawPayload, desPayload.getRawPayload());
    }

    /**
     * Compare the raw payload.
     *
     * @param expected  The expected raw payload.
     * @param payload   The raw payload to be tested.
     */
    private void checkRawPayload(byte[] expected, byte[] payload) {
        if (expected == null || expected.length == 0) {
            assertEquals(null, payload);
        } else {
            assertArrayEquals(expected, payload);
        }
    }
}
