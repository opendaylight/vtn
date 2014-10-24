/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.junit.Test;

/**
 * JUnit test for {@link VNodeState}.
 */
public class VNodeStateTest extends TestBase {
    /**
     * Test case for enum values.
     */
    @Test
    public void testEnum() {
        VNodeState[] values = VNodeState.values();
        assertEquals(3, values.length);
        assertEquals(VNodeState.UNKNOWN, values[0]);
        assertEquals(VNodeState.DOWN, values[1]);
        assertEquals(VNodeState.UP, values[2]);
        assertEquals(0, VNodeState.UNKNOWN.ordinal());
        assertEquals(1, VNodeState.DOWN.ordinal());
        assertEquals(2, VNodeState.UP.ordinal());
        assertEquals(-1, VNodeState.UNKNOWN.getValue());
        assertEquals(0, VNodeState.DOWN.getValue());
        assertEquals(1, VNodeState.UP.getValue());
    }

    /**
     * Test case for {@link VNodeState#valueOf(int)}.
     */
    @Test
    public void testValueOf() {
        int[] unknown = {
            Integer.MIN_VALUE, -100000, -40000, -12345, -333, -15, -3, -2, -1,
            2, 3, 100, 4444, 50000, 678901, 12345678, Integer.MAX_VALUE,
        };
        for (int st: unknown) {
            assertEquals(VNodeState.UNKNOWN, VNodeState.valueOf(st));
        }

        assertEquals(VNodeState.DOWN, VNodeState.valueOf(0));
        assertEquals(VNodeState.UP, VNodeState.valueOf(1));
    }

    /**
     * Ensure that {@link VNodeState} is serializable.
     */
    @Test
    public void testSerialize() {
        for (VNodeState state: VNodeState.values()) {
            serializeTest(state);
        }
    }
}
