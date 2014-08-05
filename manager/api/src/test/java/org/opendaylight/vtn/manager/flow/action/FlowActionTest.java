/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.net.InetAddress;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Drop;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.action.SetNwTos;
import org.opendaylight.controller.sal.action.SetTpDst;
import org.opendaylight.controller.sal.action.SetTpSrc;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.action.SetVlanPcp;

/**
 * JUnit test for {@link FlowAction}.
 */
public class FlowActionTest extends TestBase {
    /**
     * Test case for {@link FlowAction#create(Action, boolean)}.
     */
    @Test
    public void testCreate() {
        boolean[] bools = {true, false};
        byte[][] macAddrs = {
            new byte[]{
                (byte)0x00, (byte)0x01, (byte)0x02,
                (byte)0x03, (byte)0x04, (byte)0x05,
            },
            new byte[]{
                (byte)0xfa, (byte)0xfb, (byte)0xfc,
                (byte)0xfc, (byte)0xfe, (byte)0xff,
            },
        };
        short[] vlans = {1, 2, 100, 1000, 4000, 4095};
        List<InetAddress> inet4Addrs = createInet4Addresses(false);
        byte[] tos = {0, 1, 20, 30, 40, 50, 60, 63};

        for (boolean icmp: bools) {
            assertEquals(null, FlowAction.create(null, icmp));

            FlowAction act = FlowAction.create(new Drop(), icmp);
            assertEquals(new DropAction(), act);
            act = FlowAction.create(new PopVlan(), icmp);
            assertEquals(new PopVlanAction(), act);

            for (byte[] mac: macAddrs) {
                act = FlowAction.create(new SetDlDst(mac), icmp);
                assertEquals(new SetDlDstAction(mac), act);
                act = FlowAction.create(new SetDlSrc(mac), icmp);
                assertEquals(new SetDlSrcAction(mac), act);
            }

            for (short vlan: vlans) {
                act = FlowAction.create(new SetVlanId(vlan), icmp);
                assertEquals(new SetVlanIdAction(vlan), act);
            }
            for (byte pcp = 0; pcp <= 7; pcp++) {
                act = FlowAction.create(new SetVlanPcp(pcp), icmp);
                assertEquals(new SetVlanPcpAction(pcp), act);
            }
            for (InetAddress iaddr: inet4Addrs) {
                act = FlowAction.create(new SetNwDst(iaddr), icmp);
                assertEquals(new SetInet4DstAction(iaddr), act);
                act = FlowAction.create(new SetNwSrc(iaddr), icmp);
                assertEquals(new SetInet4SrcAction(iaddr), act);
            }
            for (byte dscp: tos) {
                act = FlowAction.create(new SetNwTos(dscp), icmp);
                assertEquals(new SetDscpAction(dscp), act);
            }
        }

        short[] icmpValues = {1, 2, 64, 128, 200, 255};
        for (short v: icmpValues) {
            Action sal = new SetTpSrc((int)v);
            FlowAction act = FlowAction.create(sal, true);
            assertEquals(new SetIcmpTypeAction(v), act);
            sal = new SetTpDst((int)v);
            act = FlowAction.create(sal, true);
            assertEquals(new SetIcmpCodeAction(v), act);
        }

        int[] ports = {1, 2, 100, 200, 500, 10000, 30000, 50000, 65535};
        for (int port: ports) {
            Action sal = new SetTpSrc(port);
            FlowAction act = FlowAction.create(sal, false);
            assertEquals(new SetTpSrcAction(port), act);
            sal = new SetTpDst(port);
            act = FlowAction.create(sal, false);
            assertEquals(new SetTpDstAction(port), act);
        }
    }

    /**
     * Test case for {@link FlowAction#equals(Object)} and
     * {@link FlowAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        FlowAction[] actions = {
            new DropAction(),
            new PopVlanAction(),
            new SetDlDstAction(null),
            new SetDlSrcAction(null),
            new SetDscpAction((byte)0),
            new SetIcmpCodeAction((short)0),
            new SetIcmpTypeAction((short)0),
            new SetInet4DstAction(null),
            new SetInet4SrcAction(null),
            new SetTpDstAction(0),
            new SetTpSrcAction(0),
            new SetVlanIdAction((short)0),
            new SetVlanPcpAction((byte)0),
        };

        // FlowAction variants should be treated as different objects if
        // classes don't match.
        int count = 0;
        for (FlowAction act: actions) {
            assertTrue(set.add(act));
            assertFalse(set.add(act));
            count++;
            assertEquals(count, set.size());
        }
        assertEquals(actions.length, set.size());

        for (FlowAction act: actions) {
            assertTrue(set.remove(act));
            assertFalse(set.remove(act));
            count--;
            assertEquals(count, set.size());
        }
        assertTrue(set.isEmpty());
    }
}
