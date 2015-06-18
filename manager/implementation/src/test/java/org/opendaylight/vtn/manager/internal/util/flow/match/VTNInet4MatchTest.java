/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatch;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.layer._3.match.ArpMatchBuilder;

/**
 * JUnit test for {@link VTNInet4Match}.
 */
public class VTNInet4MatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNInet4Match} class.
     */
    private static final String  XML_ROOT = "vtn-inet4-match";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNInet4Match} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlValueType("source-network-v4",
                             Ip4Network.class).add(name).prepend(parent),
            new XmlValueType("destination-network-v4",
                             Ip4Network.class).add(name).prepend(parent),
            new XmlValueType("protocol", Short.class).add(name).prepend(parent),
            new XmlValueType("dscp", Short.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for the following constructors.
     *
     * <ul>
     *   <li>{@link VTNInet4Match#VTNInet4Match()}</li>
     *   <li>{@link VTNInet4Match#VTNInet4Match(Short)}</li>
     *   <li>{@link VTNInet4Match#setProtocol(Short)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor1() throws Exception {
        VTNInet4Match imatch = new VTNInet4Match();
        assertEquals(null, imatch.getSourceNetwork());
        assertEquals(null, imatch.getDestinationNetwork());
        assertEquals(null, imatch.getProtocol());
        assertEquals(null, imatch.getDscp());

        Short[] protocols = {
            0, 1, 30, 63, 64, 127, 128, 254, 255,
        };
        for (Short proto: protocols) {
            imatch = new VTNInet4Match(proto);
            assertEquals(null, imatch.getSourceNetwork());
            assertEquals(null, imatch.getDestinationNetwork());
            assertEquals(proto, imatch.getProtocol());
            assertEquals(null, imatch.getDscp());

            imatch = new VTNInet4Match();
            assertEquals(null, imatch.getProtocol());
            imatch.setProtocol(proto);
            assertEquals(proto, imatch.getProtocol());
            imatch.setProtocol(proto);
            assertEquals(proto, imatch.getProtocol());
            for (int i = 1; i <= 10; i++) {
                short p = (short)((proto.shortValue() + i) & 0xff);
                try {
                    imatch.setProtocol(p);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    String msg = "IP protocol conflict: proto=" + proto +
                        ", expected=" + p;
                    assertEquals(msg, st.getDescription());
                }
            }
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNInet4Match#VTNInet4Match(IpNetwork,IpNetwork,Short,Short)}</li>
     *   <li>{@link VTNInet4Match#VTNInet4Match(Inet4Match)}</li>
     *   <li>{@link VTNInet4Match#setMatch(MatchBuilder)}</li>
     *   <li>{@link VTNInetMatch#create(org.opendaylight.vtn.manager.flow.cond.InetMatch)}</li>
     *   <li>{@link VTNInetMatch#create(Match)}</li>
     *   <li>Getter methods.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor2() throws Exception {
        IpNetwork[] srcs = {
            null, new Ip4Network("10.1.2.3"),
            new Ip4Network("192.168.234.193"),
        };
        Short[] srcPfx = {null, 1, 8, 32};
        IpNetwork[] dsts = {
            null, new Ip4Network("10.20.30.40"),
            new Ip4Network("172.30.193.35"),
        };
        Short[] dstPfx = {null, 1, 16, 32};
        Short[] protocols = {null, 0, 6, 255};
        Short[] dscps = {null, 0, 30, 63};

        Inet4MatchParams params = new Inet4MatchParams();
        for (IpNetwork src: srcs) {
            params.setSourceAddress(src);
            for (Short spfx: srcPfx) {
                params.setSourcePrefix(spfx);
                IpNetwork srcNw;
                if (src == null) {
                    srcNw = null;
                } else if (spfx == null) {
                    srcNw = src;
                } else {
                    srcNw = new Ip4Network(src.getInetAddress(),
                                           spfx.intValue());
                }

                for (IpNetwork dst: dsts) {
                    params.setDestinationAddress(dst);
                    for (Short dpfx: dstPfx) {
                        params.setDestinationPrefix(dpfx);
                        IpNetwork dstNw;
                        if (dst == null) {
                            dstNw = null;
                        } else if (dpfx == null) {
                            dstNw = dst;
                        } else {
                            dstNw = new Ip4Network(dst.getInetAddress(),
                                                   dpfx.intValue());
                        }

                        for (Short proto: protocols) {
                            params.setProtocol(proto);
                            for (Short dscp: dscps) {
                                params.setDscp(dscp);
                                Inet4Match im = params.toInet4Match();
                                VTNInet4Match imatch = new VTNInet4Match(im);
                                params.verify(imatch);
                                assertEquals(imatch, VTNInetMatch.create(im));

                                VTNInet4Match imatch1 = new VTNInet4Match(
                                    srcNw, dstNw, proto, dscp);
                                assertEquals(imatch, imatch1);
                            }
                        }
                    }
                }
            }
        }

        // Create broken Inet4Match.
        String v6addr = "::1";
        String[] badAddrs = {
            "", "192.168.100.256", v6addr, "bad_address",
        };
        Unmarshaller um = createUnmarshaller(Inet4Match.class);
        for (String addr: badAddrs) {
            String xml = new XmlNode("inet4match").
                setAttribute("src", addr).toString();
            Inet4Match im = unmarshal(um, xml, Inet4Match.class);
            assertNotNull(im.getValidationStatus());
            try {
                new VTNInet4Match(im);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                String desc = st.getDescription();
                if (addr.isEmpty()) {
                    assertEquals("Empty source address.", desc);
                } else if (addr.equals(v6addr)) {
                    String msg = "Unexpected source address type: addr=" +
                        addr + ", expected=Inet4Address";
                    assertEquals(msg, desc);
                } else {
                    String msg = "Invalid source IP address: " + addr + ": ";
                    assertTrue("Unexpected error message: " + desc,
                               desc.startsWith(msg));
                }
            }
        }

        // Invalid prefix length.
        Short[] badPrefix = {
            Short.MIN_VALUE, -30000, -10000, -0x100, -3, -2, -1, 0,
            33, 34, 1000, 0x1f00, Short.MAX_VALUE - 1, Short.MAX_VALUE,
        };
        IpNetwork ipaddr = new Ip4Network("192.168.10.20");
        for (Short prefix: badPrefix) {
            params.reset().setSourceAddress(ipaddr);
            params.setSourcePrefix(prefix);
            Inet4Match im = params.toInet4Match();
            assertEquals(null, im.getValidationStatus());
            try {
                new VTNInet4Match(im);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                String msg = st.getDescription();
                assertTrue(msg.startsWith("Invalid source IP address: "));
            }

            params.reset().setDestinationAddress(ipaddr);
            params.setDestinationPrefix(prefix);
            im = params.toInet4Match();
            assertEquals(null, im.getValidationStatus());
            try {
                new VTNInet4Match(im);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                String msg = st.getDescription();
                assertTrue(msg.startsWith("Invalid destination IP address: "));
            }
        }

        // Invalid protocol numbers.
        Short[] badProtocols = {
            Short.MIN_VALUE, -30000, -10000, -0x100, -3, -2, -1,
            256, 257, 999, 0x1f00, Short.MAX_VALUE - 1, Short.MAX_VALUE,
        };
        for (Short proto: badProtocols) {
            params.reset().setProtocol(proto);
            Inet4Match im = params.toInet4Match();
            assertEquals(null, im.getValidationStatus());
            try {
                new VTNInet4Match(im);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid IP protocol number: " + proto,
                             st.getDescription());
            }

            try {
                new VTNInet4Match(null, null, proto, null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid IP protocol number: " + proto,
                             st.getDescription());
            }
        }

        // Invalid DSCP values.
        params.setProtocol((short)6);
        Short[] badDscps = {
            Byte.MIN_VALUE, -127, -100, -50, -16, -3, -2, -1,
            64, 65, 70, 80, 99, 127, Byte.MAX_VALUE,
        };
        for (Short dscp: badDscps) {
            params.setDscp(dscp);
            Inet4Match im = params.toInet4Match();
            assertEquals(null, im.getValidationStatus());
            try {
                new VTNInet4Match(im);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid IP DSCP field value: " + dscp,
                             st.getDescription());
            }

            try {
                new VTNInet4Match(null, null, null, dscp);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid IP DSCP field value: " + dscp,
                             st.getDescription());
            }
        }

        MatchBuilder mb = new MatchBuilder();
        Match match = mb.build();
        assertEquals(null, VTNInetMatch.create(match));
        assertEquals(null, VTNInetMatch.create((InetMatch)null));

        // Unsupported L3 match type.
        mb.setLayer3Match(new ArpMatchBuilder().build());
        match = mb.build();
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        StatusCode code = StatusCode.BADREQUEST;
        String msg = "Unsupported layer 3 match: " + mb.getLayer3Match();
        try {
            VTNInetMatch.create(match);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(code, st.getCode());
            assertEquals(msg, st.getDescription());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNInet4Match#VTNInet4Match(org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnInetMatchFields)}</li>
     *   <li>{@link VTNInet4Match#setMatch(MatchBuilder)}</li>
     *   <li>{@link VTNInetMatch#create(Match)}</li>
     *   <li>{@link VTNInetMatch#create(VTNEtherMatch, VtnInetMatch)}</li>
     *   <li>Getter methods.</li>
     *   <li>JAXB bindings.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor3() throws Exception {
        IpNetwork[] srcs = {
            null, new Ip4Network("10.1.2.3"),
        };
        Short[] srcPfx = {null, 8, 24};
        IpNetwork[] dsts = {
            null, new Ip4Network("10.20.30.40"),
        };
        Short[] dstPfx = {null, 17, 23};
        Short[] protocols = {null, 33};
        Short[] dscps = {null, 35};

        Inet4MatchParams params = new Inet4MatchParams();
        Class<VTNInet4Match> mtype = VTNInet4Match.class;
        VTNEtherMatch ematch = null;
        for (IpNetwork src: srcs) {
            params.setSourceAddress(src);
            for (Short spfx: srcPfx) {
                params.setSourcePrefix(spfx);
                for (IpNetwork dst: dsts) {
                    params.setDestinationAddress(dst);
                    for (Short dpfx: dstPfx) {
                        params.setDestinationPrefix(dpfx);
                        for (Short proto: protocols) {
                            params.setProtocol(proto);
                            for (Short dscp: dscps) {
                                params.setDscp(dscp);
                                VtnInetMatch vim = params.toVtnInetMatch();
                                VTNInet4Match imatch = new VTNInet4Match(vim);
                                params.verify(imatch);
                                assertEquals(imatch,
                                             VTNInetMatch.create(ematch, vim));

                                // JAXB test.
                                VTNInet4Match jaxb =
                                    jaxbTest(imatch, mtype, XML_ROOT);
                                jaxb.verify();
                            }
                        }
                    }
                }
            }
        }

        assertEquals(null, VTNInetMatch.create(ematch, null));
    }

    /**
     * Test case for {@link VTNInetMatch#verify()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVerify() throws Exception {
        Unmarshaller um = createUnmarshaller(VTNInet4Match.class);

        // Invalid protocol numbers.
        Short[] badProtocols = {
            Short.MIN_VALUE, -30000, -10000, -0x100, -3, -2, -1,
            256, 257, 999, 0x1f00, Short.MAX_VALUE - 1, Short.MAX_VALUE,
        };
        for (Short proto: badProtocols) {
            String xml = new XmlNode(XML_ROOT).
                add(new XmlNode("protocol", proto)).toString();
            VTNInet4Match imatch = unmarshal(um, xml, VTNInet4Match.class);
            try {
                imatch.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid IP protocol number: " + proto,
                             st.getDescription());
            }
        }

        // Invalid DSCP values.
        Short[] badDscps = {
            Short.MIN_VALUE, -30000, -20000, -256, -100, -3, -2, -1,
            64, 65, 100, 256, 10000, 25000, Short.MAX_VALUE,
        };
        for (Short dscp: badDscps) {
            String xml = new XmlNode(XML_ROOT).
                add(new XmlNode("dscp", dscp)).toString();
            VTNInet4Match imatch = unmarshal(um, xml, VTNInet4Match.class);
            try {
                imatch.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid IP DSCP field value: " + dscp,
                             st.getDescription());
            }
        }
    }

    /**
     * Test case for object identity.
     *
     * <ul>
     *   <li>{@link VTNInetMatch#equals(Object)}</li>
     *   <li>{@link VTNInetMatch#hashCode()}</li>
     *   <li>{@link VTNInetMatch#setConditionKey(StringBuilder)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        HashSet<String> keySet = new HashSet<>();

        IpNetwork[] srcs = {
            null, new Ip4Network("192.168.199.0/24"),
            new Ip4Network("172.168.33.44"),
        };
        IpNetwork[] dsts = {
            null, new Ip4Network("11.22.33.44"),
            new Ip4Network("100.200.30.192/26"),
        };
        Short[] protocols = {null, 6, 255};
        Short[] dscps = {null, 44, 63};

        StringBuilder b = new StringBuilder();
        Inet4MatchParams params = new Inet4MatchParams();
        for (IpNetwork src: srcs) {
            params.setSourceNetwork(src);
            for (IpNetwork dst: dsts) {
                params.setDestinationNetwork(dst);
                for (Short proto: protocols) {
                    params.setProtocol(proto);
                    for (Short dscp: dscps) {
                        params.setDscp(dscp);
                        VTNInet4Match imatch1 = params.toVTNInet4Match();
                        VTNInet4Match imatch2 = params.toVTNInet4Match();
                        testEquals(set, imatch1, imatch2);

                        b.setLength(0);
                        imatch1.setConditionKey(b);
                        assertEquals(true, keySet.add(b.toString()));
                        b.setLength(0);
                        imatch2.setConditionKey(b);
                        assertEquals(false, keySet.add(b.toString()));
                    }
                }
            }
        }

        int count = srcs.length * dsts.length * protocols.length *
            dscps.length;
        assertEquals(count, set.size());
        assertEquals(count, keySet.size());
    }

    /**
     * Ensure that {@link VTNInet4Match} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        // VTNInet4Match can be unmashalled by VTNInetMatch unmarshaller.
        Unmarshaller um = createUnmarshaller(VTNInetMatch.class);
        Unmarshaller um4 = createUnmarshaller(VTNInet4Match.class);
        Inet4MatchParams params = new Inet4MatchParams();

        // Empty match.
        String xml = new XmlNode(XML_ROOT).toString();
        VTNInet4Match imatch = unmarshal(um, xml, VTNInet4Match.class);
        assertEquals(imatch, unmarshal(um4, xml, VTNInet4Match.class));
        imatch.verify();
        assertEquals(params.toVTNInet4Match(), imatch);

        // Specifying all fields.
        IpNetwork src = new Ip4Network("192.168.10.0/24");
        IpNetwork dst = new Ip4Network("1.2.3.4");
        Short proto = Short.valueOf((short)17);
        Short dscp = Short.valueOf((short)3);
        params.setSourceNetwork(src).setDestinationNetwork(dst).
            setProtocol(proto).setDscp(dscp);

        final String tagSrc = "source-network-v4";
        final String tagDst = "destination-network-v4";
        final String tagProto = "protocol";
        final String tagDscp = "dscp";
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode(tagSrc, src.getText())).
            add(new XmlNode(tagDst, dst.getText())).
            add(new XmlNode(tagProto, proto)).
            add(new XmlNode(tagDscp, dscp)).toString();
        imatch = unmarshal(um, xml, VTNInet4Match.class);
        assertEquals(imatch, unmarshal(um4, xml, VTNInet4Match.class));
        imatch.verify();
        assertEquals(params.toVTNInet4Match(), imatch);

        // Specifying single field.
        params.reset().setSourceNetwork(src);
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode(tagSrc, src.getText())).toString();
        imatch = unmarshal(um, xml, VTNInet4Match.class);
        assertEquals(imatch, unmarshal(um4, xml, VTNInet4Match.class));
        imatch.verify();
        assertEquals(params.toVTNInet4Match(), imatch);

        params.reset().setDestinationNetwork(dst);
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode(tagDst, dst.getText())).toString();
        imatch = unmarshal(um, xml, VTNInet4Match.class);
        assertEquals(imatch, unmarshal(um4, xml, VTNInet4Match.class));
        imatch.verify();
        assertEquals(params.toVTNInet4Match(), imatch);

        params.reset().setProtocol(proto);
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode(tagProto, proto)).toString();
        imatch = unmarshal(um, xml, VTNInet4Match.class);
        assertEquals(imatch, unmarshal(um4, xml, VTNInet4Match.class));
        imatch.verify();
        assertEquals(params.toVTNInet4Match(), imatch);

        params.reset().setDscp(dscp);
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode(tagDscp, dscp)).toString();
        imatch = unmarshal(um, xml, VTNInet4Match.class);
        assertEquals(imatch, unmarshal(um4, xml, VTNInet4Match.class));
        imatch.verify();
        assertEquals(params.toVTNInet4Match(), imatch);

        // Ensure that broken values in XML can be detected.
        List<XmlDataType> dtypes = getXmlDataTypes(XML_ROOT);
        jaxbErrorTest(um, VTNInet4Match.class, dtypes);
        jaxbErrorTest(um4, VTNInet4Match.class, dtypes);
    }

    /**
     * Test case for {@link VTNInetMatch#match(FlowMatchContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        for (short slen = 1; slen <= 32; slen++) {
            for (short dlen = 1; dlen <= 32; dlen++) {
                matchTest(slen, dlen);
            }
        }
    }

    /**
     * Run tests for {@link VTNInetMatch#match(FlowMatchContext)}.
     *
     * @param srcLen  The length of the source network prefix.
     * @param dstLen  The length of the destination network prefix.
     * @throws Exception  An error occurred.
     */
    private void matchTest(short srcLen, short dstLen) throws Exception {
        Ip4Network src = new Ip4Network("192.168.10.20");
        Ip4Network dst = new Ip4Network("172.168.250.254");
        short proto = 6;
        short dscp = 39;
        Inet4MatchParams header = new Inet4MatchParams();
        header.setSourceAddress(src);
        header.setDestinationAddress(dst);
        header.setProtocol(proto);
        header.setDscp(dscp);
        TestMatchContext ctx = new TestMatchContext().setInetHeader(header);

        // Empty match should match every packet.
        VTNInet4Match imatch = new VTNInet4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields();

        // Specify single field in IP header.
        ctx.reset();
        Inet4MatchParams params = new Inet4MatchParams();
        params.setSourceAddress(src);
        params.setSourcePrefix(srcLen);
        imatch = params.toVTNInet4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_SRC);

        ctx.reset();
        params.reset().setDestinationAddress(dst);
        params.setDestinationPrefix(dstLen);
        imatch = params.toVTNInet4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_DST);

        ctx.reset();
        params.reset().setProtocol(proto);
        imatch = params.toVTNInet4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_PROTO);

        ctx.reset();
        params.reset().setDscp(dscp);
        imatch = params.toVTNInet4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_DSCP);

        // Specify all fields.
        ctx.reset();
        params.setSourceAddress(src);
        params.setSourcePrefix(srcLen);
        params.setDestinationAddress(dst);
        params.setDestinationPrefix(dstLen);
        params.setProtocol(proto);
        params.setDscp(dscp);
        imatch = params.toVTNInet4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_SRC, FlowMatchType.IP_DST,
                             FlowMatchType.IP_PROTO, FlowMatchType.IP_DSCP);

        // Inet4 match should not match to a non-IP packet.
        ctx.reset();
        ctx.setInetHeader(null);
        assertEquals(false, imatch.match(ctx));
        ctx.checkMatchFields();
        ctx.setInetHeader(header);
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_SRC, FlowMatchType.IP_DST,
                             FlowMatchType.IP_PROTO, FlowMatchType.IP_DSCP);

        // Ensure match() returns false if one field does not match.
        int addr = src.getAddress() ^ (int)0x80000000;
        Ip4Network anotherSrc = new Ip4Network(addr);
        ctx.reset();
        params.setSourceAddress(anotherSrc);
        imatch = params.toVTNInet4Match();
        assertEquals(false, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_SRC);

        addr = dst.getAddress() ^ (int)0x80000000;
        Ip4Network anotherDst = new Ip4Network(addr);
        ctx.reset();
        params.setSourceAddress(src);
        params.setDestinationAddress(anotherDst);
        imatch = params.toVTNInet4Match();
        assertEquals(false, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_SRC, FlowMatchType.IP_DST);

        ctx.reset();
        params.setDestinationAddress(dst);
        params.setProtocol((short)17);
        imatch = params.toVTNInet4Match();
        assertEquals(false, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_SRC, FlowMatchType.IP_DST,
                             FlowMatchType.IP_PROTO);

        ctx.reset();
        params.setProtocol(proto);
        params.setDscp((short)0);
        imatch = params.toVTNInet4Match();
        assertEquals(false, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.IP_SRC, FlowMatchType.IP_DST,
                             FlowMatchType.IP_PROTO, FlowMatchType.IP_DSCP);
    }
}
