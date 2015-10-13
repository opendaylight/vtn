/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.net.InetAddress;
import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherTypes;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link FlowAction}.
 */
public class FlowActionTest extends TestBase {
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
            new PushVlanAction(EtherTypes.VLAN),
            new SetDlDstAction((byte[])null),
            new SetDlSrcAction((byte[])null),
            new SetDscpAction((byte)0),
            new SetIcmpCodeAction((short)0),
            new SetIcmpTypeAction((short)0),
            new SetInet4DstAction((InetAddress)null),
            new SetInet4SrcAction((InetAddress)null),
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
