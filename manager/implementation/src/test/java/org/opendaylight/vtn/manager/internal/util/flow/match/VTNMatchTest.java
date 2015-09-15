/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

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

import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.packet.IcmpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.packet.TcpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.UdpHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.DataGenerator;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;

import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;

/**
 * JUnit test for {@link VTNMatch}.
 */
public class VTNMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNMatch} class.
     */
    private static final String  XML_ROOT = "vtn-match";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNMatch} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        String[] p = XmlDataType.addPath(name, parent);
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.addAll(VTNEtherMatchTest.getXmlDataTypes("vtn-ether-match", p));
        dlist.addAll(VTNInet4MatchTest.getXmlDataTypes("vtn-inet4-match", p));
        dlist.addAll(VTNTcpMatchTest.getXmlDataTypes("vtn-tcp-match", p));
        dlist.addAll(VTNUdpMatchTest.getXmlDataTypes("vtn-udp-match", p));
        dlist.addAll(VTNIcmpMatchTest.getXmlDataTypes("vtn-icmp-match", p));

        return dlist;
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNMatch#VTNMatch()}</li>
     *   <li>{@link VTNMatch#VTNMatch(VTNEtherMatch,VTNInetMatch,VTNLayer4Match)}</li>
     *   <li>{@link VTNMatch#VTNMatch(Match)}</li>
     *   <li>{@link VTNMatch#set(FlowMatch)}</li>
     *   <li>{@link VTNMatch#complete()}</li>
     *   <li>{@link VTNMatch#toDataFlowMatchBuilder()}</li>
     *   <li>Getter methods</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        VTNMatch empty = new VTNMatch();
        assertEquals(null, empty.getEtherMatch());
        assertEquals(null, empty.getInetMatch());
        assertEquals(null, empty.getLayer4Match());

        empty = new VTNMatch((VTNEtherMatch)null, (VTNInetMatch)null,
                             (VTNLayer4Match)null);
        assertEquals(null, empty.getEtherMatch());
        assertEquals(null, empty.getInetMatch());
        assertEquals(null, empty.getLayer4Match());

        empty = new VTNMatch((Match)null);
        assertEquals(null, empty.getEtherMatch());
        assertEquals(null, empty.getInetMatch());
        assertEquals(null, empty.getLayer4Match());

        Map<MatchParams, MatchParams> cases = MatchParams.createMatches();
        for (Map.Entry<MatchParams, MatchParams> entry: cases.entrySet()) {
            MatchParams params = entry.getKey();
            MatchParams expected = entry.getValue();
            VTNMatch vmatch = params.toVTNMatch();
            expected.verify(vmatch);
        }

        // Ethernet condition will be omitted if it is empty.
        MatchParams params = new MatchParams().
            setEtherParams(new EtherMatchParams());
        new MatchParams().verify(params.toVTNMatch());

        // Inconsistent Ethernet type.
        int etype = 0x806;
        EtherMatchParams eparams = new EtherMatchParams().
            setEtherType(Integer.valueOf(etype));
        Inet4MatchParams i4params = new Inet4MatchParams();
        String msg = "Ethernet type conflict: type=0x" +
            Integer.toHexString(etype) + ", expected=0x" +
            Integer.toHexString(EtherTypes.IPv4.intValue());
        params = new MatchParams().setEtherParams(eparams).
            setInet4Params(i4params);
        FlowMatch fm = params.toFlowMatch();
        VTNMatch vmatch = new VTNMatch();
        try {
            vmatch.set(fm);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals(msg, st.getDescription());
        }

        VTNEtherMatch em = new VTNEtherMatch(null, null, etype, null, null);
        VTNInetMatch im = new VTNInet4Match();
        try {
            new VTNMatch(em, im, null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals(msg, st.getDescription());
        }

        // Inconsistent IP protocol number.
        short proto = 17;
        i4params.setProtocol(Short.valueOf(proto));
        TcpMatchParams tparams = new TcpMatchParams();
        msg = "IP protocol conflict: proto=" + proto + ", expected=" +
            IPProtocols.TCP.shortValue();
        params = new MatchParams().
            setInet4Params(i4params).
            setLayer4Params(tparams);
        fm = params.toFlowMatch();
        try {
            vmatch.set(fm);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals(msg, st.getDescription());
        }

        im = new VTNInet4Match(null, null, proto, null);
        VTNTcpMatch tm = new VTNTcpMatch();
        try {
            new VTNMatch(null, im, tm);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals(msg, st.getDescription());
        }
    }

    /**
     * Test case for {@link VTNMatch#getInetProtocol()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetInetProtocol() throws Exception {
        VTNMatch vmatch = new VTNMatch();
        assertEquals(null, vmatch.getInetProtocol());

        VTNEtherMatch em = new VTNEtherMatch(EtherTypes.IPv4.intValue());
        vmatch = new VTNMatch(em, null, null);
        assertEquals(null, vmatch.getInetProtocol());

        IpNetwork ipSrc = IpNetwork.create("10.20.30.40");
        IpNetwork ipDst = IpNetwork.create("192.168.10.1");
        Short[] protocols = {
            null, 1, 2, 6, 14, 63, 64, 126, 127,
        };
        for (Short proto: protocols) {
            VTNInet4Match im =
                new VTNInet4Match(ipSrc, ipDst, proto, (short)10);
            vmatch = new VTNMatch(null, im, null);
            assertEquals(proto, vmatch.getInetProtocol());
        }

        VTNTcpMatch tmatch = new VTNTcpMatch();
        vmatch = new VTNMatch(null, null, tmatch);
        assertEquals(IPProtocols.TCP.shortValue(),
                     vmatch.getInetProtocol().shortValue());

        VTNUdpMatch umatch = new VTNUdpMatch();
        vmatch = new VTNMatch(null, null, umatch);
        assertEquals(IPProtocols.UDP.shortValue(),
                     vmatch.getInetProtocol().shortValue());
    }

    /**
     * Test case for object identity.
     *
     * <ul>
     *   <li>{@link VTNMatch#equals(Object)}</li>
     *   <li>{@link VTNMatch#hashCode()}</li>
     *   <li>{@link VTNMatch#getConditionKey()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        HashSet<String> keySet = new HashSet<>();

        SalNode[] nodes = {
            new SalNode(1L),
            new SalNode(-1L),
        };
        int[] priorities = {
            0, 30, 65535,
        };
        SalPort[] ports = {
            null,
            new SalPort(1L, 1L),
            new SalPort(1L, 2L),
            new SalPort(2L, 1L),
            new SalPort(2L, 2L),
            new SalPort(-1L, 1L),
            new SalPort(-1L, 0xffffff00L),
        };

        Map<MatchParams, MatchParams> cases = MatchParams.createMatches();
        for (Map.Entry<MatchParams, MatchParams> entry: cases.entrySet()) {
            MatchParams params = entry.getKey();
            MatchParams expected = entry.getValue();
            VTNMatch vmatch1 = params.toVTNMatch();
            VTNMatch vmatch2 = expected.toVTNMatch();
            testEquals(set, vmatch1, vmatch2);

            String key1 = vmatch1.getConditionKey();
            String key2 = vmatch2.getConditionKey();
            assertEquals(true, keySet.add(key1));
            assertEquals(false, keySet.add(key2));

            for (SalNode node: nodes) {
                for (int pri: priorities) {
                    for (SalPort port: ports) {
                        key1 = vmatch1.getFlowKey(node, pri, port);
                        key2 = vmatch2.getFlowKey(node, pri, port);
                        assertEquals(true, keySet.add(key1));
                        assertEquals(false, keySet.add(key2));
                    }
                }
            }
        }

        assertEquals(cases.size(), set.size());

        int expected = (ports.length * priorities.length * nodes.length *
                        cases.size()) + cases.size();
        assertEquals(expected, keySet.size());
    }

    /**
     * Test case for {@link VTNMatch#verify()} and JAXB mapping.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        // Normal case.
        Class<VTNMatch> type = VTNMatch.class;
        Marshaller m = createMarshaller(type);
        Unmarshaller um = createUnmarshaller(type);
        Map<MatchParams, MatchParams> cases = MatchParams.createMatches();
        for (Map.Entry<MatchParams, MatchParams> entry: cases.entrySet()) {
            MatchParams params = entry.getKey();
            MatchParams expected = entry.getValue();
            String xml = params.toXmlNode(XML_ROOT).toString();
            VTNMatch vmatch = unmarshal(um, xml, type);
            vmatch.verify();
            expected.verify(vmatch);

            xml = expected.toXmlNode(XML_ROOT).toString();
            VTNMatch vmatch1 = unmarshal(um, xml, type);
            vmatch1.verify();
            assertEquals(vmatch, vmatch1);

            xml = marshal(m, vmatch, type, XML_ROOT);
            vmatch1 = unmarshal(um, xml, type);
            vmatch1.verify();
            assertEquals(vmatch, vmatch1);
        }

        // Inconsistent Ethernet type.
        int etype = 0x806;
        EtherMatchParams eparams = new EtherMatchParams().
            setEtherType(Integer.valueOf(etype));
        Inet4MatchParams i4params = new Inet4MatchParams();
        MatchParams params = new MatchParams().
            setEtherParams(eparams).
            setInet4Params(i4params);
        String xml = params.toXmlNode(XML_ROOT).toString();
        VTNMatch vmatch = unmarshal(um, xml, type);
        try {
            vmatch.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            String msg = "Ethernet type conflict: type=0x" +
                Integer.toHexString(etype) + ", expected=0x" +
                Integer.toHexString(EtherTypes.IPv4.intValue());
            assertEquals(msg, st.getDescription());
        }

        // Inconsistent IP protocol number.
        short proto = 17;
        i4params.setProtocol(Short.valueOf(proto));
        TcpMatchParams tparams = new TcpMatchParams();
        params = new MatchParams().
            setInet4Params(i4params).
            setLayer4Params(tparams);
        xml = params.toXmlNode(XML_ROOT).toString();
        vmatch = unmarshal(um, xml, type);
        try {
            vmatch.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            String msg = "IP protocol conflict: proto=" + proto +
                ", expected=" + IPProtocols.TCP.shortValue();
            assertEquals(msg, st.getDescription());
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Test case for {@link VTNMatch#match(FlowMatchContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        DataGenerator gen = new DataGenerator();
        ArrayList<TestMatchContext> ipPackets = new ArrayList<>();

        TcpMatchParams tcp = new TcpMatchParams();
        tcp.setSourcePortFrom(gen.getPort()).
            setDestinationPortFrom(gen.getPort());
        TestMatchContext packet = new TestMatchContext().
            setInetHeader(new Inet4MatchParams().
                          setSourceAddress(gen.getInetAddress()).
                          setDestinationAddress(gen.getInetAddress()).
                          setProtocol(IPProtocols.TCP.shortValue()).
                          setDscp(Short.valueOf((short)0))).
            setLayer4Header(new TcpMatchParams().
                            setSourcePortFrom(gen.getPort()).
                            setDestinationPortFrom(gen.getPort()));
        ipPackets.add(packet);

        packet = new TestMatchContext().
            setInetHeader(new Inet4MatchParams().
                          setSourceAddress(gen.getInetAddress()).
                          setDestinationAddress(gen.getInetAddress()).
                          setProtocol(IPProtocols.UDP.shortValue()).
                          setDscp(Short.valueOf((short)1))).
            setLayer4Header(new UdpMatchParams().
                            setSourcePortFrom(gen.getPort()).
                            setDestinationPortFrom(gen.getPort()));
        ipPackets.add(packet);

        packet = new TestMatchContext().
            setInetHeader(new Inet4MatchParams().
                          setSourceAddress(gen.getInetAddress()).
                          setDestinationAddress(gen.getInetAddress()).
                          setProtocol(IPProtocols.ICMP.shortValue()).
                          setDscp(Short.valueOf((short)2))).
            setLayer4Header(new IcmpMatchParams().
                            setType(gen.getIcmpValue()).
                            setCode(gen.getIcmpValue()));
        ipPackets.add(packet);

        packet = new TestMatchContext().
            setInetHeader(new Inet4MatchParams().
                          setSourceAddress(gen.getInetAddress()).
                          setDestinationAddress(gen.getInetAddress()).
                          setProtocol((short)123).
                          setDscp(Short.valueOf((short)3)));
        ipPackets.add(packet);

        packet = new TestMatchContext().
            setInetHeader(new Inet4MatchParams().
                          setSourceAddress(gen.getInetAddress()).
                          setDestinationAddress(gen.getInetAddress()).
                          setProtocol(IPProtocols.TCP.shortValue()).
                          setDscp(Short.valueOf((short)4))).
            setLayer4Header(new TcpMatchParams().
                            setSourcePortFrom(gen.getPort()).
                            setDestinationPortFrom(gen.getPort()));
        ipPackets.add(packet);

        ArrayList<TestMatchContext> etherPackets = new ArrayList<>();
        Integer[] vlans = {
            EtherHeader.VLAN_NONE, 1, 10, 4095,
        };

        short pcp = 0;
        int ipv4Type = EtherTypes.IPv4.intValue();
        for (Integer vlan: vlans) {
            // Unsupported packet.
            packet = new TestMatchContext().
                setEtherHeader(new EtherMatchParams().
                               setSourceAddress(gen.getMacAddress()).
                               setDestinationAddress(gen.getMacAddress()).
                               setEtherType(0x4444).setVlanId(vlan).
                               setVlanPriority(Short.valueOf(pcp)));
            etherPackets.add(packet);
            pcp = (short)((pcp + 1) & 0x7);

            // ARP packet.
            packet = new TestMatchContext().
                setEtherHeader(new EtherMatchParams().
                               setSourceAddress(gen.getMacAddress()).
                               setDestinationAddress(gen.getMacAddress()).
                               setEtherType(EtherTypes.ARP.intValue()).
                               setVlanId(vlan).
                               setVlanPriority(Short.valueOf(pcp)));
            etherPackets.add(packet);
            pcp = (byte)((pcp + 1) & 0x7);

            // IPv4 packets.
            for (TestMatchContext ipv4: ipPackets) {
                packet = ipv4.clone().
                    setEtherHeader(new EtherMatchParams().
                                   setSourceAddress(gen.getMacAddress()).
                                   setDestinationAddress(gen.getMacAddress()).
                                   setEtherType(ipv4Type).
                                   setVlanId(vlan).
                                   setVlanPriority(Short.valueOf(pcp)));
                etherPackets.add(packet);
                pcp = (byte)((pcp + 1) & 0x7);
            }
        }

        List<VTNMatch> empties = new ArrayList<>();
        Collections.addAll(empties,
                           new VTNMatch(),
                           new MatchParams().setEtherParams(
                               new EtherMatchParams()).toVTNMatch());
        for (TestMatchContext ctx: etherPackets) {
            // Empty condition should match every packet.
            for (VTNMatch vmatch: empties) {
                assertEquals(true, vmatch.match(ctx));
                ctx.checkMatchFields();
            }

            ethernetMatchTest(ctx);
            ipv4MatchTest(ctx);
            layer4MatchTest(ctx);
        }
    }

    /**
     * Test case for matching Ethernet header.
     *
     * @param ctx  A {@link TestMatchContext} instance for test.
     * @throws Exception  An error occurred.
     */
    private void ethernetMatchTest(TestMatchContext ctx) throws Exception {
        EtherHeader ether = ctx.getEtherHeader();
        EtherAddress src = ether.getSourceAddress();
        EtherAddress dst = ether.getDestinationAddress();
        int type = ether.getEtherType();
        int vlan = ether.getVlanId();
        short pcp = ether.getVlanPriority();
        boolean tagged = vlan != EtherHeader.VLAN_NONE;

        EtherAddress anotherSrc =
            new EtherAddress(src.getAddress() ^ 0x8000L);
        EtherAddress anotherDst =
            new EtherAddress(dst.getAddress() ^ 0x10L);
        Integer anotherType = type ^ 0x1;
        Integer anotherVlan = vlan ^ 0x2;
        Short anotherPcp = (tagged) ? (short)(pcp ^ 0x3) : null;

        // Create flow match that specifies all Ethernet header fields.
        EtherMatchParams eparams = new EtherMatchParams().
            setSourceAddress(src.getAddress()).
            setDestinationAddress(dst.getAddress()).
            setEtherType(Integer.valueOf(type)).
            setVlanId(Integer.valueOf(vlan));
        Set<FlowMatchType> expectedTypes = EnumSet.of(
            FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
            FlowMatchType.DL_TYPE);
        if (tagged) {
            eparams.setVlanPriority(Short.valueOf(pcp));
            expectedTypes.add(FlowMatchType.DL_VLAN_PCP);
        }

        VTNMatch vmatch = new MatchParams().
            setEtherParams(eparams).toVTNMatch();
        assertEquals(true, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        // Ensure that the packet matches only if all conditions are met.
        ctx.reset();
        eparams.setSourceAddress(anotherSrc);
        vmatch = new MatchParams().setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC);

        ctx.reset();
        eparams.setSourceAddress(src.getAddress()).
            setDestinationAddress(anotherDst);
        vmatch = new MatchParams().setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST);

        ctx.reset();
        eparams.setDestinationAddress(dst.getAddress()).
            setEtherType(anotherType);
        vmatch = new MatchParams().setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                             FlowMatchType.DL_TYPE);

        ctx.reset();
        eparams.setEtherType(type).setVlanId(anotherVlan);
        vmatch = new MatchParams().setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                             FlowMatchType.DL_TYPE);

        if (tagged) {
            ctx.reset();
            eparams.setEtherType(type).
                setVlanId(Integer.valueOf(EtherHeader.VLAN_NONE)).
                setVlanPriority((Short)null);
            vmatch = new MatchParams().setEtherParams(eparams).toVTNMatch();
            assertEquals(false, vmatch.match(ctx));
            ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                 FlowMatchType.DL_TYPE);
        }

        ctx.reset();
        eparams.setEtherType(type).setVlanId(Integer.valueOf(vlan)).
            setVlanPriority(anotherPcp);
        vmatch = new MatchParams().setEtherParams(eparams).toVTNMatch();
        assertEquals(!tagged, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        ctx.reset();
    }

    /**
     * Test case for matching IPv4 header.
     *
     * @param ctx  A {@link TestMatchContext} instance for test.
     * @throws Exception  An error occurred.
     */
    private void ipv4MatchTest(TestMatchContext ctx) throws Exception {
        EtherHeader ether = ctx.getEtherHeader();
        EtherAddress src = ether.getSourceAddress();
        EtherAddress dst = ether.getDestinationAddress();
        int type = ether.getEtherType();
        int vlan = ether.getVlanId();
        short pcp = ether.getVlanPriority();
        boolean tagged = vlan != EtherHeader.VLAN_NONE;

        EtherAddress anotherSrc =
            new EtherAddress(src.getAddress() ^ 0x40000L);
        EtherAddress anotherDst =
            new EtherAddress(dst.getAddress() ^ 0x20L);
        Integer anotherType = type ^ 0x10;
        Integer anotherVlan = vlan ^ 0x100;
        Short anotherPcp = (tagged) ? (short)(pcp ^ 0x1) : null;

        // Create flow match that specifies all header fields.
        EtherMatchParams eparams = new EtherMatchParams().
            setSourceAddress(src.getAddress()).
            setDestinationAddress(dst.getAddress()).
            setEtherType(Integer.valueOf(type)).
            setVlanId(Integer.valueOf(vlan));
        Set<FlowMatchType> expectedTypes = EnumSet.of(
            FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
            FlowMatchType.DL_TYPE);
        if (tagged) {
            eparams.setVlanPriority(Short.valueOf(pcp));
            expectedTypes.add(FlowMatchType.DL_VLAN_PCP);
        }

        Inet4MatchParams i4params = new Inet4MatchParams();
        InetHeader inet = ctx.getInetHeader();
        if (inet == null) {
            // IPv4 conditions should never be used.
            i4params.setSourceNetwork(IpNetwork.create("10.0.0.0/1")).
                setDestinationNetwork(IpNetwork.create("192.168.10.1/8")).
                setProtocol((short)6).setDscp((short)1);

            VTNMatch vmatch = new MatchParams().
                setEtherParams(eparams.setEtherType(EtherTypes.IPv4.
                                                    intValue())).
                setInet4Params(i4params).toVTNMatch();
            assertEquals(false, vmatch.match(ctx));

            // Ether type will be tested before VLAN tag.
            ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                 FlowMatchType.DL_TYPE);
            return;
        }

        IpNetwork ipSrc = inet.getSourceAddress();
        IpNetwork ipDst = inet.getDestinationAddress();
        short ipProto = inet.getProtocol();
        short ipDscp = inet.getDscp();
        i4params.setSourceNetwork(ipSrc).setDestinationNetwork(ipDst).
            setProtocol(ipProto).setDscp(Short.valueOf(ipDscp));
        expectedTypes.add(FlowMatchType.IP_SRC);
        expectedTypes.add(FlowMatchType.IP_DST);
        expectedTypes.add(FlowMatchType.IP_PROTO);
        expectedTypes.add(FlowMatchType.IP_DSCP);

        assertTrue(ipSrc instanceof Ip4Network);
        IpNetwork anotherIpSrc =
            new Ip4Network(((Ip4Network)ipSrc).getAddress() ^ 0x100);
        assertTrue(ipDst instanceof Ip4Network);
        IpNetwork anotherIpDst =
            new Ip4Network(((Ip4Network)ipDst).getAddress() ^ 0x4000);
        Short anotherIpProto = Short.valueOf((short)(ipProto ^ 0x40));
        Short anotherIpDscp = Short.valueOf((short)(ipDscp ^ 0x8));

        MatchParams params = new MatchParams().
            setEtherParams(eparams).setInet4Params(i4params);
        VTNMatch vmatch = params.toVTNMatch();
        assertEquals(true, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        // Ensure that the packet matches only if all conditions are met.
        ctx.reset();
        eparams.setSourceAddress(anotherSrc);
        vmatch = params.setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC);

        ctx.reset();
        eparams.setSourceAddress(src.getAddress()).
            setDestinationAddress(anotherDst);
        vmatch = params.setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST);

        // DL_TYPE condition cannot be changed because IPv4 condition
        // will be configured.
        ctx.reset();
        eparams.setDestinationAddress(dst.getAddress()).
            setVlanId(anotherVlan);
        vmatch = params.setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                             FlowMatchType.DL_TYPE);

        if (tagged) {
            ctx.reset();
            eparams.setVlanId(Integer.valueOf(EtherHeader.VLAN_NONE)).
                setVlanPriority((Short)null);
            vmatch = params.setEtherParams(eparams).toVTNMatch();
            assertEquals(false, vmatch.match(ctx));
            ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                 FlowMatchType.DL_TYPE);
        }

        ctx.reset();
        eparams.setVlanId(Integer.valueOf(vlan)).setVlanPriority(anotherPcp);
        vmatch = params.setEtherParams(eparams).toVTNMatch();
        if (tagged) {
            assertEquals(false, vmatch.match(ctx));
            ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                 FlowMatchType.DL_TYPE,
                                 FlowMatchType.DL_VLAN_PCP);
            params.setEtherParams(eparams.setVlanPriority(Short.valueOf(pcp)));
        } else {
            assertEquals(true, vmatch.match(ctx));
            ctx.checkMatchFields(expectedTypes);
        }

        expectedTypes = EnumSet.of(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                   FlowMatchType.DL_TYPE, FlowMatchType.IP_SRC);
        if (tagged) {
            expectedTypes.add(FlowMatchType.DL_VLAN_PCP);
        }

        ctx.reset();
        i4params.setSourceNetwork(anotherIpSrc);
        vmatch = params.setInet4Params(i4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        ctx.reset();
        expectedTypes.add(FlowMatchType.IP_DST);
        i4params.setSourceNetwork(ipSrc).setDestinationNetwork(anotherIpDst);
        vmatch = params.setInet4Params(i4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        ctx.reset();
        expectedTypes.add(FlowMatchType.IP_PROTO);
        i4params.setDestinationNetwork(ipDst).setProtocol(anotherIpProto);
        vmatch = params.setInet4Params(i4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        ctx.reset();
        expectedTypes.add(FlowMatchType.IP_DSCP);
        i4params.setProtocol(Short.valueOf(ipProto)).setDscp(anotherIpDscp);
        vmatch = params.setInet4Params(i4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        ctx.reset();
    }

    /**
     * Test case for matching layer 4 header.
     *
     * @param ctx  A {@link TestMatchContext} instance for test.
     * @throws Exception  An error occurred.
     */
    private void layer4MatchTest(TestMatchContext ctx) throws Exception {
        EtherHeader ether = ctx.getEtherHeader();
        EtherAddress src = ether.getSourceAddress();
        EtherAddress dst = ether.getDestinationAddress();
        int type = ether.getEtherType();
        int vlan = ether.getVlanId();
        short pcp = ether.getVlanPriority();
        boolean tagged = vlan != EtherHeader.VLAN_NONE;

        EtherAddress anotherSrc =
            new EtherAddress(src.getAddress() ^ 0x200L);
        EtherAddress anotherDst =
            new EtherAddress(dst.getAddress() ^ 0x800000L);
        Integer anotherType = type ^ 0x4;
        Integer anotherVlan = vlan ^ 0x8;
        Short anotherPcp = (tagged) ? (short)(pcp ^ 0x2) : null;

        // Create flow match that specifies all header fields.
        EtherMatchParams eparams = new EtherMatchParams().
            setSourceAddress(src.getAddress()).
            setDestinationAddress(dst.getAddress()).
            setEtherType(Integer.valueOf(type)).
            setVlanId(Integer.valueOf(vlan));
        Set<FlowMatchType> expectedTypes = EnumSet.of(
            FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
            FlowMatchType.DL_TYPE);
        if (tagged) {
            eparams.setVlanPriority(Short.valueOf(pcp));
            expectedTypes.add(FlowMatchType.DL_VLAN_PCP);
        }

        Inet4MatchParams i4params = new Inet4MatchParams();
        InetHeader inet = ctx.getInetHeader();
        if (inet == null) {
            // IPv4 and layer 4 conditions should never be used.
            i4params.setSourceNetwork(IpNetwork.create("10.0.0.1")).
                setDestinationNetwork(IpNetwork.create("192.168.10.1")).
                setDscp((short)1);

            TcpMatchParams tparams = new TcpMatchParams().
                setSourcePortFrom(10).setDestinationPortFrom(300);
            VTNMatch vmatch = new MatchParams().
                setEtherParams(eparams.setEtherType(EtherTypes.IPv4.
                                                    intValue())).
                setInet4Params(i4params).
                setLayer4Params(tparams).toVTNMatch();
            assertEquals(false, vmatch.match(ctx));

            // Ether type will be tested before VLAN tag.
            ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                 FlowMatchType.DL_TYPE);
            return;
        }


        IpNetwork ipSrc = inet.getSourceAddress();
        IpNetwork ipDst = inet.getDestinationAddress();
        short ipProto = inet.getProtocol();
        short ipDscp = inet.getDscp();
        i4params.setSourceNetwork(ipSrc).setDestinationNetwork(ipDst).
            setProtocol(ipProto).setDscp(Short.valueOf(ipDscp));
        expectedTypes.add(FlowMatchType.IP_SRC);
        expectedTypes.add(FlowMatchType.IP_DST);
        expectedTypes.add(FlowMatchType.IP_PROTO);
        expectedTypes.add(FlowMatchType.IP_DSCP);

        Layer4Header l4 = ctx.getLayer4Header();
        Layer4MatchParams l4params = null;
        IPProtocols ipType = null;
        TcpHeader tcp = null;
        TcpMatchParams tparams = null;
        UdpHeader udp = null;
        UdpMatchParams uparams = null;
        IcmpHeader icmp = null;
        IcmpMatchParams iparams = null;
        FlowMatchType srcType = null;
        FlowMatchType dstType = null;
        int srcPort = -1;
        int dstPort = -1;
        Integer anotherSrcPort = null;
        Integer anotherDstPort = null;
        short icmpType = -1;
        short icmpCode = -1;
        Short anotherIcmpType = null;
        Short anotherIcmpCode = null;
        if (l4 instanceof TcpHeader) {
            ipType = IPProtocols.TCP;
            tcp = (TcpHeader)l4;
            srcPort = tcp.getSourcePort();
            dstPort = tcp.getDestinationPort();
            anotherSrcPort = Integer.valueOf(srcPort ^ 0x4);
            anotherDstPort = Integer.valueOf(dstPort ^ 0x80);
            tparams = new TcpMatchParams().setSourcePortFrom(srcPort).
                setDestinationPortFrom(dstPort);
            l4params = tparams;
            srcType = FlowMatchType.TCP_SRC;
            dstType = FlowMatchType.TCP_DST;
        } else if (l4 instanceof UdpHeader) {
            ipType = IPProtocols.UDP;
            udp = (UdpHeader)l4;
            srcPort = udp.getSourcePort();
            dstPort = udp.getDestinationPort();
            anotherSrcPort = Integer.valueOf(srcPort ^ 0x400);
            anotherDstPort = Integer.valueOf(dstPort ^ 0x8);
            uparams = new UdpMatchParams().setSourcePortFrom(srcPort).
                setDestinationPortFrom(dstPort);
            l4params = uparams;
            srcType = FlowMatchType.UDP_SRC;
            dstType = FlowMatchType.UDP_DST;
        } else if (l4 instanceof IcmpHeader) {
            ipType = IPProtocols.ICMP;
            icmp = (IcmpHeader)l4;
            icmpType = icmp.getIcmpType();
            icmpCode = icmp.getIcmpCode();
            anotherIcmpType = Short.valueOf((short)(icmpType ^ 0x8));
            anotherIcmpCode = Short.valueOf((short)(icmpCode ^ 0x80));
            iparams = new IcmpMatchParams().setType(icmpType).
                setCode(icmpCode);
            l4params = iparams;
            srcType = FlowMatchType.ICMP_TYPE;
            dstType = FlowMatchType.ICMP_CODE;
        }

        if (ipType == null) {
            // Layer 4 conditions should never be used.
            i4params.setProtocol(IPProtocols.TCP.shortValue());
            VTNMatch vmatch = new MatchParams().
                setEtherParams(eparams).
                setInet4Params(i4params).
                setLayer4Params(new TcpMatchParams().
                                setSourcePortFrom(12345).
                                setDestinationPortFrom(333)).
                toVTNMatch();
            assertEquals(false, vmatch.match(ctx));

            // IP protocol will be tested before DSCP.
            if (ipProto != IPProtocols.TCP.shortValue()) {
                expectedTypes.remove(FlowMatchType.IP_DSCP);
            }
            ctx.checkMatchFields(expectedTypes);
            return;
        }

        assertTrue(ipSrc instanceof Ip4Network);
        IpNetwork anotherIpSrc =
            new Ip4Network(((Ip4Network)ipSrc).getAddress() ^ 0x200000);
        assertTrue(ipDst instanceof Ip4Network);
        IpNetwork anotherIpDst =
            new Ip4Network(((Ip4Network)ipDst).getAddress() ^ 0x200);
        Short anotherIpProto = Short.valueOf((short)(ipProto ^ 0x4));
        Short anotherIpDscp = Short.valueOf((short)(ipDscp ^ 0x10));

        expectedTypes.add(srcType);
        expectedTypes.add(dstType);
        MatchParams params = new MatchParams().
            setEtherParams(eparams).setInet4Params(i4params).
            setLayer4Params(l4params);
        VTNMatch vmatch = params.toVTNMatch();
        assertEquals(true, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        // Ensure that the packet matches only if all conditions are met.
        ctx.reset();
        eparams.setSourceAddress(anotherSrc);
        vmatch = params.setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC);

        ctx.reset();
        eparams.setSourceAddress(src.getAddress()).
            setDestinationAddress(anotherDst);
        vmatch = params.setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST);

        // DL_TYPE condition cannot be changed because IPv4 condition
        // will be configured.
        ctx.reset();
        eparams.setDestinationAddress(dst.getAddress()).
            setVlanId(anotherVlan);
        vmatch = params.setEtherParams(eparams).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                             FlowMatchType.DL_TYPE);


        if (tagged) {
            ctx.reset();
            eparams.setVlanId(Integer.valueOf(EtherHeader.VLAN_NONE)).
                setVlanPriority((Short)null);
            vmatch = params.setEtherParams(eparams).toVTNMatch();
            assertEquals(false, vmatch.match(ctx));
            ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                 FlowMatchType.DL_TYPE);
        }

        ctx.reset();
        eparams.setVlanId(Integer.valueOf(vlan)).setVlanPriority(anotherPcp);
        vmatch = params.setEtherParams(eparams).toVTNMatch();
        if (tagged) {
            assertEquals(false, vmatch.match(ctx));
            ctx.checkMatchFields(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                 FlowMatchType.DL_TYPE,
                                 FlowMatchType.DL_VLAN_PCP);
            params.setEtherParams(eparams.setVlanPriority(Short.valueOf(pcp)));
        } else {
            assertEquals(true, vmatch.match(ctx));
            ctx.checkMatchFields(expectedTypes);
        }

        expectedTypes = EnumSet.of(FlowMatchType.DL_SRC, FlowMatchType.DL_DST,
                                   FlowMatchType.DL_TYPE, FlowMatchType.IP_SRC);
        if (tagged) {
            expectedTypes.add(FlowMatchType.DL_VLAN_PCP);
        }

        ctx.reset();
        i4params.setSourceNetwork(anotherIpSrc);
        vmatch = params.setInet4Params(i4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        ctx.reset();
        expectedTypes.add(FlowMatchType.IP_DST);
        i4params.setSourceNetwork(ipSrc).setDestinationNetwork(anotherIpDst);
        vmatch = params.setInet4Params(i4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        // IP_PROTO condition cannot be changed because L4 condition will be
        // configured.
        ctx.reset();
        Collections.addAll(expectedTypes, FlowMatchType.IP_PROTO,
                           FlowMatchType.IP_DSCP);
        i4params.setDestinationNetwork(ipDst).setDscp(anotherIpDscp);
        vmatch = params.setInet4Params(i4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);
        params.setInet4Params(i4params.setDscp(Short.valueOf(ipDscp)));

        if (tparams != null) {
            tparams.setSourcePortFrom(anotherSrcPort);
        } else if (uparams != null) {
            uparams.setSourcePortFrom(anotherSrcPort);
        } else {
            iparams.setType(anotherIcmpType);
        }
        ctx.reset();
        expectedTypes.add(srcType);
        vmatch = params.setLayer4Params(l4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);

        if (tparams != null) {
            tparams.setSourcePortFrom(srcPort).
                setDestinationPortFrom(anotherDstPort);
        } else if (uparams != null) {
            uparams.setSourcePortFrom(srcPort).
                setDestinationPortFrom(anotherDstPort);
        } else {
            iparams.setType(icmpType).setCode(anotherIcmpCode);
        }
        ctx.reset();
        expectedTypes.add(dstType);
        vmatch = params.setLayer4Params(l4params).toVTNMatch();
        assertEquals(false, vmatch.match(ctx));
        ctx.checkMatchFields(expectedTypes);
    }
}
