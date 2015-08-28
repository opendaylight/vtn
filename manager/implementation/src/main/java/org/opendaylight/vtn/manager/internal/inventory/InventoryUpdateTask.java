/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import com.google.common.base.Optional;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * Base class for tasks to update VTN inventory information.
 *
 * @param <T>  The type of MD-SAL inventory to listen.
 * @param <L>  The type of instance which represents the location of the
 *             target inventory.
 */
abstract class InventoryUpdateTask<T extends DataObject, L>
    extends AbstractTxTask<Void> {
    /**
     * A map that keeps locations of notified inventories.
     */
    private final Map<L, InstanceIdentifier<T>>  updated = new HashMap<>();

    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * Construct a new instance.
     *
     * @param log  A {@link Logger} instance.
     */
    InventoryUpdateTask(Logger log) {
        logger = log;
    }

    /**
     * Add an inventory information notified by a data change event.
     *
     * @param path  Path to the target instance.
     * @param loc   An object that represents the location of the target
     *              inventory.
     */
    final void addUpdated(InstanceIdentifier<T> path, L loc) {
        updated.put(loc, path);
    }

    /**
     * Determine whether this task contains at least one notification or not.
     *
     * @return  {@code true} only if this instance contains at least one
     *          notification.
     */
    final boolean hasUpdates() {
        return !updated.isEmpty();
    }

    /**
     * Return a map which keeps updated inventories.
     *
     * <p>
     *   This method is used only for test.
     * </p>
     *
     * @return  A map which keeps updated inventories.
     */
    final Map<L, InstanceIdentifier<T>> getUpdatedMap() {
        return Collections.unmodifiableMap(updated);
    }

    /**
     * Return a {@link Logger} instance.
     *
     * @return  A {@link Logger} instance.
     */
    final Logger getLogger() {
        return logger;
    }

    /**
     * Add an inventory information notified by a data change event.
     *
     * @param ctx   A {@link TxContext} instance.
     * @param tx    A {@link ReadWriteTransaction} instance.
     * @param loc   An object that represents the location of the target
     *              inventory.
     * @param path  Path to the target inventory in the MD-SAL datastore.
     * @param data  An inventory data in the MD-SAL datastore.
     * @throws VTNException  An error occurred.
     */
    protected abstract void add(TxContext ctx, ReadWriteTransaction tx,
                                L loc, InstanceIdentifier<T> path, T data)
        throws VTNException;

    /**
     * Remove an inventory information notified by a data change event.
     *
     * @param ctx   A {@link TxContext} instance.
     * @param tx    A {@link ReadWriteTransaction} instance.
     * @param loc   An object that represents the location of the target
     *              inventory.
     * @throws VTNException  An error occurred.
     */
    protected abstract void remove(TxContext ctx, ReadWriteTransaction tx,
                                   L loc)
        throws VTNException;

    /**
     * A pre-event hook which will be invoked just after a new MD-SAL DS
     * transaction has started.
     *
     * @param ctx    A {@link TxContext} instance.
     * @throws VTNException  An error occurred.
     */
    protected abstract void prepare(TxContext ctx) throws VTNException;

    /**
     * A post-event hook which will be invoked after event processing.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param added  {@code true} is passed if at least one inventory
     *               information has been created.
     * @throws VTNException  An error occurred.
     */
    protected abstract void fixUp(TxContext ctx, boolean added)
        throws VTNException;

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public final Void execute(TxContext ctx) throws VTNException {
        prepare(ctx);

        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        boolean added = false;
        for (Map.Entry<L, InstanceIdentifier<T>> entry: updated.entrySet()) {
            // Read the notified inventory.
            L loc = entry.getKey();
            InstanceIdentifier<T> path = entry.getValue();
            Optional<T> opt = DataStoreUtils.read(tx, oper, path);
            if (opt.isPresent()) {
                // Add the inventory information corresponding to the target
                // inventory.
                add(ctx, tx, loc, path, opt.get());
                added = true;
            } else {
                // Remove the inventory information corresponding to the target
                // inventory.
                remove(ctx, tx, loc);
            }
        }

        fixUp(ctx, added);
        return null;
    }
}
