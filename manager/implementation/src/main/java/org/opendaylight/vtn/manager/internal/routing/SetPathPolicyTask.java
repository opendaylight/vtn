/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyConfigBuilder;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.PutDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code SetPathPolicyTask} describes the MD-SAL transaction task that
 * creates or updates the specified path policy configuration.
 *
 * @see #create(TopologyGraph, SetPathPolicyInput)
 */
public final class SetPathPolicyTask extends PutDataTask<VtnPathPolicy>
    implements RpcOutputGenerator<VtnUpdateType, SetPathPolicyOutput> {
    /**
     * Default value for the default cost.
     */
    private static final long  DEFAULT_DEF_COST = 0L;

    /**
     * Runtime context for updating the path policy.
     */
    private final PathPolicyRpcContext  context;

    /**
     * Set {@code true} if the target path policy is required to be present.
     */
    private final boolean  present;

    /**
     * Construct a new task that creates or updates the specified path policy
     * configuration.
     *
     * @param topo   The network topology graph.
     * @param input  A {@link SetPathPolicyInput} instance.
     * @return  A {@link SetPathPolicyTask} instance associated with the task
     *          that set the given path policy configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetPathPolicyTask create(TopologyGraph topo,
                                           SetPathPolicyInput input)
        throws RpcException {
        // Verify the given input.
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        VtnUpdateOperationType op = input.getOperation();
        if (VtnUpdateOperationType.REMOVE.equals(op)) {
            throw RpcUtils.getInvalidOperationException(op);
        }

        VtnPathPolicyBuilder builder = new PathPolicyConfigBuilder.Data().
            set(input).getBuilder();
        boolean repl = VtnUpdateOperationType.SET.equals(op);
        if (repl && builder.getDefaultCost() == null) {
            // Use default value for "default-cost".
            builder.setDefaultCost(Long.valueOf(DEFAULT_DEF_COST));
        }

        // Create a new task.
        InstanceIdentifier<VtnPathPolicy> path =
            PathPolicyUtils.getIdentifier(input);
        VtnPathPolicy vpp = builder.build();
        boolean present = Boolean.TRUE.equals(input.isPresent());
        return new SetPathPolicyTask(topo, path, vpp, repl, present);
    }

    /**
     * Construct a new instance.
     *
     * @param topo  The network topology graph.
     * @param path  Path to the path policy configuration.
     * @param vpp   A {@link VtnPathPolicy} instance.
     * @param repl  If {@code true}, the target object will be replaced with
     *              the given object. Otherwise the given object will be
     *              merged with the target object.
     * @param pr    {@code true} means that the target path policy is required
     *              to be present.
     */
    private SetPathPolicyTask(TopologyGraph topo,
                              InstanceIdentifier<VtnPathPolicy> path,
                              VtnPathPolicy vpp, boolean repl, boolean pr) {
        super(LogicalDatastoreType.OPERATIONAL, path, vpp, repl);
        context = new PathPolicyRpcContext(topo, vpp.getId());
        present = pr;
    }

    // PutDataTask

    /**
     * Determine whether missing parent nodes should be created or not.
     *
     * <p>
     *   This method always return true in order to create the root container
     *   for the path policy if missing.
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
    protected void onStarted(TxContext ctx, VtnPathPolicy current)
        throws VTNException {
        if (current == null && present) {
            // The target path policy is not present.
            throw PathPolicyUtils.getNotFoundException(context.getPolicyId());
        }

        context.onStarted();
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, VtnUpdateType result) {
        if (result != null) {
            FlowRemover remover;
            if (VtnUpdateType.CREATED.equals(result)) {
                // We need to remove all flow entries because a new path policy
                // may affect existing flow entries.
                remover = new AllFlowRemover();
            } else {
                // Remove all flow entries affected by the target path policy.
                remover = context.getFlowRemover();
            }

            // Remove flow entries affected by the update.
            addBackgroundTask(provider.removeFlows(remover));

            context.onUpdated();
        }
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<SetPathPolicyOutput> getOutputType() {
        return SetPathPolicyOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetPathPolicyOutput createOutput(VtnUpdateType result) {
        return new SetPathPolicyOutputBuilder().setStatus(result).build();
    }
}
