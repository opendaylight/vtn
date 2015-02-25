/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link EthernetMatchImpl}.
 */
public class EthernetMatchImplTest extends TestBase {
    /**
     * Test for {@link EthernetMatchImpl#EthernetMatchImpl(int)} and
     * getter methods.
     */
    @Test
    public void testConstructor1() {
        int[] types = {
            0, 1, 0x800, 0x806, 0x86dd,
        };
        for (int type: types) {
            EthernetMatchImpl emi = new EthernetMatchImpl(type);
            assertEquals(-1L, emi.getSourceAddress());
            assertEquals(-1L, emi.getDestinationAddress());
            assertEquals(type, emi.getEtherType());
            assertEquals((short)-1, emi.getVlan());
            assertEquals((byte)-1, emi.getVlanPriority());

            Integer t = Integer.valueOf(type);
            EthernetMatch em = new EthernetMatch(null, null, t, null, null);
            assertEquals(em, emi.getMatch());
        }
    }

    /**
     * Test for {@link EthernetMatchImpl#EthernetMatchImpl(EthernetMatch)} and
     * getter methods.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testConstructor2() throws VTNException {
        long[] srcs = {-1L, 1L, 0x000102030405L};
        long[] dsts = {-1L, 0xf0f1f2f3f4f5L, 0xa8b9cadbecfdL};
        int[] types = {-1, 0x800, 0x86dd};
        short[] vlans = {-1, 0, 1, 4095};
        byte[] priorities = {0, 3, 7};

        EthernetMatchImplBuilder builder = new EthernetMatchImplBuilder();
        for (long src: srcs) {
            builder.setSourceAddress(src);
            for (long dst: dsts) {
                builder.setDestinationAddress(dst);
                for (int type: types) {
                    builder.setEtherType(type);
                    for (short vlan: vlans) {
                        builder.setVlanId(vlan);
                        builder.setVlanPriority((byte)-1);
                        EthernetMatchImpl emi = builder.create();
                        assertEquals(src, emi.getSourceAddress());
                        assertEquals(dst, emi.getDestinationAddress());
                        assertEquals(type, emi.getEtherType());
                        assertEquals(vlan, emi.getVlan());
                        assertEquals((byte)-1, emi.getVlanPriority());
                        assertEquals(builder.getEthernetMatch(),
                                     emi.getMatch());

                        if (vlan <= 0) {
                            continue;
                        }

                        for (byte pri: priorities) {
                            builder.setVlanPriority(pri);
                            emi = builder.create();
                            assertEquals(src, emi.getSourceAddress());
                            assertEquals(dst, emi.getDestinationAddress());
                            assertEquals(type, emi.getEtherType());
                            assertEquals(vlan, emi.getVlan());
                            assertEquals(pri, emi.getVlanPriority());
                            assertEquals(builder.getEthernetMatch(),
                                         emi.getMatch());
                        }
                    }
                }
            }
        }

        // Specifying invalid MAC address via JAXB.
        String[] badAddrs = {
            "", "aa:bb:cc:dd:ee:ff:11", "00:11", "bad_address",
        };
        Unmarshaller um = createUnmarshaller(EthernetMatch.class);
        for (String addr: badAddrs) {
            for (String attr: new String[]{"src", "dst"}) {
                StringBuilder sb = new StringBuilder(XML_DECLARATION);
                String xml = sb.append("<ethermatch ").append(attr).
                    append("=\"").append(addr).append("\" />").toString();
                EthernetMatch em = null;
                try {
                    em = unmarshal(um, xml, EthernetMatch.class);
                } catch (Exception e) {
                    unexpected(e);
                }

                assertNotNull(em);
                assertNotNull(em.getValidationStatus());
                try {
                    new EthernetMatchImpl(em);
                    unexpected();
                } catch (VTNException e) {
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                }
            }
        }

        // Specifying invalid ethernet type.
        Integer[] badTypes = {
            Integer.valueOf(Integer.MIN_VALUE), Integer.valueOf(-1),
            Integer.valueOf(0x10000), Integer.valueOf(Integer.MAX_VALUE),
        };
        for (Integer type: badTypes) {
            EthernetMatch em = new EthernetMatch(null, null, type, null, null);
            try {
                new EthernetMatchImpl(em);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid Ethernet type: " + type,
                             st.getDescription());
            }
        }

        // Specifying invalid VLAN ID.
        Short[] badVlans = {
            Short.valueOf(Short.MIN_VALUE), Short.valueOf((short)-1),
            Short.valueOf((short)4096), Short.valueOf(Short.MAX_VALUE),
        };
        for (Short vlan: badVlans) {
            EthernetMatch em = new EthernetMatch(null, null, null, vlan, null);
            try {
                new EthernetMatchImpl(em);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid VLAN ID: " + vlan, st.getDescription());
            }
        }

        // Specifying invalid VLAN priority.
        Short validVlan = Short.valueOf((short)1);
        Byte[] badPriorities = {
            Byte.valueOf(Byte.MIN_VALUE), Byte.valueOf((byte)-1),
            Byte.valueOf((byte)8), Byte.valueOf(Byte.MAX_VALUE),
        };
        for (Byte pri: badPriorities) {
            EthernetMatch em = new EthernetMatch(null, null, null, validVlan,
                                                 pri);
            try {
                new EthernetMatchImpl(em);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid VLAN priority: " + pri,
                             st.getDescription());
            }
        }

        // Specifying VLAN priority without specifying valid VLAN ID.
        Short[] noVlans = {
            null, Short.valueOf((short)0),
        };
        for (Short vlan: noVlans) {
            for (byte p = 0; p <= 7; p++) {
                Byte pri = Byte.valueOf(p);
                EthernetMatch em = new EthernetMatch(null, null, null, vlan,
                                                     pri);
                try {
                    new EthernetMatchImpl(em);
                    unexpected();
                } catch (VTNException e) {
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    assertEquals("VLAN priority requires a valid VLAN ID.",
                                 st.getDescription());
                }
            }
        }
    }

    /**
     * Test case for {@link EthernetMatchImpl#setEtherType(int)}.
     */
    @Test
    public void testSetEtherType() {
        int[] types = {
            0, 1, 0x800, 0x806, 0x86dd,
        };
        for (int type: types) {
            EthernetMatch em = new EthernetMatch(null, null, null, null, null);
            EthernetMatchImpl emi = null;
            try {
                emi = new EthernetMatchImpl(em);
            } catch (VTNException e) {
                unexpected(e);
            }

            // New ethernet type can be set as long as existing type is not
            // changed.
            try {
                for (int i = 0; i < 4; i++) {
                    emi.setEtherType(type);
                    assertEquals(type, emi.getEtherType());
                }
            } catch (VTNException e) {
                unexpected(e);
            }

            try {
                emi.setEtherType(type + 1);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                String expected = "Ethernet type conflict: type=0x" +
                    Integer.toHexString(type) + ", expected=0x" +
                    Integer.toHexString(type + 1);
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals(expected, st.getDescription());
            }
        }
    }

    /**
     * Test case for {@link EthernetMatchImpl#equals(Object)} and
     * {@link EthernetMatchImpl#hashCode()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testEquals() throws VTNException {
        HashSet<Object> set = new HashSet<Object>();
        long[] srcs = {-1L, 1L, 0x000102030405L};
        long[] dsts = {-1L, 0xf0f1f2f3f4f5L, 0xa8b9cadbecfdL};
        int[] types = {-1, 0x800, 0x86dd};
        short[] vlans = {-1, 0, 1, 4095};
        byte[] priorities = {0, 3, 7};
        int count = 0;
        for (long src: srcs) {
            EthernetAddress ethSrc = createEthernetAddress(src);
            for (long dst: dsts) {
                EthernetAddress ethDst = createEthernetAddress(dst);
                for (int type: types) {
                    Integer ethType = (type == -1) ? null
                        : Integer.valueOf(type);
                    for (short vlan: vlans) {
                        Short ethVlan = (vlan == -1) ? null
                            : Short.valueOf(vlan);
                        EthernetMatch em = new EthernetMatch(
                            ethSrc, ethDst, ethType, ethVlan, null);
                        EthernetMatchImpl emi1 = new EthernetMatchImpl(em);
                        EthernetMatchImpl emi2 = new EthernetMatchImpl(em);
                        testEquals(set, emi1, emi2);
                        count++;

                        if (vlan <= 0) {
                            continue;
                        }

                        for (byte pri: priorities) {
                            Byte ethPri = Byte.valueOf(pri);
                            em = new EthernetMatch(ethSrc, ethDst, ethType,
                                                   ethVlan, ethPri);
                            emi1 = new EthernetMatchImpl(em);
                            emi2 = new EthernetMatchImpl(em);
                            testEquals(set, emi1, emi2);
                            count++;
                        }
                    }
                }
            }
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link EthernetMatchImpl#toString()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testToString() throws VTNException {
        String prefix = "EthernetMatchImpl[";
        String suffix = "]";

        long[] srcs = {-1L, 1L, 0x000102030405L};
        long[] dsts = {-1L, 0xf0f1f2f3f4f5L, 0xa8b9cadbecfdL};
        int[] types = {-1, 0x800, 0x86dd};
        short[] vlans = {-1, 0, 1, 4095};
        byte[] priorities = {0, 3, 7};

        for (long src: srcs) {
            EthernetAddress ethSrc = createEthernetAddress(src);
            String s = (src == -1L) ? null
                : "src=" + MiscUtils.formatMacAddress(src);
            for (long dst: dsts) {
                EthernetAddress ethDst = createEthernetAddress(dst);
                String d = (dst == -1L) ? null
                    : "dst=" + MiscUtils.formatMacAddress(dst);
                for (int type: types) {
                    Integer ethType;
                    String t;
                    if (type == -1) {
                        ethType = null;
                        t = null;
                    } else {
                        ethType = Integer.valueOf(type);
                        t = "type=0x" + Integer.toHexString(type);
                    }
                    for (short vlan: vlans) {
                        Short ethVlan;
                        String v;
                        if (vlan == -1) {
                            ethVlan = null;
                            v = null;
                        } else {
                            ethVlan = Short.valueOf(vlan);
                            v = "vlan=" + vlan;
                        }

                        EthernetMatch em = new EthernetMatch(
                            ethSrc, ethDst, ethType, ethVlan, null);
                        EthernetMatchImpl emi = new EthernetMatchImpl(em);
                        String required = joinStrings(prefix, suffix, ",",
                                                      s, d, t, v);
                        assertEquals(required, emi.toString());

                        if (vlan <= 0) {
                            continue;
                        }

                        for (byte pri: priorities) {
                            Byte ethPri = Byte.valueOf(pri);
                            String p = "pcp=" + pri;
                            em = new EthernetMatch(ethSrc, ethDst, ethType,
                                                   ethVlan, ethPri);
                            emi = new EthernetMatchImpl(em);
                            required = joinStrings(prefix, suffix, ",",
                                                   s, d, t, v, p);
                            assertEquals(required, emi.toString());
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link EthernetMatchImpl} is serializable.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testSerialize() throws VTNException {
        long[] srcs = {-1L, 1L, 0x000102030405L};
        long[] dsts = {-1L, 0xf0f1f2f3f4f5L, 0xa8b9cadbecfdL};
        int[] types = {-1, 0x800, 0x86dd};
        short[] vlans = {-1, 0, 1, 4095};
        byte[] priorities = {0, 3, 7};

        for (long src: srcs) {
            EthernetAddress ethSrc = createEthernetAddress(src);
            for (long dst: dsts) {
                EthernetAddress ethDst = createEthernetAddress(dst);
                for (int type: types) {
                    Integer ethType = (type == -1) ? null
                        : Integer.valueOf(type);
                    for (short vlan: vlans) {
                        Short ethVlan = (vlan == -1) ? null
                            : Short.valueOf(vlan);
                        EthernetMatch em = new EthernetMatch(
                            ethSrc, ethDst, ethType, ethVlan, null);
                        EthernetMatchImpl emi = new EthernetMatchImpl(em);
                        serializeTest(emi);

                        if (vlan <= 0) {
                            continue;
                        }

                        for (byte pri: priorities) {
                            Byte ethPri = Byte.valueOf(pri);
                            em = new EthernetMatch(ethSrc, ethDst, ethType,
                                                   ethVlan, ethPri);
                            emi = new EthernetMatchImpl(em);
                            serializeTest(emi);
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link EthernetMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testMatch() throws VTNException {
        byte[] src = {
            (byte)0x10, (byte)0x20, (byte)0x30,
            (byte)0x40, (byte)0x50, (byte)0x60,
        };
        byte[] dst = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xfa, (byte)0xfb, (byte)0xfc,
        };
        int type = 0x0806;
        byte pcp = 3;

        // Run tests with an ARP packet.
        short[] vlans = {
            MatchType.DL_VLAN_NONE, 1, 10, 200, 3000, 4095,
        };
        for (short vlan: vlans) {
            Ethernet pkt = createEthernet(src, dst, type, vlan, pcp,
                                          PacketMatchTest.ARP_PACKET);
            matchTest(pkt);
        }
    }

    /**
     * Run tests for {@link EthernetMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}
     * using the given packet.
     *
     * @param pkt  An {@link Ethernet} instance for test.
     */
    private void matchTest(Ethernet pkt) {
        EthernetParser ethParser = new EthernetParser(pkt);
        byte[] src = ethParser.getSourceAddress();
        byte[] dst = ethParser.getDestinationAddress();
        int type = ethParser.getEtherType();
        short vlan = ethParser.getVlanId();
        byte pcp = ethParser.getVlanPcp();

        byte[] anotherSrc = ethParser.getAnotherSourceAddress();
        byte[] anotherDst = ethParser.getAnotherDestinationAddress();
        int anotherType = ethParser.getAnotherEtherType();
        short anotherVlan = ethParser.getAnotherVlanId();
        byte anotherPcp = ethParser.getAnotherVlanPcp();

        PacketMatchTest test = new PacketMatchTest();
        EthernetMatchImplBuilder builder = new EthernetMatchImplBuilder();

        // Empty match should match every packet.
        assertEquals(true, test.run(builder.create(), pkt));

        // Specify single field in Ethernet frame.
        EthernetMatchImpl emi = builder.reset().setSourceAddress(src).create();
        assertEquals(true, test.setMatchType(MatchType.DL_SRC).run(emi, pkt));
        emi = builder.reset().setSourceAddress(anotherSrc).create();
        assertEquals(false, test.run(emi, pkt));

        test.reset().setMatchType(MatchType.DL_DST);
        emi = builder.reset().setDestinationAddress(dst).create();
        assertEquals(true, test.run(emi, pkt));
        emi = builder.reset().setDestinationAddress(anotherDst).create();
        assertEquals(false, test.run(emi, pkt));

        test.reset().setMatchType(MatchType.DL_TYPE);
        emi = builder.reset().setEtherType(type).create();
        assertEquals(true, test.run(emi, pkt));
        emi = builder.reset().setEtherType(anotherType).create();
        assertEquals(false, test.run(emi, pkt));

        // DL_VLAN should never be configured in PacketContext.
        test.reset();
        if (vlan == MatchType.DL_VLAN_NONE) {
            emi = builder.reset().setVlanId(vlan).create();
            assertEquals(true, test.run(emi, pkt));

            short[] anotherVids = {1, 10, 300, 2345, 4095};
            for (short vid: anotherVids) {
                emi = builder.reset().setVlanId(vid).create();
                assertEquals(false, test.run(emi, pkt));
                emi = builder.setVlanPriority((byte)2).create();
                assertEquals(false, test.run(emi, pkt));
            }
        } else {
            emi = builder.reset().setVlanId(vlan).create();
            assertEquals(true, test.run(emi, pkt));

            short[] anotherVids = {
                MatchType.DL_VLAN_NONE,
                anotherVlan,
            };
            for (short vid: anotherVids) {
                emi = builder.reset().setVlanId(vid).create();
                assertEquals(false, test.run(emi, pkt));
            }

            short[] vids = {vlan, anotherVlan};
            for (short vid: vids) {
                builder.reset().setVlanId(vid);
                test.reset();
                if (vid == vlan) {
                    test.setMatchType(MatchType.DL_VLAN_PR);
                }
                for (byte pri = 0; pri <= 7; pri++) {
                    boolean expected = (vid == vlan && pri == pcp);
                    emi = builder.setVlanPriority(pri).create();
                    assertEquals(expected, test.run(emi, pkt));
                }
            }
        }

        // Specify all fields.
        builder.reset().setSourceAddress(src).setDestinationAddress(dst).
            setEtherType(type).setVlanId(vlan);
        test.reset().setMatchType(MatchType.DL_SRC, MatchType.DL_DST,
                                  MatchType.DL_TYPE);
        if (vlan != MatchType.DL_VLAN_NONE) {
            builder.setVlanPriority(pcp);
            test.setMatchType(MatchType.DL_VLAN_PR);
        }

        emi = builder.create();
        assertEquals(true, test.run(emi, pkt));

        // Ensure that match() returns false if one field does not match.
        test.reset().setMatchType(MatchType.DL_SRC);
        emi = builder.setSourceAddress(anotherSrc).create();
        assertEquals(false, test.run(emi, pkt));

        test.setMatchType(MatchType.DL_DST);
        emi = builder.setSourceAddress(src).setDestinationAddress(anotherDst).
            create();
        assertEquals(false, test.run(emi, pkt));

        test.setMatchType(MatchType.DL_TYPE);
        emi = builder.setDestinationAddress(dst).setEtherType(anotherType).
            create();
        assertEquals(false, test.run(emi, pkt));

        emi = builder.setEtherType(type).setVlanId(anotherVlan).create();
        assertEquals(false, test.run(emi, pkt));

        emi = builder.setVlanId(vlan).setVlanPriority(anotherPcp).create();
        boolean expected = (vlan == MatchType.DL_VLAN_NONE);
        if (!expected) {
            test.setMatchType(MatchType.DL_VLAN_PR);
        }
        assertEquals(expected, test.run(emi, pkt));
    }
}

/**
 * Utility class to create {@link EthernetMatchImpl} instance.
 */
final class EthernetMatchImplBuilder extends TestBase {
    /**
     * Source MAC address.
     */
    private EthernetAddress  sourceAddress;

    /**
     * Destinaiton MAC address.
     */
    private EthernetAddress  destinationAddress;

    /**
     * Ethernet type.
     */
    private Integer  etherType;

    /**
     * VLAN ID.
     */
    private Short  vlanId;

    /**
     * VLAN priority.
     */
    private Byte  vlanPriority;

    /**
     * Created {@link EthernetMatch} instance.
     */
    private EthernetMatch  ethernetMatch;

    /**
     * Set source MAC address.
     *
     * @param addr  A long integer value which represents a MAC address.
     * @return  This instance.
     */
    public EthernetMatchImplBuilder setSourceAddress(long addr) {
        sourceAddress = createEthernetAddress(addr);
        ethernetMatch = null;
        return this;
    }

    /**
     * Set source MAC address.
     *
     * @param addr  A byte array which represents a MAC address.
     * @return  This instance.
     */
    public EthernetMatchImplBuilder setSourceAddress(byte[] addr) {
        sourceAddress = createEthernetAddress(addr);
        ethernetMatch = null;
        return this;
    }

    /**
     * Set destination MAC address.
     *
     * @param addr  A long integer value which represents a MAC address.
     * @return  This instance.
     */
    public EthernetMatchImplBuilder setDestinationAddress(long addr) {
        destinationAddress = createEthernetAddress(addr);
        ethernetMatch = null;
        return this;
    }

    /**
     * Set destination MAC address.
     *
     * @param addr  A byte array which represents a MAC address.
     * @return  This instance.
     */
    public EthernetMatchImplBuilder setDestinationAddress(byte[] addr) {
        destinationAddress = createEthernetAddress(addr);
        ethernetMatch = null;
        return this;
    }

    /**
     * Set ethernet type.
     *
     * @param type  Ethernet type.
     * @return  This instance.
     */
    public EthernetMatchImplBuilder setEtherType(int type) {
        etherType = (type == -1) ? null : Integer.valueOf(type);
        ethernetMatch = null;
        return this;
    }

    /**
     * Set VLAN ID.
     *
     * @param vid  VLAN ID.
     * @return  This instance.
     */
    public EthernetMatchImplBuilder setVlanId(short vid) {
        vlanId = (vid == -1) ? null : Short.valueOf(vid);
        ethernetMatch = null;
        return this;
    }

    /**
     * Set VLAN priority.
     *
     * @param pcp  VLAN priority.
     * @return  This instance.
     */
    public EthernetMatchImplBuilder setVlanPriority(byte pcp) {
        vlanPriority = (pcp == -1) ? null : Byte.valueOf(pcp);
        ethernetMatch = null;
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public EthernetMatchImplBuilder reset() {
        sourceAddress = null;
        destinationAddress = null;
        etherType = null;
        vlanId = null;
        vlanPriority = null;
        ethernetMatch = null;

        return this;
    }

    /**
     * Construct an {@link EthernetMatch} instance.
     *
     * @return  An {@link EthernetMatch} instance.
     */
    public EthernetMatch getEthernetMatch() {
        EthernetMatch em = ethernetMatch;
        if (em == null) {
            em = new EthernetMatch(sourceAddress, destinationAddress,
                                   etherType, vlanId, vlanPriority);
            ethernetMatch = em;
        }

        return em;
    }

    /**
     * Construct an {@link EthernetMatchImpl} instance.
     *
     * @return  An {@link EthernetMatchImpl} instance.
     * @throws VTNException  An error occurred.
     */
    public EthernetMatchImpl build() throws VTNException {
        EthernetMatch em = getEthernetMatch();
        return new EthernetMatchImpl(em);
    }

    /**
     * Construct an {@link EthernetMatchImpl} instance and ensure that it was
     * constructed successfully.
     *
     * @return  An {@link EthernetMatchImpl} instance.
     */
    public EthernetMatchImpl create() {
        EthernetMatchImpl emi = null;
        try {
            emi = build();
        } catch (Exception e) {
            unexpected(e);
        }

        return emi;
    }
}

/**
 * Ethernet header parser.
 */
final class EthernetParser {
    /**
     * Ethernet frame.
     */
    private final Ethernet  ethernet;

    /**
     * Source MAC address.
     */
    private final byte[]  sourceAddress;

    /**
     * Destination MAC address.
     */
    private final byte[]  destinationAddress;

    /**
     * Ethernet type.
     */
    private final int  etherType;

    /**
     * VLAN ID.
     */
    private final short  vlanId;

    /**
     * VLAN priority.
     */
    private final byte  vlanPcp;

    /**
     * Payload of the ethernet frame.
     */
    private final Packet  payload;

    /**
     * Construct a new instance.
     *
     * @param ether  An ethernet frame.
     */
    EthernetParser(Ethernet ether) {
        ethernet = ether;
        sourceAddress = ether.getSourceMACAddress();
        destinationAddress = ether.getDestinationMACAddress();

        Packet pld = ether.getPayload();
        if (pld instanceof IEEE8021Q) {
            IEEE8021Q tag = (IEEE8021Q)pld;
            vlanId = tag.getVid();
            vlanPcp = tag.getPcp();
            etherType = NumberUtils.getUnsigned(tag.getEtherType());
            payload = tag.getPayload();
        } else {
            vlanId = MatchType.DL_VLAN_NONE;
            vlanPcp = -1;
            etherType = NumberUtils.getUnsigned(ether.getEtherType());
            payload = pld;
        }
    }

    /**
     * Return the ethernet frame configured in this instance.
     *
     * @return  An {@link Ethernet} instance.
     */
    public Ethernet getEthernet() {
        return ethernet;
    }

    /**
     * Return the source MAC address.
     *
     * @return  A byte array which represents the source MAC address.
     */
    public byte[] getSourceAddress() {
        return sourceAddress;
    }

    /**
     * Return the destination MAC address.
     *
     * @return  A byte array which represents the destination MAC address.
     */
    public byte[] getDestinationAddress() {
        return destinationAddress;
    }

    /**
     * Return the ethernet type.
     *
     * @return  Ethernet type.
     */
    public int getEtherType() {
        return etherType;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  VLAN ID.
     */
    public short getVlanId() {
        return vlanId;
    }

    /**
     * Return the VLAN priority.
     *
     * @return  VLAN ID.
     */
    public byte getVlanPcp() {
        return vlanPcp;
    }

    /**
     * Return the payload of the ethernet frame.
     *
     * @return  The payload of the ethernet frame.
     */
    public Packet getPayload() {
        return payload;
    }

    /**
     * Return a source MAC address that does not match this packet.
     *
     * @return  A source MAC address that does not match this packet.
     */
    public byte[] getAnotherSourceAddress() {
        byte[] addr = sourceAddress.clone();
        addr[4] = (byte)~addr[4];
        return addr;
    }

    /**
     * Return a destination MAC address that does not match this packet.
     *
     * @return  A destination MAC address that does not match this packet.
     */
    public byte[] getAnotherDestinationAddress() {
        byte[] addr = destinationAddress.clone();
        addr[5] = (byte)~addr[5];
        return addr;
    }

    /**
     * Return an ethernet type that does not match this packet.
     *
     * @return  An ethernet type that does not match this packet.
     */
    public int getAnotherEtherType() {
        return (~etherType) & 0xffff;
    }

    /**
     * Return a VLAN ID that does not match this packet.
     *
     * @return  A VLAN ID that does not match this packet.
     */
    public short getAnotherVlanId() {
        if (vlanId == MatchType.DL_VLAN_NONE) {
            return (short)4095;
        }

        short vid = (short)(~vlanId & 0xfff);
        if (vid == MatchType.DL_VLAN_NONE) {
            vid = 1;
        }

        return vid;
    }

    /**
     * Return a VLAN priority that does not match this packet.
     *
     * @return  A VLAN priority that does not match this packet.
     */
    public byte getAnotherVlanPcp() {
        if (vlanId == MatchType.DL_VLAN_NONE) {
            return -1;
        }

        return (byte)((vlanPcp + 1) & 0x7);
    }
}
