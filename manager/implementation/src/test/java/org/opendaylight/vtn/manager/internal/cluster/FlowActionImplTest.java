/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Random;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.DropAction;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.PopVlanAction;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanIdAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link FlowActionImpl}.
 */
public class FlowActionImplTest extends TestBase {
    /**
     * Mask bits for valid unicast MAC address bits.
     */
    private static final long  MACADDR_MASK = 0xfeffffffffffL;

    /**
     * Test case for {@link FlowActionImpl#create(FlowAction)}.
     */
    @Test
    public void testCreate() throws Exception {
        // Expected action implementation classes.
        HashMap<Class<?>, Class<?>> implClasses =
            new HashMap<Class<?>, Class<?>>();
        implClasses.put(SetDlSrcAction.class, SetDlSrcActionImpl.class);
        implClasses.put(SetDlDstAction.class, SetDlDstActionImpl.class);
        implClasses.put(SetVlanPcpAction.class, SetVlanPcpActionImpl.class);
        implClasses.put(SetInet4SrcAction.class, SetInet4SrcActionImpl.class);
        implClasses.put(SetInet4DstAction.class, SetInet4DstActionImpl.class);
        implClasses.put(SetDscpAction.class, SetDscpActionImpl.class);
        implClasses.put(SetTpSrcAction.class, SetTpSrcActionImpl.class);
        implClasses.put(SetTpDstAction.class, SetTpDstActionImpl.class);
        implClasses.put(SetIcmpTypeAction.class, SetIcmpTypeActionImpl.class);
        implClasses.put(SetIcmpCodeAction.class, SetIcmpCodeActionImpl.class);

        // Create actions.
        ArrayList<FlowAction> actions = new ArrayList<FlowAction>();
        Random rand = new Random();
        final int naddrs = 5;
        int count = 0;
        do {
            long v = rand.nextLong() & MACADDR_MASK;
            if (v != 0) {
                byte[] addr = EtherAddress.toBytes(v);
                actions.add(new SetDlSrcAction(addr));
                count++;
            }
        } while (count < naddrs);

        count = 0;
        do {
            long v = rand.nextLong() & MACADDR_MASK;
            if (v != 0) {
                byte[] addr = EtherAddress.toBytes(v);
                actions.add(new SetDlDstAction(addr));
                count++;
            }
        } while (count < naddrs);

        byte[] priorities = {0, 4, 7};
        for (byte pri: priorities) {
            actions.add(new SetVlanPcpAction(pri));
        }

        count = 0;
        do {
            int v = rand.nextInt();
            byte[] addr = NumberUtils.toBytes(v);
            InetAddress iaddr = InetAddress.getByAddress(addr);
            actions.add(new SetInet4SrcAction(iaddr));
            count++;
        } while (count < naddrs);

        count = 0;
        do {
            int v = rand.nextInt();
            byte[] addr = NumberUtils.toBytes(v);
            InetAddress iaddr = InetAddress.getByAddress(addr);
            actions.add(new SetInet4DstAction(iaddr));
            count++;
        } while (count < naddrs);

        byte[] dscps = {0, 18, 32, 63};
        for (byte dscp: dscps) {
            actions.add(new SetDscpAction(dscp));
        }

        int[] ports = {0, 53, 200, 456, 20000, 40000, 65535};
        for (int port: ports) {
            actions.add(new SetTpSrcAction(port));
        }

        ports = new int[]{0, 31, 113, 789, 12345, 34567, 65535};
        for (int port: ports) {
            actions.add(new SetTpDstAction(port));
        }

        short[] types = {0, 63, 112, 255};
        for (short type: types) {
            actions.add(new SetIcmpTypeAction(type));
        }

        short[] codes = {0, 57, 128, 231, 255};
        for (short code: codes) {
            actions.add(new SetIcmpCodeAction(code));
        }

        for (FlowAction act: actions) {
            try {
                FlowActionImpl impl = FlowActionImpl.create(act);
                assertEquals(act, impl.getFlowAction());
                Class<?> cl = act.getClass();
                assertEquals(implClasses.get(cl), impl.getClass());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        // null action.
        try {
            FlowActionImpl.create(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Unsupported actions.
        FlowAction[] unsupported = {
            new DropAction(),
            new SetVlanIdAction((short)1),
            new PopVlanAction(),
        };
        for (FlowAction act: unsupported) {
            try {
                FlowActionImpl.create(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }

        // Ensure that we can catch an exception thrown by constructor.
        FlowAction[] invalid = {
            new SetDlSrcAction(new byte[0]),
            new SetDlDstAction(new byte[]{0, 0, 0, 0, 0, 0}),
            new SetVlanPcpAction((byte)-1),
            new SetInet4SrcAction((InetAddress)null),
            new SetInet4DstAction((InetAddress)null),
            new SetDscpAction((byte)64),
            new SetTpSrcAction(-1),
            new SetTpDstAction(0x10000),
            new SetIcmpTypeAction((short)256),
            new SetIcmpCodeAction((short)-1),
        };
        for (FlowAction act: invalid) {
            try {
                FlowActionImpl.create(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link FlowActionImpl#equals(Object)} and
     * {@link FlowActionImpl#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        byte[] addr = {0, 0, 0, 0, 0, 1};

        try {
            FlowActionImpl[] actions = {
                new SetDlSrcActionImpl(new SetDlSrcAction(addr)),
                new SetDlDstActionImpl(new SetDlDstAction(addr)),
                new SetVlanPcpActionImpl(new SetVlanPcpAction((byte)0)),
                new SetDscpActionImpl(new SetDscpAction((byte)0)),
                new SetIcmpTypeActionImpl(new SetIcmpTypeAction((short)0)),
                new SetIcmpCodeActionImpl(new SetIcmpCodeAction((short)0)),
            };

            // FlowActionImpl variants should be treated as different objects
            // if classes don't match.
            int count = 0;
            for (FlowActionImpl impl: actions) {
                assertTrue(set.add(impl));
                assertFalse(set.add(impl));
                count++;
                assertEquals(count, set.size());
            }

            for (FlowActionImpl impl: actions) {
                assertTrue(set.remove(impl));
                assertFalse(set.remove(impl));
                count--;
                assertEquals(count, set.size());
            }
            assertTrue(set.isEmpty());
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
