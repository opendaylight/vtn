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

import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dst.action._case.VtnSetInetDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.dst.action._case.VtnSetInetDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.dst.action._case.SetNwDstAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.dst.action._case.SetNwDstActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4Builder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv6Builder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev130715.Ipv4Prefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev130715.Ipv6Prefix;

/**
 * JUnit test for {@link VTNSetInetDstAction}.
 */
public class VTNSetInetDstActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetInetDstAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-inet-dst-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetInetDstAction} instance.
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
            new XmlValueType("ipv4-address",
                             Ip4Network.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNSetInetDstAction#VTNSetInetDstAction(IpNetwork)}</li>
     *   <li>{@link VTNSetInetDstAction#VTNSetInetDstAction(VtnSetInetDstActionCase, Integer)}</li>
     *   <li>{@link VTNSetInetDstAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetInetDstAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetInetDstAction#toFlowFilterAction(VtnAction,Integer)}</li>
     *   <li>{@link VTNSetInetDstAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetInetDstAction#getDescription(Action)}</li>
     *   <li>{@link VTNSetInetDstAction#getDescription(VtnAction)}</li>
     *   <li>{@link VTNInetAddrAction#getAddress()}</li>
     *   <li>{@link VTNInetAddrAction#getMdAddress()}</li>
     *   <li>{@link VTNInetAddrAction#verifyImpl()}</li>
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
        IpNetwork[] addresses = {
            new Ip4Network("10.20.30.40"),
            new Ip4Network("192.168.0.1"),
            new Ip4Network("172.16.100.200"),
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetInetDstActionCaseBuilder vacBuilder =
            new VtnSetInetDstActionCaseBuilder();
        for (Integer order: orders) {
            for (IpNetwork iaddr: addresses) {
                Address mdaddr = new Ipv4Builder().
                    setIpv4Address(new Ipv4Prefix(iaddr.getCidrText())).
                    build();
                VtnSetInetDstAction vact = new VtnSetInetDstActionBuilder().
                    setAddress(mdaddr).build();
                VtnSetInetDstActionCase vac = vacBuilder.
                    setVtnSetInetDstAction(vact).build();
                SetNwDstAction ma = new SetNwDstActionBuilder().
                    setAddress(mdaddr).build();
                SetNwDstActionCase mact = new SetNwDstActionCaseBuilder().
                    setSetNwDstAction(ma).build();

                VTNSetInetDstAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetInetDstAction(iaddr);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetInetDstAction(vac, order);
                    anotherOrder = order.intValue();
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(iaddr, va.getAddress());
                assertEquals(mdaddr, va.getMdAddress());

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
                    VTNSetInetDstAction conv = new VTNSetInetDstAction();
                    assertEquals(va, conv.toFlowFilterAction(vac, order));

                    // toFlowFilterAction() should never affect instance
                    // variables.
                    assertEquals(null, conv.getAddress());
                }
            }
        }

        VTNSetInetDstAction va = new VTNSetInetDstAction();
        for (IpNetwork iaddr: addresses) {
            // toVtnAction() test.
            Address mdaddr = new Ipv4Builder().
                setIpv4Address(new Ipv4Prefix(iaddr.getCidrText())).
                build();
            VtnSetInetDstAction vact = new VtnSetInetDstActionBuilder().
                setAddress(mdaddr).build();
            VtnAction vaction = vacBuilder.
                setVtnSetInetDstAction(vact).build();
            SetNwDstAction ma = new SetNwDstActionBuilder().
                setAddress(mdaddr).build();
            Action action = new SetNwDstActionCaseBuilder().
                setSetNwDstAction(ma).build();
            assertEquals(vaction, va.toVtnAction(action));

            action = new SetNwSrcActionCaseBuilder().build();
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
            String emsg = "VTNSetInetDstAction: Unexpected type: " + action;
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
            action = new SetNwDstActionCaseBuilder().build();
            emsg = "VTNSetInetDstAction: No IP address: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }

            action = new SetNwDstActionCaseBuilder().
                setSetNwDstAction(new SetNwDstActionBuilder().build()).
                build();
            emsg = "VTNSetInetDstAction: No IP address: " + action;
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
            String desc = "set-inet-dst(ipv4=" + iaddr.getCidrText() + ")";
            assertEquals(desc, va.getDescription(vaction));
            assertEquals(null, va.getAddress());

            VtnSetInetDstAction vact1 = new VtnSetInetDstActionBuilder().
                build();
            VtnAction vaction1 = new VtnSetInetDstActionCaseBuilder().
                setVtnSetInetDstAction(vact1).
                build();
            desc = "set-inet-dst(null)";
            assertEquals(desc, va.getDescription(vaction1));
            assertEquals(null, va.getAddress());

            String ipv6 = "abcd:ffee::/32";
            Address maddr = new Ipv6Builder().
                setIpv6Address(new Ipv6Prefix(ipv6)).
                build();
            vact1 = new VtnSetInetDstActionBuilder().
                setAddress(maddr).
                build();
            vaction1 = new VtnSetInetDstActionCaseBuilder().
                setVtnSetInetDstAction(vact1).
                build();
            desc = "set-inet-dst(ipv6=" + ipv6 + ")";
            assertEquals(desc, va.getDescription(vaction1));
            assertEquals(null, va.getAddress());

            // getDescription(Action) test.
            // It should never affect instance variables.
            action = new SetNwDstActionCaseBuilder().
                setSetNwDstAction(ma).build();
            desc = "SET_NW_DST(ipv4=" + iaddr.getCidrText() + ")";
            assertEquals(desc, va.getDescription(action));
            assertEquals(null, va.getAddress());
        }

        Action action = new SetNwDstActionCaseBuilder().build();
        String desc = "SET_NW_DST(null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        SetNwDstAction mact = new SetNwDstActionBuilder().build();
        action = new SetNwDstActionCaseBuilder().
            setSetNwDstAction(mact).build();
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        mact = new SetNwDstActionBuilder().
            setAddress(new Ipv4Builder().build()).build();
        action = new SetNwDstActionCaseBuilder().
            setSetNwDstAction(mact).build();
        desc = "SET_NW_DST(ipv4=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        mact = new SetNwDstActionBuilder().
            setAddress(new Ipv6Builder().build()).build();
        action = new SetNwDstActionCaseBuilder().
            setSetNwDstAction(mact).build();
        desc = "SET_NW_DST(ipv6=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        String ipv6 = "::1/64";
        Address maddr = new Ipv6Builder().
            setIpv6Address(new Ipv6Prefix(ipv6)).
            build();
        mact = new SetNwDstActionBuilder().setAddress(maddr).build();
        action = new SetNwDstActionCaseBuilder().
            setSetNwDstAction(mact).build();
        desc = "SET_NW_DST(ipv6=" + ipv6 + ")";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        IpNetwork iaddr = new Ip4Network(0x0a0b0c0d);
        String emsg = "VTNSetInetDstAction: Action order cannot be null";
        VtnSetInetDstAction vact = new VtnSetInetDstActionBuilder().
            setAddress(iaddr.getMdAddress()).build();
        VtnSetInetDstActionCase vac = vacBuilder.
            setVtnSetInetDstAction(vact).build();
        try {
            new VTNSetInetDstAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Null IP address.
        emsg = "VTNSetInetDstAction: IP address cannot be null";
        vact = new VtnSetInetDstActionBuilder().build();
        vac = vacBuilder.setVtnSetInetDstAction(vact).build();
        try {
            new VTNSetInetDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        vac = vacBuilder.setVtnSetInetDstAction(null).build();
        try {
            new VTNSetInetDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        vact = new VtnSetInetDstActionBuilder().
            setAddress(new Ipv4Builder().build()).build();
        vac = vacBuilder.setVtnSetInetDstAction(vact).build();
        try {
            new VTNSetInetDstAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Specifying netmask.
        etag = RpcErrorTag.BAD_ELEMENT;
        for (int i = 1; i < 32; i++) {
            iaddr = new Ip4Network("192.168.100.233/" + i);
            emsg = "VTNSetInetDstAction: Netmask cannot be specified: " +
                iaddr;
            vact = new VtnSetInetDstActionBuilder().
                setAddress(iaddr.getMdAddress()).build();
            vac = vacBuilder.setVtnSetInetDstAction(vact).build();
            try {
                new VTNSetInetDstAction(vac, 1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VTNSetInetDstAction#newVtnAction(Address)}.
     */
    @Test
    public void testNewVtnAction() {
        IpNetwork[] addrs = {
            null,
            new Ip4Network("10.20.30.40"),
            new Ip4Network("192.168.0.1"),
            new Ip4Network("172.16.100.200"),
            new Ip4Network("1.2.3.4"),
        };

        for (IpNetwork ipn: addrs) {
            Address addr = (ipn == null) ? null : ipn.getMdAddress();
            VtnSetInetDstActionCase ac =
                VTNSetInetDstAction.newVtnAction(addr);
            VtnSetInetDstAction vaction = ac.getVtnSetInetDstAction();
            assertEquals(addr, vaction.getAddress());
        }
    }

    /**
     * Test case for {@link VTNSetInetDstAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        Integer order = 10;
        IpNetwork iaddr = new Ip4Network("192.168.100.200");
        VtnSetInetDstActionCase vac = VTNSetInetDstAction.
            newVtnAction(iaddr.getMdAddress());
        VTNSetInetDstAction va = new VTNSetInetDstAction(vac, order);
        String expected = "VTNSetInetDstAction[addr=192.168.100.200,order=10]";
        assertEquals(expected, va.toString());

        order = 123;
        iaddr = new Ip4Network("1.2.3.4");
        vac = VTNSetInetDstAction.newVtnAction(iaddr.getMdAddress());
        va = new VTNSetInetDstAction(vac, order);
        expected = "VTNSetInetDstAction[addr=1.2.3.4,order=123]";
        assertEquals(expected, va.toString());

        va = new VTNSetInetDstAction(iaddr);
        expected = "VTNSetInetDstAction[addr=1.2.3.4]";
        assertEquals(expected, va.toString());
    }

    /**
     * Test case for {@link VTNSetInetDstAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        IpNetwork iaddr = new Ip4Network("192.168.0.1");
        VtnSetInetDstActionCase vac = VTNSetInetDstAction.
            newVtnAction(iaddr.getMdAddress());
        VTNSetInetDstAction va = new VTNSetInetDstAction(vac, order);

        // In case of IPv4 packet.
        FlowActionContext ctx = mock(FlowActionContext.class);
        InetHeader inet = mock(InetHeader.class);
        when(ctx.getInetHeader()).thenReturn(inet);
        when(inet.setDestinationAddress(iaddr)).thenReturn(true);

        assertEquals(true, va.apply(ctx));
        verify(ctx).getInetHeader();
        verify(ctx).addFilterAction(va);

        verify(inet).setDestinationAddress(iaddr);
        verifyNoMoreInteractions(ctx, inet);

        // In case of IPv6 packet.
        // InetHeader.setSourceAddress() will return false.
        reset(ctx, inet);
        when(ctx.getInetHeader()).thenReturn(inet);
        when(inet.setDestinationAddress(iaddr)).thenReturn(false);

        assertEquals(false, va.apply(ctx));
        verify(ctx).getInetHeader();

        verify(inet).setDestinationAddress(iaddr);
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
     * Ensure that {@link VTNSetInetDstAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        List<Class<?>> jaxbClasses = new ArrayList<>();
        Collections.addAll(jaxbClasses, VTNSetInetDstAction.class,
                           VTNInetAddrAction.class, FlowFilterAction.class);

        IpNetwork[] addresses = {
            new Ip4Network("10.20.30.40"),
            new Ip4Network("192.168.0.1"),
            new Ip4Network("172.16.100.200"),
            new Ip4Network("1.2.3.4"),
        };
        int[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetInetDstActionCaseBuilder vacBuilder =
            new VtnSetInetDstActionCaseBuilder();
        Class<VTNSetInetDstAction> type = VTNSetInetDstAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Class<?> cls: jaxbClasses) {
            Marshaller m = createMarshaller(cls);
            Unmarshaller um = createUnmarshaller(cls);

            for (IpNetwork iaddr: addresses) {
                Address mdaddr = new Ipv4Builder().
                    setIpv4Address(new Ipv4Prefix(iaddr.getCidrText())).
                    build();
                VtnSetInetDstAction vact = new VtnSetInetDstActionBuilder().
                    setAddress(mdaddr).build();
                VtnSetInetDstActionCase vac = vacBuilder.
                    setVtnSetInetDstAction(vact).build();
                for (Integer order: orders) {
                    VTNSetInetDstAction va =
                        new VTNSetInetDstAction(vac, order);
                    String xml = marshal(m, va, type, XML_ROOT);
                    VTNSetInetDstAction va1 = unmarshal(um, xml, type);
                    va1.verify();
                    assertEquals(order, va1.getIdentifier());
                    assertEquals(iaddr, va1.getAddress());
                    assertEquals(va, va1);
                }
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = createUnmarshaller(type);
        IpNetwork iaddr = addresses[0];
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "VTNSetInetDstAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("ipv4-address", iaddr.getHostAddress())).
            toString();
        VTNSetInetDstAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // No IP address.
        Integer order = 1;
        emsg = "VTNSetInetDstAction: IP address cannot be null";
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

        // Specifying netmask.
        etag = RpcErrorTag.BAD_ELEMENT;
        for (int i = 1; i < 32; i++) {
            String cidr = "10.20.30.40/" + i;
            iaddr = new Ip4Network(cidr);
            emsg = "VTNSetInetDstAction: Netmask cannot be specified: " +
                iaddr;
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("order", order)).
                add(new XmlNode("ipv4-address", cidr)).
                toString();
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
