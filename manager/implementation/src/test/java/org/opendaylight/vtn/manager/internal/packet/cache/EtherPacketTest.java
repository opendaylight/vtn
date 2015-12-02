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
import java.util.EnumSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.packet.ARP;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.packet.IEEE8021Q;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpAction;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.vnode.PacketContext;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link EtherPacket}.
 */
public class EtherPacketTest extends TestBase {
    /**
     * The flag bit which indicates the source MAC address.
     */
    private static final int  ETH_SRC = 0x1;

    /**
     * The flag bit which indicates the destination MAC address.
     */
    private static final int  ETH_DST = 0x2;

    /**
     * The flag bit which indicates the VLAN ID.
     */
    private static final int  ETH_VLAN_VID = 0x4;

    /**
     * The flag bit which indicates the VLAN priority.
     */
    private static final int  ETH_VLAN_PCP = 0x8;


    /**
     * The flag bits which indicates all modifyable fields.
     */
    private static final int  ETH_ALL =
        (ETH_SRC | ETH_DST | ETH_VLAN_VID | ETH_VLAN_PCP);

    /**
     * A raw payload for test.
     */
    static final byte[] RAW_PAYLOAD = {
        (byte)0xaa, (byte)0xbb, (byte)0xcc, (byte)0xdd,
        (byte)0xee, (byte)0xff, (byte)0x01, (byte)0x23,
        (byte)0x45, (byte)0x67, (byte)0x89, (byte)0xab,
        (byte)0xcd, (byte)0xef, (byte)0x00, (byte)0x01,
    };

    /**
     * VLAN priority for test.
     */
    private short  vlanPcp;

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        TxContext ctx = mock(TxContext.class);
        SalPort ingress = new SalPort(1L, 3L);
        long[] octets = {0x00, 0x0a, 0x7f, 0x88, 0xff};
        int[] types = {0x0001, 0x0815, 0x9999};
        int[] vlans = {0, 1, 2, 4095};
        ARP arp = createArp();
        for (long srcOctet: octets) {
            EtherAddress src = new EtherAddress(0x001122334400L + srcOctet);
            for (long dstOctet: octets) {
                EtherAddress dst = new EtherAddress(0xf0f1f2f3f400L + dstOctet);
                for (int type: types) {
                    for (int vlan: vlans) {
                        // Create Ethernet frame using ARP packet.
                        Ethernet pkt = createEthernet(
                            src, dst, type, (short)vlan, arp);
                        EtherPacket ether = new EtherPacket(pkt);
                        assertEquals(src, ether.getSourceAddress());
                        assertEquals(dst, ether.getDestinationAddress());
                        assertEquals(type, ether.getEtherType());
                        assertEquals(vlan, ether.getOriginalVlan());
                        assertEquals(vlan, ether.getVlanId());
                        IEEE8021Q vlanTag;
                        if (vlan == EtherHeader.VLAN_NONE) {
                            assertTrue(ether.getVlanPriority() < 0);
                            vlanTag = null;
                        } else {
                            assertEquals(vlanPcp, ether.getVlanPriority());
                            vlanTag = new IEEE8021Q();
                            vlanTag.setCfi((byte)0).setPcp((byte)vlanPcp).
                                setVid((short)vlan).setEtherType((short)type);
                        }
                        assertEquals(vlanTag, ether.getVlanTag());
                        assertEquals(arp, ether.getPayload());
                        assertEquals(null, ether.getRawPayload());
                        assertSame(pkt, ether.getPacket());

                        // Create another Ethernet frame using raw packet.
                        pkt = createEthernet(src, dst, type, vlan, RAW_PAYLOAD);
                        ether = new EtherPacket(pkt);
                        assertEquals(src, ether.getSourceAddress());
                        assertEquals(dst, ether.getDestinationAddress());
                        assertEquals(type, ether.getEtherType());
                        assertEquals(vlan, ether.getOriginalVlan());
                        if (vlan == EtherHeader.VLAN_NONE) {
                            assertTrue(ether.getVlanPriority() < 0);
                            vlanTag = null;
                        } else {
                            assertEquals(vlanPcp, ether.getVlanPriority());
                            vlanTag = new IEEE8021Q();
                            vlanTag.setCfi((byte)0).setPcp((byte)vlanPcp).
                                setVid((short)vlan).setEtherType((short)type);
                        }
                        assertEquals(vlanTag, ether.getVlanTag());
                        assertEquals(null, ether.getPayload());
                        assertArrayEquals(RAW_PAYLOAD, ether.getRawPayload());
                        assertSame(pkt, ether.getPacket());

                        // commit() should return false.
                        PacketContext pctx =
                            new PacketContext(ctx, pkt, ingress);
                        assertFalse(ether.commit(pctx));
                        assertEquals(null, pctx.getFilterActions());

                        for (FlowMatchType mtype: FlowMatchType.values()) {
                            assertFalse(pctx.hasMatchField(mtype));
                        }
                    }
                }
            }
        }

        verifyZeroInteractions(ctx);
    }

    /**
     * Test case for setter methods and {@link EtherPacket#clone()}.
     */
    @Test
    public void testSetter() {
        TxContext ctx = mock(TxContext.class);
        SalPort ingress = new SalPort(1L, 3L);
        int type = 0x0806;
        int[] vids = {EtherHeader.VLAN_NONE, 1, 4095};
        int vid1 = 1000;

        EtherAddress src0 = new EtherAddress(0x102030405060L);
        EtherAddress dst0 = new EtherAddress(0xf0f1f2fafbfcL);
        EtherAddress src1 = new EtherAddress(0xf0f1f2f3f4f5L);
        EtherAddress dst1 = new EtherAddress(0xa0bcdef12345L);
        EtherAddress src2 = new EtherAddress(0xe0e1e2e3e4e5L);
        EtherAddress dst2 = new EtherAddress(0xb0cdef123456L);

        Map<Class<? extends FlowFilterAction>, FlowFilterAction> fltActions =
            new LinkedHashMap<>();
        fltActions.put(VTNSetDlSrcAction.class, new VTNSetDlSrcAction(src2));
        fltActions.put(VTNSetDlDstAction.class, new VTNSetDlDstAction(dst2));
        fltActions.put(VTNSetVlanPcpAction.class,
                       new VTNSetVlanPcpAction((short)7));

        byte[] spa = {(byte)10, (byte)20, (byte)30, (byte)40};
        byte[] tpa = {(byte)192, (byte)168, (byte)0, (byte)254};
        ARP arp = createArp();
        ARP anotherArp = createArp(src2.getBytes(), dst2.getBytes(), spa, tpa,
                                   ARP.REPLY);

        for (int flags = ETH_SRC; flags <= ETH_ALL; flags++) {
            for (int vid: vids) {
                for (int outVid: vids) {
                    Ethernet pkt = createEthernet(src0, dst0, type, vid, arp);
                    EtherPacket ether = new EtherPacket(pkt);

                    EtherAddress src = src0;
                    EtherAddress dst = dst0;
                    int vlan = vid;
                    short pcp = vlanPcp;
                    boolean setPcp = false;

                    boolean mod = false;
                    PacketContext pctx = new PacketContext(ctx, pkt, ingress);
                    for (FlowFilterAction act: fltActions.values()) {
                        pctx.addFilterAction(act);
                    }
                    EtherPacket ether1 = ether.clone();
                    assertNotSame(ether, ether1);

                    if ((flags & ETH_SRC) != 0) {
                        // Modify source address.
                        ether1.setSourceAddress(src1);
                        src = src1;
                        mod = true;
                    }
                    if ((flags & ETH_DST) != 0) {
                        // Modify destination address.
                        ether1.setDestinationAddress(dst1);
                        dst = dst1;
                        mod = true;
                    }
                    if ((flags & ETH_VLAN_VID) != 0) {
                        // Modify VLAN ID.
                        ether1.setVlanId(outVid);
                        vlan = outVid;
                    }
                    if ((flags & ETH_VLAN_PCP) != 0) {
                        // Modify VLAN priority.
                        pcp = (short)((pcp + 1) & 0x7);
                        ether1.setVlanPriority(pcp);
                        setPcp = true;
                        if (vlan != EtherHeader.VLAN_NONE) {
                            mod = true;
                        }
                    }
                    ether1.setPayload(anotherArp);
                    assertSame(anotherArp, ether1.getPayload());

                    assertEquals(src, ether1.getSourceAddress());
                    assertEquals(dst, ether1.getDestinationAddress());
                    assertEquals(type, ether1.getEtherType());
                    assertEquals(vlan, ether1.getVlanId());
                    assertEquals(vid, ether1.getOriginalVlan());
                    if (vid == EtherHeader.VLAN_NONE && !setPcp) {
                        assertTrue(ether1.getVlanPriority() < 0);
                    } else {
                        assertEquals(pcp, ether1.getVlanPriority());
                    }

                    // SET_VLAN_PCP action should be removed if a VLAN tag is
                    // not configured in outgoing packet.
                    List<FlowFilterAction> actions = new ArrayList<>();
                    for (FlowFilterAction act: fltActions.values()) {
                        if (!(vlan == EtherHeader.VLAN_NONE &&
                              VTNSetVlanPcpAction.class.isInstance(act))) {
                            actions.add(act);
                        }
                    }
                    assertEquals(mod, ether1.commit(pctx));

                    List<FlowFilterAction> filterActions =
                        new ArrayList<>(pctx.getFilterActions());
                    assertEquals(actions, filterActions);

                    // Actions for unchanged field will be removed if
                    // corresponding match type is configured in PacketContext.
                    actions.clear();
                    if ((flags & ETH_SRC) != 0) {
                        actions.add(fltActions.get(VTNSetDlSrcAction.class));
                    }
                    if ((flags & ETH_DST) != 0) {
                        actions.add(fltActions.get(VTNSetDlDstAction.class));
                    }
                    if (vlan != EtherHeader.VLAN_NONE &&
                        (flags & ETH_VLAN_PCP) != 0) {
                        actions.add(fltActions.get(VTNSetVlanPcpAction.class));
                    }
                    for (FlowMatchType mt: FlowMatchType.values()) {
                        pctx.addMatchField(mt);
                    }

                    assertEquals(mod, ether1.commit(pctx));
                    filterActions = new ArrayList<>(pctx.getFilterActions());
                    assertEquals(actions, filterActions);

                    // EtherPacket class should never modify the packet.
                    assertSame(pkt, ether1.getPacket());
                    assertArrayEquals(src0.getBytes(),
                                      pkt.getSourceMACAddress());
                    assertArrayEquals(dst0.getBytes(),
                                      pkt.getDestinationMACAddress());
                    if (vid == EtherHeader.VLAN_NONE) {
                        assertEquals((short)type, pkt.getEtherType());
                        assertSame(arp, pkt.getPayload());
                    } else {
                        assertEquals(EtherTypes.VLAN.shortValue(),
                                     pkt.getEtherType());
                        IEEE8021Q tag = (IEEE8021Q)pkt.getPayload();
                        assertEquals((short)vid, tag.getVid());
                        assertEquals((byte)vlanPcp, tag.getPcp());
                        assertSame(arp, tag.getPayload());
                    }

                    // The original packet should not be affected.
                    assertEquals(src0, ether.getSourceAddress());
                    assertEquals(dst0, ether.getDestinationAddress());
                    assertEquals(type, ether.getEtherType());
                    assertSame(arp, ether.getPayload());
                    if (vid == EtherHeader.VLAN_NONE) {
                        assertTrue(ether.getVlanPriority() < 0);
                    } else {
                        assertEquals(vlanPcp, ether.getVlanPriority());
                    }

                    // Set values in the original packet.
                    ether1.setSourceAddress(src0);
                    ether1.setDestinationAddress(dst0);
                    ether1.setVlanId(vid);
                    ether1.setVlanPriority(vlanPcp);
                    assertEquals(false, ether1.commit(pctx));
                    assertEquals(src0, ether1.getSourceAddress());
                    assertEquals(dst0, ether1.getDestinationAddress());
                    assertEquals(vid, ether1.getVlanId());
                    assertEquals(vlanPcp, ether1.getVlanPriority());
                    assertEquals(type, ether1.getEtherType());

                    // Ensure that a set of modified values is deeply cloned.
                    EtherPacket ether2 = ether1.clone();
                    assertEquals(src0, ether1.getSourceAddress());
                    assertEquals(dst0, ether1.getDestinationAddress());
                    assertEquals(vid, ether1.getVlanId());
                    assertEquals(vlanPcp, ether1.getVlanPriority());
                    assertEquals(type, ether1.getEtherType());
                    assertEquals(src0, ether2.getSourceAddress());
                    assertEquals(dst0, ether2.getDestinationAddress());
                    assertEquals(vid, ether2.getVlanId());
                    assertEquals(vlanPcp, ether2.getVlanPriority());
                    assertEquals(type, ether2.getEtherType());
                    ether2.setSourceAddress(src1);
                    ether2.setDestinationAddress(dst1);
                    ether2.setVlanId(vid1);
                    ether2.setVlanPriority(pcp);
                    assertEquals(src0, ether1.getSourceAddress());
                    assertEquals(dst0, ether1.getDestinationAddress());
                    assertEquals(vid, ether1.getVlanId());
                    assertEquals(vlanPcp, ether1.getVlanPriority());
                    assertEquals(type, ether1.getEtherType());
                    assertEquals(src1, ether2.getSourceAddress());
                    assertEquals(dst1, ether2.getDestinationAddress());
                    assertEquals(vid1, ether2.getVlanId());
                    assertEquals(pcp, ether2.getVlanPriority());
                    assertEquals(type, ether2.getEtherType());
                }
            }
        }

        verifyZeroInteractions(ctx);
    }

    /**
     * Test case for {@link EtherPacket#createMatch(Set)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateMatch() throws Exception {
        EtherAddress src = new EtherAddress(0x102030405060L);
        EtherAddress dst = new EtherAddress(0xf0f1f2fafbfcL);
        Integer type = 0x0806;
        Integer[] vids = {EtherHeader.VLAN_NONE, 1, 4095};

        EtherAddress src1 = new EtherAddress(0x00446688aaccL);
        EtherAddress dst1 = new EtherAddress(0xf033557799bbL);
        ARP arp = createArp();

        for (Integer vid: vids) {
            Ethernet pkt = createEthernet(src, dst, type.intValue(),
                                          vid.shortValue(), arp);
            EtherPacket ether = new EtherPacket(pkt);

            // VLAN ID must be configured even if the type set is empty.
            VTNEtherMatch expected =
                new VTNEtherMatch(null, null, null, vid, null);
            Set<FlowMatchType> fields = EnumSet.noneOf(FlowMatchType.class);
            assertEquals(expected, ether.createMatch(fields));
            fields = EnumSet.of(FlowMatchType.DL_VLAN);
            assertEquals(expected, ether.createMatch(fields));

            expected = new VTNEtherMatch(src, null, null, vid, null);
            fields = EnumSet.of(FlowMatchType.DL_SRC);
            assertEquals(expected, ether.createMatch(fields));

            expected = new VTNEtherMatch(null, dst, null, vid, null);
            fields = EnumSet.of(FlowMatchType.DL_DST);
            assertEquals(expected, ether.createMatch(fields));

            expected = new VTNEtherMatch(null, null, type, vid, null);
            fields = EnumSet.of(FlowMatchType.DL_TYPE);
            assertEquals(expected, ether.createMatch(fields));

            Short pcp = (vid.intValue() == EtherHeader.VLAN_NONE)
                ? null : Short.valueOf(vlanPcp);
            expected = new VTNEtherMatch(null, null, null, vid, pcp);
            fields = EnumSet.of(FlowMatchType.DL_VLAN_PCP);
            assertEquals(expected, ether.createMatch(fields));

            // createMatch() always has to see the original.
            short pri = (short)((vlanPcp + 1) & 0x7);
            ether.setSourceAddress(src1);
            ether.setDestinationAddress(dst1);
            ether.setVlanId(Integer.valueOf((vid.intValue() + 1) & 0xfff));
            ether.setVlanPriority(pri);
            fields = EnumSet.of(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                FlowMatchType.DL_TYPE, FlowMatchType.DL_VLAN,
                                FlowMatchType.DL_VLAN_PCP);
            expected = new VTNEtherMatch(src, dst, type, vid, pcp);
            assertEquals(expected, ether.createMatch(fields));
        }
    }

    /**
     * Create an {@link Ethernet} instance for test.
     *
     * @param src      Source MAC address.
     * @param dst      Destination MAC address.
     * @param type     Ethernet type.
     * @param vid      A VLAN ID.
     * @param payload  A {@link Packet} instance to be set as payload.
     * @return  An {@link Ethernet} instance.
     */
    private Ethernet createEthernet(EtherAddress src, EtherAddress dst,
                                    int type, int vid, Packet payload) {
        short pcp;
        if (vid == EtherHeader.VLAN_NONE) {
            pcp = 0;
        } else {
            pcp = (short)((vlanPcp + 1) & 7);
            vlanPcp = pcp;
        }

        return createEthernet(src, dst, type, (short)vid, (byte)pcp, payload);
    }

    /**
     * Create an {@link Ethernet} instance for test.
     *
     * @param src   Source MAC address.
     * @param dst   Destination MAC address.
     * @param type  Ethernet type.
     * @param vid   A VLAN ID.
     * @param raw   A byte array to be set as payload.
     * @return  An {@link Ethernet} instance.
     */
    private Ethernet createEthernet(EtherAddress src, EtherAddress dst,
                                    int type, int vid, byte[] raw) {
        short pcp;
        if (vid == EtherHeader.VLAN_NONE) {
            pcp = 0;
        } else {
            pcp = (short)((vlanPcp + 1) & 7);
            vlanPcp = pcp;
        }

        return createEthernet(src, dst, type, (short)vid, (byte)pcp, raw);
    }

    /**
     * Create an ARP packet for test.
     *
     * @return  An {@link ARP} instance.
     */
    private ARP createArp() {
        byte[] sha = {
            (byte)0x01, (byte)0x23, (byte)0x45,
            (byte)0x67, (byte)0x89, (byte)0xab,
        };
        byte[] tha = {
            (byte)0xff, (byte)0xff, (byte)0xff,
            (byte)0xff, (byte)0xff, (byte)0xff,
        };
        byte[] spa = {(byte)10, (byte)0, (byte)1, (byte)2};
        byte[] tpa = {(byte)192, (byte)168, (byte)100, (byte)200};

        return createArp(sha, tha, spa, tpa, ARP.REQUEST);
    }

    /**
     * Create an ARP packet for test.
     *
     * @param sha  Sender hardware address.
     * @param tha  Target hardware address.
     * @param spa  Sender protocol address.
     * @param tpa  Target protocol address.
     * @param op   ARP operation code.
     * @return  An {@link ARP} instance.
     */
    private ARP createArp(byte[] sha, byte[] tha, byte[] spa, byte[] tpa,
                          short op) {
        ARP arp = new ARP();
        arp.setHardwareType(ARP.HW_TYPE_ETHERNET).
            setHardwareAddressLength((byte)EtherAddress.SIZE).
            setProtocolType(EtherTypes.IPV4.shortValue()).
            setProtocolAddressLength((byte)4).
            setOpCode(op).
            setSenderHardwareAddress(sha).
            setTargetHardwareAddress(tha).
            setSenderProtocolAddress(spa).
            setTargetProtocolAddress(tpa);

        return arp;
    }
}
