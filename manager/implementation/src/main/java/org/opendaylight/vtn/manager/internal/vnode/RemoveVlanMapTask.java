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
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code RemoveVlanMapTask} describes the MD-SAL datastore transaction task
 * that removes all the specified VLAN mappings from the specified vBridge.
 *
 * @see #create(VBridgeIdentifier, List)
 */
public final class RemoveVlanMapTask
    extends CompositeTxTask<VtnUpdateType, DeleteVlanMapTask>
    implements RpcOutputGenerator<List<VtnUpdateType>, RemoveVlanMapOutput> {
    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  identifier;

    /**
     * Construct a new task that removes all the specified VLAN mappings from
     * the specified vBridge.
     *
     * @param ident  A {@link VBridgeIdentifier} instance that specifies the
     *               target vBridge.
     * @param ids    A list of VLAN mappings IDs to be removed.
     *               The caller must ensure that the list is not empty.
     * @return  A {@link RemoveVlanMapTask} instance associated with the task
     *          that removes all the given VLAN mappings from the specified
     *          vBridge.
     * @throws RpcException  An error occurred.
     */
    public static RemoveVlanMapTask create(
        VBridgeIdentifier ident, List<String> ids) throws RpcException {
        Set<String> idSet = new HashSet<>();
        List<DeleteVlanMapTask> taskList = new ArrayList<>();
        for (String id: ids) {
            // Reject null ID.
            if (id == null) {
                throw RpcException.getNullArgumentException("VLAN mapping ID");
            }

            if (idSet.add(id)) {
                VlanMapIdentifier vmapId = new VlanMapIdentifier(ident, id);
                taskList.add(new DeleteVlanMapTask(vmapId));
            }
        }

        return new RemoveVlanMapTask(ident, taskList);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VBridgeIdentifier} instance that specifies the
     *               target vBridge.
     * @param tasks  A list of tasks that delete VLAN mappings.
     */
    private RemoveVlanMapTask(VBridgeIdentifier ident,
                              List<DeleteVlanMapTask> tasks) {
        super(tasks);
        identifier = ident;
    }

    // CompositeTxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onStarted(TxContext ctx) throws VTNException {
        // Ensure that the target vBridge is present.
        identifier.fetch(ctx.getReadWriteTransaction());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCompleted(TxContext ctx, List<VtnUpdateType> results)
        throws VTNException {
        // Update the status of the target vBridge.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        VBridge vbr = new VBridge(identifier, identifier.fetch(tx));
        vbr.putState(ctx);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<RemoveVlanMapOutput> getOutputType() {
        return RemoveVlanMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RemoveVlanMapOutput createOutput(List<VtnUpdateType> result) {
        List<RemoveVlanMapResult> list = new ArrayList<>();
        Iterator<DeleteVlanMapTask> taskIterator = getSubTasks().iterator();
        for (VtnUpdateType status: result) {
            DeleteVlanMapTask task = taskIterator.next();
            RemoveVlanMapResult res = new RemoveVlanMapResultBuilder().
                setMapId(task.getMapId()).
                setStatus(status).
                build();
            list.add(res);
        }

        return new RemoveVlanMapOutputBuilder().
            setRemoveVlanMapResult(list).build();
    }
}
