/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathCostConfigBuilder;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.output.SetPathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.output.SetPathCostResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code SetPathCostTask} describes the MD-SAL transaction task that set
 * all the specified link cost configurations into the path policy
 * configuration.
 *
 * @see #create(TopologyGraph, SetPathCostInput)
 */
public final class SetPathCostTask
    extends CompositeTxTask<VtnUpdateType, SetCostTask>
    implements RpcOutputGenerator<List<VtnUpdateType>, SetPathCostOutput> {
    /**
     * Runtime context for updating the path policy.
     */
    private final PathPolicyRpcContext  context;

    /**
     * Construct a new task that set all the given link cost configurations
     * into the given path policy.
     *
     * @param topo   The network topology graph.
     * @param input  A {@link SetPathCostInput} instance.
     * @return  A {@link SetPathCostTask} instance associated with the task
     *          that set the given link cost configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetPathCostTask create(TopologyGraph topo,
                                         SetPathCostInput input)
        throws RpcException {
        Integer id = getPathPolicyId(input);
        List<PathCostList> list = input.getPathCostList();
        if (list == null || list.isEmpty()) {
            throw PathPolicyUtils.getNoSwitchPortException();
        }

        List<SetCostTask> taskList = new ArrayList<>();
        Set<String> descSet = new HashSet<>();
        for (PathCostList pcl: list) {
            VtnPathCost vpc = new PathCostConfigBuilder.Data().set(pcl).
                getBuilder().build();
            String desc = vpc.getPortDesc().getValue();
            if (!descSet.add(desc)) {
                throw PathPolicyUtils.getDuplicatePortException(desc);
            }
            taskList.add(new SetCostTask(id, vpc));
        }

        return new SetPathCostTask(topo, id, taskList);
    }

    /**
     * Return the path policy ID configured in the given RPC input.
     *
     * @param input  A {@link SetPathCostInput} instance.
     * @return  The path policy identifier in the given input.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    private static Integer getPathPolicyId(SetPathCostInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        Integer id = input.getId();
        if (id == null) {
            throw PathPolicyUtils.getNullPolicyIdException();
        }

        return id;
    }

    /**
     * Construct a new instance.
     *
     * @param topo   The network topology graph.
     * @param id     The identifier of the target path policy.
     * @param tasks  A list of tasks that set link cost configuration.
     */
    private SetPathCostTask(TopologyGraph topo, Integer id,
                            List<SetCostTask> tasks) {
        super(tasks);
        context = new PathPolicyRpcContext(topo, id);
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx) throws VTNException {
        // Ensure that the target path policy is present.
        Integer pid = context.getPolicyId();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        PathPolicyUtils.readVtnPathPolicy(tx, pid.intValue());

        context.onStarted();
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider,
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

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<SetPathCostOutput> getOutputType() {
        return SetPathCostOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetPathCostOutput createOutput(List<VtnUpdateType> result) {
        List<SetPathCostResult> list = new ArrayList<>();
        Iterator<SetCostTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            SetCostTask task = taskIterator.next();
            SetPathCostResult res = new SetPathCostResultBuilder().
                setPortDesc(task.getPortDesc()).setStatus(status).build();
            list.add(res);
        }

        return new SetPathCostOutputBuilder().
            setSetPathCostResult(list).build();
    }
}
