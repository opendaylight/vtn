/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;
import org.opendaylight.vtn.manager.flow.cond.UdpMatch;

import org.opendaylight.vtn.manager.internal.DataGenerator;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.ICMP;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.TCP;
import org.opendaylight.controller.sal.packet.UDP;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link FlowMatchImpl}.
 */
public class FlowMatchImplTest extends TestBase {

    /**
     * Create a map that contains {@link FlowMatch} instances.
     *
     * <p>
     *   An instance of {@link FlowMatch} to be passed to the constructor of
     *   {@link FlowMatchImpl} is set as a key, and an instance of
     *   {@link FlowMatch} instances to be returned by the call of
     *   {@link FlowMatchImpl#getMatch()} is set as a value.
     * </p>
     *
     * @return  A map that contains {@link FlowMatch} instances.
     */
    public static Map<FlowMatch, FlowMatch> createFlowMatches() {
        Map<FlowMatch, FlowMatch> map = new HashMap<FlowMatch, FlowMatch>();
        FlowMatch fm = new FlowMatch(null, null, null);
        map.put(fm, fm);

        // Specify EthernetMatch.
        EthernetMatchImplBuilder ethBuilder = new EthernetMatchImplBuilder();
        EthernetMatch em = ethBuilder.reset().
            setSourceAddress(0x000102030405L).
            setDestinationAddress(0xf0f1f2f3f4f5L).setVlanId((short)0).
            getEthernetMatch();
        fm = new FlowMatch(em, null, null);
        map.put(fm, fm);

        em = ethBuilder.reset().setSourceAddress(0xa83401bf34ceL).
            setDestinationAddress(0x00abcdef1234L).setEtherType(0x800).
            setVlanId((short)4095).setVlanPriority((byte)7).getEthernetMatch();
        fm = new FlowMatch(em, null, null);
        map.put(fm, fm);

        // Specify Inet4Match.
        Inet4MatchImplBuilder inet4Builder = new Inet4MatchImplBuilder();
        Inet4Match im = inet4Builder.getInet4Match();
        fm = new FlowMatch(null, im, null);

        // Ethernet type will be configured.
        em = ethBuilder.reset().setEtherType(EtherTypes.IPv4.intValue()).
            getEthernetMatch();
        FlowMatch expected = new FlowMatch(em, im, null);
        map.put(fm, expected);

        em = ethBuilder.setSourceAddress(0x3873cad23847L).setVlanId((short)3).
            getEthernetMatch();
        im = inet4Builder.reset().
            setSourceAddress("192.168.100.255").setSourceSuffix((short)25).
            setDestinationAddress("203.198.39.255").
            setDestinationSuffix((short)23).setProtocol((short)123).
            setDscp((byte)45).getInet4Match();
        fm = new FlowMatch(em, im, null);
        expected = new FlowMatch(em, inet4Builder.getExpectedInet4Match(),
                                 null);
        map.put(fm, expected);

        // Specify TcpMatch.
        TcpMatchImplBuilder tcpBuilder = new TcpMatchImplBuilder();
        TcpMatch tm = tcpBuilder.getTcpMatch();
        fm = new FlowMatch(null, null, tm);

        // Ethernet type and IP protocol will be configured.
        em = ethBuilder.reset().setEtherType(EtherTypes.IPv4.intValue()).
            getEthernetMatch();
        im = inet4Builder.reset().setProtocol(IPProtocols.TCP.shortValue()).
            getInet4Match();
        expected = new FlowMatch(em, im, tm);
        map.put(fm, expected);

        em = ethBuilder.setSourceAddress(0x043e1eb0e32cL).
            setDestinationAddress(0xc8ff32f9e7feL).setVlanId((short)123).
            setVlanPriority((byte)4).getEthernetMatch();
        im = inet4Builder.setSourceAddress("10.20.30.45").
            setDestinationAddress("192.168.90.100").setDscp((byte)0).
            getInet4Match();
        tm = tcpBuilder.setSourcePortFrom(100).
            setDestinationPortFrom(30000).setDestinationPortTo(40000).
            getTcpMatch();
        fm = new FlowMatch(em, im, tm);
        map.put(fm, fm);

        // Specify UdpMatch.
        UdpMatchImplBuilder udpBuilder = new UdpMatchImplBuilder();
        UdpMatch um = udpBuilder.getUdpMatch();
        fm = new FlowMatch(null, null, um);

        // Ethernet type and IP protocol will be configured.
        em = ethBuilder.reset().setEtherType(EtherTypes.IPv4.intValue()).
            getEthernetMatch();
        im = inet4Builder.reset().setProtocol(IPProtocols.UDP.shortValue()).
            getInet4Match();
        expected = new FlowMatch(em, im, um);
        map.put(fm, expected);

        em = ethBuilder.setSourceAddress(0xfc086b487935L).setVlanId((short)19).
            getEthernetMatch();
        im = inet4Builder.setDestinationAddress("123.234.56.78").
            getInet4Match();
        um = udpBuilder.setSourcePortFrom(12345).setSourcePortTo(20000).
            setDestinationPortFrom(50).setDestinationPortTo(60).
            getUdpMatch();
        fm = new FlowMatch(em, im, um);
        map.put(fm, fm);

        // Specify IcmpMatch.
        IcmpMatchImplBuilder icmpBuilder = new IcmpMatchImplBuilder();
        IcmpMatch icm = icmpBuilder.getIcmpMatch();
        fm = new FlowMatch(null, null, icm);

        // Ethernet type and IP protocol will be configured.
        em = ethBuilder.reset().setEtherType(EtherTypes.IPv4.intValue()).
            getEthernetMatch();
        im = inet4Builder.reset().setProtocol(IPProtocols.ICMP.shortValue()).
            getInet4Match();
        expected = new FlowMatch(em, im, icm);
        map.put(fm, expected);

        em = ethBuilder.setVlanId((short)159).setVlanPriority((byte)6).
            getEthernetMatch();
        im = inet4Builder.setSourceAddress("10.245.32.189").
            setSourceSuffix((short)28).
            setDestinationAddress("192.168.195.209").
            setDestinationSuffix((short)31).setDscp((byte)49).
            getInet4Match();
        icm = icmpBuilder.setType((short)123).setCode((short)91).
            getIcmpMatch();
        fm = new FlowMatch(em, im, icm);
        expected = new FlowMatch(em, inet4Builder.getExpectedInet4Match(),
                                 icm);
        map.put(fm, expected);

        return map;
    }

    /**
     * Test for {@link FlowMatchImpl#FlowMatchImpl(FlowMatch)},
     * {@link InetMatchImpl#create(InetMatch)}, and getter methods.n
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testConstructor() throws VTNException {
        int[] indices = {1, 100, 256, 1000, 40000, 65535};
        Map<FlowMatch, FlowMatch> map = createFlowMatches();

        FlowMatch fmatch = null;
        for (Map.Entry<FlowMatch, FlowMatch> entry: map.entrySet()) {
            FlowMatch key = entry.getKey();
            FlowMatch value = entry.getValue();
            fmatch = key;
            for (int index: indices) {
                FlowMatch fm = key.assignIndex(index);
                FlowMatch expected = value.assignIndex(index);
                FlowMatchImpl fmi = new FlowMatchImpl(fm);
                assertEquals(index, fmi.getIndex());
                assertEquals(expected, fmi.getMatch());
            }
        }

        assertNotNull(fmatch);

        // Pass null.
        try {
            new FlowMatchImpl(null);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Flow match cannot be null", st.getDescription());
        }

        // No index is assigned.
        try {
            new FlowMatchImpl(fmatch);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Match index cannot be null", st.getDescription());
        }

        // Invalid index is specified.
        int[] badIndices = {
            Integer.MIN_VALUE, -3, -2, -1, 0,
            65536, 65537, 1000000, 20000000, Integer.MAX_VALUE,
        };
        for (int index: badIndices) {
            FlowMatch fm = fmatch.assignIndex(index);
            try {
                new FlowMatchImpl(fm);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid match index: " + index,
                             st.getDescription());
            }
        }

        // Unexpecetd ethenet type.
        int ethType = 0x86dd;
        EthernetMatchImplBuilder ethBuilder = new EthernetMatchImplBuilder();
        EthernetMatch em = ethBuilder.setEtherType(ethType).getEthernetMatch();
        Inet4MatchImplBuilder inet4Builder = new Inet4MatchImplBuilder();
        Inet4Match im = inet4Builder.getInet4Match();
        FlowMatch fm = new FlowMatch(1, em, im, null);
        try {
            new FlowMatchImpl(fm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Ethernet type conflict: type=0x" +
                         Integer.toHexString(ethType) + ", expected=0x" +
                         Integer.toHexString(EtherTypes.IPv4.intValue()),
                         st.getDescription());
        }

        // Unexpected IP protocol type.
        short proto = IPProtocols.UDP.shortValue();
        im = inet4Builder.setProtocol(proto).getInet4Match();
        TcpMatchImplBuilder tcpBuilder = new TcpMatchImplBuilder();
        L4Match lm = tcpBuilder.getTcpMatch();
        fm = new FlowMatch(1, null, im, lm);
        try {
            new FlowMatchImpl(fm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("IP protocol conflict: proto=" + proto +
                         ", expected=" + IPProtocols.TCP.intValue(),
                         st.getDescription());
        }

        proto = IPProtocols.EGP.shortValue();
        im = inet4Builder.setProtocol(proto).getInet4Match();
        UdpMatchImplBuilder udpBuilder = new UdpMatchImplBuilder();
        lm = udpBuilder.getUdpMatch();
        fm = new FlowMatch(1, null, im, lm);
        try {
            new FlowMatchImpl(fm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("IP protocol conflict: proto=" + proto +
                         ", expected=" + IPProtocols.UDP.intValue(),
                         st.getDescription());
        }

        proto = IPProtocols.TCP.shortValue();
        im = inet4Builder.setProtocol(proto).getInet4Match();
        IcmpMatchImplBuilder icmpBuilder = new IcmpMatchImplBuilder();
        lm = icmpBuilder.getIcmpMatch();
        fm = new FlowMatch(1, null, im, lm);
        try {
            new FlowMatchImpl(fm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("IP protocol conflict: proto=" + proto +
                         ", expected=" + IPProtocols.ICMP.intValue(),
                         st.getDescription());
        }
    }

    /**
     * Test case for {@link FlowMatchImpl#equals(Object)} and
     * {@link FlowMatchImpl#hashCode()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testEquals() throws VTNException {
        HashSet<Object> set = new HashSet<Object>();

        int[] indices = {1, 100, 256, 1000, 40000, 65535};
        Map<FlowMatch, FlowMatch> map = createFlowMatches();
        for (FlowMatch fmatch: map.keySet()) {
            for (int index: indices) {
                FlowMatch fm = fmatch.assignIndex(index);
                FlowMatchImpl fmi1 = new FlowMatchImpl(fm);
                FlowMatchImpl fmi2 = new FlowMatchImpl(fm);
                testEquals(set, fmi1, fmi2);
            }
        }

        assertEquals(map.size() * indices.length, set.size());
    }

    /**
     * Test case for {@link FlowMatchImpl#toString()}.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testToString() throws VTNException {
        String prefix = "FlowMatchImpl[";
        String suffix = "]";

        int[] indices = {1, 100, 256, 1000, 40000, 65535};
        Map<FlowMatch, FlowMatch> map = createFlowMatches();
        for (Map.Entry<FlowMatch, FlowMatch> entry: map.entrySet()) {
            FlowMatch key = entry.getKey();
            FlowMatch value = entry.getValue();
            for (int index: indices) {
                FlowMatch fm = key.assignIndex(index);
                FlowMatch expected = value.assignIndex(index);
                FlowMatchImpl fmi = new FlowMatchImpl(fm);

                EthernetMatch em = expected.getEthernetMatch();
                InetMatch im = expected.getInetMatch();
                L4Match lm = expected.getLayer4Match();

                String idx = "index=" + index;
                String e = (em == null) ? null
                    : "ether=" + new EthernetMatchImpl(em);
                String i = (im == null) ? null
                    : "inet=" + new Inet4MatchImpl(im);
                String l;
                if (lm == null) {
                    l = null;
                } else if (lm instanceof TcpMatch) {
                    l = "L4=" + new TcpMatchImpl((TcpMatch)lm);
                } else if (lm instanceof UdpMatch) {
                    l = "L4=" + new UdpMatchImpl((UdpMatch)lm);
                } else {
                    l = "L4=" + new IcmpMatchImpl((IcmpMatch)lm);
                }

                String required = joinStrings(prefix, suffix, ",", idx,
                                              e, i, l);
                assertEquals(required, fmi.toString());
            }
        }
    }

    /**
     * Ensure that {@link FlowMatchImpl} is serializable.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testSerialize() throws VTNException {
        int[] indices = {1, 100, 256, 1000, 40000, 65535};
        Map<FlowMatch, FlowMatch> map = createFlowMatches();
        for (FlowMatch fmatch: map.keySet()) {
            for (int index: indices) {
                FlowMatch fm = fmatch.assignIndex(index);
                FlowMatchImpl fmi = new FlowMatchImpl(fm);
                serializeTest(fmi);
            }
        }
    }

    /**
     * Test case for {@link FlowMatchImpl#match(org.opendaylight.vtn.manager.internal.PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        DataGenerator gen = new DataGenerator();
        ArrayList<IPv4> ipPackets = new ArrayList<IPv4>();
        byte[] payload = {
            (byte)0xa8, (byte)0xa6, (byte)0x94, (byte)0xd4,
            (byte)0x46, (byte)0xe2, (byte)0x08, (byte)0xe0,
            (byte)0x78, (byte)0x4e, (byte)0x5e, (byte)0xf4,
        };

        TCP tcp = new TCP();
        tcp.setSourcePort((short)gen.getPort()).
            setDestinationPort((short)gen.getPort()).
            setSequenceNumber(0x12345).setAckNumber(0x23456).
            setDataOffset((byte)1).setHeaderLenFlags((short)0x18).
            setReserved((byte)0).setWindowSize((short)1500).
            setChecksum((short)0x9999).setRawPayload(payload);
        IPv4 ipv4 = createIPv4(gen.getInetAddress(), gen.getInetAddress(),
                               IPProtocols.TCP.shortValue(), (byte)0);
        ipv4.setPayload(tcp);
        ipPackets.add(ipv4);

        UDP udp = new UDP();
        udp.setSourcePort((short)gen.getPort()).
            setDestinationPort((short)gen.getPort()).
            setLength((short)123).setChecksum((short)0x3333).
            setRawPayload(payload);
        ipv4 = createIPv4(gen.getInetAddress(), gen.getInetAddress(),
                          IPProtocols.UDP.shortValue(), (byte)1);
        ipv4.setPayload(udp);
        ipPackets.add(ipv4);

        ICMP icmp = new ICMP();
        icmp.setType((byte)gen.getPort()).setCode((byte)gen.getPort()).
            setIdentifier((short)0x1234).setSequenceNumber((short)0x5678).
            setRawPayload(payload);
        ipv4 = createIPv4(gen.getInetAddress(), gen.getInetAddress(),
                          IPProtocols.ICMP.shortValue(), (byte)2);
        ipv4.setPayload(icmp);
        ipPackets.add(ipv4);

        ipv4 = createIPv4(gen.getInetAddress(), gen.getInetAddress(),
                          (short)123, (byte)3);
        ipv4.setRawPayload(payload);
        ipPackets.add(ipv4);

        ipv4 = createIPv4(gen.getInetAddress(), gen.getInetAddress(),
                          IPProtocols.TCP.shortValue(), (byte)4);
        ipv4.setFragmentOffset((short)123);
        ipv4.setRawPayload(payload);
        ipPackets.add(ipv4);

        ArrayList<Ethernet> etherPackets = new ArrayList<Ethernet>();
        short[] vlans = {
            MatchType.DL_VLAN_NONE, 1, 10, 4095,
        };

        byte pcp = 0;
        for (short vlan: vlans) {
            // Unsupported packet.
            Ethernet ether = createEthernet(
                gen.getMacAddress(), gen.getMacAddress(), 0x4444, vlan, pcp,
                payload);
            etherPackets.add(ether);
            pcp = (byte)((pcp + 1) & 0x7);

            // ARP packet.
            ether = createEthernet(gen.getMacAddress(), gen.getMacAddress(),
                                   EtherTypes.ARP.intValue(), vlan, pcp,
                                   PacketMatchTest.ARP_PACKET);
            etherPackets.add(ether);
            pcp = (byte)((pcp + 1) & 0x7);

            // IPv4 packets.
            for (IPv4 pkt: ipPackets) {
                ether = createEthernet(gen.getMacAddress(),
                                       gen.getMacAddress(),
                                       EtherTypes.IPv4.intValue(), vlan, pcp,
                                       pkt);
                etherPackets.add(ether);
                pcp = (byte)((pcp + 1) & 0x7);
            }
        }

        PacketMatchTest test = new PacketMatchTest();
        List<FlowMatchImpl> empties = getEmptyMatches();
        for (Ethernet ether: etherPackets) {
            // Empty condition should match every packet.
            for (FlowMatchImpl fmi: empties) {
                assertEquals(true, test.run(fmi, ether));
            }

            ethernetMatchTest(ether);
            ipv4MatchTest(ether);
            layer4MatchTest(ether);
        }
    }

    /**
     * Create a list of empty {@link FlowMatchImpl} instances.
     *
     * @return  A list of empty {@link FlowMatchImpl} instances.
     * @throws VTNException  An error occurred.
     */
    private List<FlowMatchImpl> getEmptyMatches() throws VTNException {
        ArrayList<FlowMatchImpl> empty = new ArrayList<FlowMatchImpl>();
        EthernetMatchImplBuilder ethBuilder = new EthernetMatchImplBuilder();
        EthernetMatch em = ethBuilder.getEthernetMatch();

        empty.add(new FlowMatchImpl(new FlowMatch(1, null, null, null)));
        empty.add(new FlowMatchImpl(new FlowMatch(2, em, null, null)));

        return empty;
    }

    /**
     * Test ethernet matches.
     *
     * @param pkt  An {@link Ethernet} instance for test.
     * @throws VTNException  An error occurred.
     */
    private void ethernetMatchTest(Ethernet pkt) throws VTNException {
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

        // Create flow match that specifies all Ethernet header fields.
        EthernetMatchImplBuilder ethBuilder = new EthernetMatchImplBuilder();
        ethBuilder.setSourceAddress(src).setDestinationAddress(dst).
            setEtherType(type).setVlanId(vlan);
        test.reset().setMatchType(MatchType.DL_SRC, MatchType.DL_DST,
                                  MatchType.DL_TYPE);
        if (vlan != MatchType.DL_VLAN_NONE) {
            ethBuilder.setVlanPriority(pcp);
            test.setMatchType(MatchType.DL_VLAN_PR);
        }

        EthernetMatch em = ethBuilder.getEthernetMatch();
        FlowMatch fm = new FlowMatch(1, em, null, null);
        FlowMatchImpl fmi = new FlowMatchImpl(fm);
        assertEquals(true, test.run(fmi, pkt));

        // Ensure that the packet matches only if all conditions are met.
        test.reset().setMatchType(MatchType.DL_SRC);
        em = ethBuilder.setSourceAddress(anotherSrc).getEthernetMatch();
        fm = new FlowMatch(1, em, null, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.DL_DST);
        em = ethBuilder.setSourceAddress(src).
            setDestinationAddress(anotherDst).getEthernetMatch();
        fm = new FlowMatch(1, em, null, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.DL_TYPE);
        em = ethBuilder.setDestinationAddress(dst).setEtherType(anotherType).
            getEthernetMatch();
        fm = new FlowMatch(1, em, null, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        em = ethBuilder.setEtherType(type).setVlanId(anotherVlan).
            getEthernetMatch();
        fm = new FlowMatch(1, em, null, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));
        boolean tagged = (vlan != MatchType.DL_VLAN_NONE);
        if (tagged) {
            em = ethBuilder.setEtherType(type).
                setVlanId(MatchType.DL_VLAN_NONE).setVlanPriority((byte)-1).
                getEthernetMatch();
            fm = new FlowMatch(1, em, null, null);
            fmi = new FlowMatchImpl(fm);
            assertEquals(false, test.run(fmi, pkt));
            test.setMatchType(MatchType.DL_VLAN_PR);
        }

        em = ethBuilder.setVlanId(vlan).setVlanPriority(anotherPcp).
            getEthernetMatch();
        fm = new FlowMatch(1, em, null, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(!tagged, test.run(fmi, pkt));
    }

    /**
     * Test ethernet and IPv4 matches.
     *
     * @param pkt  An {@link Ethernet} instance for test.
     * @throws VTNException  An error occurred.
     */
    private void ipv4MatchTest(Ethernet pkt) throws VTNException {
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

        // Create flow match that specifies all Ethernet header fields.
        EthernetMatchImplBuilder ethBuilder = new EthernetMatchImplBuilder();
        ethBuilder.setSourceAddress(src).setDestinationAddress(dst).
            setEtherType(type).setVlanId(vlan);
        test.reset().setMatchType(MatchType.DL_SRC, MatchType.DL_DST,
                                  MatchType.DL_TYPE);
        if (vlan != MatchType.DL_VLAN_NONE) {
            ethBuilder.setVlanPriority(pcp);
            test.setMatchType(MatchType.DL_VLAN_PR);
        }

        Inet4MatchImplBuilder inet4Builder = new Inet4MatchImplBuilder();
        Packet payload = ethParser.getPayload();
        if (!(payload instanceof IPv4)) {
            // IPv4 conditions should never be used.
            inet4Builder.setSourceAddress("10.0.0.1").
                setDestinationAddress("192.168.10.1").
                setProtocol((short)6).setDscp((byte)1);

            // Ether type will be tested before VLAN tag.
            test.reset().setMatchType(MatchType.DL_SRC, MatchType.DL_DST,
                                      MatchType.DL_TYPE);
            EthernetMatch em = ethBuilder.
                setEtherType(EtherTypes.IPv4.shortValue()).getEthernetMatch();
            Inet4Match im = inet4Builder.getInet4Match();
            FlowMatch fm = new FlowMatch(1, em, im, null);
            FlowMatchImpl fmi = new FlowMatchImpl(fm);
            assertEquals(false, test.run(fmi, pkt));

            return;
        }

        EthernetMatch em = ethBuilder.getEthernetMatch();
        Inet4Parser inet4Parser = new Inet4Parser((IPv4)payload);
        int ipSrc = inet4Parser.getSourceAddress();
        int ipDst = inet4Parser.getDestinationAddress();
        short ipProto = inet4Parser.getProtocol();
        byte ipDscp = inet4Parser.getDiffServ();
        test.setMatchType(MatchType.NW_SRC, MatchType.NW_DST,
                          MatchType.NW_PROTO, MatchType.NW_TOS);
        inet4Builder.setSourceAddress(ipSrc).setDestinationAddress(ipDst).
            setProtocol(ipProto).setDscp(ipDscp);

        int anotherIpSrc = inet4Parser.getAnotherSourceAddress();
        int anotherIpDst = inet4Parser.getAnotherDestinationAddress();
        short anotherIpProto = inet4Parser.getAnotherProtocol();
        byte anotherIpDscp = inet4Parser.getAnotherDiffServ();

        Inet4Match im = inet4Builder.getInet4Match();
        FlowMatch fm = new FlowMatch(1, em, im, null);
        FlowMatchImpl fmi = new FlowMatchImpl(fm);
        assertEquals(true, test.run(fmi, pkt));

        // Ensure that the packet matches only if all conditions are met.
        test.reset().setMatchType(MatchType.DL_SRC);
        em = ethBuilder.setSourceAddress(anotherSrc).getEthernetMatch();
        fm = new FlowMatch(1, em, im, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.DL_DST);
        em = ethBuilder.setSourceAddress(src).
            setDestinationAddress(anotherDst).getEthernetMatch();
        fm = new FlowMatch(1, em, im, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        // DL_TYPE condition cannot be changed because IPv4 condition
        // will be configured.
        test.setMatchType(MatchType.DL_TYPE);
        em = ethBuilder.setDestinationAddress(dst).setVlanId(anotherVlan).
            getEthernetMatch();
        fm = new FlowMatch(1, em, im, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));
        boolean tagged = (vlan != MatchType.DL_VLAN_NONE);
        if (tagged) {
            em = ethBuilder.setEtherType(type).
                setVlanId(MatchType.DL_VLAN_NONE).setVlanPriority((byte)-1).
                getEthernetMatch();
            fm = new FlowMatch(1, em, im, null);
            fmi = new FlowMatchImpl(fm);
            assertEquals(false, test.run(fmi, pkt));
            test.setMatchType(MatchType.DL_VLAN_PR);
        }

        em = ethBuilder.setVlanId(vlan).setVlanPriority(anotherPcp).
            getEthernetMatch();
        fm = new FlowMatch(1, em, im, null);
        fmi = new FlowMatchImpl(fm);
        if (tagged) {
            assertEquals(false, test.run(fmi, pkt));
            em = ethBuilder.setVlanPriority(pcp).getEthernetMatch();
        } else {
            test.setMatchType(MatchType.NW_SRC, MatchType.NW_DST,
                              MatchType.NW_PROTO, MatchType.NW_TOS);
            assertEquals(true, test.run(fmi, pkt));
            test.clearMatchType(MatchType.NW_SRC, MatchType.NW_DST,
                                MatchType.NW_PROTO, MatchType.NW_TOS);
        }

        test.setMatchType(MatchType.NW_SRC);
        im = inet4Builder.setSourceAddress(anotherIpSrc).getInet4Match();
        fm = new FlowMatch(1, em, im, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.NW_DST);
        im = inet4Builder.setSourceAddress(ipSrc).
            setDestinationAddress(anotherIpDst).getInet4Match();
        fm = new FlowMatch(1, em, im, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.NW_PROTO);
        im = inet4Builder.setDestinationAddress(ipDst).
            setProtocol(anotherIpProto).getInet4Match();
        fm = new FlowMatch(1, em, im, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.NW_TOS);
        im = inet4Builder.setProtocol(ipProto).setDscp(anotherIpDscp).
            getInet4Match();
        fm = new FlowMatch(1, em, im, null);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));
    }

    /**
     * Test ethernet, IPv4, and layer 4 matches.
     *
     * @param pkt  An {@link Ethernet} instance for test.
     * @throws VTNException  An error occurred.
     */
    private void layer4MatchTest(Ethernet pkt) throws VTNException {
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

        // Create flow match that specifies all Ethernet header fields.
        EthernetMatchImplBuilder ethBuilder = new EthernetMatchImplBuilder();
        ethBuilder.setSourceAddress(src).setDestinationAddress(dst).
            setEtherType(type).setVlanId(vlan);
        test.reset().setMatchType(MatchType.DL_SRC, MatchType.DL_DST,
                                  MatchType.DL_TYPE);
        if (vlan != MatchType.DL_VLAN_NONE) {
            ethBuilder.setVlanPriority(pcp);
            test.setMatchType(MatchType.DL_VLAN_PR);
        }

        Inet4MatchImplBuilder inet4Builder = new Inet4MatchImplBuilder();
        Packet payload = ethParser.getPayload();
        if (!(payload instanceof IPv4)) {
            // IPv4 conditions should never be used.
            inet4Builder.setSourceAddress("10.0.0.1").
                setDestinationAddress("192.168.10.1").
                setProtocol((short)6).setDscp((byte)1);

            // Ether type will be tested before VLAN tag.
            test.reset().setMatchType(MatchType.DL_SRC, MatchType.DL_DST,
                                      MatchType.DL_TYPE);
            EthernetMatch em = ethBuilder.
                setEtherType(EtherTypes.IPv4.shortValue()).getEthernetMatch();
            Inet4Match im = inet4Builder.getInet4Match();
            FlowMatch fm = new FlowMatch(1, em, im, null);
            FlowMatchImpl fmi = new FlowMatchImpl(fm);
            assertEquals(false, test.run(fmi, pkt));

            return;
        }

        EthernetMatch em = ethBuilder.getEthernetMatch();
        Inet4Parser inet4Parser = new Inet4Parser((IPv4)payload);
        int ipSrc = inet4Parser.getSourceAddress();
        int ipDst = inet4Parser.getDestinationAddress();
        short ipProto = inet4Parser.getProtocol();
        byte ipDscp = inet4Parser.getDiffServ();
        test.setMatchType(MatchType.NW_SRC, MatchType.NW_DST,
                          MatchType.NW_PROTO, MatchType.NW_TOS);
        inet4Builder.setSourceAddress(ipSrc).setDestinationAddress(ipDst).
            setProtocol(ipProto).setDscp(ipDscp);

        payload = inet4Parser.getPayload();
        IPProtocols ipType = null;
        if (payload instanceof TCP) {
            ipType = IPProtocols.TCP;
        } else if (payload instanceof UDP) {
            ipType = IPProtocols.UDP;
        } else if (payload instanceof ICMP) {
            ipType = IPProtocols.ICMP;
        }

        TcpMatchImplBuilder tcpBuilder = new TcpMatchImplBuilder();
        if (ipType == null) {
            // Layer 4 conditions should never be used.
            Inet4Match im = inet4Builder.
                setProtocol(IPProtocols.TCP.shortValue()).getInet4Match();
            TcpMatch tm = tcpBuilder.setSourcePortFrom(12345).
                setDestinationPortFrom(333).getTcpMatch();
            FlowMatch fm = new FlowMatch(1, em, im, tm);
            FlowMatchImpl fmi = new FlowMatchImpl(fm);

            // IP protocol will be tested before TOS.
            if (ipProto != IPProtocols.TCP.shortValue()) {
                test.clearMatchType(MatchType.NW_TOS);
            }
            assertEquals(false, test.run(fmi, pkt));

            return;
        }

        int anotherIpSrc = inet4Parser.getAnotherSourceAddress();
        int anotherIpDst = inet4Parser.getAnotherDestinationAddress();
        short anotherIpProto = inet4Parser.getAnotherProtocol();
        byte anotherIpDscp = inet4Parser.getAnotherDiffServ();

        L4MatchImplBuilder l4Builder;
        L4Parser l4Parser;
        switch (ipType) {
        case TCP:
            l4Builder = new TcpMatchImplBuilder();
            l4Parser = new TcpParser((TCP)payload);
            break;

        case UDP:
            l4Builder = new UdpMatchImplBuilder();
            l4Parser = new UdpParser((UDP)payload);
            break;

        default:
            l4Builder = new IcmpMatchImplBuilder();
            l4Parser = new IcmpParser((ICMP)payload);
            break;
        }

        int l4Src = l4Parser.getSource();
        int l4Dst = l4Parser.getDestination();
        int anotherL4Src = l4Parser.getAnotherSource();
        int anotherL4Dst = l4Parser.getAnotherDestination();
        test.setMatchType(MatchType.TP_SRC, MatchType.TP_DST);

        L4Match lm = l4Builder.setSource(l4Src).setDestination(l4Dst).
            getLayer4Match();
        Inet4Match im = inet4Builder.getInet4Match();
        FlowMatch fm = new FlowMatch(1, em, im, lm);
        FlowMatchImpl fmi = new FlowMatchImpl(fm);
        assertEquals(true, test.run(fmi, pkt));

        // Ensure that the packet matches only if all conditions are met.
        test.reset().setMatchType(MatchType.DL_SRC);
        em = ethBuilder.setSourceAddress(anotherSrc).getEthernetMatch();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.DL_DST);
        em = ethBuilder.setSourceAddress(src).
            setDestinationAddress(anotherDst).getEthernetMatch();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        // DL_TYPE condition cannot be changed because IPv4 condition
        // will be configured.
        test.setMatchType(MatchType.DL_TYPE);
        em = ethBuilder.setDestinationAddress(dst).setVlanId(anotherVlan).
            getEthernetMatch();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));
        boolean tagged = (vlan != MatchType.DL_VLAN_NONE);
        if (tagged) {
            em = ethBuilder.setEtherType(type).
                setVlanId(MatchType.DL_VLAN_NONE).setVlanPriority((byte)-1).
                getEthernetMatch();
            fm = new FlowMatch(1, em, im, lm);
            fmi = new FlowMatchImpl(fm);
            assertEquals(false, test.run(fmi, pkt));
            test.setMatchType(MatchType.DL_VLAN_PR);
        }

        em = ethBuilder.setVlanId(vlan).setVlanPriority(anotherPcp).
            getEthernetMatch();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        if (tagged) {
            assertEquals(false, test.run(fmi, pkt));
            em = ethBuilder.setVlanPriority(pcp).getEthernetMatch();
        } else {
            test.setMatchType(MatchType.NW_SRC, MatchType.NW_DST,
                              MatchType.NW_PROTO, MatchType.NW_TOS,
                              MatchType.TP_SRC, MatchType.TP_DST);
            assertEquals(true, test.run(fmi, pkt));
            test.clearMatchType(MatchType.NW_SRC, MatchType.NW_DST,
                                MatchType.NW_PROTO, MatchType.NW_TOS,
                                MatchType.TP_SRC, MatchType.TP_DST);
        }

        test.setMatchType(MatchType.NW_SRC);
        im = inet4Builder.setSourceAddress(anotherIpSrc).getInet4Match();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.NW_DST);
        im = inet4Builder.setSourceAddress(ipSrc).
            setDestinationAddress(anotherIpDst).getInet4Match();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        // NW_PROTO condition cannot be changed because L4 condition will be
        // configured.
        test.setMatchType(MatchType.NW_PROTO, MatchType.NW_TOS);
        im = inet4Builder.setDestinationAddress(ipDst).setDscp(anotherIpDscp).
            getInet4Match();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.TP_SRC);
        im = inet4Builder.setDscp(ipDscp).getInet4Match();
        lm = l4Builder.setSource(anotherL4Src).getLayer4Match();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));

        test.setMatchType(MatchType.TP_DST);
        lm = l4Builder.setSource(l4Src).setDestination(anotherL4Dst).
            getLayer4Match();
        fm = new FlowMatch(1, em, im, lm);
        fmi = new FlowMatchImpl(fm);
        assertEquals(false, test.run(fmi, pkt));
    }
}
