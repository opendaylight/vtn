/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * Base class for MD-SAL data change listeners that maintain the VTN inventory
 * data.
 *
 * @param <T>  Type of data object in the MD-SAL datastore to listen.
 * @param <C>  Type of event context.
 */
public abstract class InventoryMaintainer<T extends DataObject, C>
    extends DataStoreListener<T, C> {
    /**
     * The transaction submit queue for the VTN inventory data models.
     */
    private final TxQueue  txQueue;

    /**
     * Construct a new instance.
     *
     * @param queue   A {@link TxQueue} instance used to update the
     *                VTN inventory.
     * @param broker  A {@link DataBroker} service instance.
     * @param clz     A {@link Class} instance that represents the target type.
     * @param scope   A {@link DataChangeScope} instance used to register
     *                data change listener.
     */
    protected InventoryMaintainer(TxQueue queue, DataBroker broker,
                                  Class<T> clz, DataChangeScope scope) {
        super(clz);
        txQueue = queue;
        registerListener(broker, LogicalDatastoreType.OPERATIONAL, scope,
                         false);
    }

    /**
     * Execute the given transaction task on the transaction queue.
     *
     * @param task  A {@link TxTask} that updates the MD-SAL datastore.
     */
    protected final void submit(TxTask<?> task) {
        txQueue.post(task);
    }

    /**
     * Execute the given transaction task for initialization on the
     * transaction queue.
     *
     * @param task  A {@link TxTask} that initializes the MD-SAL datastore.
     */
    protected final void submitInitial(TxTask<?> task) {
        txQueue.postFirst(task);
    }
}
