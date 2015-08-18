/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Collections;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
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
    implements AutoCloseable, DataChangeListener {
    /**
     * The target type.
     */
    private final Class<T>  targetType;

    /**
     * Registration of the data change listener.
     */
    private final AtomicReference<ListenerRegistration<DataChangeListener>> registration =
        new AtomicReference<>();

    /**
     * Construct a new instance.
     *
     * @param clz     A {@link Class} instance that represents the target type.
     */
    protected DataStoreListener(Class<T> clz) {
        targetType = clz;
    }

    /**
     * Register this instance as a data change listener.
     *
     * @param broker  A {@link DataBroker} service instance.
     * @param store   A {@link LogicalDatastoreType} instance used to determine
     *                datastore.
     * @param scope   A {@link DataChangeScope} instance used to register
     *                data change listener.
     */
    protected final void registerListener(DataBroker broker,
                                          LogicalDatastoreType store,
                                          DataChangeScope scope) {
        InstanceIdentifier<?> path = getWildcardPath();
        try {
            ListenerRegistration<DataChangeListener> reg =
                broker.registerDataChangeListener(store, path, this, scope);
            registration.set(reg);
        } catch (Exception e) {
            String msg =
                "Failed to register data change listener for " +
                targetType.getName();
            getLogger().error(msg, e);
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Return the type of data which this instance listens.
     *
     * @return  A {@link Class} instance that represents the target type.
     */
    public final Class<T> getTargetType() {
        return targetType;
    }

    /**
     * Verify data objects notified by a data change event.
     *
     * @param label   A label for logging.
     * @param key     An instance identifier that specifies data.
     * @param values  An array of data objects associated with {@code key}.
     * @return  {@code key} if both the given key and value are valid.
     *          Otherwise {@code null}.
     */
    private InstanceIdentifier<T> checkEvent(String label,
                                             InstanceIdentifier<?> key,
                                             DataObject ... values) {
        if (key == null) {
            getLogger().warn("{}: Null instance identifier.", label);
            return null;
        }
        if (key.isWildcarded()) {
            getLogger().trace("{}: Ignore wildcard path: {}", label, key);
            return null;
        }

        if (!targetType.equals(key.getTargetType())) {
            getLogger().
                trace("{}: Unwanted target type in instance identifier: {}",
                      label, key);
            return null;
        }

        for (DataObject value: values) {
            if (!targetType.isInstance(value)) {
                getLogger().warn("{}: Unexpected data is associated: " +
                                 "key={}, value={}", label, key, value);
                return null;
            }
        }

        @SuppressWarnings("unchecked")
        InstanceIdentifier<T> path = (InstanceIdentifier<T>)key;
        return path;
    }

    /**
     * Determine whether the specified event type should be handled or not.
     *
     * @param required  A set of {@link VtnUpdateType} instances returned
     *                  by {@link #getRequiredEvents()}.
     * @param type      A {@link VtnUpdateType} instance which indicates the
     *                  event type.
     * @return  {@code true} only if the given event type should be handled.
     */
    private boolean isRequiredEvent(Set<VtnUpdateType> required,
                                    VtnUpdateType type) {
        return (required == null || required.contains(type));
    }

    /**
     * Handle data creation events.
     *
     * @param ectx     An event context created by
     *                 {@link #enterEvent(AsyncDataChangeEvent)}.
     * @param created  A map that contains information about newly created
     *                 data objects.
     */
    private void onCreated(C ectx,
                           Map<InstanceIdentifier<?>, DataObject> created) {
        if (created == null) {
            return;
        }

        for (Map.Entry<InstanceIdentifier<?>, DataObject> entry:
                 created.entrySet()) {
            InstanceIdentifier<?> key = entry.getKey();
            DataObject value = entry.getValue();
            InstanceIdentifier<T> path =
                checkEvent("onCreated", key, value);
            if (path != null) {
                @SuppressWarnings("unchecked")
                T v = (T)value;
                onCreated(ectx, path, v);
            }
        }
    }

    /**
     * Handle data update events.
     *
     * @param ectx      An event context created by
     *                  {@link #enterEvent(AsyncDataChangeEvent)}.
     * @param updated   A map that contains information about updated data
     *                  objects.
     * @param original  A map that contains information about data objects
     *                  before update.
     */
    private void onUpdated(C ectx,
                           Map<InstanceIdentifier<?>, DataObject> updated,
                           Map<InstanceIdentifier<?>, DataObject> original) {
        if (updated == null) {
            return;
        }

        for (Map.Entry<InstanceIdentifier<?>, DataObject> entry:
                 updated.entrySet()) {
            InstanceIdentifier<?> key = entry.getKey();
            DataObject value = entry.getValue();
            DataObject org = original.get(key);
            InstanceIdentifier<T> path =
                checkEvent("onUpdated", key, org, value);
            if (path != null) {
                @SuppressWarnings("unchecked")
                T o = (T)org;
                @SuppressWarnings("unchecked")
                T v = (T)value;
                onUpdated(ectx, path, o, v);
            }
        }
    }

    /**
     * Handle data update events.
     *
     * @param ectx      An event context created by
     *                  {@link #enterEvent(AsyncDataChangeEvent)}.
     * @param removed   A set of paths for removed data objects.
     *                  objects.
     * @param original  A map that contains information about data objects
     *                  before removal.
     */
    private void onRemoved(C ectx, Set<InstanceIdentifier<?>> removed,
                           Map<InstanceIdentifier<?>, DataObject> original) {
        if (removed == null) {
            return;
        }

        for (InstanceIdentifier<?> key: removed) {
            DataObject value = original.get(key);
            InstanceIdentifier<T> path = checkEvent("onRemoved", key, value);
            if (path != null) {
                @SuppressWarnings("unchecked")
                T v = (T)value;
                onRemoved(ectx, path, v);
            }
        }
    }

    /**
     * Invoked when {@link #onDataChanged(AsyncDataChangeEvent)} has just been
     * called.
     *
     * @param ev  An {@link AsyncDataChangeEvent} instance.
     * @return  A new event context.
     */
    protected abstract C enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev);

    /**
     * Invoked when {@link #onDataChanged(AsyncDataChangeEvent)} is going to
     * leave.
     *
     * @param ectx  An event context created by
     *              {@link #enterEvent(AsyncDataChangeEvent)}.
     */
    protected abstract void exitEvent(C ectx);

    /**
     * Invoked when a new data has been created in the datastore.
     *
     * @param ectx  An event context created by
     *              {@link #enterEvent(AsyncDataChangeEvent)}.
     * @param path  An instance identifier that specifies data.
     * @param data  A created data object.
     */
    protected abstract void onCreated(C ectx, InstanceIdentifier<T> path,
                                      T data);

    /**
     * Invoked when a data in the datastore has been updated.
     *
     * @param ectx  An event context created by
     *              {@link #enterEvent(AsyncDataChangeEvent)}.
     * @param path     An instance identifier that specifies data.
     * @param oldData  An old data object.
     * @param newData  An updated data object.
     */
    protected abstract void onUpdated(C ectx, InstanceIdentifier<T> path,
                                      T oldData, T newData);

    /**
     * Invoked when a data has been removed from the datastore.
     *
     * @param ectx  An event context created by
     *              {@link #enterEvent(AsyncDataChangeEvent)}.
     * @param path  An instance identifier that specifies data.
     * @param data  A removed data object.
     */
    protected abstract void onRemoved(C ectx, InstanceIdentifier<T> path,
                                      T data);

    /**
     * Return a wildcard instance identifier that specifies data objects
     * to be listened.
     *
     * @return  A wildcard instance identifier.
     */
    protected abstract InstanceIdentifier<T> getWildcardPath();

    /**
     * Return a logger instance.
     *
     * @return  A {@link Logger} instance.
     */
    protected abstract Logger getLogger();

    /**
     * Return a set of {@link VtnUpdateType} instances that represents
     * required event types.
     *
     * <p>
     *   All events will be listened if {@code null} is returned.
     * </p>
     *
     * @return  A set of {@link VtnUpdateType} instances or {@code null}.
     */
    protected abstract Set<VtnUpdateType> getRequiredEvents();

    // AutoCloseable

    /**
     * Close this listener.
     */
    @Override
    public void close() {
        ListenerRegistration<DataChangeListener> reg =
            registration.getAndSet(null);
        if (reg != null) {
            try {
                reg.close();
            } catch (Exception e) {
                String msg = "Failed to unregister data change listener for " +
                    targetType.getName();
                getLogger().error(msg, e);
            }
        }
    }

    // DataChangeListener

    /**
     * Invoked when at least an entry in the datastore has been changed.
     *
     * @param ev  An {@link AsyncDataChangeEvent} instance.
     */
    @Override
    public final void onDataChanged(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        if (ev == null) {
            getLogger().warn("Null data change event.");
            return;
        }

        C ectx = enterEvent(ev);
        Set<VtnUpdateType> required = getRequiredEvents();

        try {
            if (isRequiredEvent(required, VtnUpdateType.CREATED)) {
                // Process creation events.
                onCreated(ectx, ev.getCreatedData());
            }

            Map<InstanceIdentifier<?>, DataObject> original =
                ev.getOriginalData();
            if (original == null) {
                original = Collections.
                    <InstanceIdentifier<?>, DataObject>emptyMap();
            }

            if (isRequiredEvent(required, VtnUpdateType.CHANGED)) {
                // Process change events.
                onUpdated(ectx, ev.getUpdatedData(), original);
            }

            if (isRequiredEvent(required, VtnUpdateType.REMOVED)) {
                // Process removal events.
                onRemoved(ectx, ev.getRemovedPaths(), original);
            }
        } catch (Exception e) {
            getLogger().error(
                "Unexpected exception in data change event listener.", e);
        } finally {
            exitEvent(ectx);
        }
    }
}
