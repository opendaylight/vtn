/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

/**
 * Implementation of read-only MD-SAL transaction context.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
public final class ReadTxContext implements TxContext {
    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * Current MD-SAL datastore read-only transaction.
     */
    private ReadOnlyTransaction  transaction;

    /**
     * A VTN inventory reader.
     */
    private InventoryReader  inventoryReader;

    /**
     * A flow condition reader.
     */
    private FlowCondReader  flowCondReader;

    /**
     * Construct a new instance.
     *
     * @param provider  A {@link VTNManagerProvider} instance.
     */
    public ReadTxContext(VTNManagerProvider provider) {
        vtnProvider = provider;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public ReadTransaction getTransaction() {
        ReadOnlyTransaction tx = transaction;
        if (tx == null) {
            tx = vtnProvider.getDataBroker().newReadOnlyTransaction();
            transaction = tx;
        }

        return tx;
    }

    /**
     * This method always throws an exception because this transaction is
     * read-only.
     *
     * @return  Never returns.
     * @throws IllegalStateException   Always thrown.
     */
    @Override
    public ReadWriteTransaction getReadWriteTransaction() {
        throw new IllegalStateException("Read-only transaction.");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InventoryReader getInventoryReader() {
        InventoryReader reader = inventoryReader;
        if (reader == null) {
            reader = new InventoryReader(getTransaction());
            inventoryReader = reader;
        }

        return reader;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowCondReader getFlowCondReader() {
        FlowCondReader reader = flowCondReader;
        if (reader == null) {
            reader = new FlowCondReader(getTransaction());
            flowCondReader = reader;
        }

        return reader;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void cancelTransaction() {
        ReadOnlyTransaction tx = transaction;
        if (tx != null) {
            transaction = null;
            inventoryReader = null;
            tx.close();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNManagerProvider getProvider() {
        return vtnProvider;
    }
}
