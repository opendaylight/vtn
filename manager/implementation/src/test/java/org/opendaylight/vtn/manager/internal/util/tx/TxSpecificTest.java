/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import static org.mockito.Mockito.mock;

import java.lang.reflect.InvocationTargetException;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

/**
 * JUnit test for {@link TxSpecific}.
 */
public class TxSpecificTest extends TestBase {
    /**
     * Test class that cannot be instantiated.
     */
    public static final class InvalidData {
        /**
         * A {@link IllegalArgumentException} is always thrown.
         *
         * @param rtx  A {@link ReadTransaction} instance.
         */
        public InvalidData(ReadTransaction rtx) {
            throw new IllegalArgumentException();
        }
    }

    /**
     * Test case for {@link TxSpecific#get(Class, Object)} and
     * {@link TxSpecific#clear()}.
     */
    @Test
    public void testGet() {
        TxSpecific<ReadTransaction> spec =
            new TxSpecific<>(ReadTransaction.class);

        ReadTransaction rtx = mock(ReadTransaction.class);

        // Get InventoryReader.
        InventoryReader ir = spec.get(InventoryReader.class, rtx);
        assertNotNull(ir);
        assertSame(rtx, ir.getReadTransaction());
        for (int i = 0; i < 10; i++) {
            assertSame(ir, spec.get(InventoryReader.class, rtx));
        }

        // Get FlowCondReader.
        FlowCondReader fcr = spec.get(FlowCondReader.class, rtx);
        assertNotNull(fcr);
        assertSame(rtx, fcr.getReadTransaction());
        for (int i = 0; i < 10; i++) {
            assertSame(fcr, spec.get(FlowCondReader.class, rtx));
        }

        // Clear cache.
        spec.clear();

        // Get data again.
        InventoryReader ir1 = spec.get(InventoryReader.class, rtx);
        assertNotNull(ir1);
        assertNotSame(ir, ir1);
        assertSame(rtx, ir1.getReadTransaction());
        for (int i = 0; i < 10; i++) {
            assertSame(ir1, spec.get(InventoryReader.class, rtx));
        }

        FlowCondReader fcr1 = spec.get(FlowCondReader.class, rtx);
        assertNotNull(fcr1);
        assertNotSame(fcr, fcr1);
        assertSame(rtx, fcr1.getReadTransaction());
        for (int i = 0; i < 10; i++) {
            assertSame(fcr1, spec.get(FlowCondReader.class, rtx));
        }

        // Invalid data type.
        String msg = "Unable to instantiate transaction specific data: ";
        try {
            spec.get(Integer.class, rtx);
            unexpected();
        } catch (IllegalStateException e) {
            assertTrue(e.getMessage().startsWith(msg));
            assertTrue(e.getCause() instanceof NoSuchMethodException);
        }

        // Unable to instantiate.
        try {
            spec.get(InvalidData.class, rtx);
            unexpected();
        } catch (IllegalStateException e) {
            assertTrue(e.getMessage().startsWith(msg));
            Throwable cause = e.getCause();
            assertTrue(cause instanceof InvocationTargetException);
            InvocationTargetException ite = (InvocationTargetException)cause;
            assertTrue(ite.getCause() instanceof
                       IllegalArgumentException);
        }
    }
}
