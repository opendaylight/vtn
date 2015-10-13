/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.flow.cond.PortMatch;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;
import org.opendaylight.vtn.manager.flow.cond.UdpMatch;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link FlowConditionList}.
 */
public class FlowConditionListTest extends TestBase {
    /**
     * Root XML element name associated with {@link FlowConditionList} class.
     */
    private static final String  XML_ROOT = "flowconditions";

    /**
     * Ensure that {@link FlowConditionList} is mapped to both XML root element
     * and JSON object.
     */
    @Test
    public void testJAXB() {
        // Null list.
        FlowConditionList fl = new FlowConditionList(null);
        jaxbTest(fl, FlowConditionList.class, XML_ROOT);
        jsonTest(fl, FlowConditionList.class);

        // Empty list.
        List<FlowCondition> list = new ArrayList<FlowCondition>();
        fl = new FlowConditionList(list);
        jaxbTest(fl, FlowConditionList.class, XML_ROOT);
        jsonTest(fl, FlowConditionList.class);

        for (String name: createStrings("cond")) {
            for (List<FlowMatch> matches: createFlowMatchLists()) {
                FlowCondition fc = new FlowCondition(name, matches);
                list.add(fc);
                fl = new FlowConditionList(list);
                jaxbTest(fl, FlowConditionList.class, XML_ROOT);
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(FlowConditionList.class,
                      new XmlAttributeType("match", "index", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches"),
                      new XmlAttributeType("ethernet", "type", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"),
                      new XmlAttributeType("ethernet", "vlan", Short.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"),
                      new XmlAttributeType("ethernet", "vlanpri", Byte.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"),
                      new XmlAttributeType("inet4", "srcsuffix", Short.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"),
                      new XmlAttributeType("inet4", "dstsuffix", Short.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"),
                      new XmlAttributeType("inet4", "protocol", Short.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"),
                      new XmlAttributeType("inet4", "dscp", Byte.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"),
                      new XmlAttributeType("src", "from", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match", "tcp"),
                      new XmlAttributeType("src", "to", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match", "tcp"),
                      new XmlAttributeType("dst", "from", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match", "tcp"),
                      new XmlAttributeType("dst", "to", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match", "tcp"),
                      new XmlAttributeType("src", "from", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match", "udp"),
                      new XmlAttributeType("src", "to", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match", "udp"),
                      new XmlAttributeType("dst", "from", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match", "udp"),
                      new XmlAttributeType("dst", "to", Integer.class).
                      add(XML_ROOT, "flowcondition", "matches", "match", "udp"),
                      new XmlAttributeType("icmp", "type", Short.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"),
                      new XmlAttributeType("icmp", "code", Short.class).
                      add(XML_ROOT, "flowcondition", "matches", "match"));
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

            InetProtocols ipproto = InetProtocols.TCP;
            im = new Inet4Match(null, null, null, null,
                                Short.valueOf(ipproto.shortValue()), null);
            lm = new TcpMatch(new PortMatch(1000, 2000), new PortMatch(61234));
            list.add(new FlowMatch(em, im, lm));

            index = Integer.valueOf(123);
            ipproto = InetProtocols.UDP;
            im = new Inet4Match(null, null, null, null,
                                Short.valueOf(ipproto.shortValue()), null);
            lm = new UdpMatch(null, new PortMatch(20000, 30000));
            list.add(new FlowMatch(index, em, im, lm));

            index = Integer.valueOf(60000);
            ipproto = InetProtocols.ICMP;
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
