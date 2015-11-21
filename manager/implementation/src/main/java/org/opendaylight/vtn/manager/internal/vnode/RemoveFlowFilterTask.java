/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter.checkIndexNotNull;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code RemoveFlowFilterTask} describes the MD-SAL datastore transaction task
 * that deletes all the specified flow filter configurations from the specified
 * virtual node.
 *
 * @see #create(VNodeIdentifier, boolean, List)
 */
public final class RemoveFlowFilterTask
    extends CompositeTxTask<VtnUpdateType, RemoveFilterTask>
    implements RpcOutputGenerator<List<VtnUpdateType>, RemoveFlowFilterOutput> {
    /**
     * The identifier for the target virtual node.
     */
    private final VNodeIdentifier<?>  identifier;

    /**
     * Construct a new task that removes all the given flow filter
     * configurations from the specified virtual node.
     *
     * @param ident    A {@link VNodeIdentifier} instance that specifies the
     *                 target virtual node.
     * @param output   {@code true} indicates the output filter.
     *                 {@code false} indicates the input filter.
     * @param indices  A list of flow filter indices to be removed.
     *                 The caller must ensure that the list is not empty.
     * @return  A {@link RemoveFlowFilterTask} instance associated with the
     *          task that removes the given flow filter configuration.
     * @throws RpcException  An error occurred.
     */
    public static RemoveFlowFilterTask create(
        VNodeIdentifier<?> ident, boolean output, List<Integer> indices)
        throws RpcException {
        Set<Integer> idxSet = new HashSet<>();
        List<RemoveFilterTask> taskList = new ArrayList<>();
        for (Integer index: indices) {
            if (idxSet.add(checkIndexNotNull(index))) {
                // This will throw an exception if the specified virtual node
                // does not support flow filter.
                InstanceIdentifier<VtnFlowFilter> path =
                    ident.getFlowFilterIdentifier(output, index);
                taskList.add(new RemoveFilterTask(path, index));
            }
        }

        return new RemoveFlowFilterTask(ident, taskList);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VNodeIdentifier} instance that specifies the
     *               target virtual node.
     * @param tasks  A list of tasks that delete flow filter configuration.
     */
    private RemoveFlowFilterTask(VNodeIdentifier<?> ident,
                                 List<RemoveFilterTask> tasks) {
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
    public Class<RemoveFlowFilterOutput> getOutputType() {
        return RemoveFlowFilterOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemoveFlowFilterOutput createOutput(List<VtnUpdateType> result) {
        List<FlowFilterResult> list = new ArrayList<>();
        Iterator<RemoveFilterTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            RemoveFilterTask task = taskIterator.next();
            FlowFilterResult res = new FlowFilterResultBuilder().
                setIndex(task.getIndex()).setStatus(status).build();
            list.add(res);
        }

        return new RemoveFlowFilterOutputBuilder().
            setFlowFilterResult(list).build();
    }
}
