/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.log;

import org.slf4j.Logger;

/**
 * {@code FixedLogger} is a wrapper for {@link Logger} that binds the
 * specified logger to the specific level.
 */
public final class FixedLogger {
    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * Logging level.
     */
    private final VTNLogLevel  logLevel;

    /**
     * Construct a new instance.
     *
     * @param log    A {@link Logger} instance.
     * @param level  A {@link VTNLogLevel} instance.
     */
    public FixedLogger(Logger log, VTNLogLevel level) {
        logger = log;
        logLevel = level;
    }

    /**
     * Return the logger instance.
     *
     * @return  A {@link Logger} instance.
     */
    public Logger getLogger() {
        return logger;
    }

    /**
     * Return the logging level.
     *
     * @return  A {@link VTNLogLevel} instance.
     */
    public VTNLogLevel getLogLevel() {
        return logLevel;
    }

    /**
     * Determine whether the logging is enabled or not.
     *
     * @return  {@code true} only if the logging is enabled.
     */
    public boolean isEnabled() {
        return logLevel.isEnabled(logger);
    }

    /**
     * Log the specified message.
     *
     * @param msg  A message to be logged.
     */
    public void log(String msg) {
        logLevel.log(logger, msg);
    }

    /**
     * Log a message according to the specified format string and arguments.
     *
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    public void log(String format, Object ... args) {
        logLevel.log(logger, format, args);
    }

    /**
     * Log the specified message and exception.
     *
     * @param msg  A message to be logged.
     * @param t    A {@link Throwable} to be logged.
     */
    public void log(String msg, Throwable t) {
        logLevel.log(logger, msg, t);
    }

    /**
     * Log the specified throwable and a message according to the given
     * format string and arguments.
     *
     * <p>
     *   Note that this method constructs a log message using
     *   {@link String#format(String, Object[])}.
     * </p>
     *
     * @param t       A {@link Throwable} to be logged.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    public void log(Throwable t, String format, Object ... args) {
        logLevel.log(logger, t, format, args);
    }
}
