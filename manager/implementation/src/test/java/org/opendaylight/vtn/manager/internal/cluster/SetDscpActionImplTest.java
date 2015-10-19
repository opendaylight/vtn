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
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.packet.IPv4;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.Inet4Packet;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetDscpActionImpl}.
 */
public class SetDscpActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        byte[] bytes = {0, 1, 2, 10, 20, 30, 40, 50, 60, 63};
        for (byte dscp: bytes) {
            SetDscpAction act = new SetDscpAction(dscp);
            try {
                SetDscpActionImpl impl = new SetDscpActionImpl(act);
                assertEquals(act, impl.getFlowAction());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        // null action.
        try {
            new SetDscpActionImpl(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Invalid DSCP field value.
        byte[] invalid = {Byte.MIN_VALUE, -100, -10, -2, -1,
                          64, 65, 66, 70, 80, 90, 100, Byte.MAX_VALUE};
        for (byte dscp: invalid) {
            SetDscpAction act = new SetDscpAction(dscp);
            try {
                SetDscpActionImpl impl = new SetDscpActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetDscpActionImpl#apply(PacketContext)}.
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

        short dscp1 = 63;
        VTNSetInetDscpAction vact = new VTNSetInetDscpAction(dscp1);
        SetDscpAction a = new SetDscpAction((byte)dscp1);
        SetDscpActionImpl act = new SetDscpActionImpl(a);
        assertEquals(true, act.apply(pctx));
        Mockito.verify(pctx).getInet4Packet();
        Mockito.verify(pctx).addFilterAction(vact);
        assertEquals(src, ipv4.getSourceAddress());
        assertEquals(dst, ipv4.getDestinationAddress());
        assertEquals(proto, ipv4.getProtocol());
        assertEquals(dscp1, ipv4.getDscp());
        Mockito.reset(pctx);

        ipv4 = null;
        Mockito.when(pctx.getInet4Packet()).thenReturn(ipv4);
        assertEquals(false, act.apply(pctx));
        Mockito.verify(pctx).getInet4Packet();
        Mockito.verify(pctx, Mockito.never()).
            addFilterAction(Mockito.any(FlowFilterAction.class));
    }

    /**
     * Test case for {@link SetDscpActionImpl#equals(Object)} and
     * {@link SetDscpActionImpl#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        byte[] bytes = {0, 1, 2, 10, 20, 30, 40, 50, 60, 63};
        for (byte dscp: bytes) {
            SetDscpAction act1 = new SetDscpAction(dscp);
            SetDscpAction act2 = new SetDscpAction(dscp);
            try {
                SetDscpActionImpl impl1 = new SetDscpActionImpl(act1);
                SetDscpActionImpl impl2 = new SetDscpActionImpl(act2);
                testEquals(set, impl1, impl2);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        assertEquals(bytes.length, set.size());
    }

    /**
     * Test case for {@link SetDscpActionImpl#toString()}.
     */
    @Test
    public void testToString() {
        byte[] bytes = {0, 1, 2, 10, 20, 30, 40, 50, 60, 63};
        for (byte dscp: bytes) {
            SetDscpAction act = new SetDscpAction(dscp);
            try {
                SetDscpActionImpl impl = new SetDscpActionImpl(act);
                assertEquals("SetDscpActionImpl[dscp=" + dscp + "]",
                             impl.toString());
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }

    /**
     * Ensure that {@link SetDscpActionImpl} is serializable.
     */
    @Test
    public void testSerialize() {
        byte[] bytes = {0, 1, 2, 10, 20, 30, 40, 50, 60, 63};
        for (byte dscp: bytes) {
            SetDscpAction act = new SetDscpAction(dscp);
            try {
                SetDscpActionImpl impl = new SetDscpActionImpl(act);
                serializeTest(impl);
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }
}
