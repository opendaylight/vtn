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
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;

import org.opendaylight.vtn.manager.internal.TestBase;

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
