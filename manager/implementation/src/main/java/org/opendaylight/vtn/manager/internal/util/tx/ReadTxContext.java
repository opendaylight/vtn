/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxHook;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;

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
     * Read-only data specific to the current transaction.
     */
    private final TxSpecific<ReadTransaction>  readSpecific =
        new TxSpecific<>(ReadTransaction.class);

    /**
     * Construct a new instance.
     *
     * @param provider  A {@link VTNManagerProvider} instance.
     */
    public ReadTxContext(VTNManagerProvider provider) {
        vtnProvider = provider;
    }

    /**
     * Return an exception that indicates read-only transaction.
     *
     * @return  An {@link IllegalStateException} instance.
     */
    private IllegalStateException getReadOnlyException() {
        return new IllegalStateException("Read-only transaction.");
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
        throw getReadOnlyException();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void cancelTransaction() {
        ReadOnlyTransaction tx = transaction;
        if (tx != null) {
            transaction = null;
            readSpecific.clear();
            tx.close();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public <T> T getReadSpecific(Class<T> type) {
        return readSpecific.get(type, getTransaction());
    }

    /**
     * This method always throws an exception because this transaction is
     * read-only.
     *
     * @param type  A class that specifies the type of data.
     * @param <T>   The type of the transaction specific data.
     * @return  Never returns.
     * @throws IllegalStateException   Always thrown.
     */
    @Override
    public <T> T getSpecific(Class<T> type) {
        throw getReadOnlyException();
    }

    /**
     * This method always throws an exception because transaction pre-submit
     * hook is not supported.
     *
     * @param hook  A hook to be invoked when the transaction is going to be
     *              submitted.
     * @throws IllegalStateException   Always thrown.
     */
    @Override
    public void addPreSubmitHook(TxHook hook) {
        throw getReadOnlyException();
    }

    /**
     * This method always throws an exception because transaction post-submit
     * hook is not supported.
     *
     * @param hook  A hook to be invoked after the successful completion of
     *              the transaction.
     * @throws IllegalStateException   Always thrown.
     */
    @Override
    public void addPostSubmitHook(TxHook hook) {
        throw getReadOnlyException();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNManagerProvider getProvider() {
        return vtnProvider;
    }

    // The transaction associated with this context is read-only and never
    // cause data confliction. So log messages should be logged immediately.

    /**
     * {@inheritDoc}
     */
    @Override
    public void log(Logger logger, VTNLogLevel level, String msg) {
        level.log(logger, msg);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void log(Logger logger, VTNLogLevel level, String format,
                    Object ... args) {
        level.log(logger, format, args);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void log(Logger logger, VTNLogLevel level, String msg, Throwable t) {
        level.log(logger, msg, t);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void log(Logger logger, VTNLogLevel level, Throwable t,
                    String format, Object ... args) {
        level.log(logger, t, format, args);
    }
}
