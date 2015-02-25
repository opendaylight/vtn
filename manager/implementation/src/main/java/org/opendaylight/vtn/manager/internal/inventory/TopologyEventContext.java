/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.opendaylight.vtn.manager.internal.TxTask;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;

/**
 * An event context used to handle topology events.
 */
interface TopologyEventContext extends TxTask<Void> {
    /**
     * Add the given link to the link map to be created.
     *
     * @param link  A {@link Link} instance to be added.
     */
    void addCreated(Link link);

    /**
     * Add the given link to the link map to be removed.
     *
     * @param link  A {@link Link} instance to be added.
     */
    void addRemoved(Link link);

    /**
     * Determine whether this instance contains at least one link information
     * to be created or removed.
     *
     * @return  {@code true} only if this instance contains at least one
     *          link information to be created or removed.
     */
    boolean hasLink();
}
