/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for a task that updates the path map configuration.
 *
 * @param <T>  The type of MD-SAL datastore transaction task that updates
 *             path map configuration.
 * @param <O>  The type of the output of the RPC.
 */
public abstract class PathMapTask<T extends AbstractTxTask<VtnUpdateType>, O>
    extends CompositeTxTask<VtnUpdateType, T>
    implements RpcOutputGenerator<List<VtnUpdateType>, O> {
    /**
     * The identifier for the target VTN.
     *
     * <p>
     *   {@code null} means that the global path map is targeted.
     * </p>
     */
    private final VTenantIdentifier  identifier;

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target VTN.
     *               {@code null} means that the global path map is targeted.
     * @param tasks  A list of tasks that update path map configuration.
     */
    protected PathMapTask(VTenantIdentifier ident, List<T> tasks) {
        super(tasks);
        identifier = ident;
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void onStarted(TxContext ctx) throws VTNException {
        if (identifier != null) {
            // Ensure that the target VTN is present.
            identifier.fetch(ctx.getReadWriteTransaction());
        }
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
                FlowRemover remover = (identifier == null)
                    ? new AllFlowRemover()
                    : new TenantFlowRemover(identifier);
                addBackgroundTask(provider.removeFlows(remover));
                break;
            }
        }
    }
}
