/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.ClearFlowConditionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.ClearFlowConditionOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code ClearFlowConditionTask} describes the MD-SAL datastore transaction
 * task that clears the flow condition container.
 */
public final class ClearFlowConditionTask extends AbstractTxTask<VtnUpdateType>
    implements RpcOutputGenerator<VtnUpdateType, ClearFlowConditionOutput> {
    // AbstractTxTask

    /**
     * Clear the root container of the flow condition.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  {@link VtnUpdateType#REMOVED} if at least one flow condition
     *          has been removed. {@code null} is returned if no flow
     *          condition is present.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected VtnUpdateType execute(TxContext ctx) throws VTNException {
        // Read the current value.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnFlowConditions> path =
            InstanceIdentifier.create(VtnFlowConditions.class);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<VtnFlowConditions> opt = DataStoreUtils.read(tx, oper, path);
        VtnUpdateType removed;
        if (opt.isPresent() && !FlowCondUtils.isEmpty(opt.get())) {
            removed = VtnUpdateType.REMOVED;
        } else {
            removed = null;
        }

        // Put an empty root container.
        VtnFlowConditions root = new VtnFlowConditionsBuilder().build();
        tx.put(oper, path, root, true);

        return removed;
    }

    // TxTask

    /**
     * Determine whether the transaction queue should log the given error
     * or not.
     *
     * <p>
     *   This method returns {@code false} if a {@link RpcException} is
     *   passed.
     * </p>
     *
     * @param t  A {@link Throwable} that is going to be thrown.
     * @return   {@code true} if the transaction queue should log the given
     *           {@link Throwable}. Otherwise {@code false}.
     */
    @Override
    public boolean needErrorLog(Throwable t) {
        return !(t instanceof RpcException);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, VtnUpdateType result) {
        if (result != null) {
            // REVISIT: Select flow entries affected by the change.
            addBackgroundTask(provider.removeFlows(new AllFlowRemover()));
        }
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<ClearFlowConditionOutput> getOutputType() {
        return ClearFlowConditionOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public ClearFlowConditionOutput createOutput(VtnUpdateType result) {
        return new ClearFlowConditionOutputBuilder().setStatus(result).build();
    }
}
