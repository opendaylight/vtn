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
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.flow.cond.PortMatch;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.TCP;
import org.opendaylight.controller.sal.packet.UDP;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link TcpMatchImpl}.
 */
public class TcpMatchImplTest extends TestBase {
    /**
     * Test for {@link TcpMatchImpl#TcpMatchImpl(TcpMatch)},
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
        TcpMatchImplBuilder builder = new TcpMatchImplBuilder();
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
        TcpMatch tm = new TcpMatch(empty, null);
        try {
            new TcpMatchImpl(tm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("source: \"from\" is not specified.",
                         st.getDescription());
        }

        tm = new TcpMatch(null, empty);
        try {
            new TcpMatchImpl(tm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("destination: \"from\" is not specified.",
                         st.getDescription());
        }

        // Specifying bad port number.
        Integer badPort = Integer.valueOf(-1);
        tm = new TcpMatch(badPort, null);
        try {
            new TcpMatchImpl(tm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("source: from: Invalid port number: " + badPort,
                         st.getDescription());
        }

        tm = new TcpMatch(null, badPort);
        try {
            new TcpMatchImpl(tm);
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
        tm = new TcpMatch(badRange, null);
        try {
            new TcpMatchImpl(tm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("source: Invalid port range: from=" + badFrom +
                         ", to=" + badTo, st.getDescription());
        }

        tm = new TcpMatch(null, badRange);
        try {
            new TcpMatchImpl(tm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("destination: Invalid port range: from=" + badFrom +
                         ", to=" + badTo, st.getDescription());
        }

        // Passing null to L4MatchImpl.create(L4Match).
        try {
            L4MatchImpl.create(null);
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Unexpected L4 match instance: null",
                         st.getDescription());
        }
    }

    /**
     * Test case for {@link TcpMatchImpl#equals(Object)} and
     * {@link TcpMatchImpl#hashCode()}.
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
        TcpMatchImplBuilder builder = new TcpMatchImplBuilder();
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
                        TcpMatchImpl tmi1 = builder.create();
                        TcpMatchImpl tmi2 = builder.create();
                        testEquals(set, tmi1, tmi2);
                        count++;
                    }
                }
            }
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link TcpMatchImpl#toString()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testToString() throws VTNException {
        String prefix = "TcpMatchImpl[";
        String suffix = "]";

        int[] srcs = {
            -1, 0, 4, 100, 678, 1357, 33445, 65535,
        };
        int[] dsts = {
            -1, 0, 8, 193, 783, 2674, 45678, 65535,
        };

        int nranges = 3;
        TcpMatchImplBuilder builder = new TcpMatchImplBuilder();
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
                        TcpMatchImpl tmi = builder.create();

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
                        assertEquals(required, tmi.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link TcpMatchImpl} is serializable.
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
        TcpMatchImplBuilder builder = new TcpMatchImplBuilder();
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
                        TcpMatchImpl tmi = builder.create();
                        serializeTest(tmi);
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link TcpMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}.
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
        TCP tcp = new TCP();
        tcp.setSourcePort((short)srcPort).setDestinationPort((short)dstPort).
            setSequenceNumber(0x12345).setAckNumber(0x23456).
            setDataOffset((byte)1).setHeaderLenFlags((short)0x18).
            setReserved((byte)0).setWindowSize((short)0xfc00).
            setChecksum((short)0x7777);

        String saddr = "10.234.198.83";
        String daddr = "192.168.238.254";
        InetAddress src = InetAddress.getByName(saddr);
        InetAddress dst = InetAddress.getByName(daddr);
        short proto = IPProtocols.TCP.shortValue();
        byte dscp = 0;
        IPv4 ipv4 = createIPv4(src, dst, proto, dscp);
        ipv4.setPayload(tcp);

        IPv4 nonTcp1 = createIPv4(src, dst, (short)100, dscp);
        nonTcp1.setRawPayload(payload);

        UDP udp = new UDP();
        udp.setSourcePort((short)1).setDestinationPort((short)2).
            setLength((short)123).setChecksum((short)0x1234);
        IPv4 nonTcp2 = createIPv4(src, dst, IPProtocols.UDP.shortValue(),
                                   dscp);
        nonTcp2.setPayload(udp);
        IPv4[] nonTcp = {nonTcp1, nonTcp2};

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

            // Test non-TCP packet.
            for (IPv4 p: nonTcp) {
                pkt = createEthernet(srcMac, dstMac, ethType, vlan, pcp, p);
                matchTest(pkt);
            }
        }
    }

    /**
     * Run tests for {@link TcpMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}
     * using the given packet.
     *
     * @param pkt  An {@link Ethernet} instance for test.
     */
    private void matchTest(Ethernet pkt) {
        PacketMatchTest test = new PacketMatchTest();
        TcpMatchImplBuilder builder = new TcpMatchImplBuilder();

        TCP tcp = getTcpPacket(pkt);
        if (tcp == null) {
            // Should not match non-TCP packet.
            assertEquals(false, test.run(builder.create(), pkt));

            builder.setSourcePortFrom(123).setDestinationPortFrom(65000);
            assertEquals(false, test.run(builder.create(), pkt));
            return;
        }

        TcpParser tcpParser = new TcpParser(tcp);
        int src = tcpParser.getSourcePort();
        int dst = tcpParser.getDestinationPort();

        int anotherSrc = tcpParser.getAnotherSourcePort();
        int anotherDst = tcpParser.getAnotherDestinationPort();

        // Empty match should match every packet.
        assertEquals(true, test.run(builder.create(), pkt));

        // Specify single field in TCP header.
        TcpMatchImpl tmi = builder.reset().setSourcePortFrom(src).create();
        assertEquals(true, test.setMatchType(MatchType.TP_SRC).run(tmi, pkt));
        tmi = builder.reset().setSourcePortFrom(anotherSrc).create();
        assertEquals(false, test.run(tmi, pkt));

        test.reset().setMatchType(MatchType.TP_DST);
        tmi = builder.reset().setDestinationPortFrom(dst).create();
        assertEquals(true, test.run(tmi, pkt));
        tmi = builder.reset().setDestinationPortFrom(anotherDst).create();
        assertEquals(false, test.run(tmi, pkt));

        // Specify port number by range.
        int nrange = 100;
        int srcFrom = src - nrange;
        int srcTo = src + nrange;
        test.reset().setMatchType(MatchType.TP_SRC);
        tmi = builder.reset().setSourcePortFrom(srcFrom).
            setSourcePortTo(srcTo).create();
        assertEquals(true, test.run(tmi, pkt));
        int bad = src + 1;
        tmi = builder.setSourcePortFrom(bad).create();
        assertEquals(false, test.run(tmi, pkt));
        bad = src - 1;
        tmi = builder.setSourcePortFrom(srcFrom).setSourcePortTo(bad).create();
        assertEquals(false, test.run(tmi, pkt));

        int dstFrom = dst - nrange;
        int dstTo = dst + nrange;
        test.reset().setMatchType(MatchType.TP_DST);
        tmi = builder.reset().setDestinationPortFrom(dstFrom).
            setDestinationPortTo(dstTo).create();
        assertEquals(true, test.run(tmi, pkt));
        bad = dst + 1;
        tmi = builder.setDestinationPortFrom(bad).create();
        assertEquals(false, test.run(tmi, pkt));
        bad = dst - 1;
        tmi = builder.setDestinationPortFrom(dstFrom).
            setDestinationPortTo(bad).create();
        assertEquals(false, test.run(tmi, pkt));

        // Specify all fields.
        test.reset().setMatchType(MatchType.TP_SRC, MatchType.TP_DST);
        tmi = builder.reset().
            setSourcePortFrom(srcFrom).setSourcePortTo(srcTo).
            setDestinationPortFrom(dstFrom).setDestinationPortTo(dstTo).
            create();
        assertEquals(true, test.run(tmi, pkt));

        test.reset().setMatchType(MatchType.TP_SRC);
        tmi = builder.setSourcePortFrom(src + 1).create();
        assertEquals(false, test.run(tmi, pkt));

        test.setMatchType(MatchType.TP_DST);
        tmi = builder.setSourcePortFrom(src).setDestinationPortTo(dst - 1).
            create();
        assertEquals(false, test.run(tmi, pkt));

        tmi = builder.reset().setSourcePortFrom(src).
            setDestinationPortFrom(dst).create();
        assertEquals(true, test.run(tmi, pkt));

        test.reset().setMatchType(MatchType.TP_SRC);
        tmi = builder.setSourcePortFrom(src + 1).create();
        assertEquals(false, test.run(tmi, pkt));

        test.setMatchType(MatchType.TP_DST);
        tmi = builder.setSourcePortFrom(src).setDestinationPortFrom(dst + 1).
            create();
        assertEquals(false, test.run(tmi, pkt));
    }

    /**
     * Return an {@link TCP} instance configured in the given packet.
     *
     * @param pkt  An {@link Ethernet} instance.
     * @return  An {@link TCP} instance if found.
     *          {@code null} if not found.
     */
    private TCP getTcpPacket(Ethernet pkt) {
        Packet payload = pkt.getPayload();
        if (payload instanceof IEEE8021Q) {
            IEEE8021Q tag = (IEEE8021Q)payload;
            payload = tag.getPayload();
        }

        if (payload instanceof IPv4) {
            IPv4 ipv4 = (IPv4)payload;
            payload = ipv4.getPayload();
            if (payload instanceof TCP) {
                return (TCP)payload;
            }
        }

        return null;
    }
}

/**
 * Interface for {@link L4MatchImpl} builder.
 */
interface L4MatchImplBuilder {
    /**
     * Construct a new {@link L4Match} instance.
     *
     * @return  A {@link L4Match} instance.
     */
    L4Match getLayer4Match();

    /**
     * Set an integer value that specifies the source.
     *
     * @param src  An integer value that specifies the source.
     * @return  This instance.
     */
    L4MatchImplBuilder setSource(int src);

    /**
     * Set an integer value that specifies the destination.
     *
     * @param dst  An integer value that specifies the destination.
     * @return  This instance.
     */
    L4MatchImplBuilder setDestination(int dst);
}

/**
 * Utility class to create {@link TcpMatchImpl} or instance.
 */
final class TcpMatchImplBuilder extends TestBase
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
     * Created {@link TcpMatch} instance.
     */
    private TcpMatch  tcpMatch;

    /**
     * Set the minumum source port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public TcpMatchImplBuilder setSourcePortFrom(int port) {
        sourceFrom = (port == -1) ? null : Integer.valueOf(port);
        tcpMatch = null;
        return this;
    }

    /**
     * Set the minumum source port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public TcpMatchImplBuilder setSourcePortFrom(Integer port) {
        sourceFrom = port;
        tcpMatch = null;
        return this;
    }

    /**
     * Set the maximum source port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public TcpMatchImplBuilder setSourcePortTo(int port) {
        sourceTo = (port == -1) ? null : Integer.valueOf(port);
        tcpMatch = null;
        return this;
    }

    /**
     * Set the maximum source port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public TcpMatchImplBuilder setSourcePortTo(Integer port) {
        sourceTo = port;
        tcpMatch = null;
        return this;
    }

    /**
     * Set the minumum destination port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public TcpMatchImplBuilder setDestinationPortFrom(int port) {
        destinationFrom = (port == -1) ? null : Integer.valueOf(port);
        tcpMatch = null;
        return this;
    }

    /**
     * Set the minumum destination port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public TcpMatchImplBuilder setDestinationPortFrom(Integer port) {
        destinationFrom = port;
        tcpMatch = null;
        return this;
    }

    /**
     * Set the maximum destination port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public TcpMatchImplBuilder setDestinationPortTo(int port) {
        destinationTo = (port == -1) ? null : Integer.valueOf(port);
        tcpMatch = null;
        return this;
    }

    /**
     * Set the maximum destination port number.
     *
     * @param port  Port number.
     * @return  This instance.
     */
    public TcpMatchImplBuilder setDestinationPortTo(Integer port) {
        destinationTo = port;
        tcpMatch = null;
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public TcpMatchImplBuilder reset() {
        sourceFrom = null;
        sourceTo = null;
        destinationFrom = null;
        destinationTo = null;
        tcpMatch = null;

        return this;
    }

    /**
     * Construct an {@link TcpMatch} instance.
     *
     * @return  An {@link TcpMatch} instance.
     */
    public TcpMatch getTcpMatch() {
        TcpMatch tm = tcpMatch;
        if (tm == null) {
            PortMatch src = (sourceFrom == null)
                ? null : new PortMatch(sourceFrom, sourceTo);
            PortMatch dst = (destinationFrom == null)
                ? null : new PortMatch(destinationFrom, destinationTo);
            tm = new TcpMatch(src, dst);
            tcpMatch = tm;
        }

        return tm;
    }

    /**
     * Construct an {@link TcpMatchImpl} instance.
     *
     * @return  An {@link TcpMatchImpl} instance.
     * @throws VTNException  An error occurred.
     */
    public TcpMatchImpl build() throws VTNException {
        TcpMatch tm = getTcpMatch();
        return new TcpMatchImpl(tm);
    }

    /**
     * Construct an {@link TcpMatchImpl} instance and ensure that it was
     * constructed successfully.
     *
     * @return  An {@link TcpMatchImpl} instance.
     */
    public TcpMatchImpl create() {
        TcpMatchImpl tmi = null;
        try {
            tmi = build();
        } catch (Exception e) {
            unexpected(e);
        }

        return tmi;
    }

    /**
     * Construct an {@link TcpMatchImpl} instance, and verify it.
     */
    public void verify() {
        TcpMatchImpl tmi = create();
        assertEquals(IPProtocols.TCP.shortValue(), tmi.getInetProtocol());

        L4PortMatch src = tmi.getSourcePort();
        if (sourceFrom == null) {
            assertEquals(null, src);
        } else {
            int from = sourceFrom.intValue();
            int to = (sourceTo == null) ? from : sourceTo.intValue();
            assertEquals(from, src.getPortFrom());
            assertEquals(to, src.getPortTo());
        }

        L4PortMatch dst = tmi.getDestinationPort();
        if (destinationFrom == null) {
            assertEquals(null, dst);
        } else {
            int from = destinationFrom.intValue();
            int to = (destinationTo == null) ? from : destinationTo.intValue();
            assertEquals(from, dst.getPortFrom());
            assertEquals(to, dst.getPortTo());
        }

        // Instantiate using L4Match.create().
        TcpMatch tm = getTcpMatch();
        try {
            assertEquals(tmi, L4MatchImpl.create(tm));
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public TcpMatch getLayer4Match() {
        return getTcpMatch();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public TcpMatchImplBuilder setSource(int src) {
        return setSourcePortFrom(src);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public TcpMatchImplBuilder setDestination(int dst) {
        return setDestinationPortFrom(dst);
    }
}

/**
 * Interface for layer 4 protocol header parser.
 */
interface L4Parser {
    /**
     * Return an integer corresponding to the source.
     *
     * @return  An integer corresponding to the source.
     */
    int getSource();

    /**
     * Return an integer corresponding to the destination.
     *
     * @return  An integer corresponding to the destination.
     */
    int getDestination();

    /**
     * Return an integer corresponding to the source that does not match
     * the packet.
     *
     * @return  An integer corresponding to the source.
     */
    int getAnotherSource();

    /**
     * Return an integer corresponding to the destination that does not match
     * the packet.
     *
     * @return  An integer corresponding to the destination.
     */
    int getAnotherDestination();
};

/**
 * TCP header parser.
 */
final class TcpParser implements L4Parser {
    /**
     * TCP packet.
     */
    private final TCP  packet;

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
     * @param tcp  A {@link TCP} instance.
     */
    TcpParser(TCP tcp) {
        packet = tcp;
        sourcePort = NetUtils.getUnsignedShort(tcp.getSourcePort());
        destinationPort = NetUtils.getUnsignedShort(tcp.getDestinationPort());
    }

    /**
     * Return the TCP packet.
     *
     * @return  An {@link TCP} instance.
     */
    public TCP getPacket() {
        return packet;
    }

    /**
     * Return the source port number of this TCP packet.
     *
     * @return  The source port number.
     */
    public int getSourcePort() {
        return sourcePort;
    }

    /**
     * Return the destination port number of this TCP packet.
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
