/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.WriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * Utility class to wrap mdsal transactions.
 *
 */
public final class MdsalUtils {
    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory.getLogger(MdsalUtils.class);

    /**
     * The number of seconds to wait for completion of read operation.
     */
    private static final long  READ_TIMEOUT = 30L;

    /**
     * Data broker SAL service.
     */
    private final DataBroker  dataBroker;

    /**
     * Class constructor setting the data broker.
     *
     * @param db   A {@link DataBroker} instance.
     */
    public MdsalUtils(DataBroker db) {
        dataBroker = db;
    }

    /**
     * Executes delete as a blocking transaction.
     *
     * @param store {@link LogicalDatastoreType} which should be modified
     * @param path {@link InstanceIdentifier} to read from
     * @param <D> the data object type
     * @return the result of the request
     */
    public <D extends DataObject> boolean delete(
        final LogicalDatastoreType store, final InstanceIdentifier<D> path)  {
        boolean result = false;
        final WriteTransaction transaction = dataBroker.newWriteOnlyTransaction();
        transaction.delete(store, path);
        CheckedFuture<Void, TransactionCommitFailedException> future = transaction.submit();
        try {
            future.checkedGet();
            result = true;
        } catch (TransactionCommitFailedException e) {
            LOG.warn("Failed to delete {} ", path, e);
        }
        return result;
    }

    /**
     * Executes merge as a blocking transaction.
     *
     * @param <D> the data object type
     * @param logicalDatastoreType {@link LogicalDatastoreType} which should be modified
     * @param path {@link InstanceIdentifier} for path to read
     * @param data the data object of type D
     * @return the result of the request
     */
    public <D extends DataObject> boolean merge(
        final LogicalDatastoreType logicalDatastoreType, final InstanceIdentifier<D> path, D data)  {
        boolean result = false;
        final WriteTransaction transaction = dataBroker.newWriteOnlyTransaction();
        transaction.merge(logicalDatastoreType, path, data, true);
        CheckedFuture<Void, TransactionCommitFailedException> future = transaction.submit();
        try {
            future.checkedGet();
            result = true;
        } catch (TransactionCommitFailedException e) {
            LOG.warn("Failed to merge {} ", path, e);
        }
        return result;
    }

    /**
     * Executes put as a blocking transaction.
     *
     * @param <D> the data object type
     * @param logicalDatastoreType {@link LogicalDatastoreType} which should be modified
     * @param path {@link InstanceIdentifier} for path to read
     * @param data the data object of type D
     * @return the result of the request
     */
    public <D extends DataObject> boolean put(
        final LogicalDatastoreType logicalDatastoreType, final InstanceIdentifier<D> path, D data)  {
        boolean result = false;
        final WriteTransaction transaction = dataBroker.newWriteOnlyTransaction();
        transaction.put(logicalDatastoreType, path, data, true);
        CheckedFuture<Void, TransactionCommitFailedException> future = transaction.submit();
        try {
            future.checkedGet();
            result = true;
        } catch (TransactionCommitFailedException e) {
            LOG.warn("Failed to put {} ", path, e);
        }
        return result;
    }

    /**
     * Executes read as a blocking transaction.
     *
     * @param store {@link LogicalDatastoreType} to read
     * @param path  {@link InstanceIdentifier} for path to read
     * @param <D>   The data object type
     * @return  An {@link Optional} instance that contains the result.
     */
    public <D extends DataObject> Optional<D> read(
        final LogicalDatastoreType store, final InstanceIdentifier<D> path)  {
        try (ReadOnlyTransaction rtx = dataBroker.newReadOnlyTransaction()) {
            CheckedFuture<Optional<D>, ReadFailedException> future =
                rtx.read(store, path);
            return future.checkedGet(READ_TIMEOUT, TimeUnit.SECONDS);
        } catch (ReadFailedException e) {
            String msg = "Failed to read data: store=" + store + ", path=" +
                path;
            LOG.error(msg, e);
        } catch (TimeoutException e) {
            String msg = "Read timed out: store=" + store + ", path=" + path;
            LOG.error(msg, e);
        }

        return Optional.<D>absent();
    }
}
