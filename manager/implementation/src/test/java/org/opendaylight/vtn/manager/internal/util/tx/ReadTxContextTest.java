/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;


/**
 * Unit Test for ReadTxContext.
 */
public class ReadTxContextTest extends TestBase {

    /**
     * Creates ReadTxContext with VTNManager object and ReadOnlyTransaction.
     */
    private ReadTxContext getReadTxContext() {
        ReadOnlyTransaction tx = null;
        VTNManagerProvider vtnProvider = Mockito.mock(VTNManagerProvider.class);
        DataBroker dataBroker = Mockito.mock(DataBroker.class);
        ReadOnlyTransaction transaction = Mockito.mock(ReadOnlyTransaction.class);

        Mockito.when(vtnProvider.getDataBroker()).thenReturn(dataBroker);
        Mockito.when(dataBroker.newReadOnlyTransaction()).thenReturn(transaction);

        return new ReadTxContext(vtnProvider);
    }

    /**
     * Test Case for getProvider() and getReadWriteTransaction().
     */
    @Test
    public void testReadTxContext()  {
        // Validating VTNProvider with null
        VTNManagerProvider  vtnProvider = null;
        ReadTxContext rtc = new ReadTxContext(vtnProvider);
        assertEquals(rtc.getProvider(), vtnProvider);

        // getReadWriteTransaction
        try {
            rtc.getReadWriteTransaction();
        } catch (IllegalStateException illegalStateException) {
            assertEquals(illegalStateException.getMessage(), "Read-only transaction.");
        }
    }

    /**
     * Test Case for getTransaction.
     */
    @Test
    public void testGetTransaction()  {
        // Validating with valid VTNManagerProvider and with empty Transaction in ReadTxContext
        ReadTxContext rtc = getReadTxContext();
        assertNotNull(rtc.getTransaction());

        // Validating with valid VTNManagerProvider and with non empty Transaction in ReadTxContext
        assertNotNull(rtc.getTransaction());
    }

    /**
     * Test Case for getInventoryReader.
     */
    @Test
    public void testGetInventoryReader() {
        // Validating with valid VTNManagerProvider and with empty InventoryReader in ReadTxContext
        ReadTxContext rtc = getReadTxContext();
        assertNotNull(rtc.getInventoryReader());

        // Validating with valid VTNManagerProvider and with non empty InventoryReader in ReadTxContext
        assertNotNull(rtc.getInventoryReader());
    }

    /**
     * Test Case for getFlowCondReader.
     */
    @Test
    public void testGetFlowCondReader() {
        // Validating with valid VTNManagerProvider and with empty FlowCondReader in ReadTxContext
        ReadTxContext rtc = getReadTxContext();
        assertNotNull(rtc.getFlowCondReader());

        // Validating with valid VTNManagerProvider and with non empty FlowCondReader in ReadTxContext
        assertNotNull(rtc.getFlowCondReader());
    }

    /**
     * Test Case for cancelTransaction.
     */
    @Test
    public void testCancelTransaction() {
        // Without Transaction
        ReadTxContext rtc = getReadTxContext();
        rtc.cancelTransaction();

        // With Transaction
        rtc.getTransaction();
        rtc.cancelTransaction();
    }
}
