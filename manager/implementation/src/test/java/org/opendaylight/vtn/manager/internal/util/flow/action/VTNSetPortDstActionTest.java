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
import org.opendaylight.vtn.manager.internal.TestPortNumber;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.dst.action._case.VtnSetPortDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.port.dst.action._case.VtnSetPortDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.tp.dst.action._case.SetTpDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev130715.PortNumber;

/**
 * JUnit test for {@link VTNSetPortDstAction}.
 */
public class VTNSetPortDstActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetPortDstAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-port-dst-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetPortDstAction} instance.
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
            new XmlValueType("port", int.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNSetPortDstAction#VTNSetPortDstAction(int)}</li>
     *   <li>{@link VTNSetPortDstAction#VTNSetPortDstAction(VtnSetPortDstActionCase, Integer)}</li>
     *   <li>{@link VTNSetPortDstAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetPortDstAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetPortDstAction#toFlowFilterAction(VtnAction,Integer)}</li>
     *   <li>{@link VTNSetPortDstAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetPortDstAction#getDescription(Action)}</li>
     *   <li>{@link VTNSetPortDstAction#getDescription(VtnAction)}</li>
     *   <li>{@link VTNPortAction#getPort()}</li>
     *   <li>{@link VTNPortAction#getPortNumber()}</li>
     *   <li>{@link VTNPortAction#verifyImpl()}</li>
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
        int[] ports = {
            0, 1, 10, 33, 1234, 32767, 32768, 40000, 55555, 65535,
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetPortDstActionCaseBuilder vacBuilder =
            new VtnSetPortDstActionCaseBuilder();
        for (Integer order: orders) {
            for (int port: ports) {
                PortNumber pnum = new PortNumber(port);
                VtnSetPortDstAction vact = new VtnSetPortDstActionBuilder().
                    setPort(pnum).build();
                VtnSetPortDstActionCase vac = vacBuilder.
                    setVtnSetPortDstAction(vact).build();
                SetTpDstAction ma = new SetTpDstActionBuilder().
                    setPort(pnum).build();
                SetTpDstActionCase mact = new SetTpDstActionCaseBuilder().
                    setSetTpDstAction(ma).build();

                VTNSetPortDstAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetPortDstAction(port);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetPortDstAction(vac, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(port, va.getPort());
                assertEquals(pnum, va.getPortNumber());

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
                    VTNSetPortDstAction conv = new VTNSetPortDstAction();
                    assertEquals(va, conv.toFlowFilterAction(vac, order));

                    // toFlowFilterAction() should never affect instance
                    // variables.
                    assertEquals(0, conv.getPort());
                }
            }

            // Default port number test.
            if (order != null) {
                VtnSetPortDstActionBuilder actb =
                    new VtnSetPortDstActionBuilder();
                VtnSetPortDstAction[] vacts = {
                    null,
                    actb.build(),
                    actb.setPort(new TestPortNumber()).build(),
                };
                for (VtnSetPortDstAction vact: vacts) {
                    VtnSetPortDstActionCase vac = vacBuilder.
                        setVtnSetPortDstAction(vact).build();
                    VTNSetPortDstAction va =
                        new VTNSetPortDstAction(vac, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals(0, va.getPort());
                }
            }
        }

        VTNSetPortDstAction va = new VTNSetPortDstAction();
        for (int port: ports) {
            // toVtnAction() test.
            PortNumber pnum = new PortNumber(port);
            VtnSetPortDstAction vact = new VtnSetPortDstActionBuilder().
                setPort(pnum).build();
            VtnAction vaction = vacBuilder.
                setVtnSetPortDstAction(vact).build();
            SetTpDstAction ma = new SetTpDstActionBuilder().
                setPort(pnum).build();
            Action action = new SetTpDstActionCaseBuilder().
                setSetTpDstAction(ma).build();
            assertEquals(vaction, va.toVtnAction(action));

            action = new SetTpSrcActionCaseBuilder().build();
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            String emsg = "VTNSetPortDstAction: Unexpected type: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            try {
                va.getDescription(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            etag = RpcErrorTag.MISSING_ELEMENT;
            action = new SetTpDstActionCaseBuilder().build();
            emsg = "VTNSetPortDstAction: No port number: " + action;
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
            emsg = "VTNSetPortDstAction: No port number: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(0, va.getPort());

            // getDescription(VtnAction) test.
            // It should never affect instance variables.
            String desc = "set-port-dst(" + port + ")";
            assertEquals(desc, va.getDescription(vaction));
            assertEquals(0, va.getPort());

            // getDescription(Action) test.
            // It should never affect instance variables.
            action = new SetTpDstActionCaseBuilder().
                setSetTpDstAction(ma).build();
            desc = "SET_TP_DST(port=" + port + ")";
            assertEquals(desc, va.getDescription(action));
            assertEquals(0, va.getPort());
        }

        Action action = new SetTpDstActionCaseBuilder().build();
        String desc = "SET_TP_DST(port=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(0, va.getPort());

        action = new SetTpDstActionCaseBuilder().
            setSetTpDstAction(new SetTpDstActionBuilder().build()).
            build();
        assertEquals(desc, va.getDescription(action));
        assertEquals(0, va.getPort());

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        PortNumber pnum = new PortNumber(1);
        String emsg = "VTNSetPortDstAction: Action order cannot be null";
        VtnSetPortDstAction vact = new VtnSetPortDstActionBuilder().
            setPort(pnum).build();
        VtnSetPortDstActionCase vac = vacBuilder.
            setVtnSetPortDstAction(vact).build();
        try {
            new VTNSetPortDstAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Invalid port number.
        etag = RpcErrorTag.BAD_ELEMENT;
        int[] invalid = {
            Integer.MIN_VALUE, -0xfff0000, -1000000, -33333, -2, -1,
            65536, 65537, 0xabcdef, 0xfff00000, Integer.MAX_VALUE,
        };
        for (int port: invalid) {
            emsg = "VTNSetPortDstAction: Invalid port number: " + port;
            vact = mock(VtnSetPortDstAction.class);
            when(vact.getPort()).thenReturn(new TestPortNumber(port));
            vac = vacBuilder.setVtnSetPortDstAction(vact).build();
            try {
                new VTNSetPortDstAction(vac, 1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VTNSetPortDstAction#newVtnAction(PortNumber)}.
     */
    @Test
    public void testNewVtnAction() {
        PortNumber[] values = {
            null,
            new PortNumber(0),
            new PortNumber(1),
            new PortNumber(9999),
            new PortNumber(32767),
            new PortNumber(32768),
            new PortNumber(65534),
            new PortNumber(65535),
        };

        for (PortNumber pnum: values) {
            VtnSetPortDstActionCase ac =
                VTNSetPortDstAction.newVtnAction(pnum);
            VtnSetPortDstAction vaction = ac.getVtnSetPortDstAction();
            assertEquals(pnum, vaction.getPort());
        }
    }

    /**
     * Test case for {@link VTNSetPortDstAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        int[] ports = {
            0, 1, 10, 33, 1234, 32767, 32768, 40000, 55555, 65535,
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (Integer order: orders) {
            for (int port: ports) {
                VTNSetPortDstAction va;
                String expected;
                if (order == null) {
                    va = new VTNSetPortDstAction(port);
                    expected = "VTNSetPortDstAction[port=" + port + "]";
                } else {
                    VtnSetPortDstActionCase vac = VTNSetPortDstAction.
                        newVtnAction(new PortNumber(port));
                    va = new VTNSetPortDstAction(vac, order);
                    expected = "VTNSetPortDstAction[port=" + port +
                        ",order=" + order + "]";
                }
                assertEquals(expected, va.toString());
            }
        }
    }

    /**
     * Test case for {@link VTNSetPortDstAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        int port = 33333;
        VtnSetPortDstActionCase vac = VTNSetPortDstAction.
            newVtnAction(new PortNumber(port));
        VTNSetPortDstAction va = new VTNSetPortDstAction(vac, order);

        // In case of TCP packet.
        FlowActionContext ctx = mock(FlowActionContext.class);
        TcpHeader tcp = mock(TcpHeader.class);
        when(ctx.getLayer4Header()).thenReturn(tcp);

        assertEquals(true, va.apply(ctx));
        verify(ctx).getLayer4Header();
        verify(ctx).addFilterAction(va);

        verify(tcp).setDestinationPort(port);
        verifyNoMoreInteractions(ctx, tcp);

        // In case of UDP packet.
        reset(ctx);
        UdpHeader udp = mock(UdpHeader.class);
        when(ctx.getLayer4Header()).thenReturn(udp);

        assertEquals(true, va.apply(ctx));
        verify(ctx).getLayer4Header();
        verify(ctx).addFilterAction(va);

        verify(udp).setDestinationPort(port);
        verifyNoMoreInteractions(ctx, udp);

        // In case of ICMP packet.
        reset(ctx);
        IcmpHeader icmp = mock(IcmpHeader.class);
        when(ctx.getLayer4Header()).thenReturn(icmp);

        assertEquals(false, va.apply(ctx));
        verify(ctx).getLayer4Header();
        verifyNoMoreInteractions(ctx, icmp);

        // In case of non-L4 packet.
        reset(ctx);
        when(ctx.getLayer4Header()).thenReturn(null);

        assertEquals(false, va.apply(ctx));
        verify(ctx).getLayer4Header();
        verifyNoMoreInteractions(ctx);
    }

    /**
     * Ensure that {@link VTNSetPortDstAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNSetPortDstAction.class,
                           VTNPortAction.class, FlowFilterAction.class);

        int[] ports = {
            0, 1, 10, 33, 1234, 32767, 32768, 40000, 55555, 65535,
        };
        int[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetPortDstActionCaseBuilder vacBuilder =
            new VtnSetPortDstActionCaseBuilder();
        Class<VTNSetPortDstAction> type = VTNSetPortDstAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (int port: ports) {
                VtnSetPortDstAction vact = new VtnSetPortDstActionBuilder().
                    setPort(new PortNumber(port)).build();
                VtnSetPortDstActionCase vac = vacBuilder.
                    setVtnSetPortDstAction(vact).build();
                for (Integer order: orders) {
                    VTNSetPortDstAction va =
                        new VTNSetPortDstAction(vac, order);
                    String xml = marshal(m, va, type, XML_ROOT);
                    VTNSetPortDstAction va1 = unmarshal(um, xml, type);
                    va1.verify();
                    assertEquals(order, va1.getIdentifier());
                    assertEquals(port, va1.getPort());
                    assertEquals(va, va1);
                }
            }

            // Default port number test.
            for (Integer order: orders) {
                String xml = new XmlNode(XML_ROOT).
                    add(new XmlNode("order", order)).toString();
                VTNSetPortDstAction va = unmarshal(um, xml, type);
                va.verify();
                assertEquals(order, va.getIdentifier());
                assertEquals(0, va.getPort());
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = createUnmarshaller(type);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetPortDstAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("port", 12345)).toString();
        VTNSetPortDstAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Invalid port number.
        etag = RpcErrorTag.BAD_ELEMENT;
        int[] invalid = {
            Integer.MIN_VALUE, -0xfff0000, -1000000, -33333, -2, -1,
            65536, 65537, 0xabcdef, 0xfff00000, Integer.MAX_VALUE,
        };
        for (int port: invalid) {
            emsg = "VTNSetPortDstAction: Invalid port number: " + port;
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("order", 1)).
                add(new XmlNode("port", port)).toString();
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
