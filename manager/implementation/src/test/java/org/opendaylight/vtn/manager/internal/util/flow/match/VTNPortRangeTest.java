/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestPortNumber;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnPortRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev130715.PortNumber;

/**
 * JUnit test for {@link VTNPortRange}.
 */
public class VTNPortRangeTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNPortRange} class.
     */
    private static final String  XML_ROOT = "vtn-port-range";

    /**
     * Return a list of {@link XmlValueType} instances that specifies XML node
     * types mapped to a {@link VTNPortRange} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        List<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(dlist,
                           new XmlValueType("port-from", Integer.class).
                           add(name).prepend(parent),
                           new XmlValueType("port-to", Integer.class).
                           add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNPortRange#create(VtnPortRange)}</li>
     *   <li>{@link VTNPortRange#create(PortNumber)}</li>
     *   <li>{@link VTNPortRange#VTNPortRange(int)}</li>
     *   <li>{@link VTNPortRange#VTNPortRange(VtnPortRange)}</li>
     *   <li>{@link VTNPortRange#VTNPortRange(PortNumber)}</li>
     *   <li>Getter methods.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        assertEquals(null, VTNPortRange.create((VtnPortRange)null));
        assertEquals(null, VTNPortRange.create((PortNumber)null));

        PortRangeParams[] ranges = {
            new PortRangeParams(0), new PortRangeParams(65535),
            new PortRangeParams(0, 0), new PortRangeParams(0, 65535),
            new PortRangeParams(1000, 1001), new PortRangeParams(333, 4444),
        };

        for (PortRangeParams range: ranges) {
            Integer from = range.getPortFrom();
            Integer to = range.getPortTo();
            if (to == null) {
                to = from;
            }
            PortNumber pfrom = new PortNumber(from);
            PortNumber pto = new PortNumber(to);

            VtnPortRange vpr = range.toTcpSourceRange();
            VTNPortRange vr = VTNPortRange.create(vpr);
            assertEquals(from, vr.getPortFrom());
            assertEquals(to, vr.getPortTo());
            assertEquals(pfrom, vr.getPortNumberFrom());
            assertEquals(pto, vr.getPortNumberTo());

            Integer port = range.getPortFrom();
            PortNumber pn = new PortNumber(port);
            vr = VTNPortRange.create(pn);
            assertEquals(port, vr.getPortFrom());
            assertEquals(port, vr.getPortTo());
            assertEquals(pn, vr.getPortNumberFrom());
            assertEquals(pn, vr.getPortNumberTo());

            vr = new VTNPortRange(port.intValue());
            assertEquals(port, vr.getPortFrom());
            assertEquals(port, vr.getPortTo());
            assertEquals(pn, vr.getPortNumberFrom());
            assertEquals(pn, vr.getPortNumberTo());
        }

        // port-from is missing.
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        ArrayList<VtnPortRange> vprList = new ArrayList<>();
        PortRangeParams params = new PortRangeParams();
        vprList.add(params.setPortTo(0).toTcpSourceRange());
        vprList.add(params.reset().setPortTo(1234).toTcpSourceRange());
        for (VtnPortRange vpr: vprList) {
            try {
                new VTNPortRange(vpr);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("port-from cannot be null", e.getMessage());
            }
        }

        try {
            new VTNPortRange((PortNumber)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals("port-from cannot be null", e.getMessage());
        }

        // Invalid port number.
        int[] badPorts = {
            Integer.MIN_VALUE, Integer.MIN_VALUE + 1, -0x70000000,
            -0x10000000, -0x1000, -999, -100, -10, -4, -3, -2, -1,
            0x10000, 0x10001, 0x4000000, 0x12345000, 0x70000000,
            0x7fff0000, Integer.MAX_VALUE - 1, Integer.MAX_VALUE,
        };
        for (int port: badPorts) {
            vprList.clear();
            PortNumber pnum = new TestPortNumber(port);
            VtnPortRange pr = mock(VtnPortRange.class);
            when(pr.getPortFrom()).thenReturn(pnum);
            vprList.add(pr);

            pr = mock(VtnPortRange.class);
            when(pr.getPortFrom()).thenReturn(new PortNumber(0));
            when(pr.getPortTo()).thenReturn(pnum);
            vprList.add(pr);
            for (VtnPortRange vpr: vprList) {
                try {
                    new VTNPortRange(vpr);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals("Invalid port number: " + port,
                                 e.getMessage());
                }
            }

            try {
                new VTNPortRange(port);
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("Invalid port number: " + port, e.getMessage());
            }

            try {
                new VTNPortRange(pnum);
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("Invalid port number: " + port, e.getMessage());
            }
        }

        // Invalid port range.
        int[] ports = {1, 2, 3, 10, 50, 100, 2222, 5555, 10000, 34567, 65534};
        for (int port: ports) {
            int from = port + 1;
            int to = port;
            VtnPortRange vpr = params.reset().setPortFrom(from).setPortTo(to).
                toTcpSourceRange();
            try {
                new VTNPortRange(vpr);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                String msg = "Invalid port range: from=" + from +
                    ", to=" + to;
                assertEquals(msg, e.getMessage());
            }

            from = port;
            to = 0;
            vpr = params.reset().setPortFrom(from).setPortTo(to).
                toTcpSourceRange();
            vpr = params.toTcpSourceRange();
            try {
                new VTNPortRange(vpr);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                String msg = "Invalid port range: from=" + from + ", to=" + to;
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VTNPortRange#match(int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        // Match to the specific port.
        int port = 10000;
        VTNPortRange vr = new VTNPortRange(new PortNumber(port));
        assertEquals(true, vr.match(port));
        for (int i = 0; i < port; i += 100) {
            assertEquals(false, vr.match(i));
        }
        for (int i = 1; i < 10000; i += 100) {
            assertEquals(false, vr.match(port + i));
        }

        // Match to port numbers in the specified range.
        int from = 100;
        int to = 200;
        PortRangeParams params = new PortRangeParams(from, to);
        vr = new VTNPortRange(params.toTcpSourceRange());
        for (int i = from; i <= to; i++) {
            assertEquals(true, vr.match(i));
        }
        for (int i = 0; i < from; i++) {
            assertEquals(false, vr.match(i));
        }
        for (int i = 1; i <= 100; i++) {
            assertEquals(false, vr.match(to + i));
        }
    }

    /**
     * Test case for object identity.
     *
     * <ul>
     *   <li>{@link VTNPortRange#equals(Object)}</li>
     *   <li>{@link VTNPortRange#hashCode()}</li>
     *   <li>{@link VTNPortRange#setConditionKey(StringBuilder)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        HashSet<String> keySet = new HashSet<>();

        PortRangeParams[] ranges = {
            new PortRangeParams(0), new PortRangeParams(0, 1),
            new PortRangeParams(0, 12345),
            new PortRangeParams(100), new PortRangeParams(100, 200),
            new PortRangeParams(100, 23456),
            new PortRangeParams(12345), new PortRangeParams(12345, 13000),
            new PortRangeParams(12345, 30000),
            new PortRangeParams(32768), new PortRangeParams(32768, 44444),
            new PortRangeParams(32768, 65535),
            new PortRangeParams(44444), new PortRangeParams(44444, 54321),
            new PortRangeParams(44444, 60000),
            new PortRangeParams(65535),
        };

        StringBuilder b = new StringBuilder();
        for (PortRangeParams range: ranges) {
            VTNPortRange vr1 = new VTNPortRange(range.toTcpSourceRange());
            VTNPortRange vr2 = new VTNPortRange(range.toUdpSourceRange());
            testEquals(set, vr1, vr2);

            b.setLength(0);
            vr1.setConditionKey(b);
            assertEquals(true, keySet.add(b.toString()));
            b.setLength(0);
            vr2.setConditionKey(b);
            assertEquals(false, keySet.add(b.toString()));
        }

        assertEquals(ranges.length, set.size());
        assertEquals(ranges.length, keySet.size());
    }

    /**
     * Test case for {@link VTNPortRange#verify()} and JAXB mapping.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        // Normal case.
        Class<VTNPortRange> type = VTNPortRange.class;
        Marshaller m = createMarshaller(type);
        Unmarshaller um = createUnmarshaller(type);
        Integer[] ports = {
            0, 1, 100, 2000, 30000, 65535,
        };
        for (Integer from: ports) {
            String xml = new XmlNode(XML_ROOT).
                add(new XmlNode("port-from", from)).toString();
            VTNPortRange vr = unmarshal(um, xml, type);
            vr.verify();
            assertEquals(from, vr.getPortFrom());
            assertEquals(from, vr.getPortTo());

            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("port-from", from)).
                add(new XmlNode("port-to", from)).toString();
            vr = unmarshal(um, xml, type);
            vr.verify();
            assertEquals(from, vr.getPortFrom());
            assertEquals(from, vr.getPortTo());

            xml = marshal(m, vr, type, XML_ROOT);
            VTNPortRange vr1 = unmarshal(um, xml, type);
            vr1.verify();
            assertEquals(vr, vr1);

            int f = from.intValue();
            if (f != 65535) {
                Integer[] toPorts = {
                    Integer.valueOf(f + 1),
                    Integer.valueOf(65535),
                };
                for (Integer to: toPorts) {
                    xml = new XmlNode(XML_ROOT).
                        add(new XmlNode("port-from", from)).
                        add(new XmlNode("port-to", to)).toString();
                    vr = unmarshal(um, xml, type);
                    vr.verify();
                    assertEquals(from, vr.getPortFrom());
                    assertEquals(to, vr.getPortTo());
                }
            }
        }

        // port-from is missing.
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String[] missings = {
            new XmlNode(XML_ROOT).toString(),
            new XmlNode(XML_ROOT).add(new XmlNode("port-to", 10)).toString(),
        };
        for (String xml: missings) {
            VTNPortRange vr = unmarshal(um, xml, type);
            try {
                vr.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("port-from cannot be null", e.getMessage());
            }
        }

        // Invalid port range.
        int[] testPorts = {
            1, 2, 3, 10, 50, 100, 2222, 5555, 10000, 34567, 65534
        };
        for (int port: testPorts) {
            int from = port + 1;
            int to = port;
            String xml = new XmlNode(XML_ROOT).
                add(new XmlNode("port-from", from)).
                add(new XmlNode("port-to", to)).toString();
            VTNPortRange vr = unmarshal(um, xml, type);
            try {
                vr.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                String msg = "Invalid port range: from=" + from +
                    ", to=" + to;
                assertEquals(msg, e.getMessage());
            }

            from = port;
            to = 0;
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("port-from", from)).
                add(new XmlNode("port-to", to)).toString();
            vr = unmarshal(um, xml, type);
            try {
                vr.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                String msg = "Invalid port range: from=" + from + ", to=" + to;
                assertEquals(msg, e.getMessage());
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));
    }
}
