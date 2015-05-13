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

import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;

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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.src.action._case.SetTpSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.src.action._case.SetTpSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * JUnit test for {@link VTNSetIcmpTypeAction}.
 */
public class VTNSetIcmpTypeActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetIcmpTypeAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-icmp-type-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetIcmpTypeAction} instance.
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
            new XmlValueType("type", short.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNSetIcmpTypeAction#VTNSetIcmpTypeAction(short)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#VTNSetIcmpTypeAction(SetIcmpTypeAction, int)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#VTNSetIcmpTypeAction(VtnSetIcmpTypeAction, Integer)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#getType()}</li>
     *   <li>{@link VTNSetIcmpTypeAction#verifyImpl()}</li>
     *   <li>{@link VTNSetIcmpTypeAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#toVtnAction(Action)}</li>
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
        short[] types = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (Integer order: orders) {
            for (short type: types) {
                PortNumber pnum = new PortNumber((int)type);
                SetIcmpTypeAction vad = new SetIcmpTypeAction(type);
                VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
                    setType(type).build();
                SetTpSrcAction ma = new SetTpSrcActionBuilder().
                    setPort(pnum).build();
                SetTpSrcActionCase mact = new SetTpSrcActionCaseBuilder().
                    setSetTpSrcAction(ma).build();

                VTNSetIcmpTypeAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetIcmpTypeAction(type);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetIcmpTypeAction(vad, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals(type, va.getType());

                    va = new VTNSetIcmpTypeAction(vact, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(type, va.getType());

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
                // Default type test.
                VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
                    build();
                VTNSetIcmpTypeAction va = new VTNSetIcmpTypeAction(vact, order);
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getType());
            }
        }

        VTNSetIcmpTypeAction va = new VTNSetIcmpTypeAction();
        for (short type: types) {
            // toFlowAction() test.
            SetIcmpTypeAction vad = new SetIcmpTypeAction(type);
            VtnAction vaction = new VtnSetIcmpTypeActionBuilder().
                setType(type).build();
            assertEquals(vad, va.toFlowAction(vaction));

            vaction = new VtnSetIcmpCodeActionBuilder().setCode(type).build();
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            StatusCode ecode = StatusCode.BADREQUEST;
            String emsg = "VTNSetIcmpTypeAction: Unexpected type: " + vaction;
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
            assertEquals(0, va.getType());

            // toVtnAction() test.
            SetTpSrcAction ma = new SetTpSrcActionBuilder().
                setPort(new PortNumber((int)type)).build();
            Action action = new SetTpSrcActionCaseBuilder().
                setSetTpSrcAction(ma).build();
            vaction = new VtnSetIcmpTypeActionBuilder().setType(type).build();
            assertEquals(vaction, va.toVtnAction(action));

            action = new SetTpDstActionCaseBuilder().build();
            emsg = "VTNSetIcmpTypeAction: Unexpected type: " + action;
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
            action = new SetTpSrcActionCaseBuilder().build();
            emsg = "VTNSetIcmpTypeAction: No ICMP type: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            action = new SetTpSrcActionCaseBuilder().
                setSetTpSrcAction(new SetTpSrcActionBuilder().build()).
                build();
            emsg = "VTNSetIcmpTypeAction: No ICMP type: " + action;
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
            assertEquals(0, va.getType());

            // getDescription() is not supported.
            action = new SetTpSrcActionCaseBuilder().
                setSetTpSrcAction(ma).build();
            try {
                va.getDescription(action);
                unexpected();
            } catch (IllegalStateException e) {
            }
            assertEquals(0, va.getType());
        }

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNSetIcmpTypeAction: Action order cannot be null";
        VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
            setType((short)0).build();
        try {
            new VTNSetIcmpTypeAction(vact, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Invalid ICMP type.
        etag = RpcErrorTag.BAD_ELEMENT;
        short[] invalid = {
            Short.MIN_VALUE, -0x7fc0, -2000, -3, -2, -1,
            256, 257, 1000, 0xc01, 0x7fc0, Short.MAX_VALUE,
        };
        for (short type: invalid) {
            emsg = "VTNSetIcmpTypeAction: Invalid ICMP type: " + type;
            SetIcmpTypeAction vad = new SetIcmpTypeAction(type);
            try {
                new VTNSetIcmpTypeAction(vad, 1);
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
     *   <li>{@link VTNSetIcmpTypeAction#equals(Object)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        short[] types = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (short type: types) {
            VTNSetIcmpTypeAction va1 = new VTNSetIcmpTypeAction(type);
            VTNSetIcmpTypeAction va2 = new VTNSetIcmpTypeAction(type);
            testEquals(set, va1, va2);

            for (Integer order: orders) {
                VtnSetIcmpTypeAction vact1 = new VtnSetIcmpTypeActionBuilder().
                    setType(type).build();
                VtnSetIcmpTypeAction vact2 = new VtnSetIcmpTypeActionBuilder().
                    setType(type).build();
                va1 = new VTNSetIcmpTypeAction(vact1, order);
                va2 = new VTNSetIcmpTypeAction(vact2, order);
                testEquals(set, va1, va2);
            }
        }

        int expected = (orders.length + 1) * types.length;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNSetIcmpTypeAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        short[] types = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (Integer order: orders) {
            for (short type: types) {
                VTNSetIcmpTypeAction va;
                String expected;
                if (order == null) {
                    va = new VTNSetIcmpTypeAction(type);
                    expected = "VTNSetIcmpTypeAction[type=" + type + "]";
                } else {
                    VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
                        setType(type).build();
                    va = new VTNSetIcmpTypeAction(vact, order);
                    expected = "VTNSetIcmpTypeAction[type=" + type +
                        ",order=" + order + "]";
                }
                assertEquals(expected, va.toString());
            }
        }
    }

    /**
     * Test case for {@link VTNSetIcmpTypeAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        short type = 1;
        VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
            setType(type).build();
        VTNSetIcmpTypeAction va = new VTNSetIcmpTypeAction(vact, order);

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

        Mockito.verify(icmp).setIcmpType(type);
        Mockito.verify(icmp, Mockito.never()).getIcmpType();
        Mockito.verify(icmp, Mockito.never()).getIcmpCode();
        Mockito.verify(icmp, Mockito.never()).setIcmpCode(Mockito.anyShort());

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
     * Ensure that {@link VTNSetIcmpTypeAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Unmarshaller[] unmarshallers = {
            createUnmarshaller(VTNSetIcmpTypeAction.class),
            createUnmarshaller(FlowFilterAction.class),
        };

        short[] types = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        int[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        Class<VTNSetIcmpTypeAction> type = VTNSetIcmpTypeAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Unmarshaller um: unmarshallers) {
            for (Integer order: orders) {
                for (short itype: types) {
                    String xml = new XmlNode(XML_ROOT).
                        add(new XmlNode("order", order)).
                        add(new XmlNode("type", itype)).toString();
                    VTNSetIcmpTypeAction va = unmarshal(um, xml, type);
                    va.verify();
                    assertEquals(order, va.getIdentifier());
                    assertEquals(itype, va.getType());
                }

                // Default ICMP type test.
                String xml = new XmlNode(XML_ROOT).
                    add(new XmlNode("order", order)).toString();
                VTNSetIcmpTypeAction va = unmarshal(um, xml, type);
                va.verify();
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getType());
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = unmarshallers[0];
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNSetIcmpTypeAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("type", 1)).toString();
        VTNSetIcmpTypeAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Invalid ICMP type.
        etag = RpcErrorTag.BAD_ELEMENT;
        short[] invalid = {
            Short.MIN_VALUE, -0x7fc0, -2000, -3, -2, -1,
            256, 257, 1000, 0xc01, 0x7fc0, Short.MAX_VALUE,
        };
        for (short itype: invalid) {
            emsg = "VTNSetIcmpTypeAction: Invalid ICMP type: " + itype;
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("order", 1)).
                add(new XmlNode("type", itype)).toString();
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
