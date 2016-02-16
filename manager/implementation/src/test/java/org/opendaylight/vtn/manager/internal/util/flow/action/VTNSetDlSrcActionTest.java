/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
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
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.src.action._case.VtnSetDlSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.src.action._case.VtnSetDlSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

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
     *   <li>{@link VTNSetDlSrcAction#VTNSetDlSrcAction(VtnSetDlSrcActionCase, Integer)}</li>
     *   <li>{@link VTNSetDlSrcAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetDlSrcAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetDlSrcAction#toFlowFilterAction(VtnAction,Integer)}</li>
     *   <li>{@link VTNSetDlSrcAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNDlAddrAction#getAddress()}</li>
     *   <li>{@link VTNDlAddrAction#getMacAddress()}</li>
     *   <li>{@link VTNDlAddrAction#verifyImpl()}</li>
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
        for (Integer order: orders) {
            for (EtherAddress eaddr: addresses) {
                MacAddress mac = eaddr.getMacAddress();
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
                    va = new VTNSetDlSrcAction(vac, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(eaddr, va.getAddress());
                assertEquals(mac, va.getMacAddress());

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
                    VTNSetDlSrcAction conv = new VTNSetDlSrcAction();
                    assertEquals(va, conv.toFlowFilterAction(vac, order));

                    // toFlowFilterAction() should never affect instance
                    // variables.
                    assertEquals(null, conv.getAddress());
                }
            }
        }

        VTNSetDlSrcAction va = new VTNSetDlSrcAction();
        for (EtherAddress eaddr: addresses) {
            // toVtnAction() test.
            MacAddress mac = eaddr.getMacAddress();
            VtnSetDlSrcAction vact = new VtnSetDlSrcActionBuilder().
                setAddress(mac).build();
            VtnAction vaction = vacBuilder.
                setVtnSetDlSrcAction(vact).build();
            SetDlSrcAction ma = new SetDlSrcActionBuilder().
                setAddress(mac).build();
            Action action = new SetDlSrcActionCaseBuilder().
                setSetDlSrcAction(ma).build();
            assertEquals(vaction, va.toVtnAction(action));

            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            action = new SetDlDstActionCaseBuilder().build();
            String emsg = "VTNSetDlSrcAction: Unexpected type: " + action;
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
            action = new SetDlSrcActionCaseBuilder().build();
            emsg = "VTNSetDlSrcAction: No MAC address: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
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
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
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
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Null MAC address.
        emsg = "VTNSetDlSrcAction: MAC address cannot be null";
        vact = new VtnSetDlSrcActionBuilder().build();
        vac = vacBuilder.setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        vac = vacBuilder.setVtnSetDlSrcAction(null).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Broadcast MAC address.
        etag = RpcErrorTag.BAD_ELEMENT;
        eaddr = EtherAddress.BROADCAST;
        emsg = "VTNSetDlSrcAction: Broadcast address cannot be specified.";
        vact = new VtnSetDlSrcActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Multicast MAC address.
        eaddr = new EtherAddress(0x010000000000L);
        emsg = "VTNSetDlSrcAction: Multicast address cannot be specified: " +
            eaddr.getText();
        vact = new VtnSetDlSrcActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Zero MAC address.
        eaddr = new EtherAddress(0L);
        emsg = "VTNSetDlSrcAction: Zero cannot be specified.";
        vact = new VtnSetDlSrcActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlSrcAction(vact).build();
        try {
            new VTNSetDlSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
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

        // Create empty instances using JAXB.
        Unmarshaller um = createUnmarshaller(VTNDlAddrAction.class);
        String xml = new XmlNode(XML_ROOT).toString();
        Class<VTNSetDlSrcAction> srcType = VTNSetDlSrcAction.class;
        VTNSetDlSrcAction vsrc1 = unmarshal(um, xml, srcType);
        VTNSetDlSrcAction vsrc2 = unmarshal(um, xml, srcType);
        testEquals(set, vsrc1, vsrc2);

        xml = new XmlNode("vtn-set-dl-dst-action").toString();
        Class<VTNSetDlDstAction> dstType = VTNSetDlDstAction.class;
        VTNSetDlDstAction vdst1 = unmarshal(um, xml, dstType);
        VTNSetDlDstAction vdst2 = unmarshal(um, xml, dstType);
        testEquals(set, vdst1, vdst2);

        int expected = ((orders.length + 1) * addresses.length * 2) + 2;
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

        FlowActionContext ctx = mock(FlowActionContext.class);
        EtherHeader ether = mock(EtherHeader.class);
        when(ctx.getEtherHeader()).thenReturn(ether);

        assertEquals(true, va.apply(ctx));
        verify(ctx).getEtherHeader();
        verify(ctx).addFilterAction(va);

        verify(ether).setSourceAddress(eaddr);
        verifyNoMoreInteractions(ctx, ether);
    }

    /**
     * Ensure that {@link VTNSetDlSrcAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNSetDlSrcAction.class,
                           VTNDlAddrAction.class, FlowFilterAction.class);

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

        VtnSetDlSrcActionCaseBuilder vacBuilder =
            new VtnSetDlSrcActionCaseBuilder();
        Class<VTNSetDlSrcAction> type = VTNSetDlSrcAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (EtherAddress eaddr: addresses) {
                VtnSetDlSrcAction vact = new VtnSetDlSrcActionBuilder().
                    setAddress(eaddr.getMacAddress()).build();
                VtnSetDlSrcActionCase vac = vacBuilder.
                    setVtnSetDlSrcAction(vact).build();
                for (Integer order: orders) {
                    VTNSetDlSrcAction va = new VTNSetDlSrcAction(vac, order);
                    String xml = marshal(m, va, type, XML_ROOT);
                    VTNSetDlSrcAction va1 = unmarshal(um, xml, type);
                    va1.verify();
                    assertEquals(order, va1.getIdentifier());
                    assertEquals(eaddr, va1.getAddress());
                    assertEquals(va, va1);
                }
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = createUnmarshaller(type);
        EtherAddress eaddr = addresses[0];
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetDlSrcAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("address", eaddr.getText())).toString();
        VTNSetDlSrcAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
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
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }
}
