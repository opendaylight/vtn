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
import org.opendaylight.vtn.manager.XmlDataType;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.IPProtocols;

/**
 * JUnit test for {@link FlowCondition}.
 */
public class FlowConditionTest extends TestBase {
    /**
     * Root XML element name associated with {@link FlowCondition} class.
     */
    private static final String  XML_ROOT = "flowcondition";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link FlowCondition} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        String[] p = XmlDataType.addPath(
            "matches", XmlDataType.addPath(name, parent));
        return FlowMatchTest.getXmlDataTypes("match", p);
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        List<FlowMatch> empty = new ArrayList<FlowMatch>(0);
        for (String name: createStrings("cond")) {
            for (List<FlowMatch> matches: createFlowMatchLists()) {
                FlowCondition fc = new FlowCondition(name, matches);
                assertEquals(name, fc.getName());
                List<FlowMatch> fml = fc.getMatches();
                if (matches == null) {
                    assertEquals(empty, fml);
                } else {
                    assertEquals(matches, fml);
                }
                assertNotSame(matches, fml);
            }
        }

        String name = "name";
        FlowCondition fc = new FlowCondition(name, null);
        assertEquals(name, fc.getName());
        List<FlowMatch> fml = fc.getMatches();
        assertEquals(0, fml.size());

        List<FlowMatch> matches = new ArrayList<FlowMatch>();
        fc = new FlowCondition(name, matches);
        assertEquals(name, fc.getName());
        fml = fc.getMatches();
        assertEquals(0, fml.size());
        assertNotSame(matches, fml);

        matches.add(null);
        fc = new FlowCondition(name, matches);
        assertEquals(name, fc.getName());
        fml = fc.getMatches();
        assertEquals(1, fml.size());
        assertEquals(matches, fml);
        assertNotSame(matches, fml);
    }

    /**
     * Test case for {@link FlowCondition#equals(Object)} and
     * {@link FlowCondition#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        List<String> names = createStrings("cond");
        List<List<FlowMatch>> matchLists = createFlowMatchLists();

        for (String name: names) {
            for (List<FlowMatch> matches: matchLists) {
                FlowCondition fc1 = new FlowCondition(name, matches);
                FlowCondition fc2 = new FlowCondition(
                    copy(name), copy(matches, FlowMatch.class));
                testEquals(set, fc1, fc2);
            }
        }

        int required = names.size() * matchLists.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link FlowCondition#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "FlowCondition[";
        String suffix = "]";

        for (String name: createStrings("cond")) {
            for (List<FlowMatch> matches: createFlowMatchLists()) {
                FlowCondition fc = new FlowCondition(name, matches);
                String n = (name == null) ? null : "name=" + name;
                String m = (matches == null) ? null : "matches=" + matches;
                String required = joinStrings(prefix, suffix, ",", n, m);
                assertEquals(required, fc.toString());
            }
        }
    }

    /**
     * Ensure that {@link FlowCondition} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String name: createStrings("cond")) {
            for (List<FlowMatch> matches: createFlowMatchLists()) {
                FlowCondition fc = new FlowCondition(name, matches);
                serializeTest(fc);
            }
        }
    }

    /**
     * Ensure that {@link FlowCondition} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (String name: createStrings("cond")) {
            for (List<FlowMatch> matches: createFlowMatchLists()) {
                FlowCondition fc = new FlowCondition(name, matches);
                jaxbTest(fc, FlowCondition.class, XML_ROOT);
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(FlowCondition.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link FlowCondition} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (String name: createStrings("cond")) {
            for (List<FlowMatch> matches: createFlowMatchLists()) {
                FlowCondition fc = new FlowCondition(name, matches);
                jsonTest(fc, FlowCondition.class);
            }
        }
    }

    /**
     * Create a list of {@link FlowMatch} lists.
     *
     * @return  A list of {@link FlowMatch} lists.
     */
    private List<List<FlowMatch>> createFlowMatchLists() {
        List<List<FlowMatch>> list = new ArrayList<List<FlowMatch>>();
        list.add(null);
        List<FlowMatch> matches = new ArrayList<FlowMatch>();
        for (FlowMatch fm: createFlowMatches()) {
            List<FlowMatch> l = new ArrayList<FlowMatch>();
            l.add(fm);
            list.add(l);
            matches.add(fm);
            if (matches.size() > 1) {
                list.add(new ArrayList<FlowMatch>(matches));
            }
        }

        return list;
    }

    /**
     * Create a list of {@link FlowMatch} instances.
     *
     * @return  A list of {@link FlowMatch} instances.
     */
    private List<FlowMatch> createFlowMatches() {
        List<FlowMatch> list = new ArrayList<FlowMatch>();
        EtherAddress nullMac = null;

        try {
            byte[] srcMac = new byte[]{
                (byte)0xca, (byte)0x9c, (byte)0xeb,
                (byte)0xbd, (byte)0xb6, (byte)0xdc,
            };
            byte[] dstMac = {
                (byte)0x00, (byte)0x01, (byte)0x02,
                (byte)0x03, (byte)0x04, (byte)0x05,
            };
            EthernetAddress src = new EthernetAddress(srcMac);
            EthernetAddress dst = new EthernetAddress(dstMac);
            Integer type = Integer.valueOf(0x806);
            Short vlan = Short.valueOf((short)4095);
            Byte pri = Byte.valueOf((byte)7);

            EthernetMatch em = new EthernetMatch(src, dst, type, vlan, pri);
            InetMatch im = null;
            L4Match lm = null;
            list.add(new FlowMatch(em, im, lm));

            Integer index = Integer.valueOf(10);
            type = Integer.valueOf(0x800);
            em = new EthernetMatch(nullMac, nullMac, type, null, null);
            InetAddress srcIp = InetAddress.getByName("192.168.100.204");
            InetAddress dstIp = InetAddress.getByName("10.123.234.0");
            Short dstsuff = Short.valueOf((short)24);
            Short proto = Short.valueOf((short)60);
            Byte dscp = Byte.valueOf((byte)50);
            im = new Inet4Match(srcIp, null, dstIp, dstsuff, proto, dscp);
            list.add(new FlowMatch(index, em, im, lm));

            IPProtocols ipproto = IPProtocols.TCP;
            im = new Inet4Match(null, null, null, null,
                                Short.valueOf(ipproto.shortValue()), null);
            lm = new TcpMatch(new PortMatch(1000, 2000), new PortMatch(61234));
            list.add(new FlowMatch(em, im, lm));

            index = Integer.valueOf(123);
            ipproto = IPProtocols.UDP;
            im = new Inet4Match(null, null, null, null,
                                Short.valueOf(ipproto.shortValue()), null);
            lm = new UdpMatch(null, new PortMatch(20000, 30000));
            list.add(new FlowMatch(index, em, im, lm));

            index = Integer.valueOf(60000);
            ipproto = IPProtocols.ICMP;
            Short srcsuff = Short.valueOf((short)31);
            em = new EthernetMatch(src, dst, type, vlan, pri);
            im = new Inet4Match(srcIp, srcsuff, dstIp, dstsuff,
                                Short.valueOf(ipproto.shortValue()), dscp);
            lm = new IcmpMatch(Short.valueOf((short)3),
                               Short.valueOf((short)13));
            list.add(new FlowMatch(index, em, im, lm));
        } catch (Exception e) {
            unexpected(e);
        }

        return list;
    }
}
