/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

/**
 * {@code ReadTransactionHolder} holds a single read-only transaction for
 * MD-SAL datastore.
 *
 * <p>
 *   This class is used to read data from datastore using a single transaction.
 *   Actual transaction is created on the first call of {@link #get()}.
 * </p>
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
public final class ReadTransactionHolder implements AutoCloseable {
    /**
     * Data broker service.
     */
    private final DataBroker  dataBroker;

    /**
     * Read-write datastore transaction.
     */
    private ReadOnlyTransaction  transaction;

    /**
     * Construct a new instance.
     *
     * @param db  Data broker service.
     */
    public ReadTransactionHolder(DataBroker db) {
        dataBroker = db;
    }

    /**
     * Return a read-only datastore transaction.
     *
     * @return  A {@link ReadTransaction} instance.
     */
    public ReadTransaction get() {
        ReadOnlyTransaction tx = transaction;
        if (tx == null) {
            tx = dataBroker.newReadOnlyTransaction();
            transaction = tx;
        }

        return tx;
    }

    // AutoCloseable

    /**
     * Close this datastore transaction.
     */
    @Override
    public void close() {
        ReadOnlyTransaction tx = transaction;
        transaction = null;
        if (tx != null) {
            tx.close();
        }
    }
}
