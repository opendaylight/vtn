/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for a task that updates the flow match configurations
 * in the flow condition.
 *
 * @param <T>  The type of MD-SAL datastore transaction task that updates
 *             flow match configuration.
 * @param <O>  The type of the output of the RPC.
 */
public abstract class FlowMatchTask<T extends AbstractTxTask<VtnUpdateType>, O>
    extends CompositeTxTask<VtnUpdateType, T>
    implements RpcOutputGenerator<List<VtnUpdateType>, O> {
    /**
     * The name of the target flow condition.
     */
    private final VnodeName  nodeName;

    /**
     * Construct a new instance.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of the
     *               target flow condition.
     * @param tasks  A list of tasks that update flow match configuration.
     */
    protected FlowMatchTask(VnodeName vname, List<T> tasks) {
        super(tasks);
        nodeName = vname;
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void onStarted(TxContext ctx) throws VTNException {
        // Ensure that the target flow condition is present.
        FlowCondUtils.checkPresent(ctx.getReadWriteTransaction(), nodeName);
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
                addBackgroundTask(provider.removeFlows(new AllFlowRemover()));
                break;
            }
        }
    }
}
