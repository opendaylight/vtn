/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Marshaller;
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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.code.action._case.VtnSetIcmpCodeAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.code.action._case.VtnSetIcmpCodeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

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
     *   <li>{@link VTNSetIcmpCodeAction#VTNSetIcmpCodeAction(VtnSetIcmpCodeActionCase, Integer)}</li>
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

        VtnSetIcmpCodeActionCaseBuilder vacBuilder =
            new VtnSetIcmpCodeActionCaseBuilder();
        for (Integer order: orders) {
            for (short code: codes) {
                PortNumber pnum = new PortNumber((int)code);
                SetIcmpCodeAction vad = new SetIcmpCodeAction(code);
                VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
                    setCode(code).build();
                VtnSetIcmpCodeActionCase vac = vacBuilder.
                    setVtnSetIcmpCodeAction(vact).build();
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

                    va = new VTNSetIcmpCodeAction(vac, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(code, va.getCode());

                VtnFlowActionBuilder vbuilder =
                    va.toVtnFlowActionBuilder(anotherOrder);
                assertEquals(anotherOrder, vbuilder.getOrder());
                assertEquals(vac, vbuilder.getVtnAction());
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
                VtnSetIcmpCodeActionCase vac = vacBuilder.
                    setVtnSetIcmpCodeAction(vact).build();
                VTNSetIcmpCodeAction va = new VTNSetIcmpCodeAction(vac, order);
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getCode());

                vac = vacBuilder.setVtnSetIcmpCodeAction(null).build();
                va = new VTNSetIcmpCodeAction(vac, order);
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getCode());
            }
        }

        VTNSetIcmpCodeAction va = new VTNSetIcmpCodeAction();
        for (short code: codes) {
            // toFlowAction() test.
            SetIcmpCodeAction vad = new SetIcmpCodeAction(code);
            VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
                setCode(code).build();
            VtnAction vaction = vacBuilder.
                setVtnSetIcmpCodeAction(vact).build();
            assertEquals(vad, va.toFlowAction(vaction));

            vaction = VTNSetIcmpTypeAction.newVtnAction(code);
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            String emsg = "VTNSetIcmpCodeAction: Unexpected type: " + vaction;
            try {
                va.toFlowAction(vaction);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            // toFlowAction() should never affect instance variables.
            assertEquals(0, va.getCode());

            // toVtnAction() test.
            SetTpDstAction ma = new SetTpDstActionBuilder().
                setPort(new PortNumber((int)code)).build();
            Action action = new SetTpDstActionCaseBuilder().
                setSetTpDstAction(ma).build();
            vaction = vacBuilder.setVtnSetIcmpCodeAction(vact).build();
            assertEquals(vaction, va.toVtnAction(action));

            action = new SetTpSrcActionCaseBuilder().build();
            emsg = "VTNSetIcmpCodeAction: Unexpected type: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            etag = RpcErrorTag.MISSING_ELEMENT;
            action = new SetTpDstActionCaseBuilder().build();
            emsg = "VTNSetIcmpCodeAction: No ICMP code: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
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
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetIcmpCodeAction: Action order cannot be null";
        VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
            setCode((short)0).build();
        VtnSetIcmpCodeActionCase vac = vacBuilder.
            setVtnSetIcmpCodeAction(vact).build();
        try {
            new VTNSetIcmpCodeAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }

        // Default value test for toFlowAction().
        SetIcmpCodeAction vad = new SetIcmpCodeAction((short)0);
        vac = vacBuilder.setVtnSetIcmpCodeAction(null).build();
        assertEquals(vad, va.toFlowAction(vac));

        vac = vacBuilder.
            setVtnSetIcmpCodeAction(new VtnSetIcmpCodeActionBuilder().build()).
            build();
        assertEquals(vad, va.toFlowAction(vac));
    }

    /**
     * Test case for {@link VTNSetIcmpCodeAction#newVtnAction(Short)}.
     */
    @Test
    public void testNewVtnAction() {
        Short[] values = {
            null, 0, 1, 11, 22, 99, 111, 155, 234, 254, 255,
        };

        for (Short v: values) {
            VtnSetIcmpCodeActionCase ac = VTNSetIcmpCodeAction.newVtnAction(v);
            VtnSetIcmpCodeAction vaction = ac.getVtnSetIcmpCodeAction();
            assertEquals(v, vaction.getCode());
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
                VtnSetIcmpCodeActionCase vac1 = VTNSetIcmpCodeAction.
                    newVtnAction(code);
                VtnSetIcmpCodeActionCase vac2 = VTNSetIcmpCodeAction.
                    newVtnAction(code);
                va1 = new VTNSetIcmpCodeAction(vac1, order);
                va2 = new VTNSetIcmpCodeAction(vac2, order);
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
                    VtnSetIcmpCodeActionCase vac = VTNSetIcmpCodeAction.
                        newVtnAction(code);
                    va = new VTNSetIcmpCodeAction(vac, order);
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
        VtnSetIcmpCodeActionCase vac = VTNSetIcmpCodeAction.newVtnAction(code);
        VTNSetIcmpCodeAction va = new VTNSetIcmpCodeAction(vac, order);

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
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNSetIcmpCodeAction.class,
                           FlowFilterAction.class);

        short[] codes = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        int[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetIcmpCodeActionCaseBuilder vacBuilder =
            new VtnSetIcmpCodeActionCaseBuilder();
        Class<VTNSetIcmpCodeAction> type = VTNSetIcmpCodeAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (short code: codes) {
                VtnSetIcmpCodeAction vact = new VtnSetIcmpCodeActionBuilder().
                    setCode(code).build();
                VtnSetIcmpCodeActionCase vac = vacBuilder.
                    setVtnSetIcmpCodeAction(vact).build();
                for (Integer order: orders) {
                    VTNSetIcmpCodeAction va =
                        new VTNSetIcmpCodeAction(vac, order);
                    String xml = marshal(m, va, type, XML_ROOT);
                    VTNSetIcmpCodeAction va1 = unmarshal(um, xml, type);
                    va1.verify();
                    assertEquals(order, va1.getIdentifier());
                    assertEquals(code, va1.getCode());
                    assertEquals(va, va1);
                }
            }

            // Default ICMP code test.
            for (Integer order: orders) {
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
        Unmarshaller um = createUnmarshaller(type);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetIcmpCodeAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("code", 1)).toString();
        VTNSetIcmpCodeAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }
}
