/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.log;

/**
 * {@code LogRecord} describes a system log record.
 */
public final class LogRecord {
    /**
     * A {@link FixedLogger} instance which specifies the logger and logging
     * level.
     */
    private final FixedLogger  logger;

    /**
     * A format string used to construct log message.
     */
    private final String  logFormat;

    /**
     * An object array used to construct log message.
     */
    private final Object[]  logArguments;

    /**
     * A {@link Throwable} instance to be logged.
     */
    private final Throwable  logCause;

    /**
     * Construct a new instance.
     *
     * @param log   A {@link FixedLogger} instance which specifies the logger
     *              and logging level.
     * @param msg   A message to be logged.
     */
    public LogRecord(FixedLogger log, String msg) {
        this(log, null, msg, null);
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   Note that a log message is constructed using
     *   {@link String#format(String, Object[])}.
     * </p>
     *
     * @param log     A {@link FixedLogger} instance which specifies the logger
     *                and logging level.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    public LogRecord(FixedLogger log, String format, Object ... args) {
        this(log, null, format, args);
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   Note that a log message is constructed using
     *   {@link String#format(String, Object[])}.
     * </p>
     *
     * @param log     A {@link FixedLogger} instance which specifies the logger
     *                and logging level.
     * @param t       A {@link Throwable} to be logged.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    public LogRecord(FixedLogger log, Throwable t, String format,
                     Object ... args) {
        logger = log;
        logFormat = format;
        logArguments = (args == null) ? null : args.clone();
        logCause = t;
    }

    /**
     * Send this log record to the logger.
     */
    public void log() {
        logger.log(logCause, logFormat, logArguments);
    }
}
