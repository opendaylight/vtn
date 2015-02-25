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

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;

/**
 * An event context used to handle node connector events.
 */
interface NodeConnectorEventContext extends TxTask<Void> {
    /**
     * Add the given node to the node map to be updated.
     *
     * @param path   The identifier of the node connector to be added.
     * @param nc     A {@link NodeConnector} instance to be added
     * @param label  A label usd only for logging.
     */
    void addUpdated(InstanceIdentifier<NodeConnector> path, NodeConnector nc,
                    String label);

    /**
     * Add the given node to the node map to be removed.
     *
     * @param path  The identifier of the node connector to be added.
     * @param nc    A {@link NodeConnector} instance to be added.
     */
    void addRemoved(InstanceIdentifier<NodeConnector> path, NodeConnector nc);

    /**
     * Determine whether this instance contains at least one port information
     * to be updated or removed.
     *
     * @return  {@code true} only if this instance contains at least one port
     *          information to be updated or removed.
     */
    boolean hasPort();
}
