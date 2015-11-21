/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;

/**
 * {@code VtnPortEventHandler} delivers VTN port events to all the virtual
 * nodes in the VTN tree.
 */
public final class VtnPortEventHandler extends VNodeWalker<VtnPortEvent> {
    // VNodeWalker

    /**
     * Deliver a VTN port event to the given vBridge.
     *
     * @param ctx  A runtime context for transaction task.
     * @param vbr  A {@link VBridge} instance.
     * @param ev   A {@link VtnPortEvent} instance.
     */
    @Override
    protected void scan(TxContext ctx, VBridge vbr, VtnPortEvent ev) {
        vbr.notifyPort(ctx, ev);
    }

    /**
     * Remove resolved path faults in the given vTerminal.
     *
     * @param ctx  A runtime context for transaction task.
     * @param vtm  A {@link VTerminal} instance.
     * @param ev   A {@link VtnPortEvent} instance.
     */
    @Override
    protected void scan(TxContext ctx, VTerminal vtm, VtnPortEvent ev) {
        vtm.notifyPort(ctx, ev);
    }
}
