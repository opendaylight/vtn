/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for a task that updates the flow filter configurations
 * in the flow filter list.
 *
 * @param <T>  The type of MD-SAL datastore transaction task that updates
 *             flow filter configuration.
 * @param <O>  The type of the output of the RPC.
 */
public abstract class FlowFilterTask<T extends AbstractTxTask<VtnUpdateType>, O>
    extends CompositeTxTask<VtnUpdateType, T>
    implements RpcOutputGenerator<List<VtnUpdateType>, O> {
    /**
     * The identifier for the target virtual node.
     */
    private final VNodeIdentifier<?>  identifier;

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VNodeIdentifier} instance that specifies the
     *               target virtual node.
     * @param tasks  A list of tasks that update flow filter configuration.
     */
    protected FlowFilterTask(VNodeIdentifier<?> ident, List<T> tasks) {
        super(tasks);
        identifier = ident;
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void onStarted(TxContext ctx) throws VTNException {
        // Ensure that the target virtual node is present.
        identifier.fetch(ctx.getReadWriteTransaction());
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public final void onSuccess(VTNManagerProvider provider,
                                List<VtnUpdateType> result) {
        for (VtnUpdateType status: result) {
            if (status != null) {
                // REVISIT: Select flow entries affected by the change.
                String tname = identifier.getTenantNameString();
                TenantFlowRemover remover = new TenantFlowRemover(tname);
                addBackgroundTask(provider.removeFlows(remover));
                break;
            }
        }
    }
}
