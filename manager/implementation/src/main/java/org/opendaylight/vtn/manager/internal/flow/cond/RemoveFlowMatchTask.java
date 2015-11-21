/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.remove.flow.condition.match.output.RemoveMatchResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.remove.flow.condition.match.output.RemoveMatchResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code RemoveFlowMatchTask} describes the MD-SAL datastore transaction task
 * that deletes all the specified flow match configurations in the flow
 * condition configuration.
 *
 * @see #create(RemoveFlowConditionMatchInput)
 */
public final class RemoveFlowMatchTask
    extends CompositeTxTask<VtnUpdateType, RemoveMatchTask>
    implements RpcOutputGenerator<List<VtnUpdateType>,
                                  RemoveFlowConditionMatchOutput> {
    /**
     * The name of the target flow condition.
     */
    private final VnodeName  nodeName;

    /**
     * Construct a new task that removes all the given flow match
     * configurations from the given flow condition.
     *
     * @param input  A {@link RemoveFlowConditionMatchInput} instance.
     * @return  A {@link RemoveFlowMatchTask} instance associated with the task
     *          that removes the given flow match configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemoveFlowMatchTask create(
        RemoveFlowConditionMatchInput input) throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        VnodeName vname = FlowCondUtils.getVnodeName(input.getName());
        List<Integer> indexList = input.getMatchIndex();
        if (indexList == null || indexList.isEmpty()) {
            throw FlowCondUtils.getMatchIndexMissingException();
        }

        Set<Integer> idxSet = new HashSet<>();
        List<RemoveMatchTask> taskList = new ArrayList<>();
        for (Integer index: indexList) {
            if (idxSet.add(index)) {
                taskList.add(new RemoveMatchTask(vname, index));
            }
        }

        return new RemoveFlowMatchTask(vname, taskList);
    }

    /**
     * Construct a new instance.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of the
     *               target flow condition.
     * @param tasks  A list of tasks that delete flow match configuration.
     */
    private RemoveFlowMatchTask(VnodeName vname, List<RemoveMatchTask> tasks) {
        super(tasks);
        nodeName = vname;
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx) throws VTNException {
        // Ensure that the target flow condition is present.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        FlowCondUtils.checkPresent(tx, nodeName);
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
                // REVISIT: Select flow entries affected by the change.
                addBackgroundTask(provider.removeFlows(new AllFlowRemover()));
                break;
            }
        }
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<RemoveFlowConditionMatchOutput> getOutputType() {
        return RemoveFlowConditionMatchOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemoveFlowConditionMatchOutput createOutput(
        List<VtnUpdateType> result) {
        List<RemoveMatchResult> list = new ArrayList<>();
        Iterator<RemoveMatchTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            RemoveMatchTask task = taskIterator.next();
            RemoveMatchResult res = new RemoveMatchResultBuilder().
                setIndex(task.getIndex()).setStatus(status).build();
            list.add(res);
        }

        return new RemoveFlowConditionMatchOutputBuilder().
            setRemoveMatchResult(list).build();
    }
}
