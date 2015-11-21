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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.output.SetPathMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.output.SetPathMapResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code SetPathMapTask} describes the MD-SAL datastore transaction task
 * that set all the specified path map configurations into the global or VTN
 * path map configuration.
 *
 * @see #create(SetPathMapInput)
 */
public final class SetPathMapTask
    extends CompositeTxTask<VtnUpdateType, SetMapTask>
    implements RpcOutputGenerator<List<VtnUpdateType>, SetPathMapOutput> {
    /**
     * The identifier for the target VTN.
     *
     * <p>
     *   {@code null} means that the global path map is targeted.
     * </p>
     */
    private final VTenantIdentifier  identifier;

    /**
     * Construct a new task that set all the given path map configurations
     * into the global or VTN path map.
     *
     * @param input  A {@link SetPathMapInput} instance.
     * @return  A {@link SetPathMapTask} instance associated with the task
     *          that set the given path map configuration.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetPathMapTask create(SetPathMapInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        String tname = input.getTenantName();
        VTenantIdentifier ident = (tname == null)
            ? null
            : VTenantIdentifier.create(tname, true);

        List<PathMapList> vmaps = input.getPathMapList();
        if (vmaps == null || vmaps.isEmpty()) {
            throw RpcException.getMissingArgumentException(
                "At least one path map must be specified.");
        }

        List<SetMapTask> taskList = new ArrayList<>();
        Set<Integer> indices = new HashSet<>();
        for (PathMapList pml: vmaps) {
            VtnPathMap vpm = PathMapUtils.toVtnPathMapBuilder(pml).build();
            Integer index = vpm.getIndex();
            PathMapUtils.verifyMapIndex(indices, index);
            InstanceIdentifier<VtnPathMap> path = (ident == null)
                ? PathMapUtils.getIdentifier(index)
                : PathMapUtils.getIdentifier(ident, index);
            taskList.add(new SetMapTask(path, vpm));
        }

        return new SetPathMapTask(ident, taskList);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target VTN.
     *               {@code null} means that the global path map is targeted.
     * @param tasks  A list of tasks that set path map configuration.
     */
    private SetPathMapTask(VTenantIdentifier ident, List<SetMapTask> tasks) {
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
    public Class<SetPathMapOutput> getOutputType() {
        return SetPathMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetPathMapOutput createOutput(List<VtnUpdateType> result) {
        List<SetPathMapResult> list = new ArrayList<>();
        Iterator<SetMapTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            SetMapTask task = taskIterator.next();
            SetPathMapResult res = new SetPathMapResultBuilder().
                setIndex(task.getIndex()).setStatus(status).build();
            list.add(res);
        }

        return new SetPathMapOutputBuilder().
            setSetPathMapResult(list).build();
    }
}
