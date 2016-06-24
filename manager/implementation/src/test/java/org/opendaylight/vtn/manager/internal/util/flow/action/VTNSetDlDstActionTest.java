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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.dst.action._case.VtnSetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.dl.dst.action._case.VtnSetDlDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.dl.dst.action._case.SetDlDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.MacAddress;

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
     *   <li>{@link VTNSetDlDstAction#VTNSetDlDstAction(VtnSetDlDstActionCase, Integer)}</li>
     *   <li>{@link VTNSetDlDstAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetDlDstAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetDlDstAction#toFlowFilterAction(VtnAction,Integer)}</li>
     *   <li>{@link VTNSetDlDstAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetDlDstAction#getDescription(Action)}</li>
     *   <li>{@link VTNSetDlDstAction#getDescription(VtnAction)}</li>
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

        VtnSetDlDstActionCaseBuilder vacBuilder =
            new VtnSetDlDstActionCaseBuilder();
        for (Integer order: orders) {
            for (EtherAddress eaddr: addresses) {
                MacAddress mac = eaddr.getMacAddress();
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
                    va = new VTNSetDlDstAction(vac, order);
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
                    VTNSetDlDstAction conv = new VTNSetDlDstAction();
                    assertEquals(va, conv.toFlowFilterAction(vac, order));

                    // toFlowFilterAction() should never affect instance
                    // variables.
                    assertEquals(null, conv.getAddress());
                }
            }
        }

        VTNSetDlDstAction va = new VTNSetDlDstAction();
        for (EtherAddress eaddr: addresses) {
            // toVtnAction() test.
            MacAddress mac = eaddr.getMacAddress();
            VtnSetDlDstAction vact = new VtnSetDlDstActionBuilder().
                setAddress(mac).build();
            VtnAction vaction = vacBuilder.
                setVtnSetDlDstAction(vact).build();
            SetDlDstAction ma = new SetDlDstActionBuilder().
                setAddress(mac).build();
            Action action = new SetDlDstActionCaseBuilder().
                setSetDlDstAction(ma).build();
            assertEquals(vaction, va.toVtnAction(action));

            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            action = new SetDlSrcActionCaseBuilder().build();
            String emsg = "VTNSetDlDstAction: Unexpected type: " + action;
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
            action = new SetDlDstActionCaseBuilder().build();
            emsg = "VTNSetDlDstAction: No MAC address: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(null, va.getAddress());

            // getDescription(VtnAction) test.
            // It should never affect instance variables.
            String desc = "set-dl-dst(" + eaddr.getText() + ")";
            assertEquals(desc, va.getDescription(vaction));
            assertEquals(null, va.getAddress());
            VtnSetDlDstAction vact1 = new VtnSetDlDstActionBuilder().
                build();
            VtnAction vaction1 = new VtnSetDlDstActionCaseBuilder().
                setVtnSetDlDstAction(vact1).build();
            desc = "set-dl-dst(null)";
            assertEquals(desc, va.getDescription(vaction1));
            assertEquals(null, va.getAddress());

            // getDescription(Action) test.
            // It should never affect instance variables.
            action = new SetDlDstActionCaseBuilder().
                setSetDlDstAction(ma).build();
            desc = "SET_DL_DST(address=" + eaddr.getText() + ")";
            assertEquals(desc, va.getDescription(action));
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
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
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
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Null MAC address.
        emsg = "VTNSetDlDstAction: MAC address cannot be null";
        vact = new VtnSetDlDstActionBuilder().build();
        vac = vacBuilder.setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        vac = vacBuilder.setVtnSetDlDstAction(null).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Broadcast MAC address.
        etag = RpcErrorTag.BAD_ELEMENT;
        eaddr = EtherAddress.BROADCAST;
        emsg = "VTNSetDlDstAction: Broadcast address cannot be specified.";
        vact = new VtnSetDlDstActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Multicast MAC address.
        eaddr = new EtherAddress(0x010000000000L);
        emsg = "VTNSetDlDstAction: Multicast address cannot be specified: " +
            eaddr.getText();
        vact = new VtnSetDlDstActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Zero MAC address.
        eaddr = new EtherAddress(0L);
        emsg = "VTNSetDlDstAction: Zero cannot be specified.";
        vact = new VtnSetDlDstActionBuilder().
            setAddress(eaddr.getMacAddress()).build();
        vac = vacBuilder.setVtnSetDlDstAction(vact).build();
        try {
            new VTNSetDlDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
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

        FlowActionContext ctx = mock(FlowActionContext.class);
        EtherHeader ether = mock(EtherHeader.class);
        when(ctx.getEtherHeader()).thenReturn(ether);

        assertEquals(true, va.apply(ctx));
        verify(ctx).getEtherHeader();
        verify(ctx).addFilterAction(va);

        verify(ether).setDestinationAddress(eaddr);
        verifyNoMoreInteractions(ctx, ether);
    }

    /**
     * Ensure that {@link VTNSetDlDstAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNSetDlDstAction.class,
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

        VtnSetDlDstActionCaseBuilder vacBuilder =
            new VtnSetDlDstActionCaseBuilder();
        Class<VTNSetDlDstAction> type = VTNSetDlDstAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (EtherAddress eaddr: addresses) {
                VtnSetDlDstAction vact = new VtnSetDlDstActionBuilder().
                    setAddress(eaddr.getMacAddress()).build();
                VtnSetDlDstActionCase vac = vacBuilder.
                    setVtnSetDlDstAction(vact).build();
                for (Integer order: orders) {
                    VTNSetDlDstAction va = new VTNSetDlDstAction(vac, order);
                    String xml = marshal(m, va, type, XML_ROOT);
                    VTNSetDlDstAction va1 = unmarshal(um, xml, type);
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
        String emsg = "VTNSetDlDstAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("address", eaddr.getText())).toString();
        VTNSetDlDstAction va = unmarshal(um, xml, type);
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
        emsg = "VTNSetDlDstAction: MAC address cannot be null";
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
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }
}
