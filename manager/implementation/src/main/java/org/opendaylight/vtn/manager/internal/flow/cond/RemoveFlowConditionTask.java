/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code RemoveFlowConditionTask} describes the MD-SAL datastore transaction
 * task that deletes the specified flow condition configuration.
 *
 * @see #create(RemoveFlowConditionInput)
 */
public final class RemoveFlowConditionTask
    extends DeleteDataTask<VtnFlowCondition>
    implements RpcOutputGenerator<VtnUpdateType, Void> {
    /**
     * Create a new task that removes the specified flow condition.
     *
     * @param input  A {@link RemoveFlowConditionInput} instance.
     * @return  A {@link RemoveFlowConditionTask} associated with the task that
     *          removes the given flow condition.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemoveFlowConditionTask create(RemoveFlowConditionInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        InstanceIdentifier<VtnFlowCondition> path =
            FlowCondUtils.getIdentifier(input.getName());
        return new RemoveFlowConditionTask(path);
    }

    /**
     * Construct a new instance.
     *
     * @param path  Path to the flow condition to be removed.
     */
    private RemoveFlowConditionTask(InstanceIdentifier<VtnFlowCondition> path) {
        super(LogicalDatastoreType.OPERATIONAL, path);
    }

    // DeleteDataTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx, VtnFlowCondition current)
        throws VTNException {
        if (current == null) {
            // The target flow condition is not present.
            String name = FlowCondUtils.getName(getTargetPath());
            throw FlowCondUtils.getNotFoundException(name);
        }
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, VtnUpdateType result) {
        // REVISIT: Select flow entries affected by the change.
        addBackgroundTask(provider.removeFlows(new AllFlowRemover()));
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<Void> getOutputType() {
        return Void.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Void createOutput(VtnUpdateType result) {
        return null;
    }
}
