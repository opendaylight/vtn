/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.remove.path.map.output.RemovePathMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.remove.path.map.output.RemovePathMapResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code RemovePathMapTask} describes the MD-SAL datastore transaction task
 * that deletes all the specified path map configurations from the global or
 * VTN path map.
 *
 * @see #create(RemovePathMapInput)
 */
public final class RemovePathMapTask
    extends CompositeTxTask<VtnUpdateType, RemoveMapTask>
    implements RpcOutputGenerator<List<VtnUpdateType>, RemovePathMapOutput> {
    /**
     * The identifier for the target VTN.
     *
     * <p>
     *   {@code null} means that the global path map is targeted.
     * </p>
     */
    private final VTenantIdentifier  identifier;

    /**
     * Construct a new task that removes all the given path map configurations
     * from the global or VTN path map.
     *
     * @param input  A {@link RemovePathMapInput} instance.
     * @return  A {@link RemovePathMapTask} instance associated with the task
     *          that removes the given path map configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static RemovePathMapTask create(RemovePathMapInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        String tname = input.getTenantName();
        VTenantIdentifier ident = (tname == null)
            ? null
            : VTenantIdentifier.create(tname, true);

        List<Integer> indexList = input.getMapIndex();
        if (indexList == null || indexList.isEmpty()) {
            throw PathMapUtils.getNullMapIndexException();
        }

        Set<Integer> idxSet = new HashSet<>();
        List<RemoveMapTask> taskList = new ArrayList<>();
        for (Integer index: indexList) {
            if (idxSet.add(index)) {
                InstanceIdentifier<VtnPathMap> path = (ident == null)
                    ? PathMapUtils.getIdentifier(index)
                    : PathMapUtils.getIdentifier(ident, index);
                taskList.add(new RemoveMapTask(path, index));
            }
        }

        return new RemovePathMapTask(ident, taskList);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target VTN.
     *               {@code null} means that the global path map is targeted.
     * @param tasks  A list of tasks that delete path map configuration.
     */
    private RemovePathMapTask(VTenantIdentifier ident,
                              List<RemoveMapTask> tasks) {
        super(tasks);
        identifier = ident;
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx) throws VTNException {
        if (identifier != null) {
            // Ensure that the target VTN is present.
            identifier.fetch(ctx.getReadWriteTransaction());
        }
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
                FlowRemover remover = (identifier == null)
                    ? new AllFlowRemover()
                    : new TenantFlowRemover(identifier);
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
    public Class<RemovePathMapOutput> getOutputType() {
        return RemovePathMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemovePathMapOutput createOutput(
        List<VtnUpdateType> result) {
        List<RemovePathMapResult> list = new ArrayList<>();
        Iterator<RemoveMapTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            RemoveMapTask task = taskIterator.next();
            RemovePathMapResult res = new RemovePathMapResultBuilder().
                setIndex(task.getIndex()).setStatus(status).build();
            list.add(res);
        }

        return new RemovePathMapOutputBuilder().
            setRemovePathMapResult(list).build();
    }
}
