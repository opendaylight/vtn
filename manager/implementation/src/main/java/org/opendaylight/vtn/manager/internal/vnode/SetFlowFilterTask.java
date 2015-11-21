/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code SetFlowFilterTask} describes the MD-SAL datastore transaction task
 * that sets all the specified flow filter configurations into the flow filter
 * list in the specified virtual node.
 *
 * @see  #create(SetFlowFilterInput)
 */
public final class SetFlowFilterTask
    extends CompositeTxTask<VtnUpdateType, SetFilterTask>
    implements RpcOutputGenerator<List<VtnUpdateType>, SetFlowFilterOutput> {
    /**
     * The identifier for the target virtual node.
     */
    private final VNodeIdentifier<?>  identifier;

    /**
     * Construct a new task that set all the given flow filter configurations
     * into the specified flow filter list.
     *
     * @param input  A {@link SetFlowFilterInput} instance.
     * @return  A {@link SetFlowFilterTask} instance associated with the task
     *          that set the given flow filter configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetFlowFilterTask create(SetFlowFilterInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target virtual node.
        VNodeIdentifier<?> ident = VNodeIdentifier.create(input, true);

        // Ensure that the given flow filter list is not empty.
        List<VtnFlowFilter> vfilters = input.getVtnFlowFilter();
        if (vfilters == null || vfilters.isEmpty()) {
            throw RpcException.getMissingArgumentException(
                "At least one flow filter must be specified.");
        }

        List<SetFilterTask> taskList = new ArrayList<>();
        Set<Integer> indices = new HashSet<>();
        boolean output = Boolean.TRUE.equals(input.isOutput());
        for (VtnFlowFilter vff: vfilters) {
            // Verify the given flow filter configuration.
            VTNFlowFilter ff = VTNFlowFilter.create(vff);
            Integer index = ff.getIdentifier();
            if (!indices.add(index)) {
                String msg = "Duplicate flow filter index: " + index;
                throw RpcException.getBadArgumentException(msg);
            }
            ff.canSet(ident);

            // This will throw an exception if the specified virtual node
            // does not support flow filter.
            InstanceIdentifier<VtnFlowFilter> path =
                ident.getFlowFilterIdentifier(output, index);
            taskList.add(new SetFilterTask(path, ff.toVtnFlowFilter()));
        }

        return new SetFlowFilterTask(ident, taskList);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VNodeIdentifier} instance that specifies the
     *               target virtual node.
     * @param tasks  A list of tasks that set flow filter configuration.
     */
    private SetFlowFilterTask(VNodeIdentifier<?> ident,
                              List<SetFilterTask> tasks) {
        super(tasks);
        identifier = ident;
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx) throws VTNException {
        // Ensure that the target virtual node is present.
        identifier.fetch(ctx.getReadWriteTransaction());
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
                String tname = identifier.getTenantNameString();
                TenantFlowRemover remover = new TenantFlowRemover(tname);
                addBackgroundTask(provider.removeFlows(remover));
                break;
            }
        }
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<SetFlowFilterOutput> getOutputType() {
        return SetFlowFilterOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetFlowFilterOutput createOutput(List<VtnUpdateType> result) {
        List<FlowFilterResult> list = new ArrayList<>();
        Iterator<SetFilterTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            SetFilterTask task = taskIterator.next();
            FlowFilterResult res = new FlowFilterResultBuilder().
                setIndex(task.getIndex()).setStatus(status).build();
            list.add(res);
        }

        return new SetFlowFilterOutputBuilder().
            setFlowFilterResult(list).build();
    }
}
