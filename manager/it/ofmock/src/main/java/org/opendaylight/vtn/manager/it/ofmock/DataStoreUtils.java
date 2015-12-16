/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * {@code DataStoreUtils} class is a collection of utility class methods
 * for MD-SAL datastore handling.
 */
public final class DataStoreUtils {
    /**
     * Datastore read timeout in seconds.
     */
    private static final long  READ_TIMEOUT = 5;

    /**
     * The number of nanoseconds to wait for completion of transaction submit.
     */
    private static final long  SUBMIT_TIMEOUT = TimeUnit.SECONDS.toNanos(10L);

    /**
     * Private constructor that protects this class from instantiating.
     */
    private DataStoreUtils() {}

    /**
     * Read data from the MD-SAL operational datastore at the given path.
     *
     * @param rtx   A read transaction.
     * @param path  An {@link InstanceIdentifier} instance which specifies
     *              data to be read.
     * @param <T>   Type of data to be read.
     * @return  An {@link Optional} instance that contains the result.
     * @throws IllegalStateException
     *    Failed to read data.
     */
    public static <T extends DataObject> Optional<T> read(
        ReadTransaction rtx, InstanceIdentifier<T> path) {
        return read(rtx, LogicalDatastoreType.OPERATIONAL, path);
    }

    /**
     * Read data from the given MD-SAL data store at the given path.
     *
     * @param rtx    A read transaction.
     * @param store  A {@link LogicalDatastoreType} which specifies the type
     *               of logical datastore.
     * @param path   An {@link InstanceIdentifier} instance which specifies
     *               data to be read.
     * @param <T>    Type of data to be read.
     * @return  An {@link Optional} instance that contains the result.
     *          {@code null} if no data is present.
     * @throws IllegalStateException
     *    Failed to read data.
     */
    public static <T extends DataObject> Optional<T> read(
        ReadTransaction rtx, LogicalDatastoreType store,
        InstanceIdentifier<T> path) {
        return read(rtx.read(store, path));
    }

    /**
     * Wait for completion of the given read transaction future, and return
     * data.
     *
     * @param future  A future associated with a MD-SAL datastore read
     *                transaction.
     * @param <T>    Type of data to be read.
     * @return  An {@link Optional} instance that contains the result.
     *          {@code null} if no data is present.
     * @throws IllegalStateException
     *    Failed to read data.
     */
    public static <T extends DataObject> Optional<T> read(
        CheckedFuture<Optional<T>, ReadFailedException> future)
        throws IllegalStateException {
        try {
            return future.checkedGet(READ_TIMEOUT, TimeUnit.SECONDS);
        } catch (TimeoutException e) {
            throw new IllegalStateException("Datastore read timed out.", e);
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to read data from datastore.", e);
        }
    }

    /**
     * Delete data at the specified path.
     *
     * <p>
     *   This method tries to read data at the specified path, and deletes
     *   data only if the data is present.
     * </p>
     *
     * @param tx     A MD-SAL data transaction.
     * @param store  A {@link LogicalDatastoreType} which specifies the type
     *               of logical datastore.
     * @param path   An {@link InstanceIdentifier} instance which specifies
     *               data to be deleted.
     * @param <T>    Type of data to be deleted.
     * @return  {@code true} if data was actually deleted.
     *          {@code false} if data is not present.
     * @throws IllegalStateException
     *    Failed to read data.
     */
    public static <T extends DataObject> boolean delete(
        ReadWriteTransaction tx, LogicalDatastoreType store,
        InstanceIdentifier<T> path) {
        boolean ret;
        if (read(tx, store, path).isPresent()) {
            tx.delete(store, path);
            ret = true;
        } else {
            ret = false;
        }

        return ret;
    }

    /**
     * Submit the given MD-SAL datastore transaction.
     *
     * @param tx  A {@link ReadWriteTransaction} instance.
     * @throws IllegalStateException
     *    Failed to submit the given transaction.
     */
    public static void submit(ReadWriteTransaction tx) {
        CheckedFuture<Void, TransactionCommitFailedException> future =
            tx.submit();
        try {
            future.checkedGet(SUBMIT_TIMEOUT, TimeUnit.NANOSECONDS);
        } catch (TimeoutException e) {
            throw new IllegalStateException("Datastore submit timed out.", e);
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to submit the transaction.", e);
        }
    }

    /**
     * Cast the given instance identifier for the given target type.
     *
     * @param type  A class which indicates the target type.
     * @param path  An {@link InstanceIdentifier} instance.
     * @param <T>   The type of the data targeted by the instance identifier.
     * @return  An {@link InstanceIdentifier} instance on sucecss.
     *          {@code null} on failure.
     */
    @SuppressWarnings("unchecked")
    public static <T extends DataObject> InstanceIdentifier<T> cast(
        Class<T> type, InstanceIdentifier<?> path) {
        Class<?> target = path.getTargetType();
        if (target.equals(type)) {
            return (InstanceIdentifier<T>)path;
        }

        return null;
    }
}
