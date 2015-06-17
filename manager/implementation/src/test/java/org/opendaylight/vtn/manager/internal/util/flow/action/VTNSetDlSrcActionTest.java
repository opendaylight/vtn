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
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.src.action._case.VtnSetDlSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.src.action._case.VtnSetDlSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.src.action._case.SetDlSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.src.action._case.SetDlSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link VTNSetDlSrcAction}.
 */
public class VTNSetDlSrcActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetDlSrcAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-dl-src-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetDlSrcAction} instance.
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
            new XmlValueType("address",
                             EtherAddress.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNSetDlSrcAction#VTNSetDlSrcAction(EtherAddress)}</li>
     *   <li>{@link VTNSetDlSrcAction#VTNSetDlSrcAction(org.opendaylight.vtn.manager.flow.action.SetDlSrcAction, int)}</li>
     *   <li>{@link VTNSetDlSrcAction#VTNSetDlSrcAction(VtnSetDlSrcActionCase, Integer)}</li>
     *   <li>{@link VTNSetDlSrcAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetDlSrcAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetDlSrcAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNSetDlSrcAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNDlAddrAction#getAddress()}</li>
     *   <li>{@link VTNDlAddrAction#getMacAddress()}</li>
     *   <li>{@link VTNDlAddrAction#verifyImpl()}</li>
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
        EtherAddress[] addresses = {
            new EtherAddress(0x001122334455L),
            new EtherAddress(0xfcaabbccddeeL),
            new EtherAddress(0x000011112222L),
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetDlSrcActionCaseBuilder vacBuilder =
            new VtnSetDlSrcActionCaseBuilder();
        org.opendaylight.vtn.manager.flow.action.SetDlSrcAction vad;
        for (Integer order: orders) {
            for (EtherAddress eaddr: addresses) {
                MacAddress mac = eaddr.getMacAddress();
                vad = new org.opendaylight.vtn.manager.flow.action.
                    SetDlSrcAction(eaddr);
                VtnSetDlSrcAction vact = new VtnSetDlSrcActionBuilder().
                    setAddress(mac).build();
                VtnSetDlSrcActionCase vac = vacBuilder.
                    setVtnSetDlSrcAction(vact).build();
                SetDlSrcAction ma = new SetDlSrcActionBuilder().
                    setAddress(mac).build();
                SetDlSrcActionCase mact = new SetDlSrcActionCaseBuilder().
                    setSetDlSrcAction(ma).build();

                VTNSetDlSrcAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetDlSrcAction(eaddr);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetDlSrcAction(vad, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals(eaddr, va.getAddress());
                    assertEquals(mac, va.getMacAddress());

                    va = new VTNSetDlSrcAction(vac, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(eaddr, va.getAddress());
                assertEquals(mac, va.getMacAddress());

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
        }

        VTNSetDlSrcAction va = new VTNSetDlSrcAction();
        for (EtherAddress eaddr: addresses) {
            // toFlowAction() test.
            MacAddress mac = eaddr.getMacAddress();
            vad = new org.opendaylight.vtn.manager.flow.action.
                SetDlSrcAction(eaddr);
            VtnSetDlSrcAction vact = new VtnSetDlSrcActionBuilder().
                setAddress(mac).build();
            VtnAction vaction = vacBuilder.
                setVtnSetDlSrcAction(vact).build();
            assertEquals(vad, va.toFlowAction(vaction));

            vaction = VTNSetDlDstAction.newVtnAction(mac);
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            StatusCode ecode = StatusCode.BADREQUEST;
            String emsg = "VTNSetDlSrcAction: Unexpected type: " + vaction;
            try {
                va.toFlowAction(vaction);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            etag = RpcErrorTag.MISSING_ELEMENT;
            vaction = vacBuilder.
                setVtnSetDlSrcAction(null).build();
            emsg = "VTNSetDlSrcAction: No MAC address: " + vaction;
            try {
                va.toFlowAction(vaction);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            vaction = vacBuilder.
                setVtnSetDlSrcAction(new VtnSetDlSrcActionBuilder().build()).
                build();
            emsg = "VTNSetDlSrcAction: No MAC address: " + vaction;
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
            assertEquals(null, va.getAddress());

            // toVtnAction() test.
            SetDlSrcAction ma = new SetDlSrcActionBuilder().
                setAddress(mac).build();
            Action action = new SetDlSrcActionCaseBuilder().
                setSetDlSrcAction(ma).build();
            vaction = vacBuilder.setVtnSetDlSrcAction(vact).build();
            assertEquals(vaction, va.toVtnAction(action));

            etag = RpcErrorTag.BAD_ELEMENT;
            action = new SetDlDstActionCaseBuilder().build();
            emsg = "VTNSetDlSrcAction: Unexpected type: " + action;
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
            action = new SetDlSrcActionCaseBuilder().build();
            emsg = "VTNSetDlSrcAction: No MAC address: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            action = new SetDlSrcActionCaseBuilder().
                setSetDlSrcAction(new SetDlSrcActionBuilder().build()).
                build();
            emsg = "VTNSetDlSrcAction: No MAC address: " + action;
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
            assertEquals(null, va.getAddress());

            // getDescription() test.
            action = new SetDlSrcActionCaseBuilder().
                setSetDlSrcAction(ma).build();
            String desc = "SET_DL_SRC(address=" + eaddr.getText() + ")";
            assertEquals(desc, va.getDescription(action));

            // getDescription() should never affect instance variables.
            assertEquals(null, va.getAddress());
        }

        Action action = new SetDlSrcActionCaseBuilder().build();
        String desc = "SET_DL_SRC(address=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        action = new SetDlSrcActionCaseBuilder().
            setSetDlSrcAction(new SetDlSrcActionBuilder().build()).build();
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        EtherAddress eaddr = new EtherAddress(1L);
        String emsg = "VTNSetDlSrcAction: Action order cannot be null";
        VtnSetDlSrcAction vact = new VtnSetDlSrcActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        VtnSetDlSrcActionCase vac = vacBuilder.
            setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Null MAC address.
        emsg = "VTNSetDlSrcAction: MAC address cannot be null";
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlSrcAction((EtherAddress)null);
        try {
            new VTNSetDlSrcAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetDlSrcActionBuilder().build();
        vac = vacBuilder.setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vac = vacBuilder.setVtnSetDlSrcAction(null).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Broken MAC address.
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlSrcAction(new byte[]{0x00, 0x01});
        etag = RpcErrorTag.BAD_ELEMENT;
        emsg = "Invalid address: 00:01: Invalid byte array length: 2";
        try {
            new VTNSetDlSrcAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Broadcast MAC address.
        eaddr = EtherAddress.BROADCAST;
        emsg = "VTNSetDlSrcAction: Broadcast address cannot be specified.";
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlSrcAction(eaddr);
        try {
            new VTNSetDlSrcAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetDlSrcActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Multicast MAC address.
        eaddr = new EtherAddress(0x010000000000L);
        emsg = "VTNSetDlSrcAction: Multicast address cannot be specified: " +
            eaddr.getText();
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlSrcAction(eaddr);
        try {
            new VTNSetDlSrcAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetDlSrcActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Zero MAC address.
        eaddr = new EtherAddress(0L);
        emsg = "VTNSetDlSrcAction: Zero cannot be specified.";
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlSrcAction(eaddr);
        try {
            new VTNSetDlSrcAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetDlSrcActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }
    }

    /**
     * Test case for {@link VTNSetDlSrcAction#newVtnAction(MacAddress)}.
     */
    @Test
    public void testNewVtnAction() {
        MacAddress[] maddrs = {
            null,
            new MacAddress("00:11:22:33:44:55"),
            new MacAddress("0a:bc:de:f0:12:34"),
        };

        for (MacAddress mac: maddrs) {
            VtnSetDlSrcActionCase ac = VTNSetDlSrcAction.newVtnAction(mac);
            VtnSetDlSrcAction vaction = ac.getVtnSetDlSrcAction();
            assertEquals(mac, vaction.getAddress());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNSetDlSrcAction#equals(Object)}</li>
     *   <li>{@link VTNSetDlSrcAction#hashCode()}</li>
     *   <li>{@link VTNSetDlDstAction#equals(Object)}</li>
     *   <li>{@link VTNSetDlDstAction#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        EtherAddress[] addresses = {
            new EtherAddress(0x001122334455L),
            new EtherAddress(0xfcaabbccddeeL),
            new EtherAddress(0x000011112222L),
            new EtherAddress(0x000111112222L),
        };
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (EtherAddress eaddr: addresses) {
            EtherAddress eaddr1 = new EtherAddress(eaddr.getAddress());
            VTNSetDlSrcAction vsrc1 = new VTNSetDlSrcAction(eaddr);
            VTNSetDlSrcAction vsrc2 = new VTNSetDlSrcAction(eaddr1);
            VTNSetDlDstAction vdst1 = new VTNSetDlDstAction(eaddr);
            VTNSetDlDstAction vdst2 = new VTNSetDlDstAction(eaddr1);

            // VTNSetDlSrcAction should be distinguished from
            // VTNSetDlDstAction.
            testEquals(set, vsrc1, vsrc2);
            testEquals(set, vdst1, vdst2);

            for (Integer order: orders) {
                MacAddress mac1 = eaddr.getMacAddress();
                MacAddress mac2 = eaddr1.getMacAddress();
                VtnSetDlSrcActionCase src1 =
                    VTNSetDlSrcAction.newVtnAction(mac1);
                VtnSetDlSrcActionCase src2 =
                    VTNSetDlSrcAction.newVtnAction(mac2);
                vsrc1 = new VTNSetDlSrcAction(src1, order);
                vsrc2 = new VTNSetDlSrcAction(src2, order);

                VtnSetDlDstActionCase dst1 =
                    VTNSetDlDstAction.newVtnAction(mac1);
                VtnSetDlDstActionCase dst2 =
                    VTNSetDlDstAction.newVtnAction(mac2);
                vdst1 = new VTNSetDlDstAction(dst1, order);
                vdst2 = new VTNSetDlDstAction(dst2, order);

                // VTNSetDlSrcAction should be distinguished from
                // VTNSetDlDstAction.
                testEquals(set, vsrc1, vsrc2);
                testEquals(set, vdst1, vdst2);
            }
        }

        int expected = (orders.length + 1) * addresses.length * 2;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNSetDlSrcAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        Integer order = 10;
        EtherAddress eaddr = new EtherAddress(0x123L);
        VtnSetDlSrcActionCase vac = VTNSetDlSrcAction.
            newVtnAction(eaddr.getMacAddress());
        VTNSetDlSrcAction va = new VTNSetDlSrcAction(vac, order);
        String expected = "VTNSetDlSrcAction[addr=00:00:00:00:01:23,order=10]";
        assertEquals(expected, va.toString());

        order = 123;
        eaddr = new EtherAddress(0xf8ffffffabcdL);
        vac = VTNSetDlSrcAction.newVtnAction(eaddr.getMacAddress());
        va = new VTNSetDlSrcAction(vac, order);
        expected = "VTNSetDlSrcAction[addr=f8:ff:ff:ff:ab:cd,order=123]";
        assertEquals(expected, va.toString());

        va = new VTNSetDlSrcAction(eaddr);
        expected = "VTNSetDlSrcAction[addr=f8:ff:ff:ff:ab:cd]";
        assertEquals(expected, va.toString());
    }

    /**
     * Test case for {@link VTNSetDlSrcAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        EtherAddress eaddr = new EtherAddress(0x12345678L);
        VtnSetDlSrcActionCase vac = VTNSetDlSrcAction.
            newVtnAction(eaddr.getMacAddress());
        VTNSetDlSrcAction va = new VTNSetDlSrcAction(vac, order);

        FlowActionContext ctx = Mockito.mock(FlowActionContext.class);
        EtherHeader ether = Mockito.mock(EtherHeader.class);
        Mockito.when(ctx.getEtherHeader()).thenReturn(ether);

        assertEquals(true, va.apply(ctx));
        Mockito.verify(ctx).getEtherHeader();
        Mockito.verify(ctx).addFilterAction(va);
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();

        Mockito.verify(ether).setSourceAddress(eaddr);
        Mockito.verify(ether, Mockito.never()).getSourceAddress();
        Mockito.verify(ether, Mockito.never()).getDestinationAddress();
        Mockito.verify(ether, Mockito.never()).
            setDestinationAddress(Mockito.any(EtherAddress.class));
        Mockito.verify(ether, Mockito.never()).getEtherType();
        Mockito.verify(ether, Mockito.never()).getVlanId();
        Mockito.verify(ether, Mockito.never()).setVlanId(Mockito.anyInt());
        Mockito.verify(ether, Mockito.never()).getVlanPriority();
        Mockito.verify(ether, Mockito.never()).
            setVlanPriority(Mockito.anyShort());
    }

    /**
     * Ensure that {@link VTNSetDlSrcAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Unmarshaller[] unmarshallers = {
            createUnmarshaller(VTNSetDlSrcAction.class),
            createUnmarshaller(VTNDlAddrAction.class),
            createUnmarshaller(FlowFilterAction.class),
        };

        EtherAddress[] addresses = {
            new EtherAddress(0x001122334455L),
            new EtherAddress(0xfcaabbccddeeL),
            new EtherAddress(0x000011112222L),
            new EtherAddress(0x000111112222L),
        };
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        Class<VTNSetDlSrcAction> type = VTNSetDlSrcAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Unmarshaller um: unmarshallers) {
            for (Integer order: orders) {
                for (EtherAddress eaddr: addresses) {
                    String xml = new XmlNode(XML_ROOT).
                        add(new XmlNode("order", order)).
                        add(new XmlNode("address", eaddr.getText())).
                        toString();
                    VTNSetDlSrcAction va = unmarshal(um, xml, type);
                    va.verify();
                    assertEquals(order, va.getIdentifier());
                    assertEquals(eaddr, va.getAddress());
                }
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = unmarshallers[0];
        EtherAddress eaddr = addresses[0];
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNSetDlSrcAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("address", eaddr.getText())).toString();
        VTNSetDlSrcAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // No MAC address.
        Integer order = 1;
        emsg = "VTNSetDlSrcAction: MAC address cannot be null";
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("order", order)).toString();
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

        Map<EtherAddress, String> cases = new HashMap<>();
        etag = RpcErrorTag.BAD_ELEMENT;

        // Broadcast MAC address.
        cases.put(EtherAddress.BROADCAST,
                  "VTNSetDlSrcAction: Broadcast address cannot be specified.");

        // Multicast MAC address.
        eaddr = new EtherAddress(0x010000000000L);
        cases.put(eaddr,
                  "VTNSetDlSrcAction: Multicast address cannot be specified: " +
                  eaddr.getText());

        // Zero MAC address.
        cases.put(new EtherAddress(0L),
                  "VTNSetDlSrcAction: Zero cannot be specified.");

        for (Map.Entry<EtherAddress, String> entry: cases.entrySet()) {
            eaddr = entry.getKey();
            emsg = entry.getValue();
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("order", order)).
                add(new XmlNode("address", eaddr.getText())).toString();
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
