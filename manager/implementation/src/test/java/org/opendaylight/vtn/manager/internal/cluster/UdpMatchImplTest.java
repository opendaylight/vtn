/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.PortMatch;
import org.opendaylight.vtn.manager.flow.cond.UdpMatch;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.ICMP;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.UDP;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link UdpMatchImpl}.
 */
public class UdpMatchImplTest extends TestBase {
    /**
     * Test for {@link UdpMatchImpl#UdpMatchImpl(UdpMatch)},
     * {@link L4MatchImpl#create(org.opendaylight.vtn.manager.flow.cond.L4Match)},
     * and getter methods.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testConstructor() throws VTNException {
        int[] srcs = {
            -1, 0, 4, 100, 678, 1357, 33445, 65535,
        };
        int[] dsts = {
            -1, 0, 8, 193, 783, 2674, 45678, 65535,
        };

        int nranges = 3;
        UdpMatchImplBuilder builder = new UdpMatchImplBuilder();
        for (int src: srcs) {
            builder.setSourcePortFrom(src);

            int snr = (src == -1 || src == 65535) ? 0 : nranges;
            int[] srcRange = new int[snr + 2];
            srcRange[0] = -1;
            for (int i = 1; i < srcRange.length; i++) {
                srcRange[i] = src + i - 1;
            }
            for (int srcTo: srcRange) {
                builder.setSourcePortTo(srcTo);
                for (int dst: dsts) {
                    builder.setDestinationPortFrom(dst);

                    int dnr = (dst == -1 || dst == 65535) ? 0 : nranges;
                    int[] dstRange = new int[dnr + 2];
                    dstRange[0] = -1;
                    for (int i = 1; i < dstRange.length; i++) {
                        dstRange[i] = dst + i - 1;
                    }
                    for (int dstTo: dstRange) {
                        builder.setDestinationPortTo(dstTo);
                        builder.verify();
                    }
                }
            }
        }

        // Specifying empty PortMatch.
        PortMatch empty = new PortMatch((Integer)null);
        UdpMatch um = new UdpMatch(empty, null);
        try {
            new UdpMatchImpl(um);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("source: \"from\" is not specified.",
                         st.getDescription());
        }

        um = new UdpMatch(null, empty);
        try {
            new UdpMatchImpl(um);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("destination: \"from\" is not specified.",
                         st.getDescription());
        }

        // Specifying bad port number.
        Integer badPort = Integer.valueOf(-1);
        um = new UdpMatch(badPort, null);
        try {
            new UdpMatchImpl(um);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("source: from: Invalid port number: " + badPort,
                         st.getDescription());
        }

        um = new UdpMatch(null, badPort);
        try {
            new UdpMatchImpl(um);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("destination: from: Invalid port number: " + badPort,
                         st.getDescription());
        }

        // Specifying invalid port range.
        Integer badFrom = Integer.valueOf(3);
        Integer badTo = Integer.valueOf(2);
        PortMatch badRange = new PortMatch(badFrom, badTo);
        um = new UdpMatch(badRange, null);
        try {
            new UdpMatchImpl(um);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("source: Invalid port range: from=" + badFrom +
                         ", to=" + badTo, st.getDescription());
        }

        um = new UdpMatch(null, badRange);
        try {
            new UdpMatchImpl(um);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("destination: Invalid port range: from=" + badFrom +
                         ", to=" + badTo, st.getDescription());
        }
    }

    /**
     * Test case for {@link UdpMatchImpl#equals(Object)} and
     * {@link UdpMatchImpl#hashCode()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testEquals() throws VTNException {
        HashSet<Object> set = new HashSet<Object>();

        int[] srcs = {
            -1, 0, 4, 100, 678, 1357, 33445, 65535,
        };
        int[] dsts = {
            -1, 0, 8, 193, 783, 2674, 45678, 65535,
        };

        int count = 0;
        int nranges = 3;
        UdpMatchImplBuilder builder = new UdpMatchImplBuilder();
        for (int src: srcs) {
            builder.setSourcePortFrom(src);

            int snr = (src == -1 || src == 65535) ? 1 : nranges;
            int[] srcRange = new int[snr];
            srcRange[0] = -1;
            for (int i = 1; i < srcRange.length; i++) {
                srcRange[i] = src + i;
            }
            for (int srcTo: srcRange) {
                builder.setSourcePortTo(srcTo);
                for (int dst: dsts) {
                    builder.setDestinationPortFrom(dst);

                    int dnr = (dst == -1 || dst == 65535) ? 1 : nranges;
                    int[] dstRange = new int[dnr];
                    dstRange[0] = -1;
                    for (int i = 1; i < dstRange.length; i++) {
                        dstRange[i] = dst + i;
                    }
                    for (int dstTo: dstRange) {
                        builder.setDestinationPortTo(dstTo);
                        UdpMatchImpl umi1 = builder.create();
                        UdpMatchImpl umi2 = builder.create();
                        testEquals(set, umi1, umi2);
                        count++;
                    }
                }
            }
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link UdpMatchImpl#toString()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testToString() throws VTNException {
        String prefix = "UdpMatchImpl[";
        String suffix = "]";

        int[] srcs = {
            -1, 0, 4, 100, 678, 1357, 33445, 65535,
        };
        int[] dsts = {
            -1, 0, 8, 193, 783, 2674, 45678, 65535,
        };

        int nranges = 3;
        UdpMatchImplBuilder builder = new UdpMatchImplBuilder();
        for (int src: srcs) {
            builder.setSourcePortFrom(src);

            int snr = (src == -1 || src == 65535) ? 1 : nranges;
            int[] srcRange = new int[snr];
            srcRange[0] = -1;
            for (int i = 1; i < srcRange.length; i++) {
                srcRange[i] = src + i;
            }
            for (int srcTo: srcRange) {
                builder.setSourcePortTo(srcTo);
                for (int dst: dsts) {
                    builder.setDestinationPortFrom(dst);

                    int dnr = (dst == -1 || dst == 65535) ? 1 : nranges;
                    int[] dstRange = new int[dnr];
                    dstRange[0] = -1;
                    for (int i = 1; i < dstRange.length; i++) {
                        dstRange[i] = dst + i;
                    }
                    for (int dstTo: dstRange) {
                        builder.setDestinationPortTo(dstTo);
                        UdpMatchImpl umi = builder.create();

                        String s;
                        if (src == -1) {
                            s = null;
                        } else if (srcTo == -1 || src == srcTo) {
                            s = "src=" + src;
                        } else {
                            s = "src=" + src + "-" + srcTo;
                        }

                        String d;
                        if (dst == -1) {
                            d = null;
                        } else if (dstTo == -1 || dst == dstTo) {
                            d = "dst=" + dst;
                        } else {
                            d = "dst=" + dst + "-" + dstTo;
                        }

                        String required = joinStrings(prefix, suffix, ",",
                                                      s, d);
                        assertEquals(required, umi.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link UdpMatchImpl} is serializable.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testSerialize() throws VTNException {
        int[] srcs = {
            -1, 0, 4, 100, 678, 1357, 33445, 65535,
        };
        int[] dsts = {
            -1, 0, 8, 193, 783, 2674, 45678, 65535,
        };

        int nranges = 3;
        UdpMatchImplBuilder builder = new UdpMatchImplBuilder();
        for (int src: srcs) {
            builder.setSourcePortFrom(src);

            int snr = (src == -1 || src == 65535) ? 1 : nranges;
            int[] srcRange = new int[snr];
            srcRange[0] = -1;
            for (int i = 1; i < srcRange.length; i++) {
                srcRange[i] = src + i;
            }
            for (int srcTo: srcRange) {
                builder.setSourcePortTo(srcTo);
                for (int dst: dsts) {
                    builder.setDestinationPortFrom(dst);

                    int dnr = (dst == -1 || dst == 65535) ? 1 : nranges;
                    int[] dstRange = new int[dnr];
                    dstRange[0] = -1;
                    for (int i = 1; i < dstRange.length; i++) {
                        dstRange[i] = dst + i;
                    }
                    for (int dstTo: dstRange) {
                        builder.setDestinationPortTo(dstTo);
                        UdpMatchImpl umi = builder.create();
                        serializeTest(umi);
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link UdpMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        int srcPort = 123;
        int dstPort = 65000;
        byte[] payload = {
            (byte)0x04, (byte)0xce, (byte)0xfe, (byte)0x21,
            (byte)0xdc, (byte)0xb1, (byte)0x16, (byte)0x3b,
        };
        UDP udp = new UDP();
        udp.setSourcePort((short)srcPort).setDestinationPort((short)dstPort).
            setLength((short)123).setChecksum((short)0x1234);

        String saddr = "10.234.198.83";
        String daddr = "192.168.238.254";
        InetAddress src = InetAddress.getByName(saddr);
        InetAddress dst = InetAddress.getByName(daddr);
        short proto = IPProtocols.UDP.shortValue();
        byte dscp = 0;
        IPv4 ipv4 = createIPv4(src, dst, proto, dscp);
        ipv4.setPayload(udp);

        IPv4 nonUdp1 = createIPv4(src, dst, (short)100, dscp);
        nonUdp1.setRawPayload(payload);

        ICMP icmp = new ICMP();
        icmp.setType((byte)0).setCode((byte)1).setIdentifier((short)0x1234).
            setSequenceNumber((short)0x5678).setRawPayload(payload);
        IPv4 nonUdp2 = createIPv4(src, dst, IPProtocols.ICMP.shortValue(),
                                  dscp);
        nonUdp2.setPayload(icmp);
        IPv4[] nonUdp = {nonUdp1, nonUdp2};

        // Run tests with changing VLAN ID.
        short[] vlans = {
            MatchType.DL_VLAN_NONE, 10,
        };

        byte[] srcMac = {
            (byte)0x10, (byte)0x20, (byte)0x30,
            (byte)0x40, (byte)0x50, (byte)0x60,
        };
        byte[] dstMac = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xfa, (byte)0xfb, (byte)0xfc,
        };
        int ethType = EtherTypes.IPv4.intValue();
        byte pcp = 0;
        for (short vlan: vlans) {
            Ethernet pkt = createEthernet(srcMac, dstMac, ethType, vlan, pcp,
                                          ipv4);
            matchTest(pkt);

            // Test non-IPv4 packet.
            pkt = createEthernet(srcMac, dstMac, (short)100, vlan, pcp,
                                 payload);
            matchTest(pkt);

            // Test non-UDP packet.
            for (IPv4 p: nonUdp) {
                pkt = createEthernet(srcMac, dstMac, ethType, vlan, pcp, p);
                matchTest(pkt);
            }
        }
    }

    /**
     * Run tests for {@link UdpMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}
     * using the given packet.
     *
     * @param pkt  An {@link Ethernet} instance for test.
     */
    private void matchTest(Ethernet pkt) {
        PacketMatchTest test = new PacketMatchTest();
        UdpMatchImplBuilder builder = new UdpMatchImplBuilder();

        UDP udp = getUdpPacket(pkt);
        if (udp == null) {
            // Should not match non-UDP packet.
            assertEquals(false, test.run(builder.create(), pkt));

            builder.setSourcePortFrom(123).setDestinationPortFrom(65000);
            assertEquals(false, test.run(builder.create(), pkt));
            return;
        }

        UdpParser udpParser = new UdpParser(udp);
        int src = udpParser.getSourcePort();
        int dst = udpParser.getDestinationPort();

        int anotherSrc = udpParser.getAnotherSourcePort();
        int anotherDst = udpParser.getAnotherDestinationPort();

        // Empty match should match every packet.
        assertEquals(true, test.run(builder.create(), pkt));

        // Specify single field in UDP header.
        UdpMatchImpl umi = builder.reset().setSourcePortFrom(src).create();
        assertEquals(true, test.setMatchType(MatchType.TP_SRC).run(umi, pkt));
        umi = builder.reset().setSourcePortFrom(anotherSrc).create();
        assertEquals(false, test.run(umi, pkt));

        test.reset().setMatchType(MatchType.TP_DST);
        umi = builder.reset().setDestinationPortFrom(dst).create();
        assertEquals(true, test.run(umi, pkt));
        umi = builder.reset().setDestinationPortFrom(anotherDst).create();
        assertEquals(false, test.run(umi, pkt));

        // Specify port number by range.
        int nrange = 100;
        int srcFrom = src - nrange;
        int srcTo = src + nrange;
        test.reset().setMatchType(MatchType.TP_SRC);
        umi = builder.reset().setSourcePortFrom(srcFrom).
            setSourcePortTo(srcTo).create();
        assertEquals(true, test.run(umi, pkt));
        int bad = src + 1;
        umi = builder.setSourcePortFrom(bad).create();
        assertEquals(false, test.run(umi, pkt));
        bad = src - 1;
        umi = builder.setSourcePortFrom(srcFrom).setSourcePortTo(bad).create();
        assertEquals(false, test.run(umi, pkt));

        int dstFrom = dst - nrange;
        int dstTo = dst + nrange;
        test.reset().setMatchType(MatchType.TP_DST);
        umi = builder.reset().setDestinationPortFrom(dstFrom).
            setDestinationPortTo(dstTo).create();
        assertEquals(true, test.run(umi, pkt));
        bad = dst + 1;
        umi = builder.setDestinationPortFrom(bad).create();
        assertEquals(false, test.run(umi, pkt));
        bad = dst - 1;
        umi = builder.setDestinationPortFrom(dstFrom).
            setDestinationPortTo(bad).create();
        assertEquals(false, test.run(umi, pkt));

        // Specify all fields.
        test.reset().setMatchType(MatchType.TP_SRC, MatchType.TP_DST);
        umi = builder.reset().
            setSourcePortFrom(srcFrom).setSourcePortTo(srcTo).
            setDestinationPortFrom(dstFrom).setDestinationPortTo(dstTo).
            create();
        assertEquals(true, test.run(umi, pkt));

        test.reset().setMatchType(MatchType.TP_SRC);
        umi = builder.setSourcePortFrom(src + 1).create();
        assertEquals(false, test.run(umi, pkt));

        test.setMatchType(MatchType.TP_DST);
        umi = builder.setSourcePortFrom(src).setDestinationPortTo(dst - 1).
            create();
        assertEquals(false, test.run(umi, pkt));

        umi = builder.reset().setSourcePortFrom(src).
            setDestinationPortFrom(dst).create();
        assertEquals(true, test.run(umi, pkt));

        test.reset().setMatchType(MatchType.TP_SRC);
        umi = builder.setSourcePortFrom(src + 1).create();
        assertEquals(false, test.run(umi, pkt));

        test.setMatchType(MatchType.TP_DST);
        umi = builder.setSourcePortFrom(src).setDestinationPortFrom(dst + 1).
            create();
        assertEquals(false, test.run(umi, pkt));
    }

    /**
     * Return an {@link UDP} instance configured in the given packet.
     *
     * @param pkt  An {@link Ethernet} instance.
     * @return  An {@link UDP} instance if found.
     *          {@code null} if not found.
     */
    private UDP getUdpPacket(Ethernet pkt) {
        Packet payload = pkt.getPayload();
        if (payload instanceof IEEE8021Q) {
            IEEE8021Q tag = (IEEE8021Q)payload;
            payload = tag.getPayload();
        }

        if (payload instanceof IPv4) {
            IPv4 ipv4 = (IPv4)payload;
            payload = ipv4.getPayload();
            if (payload instanceof UDP) {
                return (UDP)payload;
            }
        }

        return null;
    }
}

/**
 * Utility class to create {@link UdpMatchImpl} or instance.
 */
final class UdpMatchImplBuilder extends TestBase
    implements L4MatchImplBuilder {
    /**
     * The minimum source port number to match.
     */
    private Integer  sourceFrom;

    /**
     * The maximum source port number to match.
     */
    private Integer  sourceTo;

    /**
     * The minimum destination port number to match.
     */
    private Integer  destinationFrom;

    /**
     * The maximum destination port number to match.
     */
    private Integer  destinationTo;

    /**
     * Created {@link UdpMatch} instance.
     */
    private UdpMatch  udpMatch;

    /**
     * Set the minumum source port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public UdpMatchImplBuilder setSourcePortFrom(int port) {
        sourceFrom = (port == -1) ? null : Integer.valueOf(port);
        udpMatch = null;
        return this;
    }

    /**
     * Set the minumum source port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public UdpMatchImplBuilder setSourcePortFrom(Integer port) {
        sourceFrom = port;
        udpMatch = null;
        return this;
    }

    /**
     * Set the maximum source port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public UdpMatchImplBuilder setSourcePortTo(int port) {
        sourceTo = (port == -1) ? null : Integer.valueOf(port);
        udpMatch = null;
        return this;
    }

    /**
     * Set the maximum source port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public UdpMatchImplBuilder setSourcePortTo(Integer port) {
        sourceTo = port;
        udpMatch = null;
        return this;
    }

    /**
     * Set the minumum destination port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public UdpMatchImplBuilder setDestinationPortFrom(int port) {
        destinationFrom = (port == -1) ? null : Integer.valueOf(port);
        udpMatch = null;
        return this;
    }

    /**
     * Set the minumum destination port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public UdpMatchImplBuilder setDestinationPortFrom(Integer port) {
        destinationFrom = port;
        udpMatch = null;
        return this;
    }

    /**
     * Set the maximum destination port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public UdpMatchImplBuilder setDestinationPortTo(int port) {
        destinationTo = (port == -1) ? null : Integer.valueOf(port);
        udpMatch = null;
        return this;
    }

    /**
     * Set the maximum destination port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public UdpMatchImplBuilder setDestinationPortTo(Integer port) {
        destinationTo = port;
        udpMatch = null;
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public UdpMatchImplBuilder reset() {
        sourceFrom = null;
        sourceTo = null;
        destinationFrom = null;
        destinationTo = null;
        udpMatch = null;

        return this;
    }

    /**
     * Construct an {@link UdpMatch} instance.
     *
     * @return  An {@link UdpMatch} instance.
     */
    public UdpMatch getUdpMatch() {
        UdpMatch um = udpMatch;
        if (um == null) {
            PortMatch src = (sourceFrom == null)
                ? null : new PortMatch(sourceFrom, sourceTo);
            PortMatch dst = (destinationFrom == null)
                ? null : new PortMatch(destinationFrom, destinationTo);
            um = new UdpMatch(src, dst);
            udpMatch = um;
        }

        return um;
    }

    /**
     * Construct an {@link UdpMatchImpl} instance.
     *
     * @return  An {@link UdpMatchImpl} instance.
     * @throws VTNException  An error occurred.
     */
    public UdpMatchImpl build() throws VTNException {
        UdpMatch um = getUdpMatch();
        return new UdpMatchImpl(um);
    }

    /**
     * Construct an {@link UdpMatchImpl} instance and ensure that it was
     * constructed successfully.
     *
     * @return  An {@link UdpMatchImpl} instance.
     */
    public UdpMatchImpl create() {
        UdpMatchImpl umi = null;
        try {
            umi = build();
        } catch (Exception e) {
            unexpected(e);
        }

        return umi;
    }

    /**
     * Construct an {@link UdpMatchImpl} instance, and verify it.
     */
    public void verify() {
        UdpMatchImpl umi = create();
        assertEquals(IPProtocols.UDP.shortValue(), umi.getInetProtocol());

        L4PortMatch src = umi.getSourcePort();
        if (sourceFrom == null) {
            assertEquals(null, src);
        } else {
            int from = sourceFrom.intValue();
            int to = (sourceTo == null) ? from : sourceTo.intValue();
            assertEquals(from, src.getPortFrom());
            assertEquals(to, src.getPortTo());
        }

        L4PortMatch dst = umi.getDestinationPort();
        if (destinationFrom == null) {
            assertEquals(null, dst);
        } else {
            int from = destinationFrom.intValue();
            int to = (destinationTo == null) ? from : destinationTo.intValue();
            assertEquals(from, dst.getPortFrom());
            assertEquals(to, dst.getPortTo());
        }

        // Instantiate using L4Match.create().
        UdpMatch um = getUdpMatch();
        try {
            assertEquals(umi, L4MatchImpl.create(um));
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public UdpMatch getLayer4Match() {
        return getUdpMatch();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public UdpMatchImplBuilder setSource(int src) {
        return setSourcePortFrom(src);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public UdpMatchImplBuilder setDestination(int dst) {
        return setDestinationPortFrom(dst);
    }
}

/**
 * UDP header parser.
 */
final class UdpParser implements L4Parser {
    /**
     * UDP packet.
     */
    private final UDP  packet;

    /**
     * Source port number.
     */
    private final int  sourcePort;

    /**
     * Destination port number.
     */
    private final int  destinationPort;

    /**
     * Construct a new instance.
     *
     * @param udp  A {@link UDP} instance.
     */
    UdpParser(UDP udp) {
        packet = udp;
        sourcePort = NetUtils.getUnsignedShort(udp.getSourcePort());
        destinationPort = NetUtils.getUnsignedShort(udp.getDestinationPort());
    }

    /**
     * Return the UDP packet.
     *
     * @return  An {@link UDP} instance.
     */
    public UDP getPacket() {
        return packet;
    }

    /**
     * Return the source port number of this UDP packet.
     *
     * @return  The source port number.
     */
    public int getSourcePort() {
        return sourcePort;
    }

    /**
     * Return the destination port number of this UDP packet.
     *
     * @return  The destination port number.
     */
    public int getDestinationPort() {
        return destinationPort;
    }

    /**
     * Return a source port number that does not match this packet.
     *
     * @return  A source port number that does not match this packet.
     */
    public int getAnotherSourcePort() {
        return (~sourcePort & 0xffff);
    }

    /**
     * Return a destination port number that does not match this packet.
     *
     * @return  A destination port number that does not match this packet.
     */
    public int getAnotherDestinationPort() {
        return (~destinationPort & 0xffff);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getSource() {
        return getSourcePort();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getDestination() {
        return getDestinationPort();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getAnotherSource() {
        return getAnotherSourcePort();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getAnotherDestination() {
        return getAnotherDestinationPort();
    }
}
