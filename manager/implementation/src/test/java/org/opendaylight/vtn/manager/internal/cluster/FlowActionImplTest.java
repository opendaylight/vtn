/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Random;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.DropAction;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.PopVlanAction;
import org.opendaylight.vtn.manager.flow.action.PushVlanAction;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanIdAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;
import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.flow.cond.PortMatch;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;
import org.opendaylight.vtn.manager.flow.cond.UdpMatch;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Drop;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.PushVlan;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.action.SetNwTos;
import org.opendaylight.controller.sal.action.SetTpDst;
import org.opendaylight.controller.sal.action.SetTpSrc;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.action.SetVlanPcp;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link FlowActionImpl}.
 */
public class FlowActionImplTest extends TestBase {
    /**
     * Mask bits for valid unicast MAC address bits.
     */
    private static final long  MACADDR_MASK = 0xfeffffffffffL;

    /**
     * Test case for {@link VTNFlow#toFlowMatch(Match)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToFlowMatch() throws Exception {
        Match match = null;
        FlowMatch fm = VTNFlow.toFlowMatch(match);
        assertEquals(null, fm.getIndex());
        assertEquals(null, fm.getEthernetMatch());
        assertEquals(null, fm.getInetMatch());
        assertEquals(null, fm.getLayer4Match());
        assertEquals(-1, fm.getInetProtocol());

        // L2 conditions.
        byte[] srcMac = {
            (byte)0x00, (byte)0x01, (byte)0x02,
            (byte)0x03, (byte)0x04, (byte)0x05,
        };
        byte[] dstMac = {
            (byte)0x1a, (byte)0x2b, (byte)0x3c,
            (byte)0x4d, (byte)0x5e, (byte)0x6f,
        };
        EtherAddress ethSrc = new EtherAddress(srcMac);
        EtherAddress ethDst = new EtherAddress(dstMac);
        EtherAddress nullMac = null;
        Integer ethType = Integer.valueOf(0x8765);
        Short ethVlan = Short.valueOf((short)4095);
        Byte ethPri = Byte.valueOf((byte)7);
        MatchType[] matchTypes = {
            MatchType.DL_SRC, MatchType.DL_DST, MatchType.DL_VLAN,
            MatchType.DL_VLAN_PR, MatchType.DL_TYPE,
        };
        for (MatchType mtype: matchTypes) {
            EthernetMatch em;
            match = new Match();
            switch (mtype) {
            case DL_SRC:
                em = new EthernetMatch(ethSrc, null, null, null, null);
                match.setField(mtype, srcMac);
                break;

            case DL_DST:
                em = new EthernetMatch(null, ethDst, null, null, null);
                match.setField(mtype, dstMac);
                break;

            case DL_VLAN:
                em = new EthernetMatch(nullMac, nullMac, null, ethVlan, null);
                match.setField(mtype, ethVlan);
                break;

            case DL_VLAN_PR:
                em = new EthernetMatch(nullMac, nullMac, null, null, ethPri);
                match.setField(mtype, ethPri);
                break;

            default:
                em = new EthernetMatch(nullMac, nullMac, ethType, null, null);
                match.setField(mtype, Short.valueOf(ethType.shortValue()));
                break;
            }

            fm = VTNFlow.toFlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
        }

        // L3 conditions (without netmask)
        InetAddress inetSrc = InetAddress.getByName("192.168.100.2");
        InetAddress inetDst = InetAddress.getByName("10.250.132.10");
        Short inetProto = Short.valueOf((short)0xfa);
        Byte inetDscp = Byte.valueOf((byte)3);

        matchTypes = new MatchType[]{
            MatchType.NW_SRC, MatchType.NW_DST, MatchType.NW_PROTO,
            MatchType.NW_TOS,
        };
        for (MatchType mtype: matchTypes) {
            InetMatch im;
            int proto = -1;
            match = new Match();
            switch (mtype) {
            case NW_SRC:
                im = new Inet4Match(inetSrc, null, null, null, null, null);
                match.setField(mtype, inetSrc);
                break;

            case NW_DST:
                im = new Inet4Match(null, null, inetDst, null, null, null);
                match.setField(mtype, inetDst);
                break;

            case NW_PROTO:
                im = new Inet4Match(null, null, null, null, inetProto, null);
                match.setField(mtype, Byte.valueOf(inetProto.byteValue()));
                proto = inetProto.intValue();
                break;

            default:
                im = new Inet4Match(null, null, null, null, null, inetDscp);
                match.setField(mtype, inetDscp);
                break;
            }

            fm = VTNFlow.toFlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(proto, fm.getInetProtocol());
        }

        // L3 conditions (with netmask)
        Short inetSrcSuff = Short.valueOf((short)24);
        Short inetDstSuff = Short.valueOf((short)31);
        InetAddress srcSuff = Ip4Network.getInetMask(inetSrcSuff.intValue());
        InetAddress dstSuff = Ip4Network.getInetMask(inetDstSuff.intValue());

        matchTypes = new MatchType[]{
            MatchType.NW_SRC, MatchType.NW_DST,
        };
        for (MatchType mtype: matchTypes) {
            InetMatch im;
            match = new Match();
            switch (mtype) {
            case NW_SRC:
                im = new Inet4Match(inetSrc, inetSrcSuff, null, null, null,
                                    null);
                match.setField(mtype, inetSrc, srcSuff);
                break;

            default:
                im = new Inet4Match(null, null, inetDst, inetDstSuff, null,
                                    null);
                match.setField(mtype, inetDst, dstSuff);
                break;
            }

            fm = VTNFlow.toFlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
        }

        // L4 conditions require NW_PROTO match.
        ethType = Integer.valueOf(0x800);
        matchTypes = new MatchType[]{
            MatchType.TP_SRC, MatchType.TP_DST,
        };

        Byte[] badProtocols = {
            null, Byte.valueOf((byte)41), Byte.valueOf((byte)58),
        };
        for (Byte proto: badProtocols) {
            for (MatchType mtype: matchTypes) {
                InetMatch im;
                match = new Match();
                match.setField(mtype, Short.valueOf((short)1));
                match.setField(MatchType.DL_SRC, srcMac);
                match.setField(MatchType.DL_DST, dstMac);
                match.setField(MatchType.DL_TYPE,
                               Short.valueOf(ethType.shortValue()));
                match.setField(MatchType.DL_VLAN, ethVlan);
                match.setField(MatchType.DL_VLAN_PR, ethPri);
                match.setField(MatchType.NW_SRC, inetSrc);
                match.setField(MatchType.NW_DST, inetDst);
                match.setField(MatchType.NW_TOS, inetDscp);
                if (proto != null) {
                    match.setField(MatchType.NW_PROTO, proto);
                }

                try {
                    VTNFlow.toFlowMatch(match);
                    unexpected();
                } catch (IllegalArgumentException e) {
                    String expected;
                    if (proto == null) {
                        expected = "L4 match without NW_PROTO: ";
                    } else {
                        expected = "Unexpected IP protocol: ";
                    }

                    String msg = e.getMessage();
                    assertTrue("Unexpected exception: " + e,
                               msg.startsWith(expected));
                }
            }
        }

        // TCP conditions.
        Short tcpSrc = Short.valueOf((short)30);
        Short tcpDst = Short.valueOf((short)65534);
        IPProtocols ipproto = IPProtocols.TCP;
        for (MatchType mtype: matchTypes) {
            TcpMatch tm;
            match = new Match();
            match.setField(MatchType.NW_PROTO,
                           Byte.valueOf(ipproto.byteValue()));

            InetMatch im = new Inet4Match(null, null, null, null,
                                          Short.valueOf(ipproto.shortValue()),
                                          null);

            switch (mtype) {
            case TP_SRC:
                tm = new TcpMatch(new PortMatch(tcpSrc), null);
                match.setField(mtype, tcpSrc);
                break;

            default:
                tm = new TcpMatch(null, new PortMatch(tcpDst));
                match.setField(mtype, tcpDst);
                break;
            }

            fm = VTNFlow.toFlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(tm, fm.getLayer4Match());
            assertEquals(ipproto.intValue(), fm.getInetProtocol());
        }

        // UDP conditions.
        Short udpSrc = Short.valueOf((short)60000);
        Short udpDst = Short.valueOf((short)22);
        ipproto = IPProtocols.UDP;
        for (MatchType mtype: matchTypes) {
            UdpMatch um;
            match = new Match();
            match.setField(MatchType.NW_PROTO,
                           Byte.valueOf(ipproto.byteValue()));

            InetMatch im = new Inet4Match(null, null, null, null,
                                          Short.valueOf(ipproto.shortValue()),
                                          null);

            switch (mtype) {
            case TP_SRC:
                um = new UdpMatch(new PortMatch(udpSrc), null);
                match.setField(mtype, udpSrc);
                break;

            default:
                um = new UdpMatch(null, new PortMatch(udpDst));
                match.setField(mtype, udpDst);
                break;
            }

            fm = VTNFlow.toFlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(um, fm.getLayer4Match());
            assertEquals(ipproto.intValue(), fm.getInetProtocol());
        }

        // ICMP conditions.
        Short icmpType = Short.valueOf((short)0x80);
        Short icmpCode = Short.valueOf((short)0x90);
        ipproto = IPProtocols.ICMP;
        for (MatchType mtype: matchTypes) {
            IcmpMatch icm;
            match = new Match();
            match.setField(MatchType.NW_PROTO,
                           Byte.valueOf(ipproto.byteValue()));

            InetMatch im = new Inet4Match(null, null, null, null,
                                          Short.valueOf(ipproto.shortValue()),
                                          null);

            switch (mtype) {
            case TP_SRC:
                icm = new IcmpMatch(icmpType, null);
                match.setField(mtype, icmpType);
                break;

            default:
                icm = new IcmpMatch(null, icmpCode);
                match.setField(mtype, icmpCode);
                break;
            }

            fm = VTNFlow.toFlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(icm, fm.getLayer4Match());
            assertEquals(ipproto.intValue(), fm.getInetProtocol());
        }

        // Complexed condition.
        EthernetMatch em = new EthernetMatch(ethSrc, ethDst, ethType, ethVlan,
                                             ethPri);
        IPProtocols[] protocols = {
            IPProtocols.TCP, IPProtocols.UDP, IPProtocols.ICMP,
        };
        for (IPProtocols ipp: protocols) {
            match = new Match();
            match.setField(MatchType.DL_SRC, srcMac);
            match.setField(MatchType.DL_DST, dstMac);
            match.setField(MatchType.DL_TYPE,
                           Short.valueOf(ethType.shortValue()));
            match.setField(MatchType.DL_VLAN, ethVlan);
            match.setField(MatchType.DL_VLAN_PR, ethPri);
            match.setField(MatchType.NW_SRC, inetSrc, srcSuff);
            match.setField(MatchType.NW_DST, inetDst, dstSuff);
            match.setField(MatchType.NW_PROTO, Byte.valueOf(ipp.byteValue()));
            match.setField(MatchType.NW_TOS, inetDscp);
            InetMatch im = new Inet4Match(inetSrc, inetSrcSuff, inetDst,
                                          inetDstSuff,
                                          Short.valueOf(ipp.shortValue()),
                                          inetDscp);
            L4Match lm;
            switch (ipp) {
            case TCP:
                lm = new TcpMatch(new PortMatch(tcpSrc),
                                  new PortMatch(tcpDst));
                match.setField(MatchType.TP_SRC, tcpSrc);
                match.setField(MatchType.TP_DST, tcpDst);
                break;

            case UDP:
                lm = new UdpMatch(new PortMatch(udpSrc),
                                  new PortMatch(udpDst));
                match.setField(MatchType.TP_SRC, udpSrc);
                match.setField(MatchType.TP_DST, udpDst);
                break;

            default:
                lm = new IcmpMatch(icmpType, icmpCode);
                match.setField(MatchType.TP_SRC, icmpType);
                match.setField(MatchType.TP_DST, icmpCode);
                break;
            }

            fm = VTNFlow.toFlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(lm, fm.getLayer4Match());
            assertEquals(ipp.intValue(), fm.getInetProtocol());
        }

        // Constructor of FlowMatch will throw IllegalArgumentException
        // if an invalid MAC address is configured in DL_SRC/DL_DST field.
        MatchType[] dlTypes = {
            MatchType.DL_SRC,
            MatchType.DL_DST,
        };
        byte[] badAddr = {0};
        for (MatchType type: dlTypes) {
            match = new Match();
            match.setField(type, badAddr);

            try {
                VTNFlow.toFlowMatch(match);
                unexpected();
            } catch (IllegalArgumentException e) {
            }
        }
    }

    /**
     * Test case for {@link VTNFlow#toFlowAction(Action, int)}.
     */
    @Test
    public void testToFlowAction() {
        byte[][] macAddrs = {
            new byte[]{
                (byte)0x00, (byte)0x01, (byte)0x02,
                (byte)0x03, (byte)0x04, (byte)0x05,
            },
            new byte[]{
                (byte)0xfa, (byte)0xfb, (byte)0xfc,
                (byte)0xfc, (byte)0xfe, (byte)0xff,
            },
        };
        short[] vlans = {1, 2, 100, 1000, 4000, 4095};
        List<InetAddress> inet4Addrs = createInet4Addresses(false);
        byte[] dscps = {
            0, 1, 3, 10, 15,
            30, 31, 32, 40, 50, 60, 63
        };
        int[] protocols = {
            -1, 0,
            IPProtocols.TCP.intValue(),
            IPProtocols.UDP.intValue(),
            IPProtocols.ICMP.intValue(),
            IPProtocols.IPV6.intValue(),
        };
        EtherTypes[] vlanTags = {
            EtherTypes.VLANTAGGED, EtherTypes.QINQ,
        };

        for (int proto: protocols) {
            assertEquals(null, VTNFlow.toFlowAction(null, proto));

            FlowAction act = VTNFlow.toFlowAction(new Drop(), proto);
            assertEquals(new DropAction(), act);
            act = VTNFlow.toFlowAction(new PopVlan(), proto);
            assertEquals(new PopVlanAction(), act);

            for (byte[] mac: macAddrs) {
                act = VTNFlow.toFlowAction(new SetDlDst(mac), proto);
                assertEquals(new SetDlDstAction(mac), act);
                act = VTNFlow.toFlowAction(new SetDlSrc(mac), proto);
                assertEquals(new SetDlSrcAction(mac), act);
            }

            for (EtherTypes tag: vlanTags) {
                act = VTNFlow.toFlowAction(new PushVlan(tag), proto);
                assertEquals(new PushVlanAction(tag.intValue()), act);
            }

            for (short vlan: vlans) {
                act = VTNFlow.toFlowAction(new SetVlanId(vlan), proto);
                assertEquals(new SetVlanIdAction(vlan), act);
            }
            for (byte pcp = 0; pcp <= 7; pcp++) {
                act = VTNFlow.toFlowAction(new SetVlanPcp(pcp), proto);
                assertEquals(new SetVlanPcpAction(pcp), act);
            }
            for (InetAddress iaddr: inet4Addrs) {
                act = VTNFlow.toFlowAction(new SetNwDst(iaddr), proto);
                assertEquals(new SetInet4DstAction(iaddr), act);
                act = VTNFlow.toFlowAction(new SetNwSrc(iaddr), proto);
                assertEquals(new SetInet4SrcAction(iaddr), act);
            }
            for (byte dscp: dscps) {
                act = VTNFlow.toFlowAction(new SetNwTos(dscp), proto);
                assertEquals(new SetDscpAction(dscp), act);
            }
        }

        int icmp = IPProtocols.ICMP.intValue();
        short[] icmpValues = {0, 1, 2, 64, 128, 200, 255};
        for (short v: icmpValues) {
            Action sal = new SetTpSrc((int)v);
            FlowAction act = VTNFlow.toFlowAction(sal, icmp);
            assertEquals(new SetIcmpTypeAction(v), act);
            sal = new SetTpDst((int)v);
            act = VTNFlow.toFlowAction(sal, icmp);
            assertEquals(new SetIcmpCodeAction(v), act);
        }

        int[] ports = {1, 2, 100, 200, 500, 10000, 30000, 50000, 65535};
        protocols = new int[]{
            -1, 0,
            IPProtocols.TCP.intValue(),
            IPProtocols.UDP.intValue(),
            IPProtocols.SCTP.intValue(),
            IPProtocols.IPV6.intValue(),
        };
        for (int proto: protocols) {
            boolean valid = (proto == IPProtocols.TCP.intValue() ||
                             proto == IPProtocols.UDP.intValue());
            for (int port: ports) {
                Action sal = new SetTpSrc(port);
                FlowAction act = VTNFlow.toFlowAction(sal, proto);
                FlowAction expected = (valid)
                    ? new SetTpSrcAction(port) : null;
                assertEquals(expected, act);

                sal = new SetTpDst(port);
                act = VTNFlow.toFlowAction(sal, proto);
                expected = (valid) ? new SetTpDstAction(port) : null;
                assertEquals(expected, act);
            }
        }

        // Specifying IPv6 address to SET_NW_SRC/SET_NW_DST action causes
        // IllegalArgumentException.
        InetAddress v6addr = null;
        try {
            v6addr = InetAddress.getByName("::1");
        } catch (Exception e) {
            unexpected(e);
        }

        int proto = IPProtocols.TCP.intValue();
        try {
            VTNFlow.toFlowAction(new SetNwSrc(v6addr), proto);
            unexpected();
        } catch (IllegalArgumentException e) {
        }
        try {
            VTNFlow.toFlowAction(new SetNwDst(v6addr), proto);
            unexpected();
        } catch (IllegalArgumentException e) {
        }
    }

    /**
     * Test case for {@link FlowActionImpl#create(FlowAction)}.
     */
    @Test
    public void testCreate() throws Exception {
        // Expected action implementation classes.
        HashMap<Class<?>, Class<?>> implClasses =
            new HashMap<Class<?>, Class<?>>();
        implClasses.put(SetDlSrcAction.class, SetDlSrcActionImpl.class);
        implClasses.put(SetDlDstAction.class, SetDlDstActionImpl.class);
        implClasses.put(SetVlanPcpAction.class, SetVlanPcpActionImpl.class);
        implClasses.put(SetInet4SrcAction.class, SetInet4SrcActionImpl.class);
        implClasses.put(SetInet4DstAction.class, SetInet4DstActionImpl.class);
        implClasses.put(SetDscpAction.class, SetDscpActionImpl.class);
        implClasses.put(SetTpSrcAction.class, SetTpSrcActionImpl.class);
        implClasses.put(SetTpDstAction.class, SetTpDstActionImpl.class);
        implClasses.put(SetIcmpTypeAction.class, SetIcmpTypeActionImpl.class);
        implClasses.put(SetIcmpCodeAction.class, SetIcmpCodeActionImpl.class);

        // Create actions.
        ArrayList<FlowAction> actions = new ArrayList<FlowAction>();
        Random rand = new Random();
        final int naddrs = 5;
        int count = 0;
        do {
            long v = rand.nextLong() & MACADDR_MASK;
            if (v != 0) {
                byte[] addr = EtherAddress.toBytes(v);
                actions.add(new SetDlSrcAction(addr));
                count++;
            }
        } while (count < naddrs);

        count = 0;
        do {
            long v = rand.nextLong() & MACADDR_MASK;
            if (v != 0) {
                byte[] addr = EtherAddress.toBytes(v);
                actions.add(new SetDlDstAction(addr));
                count++;
            }
        } while (count < naddrs);

        byte[] priorities = {0, 4, 7};
        for (byte pri: priorities) {
            actions.add(new SetVlanPcpAction(pri));
        }

        count = 0;
        do {
            int v = rand.nextInt();
            byte[] addr = NumberUtils.toBytes(v);
            InetAddress iaddr = InetAddress.getByAddress(addr);
            actions.add(new SetInet4SrcAction(iaddr));
            count++;
        } while (count < naddrs);

        count = 0;
        do {
            int v = rand.nextInt();
            byte[] addr = NumberUtils.toBytes(v);
            InetAddress iaddr = InetAddress.getByAddress(addr);
            actions.add(new SetInet4DstAction(iaddr));
            count++;
        } while (count < naddrs);

        byte[] dscps = {0, 18, 32, 63};
        for (byte dscp: dscps) {
            actions.add(new SetDscpAction(dscp));
        }

        int[] ports = {0, 53, 200, 456, 20000, 40000, 65535};
        for (int port: ports) {
            actions.add(new SetTpSrcAction(port));
        }

        ports = new int[]{0, 31, 113, 789, 12345, 34567, 65535};
        for (int port: ports) {
            actions.add(new SetTpDstAction(port));
        }

        short[] types = {0, 63, 112, 255};
        for (short type: types) {
            actions.add(new SetIcmpTypeAction(type));
        }

        short[] codes = {0, 57, 128, 231, 255};
        for (short code: codes) {
            actions.add(new SetIcmpCodeAction(code));
        }

        for (FlowAction act: actions) {
            try {
                FlowActionImpl impl = FlowActionImpl.create(act);
                assertEquals(act, impl.getFlowAction());
                Class<?> cl = act.getClass();
                assertEquals(implClasses.get(cl), impl.getClass());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        // null action.
        try {
            FlowActionImpl.create(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Unsupported actions.
        FlowAction[] unsupported = {
            new DropAction(),
            new SetVlanIdAction((short)1),
            new PopVlanAction(),
        };
        for (FlowAction act: unsupported) {
            try {
                FlowActionImpl.create(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        // Ensure that we can catch an exception thrown by constructor.
        FlowAction[] invalid = {
            new SetDlSrcAction(new byte[0]),
            new SetDlDstAction(new byte[]{0, 0, 0, 0, 0, 0}),
            new SetVlanPcpAction((byte)-1),
            new SetInet4SrcAction((InetAddress)null),
            new SetInet4DstAction((InetAddress)null),
            new SetDscpAction((byte)64),
            new SetTpSrcAction(-1),
            new SetTpDstAction(0x10000),
            new SetIcmpTypeAction((short)256),
            new SetIcmpCodeAction((short)-1),
        };
        for (FlowAction act: invalid) {
            try {
                FlowActionImpl.create(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link FlowActionImpl#equals(Object)} and
     * {@link FlowActionImpl#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        byte[] addr = {0, 0, 0, 0, 0, 1};

        try {
            FlowActionImpl[] actions = {
                new SetDlSrcActionImpl(new SetDlSrcAction(addr)),
                new SetDlDstActionImpl(new SetDlDstAction(addr)),
                new SetVlanPcpActionImpl(new SetVlanPcpAction((byte)0)),
                new SetDscpActionImpl(new SetDscpAction((byte)0)),
                new SetIcmpTypeActionImpl(new SetIcmpTypeAction((short)0)),
                new SetIcmpCodeActionImpl(new SetIcmpCodeAction((short)0)),
            };

            // FlowActionImpl variants should be treated as different objects
            // if classes don't match.
            int count = 0;
            for (FlowActionImpl impl: actions) {
                assertTrue(set.add(impl));
                assertFalse(set.add(impl));
                count++;
                assertEquals(count, set.size());
            }

            for (FlowActionImpl impl: actions) {
                assertTrue(set.remove(impl));
                assertFalse(set.remove(impl));
                count--;
                assertEquals(count, set.size());
            }
            assertTrue(set.isEmpty());
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
