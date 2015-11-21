/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.flow.remove.VNodeFlowRemover;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.MacEntryFilter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

/**
 * {@code VNodeMapCleaner} purges network resources originated by the
 * specified virtual node.
 */
public final class VNodeMapCleaner implements MapCleaner, MacEntryFilter {
    /**
     * The identifier for the target virtual node.
     */
    private final VNodeIdentifier<?>  identifier;

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target virtual node.
     */
    public VNodeMapCleaner(VNodeIdentifier<?> ident) {
        identifier = ident;
    }

    // MapCleaner

    /**
     * Purge network caches originated by the specified virtual node.
     *
     * @param ctx    A runtime context for transaction task.
     * @param tname  Not used. Tenant name configured in this instance
     *               is always used.
     * @throws VTNException  An error occurred.
     */
    @Override
    public void purge(TxContext ctx, String tname) throws VTNException {
        // Remove MAC addresses detected by the specified virtual node.
        // In this case we don't need to scan MAC address tables in the
        // VTN tree.
        if (identifier.getType().getBridgeType() == VNodeType.VBRIDGE) {
            VBridgeIdentifier vbrId = new VBridgeIdentifier(
                identifier.getTenantName(), identifier.getBridgeName());
            new MacEntryRemover(this).scan(ctx, vbrId);
        }

        // Remove flow entries originated by the specified virtual node.
        ctx.getSpecific(FlowRemoverQueue.class).
            enqueue(new VNodeFlowRemover(identifier));
    }

    // MacEntryFilter

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean accept(MacTableEntry ment) {
        return identifier.toString().equals(ment.getEntryData());
    }
}
