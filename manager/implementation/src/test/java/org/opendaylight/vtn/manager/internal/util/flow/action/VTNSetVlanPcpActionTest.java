/*
 * Copyright (c) 2015, 2017 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import static org.mockito.Mockito.mock;
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

import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestVlanPcp;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanPcpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanPcpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.pcp.action._case.VtnSetVlanPcpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.vlan.pcp.action._case.VtnSetVlanPcpActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanPcpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanPcpActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.pcp.action._case.SetVlanPcpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.pcp.action._case.SetVlanPcpActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * JUnit test for {@link VTNSetVlanPcpAction}.
 */
public class VTNSetVlanPcpActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetVlanPcpAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-vlan-pcp-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetVlanPcpAction} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlValueType("order", Integer.class).add(name).prepend(parent),
            new XmlValueType("priority",
                             short.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNSetVlanPcpAction#VTNSetVlanPcpAction(short)}</li>
     *   <li>{@link VTNSetVlanPcpAction#VTNSetVlanPcpAction(VtnSetVlanPcpActionCase, Integer)}</li>
     *   <li>{@link VTNSetVlanPcpAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetVlanPcpAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetVlanPcpAction#getPriority()}</li>
     *   <li>{@link VTNSetVlanPcpAction#verifyImpl()}</li>
     *   <li>{@link VTNSetVlanPcpAction#toFlowFilterAction(VtnAction,Integer)}</li>
     *   <li>{@link VTNSetVlanPcpAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetVlanPcpAction#getDescription(Action)}</li>
     *   <li>{@link VTNSetVlanPcpAction#getDescription(VtnAction)}</li>
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

        VtnSetVlanPcpActionCaseBuilder vacBuilder =
            new VtnSetVlanPcpActionCaseBuilder();
        for (Integer order: orders) {
            for (short pcp = 0; pcp <= 7; pcp++) {
                VlanPcp vpcp = new VlanPcp(pcp);
                VtnSetVlanPcpAction vact = new VtnSetVlanPcpActionBuilder().
                    setVlanPcp(vpcp).build();
                VtnSetVlanPcpActionCase vac = vacBuilder.
                    setVtnSetVlanPcpAction(vact).build();
                SetVlanPcpAction ma = new SetVlanPcpActionBuilder().
                    setVlanPcp(vpcp).build();
                SetVlanPcpActionCase mact = new SetVlanPcpActionCaseBuilder().
                    setSetVlanPcpAction(ma).build();

                VTNSetVlanPcpAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetVlanPcpAction(pcp);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetVlanPcpAction(vac, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(pcp, va.getPriority());

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
                    VTNSetVlanPcpAction conv = new VTNSetVlanPcpAction();
                    assertEquals(va, conv.toFlowFilterAction(vac, order));

                    // toFlowFilterAction() should never affect instance
                    // variables.
                    assertEquals(0, conv.getPriority());
                }
            }

            if (order != null) {
                // Default priority test.
                VtnSetVlanPcpActionBuilder actb =
                    new VtnSetVlanPcpActionBuilder();
                VtnSetVlanPcpAction[] vacts = {
                    null,
                    actb.build(),
                    actb.setVlanPcp(new TestVlanPcp()).build(),
                };
                for (VtnSetVlanPcpAction vact: vacts) {
                    VtnSetVlanPcpActionCase vac = vacBuilder.
                        setVtnSetVlanPcpAction(vact).build();
                    VTNSetVlanPcpAction va =
                        new VTNSetVlanPcpAction(vac, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals((short)0, va.getPriority());
                }
            }
        }

        VTNSetVlanPcpAction va = new VTNSetVlanPcpAction();
        for (short pcp = 0; pcp <= 7; pcp++) {
            // toVtnAction() test.
            VlanPcp vpcp = new VlanPcp(pcp);
            VtnSetVlanPcpAction vact = new VtnSetVlanPcpActionBuilder().
                setVlanPcp(vpcp).build();
            VtnAction vac = vacBuilder.setVtnSetVlanPcpAction(vact).build();
            SetVlanPcpAction ma = new SetVlanPcpActionBuilder().
                setVlanPcp(vpcp).build();
            Action action = new SetVlanPcpActionCaseBuilder().
                setSetVlanPcpAction(ma).build();
            assertEquals(vac, va.toVtnAction(action));

            action = new SetTpDstActionCaseBuilder().build();
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            String emsg = "VTNSetVlanPcpAction: Unexpected type: " + action;
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
            assertEquals(0, va.getPriority());
            action = new SetVlanPcpActionCaseBuilder().build();
            emsg = "VTNSetVlanPcpAction: No VLAN PCP: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            action = new SetVlanPcpActionCaseBuilder().
                setSetVlanPcpAction(new SetVlanPcpActionBuilder().build()).
                build();
            emsg = "VTNSetVlanPcpAction: No VLAN PCP: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(0, va.getPriority());

            // getDescription(VtnAction) test.
            // It should never affect instance variables.
            VtnAction vaction = vac;
            String desc = "set-vlan-pcp(" + pcp + ")";
            assertEquals(desc, va.getDescription(vaction));
            assertEquals(0, va.getPriority());

            // getDescription(Action) test.
            // It should never affect instance variables.
            action = new SetVlanPcpActionCaseBuilder().
                setSetVlanPcpAction(ma).build();
            desc = "SET_VLAN_PCP(pcp=" + pcp + ")";
            assertEquals(desc, va.getDescription(action));
            assertEquals(0, va.getPriority());
        }

        Action action = new SetVlanPcpActionCaseBuilder().build();
        String desc = "SET_VLAN_PCP(pcp=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(0, va.getPriority());

        action = new SetVlanPcpActionCaseBuilder().
            setSetVlanPcpAction(new SetVlanPcpActionBuilder().build()).
            build();
        assertEquals(desc, va.getDescription(action));
        assertEquals(0, va.getPriority());

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        VlanPcp vpcp = new VlanPcp((short)0);
        String emsg = "VTNSetVlanPcpAction: Action order cannot be null";
        VtnSetVlanPcpAction vact = new VtnSetVlanPcpActionBuilder().
            setVlanPcp(vpcp).build();
        VtnSetVlanPcpActionCase vac = vacBuilder.
            setVtnSetVlanPcpAction(vact).build();
        try {
            new VTNSetVlanPcpAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Invalid VLAN PCP.
        etag = RpcErrorTag.BAD_ELEMENT;
        Short[] invalid = {
            Short.MIN_VALUE, -30000, -100, -50, -3, -2, -1,
            8, 9, 30, 40, 110, 31000, Short.MAX_VALUE,
        };
        for (Short pri: invalid) {
            vact = mock(VtnSetVlanPcpAction.class);
            when(vact.getVlanPcp()).thenReturn(new TestVlanPcp(pri));
            vac = vacBuilder.setVtnSetVlanPcpAction(vact).build();
            emsg = "VTNSetVlanPcpAction: Invalid VLAN priority: " + pri;
            try {
                new VTNSetVlanPcpAction(vac, 1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VTNSetVlanPcpAction#newVtnAction(VlanPcp)}.
     */
    @Test
    public void testNewVtnAction() {
        List<VlanPcp> pcps = new ArrayList<>();
        pcps.add(null);
        for (short i = 0; i <= 7; i++) {
            pcps.add(new VlanPcp(i));
        }

        for (VlanPcp pcp: pcps) {
            VtnSetVlanPcpActionCase ac = VTNSetVlanPcpAction.newVtnAction(pcp);
            VtnSetVlanPcpAction vaction = ac.getVtnSetVlanPcpAction();
            assertEquals(pcp, vaction.getVlanPcp());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNSetVlanPcpAction#equals(Object)}</li>
     *   <li>{@link VTNSetVlanPcpAction#hashCode()}</li>
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

        VtnSetVlanPcpActionCaseBuilder vacBuilder =
            new VtnSetVlanPcpActionCaseBuilder();
        for (short pcp = 0; pcp <= 7; pcp++) {
            VTNSetVlanPcpAction va1 = new VTNSetVlanPcpAction(pcp);
            VTNSetVlanPcpAction va2 = new VTNSetVlanPcpAction(pcp);
            testEquals(set, va1, va2);

            for (Integer order: orders) {
                VtnSetVlanPcpAction vact1 = new VtnSetVlanPcpActionBuilder().
                    setVlanPcp(new VlanPcp(pcp)).build();
                VtnSetVlanPcpAction vact2 = new VtnSetVlanPcpActionBuilder().
                    setVlanPcp(new VlanPcp(pcp)).build();
                VtnSetVlanPcpActionCase vac1 = vacBuilder.
                    setVtnSetVlanPcpAction(vact1).build();
                VtnSetVlanPcpActionCase vac2 = vacBuilder.
                    setVtnSetVlanPcpAction(vact2).build();
                va1 = new VTNSetVlanPcpAction(vac1, order);
                va2 = new VTNSetVlanPcpAction(vac2, order);
                testEquals(set, va1, va2);
            }
        }

        int expected = (orders.length + 1) * 8;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNSetVlanPcpAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };
        VtnSetVlanPcpActionCaseBuilder vacBuilder =
            new VtnSetVlanPcpActionCaseBuilder();

        for (Integer order: orders) {
            for (short pcp = 0; pcp <= 7; pcp++) {
                VTNSetVlanPcpAction va;
                String expected;
                if (order == null) {
                    va = new VTNSetVlanPcpAction(pcp);
                    expected = "VTNSetVlanPcpAction[pcp=" + pcp + "]";
                } else {
                    VtnSetVlanPcpAction vact = new VtnSetVlanPcpActionBuilder().
                        setVlanPcp(new VlanPcp(pcp)).build();
                    VtnSetVlanPcpActionCase vac = vacBuilder.
                        setVtnSetVlanPcpAction(vact).build();
                    va = new VTNSetVlanPcpAction(vac, order);
                    expected = "VTNSetVlanPcpAction[pcp=" + pcp +
                        ",order=" + order + "]";
                }
                assertEquals(expected, va.toString());
            }
        }
    }

    /**
     * Test case for {@link VTNSetVlanPcpAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        short pcp = 7;
        VtnSetVlanPcpAction vact = new VtnSetVlanPcpActionBuilder().
            setVlanPcp(new VlanPcp(pcp)).build();
        VtnSetVlanPcpActionCaseBuilder vacBuilder =
            new VtnSetVlanPcpActionCaseBuilder();
        VtnSetVlanPcpActionCase vac = vacBuilder.
            setVtnSetVlanPcpAction(vact).build();
        VTNSetVlanPcpAction va = new VTNSetVlanPcpAction(vac, order);

        FlowActionContext ctx = mock(FlowActionContext.class);
        EtherHeader ether = mock(EtherHeader.class);
        when(ctx.getEtherHeader()).thenReturn(ether);

        assertEquals(true, va.apply(ctx));
        verify(ctx).getEtherHeader();
        verify(ctx).addFilterAction(va);

        verify(ether).setVlanPriority(pcp);
        verifyNoMoreInteractions(ctx, ether);
    }

    /**
     * Ensure that {@link VTNSetVlanPcpAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNSetVlanPcpAction.class,
                           FlowFilterAction.class);

        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetVlanPcpActionCaseBuilder vacBuilder =
            new VtnSetVlanPcpActionCaseBuilder();
        Class<VTNSetVlanPcpAction> type = VTNSetVlanPcpAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (short pcp = 0; pcp <= 7; pcp++) {
                VtnSetVlanPcpAction vact = new VtnSetVlanPcpActionBuilder().
                    setVlanPcp(new VlanPcp(pcp)).build();
                VtnSetVlanPcpActionCase vac = vacBuilder.
                    setVtnSetVlanPcpAction(vact).build();
                for (Integer order: orders) {
                    VTNSetVlanPcpAction va =
                        new VTNSetVlanPcpAction(vac, order);
                    String xml = marshal(m, va, type, XML_ROOT);
                    VTNSetVlanPcpAction va1 = unmarshal(um, xml, type);
                    va1.verify();
                    assertEquals(order, va1.getIdentifier());
                    assertEquals(pcp, va1.getPriority());
                    assertEquals(va, va1);
                }
            }

            // Default priority test.
            for (Integer order: orders) {
                String xml = new XmlNode(XML_ROOT).
                    add(new XmlNode("order", order)).toString();
                VTNSetVlanPcpAction va = unmarshal(um, xml, type);
                va.verify();
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getPriority());
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = createUnmarshaller(type);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetVlanPcpAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("priority", (short)7)).toString();
        VTNSetVlanPcpAction va = unmarshal(um, xml, type);
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
            8, 9, 10, 0xa0, 0xc00, 30000, 32000, Short.MAX_VALUE,
        };

        for (Short pcp: invalid) {
            emsg = "VTNSetVlanPcpAction: Invalid VLAN priority: " + pcp;
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("order", 1)).
                add(new XmlNode("priority", pcp)).toString();
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
