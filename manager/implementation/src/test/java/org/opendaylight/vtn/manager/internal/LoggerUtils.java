/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import ch.qos.logback.classic.Level;
import ch.qos.logback.classic.Logger;

import org.slf4j.LoggerFactory;

/**
 * {@code LoggerUtils} class is a collection of utility class methods for
 * logger management.
 */
public final class LoggerUtils {
    /**
     * Private constructor that protects this class from instantiating.
     */
    private LoggerUtils() {}

    /**
     * Change the log level for the logger specified by the given class.
     *
     * @param cls    A class which specifies the target logger.
     * @param level  A log level to be set.
     * @return  A log level previously configured in the target logger.
     */
    public static Level setLevel(Class<?> cls, Level level) {
        return setLevel(cls.getName(), level);
    }

    /**
     * Change the log level for the logger specified by the given name.
     *
     * @param name   The name of the target logger.
     * @param level  A log level to be set.
     * @return  A log level previously configured in the target logger.
     */
    public static Level setLevel(String name, Level level) {
        Logger logger = (Logger)LoggerFactory.getLogger(name);
        Level old = logger.getLevel();
        logger.setLevel(level);

        return old;
    }
}
