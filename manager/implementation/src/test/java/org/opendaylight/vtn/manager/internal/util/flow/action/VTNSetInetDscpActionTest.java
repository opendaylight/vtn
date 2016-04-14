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

import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestDscp;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dscp.action._case.VtnSetInetDscpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dscp.action._case.VtnSetInetDscpActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwTosActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwTosActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.tos.action._case.SetNwTosAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.tos.action._case.SetNwTosActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Dscp;

/**
 * JUnit test for {@link VTNSetInetDscpAction}.
 */
public class VTNSetInetDscpActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetInetDscpAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-inet-dscp-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetInetDscpAction} instance.
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
            new XmlValueType("dscp", short.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNSetInetDscpAction#VTNSetInetDscpAction(short)}</li>
     *   <li>{@link VTNSetInetDscpAction#VTNSetInetDscpAction(VtnSetInetDscpActionCase, Integer)}</li>
     *   <li>{@link VTNSetInetDscpAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetInetDscpAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetInetDscpAction#getDscp()}</li>
     *   <li>{@link VTNSetInetDscpAction#verifyImpl()}</li>
     *   <li>{@link VTNSetInetDscpAction#toFlowFilterAction(VtnAction,Integer)}</li>
     *   <li>{@link VTNSetInetDscpAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetInetDscpAction#getDescription(Action)}</li>
     *   <li>{@link VTNSetInetDscpAction#getDescription(VtnAction)}</li>
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
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };
        short[] dscps = {
            0, 1, 2, 11, 22, 33, 44, 55, 60, 62, 63,
        };

        VtnSetInetDscpActionCaseBuilder vacBuilder =
            new VtnSetInetDscpActionCaseBuilder();
        for (Integer order: orders) {
            for (short dscp: dscps) {
                Dscp mdscp = new Dscp(dscp);
                Integer tos = Integer.valueOf(ProtocolUtils.dscpToTos(dscp));
                VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
                    setDscp(mdscp).build();
                VtnSetInetDscpActionCase vac = vacBuilder.
                    setVtnSetInetDscpAction(vact).build();
                SetNwTosAction ma = new SetNwTosActionBuilder().
                    setTos(tos).build();
                SetNwTosActionCase mact = new SetNwTosActionCaseBuilder().
                    setSetNwTosAction(ma).build();

                VTNSetInetDscpAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetInetDscpAction(dscp);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetInetDscpAction(vac, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(dscp, va.getDscp());

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
                    VTNSetInetDscpAction conv = new VTNSetInetDscpAction();
                    assertEquals(va, conv.toFlowFilterAction(vac, order));

                    // toFlowFilterAction() should never affect instance
                    // variables.
                    assertEquals(0, conv.getDscp());
                }
            }

            if (order != null) {
                // Default DSCP test.
                VtnSetInetDscpActionBuilder actb =
                    new VtnSetInetDscpActionBuilder();
                VtnSetInetDscpAction[] vacts = {
                    null,
                    actb.build(),
                    actb.setDscp(new TestDscp()).build(),
                };
                for (VtnSetInetDscpAction vact: vacts) {
                    VtnSetInetDscpActionCase vac = vacBuilder.
                        setVtnSetInetDscpAction(vact).build();
                    VTNSetInetDscpAction va =
                        new VTNSetInetDscpAction(vac, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals((short)0, va.getDscp());
                }
            }
        }

        VTNSetInetDscpAction va = new VTNSetInetDscpAction();
        for (short dscp: dscps) {
            // toVtnAction() test.
            Integer tos = Integer.valueOf(ProtocolUtils.dscpToTos(dscp));
            SetNwTosAction ma = new SetNwTosActionBuilder().
                setTos(tos).build();
            Action action = new SetNwTosActionCaseBuilder().
                setSetNwTosAction(ma).build();
            VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
                setDscp(new Dscp(dscp)).build();
            VtnSetInetDscpActionCase vac = vacBuilder.
                setVtnSetInetDscpAction(vact).build();
            assertEquals(vac, va.toVtnAction(action));

            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            action = new SetTpSrcActionCaseBuilder().build();
            String emsg = "VTNSetInetDscpAction: Unexpected type: " + action;
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
            action = new SetNwTosActionCaseBuilder().build();
            emsg = "VTNSetInetDscpAction: No DSCP value: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            action = new SetNwTosActionCaseBuilder().
                setSetNwTosAction(new SetNwTosActionBuilder().build()).
                build();
            emsg = "VTNSetInetDscpAction: No DSCP value: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(0, va.getDscp());

            // getDescription(VtnAction) test.
            // It should never affect instance variables.
            VtnAction vaction = vac;
            String desc = "set-inet-dscp(" + dscp + ")";
            assertEquals(desc, va.getDescription(vaction));
            assertEquals(0, va.getDscp());

            // getDescription(Action) test.
            // It should never affect instance variables.
            action = new SetNwTosActionCaseBuilder().
                setSetNwTosAction(ma).build();
            desc = "SET_NW_TOS(tos=0x" +
                Integer.toHexString(tos.intValue()) + ")";
            assertEquals(desc, va.getDescription(action));
            assertEquals(0, va.getDscp());
        }

        Action action = new SetNwTosActionCaseBuilder().build();
        String desc = "SET_NW_TOS(tos=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(0, va.getDscp());

        action = new SetNwTosActionCaseBuilder().
            setSetNwTosAction(new SetNwTosActionBuilder().build()).build();
        assertEquals(desc, va.getDescription(action));
        assertEquals(0, va.getDscp());

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        Dscp mdscp = new Dscp((short)0);
        String emsg = "VTNSetInetDscpAction: Action order cannot be null";
        VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
            setDscp(mdscp).build();
        VtnSetInetDscpActionCase vac = vacBuilder.
            setVtnSetInetDscpAction(vact).build();
        try {
            new VTNSetInetDscpAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Invalid DSCP.
        etag = RpcErrorTag.BAD_ELEMENT;
        Short[] invalid = {
            Short.MIN_VALUE, -20000, -50, -3, -2, -1,
            64, 65, 70, 85, 99, 120, 30000, Short.MAX_VALUE,
        };
        for (Short dscp: invalid) {
            vact = mock(VtnSetInetDscpAction.class);
            when(vact.getDscp()).thenReturn(new TestDscp(dscp));
            vac = vacBuilder.setVtnSetInetDscpAction(vact).build();
            emsg = "VTNSetInetDscpAction: Invalid IP DSCP value: " + dscp;
            try {
                new VTNSetInetDscpAction(vac, 1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VTNSetInetDscpAction#newVtnAction(Short)}.
     */
    @Test
    public void testNewVtnAction() {
        Short[] values = {
            0, 1, 11, 22, 33, 44, 55, 62, 63,
        };

        for (Short dscp: values) {
            VtnSetInetDscpActionCase ac =
                VTNSetInetDscpAction.newVtnAction(dscp);
            VtnSetInetDscpAction vaction = ac.getVtnSetInetDscpAction();
            assertEquals(new Dscp(dscp), vaction.getDscp());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNSetInetDscpAction#equals(Object)}</li>
     *   <li>{@link VTNSetInetDscpAction#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };
        Short[] dscps = {
            0, 1, 2, 11, 22, 33, 44, 55, 60, 62, 63,
        };

        for (Short dscp: dscps) {
            VTNSetInetDscpAction va1 = new VTNSetInetDscpAction(dscp);
            VTNSetInetDscpAction va2 = new VTNSetInetDscpAction(dscp);
            testEquals(set, va1, va2);

            for (Integer order: orders) {
                VtnSetInetDscpActionCase vac1 =
                    VTNSetInetDscpAction.newVtnAction(dscp);
                VtnSetInetDscpActionCase vac2 =
                    VTNSetInetDscpAction.newVtnAction(dscp);
                va1 = new VTNSetInetDscpAction(vac1, order);
                va2 = new VTNSetInetDscpAction(vac2, order);
                testEquals(set, va1, va2);
            }
        }

        int expected = (orders.length + 1) * dscps.length;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNSetInetDscpAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };
        Short[] dscps = {
            0, 1, 2, 11, 22, 33, 44, 55, 60, 62, 63,
        };

        for (Integer order: orders) {
            for (Short dscp: dscps) {
                VTNSetInetDscpAction va;
                String expected;
                if (order == null) {
                    va = new VTNSetInetDscpAction(dscp);
                    expected = "VTNSetInetDscpAction[dscp=" + dscp + "]";
                } else {
                    VtnSetInetDscpActionCase vac = VTNSetInetDscpAction.
                        newVtnAction(dscp);
                    va = new VTNSetInetDscpAction(vac, order);
                    expected = "VTNSetInetDscpAction[dscp=" + dscp +
                        ",order=" + order + "]";
                }
                assertEquals(expected, va.toString());
            }
        }
    }

    /**
     * Test case for {@link VTNSetInetDscpAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        short dscp = 33;
        VtnSetInetDscpActionCase vac = VTNSetInetDscpAction.
            newVtnAction(dscp);
        VTNSetInetDscpAction va = new VTNSetInetDscpAction(vac, order);

        // In case of IP packet.
        FlowActionContext ctx = mock(FlowActionContext.class);
        InetHeader inet = mock(InetHeader.class);
        when(ctx.getInetHeader()).thenReturn(inet);

        assertEquals(true, va.apply(ctx));
        verify(ctx).getInetHeader();
        verify(ctx).addFilterAction(va);

        verify(inet).setDscp(dscp);
        verifyNoMoreInteractions(ctx, inet);

        // In case of non-IP packet.
        inet = null;
        reset(ctx);
        when(ctx.getInetHeader()).thenReturn(inet);

        assertEquals(false, va.apply(ctx));
        verify(ctx).getInetHeader();
        verifyNoMoreInteractions(ctx);
    }

    /**
     * Ensure that {@link VTNSetInetDscpAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNSetInetDscpAction.class,
                           FlowFilterAction.class);

        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };
        short[] dscps = {
            0, 1, 2, 11, 22, 33, 44, 55, 60, 62, 63,
        };

        VtnSetInetDscpActionCaseBuilder vacBuilder =
            new VtnSetInetDscpActionCaseBuilder();
        Class<VTNSetInetDscpAction> type = VTNSetInetDscpAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (short dscp: dscps) {
                VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
                    setDscp(new Dscp(dscp)).build();
                VtnSetInetDscpActionCase vac = vacBuilder.
                    setVtnSetInetDscpAction(vact).build();
                for (Integer order: orders) {
                    VTNSetInetDscpAction va =
                        new VTNSetInetDscpAction(vac, order);
                    String xml = marshal(m, va, type, XML_ROOT);
                    VTNSetInetDscpAction va1 = unmarshal(um, xml, type);
                    va1.verify();
                    assertEquals(order, va1.getIdentifier());
                    assertEquals(dscp, va1.getDscp());
                    assertEquals(va, va1);
                }
            }

            // Default DSCP value test.
            for (Integer order: orders) {
                String xml = new XmlNode(XML_ROOT).
                    add(new XmlNode("order", order)).toString();
                VTNSetInetDscpAction va = unmarshal(um, xml, type);
                va.verify();
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getDscp());
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = createUnmarshaller(type);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetInetDscpAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("dscp", (short)0)).toString();
        VTNSetInetDscpAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Invalid VLAN PCP.
        etag = RpcErrorTag.BAD_ELEMENT;
        Short[] invalid = {
            Short.MIN_VALUE, -30000, -0xc00, -0xf0, -3, -2, -1,
            64, 65, 0xa0, 0xc00, 30000, 32000, Short.MAX_VALUE,
        };

        for (Short dscp: invalid) {
            emsg = "VTNSetInetDscpAction: Invalid IP DSCP value: " + dscp;
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("order", 1)).
                add(new XmlNode("dscp", dscp)).toString();
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
