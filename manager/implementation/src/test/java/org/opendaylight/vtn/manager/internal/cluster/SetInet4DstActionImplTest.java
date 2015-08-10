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
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.Inet4Packet;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetInet4DstActionImpl}.
 */
public class SetInet4DstActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testGetter() throws Exception {
        for (InetAddress iaddr: createInet4Addresses(false)) {
            IpNetwork ipn = IpNetwork.create(iaddr);
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            SetInet4DstActionImpl impl = new SetInet4DstActionImpl(act);
            assertEquals(act, impl.getFlowAction());
            assertEquals(ipn, impl.getAddress());
        }

        // null action.
        try {
            new SetInet4DstActionImpl(null);
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

        Unmarshaller um = createUnmarshaller(SetInet4DstAction.class);
        for (String addr: badAddrs) {
            StringBuilder builder = new StringBuilder(XML_DECLARATION);
            if (addr == null) {
                builder.append("<setinet4dst />");
            } else {
                builder.append("<setinet4dst address=\"").append(addr).
                    append("\" />");
            }
            String xml = builder.toString();
            SetInet4DstAction act =
                unmarshal(um, xml, SetInet4DstAction.class);
            try {
                new SetInet4DstActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetInet4DstActionImpl#apply(PacketContext)}.
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

        Ip4Network dst1 = new Ip4Network("172.16.33.44");
        VTNSetInetDstAction vact = new VTNSetInetDstAction(dst1);
        SetInet4DstAction a = new SetInet4DstAction(dst1);
        SetInet4DstActionImpl act = new SetInet4DstActionImpl(a);
        assertEquals(true, act.apply(pctx));
        Mockito.verify(pctx).getInet4Packet();
        Mockito.verify(pctx).addFilterAction(vact);
        assertEquals(src, ipv4.getSourceAddress());
        assertEquals(dst1, ipv4.getDestinationAddress());
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
     * Test case for {@link SetInet4DstActionImpl#equals(Object)} and
     * {@link SetInet4DstActionImpl#hashCode()}.
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
            SetInet4DstAction act1 = new SetInet4DstAction(iaddr);
            SetInet4DstAction act2 = new SetInet4DstAction(iaddr2);
            SetInet4DstActionImpl impl1 = new SetInet4DstActionImpl(act1);
            SetInet4DstActionImpl impl2 = new SetInet4DstActionImpl(act2);
            testEquals(set, impl1, impl2);
        }

        assertEquals(iaddrs.size(), set.size());
    }

    /**
     * Test case for {@link SetInet4DstActionImpl#toString()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testToString() throws Exception {
        String prefix = "SetInet4DstActionImpl[";
        String suffix = "]";
        for (InetAddress iaddr: createInet4Addresses(false)) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            SetInet4DstActionImpl impl = new SetInet4DstActionImpl(act);
            String a = "addr=" + iaddr.getHostAddress();
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, impl.toString());
        }
    }

    /**
     * Ensure that {@link SetInet4DstActionImpl} is serializable.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        for (InetAddress iaddr: createInet4Addresses(false)) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            SetInet4DstActionImpl impl = new SetInet4DstActionImpl(act);
            serializeTest(impl);
        }
    }
}
