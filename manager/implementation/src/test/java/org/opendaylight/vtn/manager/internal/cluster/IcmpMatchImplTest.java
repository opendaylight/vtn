/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.ICMP;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.UDP;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link IcmpMatchImpl}.
 */
public class IcmpMatchImplTest extends TestBase {
    /**
     * Test for {@link IcmpMatchImpl#IcmpMatchImpl(IcmpMatch)},
     * {@link L4MatchImpl#create(org.opendaylight.vtn.manager.flow.cond.L4Match)},
     * and getter methods.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testConstructor() throws VTNException {
        short[] types = {
            -1, 0, 4, 18, 32, 159, 255,
        };
        short[] codes = {
            -1, 0, 3, 21, 45, 183, 255,
        };

        IcmpMatchImplBuilder builder = new IcmpMatchImplBuilder();
        for (short type: types) {
            builder.setType(type);
            for (short code: codes) {
                builder.setCode(code);
                IcmpMatchImpl icmi = builder.create();
                assertEquals(type, icmi.getType());
                assertEquals(code, icmi.getCode());
                assertEquals(builder.getIcmpMatch(), icmi.getMatch());
                assertEquals(IPProtocols.ICMP.shortValue(),
                             icmi.getInetProtocol());

                // Instantiate using L4Match.create().
                IcmpMatch icm = builder.getIcmpMatch();
                assertEquals(icmi, L4MatchImpl.create(icm));
            }
        }

        // Specifying invalid ICMP type and code.
        short[] badValues = {
            Short.MIN_VALUE, -3, -2, -1,
            256, 257, 300, 12345, Short.MAX_VALUE,
        };
        for (short v: badValues) {
            Short bad = Short.valueOf(v);
            builder.reset().setType(bad);
            try {
                builder.build();
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid value for ICMP type: " + bad,
                             st.getDescription());
            }

            builder.reset().setCode(bad);
            try {
                builder.build();
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid value for ICMP code: " + bad,
                             st.getDescription());
            }
        }
    }

    /**
     * Test case for {@link IcmpMatchImpl#equals(Object)} and
     * {@link IcmpMatchImpl#hashCode()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testEquals() throws VTNException {
        HashSet<Object> set = new HashSet<Object>();
        HashSet<Integer> hashCodes = new HashSet<Integer>();

        short[] types = {
            -1, 0, 4, 18, 32, 159, 255,
        };
        short[] codes = {
            -1, 0, 3, 21, 45, 183, 255,
        };

        IcmpMatchImplBuilder builder = new IcmpMatchImplBuilder();
        for (short type: types) {
            builder.setType(type);
            for (short code: codes) {
                builder.setCode(code);
                IcmpMatchImpl icmi1 = builder.create();
                IcmpMatchImpl icmi2 = builder.create();
                testEquals(set, icmi1, icmi2);
                assertTrue(hashCodes.add(icmi1.hashCode()));
                assertFalse(hashCodes.add(icmi1.hashCode()));
                assertFalse(hashCodes.add(icmi2.hashCode()));
            }
        }

        assertEquals(types.length * codes.length, set.size());
    }

    /**
     * Test case for {@link IcmpMatchImpl#toString()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testToString() throws VTNException {
        String prefix = "IcmpMatchImpl[";
        String suffix = "]";

        short[] types = {
            -1, 0, 4, 18, 32, 159, 255,
        };
        short[] codes = {
            -1, 0, 3, 21, 45, 183, 255,
        };

        IcmpMatchImplBuilder builder = new IcmpMatchImplBuilder();
        for (short type: types) {
            builder.setType(type);
            String t = (type == -1) ? null : "type=" + type;
            for (short code: codes) {
                builder.setCode(code);
                String s = (code == -1) ? null : "code=" + code;
                IcmpMatchImpl icmi = builder.create();

                String required = joinStrings(prefix, suffix, ",", t, s);
                assertEquals(required, icmi.toString());
            }
        }
    }

    /**
     * Ensure that {@link IcmpMatchImpl} is serializable.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testSerialize() throws VTNException {
        short[] types = {
            -1, 0, 4, 18, 32, 159, 255,
        };
        short[] codes = {
            -1, 0, 3, 21, 45, 183, 255,
        };

        IcmpMatchImplBuilder builder = new IcmpMatchImplBuilder();
        for (short type: types) {
            builder.setType(type);
            for (short code: codes) {
                builder.setCode(code);
                IcmpMatchImpl icmi = builder.create();
                serializeTest(icmi);
            }
        }
    }

    /**
     * Test case for {@link IcmpMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        short type = 8;
        short code = 0;
        byte[] payload = {
            (byte)0x04, (byte)0xce, (byte)0xfe, (byte)0x21,
            (byte)0xdc, (byte)0xb1, (byte)0x16, (byte)0x3b,
        };
        ICMP icmp = new ICMP();
        icmp.setType((byte)type).setCode((byte)code).
            setIdentifier((short)0x1234).setSequenceNumber((short)0x5678).
            setRawPayload(payload);

        String saddr = "10.234.198.83";
        String daddr = "192.168.238.254";
        InetAddress src = InetAddress.getByName(saddr);
        InetAddress dst = InetAddress.getByName(daddr);
        short proto = IPProtocols.ICMP.shortValue();
        byte dscp = 0;
        IPv4 ipv4 = createIPv4(src, dst, proto, dscp);
        ipv4.setPayload(icmp);

        IPv4 nonIcmp1 = createIPv4(src, dst, (short)100, dscp);
        nonIcmp1.setRawPayload(payload);

        UDP udp = new UDP();
        udp.setSourcePort((short)1).setDestinationPort((short)2).
            setLength((short)123).setChecksum((short)0x1234);
        IPv4 nonIcmp2 = createIPv4(src, dst, IPProtocols.UDP.shortValue(),
                                   dscp);
        nonIcmp2.setPayload(udp);
        IPv4[] nonIcmp = {nonIcmp1, nonIcmp2};

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
        int ethType = EtherTypes.IPv4.intValue();
        byte pcp = 0;
        for (short vlan: vlans) {
            Ethernet pkt = createEthernet(srcMac, dstMac, ethType, vlan, pcp,
                                          ipv4);
            matchTest(pkt);

            // Test non-IPv4 packet.
            pkt = createEthernet(srcMac, dstMac, (short)100, vlan, pcp,
                                 payload);
            matchTest(pkt);

            // Test non-ICMP packet.
            for (IPv4 p: nonIcmp) {
                pkt = createEthernet(srcMac, dstMac, ethType, vlan, pcp, p);
                matchTest(pkt);
            }
        }
    }

    /**
     * Run tests for {@link IcmpMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}
     * using the given packet.
     *
     * @param pkt  An {@link Ethernet} instance for test.
     */
    private void matchTest(Ethernet pkt) {
        PacketMatchTest test = new PacketMatchTest();
        IcmpMatchImplBuilder builder = new IcmpMatchImplBuilder();

        ICMP icmp = getIcmpPacket(pkt);
        if (icmp == null) {
            // Should not match non-ICMP packet.
            assertEquals(false, test.run(builder.create(), pkt));

            short type = 0;
            short code = 0;
            builder.setType(type).setCode(code).create();
            assertEquals(false, test.run(builder.create(), pkt));
            return;
        }

        IcmpParser icmpParser = new IcmpParser(icmp);
        short type = icmpParser.getType();
        short code = icmpParser.getCode();

        short anotherType = icmpParser.getAnotherType();
        short anotherCode = icmpParser.getAnotherCode();

        // Empty match should match every packet.
        assertEquals(true, test.run(builder.create(), pkt));

        // Specify single field in ICMP header.
        IcmpMatchImpl icmi = builder.reset().setType(type).create();
        assertEquals(true, test.setMatchType(MatchType.TP_SRC).run(icmi, pkt));
        icmi = builder.reset().setType(anotherType).create();
        assertEquals(false, test.run(icmi, pkt));

        test.reset().setMatchType(MatchType.TP_DST);
        icmi = builder.reset().setCode(code).create();
        assertEquals(true, test.run(icmi, pkt));
        icmi = builder.reset().setCode(anotherCode).create();
        assertEquals(false, test.run(icmi, pkt));

        // Specify all fieldd.
        icmi = builder.reset().setType(type).setCode(code).create();
        test.reset().setMatchType(MatchType.TP_SRC, MatchType.TP_DST);
        assertEquals(true, test.run(icmi, pkt));

        // Ensure that match() returns false if one field does not match.
        test.reset().setMatchType(MatchType.TP_SRC);
        icmi = builder.setType(anotherType).create();
        assertEquals(false, test.run(icmi, pkt));

        test.setMatchType(MatchType.TP_DST);
        icmi = builder.setType(type).setCode(anotherCode).create();
        assertEquals(false, test.run(icmi, pkt));
    }

    /**
     * Return an {@link ICMP} instance configured in the given packet.
     *
     * @param pkt  An {@link Ethernet} instance.
     * @return  An {@link ICMP} instance if found.
     *          {@code null} if not found.
     */
    private ICMP getIcmpPacket(Ethernet pkt) {
        Packet payload = pkt.getPayload();
        if (payload instanceof IEEE8021Q) {
            IEEE8021Q tag = (IEEE8021Q)payload;
            payload = tag.getPayload();
        }

        if (payload instanceof IPv4) {
            IPv4 ipv4 = (IPv4)payload;
            payload = ipv4.getPayload();
            if (payload instanceof ICMP) {
                return (ICMP)payload;
            }
        }

        return null;
    }
}

/**
 * Utility class to create {@link IcmpMatchImpl} instance.
 */
final class IcmpMatchImplBuilder extends TestBase
    implements L4MatchImplBuilder {
    /**
     * ICMP type.
     */
    private Short  icmpType;

    /**
     * ICMP code.
     */
    private Short  icmpCode;

    /**
     * Created {@link IcmpMatch} instance.
     */
    private IcmpMatch  icmpMatch;

    /**
     * Set ICMP type.
     *
     * @param type  ICMP type.
     * @return  This instance.
     */
    public IcmpMatchImplBuilder setType(short type) {
        icmpType = (type == -1) ? null : Short.valueOf(type);
        icmpMatch = null;
        return this;
    }

    /**
     * Set ICMP type.
     *
     * @param type  ICMP type.
     * @return  This instance.
     */
    public IcmpMatchImplBuilder setType(Short type) {
        icmpType = type;
        icmpMatch = null;
        return this;
    }

    /**
     * Set ICMP code.
     *
     * @param code  ICMP code.
     * @return  This instance.
     */
    public IcmpMatchImplBuilder setCode(short code) {
        icmpCode = (code == -1) ? null : Short.valueOf(code);
        icmpMatch = null;
        return this;
    }

    /**
     * Set ICMP code.
     *
     * @param code  ICMP code.
     * @return  This instance.
     */
    public IcmpMatchImplBuilder setCode(Short code) {
        icmpCode = code;
        icmpMatch = null;
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public IcmpMatchImplBuilder reset() {
        icmpType = null;
        icmpCode = null;
        icmpMatch = null;

        return this;
    }

    /**
     * Construct an {@link IcmpMatch} instance.
     *
     * @return  An {@link IcmpMatch} instance.
     */
    public IcmpMatch getIcmpMatch() {
        IcmpMatch icm = icmpMatch;
        if (icm == null) {
            icm = new IcmpMatch(icmpType, icmpCode);
            icmpMatch = icm;
        }

        return icm;
    }

    /**
     * Construct an {@link IcmpMatchImpl} instance.
     *
     * @return  An {@link IcmpMatchImpl} instance.
     * @throws VTNException  An error occurred.
     */
    public IcmpMatchImpl build() throws VTNException {
        IcmpMatch im = getIcmpMatch();
        return new IcmpMatchImpl(im);
    }

    /**
     * Construct an {@link IcmpMatchImpl} instance and ensure that it was
     * constructed successfully.
     *
     * @return  An {@link IcmpMatchImpl} instance.
     */
    public IcmpMatchImpl create() {
        IcmpMatchImpl icmi = null;
        try {
            icmi = build();
        } catch (Exception e) {
            unexpected(e);
        }

        return icmi;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IcmpMatch getLayer4Match() {
        return getIcmpMatch();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IcmpMatchImplBuilder setSource(int src) {
        return setType((short)src);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IcmpMatchImplBuilder setDestination(int dst) {
        return setCode((short)dst);
    }
}


/**
 * ICMP header parser.
 */
final class IcmpParser implements L4Parser {
    /**
     * ICMP packet.
     */
    private final ICMP  packet;

    /**
     * ICMP type.
     */
    private final int  icmpType;

    /**
     * ICMP code.
     */
    private final int  icmpCode;

    /**
     * Construct a new instance.
     *
     * @param icmp  An {@link ICMP} instance.
     */
    IcmpParser(ICMP icmp) {
        packet = icmp;
        icmpType = NetUtils.getUnsignedByte(icmp.getType());
        icmpCode = NetUtils.getUnsignedByte(icmp.getCode());
    }

    /**
     * Return the ICMP packet.
     *
     * @return  An {@link ICMP} instance.
     */
    public ICMP getPacket() {
        return packet;
    }

    /**
     * Return the ICMP type.
     *
     * @return  The ICMP type.
     */
    public short getType() {
        return (short)icmpType;
    }

    /**
     * Return the ICMP code.
     *
     * @return  The ICMP code.
     */
    public short getCode() {
        return (short)icmpCode;
    }

    /**
     * Return an ICMP tye that does not match this packet.
     *
     * @return  An ICMP type that does not match this packet.
     */
    public short getAnotherType() {
        return (short)(~icmpType & 0xff);
    }

    /**
     * Return an ICMP code that does not match this packet.
     *
     * @return  An ICMP code that does not match this packet.
     */
    public short getAnotherCode() {
        return (short)(~icmpCode & 0xff);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getSource() {
        return icmpType;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getDestination() {
        return icmpCode;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getAnotherSource() {
        return (int)getAnotherType();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getAnotherDestination() {
        return (int)getAnotherCode();
    }
}
