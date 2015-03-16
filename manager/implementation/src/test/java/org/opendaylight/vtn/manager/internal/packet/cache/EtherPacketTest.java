/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;

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
    private short  vlanPcp;

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        byte[] bytes = {
            (byte)0x00, (byte)0x0a, (byte)0x7f, (byte)0x88, (byte)0xff,
        };
        int[] types = {0x0001, 0x0815, 0x9999};
        int[] vlans = {0, 1, 2, 4095};
        ARP arp = createArp();
        for (byte src: bytes) {
            byte[] srcAddr = {
                (byte)0x00, (byte)0x11, (byte)0x22,
                (byte)0x33, (byte)0x44, src,
            };
            EtherAddress srcMac = new EtherAddress(srcAddr);
            for (byte dst: bytes) {
                byte[] dstAddr = {
                    (byte)0xf0, (byte)0xf1, (byte)0xf2,
                    (byte)0xf3, (byte)0xf4, dst,
                };
                EtherAddress dstMac = new EtherAddress(dstAddr);
                for (int type: types) {
                    for (int vlan: vlans) {
                        // Create Ethernet frame using ARP packet.
                        Ethernet pkt = createEthernet(srcAddr, dstAddr, type,
                                                      (short)vlan, arp);
                        EtherPacket ether = new EtherPacket(pkt);
                        assertEquals(srcMac, ether.getSourceAddress());
                        assertEquals(dstMac, ether.getDestinationAddress());
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
                        pkt = createEthernet(srcAddr, dstAddr, type, vlan,
                                             RAW_PAYLOAD);
                        ether = new EtherPacket(pkt);
                        assertEquals(srcMac, ether.getSourceAddress());
                        assertEquals(dstMac, ether.getDestinationAddress());
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
                            createPacketContext(pkt, NODE_CONNECTOR);
                        assertFalse(ether.commit(pctx));
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
     * Test case for setter methods and {@link EtherPacket#clone()}.
     */
    @Test
    public void testSetter() {
        int type = 0x0806;
        short[] vids = {EtherHeader.VLAN_NONE, (short)1, (short)4095};
        short vid1 = (short)1000;

        byte[] src0 = {
            (byte)0x10, (byte)0x20, (byte)0x30,
            (byte)0x40, (byte)0x50, (byte)0x60,
        };
        EtherAddress srcMac0 = new EtherAddress(src0);
        byte[] dst0 = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xfa, (byte)0xfb, (byte)0xfc,
        };
        EtherAddress dstMac0 = new EtherAddress(dst0);

        byte[] src1 = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xf3, (byte)0xf4, (byte)0xf5,
        };
        EtherAddress srcMac1 = new EtherAddress(src1);
        byte[] dst1 = {
            (byte)0xa0, (byte)0xbc, (byte)0xde,
            (byte)0xf1, (byte)0x23, (byte)0x45,
        };
        EtherAddress dstMac1 = new EtherAddress(dst1);

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
                    EtherAddress srcMac = srcMac0;
                    EtherAddress dstMac = dstMac0;
                    int vlan = vid;
                    short pcp = vlanPcp;
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
                        ether1.setSourceAddress(srcMac1);
                        src = src1;
                        srcMac = srcMac1;
                        mod = true;
                    }
                    if ((flags & ETH_DST) != 0) {
                        // Modify destination address.
                        ether1.setDestinationAddress(dstMac1);
                        dst = dst1;
                        dstMac = dstMac1;
                        mod = true;
                    }
                    if ((flags & ETH_VLAN_VID) != 0) {
                        // Modify VLAN ID.
                        ether1.setVlanId(outVid);
                        vlan = outVid;
                    }
                    if ((flags & ETH_VLAN_PCP) != 0) {
                        // Modify VLAN priority.
                        pcp = (byte)((pcp + 1) & 0x7);
                        ether1.setVlanPriority(pcp);
                        setPcp = true;
                        if (vlan != EtherHeader.VLAN_NONE) {
                            mod = true;
                        }
                    }
                    ether1.setPayload(anotherArp);
                    assertSame(anotherArp, ether1.getPayload());

                    assertEquals(srcMac, ether1.getSourceAddress());
                    assertEquals(dstMac, ether1.getDestinationAddress());
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
                    List<Action> actions = new ArrayList<Action>();
                    for (Action act: salActions.values()) {
                        if (!(vlan == EtherHeader.VLAN_NONE &&
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
                    if (vlan != EtherHeader.VLAN_NONE &&
                        (flags & ETH_VLAN_PCP) != 0) {
                        actions.add(salActions.get(SetVlanPcp.class));
                    }
                    for (FlowMatchType mt: FlowMatchType.values()) {
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
                    if (vid == EtherHeader.VLAN_NONE) {
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
                    assertEquals(srcMac0, ether.getSourceAddress());
                    assertEquals(dstMac0, ether.getDestinationAddress());
                    assertEquals(type, ether.getEtherType());
                    assertSame(arp, ether.getPayload());
                    if (vid == EtherHeader.VLAN_NONE) {
                        assertTrue(ether.getVlanPriority() < 0);
                    } else {
                        assertEquals(vlanPcp, ether.getVlanPriority());
                    }

                    // Set values in the original packet.
                    ether1.setSourceAddress(srcMac0);
                    ether1.setDestinationAddress(dstMac0);
                    ether1.setVlanId(vid);
                    ether1.setVlanPriority(vlanPcp);
                    assertEquals(false, ether1.commit(pctx));
                    assertEquals(srcMac0, ether1.getSourceAddress());
                    assertEquals(dstMac0, ether1.getDestinationAddress());
                    assertEquals(vid, ether1.getVlanId());
                    assertEquals(vlanPcp, ether1.getVlanPriority());
                    assertEquals(type, ether1.getEtherType());

                    // Ensure that a set of modified values is deeply cloned.
                    EtherPacket ether2 = ether1.clone();
                    assertEquals(srcMac0, ether1.getSourceAddress());
                    assertEquals(dstMac0, ether1.getDestinationAddress());
                    assertEquals(vid, ether1.getVlanId());
                    assertEquals(vlanPcp, ether1.getVlanPriority());
                    assertEquals(type, ether1.getEtherType());
                    assertEquals(srcMac0, ether2.getSourceAddress());
                    assertEquals(dstMac0, ether2.getDestinationAddress());
                    assertEquals(vid, ether2.getVlanId());
                    assertEquals(vlanPcp, ether2.getVlanPriority());
                    assertEquals(type, ether2.getEtherType());
                    ether2.setSourceAddress(srcMac1);
                    ether2.setDestinationAddress(dstMac1);
                    ether2.setVlanId(vid1);
                    ether2.setVlanPriority(pcp);
                    assertEquals(srcMac0, ether1.getSourceAddress());
                    assertEquals(dstMac0, ether1.getDestinationAddress());
                    assertEquals(vid, ether1.getVlanId());
                    assertEquals(vlanPcp, ether1.getVlanPriority());
                    assertEquals(type, ether1.getEtherType());
                    assertEquals(srcMac1, ether2.getSourceAddress());
                    assertEquals(dstMac1, ether2.getDestinationAddress());
                    assertEquals(vid1, ether2.getVlanId());
                    assertEquals(pcp, ether2.getVlanPriority());
                    assertEquals(type, ether2.getEtherType());
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
        short[] vids = {EtherHeader.VLAN_NONE, (short)1, (short)4095};
        Map<FlowMatchType, MatchField> dlFields = new HashMap<>();
        dlFields.put(FlowMatchType.DL_SRC,
                     new MatchField(MatchType.DL_SRC, src));
        dlFields.put(FlowMatchType.DL_DST,
                     new MatchField(MatchType.DL_DST, dst));
        dlFields.put(FlowMatchType.DL_TYPE,
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
            dlFields.put(FlowMatchType.DL_VLAN_PCP,
                         new MatchField(MatchType.DL_VLAN_PR,
                                        Byte.valueOf((byte)vlanPcp)));

            Match match = new Match();
            Set<FlowMatchType> fields = EnumSet.noneOf(FlowMatchType.class);
            ether.setMatch(match, fields);
            List<MatchType> matches = match.getMatchesList();
            assertEquals(1, matches.size());
            assertEquals(vlanVid, match.getField(MatchType.DL_VLAN));

            for (Map.Entry<FlowMatchType, MatchField> entry:
                     dlFields.entrySet()) {
                FlowMatchType fmtype = entry.getKey();
                MatchField mfield = entry.getValue();
                MatchType mtype = mfield.getType();

                match = new Match();
                fields = EnumSet.of(fmtype);
                ether.setMatch(match, fields);
                matches = match.getMatchesList();
                assertEquals(vlanVid, match.getField(MatchType.DL_VLAN));
                if (vid == EtherHeader.VLAN_NONE &&
                    fmtype == FlowMatchType.DL_VLAN_PCP) {
                    assertEquals(1, matches.size());
                    assertEquals(null, match.getField(mtype));
                } else {
                    assertEquals(2, matches.size());
                    assertEquals(mfield, match.getField(mtype));
                }
            }

            // setMatch() always has to see the original.
            byte pri = (byte)((vlanPcp + 1) & 0x7);
            ether.setSourceAddress(new EtherAddress(src1));
            ether.setDestinationAddress(new EtherAddress(dst1));
            ether.setVlanId((vid + 1) & 0xfff);
            ether.setVlanPriority(vlanPcp);
            fields = EnumSet.noneOf(FlowMatchType.class);
            fields.addAll(dlFields.keySet());
            fields.add(FlowMatchType.DL_VLAN);
            match = new Match();
            ether.setMatch(match, fields);
            matches = match.getMatchesList();
            assertEquals(vlanVid, match.getField(MatchType.DL_VLAN));
            if (vid == EtherHeader.VLAN_NONE) {
                assertEquals(dlFields.size(), matches.size());
                assertEquals(null, match.getField(MatchType.DL_VLAN_PR));
            } else {
                assertEquals(dlFields.size() + 1, matches.size());
                for (MatchField mfield: dlFields.values()) {
                    MatchType mtype = mfield.getType();
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
                                    int vid, Packet payload) {
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
    private Ethernet createEthernet(byte[] src, byte[] dst, int type,
                                    int vid, byte[] raw) {
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
