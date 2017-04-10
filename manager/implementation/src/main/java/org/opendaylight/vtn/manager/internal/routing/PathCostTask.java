/*
 * Copyright (c) 2016, 2017 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for a task that updates the path cost configurations
 * in the path map.
 *
 * @param <T>  The type of MD-SAL datastore transaction task that updates
 *             path cost configuration.
 * @param <O>  The type of the output of the RPC.
 */
public abstract class PathCostTask<T extends AbstractTxTask<VtnUpdateType>, O>
    extends CompositeTxTask<VtnUpdateType, T>
    implements RpcOutputGenerator<List<VtnUpdateType>, O> {
    /**
     * Runtime context for updating the path policy.
     */
    private final PathPolicyRpcContext  context;

    /**
     * Construct a new instance.
     *
     * @param topo   The network topology graph.
     * @param id     The identifier of the target path policy.
     * @param tasks  A list of tasks that update link cost configuration.
     */
    protected PathCostTask(TopologyGraph topo, Integer id, List<T> tasks) {
        super(tasks);
        context = new PathPolicyRpcContext(topo, id);
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void onStarted(TxContext ctx) throws VTNException {
        // Ensure that the target path policy is present.
        Integer pid = context.getPolicyId();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        PathPolicyUtils.readVtnPathPolicy(tx, pid);

        context.onStarted();
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
                // Remove all flow entries affected by the target path policy.
                addBackgroundTask(
                    provider.removeFlows(context.getFlowRemover()));

                context.onUpdated();
                break;
            }
        }
    }
}
