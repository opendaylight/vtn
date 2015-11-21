/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractRpcTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * An abstract implementation of
 * {@link org.opendaylight.vtn.manager.internal.TxTask} used to create or
 * update a virtual node in the MD-SAL datastore.
 *
 * <p>
 *   This task returns a {@link VtnUpdateType} instance that indicates the
 *   result of the task.
 * </p>
 * <dl style="margin-left: 1em;">
 *   <dt style="font-weight: bold;">{@link VtnUpdateType#CREATED}
 *   <dd>
 *     Indicates that the target virtual node has been newly created.
 *
 *   <dt style="font-weight: bold;">{@link VtnUpdateType#CHANGED}
 *   <dd>
 *     Indicates that the target virtual node is present and it has been
 *     successfully updated.
 *
 *   <dt style="font-weight: bold;">{@code null}
 *   <dd>
 *     No change has been made to the target virtual node.
 * </dl>
 *
 * @param <T>  The type of target virtual node.
 * @param <C>  The type of configuration container in the target virtual node.
 * @param <I>  The type of the identifier for the target virtual node.
 */
public abstract class UpdateVirtualNodeTask
    <T extends DataObject, C extends DataObject, I extends VNodeIdentifier<T>>
    extends AbstractRpcTask<VtnUpdateType> {
    /**
     * The identifier for the target data object.
     */
    private final I  identifier;

    /**
     * Describes how to update the virtual node.
     */
    private final VnodeUpdateMode  updateMode;

    /**
     * Describes how to update attributes for existing virtual node.
     */
    private final VtnUpdateOperationType  operationType;

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target virtual node.
     * @param mode   A {@link VnodeUpdateMode} instance.
     * @param op     A {@link VtnUpdateOperationType} instance.
     * @throws RpcException  An error occurred.
     */
    protected UpdateVirtualNodeTask(I ident, VnodeUpdateMode mode,
                                    VtnUpdateOperationType op)
        throws RpcException {
        identifier = ident;
        updateMode = (mode == null) ? VnodeUpdateMode.UPDATE : mode;
        if (op == null) {
            // Default update operation mode is ADD.
            operationType = VtnUpdateOperationType.ADD;
        } else if (op != VtnUpdateOperationType.REMOVE) {
            operationType = op;
        } else {
            throw RpcUtils.getInvalidOperationException(op);
        }
    }

    /**
     * Update the configuration for the target virtual node.
     *
     * @param ctx      A runtime context for transaction task.
     * @param current  The target virtual node.
     * @return  {@link VtnUpdateType#CHANGED} if the virtual node was changed.
     *          {@code null} if unchanged.
     * @throws VTNException  An error occurred.
     */
    private VtnUpdateType update(TxContext ctx, T current)
        throws VTNException {
        C cur = getConfig(current);
        C cfg;
        if (operationType == VtnUpdateOperationType.SET) {
            // Apply the given configuration as-is.
            cfg = createConfig();
        } else {
            // Merge the given configuration.
            cfg = mergeConfig(cur);
        }

        VtnUpdateType utype;
        if (equalsConfig(cur, cfg)) {
            // Not changed.
            utype = null;
        } else {
            // Apply the new configuration.
            InstanceIdentifier<C> cpath = getConfigPath(identifier);
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            tx.put(oper, cpath, cfg, false);
            utype = VtnUpdateType.CHANGED;
            onUpdated(ctx);
        }

        return utype;
    }

    // AbstractTxTask

    /**
     * Create or update the specified virtual node in the MD-SAL datastore.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  A {@link VtnUpdateType} instance which indicates the result.
     *          {@code null} is returned if the target virtual node was not
     *          changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final VtnUpdateType execute(TxContext ctx) throws VTNException {
        // Ensure that the parent node is present.
        checkParent(ctx);

        // Read the current value.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<T> opt = identifier.read(tx);
        T current;
        if (opt.isPresent()) {
            if (updateMode == VnodeUpdateMode.CREATE) {
                throw identifier.getDataExistsException();
            }
            current = opt.get();
        } else if (updateMode != VnodeUpdateMode.MODIFY) {
            current = null;
        } else {
            throw identifier.getNotFoundException();
        }

        VtnUpdateType utype;
        if (current == null) {
            // Create a new virtual node.
            create(ctx, identifier, createConfig());
            utype = VtnUpdateType.CREATED;
        } else {
            // Update the specified virtual node.
            utype = update(ctx, current);
        }

        return utype;
    }

    /**
     * Return the identifier for the target data object.
     *
     * @return  The identifier for the target data object.
     */
    public final I getIdentifier() {
        return identifier;
    }

    /**
     * Invoked when the configuration for the target virtual node has been
     * changed.
     *
     * <p>
     *   This method of this class does nothing.
     *   Subclass may override this method.
     * </p>
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    protected void onUpdated(TxContext ctx) throws VTNException {
    }

    /**
     * Ensure that the parent node is present.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    protected abstract void checkParent(TxContext ctx) throws VTNException;

    /**
     * Create a new virtual node configuration as specified by the RPC.
     *
     * @return  A new configuration container.
     * @throws RpcException
     *    The configuration specified by the RPC is invalid.
     */
    protected abstract C createConfig() throws RpcException;

    /**
     * Merge the virtual node configuration specified by the RPC with the
     * current configuration.
     *
     * @param cur  The current configuration of the target virtual node.
     * @return  A new configuration container.
     * @throws RpcException
     *    The configuration specified by the RPC is invalid.
     */
    protected abstract C mergeConfig(C cur) throws RpcException;

    /**
     * Return the virtual node configuration in the given virtual node.
     *
     * @param vnode  The virtual node.
     * @return  The configuration for the given virtual node.
     */
    protected abstract C getConfig(T vnode);

    /**
     * Return the instance identifier for the configuration container in the
     * target virtual node.
     *
     * @param ident  The identifier for the target virtual node.
     * @return  The instance identifier for the configuration container in the
     *          target virtual node.
     */
    protected abstract InstanceIdentifier<C> getConfigPath(I ident);

    /**
     * Determine whether the given two virtual node configurations are
     * identical or not.
     *
     * @param cur  The current configuration.
     * @param cfg  The configuration that is going to be applied.
     * @return  {@code true} only if the given two configurations are
     *           identical.
     */
    protected abstract boolean equalsConfig(C cur, C cfg);

    /**
     * Create a new virtual node.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the new virtual node.
     * @param cfg    The configuration for the new virtual node.
     * @throws VTNException  An error occurred.
     */
    protected abstract void create(TxContext ctx, I ident, C cfg)
        throws VTNException;
}
