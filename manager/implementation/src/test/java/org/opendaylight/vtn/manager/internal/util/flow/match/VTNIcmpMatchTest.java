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

import org.opendaylight.vtn.manager.flow.cond.IcmpMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;

import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link VTNIcmpMatch}.
 */
public class VTNIcmpMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNIcmpMatch} class.
     */
    private static final String  XML_ROOT = "vtn-icmp-match";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNIcmpMatch} instance.
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
            new XmlValueType("type", Short.class).add(name).prepend(parent),
            new XmlValueType("code", Short.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        VTNIcmpMatch empty = new VTNIcmpMatch();
        assertEquals(null, empty.getIcmpType());
        assertEquals(null, empty.getIcmpCode());
        IcmpMatchParams.checkInetProtocol(empty);

        Short[] types = {
            null, 0, 1, 50, 100, 200, 254, 255,
        };
        Short[] codes = {
            null, 0, 1, 33, 67, 128, 254, 255,
        };

        IcmpMatchParams params = new IcmpMatchParams();
        for (Short type: types) {
            params.setType(type);
            for (Short code: codes) {
                params.setCode(code);
                IcmpMatch im = params.toL4Match();
                VTNIcmpMatch imatch = new VTNIcmpMatch(im);
                params.verify(imatch);
                assertEquals(imatch, VTNLayer4Match.create(im));

                VTNIcmpMatch imatch1 = new VTNIcmpMatch(type, code);
                assertEquals(imatch, imatch1);
            }
        }

        // Invalid type/code.
        Short[] invalidValues = {
            Short.MIN_VALUE, Short.MIN_VALUE + 1, -0x7000, -0x5000, -0xf00,
            -100, -55, -33, -22, -3, -2, -1,
            256, 257, 500, 1000, 3333, 6666, 12345, 23456, 32000,
            Short.MAX_VALUE - 1, Short.MAX_VALUE,
        };
        for (Short value: invalidValues) {
            params.reset().setType(value);
            IcmpMatch im = params.toL4Match();
            try {
                new VTNIcmpMatch(im);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid ICMP type: " + value,
                             st.getDescription());
            }

            try {
                new VTNIcmpMatch(value, null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid ICMP type: " + value,
                             st.getDescription());
            }

            params.reset().setCode(value);
            im = params.toL4Match();
            try {
                new VTNIcmpMatch(im);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid ICMP code: " + value,
                             st.getDescription());
            }

            try {
                new VTNIcmpMatch(null, value);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid ICMP code: " + value,
                             st.getDescription());
            }
        }

        assertEquals(null, VTNLayer4Match.create((IcmpMatch)null));
        assertEquals(null, VTNLayer4Match.create((L4Match)null));
    }

    /**
     * Test case for object identity.
     *
     * <ul>
     *   <li>{@link VTNIcmpMatch#equals(Object)}</li>
     *   <li>{@link VTNIcmpMatch#hashCode()}</li>
     *   <li>{@link VTNIcmpMatch#setConditionKey(StringBuilder)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        HashSet<String> keySet = new HashSet<>();

        Short[] types = {
            null, 0, 1, 2, 50, 78, 100, 127, 200, 254, 255,
        };
        Short[] codes = {
            null, 0, 1, 14, 33, 67, 103, 128, 192, 254, 255,
        };

        StringBuilder b = new StringBuilder();
        IcmpMatchParams params = new IcmpMatchParams();
        for (Short type: types) {
            params.setType(type);
            for (Short code: codes) {
                params.setCode(code);
                VTNIcmpMatch imatch1 = params.toVTNLayer4Match();
                VTNIcmpMatch imatch2 = params.toVTNLayer4Match();
                testEquals(set, imatch1, imatch2);

                b.setLength(0);
                imatch1.setConditionKey(b);
                assertEquals(true, keySet.add(b.toString()));
                b.setLength(0);
                imatch2.setConditionKey(b);
                assertEquals(false, keySet.add(b.toString()));
            }
        }

        int count = types.length * codes.length;
        assertEquals(count, set.size());
        assertEquals(count, keySet.size());
    }

    /**
     * Test case for {@link VTNIcmpMatch#verify()} and JAXB mapping.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        // Normal case.
        Short[] types = {
            null, 0, 1, 50, 78, 127, 255,
        };
        Short[] codes = {
            null, 0, 1, 33, 67, 128, 255,
        };

        Class<VTNIcmpMatch> xmlType = VTNIcmpMatch.class;
        Unmarshaller um = createUnmarshaller(xmlType);
        IcmpMatchParams params = new IcmpMatchParams();
        for (Short type: types) {
            params.setType(type);
            for (Short code: codes) {
                params.setCode(code);
                XmlNode root = new XmlNode(XML_ROOT);
                if (type != null) {
                    root.add(new XmlNode("type", type));
                }
                if (code != null) {
                    root.add(new XmlNode("code", code));
                }

                String xml = root.toString();
                VTNIcmpMatch imatch = unmarshal(um, xml, xmlType);
                imatch.verify();
                params.verify(imatch);
            }
        }

        // Invalid type/code.
        Short[] invalidValues = {
            Short.MIN_VALUE, Short.MIN_VALUE + 1, -0x7000, -0x5000, -0xf00,
            -100, -55, -33, -22, -3, -2, -1,
            256, 257, 500, 1000, 3333, 6666, 12345, 23456, 32000,
            Short.MAX_VALUE - 1, Short.MAX_VALUE,
        };
        for (Short value: invalidValues) {
            String xml = new XmlNode(XML_ROOT).
                add(new XmlNode("type", value)).toString();
            VTNIcmpMatch imatch = unmarshal(um, xml, xmlType);
            try {
                imatch.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid ICMP type: " + value,
                             st.getDescription());
            }

            xml = new XmlNode(XML_ROOT).add(new XmlNode("code", value)).
                toString();
            imatch = unmarshal(um, xml, xmlType);
            try {
                imatch.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Invalid ICMP code: " + value,
                             st.getDescription());
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, xmlType, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Test case for {@link VTNIcmpMatch#match(FlowMatchContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        short icmpType = 6;
        short icmpCode = 234;
        IcmpMatchParams header = new IcmpMatchParams(icmpType, icmpCode);
        TestMatchContext ctx = new TestMatchContext().setLayer4Header(header);

        // Specify single field in ICMP header.
        IcmpMatchParams params = new IcmpMatchParams().setType(icmpType);
        VTNIcmpMatch imatch = params.toVTNLayer4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.ICMP_TYPE);

        ctx.reset();
        params.reset().setCode(icmpCode);
        imatch = params.toVTNLayer4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.ICMP_CODE);

        // Specify all fields.
        ctx.reset();
        params.setType(icmpType);
        imatch = params.toVTNLayer4Match();
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.ICMP_TYPE, FlowMatchType.ICMP_CODE);

        // ICMP match should not match to a non-ICMP packet.
        Layer4Header[] anotherL4 = {
            null,
            new TcpMatchParams().setSourcePortFrom((int)icmpType).
                setDestinationPortFrom((int)icmpCode),
            new UdpMatchParams().setSourcePortFrom((int)icmpType).
                setDestinationPortFrom((int)icmpCode),
        };
        for (Layer4Header h: anotherL4) {
            ctx.reset();
            ctx.setLayer4Header(h);
            assertEquals(false, imatch.match(ctx));
            ctx.checkMatchFields();
        }

        ctx.setLayer4Header(header);
        assertEquals(true, imatch.match(ctx));
        ctx.checkMatchFields(FlowMatchType.ICMP_TYPE, FlowMatchType.ICMP_CODE);

        // Ensure match() returns if one field does not match.
        for (short value = 0; value < 256; value++) {
            if (value != icmpType) {
                ctx.reset();
                params.setType(value);
                imatch = params.toVTNLayer4Match();
                assertEquals(false, imatch.match(ctx));
                ctx.checkMatchFields(FlowMatchType.ICMP_TYPE);
                params.setType(icmpType);
            }

            if (value != icmpCode) {
                ctx.reset();
                params.setCode(value);
                imatch = params.toVTNLayer4Match();
                assertEquals(false, imatch.match(ctx));
                ctx.checkMatchFields(FlowMatchType.ICMP_TYPE,
                                     FlowMatchType.ICMP_CODE);
                params.setCode(icmpCode);
            }
        }
    }
}
