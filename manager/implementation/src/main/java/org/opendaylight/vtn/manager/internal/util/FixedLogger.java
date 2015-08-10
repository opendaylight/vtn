/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.slf4j.Logger;

/**
 * {@code FixedLogger} is a wrapper for {@link Logger} that binds the
 * specified logger to the specific level.
 */
public abstract class FixedLogger {
    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * Construct a new instance.
     *
     * @param log  A {@link Logger} instance.
     */
    FixedLogger(Logger log) {
        logger = log;
    }

    /**
     * Return the logger instance.
     *
     * @return  A {@link Logger} instance.
     */
    public final Logger getLogger() {
        return logger;
    }

    /**
     * Log the specifiedexception and a message according to the given
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
    public final void log(Throwable t, String format, Object ... args) {
        if (isEnabled()) {
            String msg = (args == null) ? format : String.format(format, args);
            if (t == null) {
                log(msg);
            } else {
                log(msg, t);
            }
        }
    }

    /**
     * Determine whether the logging is enabled or not.
     *
     * @return  {@code true} only if the logging is enabled.
     */
    public abstract boolean isEnabled();

    /**
     * Log the specified message.
     *
     * @param msg  A message to be logged.
     */
    public abstract void log(String msg);

    /**
     * Log a message according to the specified format string and arguments.
     *
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    public abstract void log(String format, Object ... args);

    /**
     * Log the specified message and exception.
     *
     * @param msg  A message to be logged.
     * @param t    A {@link Throwable} to be logged.
     */
    public abstract void log(String msg, Throwable t);

    /**
     * {@code FixedLogger.Error} is a wrapper for {@link Logger} that binds
     * the specified logger to the ERROR level.
     */
    public static final class Error extends FixedLogger {
        /**
         * Construct a new instance.
         *
         * @param log  A {@link Logger} instance.
         */
        public Error(Logger log) {
            super(log);
        }

        // FixedLogger

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean isEnabled() {
            return getLogger().isErrorEnabled();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg) {
            getLogger().error(msg);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String format, Object ... args) {
            getLogger().error(format, args);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg, Throwable t) {
            getLogger().error(msg, t);
        }
    }

    /**
     * {@code FixedLogger.Warn} is a wrapper for {@link Logger} that binds
     * the specified logger to the WARN level.
     */
    public static final class Warn extends FixedLogger {
        /**
         * Construct a new instance.
         *
         * @param log  A {@link Logger} instance.
         */
        public Warn(Logger log) {
            super(log);
        }

        // FixedLogger

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean isEnabled() {
            return getLogger().isWarnEnabled();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg) {
            getLogger().warn(msg);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String format, Object ... args) {
            getLogger().warn(format, args);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg, Throwable t) {
            getLogger().warn(msg, t);
        }
    }

    /**
     * {@code FixedLogger.Info} is a wrapper for {@link Logger} that binds
     * the specified logger to the INFO level.
     */
    public static final class Info extends FixedLogger {
        /**
         * Construct a new instance.
         *
         * @param log  A {@link Logger} instance.
         */
        public Info(Logger log) {
            super(log);
        }

        // FixedLogger

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean isEnabled() {
            return getLogger().isInfoEnabled();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg) {
            getLogger().info(msg);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String format, Object ... args) {
            getLogger().info(format, args);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg, Throwable t) {
            getLogger().info(msg, t);
        }
    }

    /**
     * {@code FixedLogger.Debug} is a wrapper for {@link Logger} that binds
     * the specified logger to the DEBUG level.
     */
    public static final class Debug extends FixedLogger {
        /**
         * Construct a new instance.
         *
         * @param log  A {@link Logger} instance.
         */
        public Debug(Logger log) {
            super(log);
        }

        // FixedLogger

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean isEnabled() {
            return getLogger().isDebugEnabled();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg) {
            getLogger().debug(msg);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String format, Object ... args) {
            getLogger().debug(format, args);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg, Throwable t) {
            getLogger().debug(msg, t);
        }
    }

    /**
     * {@code FixedLogger.Trace} is a wrapper for {@link Logger} that binds
     * the specified logger to the TRACE level.
     */
    public static final class Trace extends FixedLogger {
        /**
         * Construct a new instance.
         *
         * @param log  A {@link Logger} instance.
         */
        public Trace(Logger log) {
            super(log);
        }

        // FixedLogger

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean isEnabled() {
            return getLogger().isTraceEnabled();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg) {
            getLogger().trace(msg);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String format, Object ... args) {
            getLogger().trace(format, args);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void log(String msg, Throwable t) {
            getLogger().trace(msg, t);
        }
    }
}
