/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.packet.IPv4;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNInet4Match;

import org.opendaylight.vtn.manager.internal.TestBase;

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
        short[] protocols = {3, 15, 100, 255};
        byte[] dscps = {(byte)0, (byte)1, (byte)10, (byte)40, (byte)63};
        for (byte s: bytes) {
            byte[] srcBytes = {(byte)10, (byte)1, (byte)2, s};
            Ip4Network src = new Ip4Network(srcBytes);
            for (byte d: bytes) {
                byte[] dstBytes = {(byte)192, (byte)168, (byte)200, d};
                Ip4Network dst = new Ip4Network(dstBytes);
                for (short proto: protocols) {
                    for (byte dscp: dscps) {
                        IPv4 pkt = createIPv4Packet(src, dst, proto, dscp);
                        Inet4Packet ipv4 = new Inet4Packet(pkt);
                        assertEquals(src, ipv4.getSourceAddress());
                        assertEquals(dst, ipv4.getDestinationAddress());
                        assertEquals(proto, ipv4.getProtocol());
                        assertEquals(dscp, ipv4.getDscp());
                        assertEquals(false, ipv4.isAddressModified());

                        // commit() should return false.
                        Ethernet ether = createEthernet(pkt);
                        PacketContext pctx = createPacketContext(
                            ether, EtherPacketTest.NODE_CONNECTOR);
                        assertFalse(ipv4.commit(pctx));
                        assertEquals(null, pctx.getFilterActions());

                        for (FlowMatchType mtype: FlowMatchType.values()) {
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
        short dscp0 = 1;
        short dscp1 = 63;
        short dscp2 = 30;

        byte[] srcBytes0 = {(byte)192, (byte)168, (byte)33, (byte)44};
        byte[] dstBytes0 = {(byte)172, (byte)30, (byte)40, (byte)50};
        Ip4Network src0 = new Ip4Network(srcBytes0);
        Ip4Network dst0 = new Ip4Network(dstBytes0);

        byte[] srcBytes1 = {(byte)127, (byte)0, (byte)0, (byte)1};
        byte[] dstBytes1 = {(byte)10, (byte)100, (byte)220, (byte)254};
        Ip4Network src1 = new Ip4Network(srcBytes1);
        Ip4Network dst1 = new Ip4Network(dstBytes1);

        byte[] srcBytes2 = {(byte)127, (byte)0, (byte)0, (byte)1};
        byte[] dstBytes2 = {(byte)10, (byte)100, (byte)220, (byte)254};
        Ip4Network src2 = new Ip4Network(srcBytes1);
        Ip4Network dst2 = new Ip4Network(dstBytes2);

        Map<Class<? extends FlowFilterAction>, FlowFilterAction> fltActions =
            new LinkedHashMap<>();
        fltActions.put(VTNSetInetSrcAction.class,
                       new VTNSetInetSrcAction(src2));
        fltActions.put(VTNSetInetDstAction.class,
                       new VTNSetInetDstAction(dst2));
        fltActions.put(VTNSetInetDscpAction.class,
                       new VTNSetInetDscpAction(dscp2));

        for (int flags = IPV4_SRC; flags <= IPV4_ALL; flags++) {
            IPv4 pkt = createIPv4Packet(src0, dst0, proto, dscp0);
            Inet4Packet ipv4 = new Inet4Packet(pkt);

            Ip4Network src = src0;
            Ip4Network dst = dst0;
            short dscp = dscp0;

            Ethernet ether = createEthernet(pkt);
            PacketContext pctx =
                createPacketContext(ether, EtherPacketTest.NODE_CONNECTOR);
            for (FlowFilterAction act: fltActions.values()) {
                pctx.addFilterAction(act);
            }

            Inet4Packet ip = ipv4.clone();
            boolean addrModified = false;
            if ((flags & IPV4_SRC) != 0) {
                // Modify source address.
                ip.setSourceAddress(src1);
                src = src1;
                addrModified = true;
            }
            if ((flags & IPV4_DST) != 0) {
                // Modify destination address.
                ip.setDestinationAddress(dst1);
                dst = dst1;
                addrModified = true;
            }
            if ((flags & IPV4_DSCP) != 0) {
                // Modify DSCP field.
                ip.setDscp(dscp1);
                dscp = dscp1;
            }

            assertEquals(src, ip.getSourceAddress());
            assertEquals(dst, ip.getDestinationAddress());
            assertEquals(proto, ip.getProtocol());
            assertEquals(dscp, ip.getDscp());
            assertEquals(addrModified, ip.isAddressModified());

            // The packet should not be modified until commit() is called.
            assertSame(pkt, ip.getPacket());

            assertEquals(true, ip.commit(pctx));
            IPv4 newPkt = ip.getPacket();
            assertNotSame(pkt, newPkt);
            assertEquals(src, newPkt.getSourceAddress());
            assertEquals(dst, newPkt.getDestinationAddress());
            assertEquals((byte)proto, newPkt.getProtocol());
            assertEquals((byte)dscp, newPkt.getDiffServ());

            List<FlowFilterAction> filterActions =
                new ArrayList<>(pctx.getFilterActions());
            assertEquals(new ArrayList<FlowFilterAction>(fltActions.values()),
                         filterActions);
            assertTrue(pctx.hasMatchField(FlowMatchType.DL_TYPE));

            // Actions for unchanged field will be removed if corresponding
            // match type is configured in PacketContext.
            List<FlowFilterAction> actions = new ArrayList<>();
            if ((flags & IPV4_SRC) != 0) {
                actions.add(fltActions.get(VTNSetInetSrcAction.class));
            }
            if ((flags & IPV4_DST) != 0) {
                actions.add(fltActions.get(VTNSetInetDstAction.class));
            }
            if ((flags & IPV4_DSCP) != 0) {
                actions.add(fltActions.get(VTNSetInetDscpAction.class));
            }

            pctx = createPacketContext(ether, EtherPacketTest.NODE_CONNECTOR);
            for (FlowMatchType mt: FlowMatchType.values()) {
                pctx.addMatchField(mt);
            }
            for (FlowFilterAction act: fltActions.values()) {
                pctx.addFilterAction(act);
            }
            assertEquals(true, ip.commit(pctx));
            assertSame(newPkt, ip.getPacket());
            filterActions = new ArrayList<>(pctx.getFilterActions());
            assertEquals(actions, filterActions);

            // The original packet should not be affected.
            assertEquals(src0, ipv4.getSourceAddress());
            assertEquals(dst0, ipv4.getDestinationAddress());
            assertEquals(proto, ipv4.getProtocol());
            assertEquals(dscp0, ipv4.getDscp());
            assertSame(pkt, ipv4.getPacket());

            assertEquals(src0, pkt.getSourceAddress());
            assertEquals(dst0, pkt.getDestinationAddress());
            assertEquals((byte)proto, pkt.getProtocol());
            assertEquals((byte)dscp0, pkt.getDiffServ());

            // Set values in the original packet.
            ip.setSourceAddress(src0);
            ip.setDestinationAddress(dst0);
            ip.setDscp(dscp0);
            assertEquals(false, ip.commit(pctx));
            assertEquals(src0, ip.getSourceAddress());
            assertEquals(dst0, ip.getDestinationAddress());
            assertEquals(proto, ip.getProtocol());
            assertEquals(dscp0, ip.getDscp());

            // Ensure that a set of modified values is deeply cloned.
            Inet4Packet ip1 = ip.clone();
            assertEquals(src0, ip.getSourceAddress());
            assertEquals(dst0, ip.getDestinationAddress());
            assertEquals(proto, ip.getProtocol());
            assertEquals(dscp0, ip.getDscp());
            assertEquals(src0, ip1.getSourceAddress());
            assertEquals(dst0, ip1.getDestinationAddress());
            assertEquals(proto, ip1.getProtocol());
            assertEquals(dscp0, ip1.getDscp());

            ip1.setSourceAddress(src1);
            ip1.setDestinationAddress(dst1);
            ip1.setDscp(dscp1);
            assertEquals(src0, ip.getSourceAddress());
            assertEquals(dst0, ip.getDestinationAddress());
            assertEquals(proto, ip.getProtocol());
            assertEquals(dscp0, ip.getDscp());
            assertEquals(src1, ip1.getSourceAddress());
            assertEquals(dst1, ip1.getDestinationAddress());
            assertEquals(proto, ip1.getProtocol());
            assertEquals(dscp1, ip1.getDscp());
        }
    }

    /**
     * Test case for {@link Inet4Packet#createMatch(Set)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateMatch() throws Exception {
        byte[] srcAddr = {(byte)10, (byte)1, (byte)2, (byte)3};
        byte[] dstAddr = {(byte)192, (byte)168, (byte)111, (byte)222};
        Ip4Network src = new Ip4Network(srcAddr);
        Ip4Network dst = new Ip4Network(dstAddr);
        Short dscp = Short.valueOf((short)0);
        Short proto = Short.valueOf((short)100);

        byte[] srcAddr1 = {(byte)100, (byte)200, (byte)33, (byte)44};
        byte[] dstAddr1 = {(byte)172, (byte)16, (byte)123, (byte)234};
        Ip4Network src1 = new Ip4Network(srcAddr1);
        Ip4Network dst1 = new Ip4Network(dstAddr1);
        Short dscp1 = Short.valueOf((short)63);

        IPv4 pkt = createIPv4Packet(src, dst, proto.shortValue(),
                                    dscp.shortValue());
        Inet4Packet ipv4 = new Inet4Packet(pkt);

        Set<FlowMatchType> fields = EnumSet.noneOf(FlowMatchType.class);
        VTNInet4Match expected = new VTNInet4Match();
        assertEquals(expected, ipv4.createMatch(fields));

        fields = EnumSet.of(FlowMatchType.IP_SRC);
        expected = new VTNInet4Match(src, null, null, null);
        assertEquals(expected, ipv4.createMatch(fields));

        fields = EnumSet.of(FlowMatchType.IP_DST);
        expected = new VTNInet4Match(null, dst, null, null);
        assertEquals(expected, ipv4.createMatch(fields));

        fields = EnumSet.of(FlowMatchType.IP_PROTO);
        expected = new VTNInet4Match(null, null, proto, null);
        assertEquals(expected, ipv4.createMatch(fields));

        fields = EnumSet.of(FlowMatchType.IP_DSCP);
        expected = new VTNInet4Match(null, null, null, dscp);
        assertEquals(expected, ipv4.createMatch(fields));

        // createMatch() always has to see the original.
        ipv4.setSourceAddress(src1);
        ipv4.setDestinationAddress(dst1);
        ipv4.setDscp(dscp1.shortValue());
        fields = EnumSet.of(FlowMatchType.IP_SRC, FlowMatchType.IP_DST,
                            FlowMatchType.IP_PROTO, FlowMatchType.IP_DSCP);
        expected = new VTNInet4Match(src, dst, proto, dscp);
        assertEquals(expected, ipv4.createMatch(fields));
    }

    /**
     * Test case for {@link Inet4Packet#getHeaderForChecksum(byte, short)}.
     */
    @Test
    public void testGetHeaderForChecksum() {
        byte[] srcAddr = {(byte)100, (byte)200, (byte)33, (byte)44};
        byte[] dstAddr = {(byte)172, (byte)16, (byte)123, (byte)234};
        Ip4Network src = new Ip4Network(srcAddr);
        Ip4Network dst = new Ip4Network(dstAddr);
        short protocol = 100;
        short len = 60;
        IPv4 pkt = createIPv4Packet(src, dst, protocol, (byte)0);
        Inet4Packet ipv4 = new Inet4Packet(pkt);
        byte[] header = ipv4.getHeaderForChecksum((byte)protocol, len);
        verifyChecksumHeader(header, srcAddr, dstAddr, (byte)protocol, len);

        Random rand = new Random();

        for (int i = 0; i < 30; i++) {
            srcAddr = NumberUtils.toBytes(rand.nextInt());
            dstAddr = NumberUtils.toBytes(rand.nextInt());
            src = new Ip4Network(srcAddr);
            dst = new Ip4Network(dstAddr);
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
    private IPv4 createIPv4Packet(Ip4Network src, Ip4Network dst, short proto,
                                  short dscp) {
        IPv4 pkt = createIPv4(src.getInetAddress(), dst.getInetAddress(),
                              proto, (byte)dscp);
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
