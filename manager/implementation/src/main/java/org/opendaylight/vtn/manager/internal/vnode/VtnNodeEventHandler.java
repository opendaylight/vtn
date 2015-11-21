/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;

/**
 * {@code VtnNodeEventHandler} delivers VTN node events to all the virtual
 * nodes in the VTN tree.
 */
public final class VtnNodeEventHandler extends VNodeWalker<VtnNodeEvent> {
    // VNodeWalker

    /**
     * Deliver a VTN node event to the given vBridge.
     *
     * @param ctx  A runtime context for transaction task.
     * @param vbr  A {@link VBridge} instance.
     * @param ev   A {@link VtnNodeEvent} instance.
     */
    @Override
    protected void scan(TxContext ctx, VBridge vbr, VtnNodeEvent ev) {
        vbr.notifyNode(ctx, ev);
    }

    /**
     * Remove resolved path faults in the given vTerminal.
     *
     * @param ctx  A runtime context for transaction task.
     * @param vtm  A {@link VTerminal} instance.
     * @param ev   A {@link VtnNodeEvent} instance.
     */
    @Override
    protected void scan(TxContext ctx, VTerminal vtm, VtnNodeEvent ev) {
        vtm.notifyNode(ctx, ev);
    }
}
