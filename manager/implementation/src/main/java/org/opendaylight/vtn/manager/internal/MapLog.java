/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.cluster.MapReference;

/**
 * A base class of classes which is used to record a trace level log message
 * related to the virtual network mapping.
 *
 * <p>
 *   An instance of this class should represent a log message.
 * </p>
 */
public abstract class MapLog {
    /**
     * A reference to the target virtual mapping.
     */
    private final MapReference  mapping;

    /**
     * Construct a new instance.
     *
     * @param ref  A reference to a virtual mapping.
     */
    protected MapLog(MapReference ref) {
        mapping = ref;
    }

    /**
     * Record a log message.
     *
     * @param log  A {@link Logger} instance.
     */
    public void log(Logger log) {
        String msg = getMessage();
        if (msg != null) {
            MapReference ref = mapping;
            log.trace("{}:{}: {}", ref.getContainerName(), ref.getPath(), msg);
        }
    }

    /**
     * Return a message to be logged.
     *
     * @return  A log message.
     *          {@code null} is returned if no message should be logged.
     */
    protected abstract String getMessage();
}
