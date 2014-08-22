/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;

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
