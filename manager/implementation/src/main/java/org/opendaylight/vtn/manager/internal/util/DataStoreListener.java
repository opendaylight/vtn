/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Map;
import java.util.Set;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for MD-SAL datastore listener.
 *
 * @param <T>  Type of data object in the datastore to listen.
 * @param <C>  Type of event context.
 */
public abstract class DataStoreListener<T extends DataObject, C>
    extends AbstractDataChangeListener<T, C> {
    /**
     * Construct a new instance.
     *
     * @param cls  A {@link Class} instance that represents the target type.
     */
    protected DataStoreListener(Class<T> cls) {
        super(cls);
    }

    /**
     * Create an {@link IdentifiedData} that represents a data object notified
     * by a data change event.
     *
     * @param path   An instance identifier that specifies data.
     * @param value  An object associated with {@code path}.
     * @param label  A label only for logging.
     * @return  An {@link IdentifiedData} on success.
     *          Otherwise {@code null}.
     */
    private IdentifiedData<T> createIdentifiedData(
        InstanceIdentifier<?> path, DataObject value, Object label) {
        if (checkPath(path, label)) {
            Class<T> target = getTargetType();
            try {
                IdentifiedData<T> data =
                    IdentifiedData.create(target, path, value);
                if (data == null) {
                    unwantedIdentifier(path, label);
                }
                return data;
            } catch (DataTypeMismatchException e) {
                unexpectedData(path, e.getObject(), label);
            }
        }

        return null;
    }

    /**
     * Create an {@link ChangedData} that represents a change of a data object
     * notified by a data change event.
     *
     * @param path   An instance identifier that specifies data.
     * @param value  An object associated with {@code path}.
     * @param old    An object prior to the change.
     * @return  An {@link ChangedData} on success.
     *          Otherwise {@code null}.
     */
    private ChangedData<T> createChangedData(
        InstanceIdentifier<?> path, DataObject value, DataObject old) {
        VtnUpdateType type = VtnUpdateType.CHANGED;
        if (checkPath(path, type)) {
            Class<T> target = getTargetType();
            try {
                ChangedData<T> data =
                    ChangedData.create(target, path, value, old);
                if (data == null) {
                    unwantedIdentifier(path, type);
                }
                return data;
            } catch (DataTypeMismatchException e) {
                unexpectedData(path, e.getObject(), type);
            }
        }

        return null;
    }

    /**
     * Invoked when a new data has been created in the datastore.
     *
     * @param ectx  An event context created by this class.
     * @param data  An {@link IdentifiedData} instance which contains the
     *              created data.
     */
    protected abstract void onCreated(C ectx, IdentifiedData<T> data);

    /**
     * Invoked when a data in the datastore has been updated.
     *
     * @param ectx  An event context created by this class.
     * @param data  A {@link ChangedData} instance which contains the
     *              changed data.
     */
    protected abstract void onUpdated(C ectx, ChangedData<T> data);

    /**
     * Invoked when a data has been removed from the datastore.
     *
     * @param ectx  An event context created by this class.
     * @param data  An {@link IdentifiedData} instance which contains the
     *              removed data.
     */
    protected abstract void onRemoved(C ectx, IdentifiedData<T> data);

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void onCreated(
        C ectx, Map<InstanceIdentifier<?>, DataObject> created) {
        if (created == null) {
            return;
        }

        for (Map.Entry<InstanceIdentifier<?>, DataObject> entry:
                 created.entrySet()) {
            InstanceIdentifier<?> key = entry.getKey();
            DataObject value = entry.getValue();
            IdentifiedData<T> data =
                createIdentifiedData(key, value, VtnUpdateType.CREATED);
            if (data != null) {
                onCreated(ectx, data);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void onUpdated(
        C ectx, Map<InstanceIdentifier<?>, DataObject> updated,
        Map<InstanceIdentifier<?>, DataObject> original) {
        if (updated == null) {
            return;
        }

        for (Map.Entry<InstanceIdentifier<?>, DataObject> entry:
                 updated.entrySet()) {
            InstanceIdentifier<?> key = entry.getKey();
            DataObject value = entry.getValue();
            DataObject org = original.get(key);
            ChangedData<T> data = createChangedData(key, value, org);
            if (data != null) {
                onUpdated(ectx, data);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void onRemoved(
        C ectx, Set<InstanceIdentifier<?>> removed,
        Map<InstanceIdentifier<?>, DataObject> original) {
        if (removed == null) {
            return;
        }

        for (InstanceIdentifier<?> key: removed) {
            DataObject value = original.get(key);
            IdentifiedData<T> data =
                createIdentifiedData(key, value, VtnUpdateType.REMOVED);
            if (data != null) {
                onRemoved(ectx, data);
            }
        }
    }
}
