/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.slf4j.Logger;

/**
 * {@code LogProvider} determines interfaces to be implemented by classes
 * that records logs via {@link Logger} instance.
 *
 * <p>
 *   This interface is used to associate logs with a class that implements
 *   this interface.
 * </p>
 */
public interface LogProvider {
    /**
     * Return a logger instance.
     *
     * @return  A {@link Logger} instance.
     */
    Logger getLogger();

    /**
     * Return a prefix string for a log record.
     *
     * @return  A prefix for a log record.
     */
    String getLogPrefix();
}
