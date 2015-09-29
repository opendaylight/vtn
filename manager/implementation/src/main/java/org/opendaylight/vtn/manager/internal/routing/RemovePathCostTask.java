/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.remove.path.cost.output.RemovePathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.remove.path.cost.output.RemovePathCostResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code RemovePathCostTask} describes the MD-SAL transaction task that
 * deletes all the specified link cost configurations in the path policy
 * configuration.
 *
 * @see #create(TopologyGraph, RemovePathCostInput)
 */
public final class RemovePathCostTask
    extends CompositeTxTask<VtnUpdateType, RemoveCostTask>
    implements RpcOutputGenerator<List<VtnUpdateType>, RemovePathCostOutput> {
    /**
     * Runtime context for updating the path policy.
     */
    private final PathPolicyRpcContext  context;

    /**
     * Construct a new task that removes all the given link cost configurations
     * from the given path policy.
     *
     * @param topo   The network topology graph.
     * @param input  A {@link RemovePathCostInput} instance.
     * @return  A {@link RemovePathCostTask} instance associated with the task
     *          that removes the given link cost configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemovePathCostTask create(TopologyGraph topo,
                                            RemovePathCostInput input)
        throws RpcException {
        Integer id = getPathPolicyId(input);
        List<VtnPortDesc> portList = input.getPortDesc();
        if (portList == null || portList.isEmpty()) {
            throw PathPolicyUtils.getNoSwitchPortException();
        }

        Map<String, RemoveCostTask> taskMap = new HashMap<>();
        for (VtnPortDesc vdesc: portList) {
            if (vdesc == null) {
                throw NodeUtils.getNullPortDescException();
            }

            String key = vdesc.getValue();
            if (key == null) {
                throw NodeUtils.getNullPortDescException();
            }

            if (!taskMap.containsKey(key)) {
                RemoveCostTask task = new RemoveCostTask(id, vdesc);
                taskMap.put(key, task);
            }
        }

        List<RemoveCostTask> taskList = new ArrayList<>(taskMap.values());
        return new RemovePathCostTask(topo, id, taskList);
    }

    /**
     * Return the path policy ID configured in the given RPC input.
     *
     * @param input  A {@link RemovePathCostInput} instance.
     * @return  The path policy identifier in the given input.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    private static Integer getPathPolicyId(RemovePathCostInput input)
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
     * @param tasks  A list of tasks that delete link cost configuration.
     */
    private RemovePathCostTask(TopologyGraph topo, Integer id,
                               List<RemoveCostTask> tasks) {
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
    public Class<RemovePathCostOutput> getOutputType() {
        return RemovePathCostOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovePathCostOutput createOutput(List<VtnUpdateType> result) {
        List<RemovePathCostResult> list = new ArrayList<>();
        Iterator<RemoveCostTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            RemoveCostTask task = taskIterator.next();
            RemovePathCostResult res = new RemovePathCostResultBuilder().
                setPortDesc(task.getPortDesc()).setStatus(status).build();
            list.add(res);
        }

        return new RemovePathCostOutputBuilder().
            setRemovePathCostResult(list).build();
    }
}
