/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * {@code ChangedData} describes a change of a data object in the MD-SAL
 * datastore.
 *
 * @param <T>  The type of the data object.
 */
public final class ChangedData<T extends DataObject>
    extends IdentifiedData<T> {
    /**
     * A data object prior to the change.
     */
    private final T  oldValue;

    /**
     * Create a new {@link ChangedData} instance.
     *
     * @param type  A class which indicates the target data type.
     * @param path  An instance identifier that specifies the data location in
     *              the MD-SAL datastore.
     * @param data  A data object that contains the current value.
     * @param old   A data object prior to the change.
     * @param <D>   The type of the target data.
     * @return  A {@link ChangedData} instance on success.
     *          {@code null} if the target data type of {@code path} does not
     *          match the type specified by {@code type}.
     * @throws DataTypeMismatchException
     *    The type of {@code data} or {@code old} does not match the expected
     *    target type.
     */
    public static <D extends DataObject> ChangedData<D> create(
        Class<D> type, InstanceIdentifier<?> path, DataObject data,
        DataObject old) throws DataTypeMismatchException {
        InstanceIdentifier<D> id = DataStoreUtils.cast(type, path);
        return (id == null) ? null : create(id, data, old);
    }

    /**
     * Create a new {@link ChangedData} instance.
     *
     * @param path  An instance identifier that specifies the data location in
     *              the MD-SAL datastore.
     * @param data  A data object that contains the current value.
     * @param old   A data object prior to the change.
     * @param <D>   The type of the target data.
     * @return  A {@link ChangedData} instance.
     * @throws DataTypeMismatchException
     *    The type of {@code data} or {@code old} does not match the expected
     *    target type.
     * @throws IllegalArgumentException
     *    {@code path} is {@code null}.
     */
    public static <D extends DataObject> ChangedData<D> create(
        InstanceIdentifier<D> path, DataObject data, DataObject old)
        throws DataTypeMismatchException {
        if (path == null) {
            throw new IllegalArgumentException(
                "Instance identifier cannot be null.");
        }

        D v = MiscUtils.checkedCast(path.getTargetType(), data);
        D ov = MiscUtils.checkedCast(path.getTargetType(), old);
        return new ChangedData<D>(path, v, ov);
    }

    /**
     * Construct a new instance.
     *
     * @param id    An instance identifier that specifies the data location in
     *              the MD-SAL datastore.
     * @param data  A data object that contains the current value.
     * @param old   A data object prior to the change.
     */
    public ChangedData(InstanceIdentifier<T> id, T data, T old) {
        super(id, data);
        oldValue = old;
    }

    /**
     * Return the data object prior to the change.
     *
     * @return  A data object.
     */
    public T getOldValue() {
        return oldValue;
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
    @Override
    @SuppressWarnings("unchecked")
    public <D extends DataObject> ChangedData<D> checkType(Class<D> type) {
        if (type.equals(getIdentifier().getTargetType())) {
            return (ChangedData<D>)this;
        }

        return null;
    }
}
