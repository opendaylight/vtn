/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;

/**
 * {@code XmlLogger} describes a logger interface used when the VTN Manager
 * loads the VTN configuration from XML file.
 */
public interface XmlLogger {
    /**
     * Log a message according to the specified format string and arguments.
     *
     * <p>
     *   This method constructs a message using SLF4J log format.
     * </p>
     *
     * @param level   A {@link VTNLogLevel} instance that specifies the
     *                logging level.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    void log(VTNLogLevel level, String format, Object ... args);

    /**
     * Log the specified throwable and a message according to the given
     * format string and arguments.
     *
     * <p>
     *   Note that this method constructs a log message using
     *   {@link String#format(String, Object[])}.
     * </p>
     *
     * @param level   A {@link VTNLogLevel} instance that specifies the
     *                logging level.
     * @param t       A {@link Throwable} to be logged.
     * @param format  A format string used to construct log message.
     * @param args    An object array used to construct log message.
     */
    void log(VTNLogLevel level, Throwable t, String format, Object ... args);
}
