/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.action.SetNwTos;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchField;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * JUnit test for {@link Inet4Packet}.
 */
public class Inet4PacketTest extends TestBase {
    /**
     * The flag bit which indicates the source IP address.
     */
    private static final int  IPV4_SRC = 0x1;

    /**
     * The flag bit which indicates the destination IP address.
     */
    private static final int  IPV4_DST = 0x2;

    /**
     * The flag bit which indicates the DSCP field.
     */
    private static final int  IPV4_DSCP = 0x4;

    /**
     * The flag bits which indicates all modifyable fields.
     */
    private static final int  IPV4_ALL = (IPV4_SRC | IPV4_DST | IPV4_DSCP);

    /**
     * Test case for getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        byte[] bytes = {
            (byte)0, (byte)1, (byte)33, (byte)128, (byte)192, (byte)255,
        };
        short[] protocols = {1, 6, 17, 100, 255};
        byte[] dscps = {(byte)0, (byte)1, (byte)10, (byte)40, (byte)63};
        boolean inetFirst = true;
        for (byte s: bytes) {
            byte[] srcBytes = {(byte)10, (byte)1, (byte)2, s};
            InetAddress src = createInetAddress(srcBytes);
            int srcAddr = NetUtils.byteArray4ToInt(srcBytes);
            for (byte d: bytes) {
                byte[] dstBytes = {(byte)192, (byte)168, (byte)200, d};
                InetAddress dst = createInetAddress(dstBytes);
                int dstAddr = NetUtils.byteArray4ToInt(dstBytes);
                for (short proto: protocols) {
                    for (byte dscp: dscps) {
                        IPv4 pkt = createIPv4Packet(src, dst, proto, dscp);
                        Inet4Packet ipv4 = new Inet4Packet(pkt);
                        if (inetFirst) {
                            assertEquals(src, ipv4.getSourceInetAddress());
                            assertEquals(dst, ipv4.getDestinationInetAddress());
                            assertEquals(srcAddr, ipv4.getSourceAddress());
                            assertEquals(dstAddr, ipv4.getDestinationAddress());
                            inetFirst = false;
                        } else {
                            assertEquals(srcAddr, ipv4.getSourceAddress());
                            assertEquals(dstAddr, ipv4.getDestinationAddress());
                            assertEquals(src, ipv4.getSourceInetAddress());
                            assertEquals(dst, ipv4.getDestinationInetAddress());
                            inetFirst = true;
                        }

                        assertEquals(proto, ipv4.getProtocol());
                        assertEquals(dscp, ipv4.getDscp());
                        assertEquals(false, ipv4.isAddressModified());

                        // commit() should return false.
                        Ethernet ether = createEthernet(pkt);
                        PacketContext pctx = createPacketContext(
                            ether, EtherPacketTest.NODE_CONNECTOR);
                        assertFalse(ipv4.commit(pctx));
                        assertEquals(null, pctx.getFilterActions());

                        for (MatchType mtype: MatchType.values()) {
                            assertFalse(pctx.hasMatchField(mtype));
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for setter methods and {@link Inet4Packet#clone()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetter() throws Exception {
        short proto = 111;
        byte dscp0 = 1;
        byte dscp1 = 63;

        byte[] srcBytes0 = {(byte)192, (byte)168, (byte)33, (byte)44};
        byte[] dstBytes0 = {(byte)172, (byte)30, (byte)40, (byte)50};
        InetAddress src0 = createInetAddress(srcBytes0);
        InetAddress dst0 = createInetAddress(dstBytes0);
        int srcAddr0 = NetUtils.byteArray4ToInt(srcBytes0);
        int dstAddr0 = NetUtils.byteArray4ToInt(dstBytes0);

        byte[] srcBytes1 = {(byte)127, (byte)0, (byte)0, (byte)1};
        byte[] dstBytes1 = {(byte)10, (byte)100, (byte)220, (byte)254};
        InetAddress src1 = createInetAddress(srcBytes1);
        InetAddress dst1 = createInetAddress(dstBytes1);
        int srcAddr1 = NetUtils.byteArray4ToInt(srcBytes1);
        int dstAddr1 = NetUtils.byteArray4ToInt(dstBytes1);

        byte[] srcBytes2 = {(byte)127, (byte)0, (byte)0, (byte)1};
        byte[] dstBytes2 = {(byte)10, (byte)100, (byte)220, (byte)254};
        InetAddress src2 = createInetAddress(srcBytes1);
        InetAddress dst2 = createInetAddress(dstBytes2);
        int srcAddr2 = NetUtils.byteArray4ToInt(srcBytes2);
        int dstAddr2 = NetUtils.byteArray4ToInt(dstBytes2);
        byte dscp2 = 30;

        Map<Class<? extends Action>, Action> salActions =
            new LinkedHashMap<Class<? extends Action>, Action>();
        salActions.put(SetNwSrc.class, new SetNwSrc(src2));
        salActions.put(SetNwDst.class, new SetNwDst(dst2));
        salActions.put(SetNwTos.class, new SetNwTos(dscp2));

        for (int flags = IPV4_SRC; flags <= IPV4_ALL; flags++) {
            IPv4 pkt = createIPv4Packet(src0, dst0, proto, dscp0);
            Inet4Packet ipv4 = new Inet4Packet(pkt);

            InetAddress src = src0;
            InetAddress dst = dst0;
            int srcAddr = srcAddr0;
            int dstAddr = dstAddr0;
            byte dscp = dscp0;

            Ethernet ether = createEthernet(pkt);
            PacketContext pctx =
                createPacketContext(ether, EtherPacketTest.NODE_CONNECTOR);
            for (Action act: salActions.values()) {
                pctx.addFilterAction(act);
            }

            Inet4Packet ip = ipv4.clone();
            boolean addrModified = false;
            if ((flags & IPV4_SRC) != 0) {
                // Modify source address.
                ip.setSourceAddress(src1);
                src = src1;
                srcAddr = srcAddr1;
                addrModified = true;
            }
            if ((flags & IPV4_DST) != 0) {
                // Modify destination address.
                ip.setDestinationAddress(dst1);
                dst = dst1;
                dstAddr = dstAddr1;
                addrModified = true;
            }
            if ((flags & IPV4_DSCP) != 0) {
                // Modify DSCP field.
                ip.setDscp(dscp1);
                dscp = dscp1;
            }

            assertEquals(src, ip.getSourceInetAddress());
            assertEquals(srcAddr, ip.getSourceAddress());
            assertEquals(dst, ip.getDestinationInetAddress());
            assertEquals(dstAddr, ip.getDestinationAddress());
            assertEquals(proto, ip.getProtocol());
            assertEquals(dscp, ip.getDscp());
            assertEquals(addrModified, ip.isAddressModified());

            // The packet should not be modified until commit() is called.
            assertSame(pkt, ip.getPacket());

            assertEquals(true, ip.commit(pctx));
            IPv4 newPkt = ip.getPacket();
            assertNotSame(pkt, newPkt);
            assertEquals(srcAddr, newPkt.getSourceAddress());
            assertEquals(dstAddr, newPkt.getDestinationAddress());
            assertEquals((byte)proto, newPkt.getProtocol());
            assertEquals(dscp, newPkt.getDiffServ());

            List<Action> filterActions =
                new ArrayList<Action>(pctx.getFilterActions());
            assertEquals(new ArrayList<Action>(salActions.values()),
                         filterActions);
            assertTrue(pctx.hasMatchField(MatchType.DL_TYPE));

            // Actions for unchanged field will be removed if corresponding
            // match type is configured in PacketContext.
            List<Action> actions = new ArrayList<Action>();
            if ((flags & IPV4_SRC) != 0) {
                actions.add(salActions.get(SetNwSrc.class));
            }
            if ((flags & IPV4_DST) != 0) {
                actions.add(salActions.get(SetNwDst.class));
            }
            if ((flags & IPV4_DSCP) != 0) {
                actions.add(salActions.get(SetNwTos.class));
            }

            pctx = createPacketContext(ether, EtherPacketTest.NODE_CONNECTOR);
            for (MatchType mt: MatchType.values()) {
                pctx.addMatchField(mt);
            }
            for (Action act: salActions.values()) {
                pctx.addFilterAction(act);
            }
            assertEquals(true, ip.commit(pctx));
            assertSame(newPkt, ip.getPacket());
            filterActions = new ArrayList<Action>(pctx.getFilterActions());
            assertEquals(actions, filterActions);

            // The original packet should not be affected.
            assertEquals(srcAddr0, ipv4.getSourceAddress());
            assertEquals(dstAddr0, ipv4.getDestinationAddress());
            assertEquals(proto, ipv4.getProtocol());
            assertEquals(dscp0, ipv4.getDscp());
            assertSame(pkt, ipv4.getPacket());

            assertEquals(srcAddr0, pkt.getSourceAddress());
            assertEquals(dstAddr0, pkt.getDestinationAddress());
            assertEquals((byte)proto, pkt.getProtocol());
            assertEquals(dscp0, pkt.getDiffServ());

            // Set values in the original packet.
            ip.setSourceAddress(src0);
            ip.setDestinationAddress(dst0);
            ip.setDscp(dscp0);
            assertEquals(false, ip.commit(pctx));
            assertEquals(src0, ip.getSourceInetAddress());
            assertEquals(srcAddr0, ip.getSourceAddress());
            assertEquals(dst0, ip.getDestinationInetAddress());
            assertEquals(dstAddr0, ip.getDestinationAddress());
            assertEquals(proto, ip.getProtocol());
            assertEquals(dscp0, ip.getDscp());
        }
    }

    /**
     * Test case for {@link Inet4Packet#setMatch(Match, Set)}.
     */
    @Test
    public void testSetMatch() {
        byte[] srcAddr = {(byte)10, (byte)1, (byte)2, (byte)3};
        byte[] dstAddr = {(byte)192, (byte)168, (byte)111, (byte)222};
        InetAddress src = createInetAddress(srcAddr);
        InetAddress dst = createInetAddress(dstAddr);
        byte dscp = 0;
        short proto = 100;
        Map<MatchType, MatchField> nwFields =
            new HashMap<MatchType, MatchField>();
        nwFields.put(MatchType.NW_SRC, new MatchField(MatchType.NW_SRC, src));
        nwFields.put(MatchType.NW_DST, new MatchField(MatchType.NW_DST, dst));
        nwFields.put(MatchType.NW_PROTO,
                     new MatchField(MatchType.NW_PROTO,
                                    Byte.valueOf((byte)proto)));
        nwFields.put(MatchType.NW_TOS,
                     new MatchField(MatchType.NW_TOS, Byte.valueOf(dscp)));

        byte[] srcAddr1 = {(byte)100, (byte)200, (byte)33, (byte)44};
        byte[] dstAddr1 = {(byte)172, (byte)16, (byte)123, (byte)234};
        InetAddress src1 = createInetAddress(srcAddr1);
        InetAddress dst1 = createInetAddress(dstAddr1);
        byte dscp1 = 63;

        IPv4 pkt = createIPv4Packet(src, dst, proto, dscp);
        Inet4Packet ipv4 = new Inet4Packet(pkt);

        Match match = new Match();
        Set<MatchType> fields = EnumSet.noneOf(MatchType.class);
        ipv4.setMatch(match, fields);
        List<MatchType> matches = match.getMatchesList();
        assertEquals(0, matches.size());

        for (Map.Entry<MatchType, MatchField> entry: nwFields.entrySet()) {
            MatchType mtype = entry.getKey();
            MatchField mfield = entry.getValue();

            match = new Match();
            fields = EnumSet.of(mtype);
            ipv4.setMatch(match, fields);
            matches = match.getMatchesList();
            assertEquals(1, matches.size());
            assertEquals(mfield, match.getField(mtype));
        }

        // setMatch() always has to see the original.
        ipv4.setSourceAddress(src1);
        ipv4.setDestinationAddress(dst1);
        ipv4.setDscp(dscp1);
        fields = EnumSet.noneOf(MatchType.class);
        fields.addAll(nwFields.keySet());
        match = new Match();
        ipv4.setMatch(match, fields);
        matches = match.getMatchesList();
        assertEquals(nwFields.size(), matches.size());
        for (Map.Entry<MatchType, MatchField> entry: nwFields.entrySet()) {
            MatchType mtype = entry.getKey();
            MatchField mfield = entry.getValue();
            assertEquals(mfield, match.getField(mtype));
        }
    }

    /**
     * Test case for {@link Inet4Packet#getHeaderForChecksum(byte, short)}.
     */
    @Test
    public void testGetHeaderForChecksum() {
        byte[] srcAddr = {(byte)100, (byte)200, (byte)33, (byte)44};
        byte[] dstAddr = {(byte)172, (byte)16, (byte)123, (byte)234};
        InetAddress src = createInetAddress(srcAddr);
        InetAddress dst = createInetAddress(dstAddr);
        short protocol = 100;
        short len = 60;
        IPv4 pkt = createIPv4Packet(src, dst, protocol, (byte)0);
        Inet4Packet ipv4 = new Inet4Packet(pkt);
        byte[] header = ipv4.getHeaderForChecksum((byte)protocol, len);
        verifyChecksumHeader(header, srcAddr, dstAddr, (byte)protocol, len);

        Random rand = new Random();

        for (int i = 0; i < 30; i++) {
            srcAddr = NetUtils.intToByteArray4(rand.nextInt());
            dstAddr = NetUtils.intToByteArray4(rand.nextInt());
            src = createInetAddress(srcAddr);
            dst = createInetAddress(dstAddr);
            short proto = (short)(rand.nextInt() & 0xff);
            do {
                len = (short)(rand.nextInt() & 0xffff);
            } while (len < 20);

            ipv4.setSourceAddress(src);
            ipv4.setDestinationAddress(dst);
            header = ipv4.getHeaderForChecksum((byte)proto, len);
            verifyChecksumHeader(header, srcAddr, dstAddr, (byte)proto, len);
        }
    }

    /**
     * Create an {@link IPv4} instance for test.
     *
     * @param src    Source IP address.
     * @param dst    Destination IP address.
     * @param proto  IP protocol number.
     * @param dscp   DSCP field value.
     * @return  An {@link IPv4} instance.
     */
    private IPv4 createIPv4Packet(InetAddress src, InetAddress dst,
                                  short proto, byte dscp) {
        IPv4 pkt = createIPv4(src, dst, proto, dscp);
        pkt.setRawPayload(EtherPacketTest.RAW_PAYLOAD);

        return pkt;
    }

    /**
     * Verify the contents of pseudo IPv4 header for computing checksum.
     *
     * @param header  Header to be tested.
     * @param src     Expected source IP address.
     * @param dst     Expected destination IP address.
     * @param proto   Expected IP protocol number.
     * @param len     Expected packet length.
     */
    private void verifyChecksumHeader(byte[] header, byte[] src, byte[] dst,
                                      byte proto, short len) {
        assertEquals(12, header.length);
        for (int i = 0; i < src.length; i++) {
            assertEquals(src[i], header[i]);
        }

        int off = src.length;
        for (int i = 0; i < dst.length; i++) {
            assertEquals(dst[i], header[off + i]);
        }

        off += dst.length;
        assertEquals(0, header[off]);

        off++;
        assertEquals((byte)proto, header[off]);

        off++;
        assertEquals((byte)(len >>> Byte.SIZE), header[off]);
        off++;
        assertEquals((byte)(len & ((1 << Byte.SIZE) - 1)), header[off]);
    }
}
