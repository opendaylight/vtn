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
 * JUnit test for {@link FixedLogger}.
 */
public class FixedLoggerTest extends TestBase {
    /**
     * Test case for ERROR level.
     */
    @Test
    public void testError() {
        Logger mock = Mockito.mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.ERROR);
        assertSame(mock, logger.getLogger());

        // isEnabled()
        Mockito.when(mock.isErrorEnabled()).thenReturn(true).thenReturn(false);
        assertEquals(true, logger.isEnabled());
        assertEquals(false, logger.isEnabled());
        Mockito.verify(mock, Mockito.times(2)).isErrorEnabled();
        Mockito.reset(mock);

        // log(Throwable, String, Object[])
        Mockito.when(mock.isErrorEnabled()).thenReturn(true).thenReturn(true).
            thenReturn(false).thenReturn(false).thenReturn(true);
        IllegalArgumentException iae = new IllegalArgumentException();
        String msg = "Illegal argument";
        logger.log(iae, msg);
        Mockito.verify(mock).isErrorEnabled();
        Mockito.verify(mock).error(msg, iae);
        logger.log(iae, "Illegal argument: i=%d, str=%s", 10, "test message");
        String formatted = "Illegal argument: i=10, str=test message";
        Mockito.verify(mock, Mockito.times(2)).isErrorEnabled();
        Mockito.verify(mock).error(msg, iae);
        Mockito.verify(mock).error(formatted, iae);

        logger.log(iae, msg);
        Mockito.verify(mock, Mockito.times(3)).isErrorEnabled();
        Mockito.verify(mock).error(msg, iae);
        Mockito.verify(mock).error(formatted, iae);

        logger.log(iae, "This should not be logger: %d", 12345);
        Mockito.verify(mock, Mockito.times(4)).isErrorEnabled();
        Mockito.verify(mock).error(msg, iae);
        Mockito.verify(mock).error(formatted, iae);

        String msg1 = "Another message";
        logger.log((Throwable)null, msg1, (Object[])null);
        Mockito.verify(mock, Mockito.times(5)).isErrorEnabled();
        Mockito.verify(mock).error(msg, iae);
        Mockito.verify(mock).error(formatted, iae);
        Mockito.verify(mock).error(msg1);
        Mockito.reset(mock);

        // log(String)
        msg = "This is a log message.";
        logger.log(msg);
        Mockito.verify(mock).error(msg);

        // log(String, Throwable)
        Exception e = new Exception();
        logger.log(msg, e);
        Mockito.verify(mock).error(msg);
        Mockito.verify(mock).error(msg, e);

        // log(String, Object ...)
        String format = "This a log message: {}, {}, {}";
        logger.log(format, 123, "test", 99999999L);
        Mockito.verify(mock).error(msg);
        Mockito.verify(mock).error(msg, e);
        Mockito.verify(mock).error(format, 123, "test", 99999999L);
    }

    /**
     * Test case for WARN level.
     */
    @Test
    public void testWarn() {
        Logger mock = Mockito.mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.WARN);
        assertSame(mock, logger.getLogger());

        // isEnabled()
        Mockito.when(mock.isWarnEnabled()).thenReturn(true).thenReturn(false);
        assertEquals(true, logger.isEnabled());
        assertEquals(false, logger.isEnabled());
        Mockito.verify(mock, Mockito.times(2)).isWarnEnabled();
        Mockito.reset(mock);

        // log(Throwable, String, Object[])
        Mockito.when(mock.isWarnEnabled()).thenReturn(true).thenReturn(true).
            thenReturn(false).thenReturn(false).thenReturn(true);
        IllegalArgumentException iae = new IllegalArgumentException();
        String msg = "Illegal argument";
        logger.log(iae, msg);
        Mockito.verify(mock).isWarnEnabled();
        Mockito.verify(mock).warn(msg, iae);
        logger.log(iae, "Illegal argument: i=%d, str=%s", 10, "test message");
        String formatted = "Illegal argument: i=10, str=test message";
        Mockito.verify(mock, Mockito.times(2)).isWarnEnabled();
        Mockito.verify(mock).warn(msg, iae);
        Mockito.verify(mock).warn(formatted, iae);

        logger.log(iae, msg);
        Mockito.verify(mock, Mockito.times(3)).isWarnEnabled();
        Mockito.verify(mock).warn(msg, iae);
        Mockito.verify(mock).warn(formatted, iae);

        logger.log(iae, "This should not be logger: %d", 12345);
        Mockito.verify(mock, Mockito.times(4)).isWarnEnabled();
        Mockito.verify(mock).warn(msg, iae);
        Mockito.verify(mock).warn(formatted, iae);

        String msg1 = "Another message";
        logger.log((Throwable)null, msg1, (Object[])null);
        Mockito.verify(mock, Mockito.times(5)).isWarnEnabled();
        Mockito.verify(mock).warn(msg, iae);
        Mockito.verify(mock).warn(formatted, iae);
        Mockito.verify(mock).warn(msg1);
        Mockito.reset(mock);

        // log(String)
        msg = "This is a log message.";
        logger.log(msg);
        Mockito.verify(mock).warn(msg);

        // log(String, Throwable)
        Exception e = new Exception();
        logger.log(msg, e);
        Mockito.verify(mock).warn(msg);
        Mockito.verify(mock).warn(msg, e);

        // log(String, Object ...)
        String format = "This a log message: {}, {}, {}";
        logger.log(format, 123, "test", 99999999L);
        Mockito.verify(mock).warn(msg);
        Mockito.verify(mock).warn(msg, e);
        Mockito.verify(mock).warn(format, 123, "test", 99999999L);
    }

    /**
     * Test case for INFO level.
     */
    @Test
    public void testInfo() {
        Logger mock = Mockito.mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.INFO);
        assertSame(mock, logger.getLogger());

        // isEnabled()
        Mockito.when(mock.isInfoEnabled()).thenReturn(true).thenReturn(false);
        assertEquals(true, logger.isEnabled());
        assertEquals(false, logger.isEnabled());
        Mockito.verify(mock, Mockito.times(2)).isInfoEnabled();
        Mockito.reset(mock);

        // log(Throwable, String, Object[])
        Mockito.when(mock.isInfoEnabled()).thenReturn(true).thenReturn(true).
            thenReturn(false).thenReturn(false).thenReturn(true);
        IllegalArgumentException iae = new IllegalArgumentException();
        String msg = "Illegal argument";
        logger.log(iae, msg);
        Mockito.verify(mock).isInfoEnabled();
        Mockito.verify(mock).info(msg, iae);
        logger.log(iae, "Illegal argument: i=%d, str=%s", 10, "test message");
        String formatted = "Illegal argument: i=10, str=test message";
        Mockito.verify(mock, Mockito.times(2)).isInfoEnabled();
        Mockito.verify(mock).info(msg, iae);
        Mockito.verify(mock).info(formatted, iae);

        logger.log(iae, msg);
        Mockito.verify(mock, Mockito.times(3)).isInfoEnabled();
        Mockito.verify(mock).info(msg, iae);
        Mockito.verify(mock).info(formatted, iae);

        logger.log(iae, "This should not be logger: %d", 12345);
        Mockito.verify(mock, Mockito.times(4)).isInfoEnabled();
        Mockito.verify(mock).info(msg, iae);
        Mockito.verify(mock).info(formatted, iae);

        String msg1 = "Another message";
        logger.log((Throwable)null, msg1, (Object[])null);
        Mockito.verify(mock, Mockito.times(5)).isInfoEnabled();
        Mockito.verify(mock).info(msg, iae);
        Mockito.verify(mock).info(formatted, iae);
        Mockito.verify(mock).info(msg1);
        Mockito.reset(mock);

        // log(String)
        msg = "This is a log message.";
        logger.log(msg);
        Mockito.verify(mock).info(msg);

        // log(String, Throwable)
        Exception e = new Exception();
        logger.log(msg, e);
        Mockito.verify(mock).info(msg);
        Mockito.verify(mock).info(msg, e);

        // log(String, Object ...)
        String format = "This a log message: {}, {}, {}";
        logger.log(format, 123, "test", 99999999L);
        Mockito.verify(mock).info(msg);
        Mockito.verify(mock).info(msg, e);
        Mockito.verify(mock).info(format, 123, "test", 99999999L);
    }

    /**
     * Test case for DEBUG level.
     */
    @Test
    public void testDebug() {
        Logger mock = Mockito.mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.DEBUG);
        assertSame(mock, logger.getLogger());

        // isEnabled()
        Mockito.when(mock.isDebugEnabled()).thenReturn(true).thenReturn(false);
        assertEquals(true, logger.isEnabled());
        assertEquals(false, logger.isEnabled());
        Mockito.verify(mock, Mockito.times(2)).isDebugEnabled();
        Mockito.reset(mock);

        // log(Throwable, String, Object[])
        Mockito.when(mock.isDebugEnabled()).thenReturn(true).thenReturn(true).
            thenReturn(false).thenReturn(false).thenReturn(true);
        IllegalArgumentException iae = new IllegalArgumentException();
        String msg = "Illegal argument";
        logger.log(iae, msg);
        Mockito.verify(mock).isDebugEnabled();
        Mockito.verify(mock).debug(msg, iae);
        logger.log(iae, "Illegal argument: i=%d, str=%s", 10, "test message");
        String formatted = "Illegal argument: i=10, str=test message";
        Mockito.verify(mock, Mockito.times(2)).isDebugEnabled();
        Mockito.verify(mock).debug(msg, iae);
        Mockito.verify(mock).debug(formatted, iae);

        logger.log(iae, msg);
        Mockito.verify(mock, Mockito.times(3)).isDebugEnabled();
        Mockito.verify(mock).debug(msg, iae);
        Mockito.verify(mock).debug(formatted, iae);

        logger.log(iae, "This should not be logger: %d", 12345);
        Mockito.verify(mock, Mockito.times(4)).isDebugEnabled();
        Mockito.verify(mock).debug(msg, iae);
        Mockito.verify(mock).debug(formatted, iae);

        String msg1 = "Another message";
        logger.log((Throwable)null, msg1, (Object[])null);
        Mockito.verify(mock, Mockito.times(5)).isDebugEnabled();
        Mockito.verify(mock).debug(msg, iae);
        Mockito.verify(mock).debug(formatted, iae);
        Mockito.verify(mock).debug(msg1);
        Mockito.reset(mock);

        // log(String)
        msg = "This is a log message.";
        logger.log(msg);
        Mockito.verify(mock).debug(msg);

        // log(String, Throwable)
        Exception e = new Exception();
        logger.log(msg, e);
        Mockito.verify(mock).debug(msg);
        Mockito.verify(mock).debug(msg, e);

        // log(String, Object ...)
        String format = "This a log message: {}, {}, {}";
        logger.log(format, 123, "test", 99999999L);
        Mockito.verify(mock).debug(msg);
        Mockito.verify(mock).debug(msg, e);
        Mockito.verify(mock).debug(format, 123, "test", 99999999L);
    }

    /**
     * Test case for TRACE level.
     */
    @Test
    public void testTrace() {
        Logger mock = Mockito.mock(Logger.class);
        FixedLogger logger = new FixedLogger(mock, VTNLogLevel.TRACE);
        assertSame(mock, logger.getLogger());

        // isEnabled()
        Mockito.when(mock.isTraceEnabled()).thenReturn(true).thenReturn(false);
        assertEquals(true, logger.isEnabled());
        assertEquals(false, logger.isEnabled());
        Mockito.verify(mock, Mockito.times(2)).isTraceEnabled();
        Mockito.reset(mock);

        // log(Throwable, String, Object[])
        Mockito.when(mock.isTraceEnabled()).thenReturn(true).thenReturn(true).
            thenReturn(false).thenReturn(false).thenReturn(true);
        IllegalArgumentException iae = new IllegalArgumentException();
        String msg = "Illegal argument";
        logger.log(iae, msg);
        Mockito.verify(mock).isTraceEnabled();
        Mockito.verify(mock).trace(msg, iae);
        logger.log(iae, "Illegal argument: i=%d, str=%s", 10, "test message");
        String formatted = "Illegal argument: i=10, str=test message";
        Mockito.verify(mock, Mockito.times(2)).isTraceEnabled();
        Mockito.verify(mock).trace(msg, iae);
        Mockito.verify(mock).trace(formatted, iae);

        logger.log(iae, msg);
        Mockito.verify(mock, Mockito.times(3)).isTraceEnabled();
        Mockito.verify(mock).trace(msg, iae);
        Mockito.verify(mock).trace(formatted, iae);

        logger.log(iae, "This should not be logger: %d", 12345);
        Mockito.verify(mock, Mockito.times(4)).isTraceEnabled();
        Mockito.verify(mock).trace(msg, iae);
        Mockito.verify(mock).trace(formatted, iae);

        String msg1 = "Another message";
        logger.log((Throwable)null, msg1, (Object[])null);
        Mockito.verify(mock, Mockito.times(5)).isTraceEnabled();
        Mockito.verify(mock).trace(msg, iae);
        Mockito.verify(mock).trace(formatted, iae);
        Mockito.verify(mock).trace(msg1);
        Mockito.reset(mock);

        // log(String)
        msg = "This is a log message.";
        logger.log(msg);
        Mockito.verify(mock).trace(msg);

        // log(String, Throwable)
        Exception e = new Exception();
        logger.log(msg, e);
        Mockito.verify(mock).trace(msg);
        Mockito.verify(mock).trace(msg, e);

        // log(String, Object ...)
        String format = "This a log message: {}, {}, {}";
        logger.log(format, 123, "test", 99999999L);
        Mockito.verify(mock).trace(msg);
        Mockito.verify(mock).trace(msg, e);
        Mockito.verify(mock).trace(format, 123, "test", 99999999L);
    }
}
