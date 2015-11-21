/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

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
     * @throws VTNException  Failed to read data.
     */
    public static <T extends DataObject> Optional<T> read(
        ReadTransaction rtx, InstanceIdentifier<T> path) throws VTNException {
        return read(rtx, LogicalDatastoreType.OPERATIONAL, path);
    }

    /**
     * Read data from the given MD-SAL datastore at the given path.
     *
     * @param rtx    A read transaction.
     * @param store  A {@link LogicalDatastoreType} which specifies the type
     *               of logical datastore.
     * @param path   An {@link InstanceIdentifier} instance which specifies
     *               data to be read.
     * @param <T>    Type of data to be read.
     * @return  An {@link Optional} instance that contains the result.
     * @throws VTNException  Failed to read data.
     */
    public static <T extends DataObject> Optional<T> read(
        ReadTransaction rtx, LogicalDatastoreType store,
        InstanceIdentifier<T> path) throws VTNException {
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
     * @throws VTNException
     *    Failed to read data.
     */
    public static <T extends DataObject> Optional<T> read(
        CheckedFuture<Optional<T>, ReadFailedException> future)
        throws VTNException {
        try {
            return future.checkedGet(READ_TIMEOUT, TimeUnit.SECONDS);
        } catch (TimeoutException e) {
            throw new VTNException(VtnErrorTag.TIMEOUT,
                                   "Datastore read timed out.", e);
        } catch (Exception e) {
            throw new VTNException("Failed to read data from datastore.", e);
        }
    }

    /**
     * Delete data in the MD-SAL operational datastore at the specified path.
     *
     * <p>
     *   This method tries to read data at the specified path, and deletes
     *   data only if the data is present.
     * </p>
     *
     * @param tx     A MD-SAL data transaction.
     * @param path   An {@link InstanceIdentifier} instance which specifies
     *               data to be deleted.
     * @param <T>    Type of data to be deleted.
     * @return  A deleted data object if deleted.
     *          {@code null} if the specified data object is not present.
     * @throws VTNException   Failed to read data.
     */
    public static <T extends DataObject> T delete(
        ReadWriteTransaction tx, InstanceIdentifier<T> path)
        throws VTNException {
        return delete(tx, LogicalDatastoreType.OPERATIONAL, path);
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
     * @throws VTNException  Failed to read data.
     */
    public static <T extends DataObject> T delete(
        ReadWriteTransaction tx, LogicalDatastoreType store,
        InstanceIdentifier<T> path) throws VTNException {
        Optional<T> opt = read(tx, store, path);
        T deleted;
        if (opt.isPresent()) {
            deleted = opt.get();
            tx.delete(store, path);
        } else {
            deleted = null;
        }

        return deleted;
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
