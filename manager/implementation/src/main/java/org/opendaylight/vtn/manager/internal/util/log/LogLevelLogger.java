/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.log;

import org.slf4j.Logger;

/**
 * {@code LogLevelLogger} describes a set of logging APIs to be implemented
 * by {@link VTNLogLevel}.
 */
interface LogLevelLogger {
    /**
     * Determine whether the logging is enabled or not.
     *
     * @param logger  A {@link Logger} instance.
     * @return  {@code true} only if the logging is enabled.
     */
    boolean isEnabled(Logger logger);

    /**
     * Log the specified message.
     *
     * @param logger  A {@link Logger} instance.
     * @param msg     A message to be logged.
     */
    void log(Logger logger, String msg);

    /**
     * Log a message according to the specified format string and arguments.
     *
     * <p>
     *   This method constructs a message using SLF4J log format.
     * </p>
     *
     * @param logger  A {@link Logger} instance.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    void log(Logger logger, String format, Object ... args);

    /**
     * Log the specified message and exception.
     *
     * @param logger  A {@link Logger} instance.
     * @param msg     A message to be logged.
     * @param t       A {@link Throwable} to be logged.
     */
    void log(Logger logger, String msg, Throwable t);

    /**
     * Log the specified throwable and a message according to the given
     * format string and arguments.
     *
     * <p>
     *   Note that this method constructs a log message using
     *   {@link String#format(String, Object[])}.
     * </p>
     *
     * @param logger  A {@link Logger} instance.
     * @param t       A {@link Throwable} to be logged.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    void log(Logger logger, Throwable t, String format, Object ... args);
}
