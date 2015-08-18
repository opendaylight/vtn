/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.src.action._case.VtnSetInetSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.set.inet.src.action._case.VtnSetInetSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwSrcActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.src.action._case.SetNwSrcAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.nw.src.action._case.SetNwSrcActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4Builder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv6Builder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv6Prefix;

/**
 * JUnit test for {@link VTNSetInetSrcAction}.
 */
public class VTNSetInetSrcActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNSetInetSrcAction} class.
     */
    private static final String  XML_ROOT = "vtn-set-inet-src-action";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNSetInetSrcAction} instance.
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
     *   <li>{@link VTNSetInetSrcAction#VTNSetInetSrcAction(IpNetwork)}</li>
     *   <li>{@link VTNSetInetSrcAction#VTNSetInetSrcAction(SetInet4SrcAction, int)}</li>
     *   <li>{@link VTNSetInetSrcAction#VTNSetInetSrcAction(VtnSetInetSrcActionCase, Integer)}</li>
     *   <li>{@link VTNSetInetSrcAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetInetSrcAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetInetSrcAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNSetInetSrcAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetInetSrcAction#getDescription(Action)}</li>
     *   <li>{@link VTNInetAddrAction#getAddress()}</li>
     *   <li>{@link VTNInetAddrAction#getMdAddress()}</li>
     *   <li>{@link VTNInetAddrAction#verifyImpl()}</li>
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
        IpNetwork[] addresses = {
            new Ip4Network("10.20.30.40"),
            new Ip4Network("192.168.0.1"),
            new Ip4Network("172.16.100.200"),
        };
        Integer[] orders = {
            null, Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnSetInetSrcActionCaseBuilder vacBuilder =
            new VtnSetInetSrcActionCaseBuilder();
        for (Integer order: orders) {
            for (IpNetwork iaddr: addresses) {
                Address mdaddr = new Ipv4Builder().
                    setIpv4Address(new Ipv4Prefix(iaddr.getCidrText())).
                    build();
                SetInet4SrcAction vad =
                    new SetInet4SrcAction(iaddr.getInetAddress());
                VtnSetInetSrcAction vact = new VtnSetInetSrcActionBuilder().
                    setAddress(mdaddr).build();
                VtnSetInetSrcActionCase vac = vacBuilder.
                    setVtnSetInetSrcAction(vact).build();
                SetNwSrcAction ma = new SetNwSrcActionBuilder().
                    setAddress(mdaddr).build();
                SetNwSrcActionCase mact = new SetNwSrcActionCaseBuilder().
                    setSetNwSrcAction(ma).build();

                VTNSetInetSrcAction va;
                Integer anotherOrder;
                if (order == null) {
                    va = new VTNSetInetSrcAction(iaddr);
                    anotherOrder = 0;
                } else {
                    va = new VTNSetInetSrcAction(vad, order);
                    assertEquals(order, va.getIdentifier());
                    assertEquals(iaddr, va.getAddress());
                    assertEquals(mdaddr, va.getMdAddress());

                    va = new VTNSetInetSrcAction(vac, order);
                    anotherOrder = order.intValue() + 1;
                }
                assertEquals(order, va.getIdentifier());
                assertEquals(iaddr, va.getAddress());
                assertEquals(mdaddr, va.getMdAddress());

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

        VTNSetInetSrcAction va = new VTNSetInetSrcAction();
        for (IpNetwork iaddr: addresses) {
            // toFlowAction() test.
            Address mdaddr = new Ipv4Builder().
                setIpv4Address(new Ipv4Prefix(iaddr.getCidrText())).
                build();
            SetInet4SrcAction vad =
                new SetInet4SrcAction(iaddr.getInetAddress());
            VtnSetInetSrcAction vact = new VtnSetInetSrcActionBuilder().
                setAddress(mdaddr).build();
            VtnAction vaction = vacBuilder.
                setVtnSetInetSrcAction(vact).build();
            assertEquals(vad, va.toFlowAction(vaction));

            vaction = VTNSetInetDstAction.newVtnAction(mdaddr);
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            StatusCode ecode = StatusCode.BADREQUEST;
            String emsg = "VTNSetInetSrcAction: Unexpected type: " + vaction;
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
                setVtnSetInetSrcAction(new VtnSetInetSrcActionBuilder().
                                       build()).
                build();
            emsg = "VTNSetInetSrcAction: No IP address: " + vaction;
            try {
                va.toFlowAction(vaction);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            vaction = vacBuilder.setVtnSetInetSrcAction(null).build();
            emsg = "VTNSetInetSrcAction: No IP address: " + vaction;
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
            SetNwSrcAction ma = new SetNwSrcActionBuilder().
                setAddress(mdaddr).build();
            Action action = new SetNwSrcActionCaseBuilder().
                setSetNwSrcAction(ma).build();
            vaction = vacBuilder.setVtnSetInetSrcAction(vact).build();
            assertEquals(vaction, va.toVtnAction(action));

            etag = RpcErrorTag.BAD_ELEMENT;
            action = new SetNwDstActionCaseBuilder().build();
            emsg = "VTNSetInetSrcAction: Unexpected type: " + action;
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
            action = new SetNwSrcActionCaseBuilder().build();
            emsg = "VTNSetInetSrcAction: No IP address: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            action = new SetNwSrcActionCaseBuilder().
                setSetNwSrcAction(new SetNwSrcActionBuilder().build()).
                build();
            emsg = "VTNSetInetSrcAction: No IP address: " + action;
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
            action = new SetNwSrcActionCaseBuilder().
                setSetNwSrcAction(ma).build();
            String desc = "SET_NW_SRC(ipv4=" + iaddr.getCidrText() + ")";
            assertEquals(desc, va.getDescription(action));

            // getDescription() should never affect instance variables.
            assertEquals(null, va.getAddress());
        }

        Action action = new SetNwSrcActionCaseBuilder().build();
        String desc = "SET_NW_SRC()";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        SetNwSrcAction mact = new SetNwSrcActionBuilder().build();
        action = new SetNwSrcActionCaseBuilder().
            setSetNwSrcAction(mact).build();
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        mact = new SetNwSrcActionBuilder().
            setAddress(new Ipv4Builder().build()).build();
        action = new SetNwSrcActionCaseBuilder().
            setSetNwSrcAction(mact).build();
        desc = "SET_NW_SRC(ipv4=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        mact = new SetNwSrcActionBuilder().
            setAddress(new Ipv6Builder().build()).build();
        action = new SetNwSrcActionCaseBuilder().
            setSetNwSrcAction(mact).build();
        desc = "SET_NW_SRC(ipv6=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        String ipv6 = "::1/64";
        Address maddr = new Ipv6Builder().
            setIpv6Address(new Ipv6Prefix(ipv6)).
            build();
        mact = new SetNwSrcActionBuilder().setAddress(maddr).build();
        action = new SetNwSrcActionCaseBuilder().
            setSetNwSrcAction(mact).build();
        desc = "SET_NW_SRC(ipv6=" + ipv6 + ")";
        assertEquals(desc, va.getDescription(action));
        assertEquals(null, va.getAddress());

        // Null order.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        IpNetwork iaddr = new Ip4Network(0x0a0b0c0d);
        String emsg = "VTNSetInetSrcAction: Action order cannot be null";
        VtnSetInetSrcAction vact = new VtnSetInetSrcActionBuilder().
            setAddress(iaddr.getMdAddress()).build();
        VtnSetInetSrcActionCase vac = vacBuilder.
            setVtnSetInetSrcAction(vact).build();
        try {
            new VTNSetInetSrcAction(vac, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Null IP address.
        emsg = "VTNSetInetSrcAction: IP address cannot be null";
        SetInet4SrcAction vad = new SetInet4SrcAction((InetAddress)null);
        try {
            new VTNSetInetSrcAction(vad, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetInetSrcActionBuilder().build();
        vac = vacBuilder.setVtnSetInetSrcAction(vact).build();
        try {
            new VTNSetInetSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vac = vacBuilder.setVtnSetInetSrcAction(null).build();
        try {
            new VTNSetInetSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        vact = new VtnSetInetSrcActionBuilder().
            setAddress(new Ipv4Builder().build()).build();
        vac = vacBuilder.setVtnSetInetSrcAction(vact).build();
        try {
            new VTNSetInetSrcAction(vac, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // Specifying netmask.
        etag = RpcErrorTag.BAD_ELEMENT;
        for (int i = 1; i < 32; i++) {
            iaddr = new Ip4Network("192.168.100.233/" + i);
            emsg = "VTNSetInetSrcAction: Netmask cannot be specified: " +
                iaddr;
            vact = new VtnSetInetSrcActionBuilder().
                setAddress(iaddr.getMdAddress()).build();
            vac = vacBuilder.setVtnSetInetSrcAction(vact).build();
            try {
                new VTNSetInetSrcAction(vac, 1);
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
     * Test case for {@link VTNSetInetSrcAction#newVtnAction(Address)}.
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
            VtnSetInetSrcActionCase ac =
                VTNSetInetSrcAction.newVtnAction(addr);
            VtnSetInetSrcAction vaction = ac.getVtnSetInetSrcAction();
            assertEquals(addr, vaction.getAddress());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNSetInetSrcAction#equals(Object)}</li>
     *   <li>{@link VTNSetInetSrcAction#hashCode()}</li>
     *   <li>{@link VTNSetInetDstAction#equals(Object)}</li>
     *   <li>{@link VTNSetInetDstAction#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        IpNetwork[] addresses = {
            new Ip4Network("10.20.30.40"),
            new Ip4Network("192.168.0.1"),
            new Ip4Network("172.16.100.200"),
            new Ip4Network("1.2.3.4"),
        };
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (IpNetwork iaddr: addresses) {
            IpNetwork iaddr1 = IpNetwork.create(iaddr.getText());
            VTNSetInetSrcAction vsrc1 = new VTNSetInetSrcAction(iaddr);
            VTNSetInetSrcAction vsrc2 = new VTNSetInetSrcAction(iaddr1);
            VTNSetInetDstAction vdst1 = new VTNSetInetDstAction(iaddr);
            VTNSetInetDstAction vdst2 = new VTNSetInetDstAction(iaddr1);

            // VTNSetInetSrcAction should be distinguished from
            // VTNSetInetDstAction.
            testEquals(set, vsrc1, vsrc2);
            testEquals(set, vdst1, vdst2);

            for (Integer order: orders) {
                VtnSetInetSrcActionCase src1 = VTNSetInetSrcAction.
                    newVtnAction(iaddr.getMdAddress());
                VtnSetInetSrcActionCase src2 = VTNSetInetSrcAction.
                    newVtnAction(iaddr1.getMdAddress());
                vsrc1 = new VTNSetInetSrcAction(src1, order);
                vsrc2 = new VTNSetInetSrcAction(src2, order);

                VtnSetInetDstActionCase dst1 = VTNSetInetDstAction.
                    newVtnAction(iaddr.getMdAddress());
                VtnSetInetDstActionCase dst2 = VTNSetInetDstAction.
                    newVtnAction(iaddr1.getMdAddress());
                vdst1 = new VTNSetInetDstAction(dst1, order);
                vdst2 = new VTNSetInetDstAction(dst2, order);

                // VTNSetInetSrcAction should be distinguished from
                // VTNSetInetDstAction.
                testEquals(set, vsrc1, vsrc2);
                testEquals(set, vdst1, vdst2);
            }
        }

        int expected = (orders.length + 1) * addresses.length * 2;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VTNSetInetSrcAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        Integer order = 10;
        IpNetwork iaddr = new Ip4Network("192.168.100.200");
        VtnSetInetSrcActionCase vac = VTNSetInetSrcAction.
            newVtnAction(iaddr.getMdAddress());
        VTNSetInetSrcAction va = new VTNSetInetSrcAction(vac, order);
        String expected = "VTNSetInetSrcAction[addr=192.168.100.200,order=10]";
        assertEquals(expected, va.toString());

        order = 123;
        iaddr = new Ip4Network("1.2.3.4");
        vac = VTNSetInetSrcAction.newVtnAction(iaddr.getMdAddress());
        va = new VTNSetInetSrcAction(vac, order);
        expected = "VTNSetInetSrcAction[addr=1.2.3.4,order=123]";
        assertEquals(expected, va.toString());

        va = new VTNSetInetSrcAction(iaddr);
        expected = "VTNSetInetSrcAction[addr=1.2.3.4]";
        assertEquals(expected, va.toString());
    }

    /**
     * Test case for {@link VTNSetInetSrcAction#apply(FlowActionContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        Integer order = 10;
        IpNetwork iaddr = new Ip4Network("192.168.0.1");
        VtnSetInetSrcActionCase vac = VTNSetInetSrcAction.
            newVtnAction(iaddr.getMdAddress());
        VTNSetInetSrcAction va = new VTNSetInetSrcAction(vac, order);

        // In case of IPv4 packet.
        FlowActionContext ctx = Mockito.mock(FlowActionContext.class);
        InetHeader inet = Mockito.mock(InetHeader.class);
        Mockito.when(ctx.getInetHeader()).thenReturn(inet);
        Mockito.when(inet.setSourceAddress(iaddr)).thenReturn(true);

        assertEquals(true, va.apply(ctx));
        Mockito.verify(ctx).getInetHeader();
        Mockito.verify(ctx).addFilterAction(va);
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();

        Mockito.verify(inet).setSourceAddress(iaddr);
        Mockito.verify(inet, Mockito.never()).getSourceAddress();
        Mockito.verify(inet, Mockito.never()).getDestinationAddress();
        Mockito.verify(inet, Mockito.never()).
            setDestinationAddress(Mockito.any(IpNetwork.class));
        Mockito.verify(inet, Mockito.never()).getProtocol();
        Mockito.verify(inet, Mockito.never()).getDscp();
        Mockito.verify(inet, Mockito.never()).setDscp(Mockito.anyShort());

        // In case of IPv6 packet.
        // InetHeader.setSourceAddress() will return false.
        Mockito.reset(ctx, inet);
        Mockito.when(ctx.getInetHeader()).thenReturn(inet);
        Mockito.when(inet.setSourceAddress(iaddr)).thenReturn(false);

        assertEquals(false, va.apply(ctx));
        Mockito.verify(ctx).getInetHeader();
        Mockito.verify(ctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.verify(ctx, Mockito.never()).
            removeFilterAction(Mockito.any(Class.class));
        Mockito.verify(ctx, Mockito.never()).getFilterAction();

        Mockito.verify(inet).setSourceAddress(iaddr);
        Mockito.verify(inet, Mockito.never()).getSourceAddress();
        Mockito.verify(inet, Mockito.never()).getDestinationAddress();
        Mockito.verify(inet, Mockito.never()).
            setDestinationAddress(Mockito.any(IpNetwork.class));
        Mockito.verify(inet, Mockito.never()).getProtocol();
        Mockito.verify(inet, Mockito.never()).getDscp();
        Mockito.verify(inet, Mockito.never()).setDscp(Mockito.anyShort());

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
     * Ensure that {@link VTNSetInetSrcAction} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Unmarshaller[] unmarshallers = {
            createUnmarshaller(VTNSetInetSrcAction.class),
            createUnmarshaller(VTNInetAddrAction.class),
            createUnmarshaller(FlowFilterAction.class),
        };

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

        Class<VTNSetInetSrcAction> type = VTNSetInetSrcAction.class;
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        for (Unmarshaller um: unmarshallers) {
            for (Integer order: orders) {
                for (IpNetwork iaddr: addresses) {
                    String xml = new XmlNode(XML_ROOT).
                        add(new XmlNode("order", order)).
                        add(new XmlNode("ipv4-address",
                                        iaddr.getHostAddress())).
                        toString();
                    VTNSetInetSrcAction va = unmarshal(um, xml, type);
                    va.verify();
                    assertEquals(order, va.getIdentifier());
                    assertEquals(iaddr, va.getAddress());
                }
            }

            // Ensure that broken values in XML can be detected.
            jaxbErrorTest(um, type, dlist);
        }

        // No action order.
        Unmarshaller um = unmarshallers[0];
        IpNetwork iaddr = addresses[0];
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNSetInetSrcAction: Action order cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("ipv4-address", iaddr.getHostAddress())).
            toString();
        VTNSetInetSrcAction va = unmarshal(um, xml, type);
        try {
            va.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        // No IP address.
        Integer order = 1;
        emsg = "VTNSetInetSrcAction: IP address cannot be null";
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

        // Specifying netmask.
        etag = RpcErrorTag.BAD_ELEMENT;
        for (int i = 1; i < 32; i++) {
            String cidr = "10.20.30.40/" + i;
            iaddr = new Ip4Network(cidr);
            emsg = "VTNSetInetSrcAction: Netmask cannot be specified: " +
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
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }
        }
    }
}
