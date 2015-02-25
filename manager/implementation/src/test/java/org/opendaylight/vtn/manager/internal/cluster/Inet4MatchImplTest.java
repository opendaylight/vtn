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
import java.util.HashSet;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link Inet4MatchImpl}.
 */
public class Inet4MatchImplTest extends TestBase {
    /**
     * Test for {@link Inet4MatchImpl#Inet4MatchImpl(short)} and
     * getter methods.
     */
    @Test
    public void testConstructor1() {
        short[] protocols = {
            0, 1, 6, 17, 50, 129, 255,
        };
        for (short proto: protocols) {
            Inet4MatchImpl imi = new Inet4MatchImpl(proto);
            Short p = Short.valueOf(proto);
            assertEquals(null, imi.getSource());
            assertEquals(null, imi.getDestination());
            assertEquals(proto, imi.getProtocol());
            assertEquals(p, imi.getProtocolShort());
            assertEquals((byte)-1, imi.getDscp());
            assertEquals(null, imi.getDscpByte());
            assertEquals(EtherTypes.IPv4.intValue(), imi.getEtherType());

            InetMatch im = new Inet4Match(null, null, null, null, p, null);
            assertEquals(im, imi.getMatch());
        }
    }

    /**
     * Test for {@link Inet4MatchImpl#Inet4MatchImpl(InetMatch)},
     * {@link InetMatchImpl#create(InetMatch)}, and getter methods.n
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testConstructor2() throws VTNException {
        String[] srcs = {
            null, "10.0.0.2", "192.168.230.180",
        };
        short[] srcSuffs = {
            -1, 8, 24,
        };
        String[] dsts = {
            null, "10.20.30.40", "172.30.193.35",
        };
        short[] dstSuffs = {
            -1, 16, 23,
        };
        short[] protocols = {
            -1, 0, 6, 17,
        };
        byte[] dscps = {
            -1, 0, 30, 63,
        };

        Inet4MatchImplBuilder builder = new Inet4MatchImplBuilder();
        for (String src: srcs) {
            builder.setSourceAddress(src);
            for (short srcSuff: srcSuffs) {
                builder.setSourceSuffix(srcSuff);
                for (String dst: dsts) {
                    builder.setDestinationAddress(dst);
                    for (short dstSuff: dstSuffs) {
                        builder.setDestinationSuffix(dstSuff);
                        for (short proto: protocols) {
                            builder.setProtocol(proto);
                            for (byte dscp: dscps) {
                                builder.setDscp(dscp);
                                builder.verify();
                            }
                        }
                    }
                }
            }
        }

        // Specifying invalid MAC address via JAXB.
        String[] badAddrs = {
            "", "192.168.100.256", "::1", "bad_address",
        };
        Unmarshaller um = createUnmarshaller(Inet4Match.class);
        for (String addr: badAddrs) {
            for (String attr: new String[]{"src", "dst"}) {
                StringBuilder sb = new StringBuilder(XML_DECLARATION);
                String xml = sb.append("<inet4match ").append(attr).
                    append("=\"").append(addr).append("\" />").toString();

                InetMatch im = null;
                try {
                    im = unmarshal(um, xml, Inet4Match.class);
                } catch (Exception e) {
                    unexpected(e);
                }

                assertNotNull(im);
                assertNotNull(im.getValidationStatus());
                try {
                    new Inet4MatchImpl(im);
                    unexpected();
                } catch (VTNException e) {
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                }
            }
        }

        // Specifying invalid IP protocol number.
        Short[] badProtocols = {
            Short.valueOf(Short.MIN_VALUE), Short.valueOf((short)-1),
            Short.valueOf((short)0x100), Short.valueOf(Short.MAX_VALUE),
        };
        for (Short proto: badProtocols) {
            InetMatch im = new Inet4Match(null, null, null, null, proto, null);
            try {
                new Inet4MatchImpl(im);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid IP protocol number: " + proto,
                             st.getDescription());
            }
        }

        // Specifying invalid DSCP value.
        Byte[] badDscps = {
            Byte.valueOf(Byte.MIN_VALUE), Byte.valueOf((byte)-1),
            Byte.valueOf((byte)64), Byte.valueOf(Byte.MAX_VALUE),
        };
        for (Byte dscp: badDscps) {
            InetMatch im = new Inet4Match(null, null, null, null, null, dscp);
            try {
                new Inet4MatchImpl(im);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid DSCP field value: " + dscp,
                             st.getDescription());
            }
        }

        // Specifying invalid CIDR suffix.
        Short[] badSuffixes = {
            Short.valueOf(Short.MIN_VALUE), Short.valueOf((short)-1),
            Short.valueOf((short)0), Short.valueOf((short)32),
            Short.valueOf((short)100), Short.valueOf(Short.MAX_VALUE),
        };

        InetAddress iaddr = null;
        try {
            iaddr = InetAddress.getByName("192.168.100.200");
        } catch (Exception e) {
            unexpected(e);
        }

        for (Short suff: badSuffixes) {
            InetMatch im = new Inet4Match(iaddr, suff, null, null, null, null);
            try {
                new Inet4MatchImpl(im);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid CIDR suffix: " + suff,
                             st.getDescription());
            }

            im = new Inet4Match(null, null, iaddr, suff, null, null);
            try {
                new Inet4MatchImpl(im);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid CIDR suffix: " + suff,
                             st.getDescription());
            }
        }

        // Passing null to InetMatchImpl.create(InetMatch).
        try {
            InetMatchImpl.create(null);
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Unexpected inet match instance: null",
                         st.getDescription());
        }
    }

    /**
     * Test case for {@link Inet4MatchImpl#setProtocol(short)}.
     */
    @Test
    public void testSetProtocol() {
        short[] protocols = {
            0, 1, 6, 17, 254,
        };
        for (short proto: protocols) {
            Short p = Short.valueOf(proto);
            InetMatch im = new Inet4Match(null, null, null, null, p, null);
            Inet4MatchImpl imi = null;
            try {
                imi = new Inet4MatchImpl(im);
            } catch (VTNException e) {
                unexpected(e);
            }

            // New IP protocol can be set as long as existing protocol is not
            // changed.
            try {
                for (int i = 0; i < 4; i++) {
                    imi.setProtocol(proto);
                    assertEquals(proto, imi.getProtocol());
                    assertEquals(p, imi.getProtocolShort());
                }
            } catch (VTNException e) {
                unexpected(e);
            }

            short newproto = (short)(proto + 1);
            try {
                imi.setProtocol(newproto);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                String expected = "IP protocol conflict: proto=" + proto +
                    ", expected=" + newproto;
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals(expected, st.getDescription());
            }
        }
    }

    /**
     * Test case for {@link Inet4MatchImpl#equals(Object)} and
     * {@link Inet4MatchImpl#hashCode()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testEquals() throws VTNException {
        HashSet<Object> set = new HashSet<Object>();

        String[] srcs = {
            null, "10.0.0.2", "192.168.230.180",
        };
        short[] srcSuffs = {
            -1, 8, 24,
        };
        String[] dsts = {
            null, "10.20.30.40", "172.30.193.35",
        };
        short[] dstSuffs = {
            -1, 16, 23,
        };
        short[] protocols = {
            -1, 0, 6, 17,
        };
        byte[] dscps = {
            -1, 0, 30, 63,
        };

        int count = 0;
        Inet4MatchImplBuilder builder = new Inet4MatchImplBuilder();
        for (String src: srcs) {
            builder.setSourceAddress(src);
            for (short srcSuff: srcSuffs) {
                if (src == null && srcSuff != -1) {
                    continue;
                }
                builder.setSourceSuffix(srcSuff);
                for (String dst: dsts) {
                    builder.setDestinationAddress(dst);
                    for (short dstSuff: dstSuffs) {
                        if (dst == null && dstSuff != -1) {
                            continue;
                        }
                        builder.setDestinationSuffix(dstSuff);
                        for (short proto: protocols) {
                            builder.setProtocol(proto);
                            for (byte dscp: dscps) {
                                builder.setDscp(dscp);
                                Inet4MatchImpl imi1 = builder.create();
                                Inet4Match im = builder.getInet4Match();
                                Inet4MatchImpl imi2 = builder.create();
                                testEquals(set, imi1, imi2);
                                count++;
                            }
                        }
                    }
                }
            }
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link Inet4MatchImpl#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        String prefix = "Inet4MatchImpl[";
        String suffix = "]";

        String[] srcs = {
            null, "10.0.0.2", "192.168.230.180",
        };
        short[] srcSuffs = {
            -1, 8, 24,
        };
        String[] dsts = {
            null, "10.20.30.40", "172.30.193.35",
        };
        short[] dstSuffs = {
            -1, 16, 23,
        };
        short[] protocols = {
            -1, 0, 6, 17,
        };
        byte[] dscps = {
            -1, 0, 30, 63,
        };

        Inet4MatchImplBuilder builder = new Inet4MatchImplBuilder();
        for (String src: srcs) {
            builder.setSourceAddress(src);
            for (short srcSuff: srcSuffs) {
                builder.setSourceSuffix(srcSuff);
                String s;
                if (src == null) {
                    s = null;
                } else if (srcSuff == -1) {
                    s = "src=" + src;
                } else {
                    InetAddress iaddr = InetAddress.getByName(src);
                    InetAddress raddr =
                        Ip4Network.getNetworkAddress(iaddr, (int)srcSuff);
                    s = "src=" + raddr.getHostAddress() + "/" + srcSuff;
                }
                for (String dst: dsts) {
                    builder.setDestinationAddress(dst);
                    for (short dstSuff: dstSuffs) {
                        builder.setDestinationSuffix(dstSuff);
                        String d;
                        if (dst == null) {
                            d = null;
                        } else if (dstSuff == -1) {
                            d = "dst=" + dst;
                        } else {
                            InetAddress iaddr = InetAddress.getByName(dst);
                            InetAddress raddr = Ip4Network.
                                getNetworkAddress(iaddr, (int)dstSuff);
                            d = "dst=" + raddr.getHostAddress() + "/" +
                                dstSuff;
                        }
                        for (short proto: protocols) {
                            builder.setProtocol(proto);
                            String p = (proto == -1) ? null
                                : "proto=" + proto;
                            for (byte dscp: dscps) {
                                builder.setDscp(dscp);
                                String ds = (dscp == -1) ? null
                                    : "dscp=" + dscp;
                                Inet4MatchImpl imi = builder.create();
                                String required = joinStrings(
                                    prefix, suffix, ",", s, d, p, ds);
                                assertEquals(required, imi.toString());
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link Inet4MatchImpl} is serializable.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testSerialize() throws VTNException {
        String[] srcs = {
            null, "10.0.0.2", "192.168.230.180",
        };
        short[] srcSuffs = {
            -1, 8, 24,
        };
        String[] dsts = {
            null, "10.20.30.40", "172.30.193.35",
        };
        short[] dstSuffs = {
            -1, 16, 23,
        };
        short[] protocols = {
            -1, 0, 6, 17,
        };
        byte[] dscps = {
            -1, 0, 30, 63,
        };

        Inet4MatchImplBuilder builder = new Inet4MatchImplBuilder();
        for (String src: srcs) {
            builder.setSourceAddress(src);
            for (short srcSuff: srcSuffs) {
                builder.setSourceSuffix(srcSuff);
                for (String dst: dsts) {
                    builder.setDestinationAddress(dst);
                    for (short dstSuff: dstSuffs) {
                        builder.setDestinationSuffix(dstSuff);
                        for (short proto: protocols) {
                            builder.setProtocol(proto);
                            for (byte dscp: dscps) {
                                builder.setDscp(dscp);
                                Inet4MatchImpl imi = builder.create();
                                serializeTest(imi);
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link Inet4MatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        String saddr = "10.234.198.83";
        String daddr = "192.168.238.254";
        InetAddress src = InetAddress.getByName(saddr);
        InetAddress dst = InetAddress.getByName(daddr);
        short proto = 100;
        byte dscp = 35;
        byte[] payload = {
            (byte)0x04, (byte)0xce, (byte)0xfe, (byte)0x21,
            (byte)0xdc, (byte)0xb1, (byte)0x16, (byte)0x3b,
        };

        IPv4 ipv4 = createIPv4(src, dst, proto, dscp);
        ipv4.setRawPayload(payload);

        // Run tests with changing VLAN ID.
        short[] vlans = {
            MatchType.DL_VLAN_NONE, 10,
        };

        byte[] srcMac = {
            (byte)0x10, (byte)0x20, (byte)0x30,
            (byte)0x40, (byte)0x50, (byte)0x60,
        };
        byte[] dstMac = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xfa, (byte)0xfb, (byte)0xfc,
        };
        int type = EtherTypes.IPv4.intValue();
        byte pcp = 0;
        for (short vlan: vlans) {
            Ethernet pkt = createEthernet(srcMac, dstMac, type, vlan, pcp,
                                          ipv4);
            matchTest(pkt);

            // Test non-IPv4 packet.
            pkt = createEthernet(srcMac, dstMac, (short)100, vlan, pcp,
                                 payload);
            matchTest(pkt);
        }
    }

    /**
     * Run tests for {@link Inet4MatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}
     * using the given packet.
     *
     * @param pkt  An {@link Ethernet} instance for test.
     */
    private void matchTest(Ethernet pkt) {
        Packet payload = pkt.getPayload();
        if (payload instanceof IEEE8021Q) {
            IEEE8021Q tag = (IEEE8021Q)payload;
            payload = tag.getPayload();
        }

        PacketMatchTest test = new PacketMatchTest();
        Inet4MatchImplBuilder builder = new Inet4MatchImplBuilder();

        if (!(payload instanceof IPv4)) {
            // Should not match non-IPv4 packet.
            assertEquals(false, test.run(builder.create(), pkt));

            int src = 123;
            int dst = 456;
            builder.setSourceAddress(src).
                setDestinationAddress(dst).setProtocol((short)6).
                setDscp((byte)0);
            assertEquals(false, test.run(builder.create(), pkt));
            return;
        }

        Inet4Parser inet4Parser = new Inet4Parser((IPv4)payload);
        int src = inet4Parser.getSourceAddress();
        int dst = inet4Parser.getDestinationAddress();
        short proto = inet4Parser.getProtocol();
        byte dscp = inet4Parser.getDiffServ();

        int anotherSrc = inet4Parser.getAnotherSourceAddress();
        int anotherDst = inet4Parser.getAnotherDestinationAddress();
        short anotherProto = inet4Parser.getAnotherProtocol();
        byte anotherDscp = inet4Parser.getAnotherDiffServ();

        // Empty match should match every packet.
        assertEquals(true, test.run(builder.create(), pkt));

        // Specify single field in IPv4 header.
        Inet4MatchImpl imi = builder.reset().setSourceAddress(src).create();
        assertEquals(true, test.setMatchType(MatchType.NW_SRC).run(imi, pkt));
        imi = builder.reset().setSourceAddress(anotherSrc).create();
        assertEquals(false, test.run(imi, pkt));

        test.reset().setMatchType(MatchType.NW_DST);
        imi = builder.reset().setDestinationAddress(dst).create();
        assertEquals(true, test.run(imi, pkt));
        imi = builder.reset().setDestinationAddress(anotherDst).create();
        assertEquals(false, test.run(imi, pkt));

        test.reset().setMatchType(MatchType.NW_PROTO);
        imi = builder.reset().setProtocol(proto).create();
        assertEquals(true, test.run(imi, pkt));
        imi = builder.reset().setProtocol(anotherProto).create();
        assertEquals(false, test.run(imi, pkt));

        test.reset().setMatchType(MatchType.NW_TOS);
        imi = builder.reset().setDscp(dscp).create();
        assertEquals(true, test.run(imi, pkt));
        imi = builder.reset().setDscp(anotherDscp).create();
        assertEquals(false, test.run(imi, pkt));

        // Specify IP address by network address.
        for (short suff = 1; suff <= 31; suff++) {
            int mask = -1 << (32 - suff);

            int rsrc = src & mask;
            test.reset().setMatchType(MatchType.NW_SRC);
            imi = builder.reset().setSourceAddress(rsrc).
                setSourceSuffix(suff).create();
            assertEquals(true, test.run(imi, pkt));
            int badAddr = rsrc & ~0x80000000;
            imi = builder.reset().setSourceAddress(badAddr).create();
            assertEquals(false, test.run(imi, pkt));

            int rdst = dst & mask;
            test.reset().setMatchType(MatchType.NW_DST);
            imi = builder.reset().setDestinationAddress(rdst).
                setDestinationSuffix(suff).create();
            assertEquals(true, test.run(imi, pkt));
            badAddr = rdst & ~0x80000000;
            imi = builder.reset().setDestinationAddress(badAddr).create();
            assertEquals(false, test.run(imi, pkt));

            // Specify all fields.
            imi = builder.reset().setSourceAddress(src).setSourceSuffix(suff).
                setDestinationAddress(dst).
                setProtocol(proto).setDscp(dscp).create();
            test.reset().setMatchType(MatchType.NW_SRC, MatchType.NW_DST,
                                      MatchType.NW_PROTO, MatchType.NW_TOS);
            assertEquals(true, test.run(imi, pkt));

            imi = builder.reset().setSourceAddress(src).
                setDestinationAddress(dst).setDestinationSuffix(suff).
                setProtocol(proto).setDscp(dscp).create();
            assertEquals(true, test.run(imi, pkt));
        }

        // Specify all fields by implicit IP addresses.
        imi = builder.reset().setSourceAddress(src).setDestinationAddress(dst).
            setProtocol(proto).setDscp(dscp).create();
        test.reset().setMatchType(MatchType.NW_SRC, MatchType.NW_DST,
                                  MatchType.NW_PROTO, MatchType.NW_TOS);
        assertEquals(true, test.run(imi, pkt));

        // Ensure that match() returns false if one field does not match.
        test.reset().setMatchType(MatchType.NW_SRC);
        imi = builder.setSourceAddress(anotherSrc).create();
        assertEquals(false, test.run(imi, pkt));

        test.setMatchType(MatchType.NW_DST);
        imi = builder.setSourceAddress(src).setDestinationAddress(anotherDst).
            create();
        assertEquals(false, test.run(imi, pkt));

        test.setMatchType(MatchType.NW_PROTO);
        imi = builder.setDestinationAddress(dst).setProtocol(anotherProto)
            .create();
        assertEquals(false, test.run(imi, pkt));

        test.setMatchType(MatchType.NW_TOS);
        imi = builder.setProtocol(proto).setDscp(anotherDscp).create();
        assertEquals(false, test.run(imi, pkt));
    }
}

/**
 * Utility class to create {@link Inet4MatchImpl} instance.
 */
final class Inet4MatchImplBuilder extends TestBase {
    /**
     * Source IP address.
     */
    private InetAddress  sourceAddress;

    /**
     * CIDR suffix for source IP address.
     */
    private Short  sourceSuffix;

    /**
     * Destinaiton IP address.
     */
    private InetAddress  destinationAddress;

    /**
     * CIDR suffix for destination IP address.
     */
    private Short  destinationSuffix;

    /**
     * IP protocol number.
     */
    private Short  protocol;

    /**
     * DSCP field in the TOS field.
     */
    private Byte  dscp;

    /**
     * Created {@link Inet4Match} instance.
     */
    private Inet4Match  inet4Match;

    /**
     * Set source IP address.
     *
     * @param addr  An {@link InetAddress} instance.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setSourceAddress(InetAddress addr) {
        sourceAddress = addr;
        inet4Match = null;
        return this;
    }

    /**
     * Set source IP address.
     *
     * @param addr  An integer value which represents an IPv4 address.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setSourceAddress(int addr) {
        sourceAddress = MiscUtils.toInetAddress(addr);
        inet4Match = null;
        return this;
    }

    /**
     * Set source IP address.
     *
     * @param addr  A string representation of an IP address.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setSourceAddress(String addr) {
        if (addr == null) {
            sourceAddress = null;
        } else {
            try {
                sourceAddress = InetAddress.getByName(addr);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        inet4Match = null;
        return this;
    }

    /**
     * Set CIDR suffix for source address.
     *
     * @param suff  A CIDR suffix.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setSourceSuffix(short suff) {
        sourceSuffix = (suff == -1) ? null : Short.valueOf(suff);
        inet4Match = null;
        return this;
    }

    /**
     * Set destination IP address.
     *
     * @param addr  An integer value which represents an IPv4 address.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setDestinationAddress(int addr) {
        destinationAddress = MiscUtils.toInetAddress(addr);
        inet4Match = null;
        return this;
    }

    /**
     * Set destination IP address.
     *
     * @param addr  An {@link InetAddress} instance.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setDestinationAddress(InetAddress addr) {
        destinationAddress = addr;
        inet4Match = null;
        return this;
    }

    /**
     * Set destination IP address.
     *
     * @param addr  A string representation of an IP address.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setDestinationAddress(String addr) {
        if (addr == null) {
            destinationAddress = null;
        } else {
            try {
                destinationAddress = InetAddress.getByName(addr);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        inet4Match = null;
        return this;
    }

    /**
     * Set CIDR suffix for destination address.
     *
     * @param suff  A CIDR suffix.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setDestinationSuffix(short suff) {
        destinationSuffix = (suff == -1) ? null : Short.valueOf(suff);
        inet4Match = null;
        return this;
    }

    /**
     * Set IP protocol number.
     *
     * @param proto  IP protocol number.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setProtocol(short proto) {
        protocol = (proto == -1) ? null : Short.valueOf(proto);
        inet4Match = null;
        return this;
    }

    /**
     * Set DSCP field value.
     *
     * @param d  DSCP field value.
     * @return  This instance.
     */
    public Inet4MatchImplBuilder setDscp(byte d) {
        dscp = (d == -1) ? null : Byte.valueOf(d);
        inet4Match = null;
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public Inet4MatchImplBuilder reset() {
        sourceAddress = null;
        sourceSuffix = null;
        destinationAddress = null;
        destinationSuffix = null;
        protocol = null;
        dscp = null;
        inet4Match = null;

        return this;
    }

    /**
     * Construct an {@link Inet4Match} instance.
     *
     * @return  An {@link Inet4Match} instance.
     */
    public Inet4Match getInet4Match() {
        Inet4Match im = inet4Match;
        if (im == null) {
            im = new Inet4Match(sourceAddress, sourceSuffix,
                                destinationAddress, destinationSuffix,
                                protocol, dscp);
            inet4Match = im;
        }

        return im;
    }

    /**
     * Return an {@link Inet4Match} instance to be expected to be returned
     * by the call of {@link Inet4MatchImpl#getMatch()}.
     *
     * @return  An {@link Inet4Match} instance.
     */
    public Inet4Match getExpectedInet4Match() {
        InetAddress src = sourceAddress;
        Short srcSuff = sourceSuffix;
        if (src == null) {
            // Source suffix should be ignored.
            srcSuff = null;
        } else if (srcSuff != null) {
            src = Ip4Network.getNetworkAddress(src, srcSuff.intValue());
        }

        InetAddress dst = destinationAddress;
        Short dstSuff = destinationSuffix;
        if (dst == null) {
            // Destination suffix should be ignored.
            dstSuff = null;
        } else if (dstSuff != null) {
            dst = Ip4Network.getNetworkAddress(dst, dstSuff.intValue());
        }

        return new Inet4Match(src, srcSuff, dst, dstSuff, protocol, dscp);
    }

    /**
     * Construct an {@link Inet4MatchImpl} instance.
     *
     * @return  An {@link Inet4MatchImpl} instance.
     * @throws VTNException  An error occurred.
     */
    public Inet4MatchImpl build() throws VTNException {
        InetMatch im = getInet4Match();
        return new Inet4MatchImpl(im);
    }

    /**
     * Construct an {@link Inet4MatchImpl} instance and ensure that it was
     * constructed successfully.
     *
     * @return  An {@link Inet4MatchImpl} instance.
     */
    public Inet4MatchImpl create() {
        Inet4MatchImpl imi = null;
        try {
            imi = build();
        } catch (Exception e) {
            unexpected(e);
        }

        return imi;
    }

    /**
     * Construct an {@link Inet4MatchImpl} instance, and verify it.
     */
    public void verify() {
        Inet4MatchImpl imi = create();
        assertEquals(EtherTypes.IPv4.intValue(), imi.getEtherType());
        Inet4Match im = getExpectedInet4Match();
        assertEquals(im, imi.getMatch());

        Inet4AddressMatch am = imi.getSource();
        if (sourceAddress == null) {
            assertEquals(null, am);
        } else {
            verify(am, im.getSourceAddress(), im.getSourceSuffix());
        }

        am = imi.getDestination();
        if (destinationAddress == null) {
            assertEquals(null, am);
        } else {
            verify(am, im.getDestinationAddress(), im.getDestinationSuffix());
        }

        short p = (protocol == null) ? -1 : protocol.shortValue();
        assertEquals(p, imi.getProtocol());
        assertEquals(protocol, imi.getProtocolShort());

        byte d = (dscp == null) ? -1 : dscp.byteValue();
        assertEquals(d, imi.getDscp());
        assertEquals(dscp, imi.getDscpByte());

        // Instantiate using InetMatchImpl.create().
        try {
            assertEquals(imi, InetMatchImpl.create(im));
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Verify contents of the given {@link Inet4AddressMatch} instance.
     *
     * @param am    A {@link Inet4AddressMatch} instance.
     * @param addr  Expected network address.
     * @param suff  Expected CIDR suffix.
     */
    private void verify(Inet4AddressMatch am, InetAddress addr, Short suff) {
        assertEquals(addr, am.getInetAddress());
        assertEquals(MiscUtils.toInteger(addr), am.getAddress());
        assertEquals(suff, am.getCidrSuffix());
        assertEquals(toNetMask(suff), am.getMask());
    }

    /**
     * Convert the given CIDR suffix into netmask.
     *
     * @param suff  A CIDR suffix.
     * @return  A netmask value.
     */
    private int toNetMask(Short suff) {
        if (suff == null) {
            return -1;
        }

        InetAddress mask = Ip4Network.getInetMask(suff.intValue());
        return MiscUtils.toInteger(mask);
    }
}

/**
 * IPv4 header parser.
 */
final class Inet4Parser {
    /**
     * IPv4 packet.
     */
    private final IPv4  packet;

    /**
     * Source IP address.
     */
    private int  sourceAddress;

    /**
     * Destination IP address.
     */
    private int  destinationAddress;

    /**
     * IP protocol number.
     */
    private final short  protocol;

    /**
     * DSCP field value.
     */
    private final byte  diffServ;

    /**
     * Construct a new instance.
     *
     * @param ipv4  An {@link IPv4} instance.
     */
    Inet4Parser(IPv4 ipv4) {
        packet = ipv4;
        sourceAddress = ipv4.getSourceAddress();
        destinationAddress = ipv4.getDestinationAddress();
        protocol = (short)NumberUtils.getUnsigned(ipv4.getProtocol());
        diffServ = ipv4.getDiffServ();
    }

    /**
     * Return the IPv4 packet.
     *
     * @return  An {@link IPv4} instance.
     */
    public IPv4 getPacket() {
        return packet;
    }

    /**
     * Return the payload of the IPv4 packet.
     *
     * @return  A {@link Packet} instance that represents the payload of this
     *          IPv4 packet.
     */
    public Packet getPayload() {
        return packet.getPayload();
    }

    /**
     * Return the source IP address.
     *
     * @return  An integer which represents the source IP address.
     */
    public int getSourceAddress() {
        return sourceAddress;
    }

    /**
     * Return the destination IP address.
     *
     * @return  An integer which represents the destination IP address.
     */
    public int getDestinationAddress() {
        return destinationAddress;
    }

    /**
     * Return the IP protocol number.
     *
     * @return  IP protocol number.
     */
    public short getProtocol() {
        return protocol;
    }

    /**
     * Return the DSCP field value.
     *
     * @return  DSCP value.
     */
    public byte getDiffServ() {
        return diffServ;
    }

    /**
     * Return a source IP address that does not match this packet.
     *
     * @return  A source IP address that does not match this packet.
     */
    public int getAnotherSourceAddress() {
        return ~sourceAddress;
    }

    /**
     * Return a destination IP address that does not match this packet.
     *
     * @return  A destination IP address that does not match this packet.
     */
    public int getAnotherDestinationAddress() {
        return ~destinationAddress;
    }

    /**
     * Return an IP protocol number that does not match this packet.
     *
     * @return  An IP protocol number that does not match this packet.
     */
    public short getAnotherProtocol() {
        return (short)(~protocol & 0xff);
    }

    /**
     * Return a DSCP field value that does not match this packet.
     *
     * @return  A DSCP field value that does not match this packet.
     */
    public byte getAnotherDiffServ() {
        return (byte)(~diffServ & 0x3f);
    }
}
