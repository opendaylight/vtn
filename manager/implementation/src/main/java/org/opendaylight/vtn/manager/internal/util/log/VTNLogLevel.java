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
 * {@code VTNLogLevel} defines a set of logging level for VTN internal
 * messages.
 */
public enum VTNLogLevel implements LogLevelLogger {
    /**
     * Indicates the ERROR level.
     */
    ERROR {
        @Override
        public boolean isEnabled(Logger logger) {
            return logger.isErrorEnabled();
        }

        @Override
        public void log(Logger logger, String msg) {
            logger.error(msg);
        }

        @Override
        public void log(Logger logger, String format, Object ... args) {
            logger.error(format, args);
        }

        @Override
        public void log(Logger logger, String msg, Throwable t) {
            logger.error(msg, t);
        }
    },

    /**
     * Indicates the WARN level.
     */
    WARN {
        @Override
        public boolean isEnabled(Logger logger) {
            return logger.isWarnEnabled();
        }

        @Override
        public void log(Logger logger, String msg) {
            logger.warn(msg);
        }

        @Override
        public void log(Logger logger, String format, Object ... args) {
            logger.warn(format, args);
        }

        @Override
        public void log(Logger logger, String msg, Throwable t) {
            logger.warn(msg, t);
        }
    },

    /**
     * Indicates the INFO level.
     */
    INFO {
        @Override
        public boolean isEnabled(Logger logger) {
            return logger.isInfoEnabled();
        }

        @Override
        public void log(Logger logger, String msg) {
            logger.info(msg);
        }

        @Override
        public void log(Logger logger, String format, Object ... args) {
            logger.info(format, args);
        }

        @Override
        public void log(Logger logger, String msg, Throwable t) {
            logger.info(msg, t);
        }
    },

    /**
     * Indicates the DEBUG level.
     */
    DEBUG {
        @Override
        public boolean isEnabled(Logger logger) {
            return logger.isDebugEnabled();
        }

        @Override
        public void log(Logger logger, String msg) {
            logger.debug(msg);
        }

        @Override
        public void log(Logger logger, String format, Object ... args) {
            logger.debug(format, args);
        }

        @Override
        public void log(Logger logger, String msg, Throwable t) {
            logger.debug(msg, t);
        }
    },

    /**
     * Indicates the TRACE level.
     */
    TRACE {
        @Override
        public boolean isEnabled(Logger logger) {
            return logger.isTraceEnabled();
        }

        @Override
        public void log(Logger logger, String msg) {
            logger.trace(msg);
        }

        @Override
        public void log(Logger logger, String format, Object ... args) {
            logger.trace(format, args);
        }

        @Override
        public void log(Logger logger, String msg, Throwable t) {
            logger.trace(msg, t);
        }
    };

    // LogLevelLogger

    /**
     * {@inheritDoc}
     */
    @Override
    public void log(Logger logger, Throwable t, String format,
                    Object ... args) {
        if (isEnabled(logger)) {
            String msg = (args == null) ? format : String.format(format, args);
            if (t == null) {
                log(logger, msg);
            } else {
                log(logger, msg, t);
            }
        }
    }
}
