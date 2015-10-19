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
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.packet.cache.EtherPacket;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetVlanPcpActionImpl}.
 */
public class SetVlanPcpActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (byte pri = 0; pri <= 7; pri++) {
            SetVlanPcpAction act = new SetVlanPcpAction(pri);
            try {
                SetVlanPcpActionImpl impl = new SetVlanPcpActionImpl(act);
                assertEquals(act, impl.getFlowAction());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        // null action.
        try {
            new SetVlanPcpActionImpl(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Invalid VLAN prioriry.
        byte[] invalid = {Byte.MIN_VALUE, -100, -10, -2, -1,
                          8, 9, 10, 20, 30, 100, Byte.MAX_VALUE};
        for (byte pri: invalid) {
            SetVlanPcpAction act = new SetVlanPcpAction(pri);
            try {
                SetVlanPcpActionImpl impl = new SetVlanPcpActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetVlanPcpActionImpl#apply(PacketContext)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testApply() throws Exception {
        PacketContext pctx = Mockito.mock(PacketContext.class);
        EtherAddress src = new EtherAddress(0x0123456789abL);
        EtherAddress dst = new EtherAddress(0xf0fafbfcfdfeL);
        int type = 0x1234;
        short vid = 10;
        byte pcp = 1;
        byte[] payload = new byte[]{0x01, 0x02, 0x03, 0x04};
        Ethernet pkt = createEthernet(src.getBytes(), dst.getBytes(), type,
                                      vid, pcp, payload);
        EtherPacket ether = new EtherPacket(pkt);
        Mockito.when(pctx.getEtherPacket()).thenReturn(ether);

        byte pcp1 = 7;
        VTNSetVlanPcpAction vact = new VTNSetVlanPcpAction(pcp1);
        SetVlanPcpAction a = new SetVlanPcpAction(pcp1);
        SetVlanPcpActionImpl act = new SetVlanPcpActionImpl(a);
        assertEquals(true, act.apply(pctx));
        Mockito.verify(pctx).getEtherPacket();
        Mockito.verify(pctx).addFilterAction(vact);
        assertEquals(src, ether.getSourceAddress());
        assertEquals(dst, ether.getDestinationAddress());
        assertEquals(type, ether.getEtherType());
        assertEquals((int)vid, ether.getVlanId());
        assertEquals((short)pcp1, ether.getVlanPriority());
    }

    /**
     * Test case for {@link SetVlanPcpActionImpl#equals(Object)} and
     * {@link SetVlanPcpActionImpl#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        byte[] priorities = {0, 1, 2, 3, 4, 5, 6, 7};
        for (byte pri: priorities) {
            SetVlanPcpAction act1 = new SetVlanPcpAction(pri);
            SetVlanPcpAction act2 = new SetVlanPcpAction(pri);
            try {
                SetVlanPcpActionImpl impl1 = new SetVlanPcpActionImpl(act1);
                SetVlanPcpActionImpl impl2 = new SetVlanPcpActionImpl(act2);
                testEquals(set, impl1, impl2);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        assertEquals(priorities.length, set.size());
    }

    /**
     * Test case for {@link SetVlanPcpActionImpl#toString()}.
     */
    @Test
    public void testToString() {
        for (byte pri = 0; pri <= 7; pri++) {
            SetVlanPcpAction act = new SetVlanPcpAction(pri);
            try {
                SetVlanPcpActionImpl impl = new SetVlanPcpActionImpl(act);
                assertEquals("SetVlanPcpActionImpl[priority=" + pri + "]",
                             impl.toString());
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }

    /**
     * Ensure that {@link SetVlanPcpActionImpl} is serializable.
     */
    @Test
    public void testSerialize() {
        for (byte pri = 0; pri <= 7; pri++) {
            SetVlanPcpAction act = new SetVlanPcpAction(pri);
            try {
                SetVlanPcpActionImpl impl = new SetVlanPcpActionImpl(act);
                serializeTest(impl);
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }
}
