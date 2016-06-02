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


    /**
     * Record a warning log message that indicates unexpected data was
     * notified.
     *
     * @param logger  A logger instance.
     * @param type    A {@link VtnUpdateType} instance that indicates the type
     *                of event.
     */
    @Override
    public void unexpected(Logger logger, VtnUpdateType type) {
        logger.warn("Unexpected data: type={}, path={}, old={}, new={}",
                    type, getIdentifier(), oldValue, getValue());
    }
}
