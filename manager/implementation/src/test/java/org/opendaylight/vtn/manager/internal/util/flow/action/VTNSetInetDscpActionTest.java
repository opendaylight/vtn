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

import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

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
     *   <li>{@link VTNSetInetDscpAction#VTNSetInetDscpAction(SetDscpAction, int)}</li>
     *   <li>{@link VTNSetInetDscpAction#VTNSetInetDscpAction(VtnSetInetDscpAction, Integer)}</li>
     *   <li>{@link VTNSetInetDscpAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetInetDscpAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetInetDscpAction#getDscp()}</li>
     *   <li>{@link VTNSetInetDscpAction#verifyImpl()}</li>
     *   <li>{@link VTNSetInetDscpAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNSetInetDscpAction#toVtnAction(Action)}</li>
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
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };
        short[] dscps = {
            0, 1, 2, 11, 22, 33, 44, 55, 60, 62, 63,
        };

        for (Integer order: orders) {
            for (short dscp: dscps) {
                Dscp mdscp = new Dscp(dscp);
                Integer tos = Integer.valueOf(ProtocolUtils.dscpToTos(dscp));
                SetDscpAction vad = new SetDscpAction((byte)dscp);
                VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
                    setDscp(mdscp).build();
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
                    va = new VTNSetInetDscpAction(vad, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals(dscp, va.getDscp());

                    va = new VTNSetInetDscpAction(vact, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(dscp, va.getDscp());

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
                // Default DSCP test.
                VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
                    build();
                VTNSetInetDscpAction va = new VTNSetInetDscpAction(vact, order);
                assertEquals(order, va.getIdentifier());
                assertEquals((short)0, va.getDscp());
            }
        }

        VTNSetInetDscpAction va = new VTNSetInetDscpAction();
        for (short dscp: dscps) {
            // toFlowAction() test.
            SetDscpAction vad = new SetDscpAction((byte)dscp);
            VtnAction vaction = new VtnSetInetDscpActionBuilder().
                setDscp(new Dscp(dscp)).build();
            assertEquals(vad, va.toFlowAction(vaction));

            vaction = new VtnSetIcmpTypeActionBuilder().setType(dscp).build();
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            StatusCode ecode = StatusCode.BADREQUEST;
            String emsg = "VTNSetInetDscpAction: Unexpected type: " + vaction;
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
            assertEquals(0, va.getDscp());

            // toVtnAction() test.
            Integer tos = Integer.valueOf(ProtocolUtils.dscpToTos(dscp));
            SetNwTosAction ma = new SetNwTosActionBuilder().
                setTos(tos).build();
            Action action = new SetNwTosActionCaseBuilder().
                setSetNwTosAction(ma).build();
            vaction = new VtnSetInetDscpActionBuilder().
                setDscp(new Dscp(dscp)).build();
            assertEquals(vaction, va.toVtnAction(action));

            action = new SetTpSrcActionCaseBuilder().build();
            emsg = "VTNSetInetDscpAction: Unexpected type: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            try {
                va.getDescription(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            etag = RpcErrorTag.MISSING_ELEMENT;
            action = new SetNwTosActionCaseBuilder().build();
            emsg = "VTNSetInetDscpAction: No DSCP value: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
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
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(0, va.getDscp());

            // getDescription() test.
            action = new SetNwTosActionCaseBuilder().
                setSetNwTosAction(ma).build();
            String desc = "SET_NW_TOS(tos=0x" +
                Integer.toHexString(tos.intValue()) + ")";
            assertEquals(desc, va.getDescription(action));

            // getDescription() should never affect instance variables.
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
        StatusCode ecode = StatusCode.BADREQUEST;
        Dscp mdscp = new Dscp((short)0);
        String emsg = "VTNSetInetDscpAction: Action order cannot be null";
        VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
            setDscp(mdscp).build();
        try {
            new VTNSetInetDscpAction(vact, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Invalid DSCP.
        etag = RpcErrorTag.BAD_ELEMENT;
        byte[] invalid = {
            Byte.MIN_VALUE, -100, -50, -3, -2, -1,
            64, 65, 70, 85, 99, 120, Byte.MAX_VALUE,
        };
        for (byte dscp: invalid) {
            SetDscpAction vad = new SetDscpAction(dscp);
            emsg = "VTNSetInetDscpAction: Invalid IP DSCP value: " + dscp;
            try {
                new VTNSetInetDscpAction(vad, 1);
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
                VtnSetInetDscpAction vact1 = new VtnSetInetDscpActionBuilder().
                    setDscp(new Dscp(dscp)).build();
                VtnSetInetDscpAction vact2 = new VtnSetInetDscpActionBuilder().
                    setDscp(new Dscp(dscp)).build();
                va1 = new VTNSetInetDscpAction(vact1, order);
                va2 = new VTNSetInetDscpAction(vact2, order);
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
                    VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
                        setDscp(new Dscp(dscp)).build();
                    va = new VTNSetInetDscpAction(vact, order);
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
        VtnSetInetDscpAction vact = new VtnSetInetDscpActionBuilder().
            setDscp(new Dscp(dscp)).build();
        VTNSetInetDscpAction va = new VTNSetInetDscpAction(vact, order);

        // In case of IP packet.
        FlowActionContext ctx = Mockito.mock(FlowActionContext.class);
        InetHeader inet = Mockito.mock(InetHeader.class);
        Mockito.when(ctx.getInetHeader()).thenReturn(inet);

        assertEquals(true, va.apply(ctx));
        Mockito.verify(ctx).getInetHeader();
        Mockito.verify(ctx).addFilterAction(va);
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();

        Mockito.verify(inet).setDscp(dscp);
        Mockito.verify(inet, Mockito.never()).getSourceAddress();
        Mockito.verify(inet, Mockito.never()).
            setSourceAddress(Mockito.any(IpNetwork.class));
        Mockito.verify(inet, Mockito.never()).getDestinationAddress();
        Mockito.verify(inet, Mockito.never()).
            setDestinationAddress(Mockito.any(IpNetwork.class));
        Mockito.verify(inet, Mockito.never()).getProtocol();
        Mockito.verify(inet, Mockito.never()).getDscp();

        // In case of non-IP packet.
        inet = null;
        Mockito.reset(ctx);
        Mockito.when(ctx.getInetHeader()).thenReturn(inet);

        assertEquals(false, va.apply(ctx));
        Mockito.verify(ctx).getInetHeader();
        Mockito.verify(ctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();
    }

    /**
     * Ensure that {@link VTNSetInetDscpAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Unmarshaller[] unmarshallers = {
            createUnmarshaller(VTNSetInetDscpAction.class),
            createUnmarshaller(FlowFilterAction.class),
        };

        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };
        short[] dscps = {
            0, 1, 2, 11, 22, 33, 44, 55, 60, 62, 63,
        };

        Class<VTNSetInetDscpAction> type = VTNSetInetDscpAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Unmarshaller um: unmarshallers) {
            for (Integer order: orders) {
                for (short dscp: dscps) {
                    String xml = new XmlNode(XML_ROOT).
                        add(new XmlNode("order", order)).
                        add(new XmlNode("dscp", dscp)).
                        toString();
                    VTNSetInetDscpAction va = unmarshal(um, xml, type);
                    va.verify();
                    assertEquals(order, va.getIdentifier());
                    assertEquals(dscp, va.getDscp());
                }

                // Default DSCP value test.
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
        Unmarshaller um = unmarshallers[0];
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNSetInetDscpAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("dscp", (short)0)).toString();
        VTNSetInetDscpAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
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
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }
        }
    }
}
