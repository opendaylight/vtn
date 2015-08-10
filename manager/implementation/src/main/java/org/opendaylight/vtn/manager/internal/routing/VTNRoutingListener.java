/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.VTNException;

/**
 * Interface for classes that listen routing resolver events.
 */
public interface VTNRoutingListener {
    /**
     * Invoked when the packet routing table has been updated.
     *
     * @param ev  A {@link RoutingEvent} instance.
     * @throws VTNException  An error occurred.
     */
    void routingUpdated(RoutingEvent ev) throws VTNException;
}
