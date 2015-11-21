/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.VNodeFlowRemover;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code RemoveVbridgeTask} describes the MD-SAL datastore transaction task
 * that removes the specified vBridge.
 *
 * @see  #create(RemoveVbridgeInput)
 */
public final class RemoveVbridgeTask
    extends RemoveVirtualNodeTask<Vbridge, VBridgeIdentifier> {
    /**
     * Create a new task that removes the specified vBridge.
     *
     * @param input  A {@link RemoveVbridgeInput} instance.
     * @return  A {@link RemoveVbridgeTask} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemoveVbridgeTask create(RemoveVbridgeInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        VBridgeIdentifier vbrId = VBridgeIdentifier.create(input, true);
        return new RemoveVbridgeTask(vbrId);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  An identifier for the target vBridge.
     */
    private RemoveVbridgeTask(VBridgeIdentifier ident) {
        super(ident);
    }

    // RemoveVirtualNodeTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void destroy(TxContext ctx, Vbridge current)
        throws VTNException {
        // Destroy the specified vBridge.
        VBridgeIdentifier ident = getIdentifier();
        VBridge vbr = new VBridge(ident, current);
        boolean noFilter = vbr.isFilterEmpty();
        vbr.destroy(ctx, true);

        if (noFilter) {
            // Purge all the VTN flows established by the removed vBridge.
            setFlowRemover(new VNodeFlowRemover(ident));
        } else {
            // REVISIT: Select flow entries affected by obsolete flow filters.
            setFlowRemover(new TenantFlowRemover(ident));
        }
    }
}
