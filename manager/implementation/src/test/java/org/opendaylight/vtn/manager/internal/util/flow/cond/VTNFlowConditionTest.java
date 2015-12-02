/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.InetProtocols;
import org.opendaylight.vtn.manager.util.IpNetwork;
import org.opendaylight.vtn.manager.util.VTNIdentifiableComparator;

import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchType;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.DataGenerator;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestVnodeName;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;
import org.opendaylight.vtn.manager.internal.util.flow.match.EtherMatchParams;
import org.opendaylight.vtn.manager.internal.util.flow.match.IcmpMatchParams;
import org.opendaylight.vtn.manager.internal.util.flow.match.Inet4MatchParams;
import org.opendaylight.vtn.manager.internal.util.flow.match.TcpMatchParams;
import org.opendaylight.vtn.manager.internal.util.flow.match.TestMatchContext;
import org.opendaylight.vtn.manager.internal.util.flow.match.UdpMatchParams;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowCondConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VTNFlowCondition}.
 */
public class VTNFlowConditionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNFlowCondition} class.
     */
    private static final String  XML_ROOT = "vtn-flow-condition";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNFlowCondition} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        String[] p = XmlDataType.addPath(
            "vtn-flow-matches", XmlDataType.addPath(name, parent));
        List<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlValueType("name", VnodeName.class).
                  add(name).prepend(parent));
        dlist.addAll(VTNFlowMatchTest.getXmlDataTypes("vtn-flow-match", p));
        return dlist;
    }

    /**
     * Test case for constructors and getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        assertEquals(null, VTNFlowCondition.create(null));

        List<VTNFlowCondition> list = new ArrayList<>();
        Map<FlowCondParams, FlowCondParams> cases =
            FlowCondParams.createFlowConditions();
        for (Map.Entry<FlowCondParams, FlowCondParams> entry:
                 cases.entrySet()) {
            FlowCondParams params = entry.getKey();
            FlowCondParams expected = entry.getValue();
            VTNFlowCondition vfcond = params.toVTNFlowCondition();
            expected.verify(vfcond);
            list.add(vfcond);
        }

        // Ensure that VTNFlowCondition can be sorted by
        // VTNIdentifiableComparator.
        VTNIdentifiableComparator<String> comparator =
            new VTNIdentifiableComparator<>(String.class);
        Collections.sort(list, comparator);
        String prev = null;
        for (VTNFlowCondition vfcond: list) {
            String name = vfcond.getIdentifier();
            if (prev != null) {
                assertTrue(prev.compareTo(name) < 0);
            }
            prev = name;
        }

        // Test cases for valid names.
        String[] names = {
            "0123456789012345678901234567890",
            "aaaaaaaaaaaaaaaAAAAAAAAAAAAAAAA",
            "aaaa_BBBB_cccc_DDDD_0000_1111_2",
            "0",
            "1",
            "a",
            "B",
        };
        for (String name: names) {
            VtnFlowCondition vfc = new VtnFlowConditionBuilder().
                setName(new VnodeName(name)).build();
            VTNFlowCondition vfcond = new VTNFlowCondition(vfc);
            assertEquals(name, vfcond.getIdentifier());
        }

        // Null name.
        String msg = "Flow condition name cannot be null";
        VtnFlowCondition vfc = new VtnFlowConditionBuilder().build();
        try {
            new VTNFlowCondition(vfc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        assertEquals(null, VTNFlowCondition.create(vfc));

        // Empty name
        msg = "Flow condition name cannot be empty";
        VtnFlowCondConfig vfconf = mock(VtnFlowCondConfig.class);
        when(vfconf.getName()).thenReturn(new TestVnodeName(""));
        try {
            new VTNFlowCondition(vfconf);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Invalid name
        msg = "Flow condition name is invalid";
        String[] invalidNames = {
            // Too long name.
            "01234567890123456789012345678901",
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",

            // Starts with an invalid character.
            "_abc",
            ";abc",
            "/abc",
            "%abc",

            // Invalid character.
            "abcde-0123",
            "abcde.0123",
            "abcde:0123",
        };

        for (String name: invalidNames) {
            vfconf = mock(VtnFlowCondConfig.class);
            when(vfconf.getName()).thenReturn(new TestVnodeName(name));
            try {
                new VTNFlowCondition(vfconf);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Duplicate index.
        int badIndex = 20;
        msg = "Duplicate match index: " + badIndex;
        List<VtnFlowMatch> vmatches = new ArrayList<>();
        for (int i = 1; i <= badIndex + 20; i++) {
            vmatches.add(new VtnFlowMatchBuilder().setIndex(i).build());
        }
        vmatches.add(new VtnFlowMatchBuilder().setIndex(badIndex).build());

        String name = "fcond";
        vfc = new VtnFlowConditionBuilder().
            setName(new VnodeName(name)).setVtnFlowMatch(vmatches).build();
        try {
            new VTNFlowCondition(vfc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for object identity.
     *
     * <ul>
     *   <li>{@link VTNFlowCondition#equals(Object)}</li>
     *   <li>{@link VTNFlowCondition#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();

        final int numEmpties = 10;
        List<VtnFlowMatch> vmatches = Collections.<VtnFlowMatch>emptyList();
        for (int i = 0; i < numEmpties; i++) {
            VnodeName vname = new VnodeName("empty" + i);
            VtnFlowCondition vfc1 = new VtnFlowConditionBuilder().
                setName(vname).build();
            VtnFlowCondition vfc2 = new VtnFlowConditionBuilder().
                setName(vname).setVtnFlowMatch(vmatches).build();
            VTNFlowCondition vfcond1 = new VTNFlowCondition(vfc1);
            VTNFlowCondition vfcond2 = new VTNFlowCondition(vfc2);
            testEquals(set, vfcond1, vfcond2);
        }

        int count = 0;
        Map<FlowCondParams, FlowCondParams> cases =
            FlowCondParams.createFlowConditions();
        for (Map.Entry<FlowCondParams, FlowCondParams> entry:
                 cases.entrySet()) {
            FlowCondParams params = entry.getKey();
            FlowCondParams expected = entry.getValue();
            VTNFlowCondition vfcond1 = params.toVTNFlowCondition();
            VTNFlowCondition vfcond2 = expected.toVTNFlowCondition();
            testEquals(set, vfcond1, vfcond2);

            String name = "another_cond_" + count;
            VtnFlowCondition vfc = params.toVtnFlowConditionBuilder().
                setName(new VnodeName(name)).build();
            vfcond1 = new VTNFlowCondition(vfc);
            vfcond2 = params.setName(name).toVTNFlowCondition();
            testEquals(set, vfcond1, vfcond2);
        }

        int expected = cases.size() * 2 + numEmpties;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNFlowCondition#verify()} and JAXB mapping.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        // Normal case.
        Class<VTNFlowCondition> type = VTNFlowCondition.class;
        Marshaller m = createMarshaller(type);
        Unmarshaller um = createUnmarshaller(type);
        Map<FlowCondParams, FlowCondParams> cases =
            FlowCondParams.createFlowConditions();
        for (Map.Entry<FlowCondParams, FlowCondParams> entry:
                 cases.entrySet()) {
            FlowCondParams params = entry.getKey();
            FlowCondParams expected = entry.getValue();
            String xml = params.toXmlNode(XML_ROOT).toString();
            VTNFlowCondition vfcond = unmarshal(um, xml, type);
            vfcond.verify();
            expected.verify(vfcond);

            xml = expected.toXmlNode(XML_ROOT).toString();
            VTNFlowCondition vfcond1 = unmarshal(um, xml, type);
            vfcond1.verify();
            assertEquals(vfcond, vfcond1);

            xml = marshal(m, vfcond, type, XML_ROOT);
            vfcond1 = unmarshal(um, xml, type);
            vfcond1.verify();
            assertEquals(vfcond, vfcond1);
        }

        // Test cases for valid names.
        String[] names = {
            "0123456789012345678901234567890",
            "aaaaaaaaaaaaaaaAAAAAAAAAAAAAAAA",
            "aaaa_BBBB_cccc_DDDD_0000_1111_2",
            "0",
            "1",
            "a",
            "B",
        };
        for (String name: names) {
            FlowCondParams params = new FlowCondParams(name);
            String xml = params.toXmlNode(XML_ROOT).toString();
            VTNFlowCondition vfcond = unmarshal(um, xml, type);
            vfcond.verify();
            params.verify(vfcond);
        }

        // Invalid match index.
        Integer[] badIndices = {
            null, Integer.MIN_VALUE, -10000000, -3000, -200, -3, -2, -1, 0,
            65536, 65537, 70000, 1000000, Integer.MAX_VALUE,
        };

        String msg;
        RpcErrorTag etag;
        for (Integer idx: badIndices) {
            XmlNode xn = new XmlNode(XML_ROOT).
                add(new XmlNode("name", "fcond"));
            XmlNode xmatches = new XmlNode("vtn-flow-matches");
            if (idx == null) {
                msg = "Match index cannot be null";
                etag = RpcErrorTag.MISSING_ELEMENT;
                xmatches.add(new XmlNode("vtn-flow-match"));
            } else {
                msg = "Invalid match index: " + idx;
                etag = RpcErrorTag.BAD_ELEMENT;
                FlowMatchParams fmp = new FlowMatchParams().setIndex(idx);
                xmatches.add(fmp.toXmlNode("vtn-flow-match"));
            }
            xn.add(xmatches);

            VTNFlowCondition vfcond = unmarshal(um, xn.toString(), type);
            try {
                vfcond.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Null name.
        msg = "Flow condition name cannot be null";
        etag = RpcErrorTag.MISSING_ELEMENT;
        String xml = new XmlNode(XML_ROOT).toString();
        VTNFlowCondition vfcond = unmarshal(um, xml, type);
        try {
            vfcond.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Duplicate index.
        int badIndex = 31;
        XmlNode xn = new XmlNode(XML_ROOT).add(new XmlNode("name", "fcond"));
        XmlNode xmatches = new XmlNode("vtn-flow-matches").
            add(new FlowMatchParams(badIndex).toXmlNode("vtn-flow-match"));
        msg = "Duplicate match index: " + badIndex;
        etag = RpcErrorTag.BAD_ELEMENT;
        for (int i = 1; i <= badIndex + 20; i++) {
            FlowMatchParams fmp = new FlowMatchParams().setIndex(i);
            xmatches.add(fmp.toXmlNode("vtn-flow-match"));
        }
        xml = xn.add(xmatches).toString();
        vfcond = unmarshal(um, xml, type);
        try {
            vfcond.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Test case for {@link VTNFlowCondition#match(FlowMatchContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        // Create an empty flow condition.
        VtnFlowCondition vfc = new VtnFlowConditionBuilder().
            setName(new VnodeName("empty")).build();
        VTNFlowCondition empty = new VTNFlowCondition(vfc);

        // 10000: IP DSCP == 35
        List<VtnFlowMatch> vmatches = new ArrayList<>();
        Short dscp = 35;
        Inet4MatchParams i4params = new Inet4MatchParams().setDscp(dscp);
        FlowMatchParams params = new FlowMatchParams(10000);
        params.setInet4Params(i4params);
        vmatches.add(params.toVtnFlowMatchBuilder().build());

        // 10: ether type == 0x1234
        Integer ethType = 0x1234;
        EtherMatchParams eparams = new EtherMatchParams().
            setEtherType(ethType);
        params.reset().setIndex(10).setEtherParams(eparams);
        vmatches.add(params.toVtnFlowMatchBuilder().build());

        // 1500: TCP dst port 1000-2000
        int dstFrom = 1000;
        int dstTo = 2000;
        TcpMatchParams tparams = new TcpMatchParams().
            setDestinationPortFrom(dstFrom).
            setDestinationPortTo(dstTo);
        params.reset().setIndex(1500).setLayer4Params(tparams);
        vmatches.add(params.toVtnFlowMatchBuilder().build());

        // 300: Source MAC == mac1 && VLAN ID == 3
        DataGenerator gen = new DataGenerator();
        EtherAddress mac1 = new EtherAddress(gen.getMacAddress());
        Integer vlan = 3;
        eparams.reset().setSourceAddress(mac1.getAddress()).setVlanId(vlan);
        params.reset().setIndex(300).setEtherParams(eparams);
        vmatches.add(params.toVtnFlowMatchBuilder().build());

        // 150: Source IP address in 192.168.233.0/24
        IpNetwork srcIp = IpNetwork.create("192.168.233.0/24");
        i4params.reset().setSourceNetwork(srcIp);
        params.reset().setIndex(150).setInet4Params(i4params);
        vmatches.add(params.toVtnFlowMatchBuilder().build());

        vfc = new VtnFlowConditionBuilder().
            setName(new VnodeName("fcond")).
            setVtnFlowMatch(vmatches).build();
        VTNFlowCondition vfcond = new VTNFlowCondition(vfc);

        // Create an untagged Ethernet frame with type 0x5555.
        EtherAddress mac2 = new EtherAddress(gen.getMacAddress());
        EtherAddress mac3 = new EtherAddress(gen.getMacAddress());
        EtherMatchParams ether = new EtherMatchParams().
            setSourceAddress(mac2.getAddress()).
            setDestinationAddress(mac3.getAddress()).
            setEtherType(Integer.valueOf(0x5555)).
            setVlanId(Integer.valueOf(EtherHeader.VLAN_NONE)).
            setVlanPriority(Short.valueOf((short)0));
        TestMatchContext ctx = new TestMatchContext().setEtherHeader(ether);

        // Empty flow condition should match every packet.
        assertEquals(true, empty.match(ctx));
        ctx.checkMatchFields().reset();

        // vfcond should not match this packet.
        assertEquals(false, vfcond.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_TYPE, FlowMatchType.DL_SRC).
            reset();

        // Create an Ethernet frame with VLAN tag(vid=10) and type 0x1234.
        // Flow match at index 10 should match this packet.
        ctx.setEtherHeader(ether.setEtherType(ethType).
                           setVlanId(Integer.valueOf(10)).
                           setVlanPriority(Short.valueOf((short)10)));
        assertEquals(true, empty.match(ctx));
        ctx.checkMatchFields().reset();
        assertEquals(true, vfcond.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_TYPE).reset();

        // Create an Ethernet frame to be matched by match at index 300.
        ctx.setEtherHeader(ether.setSourceAddress(mac1.getAddress()).
                           setEtherType(Integer.valueOf(0x3333)).
                           setVlanId(vlan).
                           setVlanPriority(Short.valueOf((short)0)));
        assertEquals(true, empty.match(ctx));
        ctx.checkMatchFields().reset();
        assertEquals(true, vfcond.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_TYPE, FlowMatchType.DL_SRC).
            reset();

        // Create an UDP packet to be matched by match at index 10000.
        Short ipUdp = InetProtocols.UDP.shortValue();
        Integer srcPort = gen.getPort();
        Integer dstPort = gen.getPort();
        IpNetwork ipaddr1 = IpNetwork.create(gen.getInetAddress());
        IpNetwork ipaddr2 = IpNetwork.create(gen.getInetAddress());
        Integer ethIpv4 = EtherTypes.IPV4.intValue();
        ether.setSourceAddress(mac2.getAddress()).setEtherType(ethIpv4).
            setVlanId(Integer.valueOf(4095)).
            setVlanPriority(Short.valueOf((short)2));
        Inet4MatchParams ipv4 = new Inet4MatchParams().
            setSourceNetwork(ipaddr1).setDestinationNetwork(ipaddr2).
            setProtocol(ipUdp).setDscp(dscp);
        UdpMatchParams udp = new UdpMatchParams().
            setSourcePortFrom(srcPort).
            setDestinationPortFrom(dstPort);
        ctx.setEtherHeader(ether).setInetHeader(ipv4).setLayer4Header(udp);
        assertEquals(true, empty.match(ctx));
        ctx.checkMatchFields().reset();
        assertEquals(true, vfcond.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_TYPE, FlowMatchType.DL_SRC,
                             FlowMatchType.IP_SRC, FlowMatchType.IP_PROTO,
                             FlowMatchType.IP_DSCP).reset();

        // Change DSCP so that the packet is not be matched.
        Short anotherDscp = Short.valueOf((short)(dscp.shortValue() + 1));
        ipv4.setDscp(anotherDscp);
        assertEquals(true, empty.match(ctx));
        ctx.checkMatchFields().reset();
        assertEquals(false, vfcond.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_TYPE, FlowMatchType.DL_SRC,
                             FlowMatchType.IP_SRC, FlowMatchType.IP_PROTO,
                             FlowMatchType.IP_DSCP).reset();

        // Create an ICMP packet to be matched by FlowMatch at index 150.
        IpNetwork inetSrc = IpNetwork.create("192.168.233.98");
        IcmpMatchParams icmp = new IcmpMatchParams().
            setType(gen.getIcmpValue()).setCode(gen.getIcmpValue());
        Short ipIcmp = InetProtocols.ICMP.shortValue();
        ipv4.setSourceNetwork(inetSrc).setDestinationNetwork(ipaddr2).
            setProtocol(ipIcmp).setDscp(dscp);
        ether.setVlanId(Integer.valueOf(30)).
            setVlanPriority(Short.valueOf((short)1));
        ctx.setEtherHeader(ether).setInetHeader(ipv4).setLayer4Header(icmp);
        assertEquals(true, empty.match(ctx));
        ctx.checkMatchFields().reset();
        assertEquals(true, vfcond.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_TYPE, FlowMatchType.IP_SRC).
            reset();

        // Create a TCP packet.
        int[] ports = {
            dstFrom - 1,
            dstFrom,
            dstFrom + ((dstTo - dstFrom) >> 1),
            dstTo,
            dstTo + 1,
        };

        Short ipTcp = InetProtocols.TCP.shortValue();
        TcpMatchParams tcp = new TcpMatchParams();
        ipv4.setSourceNetwork(ipaddr1).setProtocol(ipTcp);
        ctx.setEtherHeader(ether).setInetHeader(ipv4).setLayer4Header(tcp);
        Set<FlowMatchType> expectedTypes = EnumSet.of(
            FlowMatchType.DL_TYPE, FlowMatchType.DL_SRC, FlowMatchType.IP_SRC,
            FlowMatchType.IP_PROTO, FlowMatchType.TCP_DST);
        for (int port: ports) {
            tcp.setSourcePortFrom(gen.getPort()).setDestinationPortFrom(port);
            assertEquals(true, empty.match(ctx));
            ctx.checkMatchFields().reset();

            // If the destination port is in range [1000, 2000], the match at
            // index 1500 should match the packet.
            // Otherwise the last match should match the packet.
            if (port < dstFrom || port > dstTo) {
                expectedTypes.add(FlowMatchType.IP_DSCP);
            } else {
                expectedTypes.remove(FlowMatchType.IP_DSCP);
            }

            assertEquals(true, vfcond.match(ctx));
            ctx.checkMatchFields(expectedTypes).reset();
        }

        // Create a TCP packet that should not be mached.
        tcp.setSourcePortFrom(gen.getPort()).
            setDestinationPortFrom(dstTo + 1);
        ipv4.setDscp(anotherDscp);
        assertEquals(true, empty.match(ctx));
        ctx.checkMatchFields().reset();
        expectedTypes.add(FlowMatchType.IP_DSCP);
        assertEquals(false, vfcond.match(ctx));
        ctx.checkMatchFields(expectedTypes);
    }
}
