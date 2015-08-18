/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code RemovePathPolicyTask} describes the MD-SAL transaction task that
 * deletes the specified path policy configuration.
 *
 * @see #create(TopologyGraph, RemovePathPolicyInput)
 */
public final class RemovePathPolicyTask extends DeleteDataTask<VtnPathPolicy>
    implements RpcOutputGenerator<VtnUpdateType, Void> {
    /**
     * Runtime context for updating the path policy.
     */
    private final PathPolicyRpcContext  context;

    /**
     * Create a new task that removes the specified path policy.
     *
     * @param topo   The network topology graph.
     * @param input  A {@link RemovePathPolicyInput} instance.
     * @return  A {@link RemovePathPolicyTask} associated with the task that
     *          removes the given path policy.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemovePathPolicyTask create(TopologyGraph topo,
                                              RemovePathPolicyInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        Integer id = input.getId();
        if (id == null) {
            throw PathPolicyUtils.getNullPolicyIdException();
        }

        return new RemovePathPolicyTask(topo, id);
    }

    /**
     * Construct a new instance.
     *
     * @param topo  The network topology graph.
     * @param id    The identifier of the target path policy.
     */
    private RemovePathPolicyTask(TopologyGraph topo, Integer id) {
        super(LogicalDatastoreType.OPERATIONAL,
              PathPolicyUtils.getIdentifier(id));
        context = new PathPolicyRpcContext(topo, id);
    }

    // DeleteDataTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx, VtnPathPolicy current)
        throws VTNException {
        if (current == null) {
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
        // Remove all flow entries affected by the target path policy.
        addBackgroundTask(provider.removeFlows(context.getFlowRemover()));

        context.onUpdated();
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
