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
import java.util.HashMap;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.dst.action._case.VtnSetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.dst.action._case.VtnSetDlDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link VTNSetDlDstAction}.
 */
public class VTNSetDlDstActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetDlDstAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-dl-dst-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetDlDstAction} instance.
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
     *   <li>{@link VTNSetDlDstAction#VTNSetDlDstAction(EtherAddress)}</li>
     *   <li>{@link VTNSetDlDstAction#VTNSetDlDstAction(org.opendaylight.vtn.manager.flow.action.SetDlDstAction, int)}</li>
     *   <li>{@link VTNSetDlDstAction#VTNSetDlDstAction(VtnSetDlDstActionCase, Integer)}</li>
     *   <li>{@link VTNSetDlDstAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetDlDstAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetDlDstAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNSetDlDstAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetDlDstAction#getDescription(Action)}</li>
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

        VtnSetDlDstActionCaseBuilder vacBuilder =
            new VtnSetDlDstActionCaseBuilder();
        org.opendaylight.vtn.manager.flow.action.SetDlDstAction vad;
        for (Integer order: orders) {
            for (EtherAddress eaddr: addresses) {
                MacAddress mac = eaddr.getMacAddress();
                vad = new org.opendaylight.vtn.manager.flow.action.
                    SetDlDstAction(eaddr);
                VtnSetDlDstAction vact = new VtnSetDlDstActionBuilder().
                    setAddress(mac).build();
                VtnSetDlDstActionCase vac = vacBuilder.
                    setVtnSetDlDstAction(vact).build();
                SetDlDstAction ma = new SetDlDstActionBuilder().
                    setAddress(mac).build();
                SetDlDstActionCase mact = new SetDlDstActionCaseBuilder().
                    setSetDlDstAction(ma).build();

                VTNSetDlDstAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetDlDstAction(eaddr);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetDlDstAction(vad, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals(eaddr, va.getAddress());
                    assertEquals(mac, va.getMacAddress());

                    va = new VTNSetDlDstAction(vac, order);
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

        VTNSetDlDstAction va = new VTNSetDlDstAction();
        for (EtherAddress eaddr: addresses) {
            // toFlowAction() test.
            MacAddress mac = eaddr.getMacAddress();
            vad = new org.opendaylight.vtn.manager.flow.action.
                SetDlDstAction(eaddr);
            VtnSetDlDstAction vact = new VtnSetDlDstActionBuilder().
                setAddress(mac).build();
            VtnAction vaction = vacBuilder.
                setVtnSetDlDstAction(vact).build();
            assertEquals(vad, va.toFlowAction(vaction));

            vaction = VTNSetDlSrcAction.newVtnAction(mac);
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            StatusCode ecode = StatusCode.BADREQUEST;
            String emsg = "VTNSetDlDstAction: Unexpected type: " + vaction;
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
                setVtnSetDlDstAction(null).build();
            emsg = "VTNSetDlDstAction: No MAC address: " + vaction;
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
                setVtnSetDlDstAction(new VtnSetDlDstActionBuilder().build()).
                build();
            emsg = "VTNSetDlDstAction: No MAC address: " + vaction;
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
            SetDlDstAction ma = new SetDlDstActionBuilder().
                setAddress(mac).build();
            Action action = new SetDlDstActionCaseBuilder().
                setSetDlDstAction(ma).build();
            vaction = vacBuilder.setVtnSetDlDstAction(vact).build();
            assertEquals(vaction, va.toVtnAction(action));

            etag = RpcErrorTag.BAD_ELEMENT;
            action = new SetDlSrcActionCaseBuilder().build();
            emsg = "VTNSetDlDstAction: Unexpected type: " + action;
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
            action = new SetDlDstActionCaseBuilder().build();
            emsg = "VTNSetDlDstAction: No MAC address: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            action = new SetDlDstActionCaseBuilder().
                setSetDlDstAction(new SetDlDstActionBuilder().build()).
                build();
            emsg = "VTNSetDlDstAction: No MAC address: " + action;
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
            action = new SetDlDstActionCaseBuilder().
                setSetDlDstAction(ma).build();
            String desc = "SET_DL_DST(address=" + eaddr.getText() + ")";
            assertEquals(desc, va.getDescription(action));

            // getDescription() should never affect instance variables.
            assertEquals(null, va.getAddress());
        }

        Action action = new SetDlDstActionCaseBuilder().build();
        String desc = "SET_DL_DST(address=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        action = new SetDlDstActionCaseBuilder().
            setSetDlDstAction(new SetDlDstActionBuilder().build()).build();
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        EtherAddress eaddr = new EtherAddress(1L);
        String emsg = "VTNSetDlDstAction: Action order cannot be null";
        VtnSetDlDstAction vact = new VtnSetDlDstActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        VtnSetDlDstActionCase vac = vacBuilder.
            setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Null MAC address.
        emsg = "VTNSetDlDstAction: MAC address cannot be null";
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlDstAction((EtherAddress)null);
        try {
            new VTNSetDlDstAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetDlDstActionBuilder().build();
        vac = vacBuilder.setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vac = vacBuilder.setVtnSetDlDstAction(null).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Broken MAC address.
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlDstAction(new byte[]{0x00, 0x01});
        etag = RpcErrorTag.BAD_ELEMENT;
        emsg = "Invalid address: 00:01: Invalid byte array length: 2";
        try {
            new VTNSetDlDstAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Broadcast MAC address.
        eaddr = EtherAddress.BROADCAST;
        emsg = "VTNSetDlDstAction: Broadcast address cannot be specified.";
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlDstAction(eaddr);
        try {
            new VTNSetDlDstAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetDlDstActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Multicast MAC address.
        eaddr = new EtherAddress(0x010000000000L);
        emsg = "VTNSetDlDstAction: Multicast address cannot be specified: " +
            eaddr.getText();
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlDstAction(eaddr);
        try {
            new VTNSetDlDstAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetDlDstActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Zero MAC address.
        eaddr = new EtherAddress(0L);
        emsg = "VTNSetDlDstAction: Zero cannot be specified.";
        vad = new org.opendaylight.vtn.manager.flow.action.
            SetDlDstAction(eaddr);
        try {
            new VTNSetDlDstAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetDlDstActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }
    }

    /**
     * Test case for {@link VTNSetDlDstAction#newVtnAction(MacAddress)}.
     */
    @Test
    public void testNewVtnAction() {
        MacAddress[] maddrs = {
            null,
            new MacAddress("00:11:22:33:44:55"),
            new MacAddress("0a:bc:de:f0:12:34"),
        };

        for (MacAddress mac: maddrs) {
            VtnSetDlDstActionCase ac = VTNSetDlDstAction.newVtnAction(mac);
            VtnSetDlDstAction vaction = ac.getVtnSetDlDstAction();
            assertEquals(mac, vaction.getAddress());
        }
    }

    /**
     * Test case for {@link VTNSetDlDstAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        Integer order = 10;
        EtherAddress eaddr = new EtherAddress(0x123L);
        VtnSetDlDstActionCase vac = VTNSetDlDstAction.
            newVtnAction(eaddr.getMacAddress());
        VTNSetDlDstAction va = new VTNSetDlDstAction(vac, order);
        String expected = "VTNSetDlDstAction[addr=00:00:00:00:01:23,order=10]";
        assertEquals(expected, va.toString());

        order = 123;
        eaddr = new EtherAddress(0xf8ffffffabcdL);
        vac = VTNSetDlDstAction.newVtnAction(eaddr.getMacAddress());
        va = new VTNSetDlDstAction(vac, order);
        expected = "VTNSetDlDstAction[addr=f8:ff:ff:ff:ab:cd,order=123]";
        assertEquals(expected, va.toString());

        va = new VTNSetDlDstAction(eaddr);
        expected = "VTNSetDlDstAction[addr=f8:ff:ff:ff:ab:cd]";
        assertEquals(expected, va.toString());
    }

    /**
     * Test case for {@link VTNSetDlDstAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        EtherAddress eaddr = new EtherAddress(0x12345678L);
        VtnSetDlDstActionCase vac = VTNSetDlDstAction.
            newVtnAction(eaddr.getMacAddress());
        VTNSetDlDstAction va = new VTNSetDlDstAction(vac, order);

        FlowActionContext ctx = Mockito.mock(FlowActionContext.class);
        EtherHeader ether = Mockito.mock(EtherHeader.class);
        Mockito.when(ctx.getEtherHeader()).thenReturn(ether);

        assertEquals(true, va.apply(ctx));
        Mockito.verify(ctx).getEtherHeader();
        Mockito.verify(ctx).addFilterAction(va);
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();

        Mockito.verify(ether).setDestinationAddress(eaddr);
        Mockito.verify(ether, Mockito.never()).getSourceAddress();
        Mockito.verify(ether, Mockito.never()).
            setSourceAddress(Mockito.any(EtherAddress.class));
        Mockito.verify(ether, Mockito.never()).getDestinationAddress();
        Mockito.verify(ether, Mockito.never()).getEtherType();
        Mockito.verify(ether, Mockito.never()).getVlanId();
        Mockito.verify(ether, Mockito.never()).setVlanId(Mockito.anyInt());
        Mockito.verify(ether, Mockito.never()).getVlanPriority();
        Mockito.verify(ether, Mockito.never()).
            setVlanPriority(Mockito.anyShort());
    }

    /**
     * Ensure that {@link VTNSetDlDstAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Unmarshaller[] unmarshallers = {
            createUnmarshaller(VTNSetDlDstAction.class),
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

        Class<VTNSetDlDstAction> type = VTNSetDlDstAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Unmarshaller um: unmarshallers) {
            for (Integer order: orders) {
                for (EtherAddress eaddr: addresses) {
                    String xml = new XmlNode(XML_ROOT).
                        add(new XmlNode("order", order)).
                        add(new XmlNode("address", eaddr.getText())).
                        toString();
                    VTNSetDlDstAction va = unmarshal(um, xml, type);
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
        String emsg = "VTNSetDlDstAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("address", eaddr.getText())).toString();
        VTNSetDlDstAction va = unmarshal(um, xml, type);
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
        emsg = "VTNSetDlDstAction: MAC address cannot be null";
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
                  "VTNSetDlDstAction: Broadcast address cannot be specified.");

        // Multicast MAC address.
        eaddr = new EtherAddress(0x010000000000L);
        cases.put(eaddr,
                  "VTNSetDlDstAction: Multicast address cannot be specified: " +
                  eaddr.getText());

        // Zero MAC address.
        cases.put(new EtherAddress(0L),
                  "VTNSetDlDstAction: Zero cannot be specified.");

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
