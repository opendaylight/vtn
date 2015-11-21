/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * An abstract implementation of {@link DeleteDataTask} used to remove
 * the specified virtual node from the MD-SAL datastore.
 *
 * <p>
 *   This task returns a {@link VtnUpdateType} instance that indicates the
 *   result of the task.
 * </p>
 * <dl style="margin-left: 1em;">
 *   <dt style="font-weight: bold;">{@link VtnUpdateType#REMOVED}
 *   <dd>
 *     Indicates that the target virtual node has been removed.
 *
 *   <dt style="font-weight: bold;">{@code null}
 *   <dd>
 *     The target virtual node is not present.
 * </dl>
 *
 * @param <T>  The type of target virtual node.
 * @param <I>  The type of the identifier for the target virtual node.
 */
public abstract class RemoveVirtualNodeTask
    <T extends DataObject, I extends VNodeIdentifier<T>>
    extends DeleteDataTask<T>
    implements RpcOutputGenerator<VtnUpdateType, Void> {
    /**
     * The identifier for the target data object.
     */
    private final I  identifier;

    /**
     * A flow remover that specifies the flow entries to be removed.
     */
    private FlowRemover  flowRemover;

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target virtual node.
     */
    protected RemoveVirtualNodeTask(I ident) {
        super(LogicalDatastoreType.OPERATIONAL, ident.getIdentifier());
        identifier = ident;
    }

    /**
     * Return an identifier for the target virtual node.
     *
     * @return  A {@link VNodeIdentifier} instance.
     */
    protected final I getIdentifier() {
        return identifier;
    }

    /**
     * Set a {@link FlowRemover} instance that specifies the flow entries
     * to be removed.
     *
     * @param fr  A {@link FlowRemover} instance.
     */
    protected final void setFlowRemover(FlowRemover fr) {
        flowRemover = fr;
    }

    /**
     * Destroy the target virtual node.
     *
     * @param ctx      A runtime context for transaction task.
     * @param current  The virtual node to be removed.
     * @throws VTNException  An error occurred.
     */
    protected abstract void destroy(TxContext ctx, T current)
        throws VTNException;

    // DeleteDataTask

    /**
     * {@inheritDoc}
     */
    protected final void onStarted(TxContext ctx, T current)
        throws VTNException {
        if (current == null) {
            throw identifier.getNotFoundException();
        }

        flowRemover = null;
        destroy(ctx, current);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public final Class<Void> getOutputType() {
        return Void.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final Void createOutput(VtnUpdateType result) {
        return null;
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public final void onSuccess(VTNManagerProvider provider,
                                VtnUpdateType result) {
        if (flowRemover != null) {
            addBackgroundTask(provider.removeFlows(flowRemover));
        }
    }
}
