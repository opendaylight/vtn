/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import org.slf4j.Logger;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TxHook;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

/**
 * Unit Test for ReadTxContext.
 */
public class ReadTxContextTest extends TestBase {

    /**
     * Creates ReadTxContext with VTNManager object and ReadOnlyTransaction.
     */
    private ReadTxContext getReadTxContext() {
        VTNManagerProvider vtnProvider = mock(VTNManagerProvider.class);
        DataBroker dataBroker = mock(DataBroker.class);
        ReadOnlyTransaction transaction = mock(ReadOnlyTransaction.class);

        when(vtnProvider.getDataBroker()).thenReturn(dataBroker);
        when(dataBroker.newReadOnlyTransaction()).thenReturn(transaction);

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
     * Test case for {@link ReadTxContext#getReadSpecific(Class)}.
     */
    @Test
    public void testGetReadSpecific() {
        ReadTxContext rtc = getReadTxContext();
        ReadTransaction rtx = rtc.getTransaction();

        InventoryReader ir = rtc.getReadSpecific(InventoryReader.class);
        assertNotNull(ir);
        assertSame(rtx, ir.getReadTransaction());
        assertSame(ir, rtc.getReadSpecific(InventoryReader.class));

        FlowCondReader fcr = rtc.getReadSpecific(FlowCondReader.class);
        assertNotNull(fcr);
        assertSame(rtx, fcr.getReadTransaction());
        assertSame(fcr, rtc.getReadSpecific(FlowCondReader.class));
    }

    /**
     * Test case for {@link ReadTxContext#getSpecific(Class)}.
     */
    @Test
    public void testGetSpecific() {
        ReadTxContext rtc = getReadTxContext();
        try {
            rtc.getSpecific(Integer.class);
            unexpected();
        } catch (IllegalStateException e) {
            assertEquals("Read-only transaction.", e.getMessage());
        }
    }

    /**
     * Test case for {@link ReadTxContext#addPreSubmitHook(TxHook)}.
     */
    @Test
    public void testPreSubmitHook() {
        ReadTxContext rtc = getReadTxContext();
        TxHook hook = null;
        try {
            rtc.addPreSubmitHook(hook);
            unexpected();
        } catch (IllegalStateException e) {
            assertEquals("Read-only transaction.", e.getMessage());
        }
    }

    /**
     * Test case for {@link ReadTxContext#addPostSubmitHook(TxHook)}.
     */
    @Test
    public void testPostSubmitHook() {
        ReadTxContext rtc = getReadTxContext();
        TxHook hook = null;
        try {
            rtc.addPostSubmitHook(hook);
            unexpected();
        } catch (IllegalStateException e) {
            assertEquals("Read-only transaction.", e.getMessage());
        }
    }

    /**
     * Test case for {@link ReadTxContext#cancelTransaction()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCancelTransaction() throws Exception {
        // Without Transaction
        ReadTxContext rtc = getReadTxContext();
        rtc.cancelTransaction();
        ReadTransaction rtx = rtc.getTransaction();
        verifyNoMoreInteractions(rtx);
        assertEquals(rtx, getFieldValue(rtc, ReadOnlyTransaction.class,
                                        "transaction"));

        // With Transaction
        rtc.cancelTransaction();
        assertEquals(null, getFieldValue(rtc, ReadOnlyTransaction.class,
                                         "transaction"));
        assertTrue(rtx instanceof ReadOnlyTransaction);
        verify((ReadOnlyTransaction)rtx).close();
        verifyNoMoreInteractions(rtx);
    }

    /**
     * Test case for {@link ReadTxContext#log(Logger, VTNLogLevel, String)}.
     */
    @Test
    public void testLog1() {
        ReadTxContext rtc = getReadTxContext();

        Logger logger = mock(Logger.class);
        VTNLogLevel level = VTNLogLevel.ERROR;
        String msg = "log message";
        rtc.log(logger, level, msg);
        verify(logger).error(msg);
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for
     * {@link ReadTxContext#log(Logger, VTNLogLevel, String, Object[])}.
     */
    @Test
    public void testLog2() {
        ReadTxContext rtc = getReadTxContext();

        Logger logger = mock(Logger.class);
        VTNLogLevel level = VTNLogLevel.ERROR;
        String msg = "log message:{}, {}";
        Integer arg1 = 10;
        String arg2 = "arg2";
        Object[] args = {arg1, arg2};
        rtc.log(logger, level, msg, args);
        verify(logger).error(msg, args);
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for
     * {@link ReadTxContext#log(Logger, VTNLogLevel, String, Throwable)}.
     */
    @Test
    public void testLog3() {
        ReadTxContext rtc = getReadTxContext();

        Logger logger = mock(Logger.class);
        VTNLogLevel level = VTNLogLevel.ERROR;
        String msg = "log message";
        IllegalArgumentException iae = new IllegalArgumentException();
        rtc.log(logger, level, msg, iae);
        verify(logger).error(msg, iae);
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for
     * {@link ReadTxContext#log(Logger, VTNLogLevel, Throwable, String, Object[])}.
     */
    @Test
    public void testLog4() {
        ReadTxContext rtc = getReadTxContext();

        Logger logger = mock(Logger.class);
        when(logger.isErrorEnabled()).thenReturn(true);

        VTNLogLevel level = VTNLogLevel.ERROR;
        String fmt = "log message: %s, %s";
        IllegalArgumentException iae = new IllegalArgumentException();
        Integer arg1 = 23;
        String arg2 = "arg2";
        String msg = "log message: " + arg1 + ", " + arg2;
        rtc.log(logger, level, iae, fmt, arg1, arg2);
        verify(logger).isErrorEnabled();
        verify(logger).error(msg, iae);
        verifyNoMoreInteractions(logger);
    }
}
