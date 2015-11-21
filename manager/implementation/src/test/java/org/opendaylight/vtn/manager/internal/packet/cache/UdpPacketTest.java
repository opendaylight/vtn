/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verifyZeroInteractions;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.packet.IPv4;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.packet.PacketException;
import org.opendaylight.vtn.manager.packet.UDP;
import org.opendaylight.vtn.manager.util.InetProtocols;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNPortRange;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNUdpMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.vnode.PacketContext;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link UdpPacket}.
 */
public class UdpPacketTest extends TestBase {
    /**
     * The size of the UDP header.
     */
    private static final int  UDP_HEADER_SIZE = 8;

    /**
     * The flag bit which indicates the source port number.
     */
    private static final int  UDP_SRC = 0x1;

    /**
     * The flag bit which indicates the destination port number.
     */
    private static final int  UDP_DST = 0x2;

    /**
     * The flag bits which indicates all modifyable fields.
     */
    private static final int  UDP_ALL = (UDP_SRC | UDP_DST);

    /**
     * Test case for getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        TxContext ctx = mock(TxContext.class);
        SalPort ingress = new SalPort(1L, 3L);

        int[] srcs = {0, 1, 53, 123, 12333, 23567, 45699, 58123, 60000, 65535};
        int[] dsts = {0, 2, 35, 99, 390, 14567, 39734, 47614, 59198, 65535};
        for (int src: srcs) {
            for (int dst: dsts) {
                UDP pkt = createUDP(src, dst);
                UdpPacket udp = new UdpPacket(pkt);
                assertEquals(src, udp.getSourcePort());
                assertEquals(dst, udp.getDestinationPort());

                // commit() should return false.
                Ethernet ether = createEthernet(pkt);
                PacketContext pctx = new PacketContext(ctx, ether, ingress);
                assertFalse(udp.commit(pctx));
                assertEquals(null, pctx.getFilterActions());
                for (FlowMatchType mtype: FlowMatchType.values()) {
                    assertFalse(pctx.hasMatchField(mtype));
                }
            }
        }

        verifyZeroInteractions(ctx);
    }

    /**
     * Test case for setter methods and {@link UdpPacket#clone()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetter() throws Exception {
        TxContext ctx = mock(TxContext.class);
        SalPort ingress = new SalPort(1L, 3L);

        int src0 = 2;
        int dst0 = 2800;
        int src1 = 48321;
        int dst1 = 8888;
        int src2 = 64531;
        int dst2 = 3271;

        Map<Class<? extends FlowFilterAction>, FlowFilterAction> fltActions =
            new LinkedHashMap<>();
        fltActions.put(VTNSetPortSrcAction.class,
                       new VTNSetPortSrcAction(src2));
        fltActions.put(VTNSetPortDstAction.class,
                       new VTNSetPortDstAction(dst2));

        for (int flags = UDP_SRC; flags <= UDP_ALL; flags++) {
            UDP pkt = createUDP(src0, dst0);
            UDP original = copy(pkt, new UDP());
            UdpPacket udp = new UdpPacket(pkt);

            int src = src0;
            int dst = dst0;
            Ethernet ether = createEthernet(pkt);
            PacketContext pctx = new PacketContext(ctx, ether, ingress);
            for (FlowFilterAction act: fltActions.values()) {
                pctx.addFilterAction(act);
            }
            UdpPacket udp1 = (UdpPacket)udp.clone();
            UDP pkt2 = copy(original, new UDP());
            if ((flags & UDP_SRC) != 0) {
                // Modify source port number.
                udp1.setSourcePort(src1);
                pkt2.setSourcePort((short)src1);
                src = src1;
            }
            if ((flags & UDP_DST) != 0) {
                // Modify destination port number.
                udp1.setDestinationPort(dst1);
                pkt2.setDestinationPort((short)dst1);
                dst = dst1;
            }

            assertEquals(src, udp1.getSourcePort());
            assertEquals(dst, udp1.getDestinationPort());

            // The packet should not be modified until commit() is called.
            assertSame(pkt, udp1.getPacket());
            assertEquals(original, pkt);

            assertEquals(true, udp1.commit(pctx));
            UDP newPkt = udp1.getPacket();
            assertNotSame(pkt, newPkt);
            assertEquals((short)src, newPkt.getSourcePort());
            assertEquals((short)dst, newPkt.getDestinationPort());
            assertEquals(pkt2, newPkt);

            assertEquals((short)src0, pkt.getSourcePort());
            assertEquals((short)dst0, pkt.getDestinationPort());
            assertEquals(original, pkt);

            assertTrue(pctx.hasMatchField(FlowMatchType.DL_TYPE));
            assertTrue(pctx.hasMatchField(FlowMatchType.IP_PROTO));

            List<FlowFilterAction> filterActions =
                new ArrayList<>(pctx.getFilterActions());
            assertEquals(new ArrayList<FlowFilterAction>(fltActions.values()),
                         filterActions);

            // Actions for unchanged field will be removed if corresponding
            // match type is configured in PacketContext.
            List<FlowFilterAction> actions = new ArrayList<>();
            if ((flags & UDP_SRC) != 0) {
                actions.add(fltActions.get(VTNSetPortSrcAction.class));
            }
            if ((flags & UDP_DST) != 0) {
                actions.add(fltActions.get(VTNSetPortDstAction.class));
            }

            pctx = new PacketContext(ctx, ether, ingress);
            for (FlowMatchType mt: FlowMatchType.values()) {
                pctx.addMatchField(mt);
            }
            for (FlowFilterAction act: fltActions.values()) {
                pctx.addFilterAction(act);
            }
            assertEquals(true, udp1.commit(pctx));
            assertSame(newPkt, udp1.getPacket());
            filterActions = new ArrayList<>(pctx.getFilterActions());
            assertEquals(actions, filterActions);

            // The original packet should not be affected.
            assertEquals(src0, udp.getSourcePort());
            assertEquals(dst0, udp.getDestinationPort());
            assertSame(pkt, udp.getPacket());
            assertEquals(original, pkt);

            // Set values in the original packet.
            udp1.setSourcePort(src0);
            udp1.setDestinationPort(dst0);
            assertEquals(false, udp1.commit(pctx));
            assertEquals(src0, udp1.getSourcePort());
            assertEquals(dst0, udp1.getDestinationPort());

            // Ensure that a set of modified values is deeply cloned.
            UdpPacket udp2 = (UdpPacket)udp1.clone();
            assertNotSame(udp1, udp2);
            assertEquals(src0, udp1.getSourcePort());
            assertEquals(dst0, udp1.getDestinationPort());
            assertEquals(src0, udp2.getSourcePort());
            assertEquals(dst0, udp2.getDestinationPort());
            udp2.setSourcePort(src1);
            udp2.setDestinationPort(dst1);
            assertEquals(src0, udp1.getSourcePort());
            assertEquals(dst0, udp1.getDestinationPort());
            assertEquals(src1, udp2.getSourcePort());
            assertEquals(dst1, udp2.getDestinationPort());
        }

        verifyZeroInteractions(ctx);
    }

    /**
     * Test case for {@link UdpPacket#createMatch(Set)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateMatch() throws Exception {
        int src = 34567;
        int dst = 4321;
        VTNPortRange sr = new VTNPortRange(src);
        VTNPortRange dr = new VTNPortRange(dst);

        int src1 = 43982;
        int dst1 = 3178;
        UDP pkt = createUDP(src, dst);
        UdpPacket udp = new UdpPacket(pkt);

        Set<FlowMatchType> fields = EnumSet.noneOf(FlowMatchType.class);
        VTNUdpMatch expected = new VTNUdpMatch();
        assertEquals(expected, udp.createMatch(fields));

        fields = EnumSet.of(FlowMatchType.UDP_SRC);
        expected = new VTNUdpMatch(sr, null);
        assertEquals(expected, udp.createMatch(fields));

        fields = EnumSet.of(FlowMatchType.UDP_DST);
        expected = new VTNUdpMatch(null, dr);
        assertEquals(expected, udp.createMatch(fields));

        // createMatch() always has to see the original.
        udp.setSourcePort(src1);
        udp.setDestinationPort(dst1);
        fields = EnumSet.of(FlowMatchType.UDP_SRC, FlowMatchType.UDP_DST);
        expected = new VTNUdpMatch(sr, dr);
        assertEquals(expected, udp.createMatch(fields));
    }

    /**
     * Test case for {@link UdpPacket#updateChecksum(Inet4Packet)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateChecksum() throws Exception {
        TxContext ctx = mock(TxContext.class);
        SalPort ingress = new SalPort(1L, 3L);

        // Create a broken UDP packet.
        UDP pkt = new UDP();
        Map<String, byte[]> header = getFieldValue(
            pkt, Packet.class, Map.class, "hdrFieldsMap");
        header.put("Length", new byte[]{0});
        pkt.setChecksum((short)1);

        // Ensure that an exception is wrapped by a VTNException.
        IPv4 ipv4 = new IPv4();
        Inet4Packet inet4 = new Inet4Packet(ipv4);
        UdpPacket udp = new UdpPacket(pkt);
        try {
            udp.updateChecksum(inet4);
            unexpected();
        } catch (VTNException e) {
            Throwable cause = e.getCause();
            assertNotNull(cause);
            assertEquals(PacketException.class, cause.getClass());
        }

        try {
            PortProtoPacket.verifyChecksum(inet4, pkt);
            unexpected();
        } catch (VTNException e) {
            Throwable cause = e.getCause();
            assertNotNull(cause);
            assertEquals(PacketException.class, cause.getClass());
        }

        // Fixed UDP parameters.
        short len = 0x1234;
        short checksumIni = (short)0xaaaa;

        byte[] empty = new byte[0];

        // UDP payloads: even-numbered size.
        byte[] even = {
            (byte)0xab, (byte)0x59, (byte)0x24, (byte)0x9f,
            (byte)0x27, (byte)0x0f, (byte)0xe9, (byte)0x28,
            (byte)0xab, (byte)0x77, (byte)0x8a, (byte)0x34,
            (byte)0xba, (byte)0x88, (byte)0x33, (byte)0x6e,
            (byte)0x0e, (byte)0x30, (byte)0x7a, (byte)0xef,
        };

        // UDP payloads: odd-numbered size.
        byte[] odd = {
            (byte)0xc3, (byte)0x13, (byte)0x12, (byte)0x54,
            (byte)0x68, (byte)0x70, (byte)0x9d, (byte)0x6d,
            (byte)0x79, (byte)0xc2, (byte)0xec, (byte)0xac,
            (byte)0x0f, (byte)0x87, (byte)0x56, (byte)0xfb,
            (byte)0x81, (byte)0x9d, (byte)0x41, (byte)0x83,
            (byte)0x9d, (byte)0x0a, (byte)0x9f, (byte)0x6e,
            (byte)0xf1,
        };

        // UDP payloads: Large size that causes so many checksum overflow.
        byte[] large = new byte[2048];
        Arrays.fill(large, (byte)0xff);

        byte[] srcIp = {(byte)172, (byte)16, (byte)253, (byte)150};
        byte[] dstIp = {(byte)192, (byte)168, (byte)3, (byte)200};
        int src = 54321;
        int dst = 7777;

        pkt = new UDP();
        pkt.setSourcePort((short)src).setDestinationPort((short)dst).
            setChecksum(checksumIni).setLength(len);
        ipv4 = createIPv4(srcIp, dstIp, pkt);
        udp = new UdpPacket(pkt);
        inet4 = new Inet4Packet(ipv4);

        List<ChecksumData> list = new ArrayList<ChecksumData>();
        list.add(new ChecksumData(empty, 0x8d07));

        // computeChecksum() should return zero.
        list.add(new ChecksumData(even, 0xffff));

        list.add(new ChecksumData(odd, 0xf41b));
        list.add(new ChecksumData(large, 0x8507));
        verifyChecksum(list, udp, inet4);
        verifyChecksumDisabled(udp, inet4, empty, even, odd, large);

        // Change UDP port number.
        pkt.setRawPayload(empty);
        src = 48173;
        dst = 871;
        udp.setSourcePort(src);
        udp.setDestinationPort(dst);
        Ethernet ether = createEthernet(pkt);
        PacketContext pctx = new PacketContext(ctx, ether, ingress);
        assertEquals(true, udp.commit(pctx));

        list.clear();
        list.add(new ChecksumData(empty, 0xc005));
        list.add(new ChecksumData(even, 0x32fe));
        list.add(new ChecksumData(odd, 0x271a));
        list.add(new ChecksumData(large, 0xb805));
        pkt.setChecksum(checksumIni);
        verifyChecksum(list, udp, inet4);
        verifyChecksumDisabled(udp, inet4, empty, even, odd, large);

        // Change IP addresses.
        pkt.setRawPayload(empty);
        srcIp = new byte[]{(byte)123, (byte)89, (byte)118, (byte)91};
        dstIp = new byte[]{(byte)100, (byte)200, (byte)73, (byte)183};
        inet4.setSourceAddress(new Ip4Network(srcIp));
        assertTrue(inet4.isAddressModified());
        inet4.setDestinationAddress(new Ip4Network(dstIp));
        assertTrue(inet4.isAddressModified());

        list.clear();
        list.add(new ChecksumData(empty, 0x8de9));
        list.add(new ChecksumData(even, 0x00e2));
        list.add(new ChecksumData(odd, 0xf4fd));
        list.add(new ChecksumData(large, 0x85e9));
        pkt.setChecksum(checksumIni);
        verifyChecksum(list, udp, inet4);
        verifyChecksumDisabled(udp, inet4, empty, even, odd, large);

        verifyZeroInteractions(ctx);
    }

    /**
     * Create an {@link UDP} instance for test.
     *
     * @param src  Source port number.
     * @param dst  Destination port number.
     * @return  An {@link UDP} instance.
     */
    private UDP createUDP(int src, int dst) {
        return createUDP(src, dst, null);
    }

    /**
     * Create an {@link UDP} instance for test.
     *
     * @param src  Source port number.
     * @param dst  Destination port number.
     * @param raw  A byte array to be configured as payload.
     * @return  An {@link UDP} instance.
     */
    private UDP createUDP(int src, int dst, byte[] raw) {
        UDP pkt = new UDP();
        short len = UDP_HEADER_SIZE;
        pkt.setSourcePort((short)src).setDestinationPort((short)dst);
        if (raw != null) {
            pkt.setRawPayload(raw);
            len += raw.length;
        }

        return pkt.setLength(len);
    }

    /**
     * Create an {@link IPv4} instance that contains the given UDP packet.
     *
     * @param src  Source IP address.
     * @param dst  Destination IP address.
     * @param udp  A {@link UDP} instance.
     * @return  An {@link IPv4} instance.
     */
    private IPv4 createIPv4(byte[] src, byte[] dst, UDP udp) {
        return createIPv4(src, dst, InetProtocols.UDP.shortValue(), (byte)0,
                          udp);
    }

    /**
     * Create an {@link Ethernet} instance that contains the given UDP packet.
     *
     * @param udp  A {@link UDP} instance.
     * @return  An {@link Ethernet} instance.
     */
    private Ethernet createEthernet(UDP udp) {
        IPv4 ipv4 = createIPv4(InetProtocols.UDP.shortValue(), udp);
        return createEthernet(ipv4);
    }

    /**
     * Ensure that {@link UdpPacket#updateChecksum(Inet4Packet)} updates
     * the checksum field correctly.
     *
     * @param list   A list of {@link ChecksumData} instances.
     * @param udp    A {@link UdpPacket} instance.
     * @param inet4  An {@link Inet4Packet} instance.
     * @throws Exception  An error occurred.
     */
    private void verifyChecksum(List<ChecksumData> list, UdpPacket udp,
                                Inet4Packet inet4) throws Exception {
        for (ChecksumData data: list) {
            short expected = data.getChecksum();
            byte[] payload = data.getPayload();

            UDP pkt = udp.getPacket();
            pkt.setRawPayload(payload);
            UDP original = copy(pkt, new UDP());

            assertTrue(udp.updateChecksum(inet4));
            assertEquals(expected, pkt.getChecksum());
            assertTrue(PortProtoPacket.verifyChecksum(inet4, pkt));

            // Ensure that other fields are retained.
            original.setChecksum(expected);
            assertEquals(original, pkt);

            // updateChecksum() should return false if the checksum was not
            // updated.
            assertFalse(udp.updateChecksum(inet4));
        }
    }

    /**
     * Ensure that the UDP checksum is disabled if the checksum in the packet
     * is zero.
     *
     * @param udp       A {@link UdpPacket} instance.
     * @param inet4     An {@link Inet4Packet} instance.
     * @param payloads  An array of payloads.
     * @throws Exception  An error occurred.
     */
    private void verifyChecksumDisabled(UdpPacket udp, Inet4Packet inet4,
                                        byte[] ... payloads) throws Exception {
        UDP pkt = udp.getPacket();
        short zero = 0;
        pkt.setChecksum(zero);

        for (byte[] payload: payloads) {
            pkt.setRawPayload(payload);
            UDP original = copy(pkt, new UDP());

            assertFalse(udp.updateChecksum(inet4));
            assertEquals(zero, pkt.getChecksum());

            // Ensure that other fields are retained.
            assertEquals(original, pkt);

            assertFalse(udp.updateChecksum(inet4));
        }
    }
}
