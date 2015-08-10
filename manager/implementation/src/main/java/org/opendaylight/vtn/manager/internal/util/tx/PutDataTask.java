/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.DataObjectIdentity;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * An abstract implementation of
 * {@link org.opendaylight.vtn.manager.internal.TxTask} used to create or
 * update a data object in the MD-SAL datastore.
 *
 * <p>
 *   This task returns a {@link VtnUpdateType} instance that indicates the
 *   result of the task.
 * </p>
 * <dl style="margin-left: 1em;">
 *   <dt style="font-weight: bold;">{@link VtnUpdateType#CREATED}
 *   <dd>
 *     Indicates that the target data object has been newly created.
 *
 *   <dt style="font-weight: bold;">{@link VtnUpdateType#CHANGED}
 *   <dd>
 *     Indicates that the target data is present and it has been successfully
 *     updated.
 *
 *   <dt style="font-weight: bold;">{@code null}
 *   <dd>
 *     No change has been made to the target data object.
 * </dl>
 *
 * @param <D>  The type of data object in MD-SAL datastore.
 */
public abstract class PutDataTask<D extends DataObject>
    extends AbstractDataTask<D, VtnUpdateType> {
    /**
     * A data object to be submitted.
     */
    private D  dataObject;

    /**
     * Determine whether the target data object should be replaced or not.
     */
    private final boolean  doReplace;

    /**
     * Construct a new instance.
     *
     * @param store  The target type of the MD-SAL datastore.
     * @param path   The path to the target object.
     * @param obj    The data object to be submitted.
     * @param repl   If {@code true}, the target object will be replaced with
     *               the given object. Otherwise the given object will be
     *               merged with the target object.
     */
    public PutDataTask(LogicalDatastoreType store, InstanceIdentifier<D> path,
                       D obj, boolean repl) {
        super(store, path);
        dataObject = obj;
        doReplace = repl;
    }

    /**
     * Return the data object to be submitted.
     *
     * @return  A data object to be submitted.
     */
    public final D getDataObject() {
        return dataObject;
    }

    /**
     * Set the data object to be submitted.
     *
     * @param data  A data object to be submitted.
     */
    public final void setDataObject(D data) {
        dataObject = data;
    }

    /**
     * Determine whether missing parent nodes should be created or not.
     *
     * @return {@code true} if parent nodes should be created if missing.
     *         Otherwise {@code false}.
     */
    protected boolean fixMissingParents() {
        return false;
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

    // AbstractTxTask

    /**
     * Create or update a data object in the MD-SAL datastore.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  A {@link VtnUpdateType} instance which indicates the result.
     *          {@code null} is returned if the target data object was not
     *          changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final VtnUpdateType execute(TxContext ctx) throws VTNException {
        // Read the current value.
        LogicalDatastoreType store = getDatastoreType();
        InstanceIdentifier<D> path = getTargetPath();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        D current = DataStoreUtils.read(tx, store, path).orNull();
        onStarted(ctx, current);

        // Apply changes.
        boolean createParents = fixMissingParents();
        if (doReplace) {
            tx.put(store, path, dataObject, createParents);
        } else {
            tx.merge(store, path, dataObject, createParents);
        }

        // Determine whether the target data has been changed or not.
        if (current == null) {
            return VtnUpdateType.CREATED;
        }

        D updated = DataStoreUtils.read(tx, store, path).orNull();
        DataObjectIdentity newData = new DataObjectIdentity(updated);
        DataObjectIdentity oldData = new DataObjectIdentity(current);
        return (oldData.equals(newData)) ? null : VtnUpdateType.CHANGED;
    }
}
