/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.packet.IcmpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.TcpHeader;
import org.opendaylight.vtn.manager.internal.util.packet.UdpHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.type.action._case.VtnSetIcmpTypeAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.icmp.type.action._case.VtnSetIcmpTypeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

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
     *   <li>{@link VTNSetIcmpTypeAction#VTNSetIcmpTypeAction(VtnSetIcmpTypeActionCase, Integer)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#getType()}</li>
     *   <li>{@link VTNSetIcmpTypeAction#verifyImpl()}</li>
     *   <li>{@link VTNSetIcmpTypeAction#toFlowFilterAction(VtnAction,Integer)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#getDescription(Action)}</li>
     *   <li>{@link VTNSetIcmpTypeAction#getDescription(VtnAction)}</li>
     *   <li>{@link FlowFilterAction#verify()}</li>
     *   <li>{@link FlowFilterAction#getIdentifier()}</li>
     *   <li>{@link FlowFilterAction#toVtnFlowAction()}</li>
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

        VtnSetIcmpTypeActionCaseBuilder vacBuilder =
            new VtnSetIcmpTypeActionCaseBuilder();
        for (Integer order: orders) {
            for (short type: types) {
                PortNumber pnum = new PortNumber((int)type);
                VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
                    setType(type).build();
                VtnSetIcmpTypeActionCase vac = vacBuilder.
                    setVtnSetIcmpTypeAction(vact).build();
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
                    va = new VTNSetIcmpTypeAction(vac, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(type, va.getType());

                VtnFlowAction vfact = va.toVtnFlowAction();
                assertEquals(order, vfact.getOrder());
                assertEquals(vac, vfact.getVtnAction());

                VtnFlowActionBuilder vbuilder =
                    va.toVtnFlowActionBuilder(anotherOrder);
                assertEquals(anotherOrder, vbuilder.getOrder());
                assertEquals(vac, vbuilder.getVtnAction());
                assertEquals(order, va.getIdentifier());

                ActionBuilder mbuilder = va.toActionBuilder(anotherOrder);
                assertEquals(anotherOrder, mbuilder.getOrder());
                assertEquals(mact, mbuilder.getAction());
                assertEquals(order, va.getIdentifier());

                if (order != null) {
                    // toFlowFilterAction() test.
                    VTNSetIcmpTypeAction conv = new VTNSetIcmpTypeAction();
                    assertEquals(va, conv.toFlowFilterAction(vac, order));

                    // toFlowFilterAction() should never affect instance
                    // variables.
                    assertEquals(0, conv.getType());
                }
            }

            if (order != null) {
                // Default type test.
                VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
                    build();
                VtnSetIcmpTypeActionCase vac = vacBuilder.
                    setVtnSetIcmpTypeAction(vact).build();
                VTNSetIcmpTypeAction va = new VTNSetIcmpTypeAction(vac, order);
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getType());

                vac = vacBuilder.setVtnSetIcmpTypeAction(null).build();
                va = new VTNSetIcmpTypeAction(vac, order);
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getType());
            }
        }

        VTNSetIcmpTypeAction va = new VTNSetIcmpTypeAction();
        for (short type: types) {
            // toVtnAction() test.
            VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
                setType(type).build();
            VtnAction vaction = vacBuilder.
                setVtnSetIcmpTypeAction(vact).build();
            SetTpSrcAction ma = new SetTpSrcActionBuilder().
                setPort(new PortNumber((int)type)).build();
            Action action = new SetTpSrcActionCaseBuilder().
                setSetTpSrcAction(ma).build();
            assertEquals(vaction, va.toVtnAction(action));

            action = new SetTpDstActionCaseBuilder().build();
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            String emsg = "VTNSetIcmpTypeAction: Unexpected type: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            etag = RpcErrorTag.MISSING_ELEMENT;
            action = new SetTpSrcActionCaseBuilder().build();
            emsg = "VTNSetIcmpTypeAction: No ICMP type: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(0, va.getType());

            // getDescription(VtnAction) test.
            // It should never affect instance variables.
            String desc = "set-icmp-type(" + type + ")";
            assertEquals(desc, va.getDescription(vaction));
            assertEquals(0, va.getType());

            // getDescription(Action) is not supported.
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
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetIcmpTypeAction: Action order cannot be null";
        VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
            setType((short)0).build();
        VtnSetIcmpTypeActionCase vac = vacBuilder.
            setVtnSetIcmpTypeAction(vact).build();
        try {
            new VTNSetIcmpTypeAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Invalid ICMP type.
        etag = RpcErrorTag.BAD_ELEMENT;
        Short[] invalid = {
            Short.MIN_VALUE, -0x7fc0, -2000, -3, -2, -1,
            256, 257, 1000, 0xc01, 0x7fc0, Short.MAX_VALUE,
        };
        for (Short type: invalid) {
            emsg = "VTNSetIcmpTypeAction: Invalid ICMP type: " + type;
            vact = mock(VtnSetIcmpTypeAction.class);
            when(vact.getType()).thenReturn(type);
            vac = vacBuilder.setVtnSetIcmpTypeAction(vact).build();
            try {
                new VTNSetIcmpTypeAction(vac, 1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VTNSetIcmpTypeAction#newVtnAction(Short)}.
     */
    @Test
    public void testNewVtnAction() {
        Short[] values = {
            null, 0, 1, 11, 22, 99, 111, 155, 234, 254, 255,
        };

        for (Short v: values) {
            VtnSetIcmpTypeActionCase ac = VTNSetIcmpTypeAction.newVtnAction(v);
            VtnSetIcmpTypeAction vaction = ac.getVtnSetIcmpTypeAction();
            assertEquals(v, vaction.getType());
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
                VtnSetIcmpTypeActionCase vac1 = VTNSetIcmpTypeAction.
                    newVtnAction(type);
                VtnSetIcmpTypeActionCase vac2 = VTNSetIcmpTypeAction.
                    newVtnAction(type);
                va1 = new VTNSetIcmpTypeAction(vac1, order);
                va2 = new VTNSetIcmpTypeAction(vac2, order);
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
                    VtnSetIcmpTypeActionCase vac = VTNSetIcmpTypeAction.
                        newVtnAction(type);
                    va = new VTNSetIcmpTypeAction(vac, order);
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
        VtnSetIcmpTypeActionCase vac = VTNSetIcmpTypeAction.newVtnAction(type);
        VTNSetIcmpTypeAction va = new VTNSetIcmpTypeAction(vac, order);

        // In case of TCP packet.
        FlowActionContext ctx = mock(FlowActionContext.class);
        TcpHeader tcp = mock(TcpHeader.class);
        when(ctx.getLayer4Header()).thenReturn(tcp);

        assertEquals(false, va.apply(ctx));
        verify(ctx).getLayer4Header();
        verifyNoMoreInteractions(ctx, tcp);

        // In case of UDP packet.
        reset(ctx);
        UdpHeader udp = mock(UdpHeader.class);
        when(ctx.getLayer4Header()).thenReturn(udp);

        assertEquals(false, va.apply(ctx));
        verify(ctx).getLayer4Header();
        verifyNoMoreInteractions(ctx, udp);

        // In case of ICMP packet.
        reset(ctx);
        IcmpHeader icmp = mock(IcmpHeader.class);
        when(ctx.getLayer4Header()).thenReturn(icmp);

        assertEquals(true, va.apply(ctx));
        verify(ctx).getLayer4Header();
        verify(ctx).addFilterAction(va);

        verify(icmp).setIcmpType(type);
        verifyNoMoreInteractions(ctx, icmp);

        // In case of non-L4 packet.
        reset(ctx);
        when(ctx.getLayer4Header()).thenReturn(null);

        assertEquals(false, va.apply(ctx));
        verify(ctx).getLayer4Header();
        verifyNoMoreInteractions(ctx);
    }

    /**
     * Ensure that {@link VTNSetIcmpTypeAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNSetIcmpTypeAction.class,
                           FlowFilterAction.class);

        short[] types = {
            0, 1, 10, 33, 127, 128, 200, 254, 255,
        };
        int[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetIcmpTypeActionCaseBuilder vacBuilder =
            new VtnSetIcmpTypeActionCaseBuilder();
        Class<VTNSetIcmpTypeAction> type = VTNSetIcmpTypeAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (short itype: types) {
                VtnSetIcmpTypeAction vact = new VtnSetIcmpTypeActionBuilder().
                    setType(itype).build();
                VtnSetIcmpTypeActionCase vac = vacBuilder.
                    setVtnSetIcmpTypeAction(vact).build();
                for (Integer order: orders) {
                    VTNSetIcmpTypeAction va =
                        new VTNSetIcmpTypeAction(vac, order);
                    String xml = marshal(m, va, type, XML_ROOT);
                    VTNSetIcmpTypeAction va1 = unmarshal(um, xml, type);
                    va1.verify();
                    assertEquals(order, va1.getIdentifier());
                    assertEquals(itype, va1.getType());
                    assertEquals(va, va1);
                }
            }

            // Default ICMP type test.
            for (Integer order: orders) {
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
        Unmarshaller um = createUnmarshaller(type);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetIcmpTypeAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("type", 1)).toString();
        VTNSetIcmpTypeAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }
}
