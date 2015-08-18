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
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.IcmpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.TcpPacket;
import org.opendaylight.vtn.manager.internal.packet.cache.UdpPacket;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpCodeAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.packet.ICMP;
import org.opendaylight.controller.sal.packet.TCP;
import org.opendaylight.controller.sal.packet.UDP;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetIcmpCodeActionImpl}.
 */
public class SetIcmpCodeActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] codes = {0, 1, 2, 10, 30, 60, 100, 130, 150, 200, 254, 255};
        for (short code: codes) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            try {
                SetIcmpCodeActionImpl impl = new SetIcmpCodeActionImpl(act);
                assertEquals(act, impl.getFlowAction());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        // null action.
        try {
            new SetIcmpCodeActionImpl(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Invalid ICMP code.
        short[] invalid = {Short.MIN_VALUE, -100, -10, -2, -1,
                           256, 257, 300, 500, 1000, 3000, Short.MAX_VALUE};
        for (short code: invalid) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            try {
                SetIcmpCodeActionImpl impl = new SetIcmpCodeActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetIcmpCodeActionImpl#apply(PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        PacketContext pctx = Mockito.mock(PacketContext.class);
        short type = 1;
        short code = 10;
        ICMP pkt = new ICMP().setType((byte)type).setCode((byte)code);
        IcmpPacket icmp = new IcmpPacket(pkt);
        Mockito.when(pctx.getL4Packet()).thenReturn(icmp);

        short code1 = 113;
        VTNSetIcmpCodeAction vact = new VTNSetIcmpCodeAction(code1);
        SetIcmpCodeAction a = new SetIcmpCodeAction(code1);
        SetIcmpCodeActionImpl act = new SetIcmpCodeActionImpl(a);
        assertEquals(true, act.apply(pctx));
        Mockito.verify(pctx).getL4Packet();
        Mockito.verify(pctx).addFilterAction(vact);
        assertEquals(type, icmp.getIcmpType());
        assertEquals(code1, icmp.getIcmpCode());
        Mockito.reset(pctx);

        icmp = null;
        Mockito.when(pctx.getL4Packet()).thenReturn(icmp);
        assertEquals(false, act.apply(pctx));
        Mockito.verify(pctx).getL4Packet();
        Mockito.verify(pctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.reset(pctx);

        TcpPacket tcp = new TcpPacket(new TCP());
        Mockito.when(pctx.getL4Packet()).thenReturn(tcp);
        assertEquals(false, act.apply(pctx));
        Mockito.verify(pctx).getL4Packet();
        Mockito.verify(pctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
        Mockito.reset(pctx);

        UdpPacket udp = new UdpPacket(new UDP());
        Mockito.when(pctx.getL4Packet()).thenReturn(udp);
        assertEquals(false, act.apply(pctx));
        Mockito.verify(pctx).getL4Packet();
        Mockito.verify(pctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
    }

    /**
     * Test case for {@link SetIcmpCodeActionImpl#equals(Object)} and
     * {@link SetIcmpCodeActionImpl#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] codes = {0, 1, 2, 10, 30, 60, 100, 130, 150, 200, 254, 255};
        for (short code: codes) {
            SetIcmpCodeAction act1 = new SetIcmpCodeAction(code);
            SetIcmpCodeAction act2 = new SetIcmpCodeAction(code);
            try {
                SetIcmpCodeActionImpl impl1 = new SetIcmpCodeActionImpl(act1);
                SetIcmpCodeActionImpl impl2 = new SetIcmpCodeActionImpl(act2);
                testEquals(set, impl1, impl2);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        assertEquals(codes.length, set.size());
    }

    /**
     * Test case for {@link SetIcmpCodeActionImpl#toString()}.
     */
    @Test
    public void testToString() {
        short[] codes = {0, 1, 2, 10, 30, 60, 100, 130, 150, 200, 254, 255};
        for (short code: codes) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            try {
                SetIcmpCodeActionImpl impl = new SetIcmpCodeActionImpl(act);
                assertEquals("SetIcmpCodeActionImpl[code=" + code + "]",
                             impl.toString());
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }

    /**
     * Ensure that {@link SetIcmpCodeActionImpl} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] codes = {0, 1, 2, 10, 30, 60, 100, 130, 150, 200, 254, 255};
        for (short code: codes) {
            SetIcmpCodeAction act = new SetIcmpCodeAction(code);
            try {
                SetIcmpCodeActionImpl impl = new SetIcmpCodeActionImpl(act);
                serializeTest(impl);
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }
}
