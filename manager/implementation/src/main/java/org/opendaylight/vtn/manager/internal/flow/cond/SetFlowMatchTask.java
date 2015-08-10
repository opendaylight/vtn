/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
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
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowMatch;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.output.SetMatchResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.output.SetMatchResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code SetFlowMatchTask} describes the MD-SAL datastore transaction task
 * that set all the specified flow match configurations into the flow condition
 * configuration.
 *
 * @see #create(SetFlowConditionMatchInput)
 */
public final class SetFlowMatchTask
    extends CompositeTxTask<VtnUpdateType, SetMatchTask>
    implements RpcOutputGenerator<List<VtnUpdateType>,
                                  SetFlowConditionMatchOutput> {
    /**
     * The name of the target flow condition.
     */
    private final VnodeName  nodeName;

    /**
     * Construct a new task that set all the given flow match configurations
     * into the given flow condition.
     *
     * @param input  A {@link SetFlowConditionMatchInput} instance.
     * @return  A {@link SetFlowMatchTask} instance associated with the task
     *          that set the given flow match configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetFlowMatchTask create(SetFlowConditionMatchInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        VnodeName vname = FlowCondUtils.getVnodeName(input.getName());
        List<FlowMatchList> vmatches = input.getFlowMatchList();
        if (vmatches == null || vmatches.isEmpty()) {
            throw RpcException.getMissingArgumentException(
                "At least one flow match must be specified.");
        }

        List<SetMatchTask> taskList = new ArrayList<>();
        Set<Integer> indices = new HashSet<>();
        for (FlowMatchList fml: vmatches) {
            VTNFlowMatch vfmatch = new VTNFlowMatch(fml);
            Integer index = vfmatch.getIdentifier();
            FlowCondUtils.verifyMatchIndex(indices, index);
            InstanceIdentifier<VtnFlowMatch> path =
                FlowCondUtils.getIdentifier(vname, index);
            VtnFlowMatch vfm = vfmatch.toVtnFlowMatchBuilder().build();
            taskList.add(new SetMatchTask(path, vfm));
        }

        return new SetFlowMatchTask(vname, taskList);
    }

    /**
     * Construct a new instance.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of the
     *               target flow condition.
     * @param tasks  A list of tasks that set flow match configuration.
     */
    private SetFlowMatchTask(VnodeName vname, List<SetMatchTask> tasks) {
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
    public Class<SetFlowConditionMatchOutput> getOutputType() {
        return SetFlowConditionMatchOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetFlowConditionMatchOutput createOutput(
        List<VtnUpdateType> result) {
        List<SetMatchResult> list = new ArrayList<>();
        Iterator<SetMatchTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            SetMatchTask task = taskIterator.next();
            SetMatchResult res = new SetMatchResultBuilder().
                setIndex(task.getIndex()).setStatus(status).build();
            list.add(res);
        }

        return new SetFlowConditionMatchOutputBuilder().
            setSetMatchResult(list).build();
    }
}
