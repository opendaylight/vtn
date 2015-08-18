/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.tx.TxEvent;

/**
 * {@code RoutingEvent} describes an event which notifies a change of
 * physical packet routing tables.
 */
public final class RoutingEvent extends TxEvent {
    /**
     * The target routing listener.
     */
    private final VTNRoutingListener  listener;

    /**
     * Construct a new instance.
     *
     * @param l  A {@link VTNRoutingListener} instance.
     */
    RoutingEvent(VTNRoutingListener l) {
        listener = l;
    }

    // TxEvent

    /**
     * {@inheritDoc}
     */
    @Override
    protected void notifyEvent() throws VTNException {
        listener.routingUpdated(this);
    }
}
