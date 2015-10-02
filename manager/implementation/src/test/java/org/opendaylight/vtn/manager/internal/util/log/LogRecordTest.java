/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.log;

import org.slf4j.Logger;

import org.junit.Test;

import org.mockito.Mockito;

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
        Logger mock = Mockito.mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.ERROR);

        Mockito.when(mock.isErrorEnabled()).thenReturn(false).thenReturn(true);

        String msg = "A log message.";
        LogRecord lr = new LogRecord(logger, msg);
        lr.log();
        Mockito.verify(mock).isErrorEnabled();
        Mockito.verify(mock, Mockito.never()).error(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).info(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).trace(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.anyVararg());

        lr.log();
        Mockito.verify(mock, Mockito.times(2)).isErrorEnabled();
        Mockito.verify(mock).error(msg);
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).info(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).trace(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.anyVararg());
    }

    /**
     * Test case for {@link LogRecord#LogRecord(FixedLogger,String,Object[])}.
     */
    @Test
    public void testFormatted() {
        Logger mock = Mockito.mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.ERROR);

        Mockito.when(mock.isErrorEnabled()).thenReturn(false).thenReturn(true);

        String msg = "A log message: %d, %s";
        int arg1 = 100;
        String arg2 = "argument(2)";
        String formatted = "A log message: 100, argument(2)";
        LogRecord lr = new LogRecord(logger, msg, arg1, arg2);
        lr.log();
        Mockito.verify(mock).isErrorEnabled();
        Mockito.verify(mock, Mockito.never()).error(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).info(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).trace(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.anyVararg());

        lr.log();
        Mockito.verify(mock, Mockito.times(2)).isErrorEnabled();
        Mockito.verify(mock).error(formatted);
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).info(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).trace(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.anyVararg());
    }


    /**
     * Test case for {@link LogRecord#LogRecord(FixedLogger,Throwable,String,Object[])}.
     */
    @Test
    public void testThrowable() {
        Logger mock = Mockito.mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.ERROR);

        Mockito.when(mock.isErrorEnabled()).thenReturn(false).thenReturn(true);

        String msg = "A log message: %d, %s, 0x%x";
        int arg1 = 100;
        String arg2 = "argument(2)";
        long arg3 = 0x123456789AL;
        IllegalStateException ise = new IllegalStateException("Illegal state");
        String formatted = "A log message: 100, argument(2), 0x123456789a";
        LogRecord lr = new LogRecord(logger, ise, msg, arg1, arg2, arg3);
        lr.log();
        Mockito.verify(mock).isErrorEnabled();
        Mockito.verify(mock, Mockito.never()).error(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).info(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).trace(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.anyVararg());

        lr.log();
        Mockito.verify(mock, Mockito.times(2)).isErrorEnabled();
        Mockito.verify(mock, Mockito.never()).error(Mockito.anyString());
        Mockito.verify(mock).error(formatted, ise);
        Mockito.verify(mock, Mockito.never()).
            error(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).warn(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            warn(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).info(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            info(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).debug(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            debug(Mockito.anyString(), Mockito.anyVararg());
        Mockito.verify(mock, Mockito.never()).trace(Mockito.anyString());
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.any(Throwable.class));
        Mockito.verify(mock, Mockito.never()).
            trace(Mockito.anyString(), Mockito.anyVararg());
    }
}
