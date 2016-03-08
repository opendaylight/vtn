/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.tx.TxEvent;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code VtnNodeEvent} describes an event which notifies a VTN node has been
 * added or removed.
 */
public final class VtnNodeEvent extends TxEvent {
    /**
     * The target inventory listener.
     */
    private final VTNInventoryListener  listener;

    /**
     * A {@link SalNode} instance.
     */
    private final SalNode  salNode;

    /**
     * A {@link VtnNode} instance.
     */
    private final VtnNode  vtnNode;

    /**
     * A {@link VtnUpdateType} instance that specifies the type of notification.
     */
    private final VtnUpdateType  updateType;

    /**
     * Construct a new instance without specifying an inventory listener.
     *
     * @param vnode  A {@link VtnNode} instance.
     * @param type   A {@link VtnUpdateType} instance.
     */
    VtnNodeEvent(VtnNode vnode, VtnUpdateType type) {
        listener = null;
        vtnNode = vnode;
        updateType = type;
        salNode = SalNode.create(vnode.getId());
    }

    /**
     * Create a copy of the given event for the given listener.
     *
     * @param l   A {@link VTNInventoryListener} instance.
     * @param ev  A {@link VtnNodeEvent} instance.
     */
    VtnNodeEvent(VTNInventoryListener l, VtnNodeEvent ev) {
        listener = l;
        vtnNode = ev.vtnNode;
        updateType = ev.updateType;
        salNode = ev.salNode;
    }

    /**
     * Return a {@link SalNode} instance that represents the location of
     * a swtich.
     *
     * @return  A {@link SalNode} instance.
     */
    public SalNode getSalNode() {
        return salNode;
    }

    /**
     * Return a {@link VtnNode} instance.
     *
     * @return  A {@link VtnNode} instance.
     */
    public VtnNode getVtnNode() {
        return vtnNode;
    }

    /**
     * Return the type of notification.
     *
     * @return
     *   <ul>
     *     <li>
     *       {@link VtnUpdateType#CREATED} if a VTN node has been added.
     *     </li>
     *     <li>
     *       {@link VtnUpdateType#CHANGED} if a VTN node has been changed.
     *       This type is returned only if the OpenFlow protocol version used
     *       by the target switch has been determined.
     *     </li>
     *     <li>
     *       {@link VtnUpdateType#REMOVED} if a VTN node has been removed.
     *     </li>
     *   </ul>
     */
    public VtnUpdateType getUpdateType() {
        return updateType;
    }

    // TxEvent

    /**
     * {@inheritDoc}
     */
    @Override
    protected void notifyEvent() throws VTNException {
        listener.notifyVtnNode(this);
    }
}
