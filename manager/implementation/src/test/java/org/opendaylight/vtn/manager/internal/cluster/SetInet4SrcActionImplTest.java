/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.packet.IPv4;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.Inet4Packet;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetSrcAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetInet4SrcActionImpl}.
 */
public class SetInet4SrcActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testGetter() throws Exception {
        for (InetAddress iaddr: createInet4Addresses(false)) {
            IpNetwork ipn = IpNetwork.create(iaddr);
            SetInet4SrcAction act = new SetInet4SrcAction(iaddr);
            SetInet4SrcActionImpl impl = new SetInet4SrcActionImpl(act);
            assertEquals(act, impl.getFlowAction());
            assertEquals(ipn, impl.getAddress());
        }

        // null action.
        try {
            new SetInet4SrcActionImpl(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Specify invalid IP address via JAXB.
        String[] badAddrs = {
            null,
            "",
            "  ",
            "::1",
            "Bad IP address",
        };

        Unmarshaller um = createUnmarshaller(SetInet4SrcAction.class);
        for (String addr: badAddrs) {
            StringBuilder builder = new StringBuilder(XML_DECLARATION);
            if (addr == null) {
                builder.append("<setinet4src />");
            } else {
                builder.append("<setinet4src address=\"").append(addr).
                    append("\" />");
            }
            String xml = builder.toString();
            SetInet4SrcAction act =
                unmarshal(um, xml, SetInet4SrcAction.class);
            try {
                new SetInet4SrcActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetInet4SrcActionImpl#apply(PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        PacketContext pctx = Mockito.mock(PacketContext.class);
        Ip4Network src = new Ip4Network("10.20.30.40");
        Ip4Network dst = new Ip4Network("192.168.100.200");
        short proto = 111;
        short dscp = 0;
        byte[] payload = new byte[]{0x01, 0x02, 0x03, 0x04};
        IPv4 pkt = createIPv4(src.getInetAddress(), dst.getInetAddress(),
                              proto, (byte)dscp);
        Inet4Packet ipv4 = new Inet4Packet(pkt);
        Mockito.when(pctx.getInet4Packet()).thenReturn(ipv4);

        Ip4Network src1 = new Ip4Network("192.168.34.56");
        VTNSetInetSrcAction vact = new VTNSetInetSrcAction(src1);
        SetInet4SrcAction a = new SetInet4SrcAction(src1);
        SetInet4SrcActionImpl act = new SetInet4SrcActionImpl(a);
        assertEquals(true, act.apply(pctx));
        Mockito.verify(pctx).getInet4Packet();
        Mockito.verify(pctx).addFilterAction(vact);
        assertEquals(src1, ipv4.getSourceAddress());
        assertEquals(dst, ipv4.getDestinationAddress());
        assertEquals(proto, ipv4.getProtocol());
        assertEquals(dscp, ipv4.getDscp());
        Mockito.reset(pctx);

        ipv4 = null;
        Mockito.when(pctx.getInet4Packet()).thenReturn(ipv4);
        assertEquals(false, act.apply(pctx));
        Mockito.verify(pctx).getInet4Packet();
        Mockito.verify(pctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
    }

    /**
     * Test case for {@link SetInet4SrcActionImpl#equals(Object)} and
     * {@link SetInet4SrcActionImpl#hashCode()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<Object>();
        List<InetAddress> iaddrs = createInet4Addresses(false);
        for (InetAddress iaddr: iaddrs) {
            byte[] raw = iaddr.getAddress();
            InetAddress iaddr2 = InetAddress.getByAddress(raw);
            SetInet4SrcAction act1 = new SetInet4SrcAction(iaddr);
            SetInet4SrcAction act2 = new SetInet4SrcAction(iaddr2);
            SetInet4SrcActionImpl impl1 = new SetInet4SrcActionImpl(act1);
            SetInet4SrcActionImpl impl2 = new SetInet4SrcActionImpl(act2);
            testEquals(set, impl1, impl2);
        }

        assertEquals(iaddrs.size(), set.size());
    }

    /**
     * Test case for {@link SetInet4SrcActionImpl#toString()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testToString() throws Exception {
        String prefix = "SetInet4SrcActionImpl[";
        String suffix = "]";
        for (InetAddress iaddr: createInet4Addresses(false)) {
            SetInet4SrcAction act = new SetInet4SrcAction(iaddr);
            SetInet4SrcActionImpl impl = new SetInet4SrcActionImpl(act);
            String a = "addr=" + iaddr.getHostAddress();
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, impl.toString());
        }
    }

    /**
     * Ensure that {@link SetInet4SrcActionImpl} is serializable.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        for (InetAddress iaddr: createInet4Addresses(false)) {
            SetInet4SrcAction act = new SetInet4SrcAction(iaddr);
            SetInet4SrcActionImpl impl = new SetInet4SrcActionImpl(act);
            serializeTest(impl);
        }
    }
}
