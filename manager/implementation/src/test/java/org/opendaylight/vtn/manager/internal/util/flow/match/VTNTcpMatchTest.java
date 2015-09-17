/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;

import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.packet.TcpHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;

import org.opendaylight.controller.sal.utils.IPProtocols;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * JUnit test for {@link VTNTcpMatch}.
 */
public class VTNTcpMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNTcpMatch} class.
     */
    private static final String  XML_ROOT = "vtn-tcp-match";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNTcpMatch} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        String[] p = XmlDataType.addPath(name, parent);
        String[] portTags = {"source-port", "destination-port"};
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        for (String tag: portTags) {
            dlist.addAll(VTNPortRangeTest.getXmlDataTypes(tag, p));
        }

        return dlist;
    }

    /**
     * Test case for constructors and getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        VTNTcpMatch empty = new VTNTcpMatch();
        assertEquals(null, empty.getSourcePort());
        assertEquals(null, empty.getDestinationPort());
        assertEquals(IPProtocols.TCP.shortValue(),
                     empty.getInetProtocol(IpVersion.Ipv4));
        assertEquals(IPProtocols.TCP.shortValue(),
                     empty.getInetProtocol(IpVersion.Ipv6));
        assertEquals(TcpHeader.class, empty.getHeaderType());
        assertEquals(FlowMatchType.TCP_SRC, empty.getSourceMatchType());
        assertEquals(FlowMatchType.TCP_DST, empty.getDestinationMatchType());

        PortRangeParams[] srcs = {
            null, new PortRangeParams(0), new PortRangeParams(65535),
            new PortRangeParams(0, 65535),  new PortRangeParams(12345),
            new PortRangeParams(3, 45),
        };
        PortRangeParams[] dsts = {
            null, new PortRangeParams(0), new PortRangeParams(65535),
            new PortRangeParams(0, 65535),  new PortRangeParams(33333),
            new PortRangeParams(1000, 5000),
        };

        TcpMatchParams params = new TcpMatchParams();
        for (PortRangeParams src: srcs) {
            params.setSourcePortParams(src);
            VTNPortRange sr = (src == null)
                ? null
                : VTNPortRange.create(src.toPortMatch());
            for (PortRangeParams dst: dsts) {
                params.setDestinationPortParams(dst);
                TcpMatch tm = params.toL4Match();
                VTNTcpMatch tmatch = new VTNTcpMatch(tm);
                params.verify(tmatch);
                assertEquals(tmatch, VTNLayer4Match.create(tm));

                VTNPortRange dr = (dst == null)
                    ? null
                    : VTNPortRange.create(dst.toPortMatch());
                VTNTcpMatch tmatch1 = new VTNTcpMatch(sr, dr);
                assertEquals(tmatch, tmatch1);
            }
        }

        // port-from is missing.
        ArrayList<TcpMatch> list = new ArrayList<>();
        params.reset().setSourcePortTo(Integer.valueOf(0));
        list.add(params.toL4Match());

        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        params.reset().setDestinationPortTo(Integer.valueOf(0));
        list.add(params.toL4Match());
        for (TcpMatch tm: list) {
            try {
                new VTNTcpMatch(tm);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("port-from cannot be null", e.getMessage());
            }
        }

        // Invalid port numbers.
        Integer[] badPorts = {
            Integer.MIN_VALUE, Integer.MIN_VALUE + 1, -0x70000000,
            -0x10000000, -0x1000, -100, -10, -3, -2, -1,
            0x10000, 0x10001, 0x4000000, 0x12345000, 0x70000000,
            0x7fff0000, Integer.MAX_VALUE - 1, Integer.MAX_VALUE,
        };
        for (Integer port: badPorts) {
            params.reset().setSourcePortFrom(port);
            list.clear();
            list.add(params.toL4Match());

            params.reset().setDestinationPortFrom(port);
            list.add(params.toL4Match());

            params.reset().setSourcePortFrom(Integer.valueOf(0)).
                setSourcePortTo(port);
            list.add(params.toL4Match());

            params.reset().setDestinationPortFrom(Integer.valueOf(0)).
                setDestinationPortTo(port);
            list.add(params.toL4Match());

            for (TcpMatch tm: list) {
                try {
                    new VTNTcpMatch(tm);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals("Invalid port number: " + port,
                                 e.getMessage());
                }
            }
        }

        // Invalid port range.
        int[] ports = {1, 2, 10, 3000, 4000, 5000, 33333, 65534};
        for (int port: ports) {
            int from = port + 1;
            int to = port;
            list.clear();
            params.reset().setSourcePortFrom(Integer.valueOf(from)).
                setSourcePortTo(Integer.valueOf(to));
            list.add(params.toL4Match());

            params.reset().setDestinationPortFrom(Integer.valueOf(from)).
                setDestinationPortTo(Integer.valueOf(to));
            list.add(params.toL4Match());
            for (TcpMatch tm: list) {
                try {
                    new VTNTcpMatch(tm);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    String msg = "Invalid port range: from=" + from +
                        ", to=" + to;
                    assertEquals(msg, e.getMessage());
                }
            }

            from = port;
            to = 0;
            list.clear();
            params.reset().setSourcePortFrom(Integer.valueOf(from)).
                setSourcePortTo(Integer.valueOf(to));
            list.add(params.toL4Match());

            params.reset().setDestinationPortFrom(Integer.valueOf(from)).
                setDestinationPortTo(Integer.valueOf(to));
            list.add(params.toL4Match());
            for (TcpMatch tm: list) {
                try {
                    new VTNTcpMatch(tm);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    String msg = "Invalid port range: from=" + from +
                        ", to=" + to;
                    assertEquals(msg, e.getMessage());
                }
            }
        }

        assertEquals(null, VTNLayer4Match.create((TcpMatch)null));
        assertEquals(null, VTNLayer4Match.create((L4Match)null));
    }

    /**
     * Test case for object identify.
     *
     * <ul>
     *   <li>{@link VTNLayer4PortMatch#equals(Object)}</li>
     *   <li>{@link VTNLayer4PortMatch#hashCode()}</li>
     *   <li>{@link VTNLayer4PortMatch#setConditionKey(StringBuilder)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        HashSet<String> keySet = new HashSet<>();

        PortRangeParams[] srcs = {
            null, new PortRangeParams(0), new PortRangeParams(65535),
            new PortRangeParams(0, 65535),  new PortRangeParams(23456),
            new PortRangeParams(17, 99),
        };
        PortRangeParams[] dsts = {
            null, new PortRangeParams(0), new PortRangeParams(65535),
            new PortRangeParams(0, 65535),  new PortRangeParams(4444),
            new PortRangeParams(4444, 5000),
        };

        StringBuilder b = new StringBuilder();
        TcpMatchParams params = new TcpMatchParams();
        for (PortRangeParams src: srcs) {
            params.setSourcePortParams(src);
            for (PortRangeParams dst: dsts) {
                params.setDestinationPortParams(dst);
                VTNTcpMatch tmatch1 = params.toVTNLayer4Match();
                VTNTcpMatch tmatch2 = params.toVTNLayer4Match();
                testEquals(set, tmatch1, tmatch2);

                b.setLength(0);
                tmatch1.setConditionKey(b);
                assertEquals(true, keySet.add(b.toString()));
                b.setLength(0);
                tmatch2.setConditionKey(b);
                assertEquals(false, keySet.add(b.toString()));
            }
        }

        int count = srcs.length * dsts.length;
        assertEquals(count, set.size());
        assertEquals(count, keySet.size());
    }

    /**
     * Test case for {@link VTNLayer4PortMatch#verify()} and JAXB mapping.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        // Normal case.
        PortRangeParams[] srcs = {
            null, new PortRangeParams(0), new PortRangeParams(65535),
            new PortRangeParams(0, 65535),  new PortRangeParams(12345),
            new PortRangeParams(3, 45),
        };
        PortRangeParams[] dsts = {
            null, new PortRangeParams(0), new PortRangeParams(65535),
            new PortRangeParams(0, 65535),  new PortRangeParams(33333),
            new PortRangeParams(1000, 5000),
        };

        Class<VTNTcpMatch> type = VTNTcpMatch.class;
        Marshaller[] marshallers = {
            createMarshaller(type),
            createMarshaller(VTNLayer4Match.class),
        };
        Unmarshaller um = createUnmarshaller(type);
        Unmarshaller um4 = createUnmarshaller(VTNLayer4Match.class);
        TcpMatchParams params = new TcpMatchParams();
        for (PortRangeParams src: srcs) {
            params.setSourcePortParams(src);
            for (PortRangeParams dst: dsts) {
                params.setDestinationPortParams(dst);
                XmlNode root = new XmlNode(XML_ROOT);
                if (src != null) {
                    root.add(src.toXmlNode("source-port"));
                }
                if (dst != null) {
                    root.add(dst.toXmlNode("destination-port"));
                }

                String xml = root.toString();
                VTNTcpMatch tmatch = unmarshal(um, xml, type);
                tmatch.verify();
                params.verify(tmatch);

                VTNTcpMatch tmatch1 = unmarshal(um4, xml, type);
                tmatch1.verify();
                assertEquals(tmatch, tmatch1);

                for (Marshaller m: marshallers) {
                    xml = marshal(m, tmatch, type, XML_ROOT);
                    tmatch1 = unmarshal(um, xml, type);
                    tmatch1.verify();
                    assertEquals(tmatch, tmatch1);
                }
            }
        }

        // Invalid port numbers.
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        Integer[] badPorts = {
            Integer.MIN_VALUE, Integer.MIN_VALUE + 1, -0x70000000,
            -0x10000000, -0x1000, -100, -10, -3, -2, -1,
            0x10000, 0x10001, 0x4000000, 0x12345000, 0x70000000,
            0x7fff0000, Integer.MAX_VALUE - 1, Integer.MAX_VALUE,
        };
        String[] portTags = {"source-port", "destination-port"};
        Unmarshaller[] unmarshallers = {um, um4};
        for (Integer port: badPorts) {
            ArrayList<XmlNode> badXmls = new ArrayList<>();
            for (String tag: portTags) {
                PortRangeParams p = new PortRangeParams(port);
                badXmls.add(new XmlNode(XML_ROOT).add(p.toXmlNode(tag)));
                p.setPortFrom(1).setPortTo(port);
                badXmls.add(new XmlNode(XML_ROOT).add(p.toXmlNode(tag)));
            }

            for (XmlNode xn: badXmls) {
                String xml = xn.toString();
                for (Unmarshaller u: unmarshallers) {
                    VTNTcpMatch tmatch = unmarshal(u, xml, type);
                    try {
                        tmatch.verify();
                        unexpected();
                    } catch (RpcException e) {
                        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                        assertEquals(vtag, e.getVtnErrorTag());
                        assertEquals("Invalid port number: " + port,
                                     e.getMessage());
                    }
                }
            }
        }

        // port-from is missing.
        for (String tag: portTags) {
            XmlNode[] badXmls = {
                new XmlNode(XML_ROOT).add(new XmlNode(tag)),
                new XmlNode(XML_ROOT).add(new XmlNode(tag).
                                          add(new XmlNode("port-to", 100))),
            };
            for (XmlNode xn: badXmls) {
                String xml = xn.toString();
                for (Unmarshaller u: unmarshallers) {
                    VTNTcpMatch tmatch = unmarshal(u, xml, type);
                    try {
                        tmatch.verify();
                        unexpected();
                    } catch (RpcException e) {
                        assertEquals(RpcErrorTag.MISSING_ELEMENT,
                                     e.getErrorTag());
                        assertEquals(vtag, e.getVtnErrorTag());
                        assertEquals("port-from cannot be null",
                                     e.getMessage());
                    }
                }
            }
        }

        // Invalid port range.
        int[] ports = {1, 2, 10, 3000, 4000, 5000, 33333, 65534};
        for (int port: ports) {
            int from = port + 1;
            int to = port;
            PortRangeParams p = new PortRangeParams(from, to);
            ArrayList<XmlNode> badXmls = new ArrayList<>();
            for (String tag: portTags) {
                badXmls.add(new XmlNode(XML_ROOT).add(p.toXmlNode(tag)));
            }
            for (XmlNode xn: badXmls) {
                String xml = xn.toString();
                for (Unmarshaller u: unmarshallers) {
                    VTNTcpMatch tmatch = unmarshal(u, xml, type);
                    try {
                        tmatch.verify();
                        unexpected();
                    } catch (RpcException e) {
                        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                        assertEquals(vtag, e.getVtnErrorTag());
                        String msg = "Invalid port range: from=" + from +
                            ", to=" + to;
                        assertEquals(msg, e.getMessage());
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Unmarshaller u: unmarshallers) {
            jaxbErrorTest(u, type, dlist);
        }
    }

    /**
     * Test case for {@link VTNLayer4PortMatch#match(FlowMatchContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        int srcPort = 45678;
        int dstPort = 12345;
        TcpMatchParams header = new TcpMatchParams();
        header.setSourcePort(srcPort);
        header.setDestinationPort(dstPort);
        TestMatchContext ctx = new TestMatchContext().setLayer4Header(header);

        // Empty match should match every TCP packet.
        VTNTcpMatch tmatch = new VTNTcpMatch();
        assertEquals(true, tmatch.match(ctx));
        ctx.checkMatchFields();

        PortRangeParams[] srcs = {
            new PortRangeParams(srcPort),
            new PortRangeParams(srcPort - 100, srcPort),
            new PortRangeParams(srcPort, srcPort + 100),
            new PortRangeParams(srcPort - 100, srcPort),
            new PortRangeParams(srcPort - 100, srcPort + 100),
        };
        PortRangeParams[] dsts = {
            new PortRangeParams(dstPort),
            new PortRangeParams(dstPort - 200, dstPort),
            new PortRangeParams(dstPort, dstPort + 200),
            new PortRangeParams(dstPort - 200, dstPort),
            new PortRangeParams(dstPort - 200, dstPort + 200),
        };

        for (PortRangeParams src: srcs) {
            for (PortRangeParams dst: dsts) {
                matchTest(ctx, src, dst);
            }
        }
    }

    /**
     * Run tests for {@link VTNLayer4PortMatch#match(FlowMatchContext)}.
     *
     * @param ctx  A {@link TestMatchContext} instance that contains the
     *             TCP header.
     * @param src  A {@link PortRangeParams} instance that represents the
     *             condition for the source port.
     * @param dst  A {@link PortRangeParams} instance that represents the
     *             condition for the destination port.
     * @throws Exception  An error occurred.
     */
    private void matchTest(TestMatchContext ctx, PortRangeParams src,
                           PortRangeParams dst) throws Exception {
        Layer4Header l4 = ctx.getLayer4Header();
        assertTrue(l4 instanceof TcpHeader);
        TcpHeader l4head = (TcpHeader)l4;
        int srcPort = l4head.getSourcePort();
        int dstPort = l4head.getDestinationPort();

        // Specify single field in TCP header.
        ctx.reset();
        TcpMatchParams params = new TcpMatchParams();
        params.setSourcePortParams(src);
        VTNTcpMatch tmatch = params.toVTNLayer4Match();
        assertEquals(true, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC);

        ctx.reset();
        params.reset().setDestinationPortParams(dst);
        tmatch = params.toVTNLayer4Match();
        assertEquals(true, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_DST);

        // Specify all fields.
        ctx.reset();
        params.setSourcePortParams(src);
        tmatch = params.toVTNLayer4Match();
        assertEquals(true, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC, FlowMatchType.TCP_DST);

        // TCP match should not match to a non-TCP packet.
        Layer4Header[] anotherL4 = {
            null,
            new UdpMatchParams().setSourcePortFrom(l4head.getSourcePort()).
                setDestinationPortFrom(l4head.getDestinationPort()),
            new IcmpMatchParams((short)(srcPort & 0xff),
                                (short)(dstPort & 0xff)),
        };
        for (Layer4Header h: anotherL4) {
            ctx.reset();
            ctx.setLayer4Header(h);
            assertEquals(false, tmatch.match(ctx));
            ctx.checkMatchFields();
        }

        ctx.setLayer4Header(l4head);
        assertEquals(true, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC, FlowMatchType.TCP_DST);

        // Ensure match() returns false if one field does not match.
        int port = srcPort - 1;
        ctx.reset();
        params.setSourcePortFrom(port).setSourcePortTo(null);
        tmatch = params.toVTNLayer4Match();
        assertEquals(false, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC);

        ctx.reset();
        params.setSourcePortFrom(0).setSourcePortTo(port);
        tmatch = params.toVTNLayer4Match();
        assertEquals(false, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC);

        port = srcPort + 1;
        ctx.reset();
        params.setSourcePortFrom(port).setSourcePortTo(null);
        tmatch = params.toVTNLayer4Match();
        assertEquals(false, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC);

        ctx.reset();
        params.setSourcePortTo(65535);
        tmatch = params.toVTNLayer4Match();
        assertEquals(false, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC);

        port = dstPort - 1;
        ctx.reset();
        params.setSourcePortParams(src).setDestinationPortFrom(port).
            setDestinationPortTo(null);
        tmatch = params.toVTNLayer4Match();
        assertEquals(false, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC, FlowMatchType.TCP_DST);

        ctx.reset();
        params.setDestinationPortFrom(0).setDestinationPortTo(port);
        tmatch = params.toVTNLayer4Match();
        assertEquals(false, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC, FlowMatchType.TCP_DST);

        port = dstPort + 1;
        ctx.reset();
        params.setDestinationPortFrom(port).setDestinationPortTo(null);
        tmatch = params.toVTNLayer4Match();
        assertEquals(false, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC, FlowMatchType.TCP_DST);

        ctx.reset();
        params.setDestinationPortTo(65535);
        tmatch = params.toVTNLayer4Match();
        assertEquals(false, tmatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.TCP_SRC, FlowMatchType.TCP_DST);
    }
}
