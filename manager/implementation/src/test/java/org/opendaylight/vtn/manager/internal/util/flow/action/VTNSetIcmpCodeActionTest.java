/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;

import org.opendaylight.vtn.manager.internal.util.packet.IcmpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.TcpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.UdpHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * JUnit test for {@link VTNSetIcmpCodeAction}.
 */
public class VTNSetIcmpCodeActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetIcmpCodeAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-icmp-code-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetIcmpCodeAction} instance.
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
            new XmlValueType("order", Integer.class).add(name).prepend(parent),
            new XmlValueType("code", short.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNSetIcmpCodeAction#VTNSetIcmpCodeAction(short)}</li>
     *   <li>{@link VTNSetIcmpCodeAction#VTNSetIcmpCodeAction(SetIcmpCodeAction, int)}</li>
     *   <li>{@link VTNSetIcmpCodeAction#VTNSetIcmpCodeAction(VtnSetIcmpCodeAction, Integer)}</li>
     *   <li>{@link VTNSetIcmpCodeAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetIcmpCodeAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetIcmpCodeAction#getCode()}</li>
     *   <li>{@link VTNSetIcmpCodeAction#verifyImpl()}</li>
     *   <li>{@link VTNSetIcmpCodeAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNSetIcmpCodeAction#toVtnAction(Action)}</li>
     *   <li>{@link FlowFilterAction#verify()}</li>
     *   <li>{@link FlowFilterAction#getIdentifier()}</li>
     *   <li>{@link VTNFlowAction#toVtnFlowActionBuilder(Integer)}</li>
     *   <li>{@link VTNFlowAction#toActionBuilder(Integer)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetSet() throws Exception {
        short[] codes = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (Integer order: orders) {
            for (short code: codes) {
                PortNumber pnum = new PortNumber((int)code);
                SetIcmpCodeAction vad = new SetIcmpCodeAction(code);
                VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
                    setCode(code).build();
                SetTpDstAction ma = new SetTpDstActionBuilder().
                    setPort(pnum).build();
                SetTpDstActionCase mact = new SetTpDstActionCaseBuilder().
                    setSetTpDstAction(ma).build();

                VTNSetIcmpCodeAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetIcmpCodeAction(code);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetIcmpCodeAction(vad, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals(code, va.getCode());

                    va = new VTNSetIcmpCodeAction(vact, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(code, va.getCode());

                VtnFlowActionBuilder vbuilder =
                    va.toVtnFlowActionBuilder(anotherOrder);
                assertEquals(anotherOrder, vbuilder.getOrder());
                assertEquals(vact, vbuilder.getVtnAction());
                assertEquals(order, va.getIdentifier());

                ActionBuilder mbuilder = va.toActionBuilder(anotherOrder);
                assertEquals(anotherOrder, mbuilder.getOrder());
                assertEquals(mact, mbuilder.getAction());
                assertEquals(order, va.getIdentifier());
            }

            if (order != null) {
                // Default code test.
                VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
                    build();
                VTNSetIcmpCodeAction va = new VTNSetIcmpCodeAction(vact, order);
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getCode());
            }
        }

        VTNSetIcmpCodeAction va = new VTNSetIcmpCodeAction();
        for (short code: codes) {
            // toFlowAction() test.
            SetIcmpCodeAction vad = new SetIcmpCodeAction(code);
            VtnAction vaction = new VtnSetIcmpCodeActionBuilder().
                setCode(code).build();
            assertEquals(vad, va.toFlowAction(vaction));

            vaction = new VtnSetIcmpTypeActionBuilder().setType(code).build();
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            StatusCode ecode = StatusCode.BADREQUEST;
            String emsg = "VTNSetIcmpCodeAction: Unexpected type: " + vaction;
            try {
                va.toFlowAction(vaction);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            // toFlowAction() should never affect instance variables.
            assertEquals(0, va.getCode());

            // toVtnAction() test.
            SetTpDstAction ma = new SetTpDstActionBuilder().
                setPort(new PortNumber((int)code)).build();
            Action action = new SetTpDstActionCaseBuilder().
                setSetTpDstAction(ma).build();
            vaction = new VtnSetIcmpCodeActionBuilder().setCode(code).build();
            assertEquals(vaction, va.toVtnAction(action));

            action = new SetTpSrcActionCaseBuilder().build();
            emsg = "VTNSetIcmpCodeAction: Unexpected type: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            etag = RpcErrorTag.MISSING_ELEMENT;
            action = new SetTpDstActionCaseBuilder().build();
            emsg = "VTNSetIcmpCodeAction: No ICMP code: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            action = new SetTpDstActionCaseBuilder().
                setSetTpDstAction(new SetTpDstActionBuilder().build()).
                build();
            emsg = "VTNSetIcmpCodeAction: No ICMP code: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(0, va.getCode());

            // getDescription() is not supported.
            action = new SetTpDstActionCaseBuilder().
                setSetTpDstAction(ma).build();
            try {
                va.getDescription(action);
                unexpected();
            } catch (IllegalStateException e) {
            }
            assertEquals(0, va.getCode());
        }

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNSetIcmpCodeAction: Action order cannot be null";
        VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
            setCode((short)0).build();
        try {
            new VTNSetIcmpCodeAction(vact, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Invalid ICMP code.
        etag = RpcErrorTag.BAD_ELEMENT;
        short[] invalid = {
            Short.MIN_VALUE, -0x7fc0, -2000, -3, -2, -1,
            256, 257, 1000, 0xc01, 0x7fc0, Short.MAX_VALUE,
        };
        for (short code: invalid) {
            emsg = "VTNSetIcmpCodeAction: Invalid ICMP code: " + code;
            SetIcmpCodeAction vad = new SetIcmpCodeAction(code);
            try {
                new VTNSetIcmpCodeAction(vad, 1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNSetIcmpCodeAction#equals(Object)}</li>
     *   <li>{@link VTNSetIcmpCodeAction#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        short[] codes = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (short code: codes) {
            VTNSetIcmpCodeAction va1 = new VTNSetIcmpCodeAction(code);
            VTNSetIcmpCodeAction va2 = new VTNSetIcmpCodeAction(code);
            testEquals(set, va1, va2);

            for (Integer order: orders) {
                VtnSetIcmpCodeAction vact1 = new VtnSetIcmpCodeActionBuilder().
                    setCode(code).build();
                VtnSetIcmpCodeAction vact2 = new VtnSetIcmpCodeActionBuilder().
                    setCode(code).build();
                va1 = new VTNSetIcmpCodeAction(vact1, order);
                va2 = new VTNSetIcmpCodeAction(vact2, order);
                testEquals(set, va1, va2);
            }
        }

        int expected = (orders.length + 1) * codes.length;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNSetIcmpCodeAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        short[] codes = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (short code: codes) {
            for (Integer order: orders) {
                VTNSetIcmpCodeAction va;
                String expected;
                if (order == null) {
                    va = new VTNSetIcmpCodeAction(code);
                    expected = "VTNSetIcmpCodeAction[code=" + code + "]";
                } else {
                    VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
                        setCode(code).build();
                    va = new VTNSetIcmpCodeAction(vact, order);
                    expected = "VTNSetIcmpCodeAction[code=" + code +
                        ",order=" + order + "]";
                }
                assertEquals(expected, va.toString());
            }
        }
    }

    /**
     * Test case for {@link VTNSetIcmpCodeAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        short code = 1;
        VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
            setCode(code).build();
        VTNSetIcmpCodeAction va = new VTNSetIcmpCodeAction(vact, order);

        // In case of TCP packet.
        FlowActionContext ctx = Mockito.mock(FlowActionContext.class);
        TcpHeader tcp = Mockito.mock(TcpHeader.class);
        Mockito.when(ctx.getLayer4Header()).thenReturn(tcp);

        assertEquals(false, va.apply(ctx));
        Mockito.verify(ctx).getLayer4Header();
        Mockito.verify(ctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();

        Mockito.verifyZeroInteractions(tcp);

        // In case of UDP packet.
        Mockito.reset(ctx);
        UdpHeader udp = Mockito.mock(UdpHeader.class);
        Mockito.when(ctx.getLayer4Header()).thenReturn(udp);

        assertEquals(false, va.apply(ctx));
        Mockito.verify(ctx).getLayer4Header();
        Mockito.verify(ctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();

        Mockito.verifyZeroInteractions(udp);

        // In case of ICMP packet.
        Mockito.reset(ctx);
        IcmpHeader icmp = Mockito.mock(IcmpHeader.class);
        Mockito.when(ctx.getLayer4Header()).thenReturn(icmp);

        assertEquals(true, va.apply(ctx));
        Mockito.verify(ctx).getLayer4Header();
        Mockito.verify(ctx).addFilterAction(va);
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();

        Mockito.verify(icmp).setIcmpCode(code);
        Mockito.verify(icmp, Mockito.never()).getIcmpType();
        Mockito.verify(icmp, Mockito.never()).setIcmpType(Mockito.anyShort());
        Mockito.verify(icmp, Mockito.never()).getIcmpCode();

        // In case of non-L4 packet.
        Mockito.reset(ctx);
        Mockito.when(ctx.getLayer4Header()).thenReturn(null);

        assertEquals(false, va.apply(ctx));
        Mockito.verify(ctx).getLayer4Header();
        Mockito.verify(ctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();
    }

    /**
     * Ensure that {@link VTNSetIcmpCodeAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Unmarshaller[] unmarshallers = {
            createUnmarshaller(VTNSetIcmpCodeAction.class),
            createUnmarshaller(FlowFilterAction.class),
        };

        short[] codes = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        int[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        Class<VTNSetIcmpCodeAction> type = VTNSetIcmpCodeAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Unmarshaller um: unmarshallers) {
            for (Integer order: orders) {
                for (short code: codes) {
                    String xml = new XmlNode(XML_ROOT).
                        add(new XmlNode("order", order)).
                        add(new XmlNode("code", code)).toString();
                    VTNSetIcmpCodeAction va = unmarshal(um, xml, type);
                    va.verify();
                    assertEquals(order, va.getIdentifier());
                    assertEquals(code, va.getCode());
                }

                // Default ICMP code test.
                String xml = new XmlNode(XML_ROOT).
                    add(new XmlNode("order", order)).toString();
                VTNSetIcmpCodeAction va = unmarshal(um, xml, type);
                va.verify();
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getCode());
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = unmarshallers[0];
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNSetIcmpCodeAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("code", 1)).toString();
        VTNSetIcmpCodeAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Invalid ICMP code.
        etag = RpcErrorTag.BAD_ELEMENT;
        short[] invalid = {
            Short.MIN_VALUE, -0x7fc0, -2000, -3, -2, -1,
            256, 257, 1000, 0xc01, 0x7fc0, Short.MAX_VALUE,
        };
        for (short code: invalid) {
            emsg = "VTNSetIcmpCodeAction: Invalid ICMP code: " + code;
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("order", 1)).
                add(new XmlNode("code", code)).toString();
            va = unmarshal(um, xml, type);
            try {
                va.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }
        }
    }
}
