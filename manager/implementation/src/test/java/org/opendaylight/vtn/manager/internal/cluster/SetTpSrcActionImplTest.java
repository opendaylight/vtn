/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.packet.ICMP;
import org.opendaylight.vtn.manager.packet.TCP;
import org.opendaylight.vtn.manager.packet.UDP;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.IcmpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.TcpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.UdpPacket;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortSrcAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetTpSrcActionImpl}.
 */
public class SetTpSrcActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testGetter() throws Exception {
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            SetTpSrcActionImpl impl = new SetTpSrcActionImpl(act);
            assertEquals(act, impl.getFlowAction());
            assertEquals(port, impl.getPort());
        }

        // null action.
        try {
            new SetTpSrcActionImpl(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Specify invalid ports.
        int[] badPorts = {
            Integer.MIN_VALUE,
            -1000,
            -1,
            65536,
            65537,
            Integer.MAX_VALUE
        };

        for (int port: badPorts) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            try {
                new SetTpSrcActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetTpSrcActionImpl#apply(PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        PacketContext pctx = Mockito.mock(PacketContext.class);
        int src = 12345;
        int dst = 333;
        TCP tpkt = new TCP().
            setSourcePort((short)src).setDestinationPort((short)dst);
        TcpPacket tcp = new TcpPacket(tpkt);
        Mockito.when(pctx.getL4Packet()).thenReturn(tcp);

        int src1 = 55555;
        VTNSetPortSrcAction vact = new VTNSetPortSrcAction(src1);
        SetTpSrcAction a = new SetTpSrcAction(src1);
        SetTpSrcActionImpl act = new SetTpSrcActionImpl(a);
        assertEquals(true, act.apply(pctx));
        Mockito.verify(pctx).getL4Packet();
        Mockito.verify(pctx).addFilterAction(vact);
        assertEquals(src1, tcp.getSourcePort());
        assertEquals(dst, tcp.getDestinationPort());
        Mockito.reset(pctx);

        src = 65535;
        dst = 987;
        UDP upkt = new UDP().
            setSourcePort((short)src).setDestinationPort((short)dst);
        UdpPacket udp = new UdpPacket(upkt);
        Mockito.when(pctx.getL4Packet()).thenReturn(udp);

        src1 = 65;
        vact = new VTNSetPortSrcAction(src1);
        a = new SetTpSrcAction(src1);
        act = new SetTpSrcActionImpl(a);
        assertEquals(true, act.apply(pctx));
        Mockito.verify(pctx).getL4Packet();
        Mockito.verify(pctx).addFilterAction(vact);
        assertEquals(src1, udp.getSourcePort());
        assertEquals(dst, udp.getDestinationPort());
        Mockito.reset(pctx);

        IcmpPacket icmp = new IcmpPacket(new ICMP());
        Mockito.when(pctx.getL4Packet()).thenReturn(icmp);
        assertEquals(false, act.apply(pctx));
        Mockito.verify(pctx).getL4Packet();
        Mockito.verify(pctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.reset(pctx);

        icmp = null;
        Mockito.when(pctx.getL4Packet()).thenReturn(icmp);
        assertEquals(false, act.apply(pctx));
        Mockito.verify(pctx).getL4Packet();
        Mockito.verify(pctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.reset(pctx);
    }

    /**
     * Test case for {@link SetTpSrcActionImpl#equals(Object)} and
     * {@link SetTpSrcActionImpl#hashCode()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<Object>();
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpSrcAction act1 = new SetTpSrcAction(port);
            SetTpSrcAction act2 = new SetTpSrcAction(port);
            SetTpSrcActionImpl impl1 = new SetTpSrcActionImpl(act1);
            SetTpSrcActionImpl impl2 = new SetTpSrcActionImpl(act2);
            testEquals(set, impl1, impl2);
        }

        assertEquals(ports.length, set.size());
    }

    /**
     * Test case for {@link SetTpSrcActionImpl#toString()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testToString() throws Exception {
        String prefix = "SetTpSrcActionImpl[";
        String suffix = "]";
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            SetTpSrcActionImpl impl = new SetTpSrcActionImpl(act);
            String a = "port=" + port;
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, impl.toString());
        }
    }

    /**
     * Ensure that {@link SetTpSrcActionImpl} is serializable.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            SetTpSrcActionImpl impl = new SetTpSrcActionImpl(act);
            serializeTest(impl);
        }
    }
}
