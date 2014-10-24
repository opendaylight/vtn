/*/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * JUnit test for {@link FlowMatch}.
 */
public class FlowMatchTest extends TestBase {
    /**
     * Test case for getter methods and {@link FlowMatch#assignIndex(int)}.
     */
    @Test
    public void testGetter() {
        Integer[] indices = {
            null, Integer.valueOf(1), Integer.valueOf(100),
            Integer.valueOf(65535)
        };

        Integer newIndex = Integer.valueOf(350);

        for (Integer idx: indices) {
            for (EthernetMatch em: createEthernetMatches()) {
                for (InetMatch im: createInetMatches()) {
                    for (L4Match lm: createL4Matches()) {
                        FlowMatch fm = (idx == null)
                            ? new FlowMatch(em, im, lm)
                            : new FlowMatch(idx.intValue(), em, im, lm);
                        assertEquals(idx, fm.getIndex());
                        assertEquals(em, fm.getEthernetMatch());
                        assertEquals(im, fm.getInetMatch());
                        assertEquals(lm, fm.getLayer4Match());

                        int p = -1;
                        if (im != null) {
                            Short proto = im.getProtocol();
                            if (proto != null) {
                                p = proto.intValue();
                            }
                        }
                        assertEquals(p, fm.getInetProtocol());

                        int ni = newIndex.intValue();
                        FlowMatch fm1 = fm.assignIndex(ni);
                        assertEquals(newIndex, fm1.getIndex());
                        assertEquals(em, fm1.getEthernetMatch());
                        assertEquals(im, fm1.getInetMatch());
                        assertEquals(lm, fm1.getLayer4Match());
                        assertNotSame(fm1, fm);
                        FlowMatch fm2 = fm1.assignIndex(ni);
                        assertEquals(newIndex, fm1.getIndex());
                        assertEquals(em, fm1.getEthernetMatch());
                        assertEquals(im, fm1.getInetMatch());
                        assertEquals(lm, fm1.getLayer4Match());
                        assertSame(fm1, fm2);
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link FlowMatch#FlowMatch(Match)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        Match match = null;
        FlowMatch fm = new FlowMatch(match);
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
        EthernetAddress ethSrc = new EthernetAddress(srcMac);
        EthernetAddress ethDst = new EthernetAddress(dstMac);
        Integer ethType = Integer.valueOf(0x800);
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
                em = new EthernetMatch(null, null, null, ethVlan, null);
                match.setField(mtype, ethVlan);
                break;

            case DL_VLAN_PR:
                em = new EthernetMatch(null, null, null, null, ethPri);
                match.setField(mtype, ethPri);
                break;

            default:
                em = new EthernetMatch(null, null, ethType, null, null);
                match.setField(mtype, Short.valueOf(ethType.shortValue()));
                break;
            }

            fm = new FlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(em, fm.getEthernetMatch());
            assertEquals(null, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
        }

        // L3 conditions (without netmask)
        InetAddress inetSrc = InetAddress.getByName("192.168.100.2");
        InetAddress inetDst = InetAddress.getByName("10.250.132.10");
        Short inetProto = Short.valueOf((short)6);
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

            fm = new FlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(proto, fm.getInetProtocol());
        }

        // L3 conditions (with netmask)
        Short inetSrcSuff = Short.valueOf((short)24);
        Short inetDstSuff = Short.valueOf((short)31);
        InetAddress srcSuff =
            NetUtils.getInetNetworkMask(inetSrcSuff.intValue(), false);
        InetAddress dstSuff =
            NetUtils.getInetNetworkMask(inetDstSuff.intValue(), false);

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

            fm = new FlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(null, fm.getLayer4Match());
            assertEquals(-1, fm.getInetProtocol());
        }

        // L4 conditions require NW_PROTO match.
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
                    new FlowMatch(match);
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

            fm = new FlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(tm, fm.getLayer4Match());
            assertEquals(ipproto.intValue(), fm.getInetProtocol());
        }

        // UDP conditions.
        Short udpSrc = Short.valueOf((short)40000);
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

            fm = new FlowMatch(match);
            assertEquals(null, fm.getIndex());
            assertEquals(null, fm.getEthernetMatch());
            assertEquals(im, fm.getInetMatch());
            assertEquals(um, fm.getLayer4Match());
            assertEquals(ipproto.intValue(), fm.getInetProtocol());
        }

        // ICMP conditions.
        Short icmpType = Short.valueOf((short)10);
        Short icmpCode = Short.valueOf((short)20);
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

            fm = new FlowMatch(match);
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

            fm = new FlowMatch(match);
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
                new FlowMatch(match);
                unexpected();
            } catch (IllegalArgumentException e) {
            }
        }
    }

    /**
     * Test case for {@link FlowMatch#equals(Object)} and
     * {@link FlowMatch#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        Integer[] indices = {
            null, Integer.valueOf(1), Integer.valueOf(65535)
        };
        List<EthernetMatch> etherMatches = createEthernetMatches();
        List<InetMatch> inetMatches = createInetMatches();
        List<L4Match> l4Matches = createL4Matches();

        for (Integer idx: indices) {
            for (EthernetMatch em: etherMatches) {
                for (InetMatch im: inetMatches) {
                    for (L4Match lm: l4Matches) {
                        FlowMatch fm1;
                        FlowMatch fm2;
                        if (idx == null) {
                            fm1 = new FlowMatch(em, im, lm);
                            fm2 = new FlowMatch(em, im, lm);
                        } else {
                            fm1 = new FlowMatch(idx.intValue(), copy(em),
                                                copy(im), copy(lm));
                            fm2 = new FlowMatch(idx.intValue(), copy(em),
                                                copy(im), copy(lm));
                        }
                        testEquals(set, fm1, fm2);
                    }
                }
            }
        }

        int required = indices.length * etherMatches.size() *
            inetMatches.size() * l4Matches.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link FlowMatch#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "FlowMatch[";
        String suffix = "]";

        Integer[] indices = {
            null, Integer.valueOf(1), Integer.valueOf(100),
            Integer.valueOf(65535)
        };

        for (Integer idx: indices) {
            for (EthernetMatch em: createEthernetMatches()) {
                for (InetMatch im: createInetMatches()) {
                    for (L4Match lm: createL4Matches()) {
                        FlowMatch fm;
                        String i;
                        if (idx == null) {
                            fm = new FlowMatch(em, im, lm);
                            i = null;
                        } else {
                            fm = new FlowMatch(idx.intValue(), em, im, lm);
                            i = "index=" + idx;
                        }
                        String e = (em == null) ? null : "ether=" + em;
                        String in = (im == null) ? null : "inet=" + im;
                        String l = (lm == null) ? null : "L4=" + lm;
                        String required = joinStrings(prefix, suffix, ",",
                                                      i, e, in, l);
                        assertEquals(required, fm.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link FlowMatch} is serializable.
     */
    @Test
    public void testSerialize() {
        Integer[] indices = {
            null, Integer.valueOf(1), Integer.valueOf(100),
            Integer.valueOf(65535)
        };

        for (Integer idx: indices) {
            for (EthernetMatch em: createEthernetMatches()) {
                for (InetMatch im: createInetMatches()) {
                    for (L4Match lm: createL4Matches()) {
                        FlowMatch fm = (idx == null)
                            ? new FlowMatch(em, im, lm)
                            : new FlowMatch(idx.intValue(), em, im, lm);
                        serializeTest(fm);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link FlowMatch} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        Integer[] indices = {
            null, Integer.valueOf(1), Integer.valueOf(100),
            Integer.valueOf(65535)
        };

        for (Integer idx: indices) {
            for (EthernetMatch em: createEthernetMatches()) {
                for (InetMatch im: createInetMatches()) {
                    for (L4Match lm: createL4Matches()) {
                        FlowMatch fm = (idx == null)
                            ? new FlowMatch(em, im, lm)
                            : new FlowMatch(idx.intValue(), em, im, lm);
                        jaxbTest(fm, "flowmatch");
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link FlowMatch} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        Integer[] indices = {
            null, Integer.valueOf(1), Integer.valueOf(100),
            Integer.valueOf(65535)
        };

        for (Integer idx: indices) {
            for (EthernetMatch em: createEthernetMatches()) {
                for (InetMatch im: createInetMatches()) {
                    for (L4Match lm: createL4Matches()) {
                        FlowMatch fm = (idx == null)
                            ? new FlowMatch(em, im, lm)
                            : new FlowMatch(idx.intValue(), em, im, lm);
                        jsonTest(fm);
                    }
                }
            }
        }
    }

    /**
     * Create a list of {@link EthernetMatch} instances.
     *
     * @return  A list of {@link EthernetMatch} instances.
     */
    private List<EthernetMatch> createEthernetMatches() {
        List<EthernetMatch> list = new ArrayList<EthernetMatch>();
        list.add(null);
        list.add(new EthernetMatch(null, null, null, null, null));

        byte[] dstMac = {
            (byte)0x00, (byte)0x01, (byte)0x02,
            (byte)0x03, (byte)0x04, (byte)0x05,
        };
        Integer type = null;
        Short vlan = Short.valueOf((short)0);
        Byte pri = null;

        try {
            EthernetAddress src = null;
            EthernetAddress dst = new EthernetAddress(dstMac);
            list.add(new EthernetMatch(src, dst, type, vlan, pri));

            byte[] srcMac = new byte[]{
                (byte)0xca, (byte)0x9c, (byte)0xeb,
                (byte)0xbd, (byte)0xb6, (byte)0xdc,
            };
            dstMac = new byte[]{
                (byte)0x38, (byte)0x03, (byte)0x80,
                (byte)0xc1, (byte)0x70, (byte)0xb6,
            };
            src = new EthernetAddress(srcMac);
            dst = new EthernetAddress(dstMac);
            type = Integer.valueOf(0x806);
            vlan = Short.valueOf((short)4095);
            pri = Byte.valueOf((byte)7);
            list.add(new EthernetMatch(src, dst, type, vlan, pri));
        } catch (Exception e) {
            unexpected(e);
        }

        return list;
    }

    /**
     * Create a list of {@link InetMatch} instances.
     *
     * @return  A list of {@link InetMatch} instances.
     */
    private List<InetMatch> createInetMatches() {
        List<InetMatch> list = new ArrayList<InetMatch>();
        list.add(null);
        list.add(new Inet4Match(null, null, null, null, null, null));

        Short srcsuff = null;
        Short dstsuff = Short.valueOf((short)8);
        Short proto = null;
        Byte dscp = Byte.valueOf((byte)63);

        try {
            InetAddress src = InetAddress.getByName("192.168.100.204");
            srcsuff = Short.valueOf((short)31);
            InetAddress dst = null;
            dstsuff = null;
            proto = Short.valueOf((short)6);
            dscp = null;
            list.add(new Inet4Match(src, srcsuff, dst, dstsuff, proto, dscp));

            src = InetAddress.getByName("192.168.243.0");
            srcsuff = Short.valueOf((short)24);
            dst = InetAddress.getByName("10.123.234.0");
            dstsuff = Short.valueOf((short)23);
            proto = Short.valueOf((short)1);
            dscp = Byte.valueOf((byte)50);
            list.add(new Inet4Match(src, srcsuff, dst, dstsuff, proto, dscp));
        } catch (Exception e) {
            unexpected(e);
        }

        return list;
    }

    /**
     * Create a list of {@link L4Match} instances.
     *
     * @return  A list of {@link L4Match} instances.
     */
    private List<L4Match> createL4Matches() {
        List<L4Match> list = new ArrayList<L4Match>();
        list.add(null);

        // TcpMatch
        PortMatch src = null;
        PortMatch dst = null;
        list.add(new TcpMatch(src, dst));

        src = new PortMatch(0, 1000);
        dst = new PortMatch(60000);
        list.add(new TcpMatch(src, dst));

        // UdpMatch
        src = null;
        dst = new PortMatch(3000, 50000);
        list.add(new UdpMatch(src, dst));

        src = new PortMatch(53);
        dst = null;
        list.add(new UdpMatch(src, dst));

        // IcmpMatch
        Short type = null;
        Short code = null;
        list.add(new IcmpMatch(type, code));

        type = Short.valueOf((short)8);
        code = Short.valueOf((short)0);
        list.add(new IcmpMatch(type, code));

        return list;
    }
}
