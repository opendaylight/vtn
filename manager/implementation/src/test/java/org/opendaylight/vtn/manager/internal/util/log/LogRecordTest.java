/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.log;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import org.slf4j.Logger;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link LogRecord}.
 */
public class LogRecordTest extends TestBase {
    /**
     * Test case for {@link LogRecord#LogRecord(FixedLogger,String)}.
     */
    @Test
    public void testSimple() {
        Logger mock = mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.ERROR);

        String msg = "A log message.";
        LogRecord lr = new LogRecord(logger, msg);
        int count = 5;
        for (int i = 0; i < count; i++) {
            lr.log();
        }

        verify(mock, times(count)).error(msg);
        verifyNoMoreInteractions(mock);
    }

    /**
     * Test case for {@link LogRecord#LogRecord(FixedLogger,String,Object[])}.
     */
    @Test
    public void testFormatted() {
        Logger mock = mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.ERROR);

        String msg = "A log message: {}, {}";
        int arg1 = 100;
        String arg2 = "argument(2)";
        Object[] args = {arg1, arg2};
        LogRecord lr = new LogRecord(logger, msg, arg1, arg2);
        int count = 5;
        for (int i = 0; i < count; i++) {
            lr.log();
        }

        verify(mock, times(count)).error(msg, args);
        verifyNoMoreInteractions(mock);

        mock = mock(Logger.class);
        logger = new FixedLogger(mock, VTNLogLevel.WARN);
        msg = "A log message.";
        args = null;
        lr = new LogRecord(logger, msg, args);
        for (int i = 0; i < count; i++) {
            lr.log();
        }

        verify(mock, times(count)).warn(msg);
        verifyNoMoreInteractions(mock);
    }

    /**
     * Test case for {@link LogRecord#LogRecord(FixedLogger,String,Throwable)}.
     */
    @Test
    public void testThrowable() {
        Logger mock = mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.ERROR);

        String msg = "A log message: {}, {}";
        IllegalStateException ise = new IllegalStateException();
        LogRecord lr = new LogRecord(logger, msg, ise);
        int count = 5;
        for (int i = 0; i < count; i++) {
            lr.log();
        }

        verify(mock, times(count)).error(msg, ise);
        verifyNoMoreInteractions(mock);
    }

    /**
     * Test case for
     * {@link LogRecord#LogRecord(FixedLogger,Throwable,String,Object[])}.
     */
    @Test
    public void testFormattedThrowable() {
        Logger mock = mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.ERROR);

        when(mock.isErrorEnabled()).thenReturn(true);

        String msg = "A log message: %d, %s, 0x%x";
        int arg1 = 100;
        String arg2 = "argument(2)";
        long arg3 = 0x123456789AL;
        IllegalStateException ise = new IllegalStateException("Illegal state");
        String formatted = "A log message: 100, argument(2), 0x123456789a";
        LogRecord lr = new LogRecord(logger, ise, msg, arg1, arg2, arg3);
        lr.log();
        verify(mock).isErrorEnabled();
        verify(mock).error(formatted, ise);
        verifyNoMoreInteractions(mock);
        reset(mock);

        // In case where logging is disabled.
        when(mock.isErrorEnabled()).thenReturn(false);
        lr.log();
        verify(mock).isErrorEnabled();
        verifyNoMoreInteractions(mock);
        reset(mock);

        mock = mock(Logger.class);
        when(mock.isInfoEnabled()).thenReturn(true);
        logger = new FixedLogger(mock, VTNLogLevel.INFO);
        msg = "A log message.";
        lr = new LogRecord(logger, ise, msg, (Object[])null);
        lr.log();
        verify(mock).isInfoEnabled();
        verify(mock).info(msg, ise);
        verifyNoMoreInteractions(mock);
    }
}
