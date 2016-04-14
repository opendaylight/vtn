/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.slf4j.Logger;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code IdentifiedData} describes a data object identified by an instance
 * identifier.
 *
 * <p>
 *   An instance of this class keeps a pair of instance identifier and a
 *   data object.
 * </p>
 *
 * @param <T>  The type of the data object.
 */
public class IdentifiedData<T extends DataObject> {
    /**
     * An instance identifier that specifies the data location in the
     * MD-SAL datastore.
     */
    private final InstanceIdentifier<T>  identifier;

    /**
     * A data object.
     */
    private final T  value;

    /**
     * Create a new {@link IdentifiedData} instance.
     *
     * @param type  A class which indicates the target data type.
     * @param path  An instance identifier that specifies the data location in
     *              the MD-SAL datastore.
     * @param data  A data object.
     * @param <D>   The type of the target data.
     * @return  An {@link IdentifiedData} instance on success.
     *          {@code null} if the target data type of {@code path} does not
     *          match the type specified by {@code type}.
     * @throws DataTypeMismatchException
     *    The type of {@code data} does not match the expected target type.
     */
    public static final <D extends DataObject> IdentifiedData<D> create(
        Class<D> type, InstanceIdentifier<?> path, DataObject data)
        throws DataTypeMismatchException {
        InstanceIdentifier<D> id = DataStoreUtils.cast(type, path);
        return (id == null) ? null : create(id, data);
    }

    /**
     * Create a new {@link IdentifiedData} instance.
     *
     * @param path  An instance identifier that specifies the data location in
     *              the MD-SAL datastore.
     * @param data  A data object.
     * @param <D>   The type of the target data.
     * @return  An {@link IdentifiedData} instance.
     * @throws DataTypeMismatchException
     *    The type of {@code data} does not match the expected target type.
     * @throws IllegalArgumentException
     *    {@code path} is {@code null}.
     */
    public static final <D extends DataObject> IdentifiedData<D> create(
        InstanceIdentifier<D> path, DataObject data)
        throws DataTypeMismatchException {
        if (path == null) {
            throw new IllegalArgumentException(
                "Instance identifier cannot be null.");
        }

        D v = MiscUtils.checkedCast(path.getTargetType(), data);
        return new IdentifiedData<D>(path, v);
    }

    /**
     * Construct a new instance.
     *
     * @param id    An instance identifier that specifies the data location in
     *              the MD-SAL datastore.
     * @param data  A data object.
     */
    public IdentifiedData(InstanceIdentifier<T> id, T data) {
        identifier = id;
        value = data;
    }

    /**
     * Return the instance identifier configured in this instance.
     *
     * @return  An {@link InstanceIdentifier} instance.
     */
    public final InstanceIdentifier<T> getIdentifier() {
        return identifier;
    }

    /**
     * Return the data object configured in this instance.
     *
     * @return  A data object.
     */
    public final T getValue() {
        return value;
    }

    /**
     * Check whether the target type of this instance matches the given type
     * or not.
     *
     * @param type  A class which indicates the target type.
     * @param <D>   The expected target type.
     * @return  This instance if the given type matches the target type of
     *          this instance. Otherwise {@code null}.
     */
    @SuppressWarnings("unchecked")
    public <D extends DataObject> IdentifiedData<D> checkType(Class<D> type) {
        if (type.equals(identifier.getTargetType())) {
            return (IdentifiedData<D>)this;
        }

        return null;
    }

    /**
     * Record a warning log message that indicates unexpected data was
     * notified.
     *
     * @param logger  A logger instance.
     * @param type    A {@link VtnUpdateType} instance that indicates the type
     *                of event.
     */
    public void unexpected(Logger logger, VtnUpdateType type) {
        logger.warn("Unexpected data: type={}, path={}, value={}",
                    type, identifier, value);
    }
}
