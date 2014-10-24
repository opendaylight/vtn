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
 * JUnit test for {@link UpdateOperation}.
 */
public class UpdateOperationTest extends TestBase {
    /**
     * Test case for enum values.
     */
    @Test
    public void testEnum() {
        UpdateOperation[] values = UpdateOperation.values();
        assertEquals(3, values.length);
        assertEquals(UpdateOperation.SET, values[0]);
        assertEquals(UpdateOperation.ADD, values[1]);
        assertEquals(UpdateOperation.REMOVE, values[2]);
        assertEquals(0, UpdateOperation.SET.ordinal());
        assertEquals(1, UpdateOperation.ADD.ordinal());
        assertEquals(2, UpdateOperation.REMOVE.ordinal());
    }

    /**
     * Ensure that {@link UpdateOperation} is serializable.
     */
    @Test
    public void testSerialize() {
        for (UpdateOperation op: UpdateOperation.values()) {
            serializeTest(op);
        }
    }
}
