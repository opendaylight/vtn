/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlAttributeType;
import org.opendaylight.vtn.manager.XmlDataType;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link FlowMatch}.
 */
public class FlowMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link FlowMatch} class.
     */
    private static final String  XML_ROOT = "flowmatch";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link FlowMatch} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        String[] p = XmlDataType.addPath(name, parent);
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "index", Integer.class).
                  add(parent));
        dlist.addAll(EthernetMatchTest.getXmlDataTypes("ethernet", p));
        dlist.addAll(Inet4MatchTest.getXmlDataTypes("inet4", p));
        dlist.addAll(TcpMatchTest.getXmlDataTypes("tcp", p));
        dlist.addAll(UdpMatchTest.getXmlDataTypes("udp", p));
        dlist.addAll(IcmpMatchTest.getXmlDataTypes("icmp", p));
        return dlist;
    }

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
                        jaxbTest(fm, FlowMatch.class, XML_ROOT);
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(FlowMatch.class, getXmlDataTypes(XML_ROOT));
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
                        jsonTest(fm, FlowMatch.class);
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
        EtherAddress nullMac = null;
        list.add(null);
        list.add(new EthernetMatch(nullMac, nullMac, null, null, null));

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
