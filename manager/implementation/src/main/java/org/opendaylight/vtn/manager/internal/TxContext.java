/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

/**
 * {@code TxContext} represents a runtime context used by tasks that uses
 * MD-SAL datastore.
 *
 * @see TxTask
 */
public interface TxContext {
    /**
     * Return read-only transaction for MD-SAL datastore.
     *
     * @return  A {@link ReadTransaction} instance.
     */
    ReadTransaction getTransaction();

    /**
     * Return read/write transaction for MD-SAL datastore.
     *
     * @return  A {@link ReadWriteTransaction} instance.
     */
    ReadWriteTransaction getReadWriteTransaction();

    /**
     * Return VTN inventory reader that uses current transaction.
     *
     * @return  An {@link InventoryReader} instance.
     */
    InventoryReader getInventoryReader();

    /**
     * Return flow condition reader that uses current transaction.
     *
     * @return  A {@link FlowCondReader} instance.
     */
    FlowCondReader getFlowCondReader();

    /**
     * Cancel current transaction for MD-SAL datastore.
     */
    void cancelTransaction();

    /**
     * Return a {@link VTNManagerProvider} service instance.
     *
     * @return  A {@link VTNManagerProvider} instance.
     */
    VTNManagerProvider getProvider();
}
