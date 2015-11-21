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
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code RemoveVterminalTask} describes the MD-SAL datastore transaction task
 * that removes the specified vTerminal.
 *
 * @see  #create(RemoveVterminalInput)
 */
public final class RemoveVterminalTask
    extends RemoveVirtualNodeTask<Vterminal, VTerminalIdentifier> {
    /**
     * Create a new task that removes the specified vTerminal.
     *
     * @param input  A {@link RemoveVterminalInput} instance.
     * @return  A {@link RemoveVterminalTask} instance.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemoveVterminalTask create(RemoveVterminalInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        VTerminalIdentifier vtermId = VTerminalIdentifier.create(input, true);
        return new RemoveVterminalTask(vtermId);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  An identifier for the target vTerminal.
     */
    private RemoveVterminalTask(VTerminalIdentifier ident) {
        super(ident);
    }

    // RemoveVirtualNodeTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void destroy(TxContext ctx, Vterminal current)
        throws VTNException {
        // Destroy the specified vTerminal.
        VTerminalIdentifier ident = getIdentifier();
        VTerminal vterm = new VTerminal(ident, current);
        boolean noFilter = vterm.isFilterEmpty();
        vterm.destroy(ctx, true);

        if (noFilter) {
            // Purge all the VTN flows established by the removed vTerminal.
            setFlowRemover(new VNodeFlowRemover(ident));
        } else {
            // REVISIT: Select flow entries affected by obsolete flow filters.
            setFlowRemover(new TenantFlowRemover(ident));
        }
    }
}
