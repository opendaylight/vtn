/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMapsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code ClearPathMapTask} describes the MD-SAL datastore transaction task
 * that clears the global or VTN path map configuration.
 *
 * @see #create(ClearPathMapInput)
 */
public final class ClearPathMapTask extends AbstractTxTask<VtnUpdateType>
    implements RpcOutputGenerator<VtnUpdateType, ClearPathMapOutput> {
    /**
     * The identifier for the target VTN.
     *
     * <p>
     *   {@code null} means that the global path map is targeted.
     * </p>
     */
    private final VTenantIdentifier  identifier;

    /**
     * Create a new task that clears the specified path map container.
     *
     * @param input  A {@link ClearPathMapInput} instance.
     * @return  A {@link ClearPathMapTask} associated with the task that
     *          clears the specified path map container.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static ClearPathMapTask create(ClearPathMapInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        String tname = input.getTenantName();
        VTenantIdentifier ident = (tname == null)
            ? null
            : VTenantIdentifier.create(tname, true);
        return new ClearPathMapTask(ident);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target VTN.
     *               {@code null} means that the global path map is targeted.
     */
    private ClearPathMapTask(VTenantIdentifier ident) {
        identifier = ident;
    }

    /**
     * Clear the global path map container.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  {@link VtnUpdateType#REMOVED} if at least one path map in the
     *          global path map container has been removed.
     *          {@code null} is returned if no path map is present in the
     *          global path map container.
     * @throws VTNException  An error occurred.
     */
    private VtnUpdateType clearGlobal(TxContext ctx) throws VTNException {
        // Read the current value.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<GlobalPathMaps> path =
            InstanceIdentifier.create(GlobalPathMaps.class);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<GlobalPathMaps> opt = DataStoreUtils.read(tx, oper, path);
        VtnUpdateType removed;
        if (opt.isPresent() && !PathMapUtils.isEmpty(opt.get())) {
            removed = VtnUpdateType.REMOVED;
        } else {
            removed = null;
        }

        // Put an empty root container.
        GlobalPathMaps maps = new GlobalPathMapsBuilder().build();
        tx.put(oper, path, maps, true);

        return removed;
    }

    /**
     * Clear the VTN path map container in the specified VTN.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  {@link VtnUpdateType#REMOVED} if at least one path map in the
     *          global path map container has been removed.
     *          {@code null} is returned if no path map is present in the
     *          global path map container.
     * @throws VTNException  An error occurred.
     */
    private VtnUpdateType clearVtn(TxContext ctx) throws VTNException {
        // Read the current value.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnPathMaps> path = identifier.
            getIdentifierBuilder().
            child(VtnPathMaps.class).
            build();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<VtnPathMaps> opt = DataStoreUtils.read(tx, oper, path);
        VtnUpdateType removed;
        if (opt.isPresent()) {
            removed = (PathMapUtils.isEmpty(opt.get()))
                ? null : VtnUpdateType.REMOVED;

            // Delete the VTN path map container.
            tx.delete(oper, path);
        } else {
            // Ensure that the target VTN is present.
            identifier.fetch(tx);
            removed = null;
        }

        return removed;
    }

    // AbstractTxTask

    /**
     * Clear the specified path map container.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  {@link VtnUpdateType#REMOVED} if at least one path map has
     *          been removed. {@code null} is returned if no path map is
     *          present in the specified path map container.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected VtnUpdateType execute(TxContext ctx) throws VTNException {
        return (identifier == null) ? clearGlobal(ctx) : clearVtn(ctx);
    }

    // TxTask

    /**
     * Determine whether the transaction queue should log the given error
     * or not.
     *
     * <p>
     *   This method returns {@code false} if a {@link RpcException} is
     *   passed.
     * </p>
     *
     * @param t  A {@link Throwable} that is going to be thrown.
     * @return   {@code true} if the transaction queue should log the given
     *           {@link Throwable}. Otherwise {@code false}.
     */
    @Override
    public boolean needErrorLog(Throwable t) {
        return !(t instanceof RpcException);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, VtnUpdateType result) {
        if (result != null) {
            // REVISIT: Select flow entries affected by the change.
            FlowRemover remover = (identifier == null)
                ? new AllFlowRemover()
                : new TenantFlowRemover(identifier);
            addBackgroundTask(provider.removeFlows(remover));
        }
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<ClearPathMapOutput> getOutputType() {
        return ClearPathMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public ClearPathMapOutput createOutput(VtnUpdateType result) {
        return new ClearPathMapOutputBuilder().setStatus(result).build();
    }
}
