/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.action.SetVlanPcp;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchField;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

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
     * A node connector for test.
     */
    static final NodeConnector  NODE_CONNECTOR;

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
     * Initialize test class.
     */
    static {
        // Create a node connector.
        Node node = NodeCreator.createOFNode(Long.valueOf(1L));
        NODE_CONNECTOR = NodeConnectorCreator.
            createNodeConnector(Short.valueOf((short)1), node);
    }

    /**
     * VLAN priority for test.
     */
    private byte  vlanPcp;

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        byte[] bytes = {
            (byte)0x00, (byte)0x0a, (byte)0x7f, (byte)0x88, (byte)0xff,
        };
        int[] types = {0x0001, 0x0800, 0x86dd};
        short[] vlans = {0, 1, 2, 4095};
        boolean arrayFirst = true;
        ARP arp = createArp();
        for (byte src: bytes) {
            byte[] srcAddr = {
                (byte)0x00, (byte)0x11, (byte)0x22,
                (byte)0x33, (byte)0x44, src,
            };
            long srcMac = NetUtils.byteArray6ToLong(srcAddr);
            for (byte dst: bytes) {
                byte[] dstAddr = {
                    (byte)0xf0, (byte)0xf1, (byte)0xf2,
                    (byte)0xf3, (byte)0xf4, dst,
                };
                long dstMac = NetUtils.byteArray6ToLong(dstAddr);
                for (int type: types) {
                    for (short vlan: vlans) {
                        // Create Ethernet frame using ARP packet.
                        Ethernet pkt = createEthernet(srcAddr, dstAddr, type,
                                                      vlan, arp);
                        EtherPacket ether = new EtherPacket(pkt);
                        if (arrayFirst) {
                            assertArrayEquals(srcAddr,
                                              ether.getSourceAddress());
                            assertArrayEquals(dstAddr,
                                              ether.getDestinationAddress());
                            assertEquals(srcMac, ether.getSourceMacAddress());
                            assertEquals(dstMac,
                                         ether.getDestinationMacAddress());
                            arrayFirst = false;
                        } else {
                            assertEquals(srcMac, ether.getSourceMacAddress());
                            assertEquals(dstMac,
                                         ether.getDestinationMacAddress());
                            assertArrayEquals(srcAddr,
                                              ether.getSourceAddress());
                            assertArrayEquals(dstAddr,
                                              ether.getDestinationAddress());
                            arrayFirst = true;
                        }

                        assertEquals(type, ether.getEtherType());
                        assertEquals(vlan, ether.getOriginalVlan());
                        assertEquals(vlan, ether.getVlan());
                        IEEE8021Q vlanTag;
                        if (vlan == MatchType.DL_VLAN_NONE) {
                            assertTrue(ether.getVlanPriority() < 0);
                            vlanTag = null;
                        } else {
                            assertEquals(vlanPcp, ether.getVlanPriority());
                            vlanTag = new IEEE8021Q();
                            vlanTag.setCfi((byte)0).setPcp(vlanPcp).
                                setVid(vlan).setEtherType((short)type);
                        }
                        assertEquals(vlanTag, ether.getVlanTag());
                        assertEquals(arp, ether.getPayload());
                        assertEquals(null, ether.getRawPayload());
                        assertSame(pkt, ether.getPacket());

                        // Create another Ethernet frame using raw packet.
                        pkt = createEthernet(srcAddr, dstAddr, type, vlan,
                                             RAW_PAYLOAD);
                        ether = new EtherPacket(pkt);
                        assertArrayEquals(srcAddr, ether.getSourceAddress());
                        assertArrayEquals(dstAddr,
                                          ether.getDestinationAddress());
                        assertEquals(srcMac, ether.getSourceMacAddress());
                        assertEquals(dstMac, ether.getDestinationMacAddress());
                        assertEquals(type, ether.getEtherType());
                        assertEquals(vlan, ether.getOriginalVlan());
                        if (vlan == MatchType.DL_VLAN_NONE) {
                            assertTrue(ether.getVlanPriority() < 0);
                            vlanTag = null;
                        } else {
                            assertEquals(vlanPcp, ether.getVlanPriority());
                            vlanTag = new IEEE8021Q();
                            vlanTag.setCfi((byte)0).setPcp(vlanPcp).
                                setVid(vlan).setEtherType((short)type);
                        }
                        assertEquals(vlanTag, ether.getVlanTag());
                        assertEquals(null, ether.getPayload());
                        assertArrayEquals(RAW_PAYLOAD, ether.getRawPayload());
                        assertSame(pkt, ether.getPacket());

                        // commit() should return false.
                        PacketContext pctx =
                            createPacketContext(pkt, NODE_CONNECTOR);
                        assertFalse(ether.commit(pctx));
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
     * Test case for setter methods and {@link EtherPacket#clone()}.
     */
    @Test
    public void testSetter() {
        int type = 0x0806;
        short[] vids = {MatchType.DL_VLAN_NONE, (short)1, (short)4095};

        byte[] src0 = {
            (byte)0x10, (byte)0x20, (byte)0x30,
            (byte)0x40, (byte)0x50, (byte)0x60,
        };
        long srcMac0 = NetUtils.byteArray6ToLong(src0);
        byte[] dst0 = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xfa, (byte)0xfb, (byte)0xfc,
        };
        long dstMac0 = NetUtils.byteArray6ToLong(dst0);

        byte[] src1 = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xf3, (byte)0xf4, (byte)0xf5,
        };
        long srcMac1 = NetUtils.byteArray6ToLong(src1);
        byte[] dst1 = {
            (byte)0xa0, (byte)0xbc, (byte)0xde,
            (byte)0xf1, (byte)0x23, (byte)0x45,
        };
        long dstMac1 = NetUtils.byteArray6ToLong(dst1);

        byte[] src2 = {
            (byte)0xe0, (byte)0xe1, (byte)0xe2,
            (byte)0xe3, (byte)0xe4, (byte)0xe5,
        };
        byte[] dst2 = {
            (byte)0xb0, (byte)0xcd, (byte)0xef,
            (byte)0x12, (byte)0x34, (byte)0x56,
        };

        Map<Class<? extends Action>, Action> salActions =
            new LinkedHashMap<Class<? extends Action>, Action>();
        salActions.put(SetDlSrc.class, new SetDlSrc(src2));
        salActions.put(SetDlDst.class, new SetDlDst(dst2));
        salActions.put(SetVlanPcp.class, new SetVlanPcp(7));

        byte[] spa = {(byte)10, (byte)20, (byte)30, (byte)40};
        byte[] tpa = {(byte)192, (byte)168, (byte)0, (byte)254};
        ARP arp = createArp();
        ARP anotherArp = createArp(src2, dst2, spa, tpa, ARP.REPLY);

        for (int flags = ETH_SRC; flags <= ETH_ALL; flags++) {
            for (short vid: vids) {
                for (short outVid: vids) {
                    Ethernet pkt =
                        createEthernet(src0, dst0, type, vid, arp);
                    EtherPacket ether = new EtherPacket(pkt);

                    byte[] src = src0;
                    byte[] dst = dst0;
                    long srcMac = srcMac0;
                    long dstMac = dstMac0;
                    short vlan = vid;
                    byte pcp = vlanPcp;
                    boolean setPcp = false;
                    boolean mod = false;

                    PacketContext pctx =
                        createPacketContext(pkt, NODE_CONNECTOR);
                    for (Action act: salActions.values()) {
                        pctx.addFilterAction(act);
                    }
                    EtherPacket ether1 = ether.clone();
                    assertNotSame(ether, ether1);

                    if ((flags & ETH_SRC) != 0) {
                        // Modify source address.
                        ether1.setSourceAddress(src1);
                        src = src1;
                        srcMac = srcMac1;
                        mod = true;
                    }
                    if ((flags & ETH_DST) != 0) {
                        // Modify destination address.
                        ether1.setDestinationAddress(dst1);
                        dst = dst1;
                        dstMac = dstMac1;
                        mod = true;
                    }
                    if ((flags & ETH_VLAN_VID) != 0) {
                        // Modify VLAN ID.
                        ether1.setVlan(outVid);
                        vlan = outVid;
                    }
                    if ((flags & ETH_VLAN_PCP) != 0) {
                        // Modify VLAN priority.
                        pcp = (byte)((pcp + 1) & 0x7);
                        ether1.setVlanPriority(pcp);
                        setPcp = true;
                        if (vlan != MatchType.DL_VLAN_NONE) {
                            mod = true;
                        }
                    }
                    ether1.setPayload(anotherArp);
                    assertSame(anotherArp, ether1.getPayload());

                    assertArrayEquals(src, ether1.getSourceAddress());
                    assertEquals(srcMac, ether1.getSourceMacAddress());
                    assertArrayEquals(dst, ether1.getDestinationAddress());
                    assertEquals(dstMac, ether1.getDestinationMacAddress());
                    assertEquals(type, ether1.getEtherType());
                    assertEquals(vlan, ether1.getVlan());
                    assertEquals(vid, ether1.getOriginalVlan());
                    if (vid == MatchType.DL_VLAN_NONE && !setPcp) {
                        assertTrue(ether1.getVlanPriority() < 0);
                    } else {
                        assertEquals(pcp, ether1.getVlanPriority());
                    }

                    // SET_VLAN_PCP action should be removed if a VLAN tag is
                    // not configured in outgoing packet.
                    List<Action> actions = new ArrayList<Action>();
                    for (Action act: salActions.values()) {
                        if (!(vlan == MatchType.DL_VLAN_NONE &&
                              SetVlanPcp.class.isInstance(act))) {
                            actions.add(act);
                        }
                    }
                    assertEquals(mod, ether1.commit(pctx));

                    List<Action> filterActions =
                        new ArrayList<Action>(pctx.getFilterActions());
                    assertEquals(actions, filterActions);

                    // Actions for unchanged field will be removed if
                    // corresponding match type is configured in PacketContext.
                    actions.clear();
                    if ((flags & ETH_SRC) != 0) {
                        actions.add(salActions.get(SetDlSrc.class));
                    }
                    if ((flags & ETH_DST) != 0) {
                        actions.add(salActions.get(SetDlDst.class));
                    }
                    if (vlan != MatchType.DL_VLAN_NONE &&
                        (flags & ETH_VLAN_PCP) != 0) {
                        actions.add(salActions.get(SetVlanPcp.class));
                    }
                    for (MatchType mt: MatchType.values()) {
                        pctx.addMatchField(mt);
                    }

                    assertEquals(mod, ether1.commit(pctx));
                    filterActions =
                        new ArrayList<Action>(pctx.getFilterActions());
                    assertEquals(actions, filterActions);

                    // EtherPacket class should never modify the packet.
                    assertSame(pkt, ether1.getPacket());
                    assertArrayEquals(src0, pkt.getSourceMACAddress());
                    assertArrayEquals(dst0, pkt.getDestinationMACAddress());
                    if (vid == MatchType.DL_VLAN_NONE) {
                        assertEquals((short)type, pkt.getEtherType());
                        assertSame(arp, pkt.getPayload());
                    } else {
                        assertEquals(EtherTypes.VLANTAGGED.shortValue(),
                                     pkt.getEtherType());
                        IEEE8021Q tag = (IEEE8021Q)pkt.getPayload();
                        assertEquals(vid, tag.getVid());
                        assertEquals(vlanPcp, tag.getPcp());
                        assertSame(arp, tag.getPayload());
                    }

                    // The original packet should not be affected.
                    assertArrayEquals(src0, ether.getSourceAddress());
                    assertEquals(srcMac0, ether.getSourceMacAddress());
                    assertArrayEquals(dst0, ether.getDestinationAddress());
                    assertEquals(dstMac0, ether.getDestinationMacAddress());
                    assertEquals(type, ether.getEtherType());
                    assertSame(arp, ether.getPayload());
                    if (vid == MatchType.DL_VLAN_NONE) {
                        assertTrue(ether.getVlanPriority() < 0);
                    } else {
                        assertEquals(vlanPcp, ether.getVlanPriority());
                    }

                    // Set values in the original packet.
                    ether1.setSourceAddress(src0);
                    ether1.setDestinationAddress(dst0);
                    ether1.setVlan(vid);
                    ether1.setVlanPriority(vlanPcp);
                    assertEquals(false, ether1.commit(pctx));
                    assertArrayEquals(src0, ether1.getSourceAddress());
                    assertEquals(srcMac0, ether1.getSourceMacAddress());
                    assertArrayEquals(dst0, ether1.getDestinationAddress());
                    assertEquals(dstMac0, ether1.getDestinationMacAddress());
                    assertEquals(vid, ether1.getVlan());
                    assertEquals(vlanPcp, ether1.getVlanPriority());
                    assertEquals(type, ether1.getEtherType());
                }
            }
        }
    }

    /**
     * Test case for {@link EtherPacket#setMatch(Match, Set)}.
     */
    @Test
    public void testSetMatch() {
        byte[] src = {
            (byte)0x10, (byte)0x20, (byte)0x30,
            (byte)0x40, (byte)0x50, (byte)0x60,
        };
        byte[] dst = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xfa, (byte)0xfb, (byte)0xfc,
        };
        int type = 0x0806;
        short[] vids = {MatchType.DL_VLAN_NONE, (short)1, (short)4095};
        Map<MatchType, MatchField> dlFields =
            new HashMap<MatchType, MatchField>();
        dlFields.put(MatchType.DL_SRC, new MatchField(MatchType.DL_SRC, src));
        dlFields.put(MatchType.DL_DST, new MatchField(MatchType.DL_DST, dst));
        dlFields.put(MatchType.DL_TYPE,
                     new MatchField(MatchType.DL_TYPE,
                                    Short.valueOf((short)type)));

        byte[] src1 = {
            (byte)0x00, (byte)0x44, (byte)0x66,
            (byte)0x88, (byte)0xaa, (byte)0xcc,
        };
        byte[] dst1 = {
            (byte)0xf0, (byte)0x33, (byte)0x55,
            (byte)0x77, (byte)0x99, (byte)0xbb,
        };

        ARP arp = createArp();

        for (short vid: vids) {
            Ethernet pkt = createEthernet(src, dst, type, vid, arp);
            EtherPacket ether = new EtherPacket(pkt);

            // VLAN ID must be configured even if the type set is empty.
            MatchField vlanVid =
                new MatchField(MatchType.DL_VLAN, Short.valueOf(vid));
            dlFields.put(MatchType.DL_VLAN_PR,
                         new MatchField(MatchType.DL_VLAN_PR,
                                        Byte.valueOf(vlanPcp)));

            Match match = new Match();
            Set<MatchType> fields = EnumSet.noneOf(MatchType.class);
            ether.setMatch(match, fields);
            List<MatchType> matches = match.getMatchesList();
            assertEquals(1, matches.size());
            assertEquals(vlanVid, match.getField(MatchType.DL_VLAN));

            for (Map.Entry<MatchType, MatchField> entry: dlFields.entrySet()) {
                MatchType mtype = entry.getKey();

                match = new Match();
                fields = EnumSet.of(mtype);
                ether.setMatch(match, fields);
                matches = match.getMatchesList();
                assertEquals(vlanVid, match.getField(MatchType.DL_VLAN));
                if (vid == MatchType.DL_VLAN_NONE &&
                    mtype == MatchType.DL_VLAN_PR) {
                    assertEquals(1, matches.size());
                    assertEquals(null, match.getField(mtype));
                } else {
                    MatchField mfield = entry.getValue();
                    assertEquals(2, matches.size());
                    assertEquals(mfield, match.getField(mtype));
                }
            }

            // setMatch() always has to see the original.
            byte pri = (byte)((vlanPcp + 1) & 0x7);
            ether.setSourceAddress(src1);
            ether.setDestinationAddress(dst1);
            ether.setVlan((short)((vid + 1) & 0xfff));
            ether.setVlanPriority(vlanPcp);
            fields = EnumSet.noneOf(MatchType.class);
            fields.addAll(dlFields.keySet());
            fields.add(MatchType.DL_VLAN);
            match = new Match();
            ether.setMatch(match, fields);
            matches = match.getMatchesList();
            assertEquals(vlanVid, match.getField(MatchType.DL_VLAN));
            if (vid == MatchType.DL_VLAN_NONE) {
                assertEquals(dlFields.size(), matches.size());
                assertEquals(null, match.getField(MatchType.DL_VLAN_PR));
            } else {
                assertEquals(dlFields.size() + 1, matches.size());
                for (Map.Entry<MatchType, MatchField> entry:
                         dlFields.entrySet()) {
                    MatchType mtype = entry.getKey();
                    MatchField mfield = entry.getValue();
                    assertEquals(mfield, match.getField(mtype));
                }
            }
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
    private Ethernet createEthernet(byte[] src, byte[] dst, int type,
                                    short vid, Packet payload) {
        byte pcp;
        if (vid == MatchType.DL_VLAN_NONE) {
            pcp = 0;
        } else {
            pcp = (byte)((vlanPcp + 1) & 7);
            vlanPcp = pcp;
        }

        return createEthernet(src, dst, type, vid, pcp, payload);
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
    private Ethernet createEthernet(byte[] src, byte[] dst, int type,
                                    short vid, byte[] raw) {
        byte pcp;
        if (vid == MatchType.DL_VLAN_NONE) {
            pcp = 0;
        } else {
            pcp = (byte)((vlanPcp + 1) & 7);
            vlanPcp = pcp;
        }

        return createEthernet(src, dst, type, vid, pcp, raw);
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
            setHardwareAddressLength((byte)NetUtils.MACAddrLengthInBytes).
            setProtocolType(EtherTypes.IPv4.shortValue()).
            setProtocolAddressLength((byte)4).
            setOpCode(op).
            setSenderHardwareAddress(sha).
            setTargetHardwareAddress(tha).
            setSenderProtocolAddress(spa).
            setTargetProtocolAddress(tpa);

        return arp;
    }
}
