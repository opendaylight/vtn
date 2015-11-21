/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * An abstract implementation of
 * {@link org.opendaylight.vtn.manager.internal.TxTask} used to delete
 * a data object in he MD-SAL datastore.
 *
 * <p>
 *   This task returns {@link VtnUpdateType#REMOVED} if the target data has
 *   been successfully removed, and {@code null} if the target data is not
 *   present.
 * </p>
 *
 * @param <D>  The type of data object in MD-SAL datastore.
 */
public abstract class DeleteDataTask<D extends DataObject>
    extends AbstractDataTask<D, VtnUpdateType> {

    /**
     * Construct a new instance.
     *
     * @param store  The target type of the MD-SAL datastore.
     * @param path   The path to the target object.
     */
    public DeleteDataTask(LogicalDatastoreType store,
                          InstanceIdentifier<D> path) {
        super(store, path);
    }

    // AbstractTxTask

    /**
     * Delete a data object in the MD-SAL datastore.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  A {@link VtnUpdateType} instance which indicates the result of
     *          this task.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final VtnUpdateType execute(TxContext ctx) throws VTNException {
        // Read the current value.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        InstanceIdentifier<D> path = getTargetPath();
        LogicalDatastoreType store = getDatastoreType();
        D current = DataStoreUtils.read(tx, store, path).orNull();
        onStarted(ctx, current);
        VtnUpdateType ret;
        if (current == null) {
            // The target data object is not present.
            ret = null;
        } else {
            tx.delete(store, path);
            onDeleted(ctx);
            ret = VtnUpdateType.REMOVED;
        }

        return ret;
    }

    /**
     * Invoked when the MD-SAL datastore transaction has started.
     *
     * @param ctx      A runtime context for transaction task.
     * @param current  The current value at the target path in the MD-SAL
     *                 datastore. Note that {@code null} is passed if the
     *                 target data is not present.
     * @throws VTNException  An error occurred.
     */
    protected void onStarted(TxContext ctx, D current) throws VTNException {
        // Nothing to do. Subclass may override this method.
    }

    /**
     * Invoked when the target data object has been deleted.
     *
     * @param ctx  A runtime context for transaction task.
     * @throws VTNException  An error occurred.
     */
    protected void onDeleted(TxContext ctx) throws VTNException {
        // Nothing to do. Subclass may override this method.
    }
}
