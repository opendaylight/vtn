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
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.PutDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code SetFlowConditionTask} describes the MD-SAL datastore transaction
 * task that creates or updates the specified flow condition configuration.
 *
 * @see  #create(SetFlowConditionInput)
 */
public final class SetFlowConditionTask extends PutDataTask<VtnFlowCondition>
    implements RpcOutputGenerator<VtnUpdateType, SetFlowConditionOutput> {
    /**
     * The name of the target flow condition.
     */
    private final String  name;

    /**
     * Set {@code true} if the target flow condition is required to be present.
     */
    private final boolean  present;

    /**
     * Construct a new task that creates or updates the specified flow
     * condition configuration.
     *
     * @param input  A {@link SetFlowConditionInput} instance.
     * @return  A {@link SetFlowConditionTask} instance associated with the
     *          task that set the given flow condition configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetFlowConditionTask create(SetFlowConditionInput input)
        throws RpcException {
        // Verify the given input.
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        VtnUpdateOperationType op = input.getOperation();
        if (VtnUpdateOperationType.REMOVE.equals(op)) {
            throw RpcUtils.getInvalidOperationException(op);
        }

        VTNFlowCondition vfcond = new VTNFlowCondition(input);
        boolean repl = VtnUpdateOperationType.SET.equals(op);

        // Create a task.
        String name = vfcond.getIdentifier();
        InstanceIdentifier<VtnFlowCondition> path = vfcond.getPath();
        VtnFlowCondition vfc = vfcond.toVtnFlowConditionBuilder().build();
        boolean present = Boolean.TRUE.equals(input.isPresent());
        return new SetFlowConditionTask(name, path, vfc, repl, present);
    }

    /**
     * Construct a new instance.
     *
     * @param nm    The name of the target flow condition.
     * @param path  Path to the target flow condition.
     * @param vfc   A {@link VtnFlowCondition} instance.
     * @param repl  If {@code true}, the target object will be replaced with
     *              the given object. Otherwise the given object will be
     *              merged with the target object.
     * @param pr    {@code true} means that the target flow condition is
     *              required to be present.
     */
    private SetFlowConditionTask(String nm,
                                 InstanceIdentifier<VtnFlowCondition> path,
                                 VtnFlowCondition vfc, boolean repl,
                                 boolean pr) {
        super(LogicalDatastoreType.OPERATIONAL, path, vfc, repl);
        name = nm;
        present = pr;
    }

    // PutDataTask

    /**
     * Determine whether missing parent nodes should be created or not.
     *
     * <p>
     *   This method always return true in order to create the root container
     *   for the flow condition if missing.
     * </p>
     *
     * @return {@code true}.
     */
    @Override
    protected boolean fixMissingParents() {
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx, VtnFlowCondition current)
        throws VTNException {
        if (current == null && present) {
            // The target flow condition is not present.
            throw FlowCondUtils.getNotFoundException(name);
        }
    }

    // TxTask

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
    public Class<SetFlowConditionOutput> getOutputType() {
        return SetFlowConditionOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetFlowConditionOutput createOutput(VtnUpdateType result) {
        return new SetFlowConditionOutputBuilder().setStatus(result).build();
    }
}
