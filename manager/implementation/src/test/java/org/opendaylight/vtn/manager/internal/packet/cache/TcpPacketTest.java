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
import org.opendaylight.vtn.manager.packet.TCP;
import org.opendaylight.vtn.manager.util.InetProtocols;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNPortRange;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNTcpMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.vnode.PacketContext;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link TcpPacket}.
 */
public class TcpPacketTest extends TestBase {
    /**
     * The flag bit which indicates the source port number.
     */
    private static final int  TCP_SRC = 0x1;

    /**
     * The flag bit which indicates the destination port number.
     */
    private static final int  TCP_DST = 0x2;

    /**
     * The flag bits which indicates all modifyable fields.
     */
    private static final int  TCP_ALL = (TCP_SRC | TCP_DST);

    /**
     * Sequence number.
     */
    private int  sequenceNumber = 3000;

    /**
     * Acknowledgement number.
     */
    private int  ackNumber = 4096;

    /**
     * Data offset.
     */
    private byte  dataOffset = 1;

    /**
     * Reserved field.
     */
    private byte  reserved = -1;

    /**
     * TCP flags.
     */
    private short tcpFlags;

    /**
     * Window size.
     */
    private short  windowSize = 0x777;

    /**
     * Checksum.
     */
    private short  checksum = 0x1234;

    /**
     * Urgent pointer.
     */
    private short  urgentPointer = (short)-43210;

    /**
     * Test case for getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        TxContext ctx = mock(TxContext.class);
        SalPort ingress = new SalPort(1L, 3L);

        int[] srcs = {0, 1, 53, 123, 12000, 23567, 45900, 58123, 60000, 65535};
        int[] dsts = {0, 2, 35, 99, 390, 14567, 39734, 47614, 59198, 65535};
        for (int src: srcs) {
            for (int dst: dsts) {
                TCP pkt = createTCP(src, dst);
                TcpPacket tcp = new TcpPacket(pkt);
                assertEquals(src, tcp.getSourcePort());
                assertEquals(dst, tcp.getDestinationPort());

                // commit() should return false.
                Ethernet ether = createEthernet(pkt);
                PacketContext pctx = new PacketContext(ctx, ether, ingress);
                assertFalse(tcp.commit(pctx));
                assertEquals(null, pctx.getFilterActions());
                for (FlowMatchType mtype: FlowMatchType.values()) {
                    assertFalse(pctx.hasMatchField(mtype));
                }
            }
        }

        verifyZeroInteractions(ctx);
    }

    /**
     * Test case for setter methods and {@link TcpPacket#clone()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetter() throws Exception {
        TxContext ctx = mock(TxContext.class);
        SalPort ingress = new SalPort(1L, 3L);

        int src0 = 1;
        int dst0 = 80;
        int src1 = 52984;
        int dst1 = 9999;
        int src2 = 413;
        int dst2 = 60345;

        Map<Class<? extends FlowFilterAction>, FlowFilterAction> fltActions =
            new LinkedHashMap<>();
        fltActions.put(VTNSetPortSrcAction.class,
                       new VTNSetPortSrcAction(src2));
        fltActions.put(VTNSetPortDstAction.class,
                       new VTNSetPortDstAction(dst2));

        for (int flags = TCP_SRC; flags <= TCP_ALL; flags++) {
            TCP pkt = createTCP(src0, dst0);
            TCP original = copy(pkt, new TCP());
            TcpPacket tcp = new TcpPacket(pkt);

            int src = src0;
            int dst = dst0;
            Ethernet ether = createEthernet(pkt);
            PacketContext pctx = new PacketContext(ctx, ether, ingress);
            for (FlowFilterAction act: fltActions.values()) {
                pctx.addFilterAction(act);
            }
            TcpPacket tcp1 = (TcpPacket)tcp.clone();
            TCP pkt2 = copy(original, new TCP());
            if ((flags & TCP_SRC) != 0) {
                // Modify source port number.
                tcp1.setSourcePort(src1);
                pkt2.setSourcePort((short)src1);
                src = src1;
            }
            if ((flags & TCP_DST) != 0) {
                // Modify destination port number.
                tcp1.setDestinationPort(dst1);
                pkt2.setDestinationPort((short)dst1);
                dst = dst1;
            }

            assertEquals(src, tcp1.getSourcePort());
            assertEquals(dst, tcp1.getDestinationPort());

            // The packet should not be modified until commit() is called.
            assertSame(pkt, tcp1.getPacket());
            assertEquals(original, pkt);

            assertEquals(true, tcp1.commit(pctx));
            TCP newPkt = tcp1.getPacket();
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
            if ((flags & TCP_SRC) != 0) {
                actions.add(fltActions.get(VTNSetPortSrcAction.class));
            }
            if ((flags & TCP_DST) != 0) {
                actions.add(fltActions.get(VTNSetPortDstAction.class));
            }

            pctx = new PacketContext(ctx, ether, ingress);
            for (FlowMatchType mt: FlowMatchType.values()) {
                pctx.addMatchField(mt);
            }
            for (FlowFilterAction act: fltActions.values()) {
                pctx.addFilterAction(act);
            }
            assertEquals(true, tcp1.commit(pctx));
            assertSame(newPkt, tcp1.getPacket());
            filterActions = new ArrayList<>(pctx.getFilterActions());
            assertEquals(actions, filterActions);

            // The original packet should not be affected.
            assertEquals(src0, tcp.getSourcePort());
            assertEquals(dst0, tcp.getDestinationPort());
            assertSame(pkt, tcp.getPacket());
            assertEquals(original, pkt);

            // Set values in the original packet.
            tcp1.setSourcePort(src0);
            tcp1.setDestinationPort(dst0);
            assertEquals(false, tcp1.commit(pctx));
            assertEquals(src0, tcp1.getSourcePort());
            assertEquals(dst0, tcp1.getDestinationPort());

            // Ensure that a set of modified values is deeply cloned.
            TcpPacket tcp2 = (TcpPacket)tcp1.clone();
            assertNotSame(tcp1, tcp2);
            assertEquals(src0, tcp1.getSourcePort());
            assertEquals(dst0, tcp1.getDestinationPort());
            assertEquals(src0, tcp2.getSourcePort());
            assertEquals(dst0, tcp2.getDestinationPort());
            tcp2.setSourcePort(src1);
            tcp2.setDestinationPort(dst1);
            assertEquals(src0, tcp1.getSourcePort());
            assertEquals(dst0, tcp1.getDestinationPort());
            assertEquals(src1, tcp2.getSourcePort());
            assertEquals(dst1, tcp2.getDestinationPort());
        }

        verifyZeroInteractions(ctx);
    }

    /**
     * Test case for {@link TcpPacket#createMatch(Set)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateMatch() throws Exception {
        int src = 12345;
        int dst = 65432;
        VTNPortRange sr = new VTNPortRange(src);
        VTNPortRange dr = new VTNPortRange(dst);

        int src1 = 34012;
        int dst1 = 25;
        TCP pkt = createTCP(src, dst);
        TcpPacket tcp = new TcpPacket(pkt);

        Set<FlowMatchType> fields = EnumSet.noneOf(FlowMatchType.class);
        VTNTcpMatch expected = new VTNTcpMatch();
        assertEquals(expected, tcp.createMatch(fields));

        fields = EnumSet.of(FlowMatchType.TCP_SRC);
        expected = new VTNTcpMatch(sr, null);
        assertEquals(expected, tcp.createMatch(fields));

        fields = EnumSet.of(FlowMatchType.TCP_DST);
        expected = new VTNTcpMatch(null, dr);
        assertEquals(expected, tcp.createMatch(fields));

        // createMatch() always has to see the original.
        tcp.setSourcePort(src1);
        tcp.setDestinationPort(dst1);
        fields = EnumSet.of(FlowMatchType.TCP_SRC, FlowMatchType.TCP_DST);
        expected = new VTNTcpMatch(sr, dr);
        assertEquals(expected, tcp.createMatch(fields));
    }

    /**
     * Test case for {@link TcpPacket#updateChecksum(Inet4Packet)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateChecksum() throws Exception {
        TxContext ctx = mock(TxContext.class);
        SalPort ingress = new SalPort(1L, 3L);

        // Create a broken TCP packet.
        TCP pkt = new TCP();
        @SuppressWarnings("unchecked")
        Map<String, byte[]> header = (Map<String, byte[]>)getFieldValue(
            pkt, Packet.class, Map.class, "hdrFieldsMap");
        header.put("WindowSize", new byte[]{0});

        // Ensure that an exception is wrapped by a VTNException.
        IPv4 ipv4 = new IPv4();
        Inet4Packet inet4 = new Inet4Packet(ipv4);
        TcpPacket tcp = new TcpPacket(pkt);

        try {
            tcp.updateChecksum(inet4);
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

        // Fixed TCP parameters.
        int seq = 0xa2ffa3fc;
        int ack = 0x7261bf41;
        byte off = 5;
        byte resv = 0;
        short flags = 0x18;
        short win = 16384;
        short urp = 6585;
        short checksumIni = (short)0xaaaa;
        byte[] empty = new byte[0];

        // TCP payload: even-numbered size.
        byte[] even = {
            (byte)0xef, (byte)0x49, (byte)0x7b, (byte)0xc3,
            (byte)0xb6, (byte)0x61, (byte)0x14, (byte)0x2b,
            (byte)0xff, (byte)0x2f, (byte)0x67, (byte)0xa9,
            (byte)0x48, (byte)0x5f, (byte)0xdb, (byte)0x8e,
            (byte)0x70, (byte)0xdc, (byte)0x0e, (byte)0xe2,
            (byte)0xe4, (byte)0xee, (byte)0xfc, (byte)0x47,
            (byte)0xad, (byte)0x69, (byte)0xcd, (byte)0x5c,
            (byte)0x2d, (byte)0x00, (byte)0x42, (byte)0xe6,
            (byte)0x15, (byte)0x2c, (byte)0x77, (byte)0xce,
            (byte)0x1b, (byte)0x0e, (byte)0xf5, (byte)0xec,
            (byte)0x0e, (byte)0xb5, (byte)0xab, (byte)0xd2,
            (byte)0x59, (byte)0xbd, (byte)0x44, (byte)0x0c,
            (byte)0xd5, (byte)0x49, (byte)0x8c, (byte)0xbd,
            (byte)0x8c, (byte)0x66, (byte)0xca, (byte)0x53,
            (byte)0x5b, (byte)0x8e, (byte)0xc0, (byte)0xe8,
            (byte)0x5e, (byte)0x9e, (byte)0x53, (byte)0x75,
        };

        // TCP payload: odd-numbered size.
        byte[] odd = {
            (byte)0x46, (byte)0xb6, (byte)0x09, (byte)0x4f,
            (byte)0x3b, (byte)0xae, (byte)0x15, (byte)0x5c,
            (byte)0xa2, (byte)0x26, (byte)0x13, (byte)0x4f,
            (byte)0x02, (byte)0xac, (byte)0x5b, (byte)0x1d,
            (byte)0x7d, (byte)0x7f, (byte)0x30, (byte)0x89,
            (byte)0x19, (byte)0xf9, (byte)0x58, (byte)0xa7,
            (byte)0xdd, (byte)0xc2, (byte)0xa3, (byte)0xdb,
            (byte)0x0f, (byte)0xed, (byte)0xe5,
        };

        // TCP payload: Large payload that causes so many checksum overflow.
        byte[] large = new byte[2048];
        Arrays.fill(large, (byte)0xff);

        Ip4Network srcIp = new Ip4Network("10.123.89.3");
        Ip4Network dstIp = new Ip4Network("192.168.35.11");
        int src = 37397;
        int dst = 9999;

        pkt.setSourcePort((short)src).setDestinationPort((short)dst).
            setSequenceNumber(seq).setAckNumber(ack).setDataOffset(off).
            setHeaderLenFlags(flags).setReserved(resv).setWindowSize(win).
            setChecksum(checksumIni).setUrgentPointer(urp);
        ipv4 = createIPv4(srcIp, dstIp, pkt);
        tcp = new TcpPacket(pkt);
        inet4 = new Inet4Packet(ipv4);

        List<ChecksumData> list = new ArrayList<ChecksumData>();
        list.add(new ChecksumData(empty, 0xdd1d));
        list.add(new ChecksumData(even, 0x5346));
        list.add(new ChecksumData(odd, 0x917a));
        list.add(new ChecksumData(large, 0xd51d));
        verifyChecksum(list, tcp, inet4);

        // Change TCP port number.
        pkt.setRawPayload(empty);
        src = 18345;
        dst = 53;
        tcp.setSourcePort(src);
        tcp.setDestinationPort(dst);
        Ethernet ether = createEthernet(pkt);
        PacketContext pctx = new PacketContext(ctx, ether, ingress);
        assertEquals(true, tcp.commit(pctx));

        list.clear();
        list.add(new ChecksumData(empty, 0x4e64));
        list.add(new ChecksumData(even, 0xc48c));
        list.add(new ChecksumData(odd, 0x02c1));
        list.add(new ChecksumData(large, 0x4664));
        verifyChecksum(list, tcp, inet4);

        // Change IP addresses.
        pkt.setRawPayload(empty);
        srcIp = new Ip4Network("212.39.43.254");
        dstIp = new Ip4Network("127.0.0.1");
        inet4.setSourceAddress(srcIp);
        assertTrue(inet4.isAddressModified());
        inet4.setDestinationAddress(dstIp);
        assertTrue(inet4.isAddressModified());

        list.clear();
        list.add(new ChecksumData(empty, 0x166f));
        list.add(new ChecksumData(even, 0x8c97));
        list.add(new ChecksumData(odd, 0xcacb));
        list.add(new ChecksumData(large, 0x0e6f));
        verifyChecksum(list, tcp, inet4);

        verifyZeroInteractions(ctx);
    }

    /**
     * Create a {@link TCP} instance for test.
     *
     * @param src  Source port number.
     * @param dst  Destination port number.
     * @return  A {@link TCP} instance.
     */
    private TCP createTCP(int src, int dst) {
        TCP pkt = new TCP();
        sequenceNumber++;
        ackNumber++;
        dataOffset++;
        reserved = (byte)((reserved + 1) & 0x3);
        tcpFlags = (short)((tcpFlags + 1) & 0x1ff);
        windowSize++;
        checksum++;
        urgentPointer++;

        return pkt.setSourcePort((short)src).setDestinationPort((short)dst).
            setSequenceNumber(sequenceNumber).setAckNumber(ackNumber).
            setDataOffset(dataOffset).setHeaderLenFlags(tcpFlags).
            setReserved(reserved).setWindowSize(windowSize).
            setChecksum(checksum).setUrgentPointer(urgentPointer);
    }

    /**
     * Create an {@link IPv4} instance that contains the given TCP packet.
     *
     * @param src  Source IP address.
     * @param dst  Destination IP address.
     * @param tcp  A {@link TCP} instance.
     * @return  An {@link IPv4} instance.
     */
    private IPv4 createIPv4(Ip4Network src, Ip4Network dst, TCP tcp) {
        return createIPv4(src, dst, InetProtocols.TCP.shortValue(), (byte)0,
                          tcp);
    }

    /**
     * Create an {@link Ethernet} instance that contains the given TCP packet.
     *
     * @param tcp  A {@link TCP} instance.
     * @return  An {@link Ethernet} instance.
     */
    private Ethernet createEthernet(TCP tcp) {
        IPv4 ipv4 = createIPv4(InetProtocols.TCP.shortValue(), tcp);
        return createEthernet(ipv4);
    }

    /**
     * Ensure that {@link TcpPacket#updateChecksum(Inet4Packet)} updates
     * the checksum field correctly.
     *
     * @param list   A list of {@link ChecksumData} instances.
     * @param tcp    A {@link TcpPacket} instance.
     * @param inet4  An {@link Inet4Packet} instance.
     * @throws Exception  An error occurred.
     */
    private void verifyChecksum(List<ChecksumData> list, TcpPacket tcp,
                                Inet4Packet inet4) throws Exception {
        TCP pkt = tcp.getPacket();

        for (ChecksumData data: list) {
            short expected = data.getChecksum();
            byte[] payload = data.getPayload();

            pkt.setRawPayload(payload);
            TCP original = copy(pkt, new TCP());

            assertTrue(tcp.updateChecksum(inet4));
            assertEquals(expected, pkt.getChecksum());
            assertTrue(PortProtoPacket.verifyChecksum(inet4, pkt));

            // Ensure that other fields are retained.
            original.setChecksum(expected);
            assertEquals(original, pkt);

            // updateChecksum() should return false if the checksum was not
            // updated.
            assertFalse(tcp.updateChecksum(inet4));
        }
    }
}

/**
 * Checksum test information.
 */
final class ChecksumData {
    /**
     * A payload to be configured.
     */
    private final byte[]  payload;

    /**
     * An expected checksum.
     */
    private final short  checksum;

    /**
     * Construct a new instance with specifying description.
     *
     * @param data  A byte array to be configured as payload.
     * @param sum   An expected checksum.
     */
    ChecksumData(byte[] data, int sum) {
        payload = data;
        checksum = (short)sum;
    }

    /**
     * Return a byte array to be configured as payload.
     *
     * @return  A byte array.
     */
    byte[] getPayload() {
        return payload;
    }

    /**
     * Return an expecetd checksum.
     *
     * @return  An expected checksum.
     */
    short getChecksum() {
        return checksum;
    }
}
