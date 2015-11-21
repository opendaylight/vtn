/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.TxContext;

/**
 * {@code PathFaultResolver} scans virtual nodes in the VTN tree, and removes
 * resolved path faults.
 */
public final class PathFaultResolver extends VNodeWalker<Void> {
    // VNodeWalker

    /**
     * Remove resolved path faults in the given vBridge.
     *
     * @param ctx   A runtime context for transaction task.
     * @param vbr   A {@link VBridge} instance.
     * @param data  Always {@code null}.
     */
    @Override
    protected void scan(TxContext ctx, VBridge vbr, Void data) {
        vbr.routingUpdated(ctx);
    }

    /**
     * Remove resolved path faults in the given vTerminal.
     *
     * @param ctx   A runtime context for transaction task.
     * @param vtm   A {@link VTerminal} instance.
     * @param data  Always {@code null}.
     */
    @Override
    protected void scan(TxContext ctx, VTerminal vtm, Void data) {
        vtm.routingUpdated(ctx);
    }
}
