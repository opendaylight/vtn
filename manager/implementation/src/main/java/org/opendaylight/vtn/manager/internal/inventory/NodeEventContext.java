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

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * An event context used to handle node events.
 */
interface NodeEventContext extends TxTask<Void> {
    /**
     * Add the given node to the node map to be created.
     *
     * @param path   The identifier of the node to be added.
     * @param node  A {@link Node} instance to be added.
     */
    void addCreated(InstanceIdentifier<Node> path, Node node);

    /**
     * Add the given node to the node map to be removed.
     *
     * @param path  The identifier of the node to be added.
     * @param node  A {@link Node} instance to be added.
     */
    void addRemoved(InstanceIdentifier<Node> path, Node node);

    /**
     * Determine whether this instance contains nodes to be created or
     * removed.
     *
     * @return  {@code true} only if this instance contains at least one
     *          node to be created or removed.
     */
    boolean hasNode();
}
