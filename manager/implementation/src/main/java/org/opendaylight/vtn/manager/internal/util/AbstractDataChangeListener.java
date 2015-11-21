/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Collections;
import java.util.Map;
import java.util.Set;

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
 * @param <T>  The type of the target data type.
 * @param <C>  Type of event context.
 */
public abstract class AbstractDataChangeListener<T extends DataObject, C>
    extends CloseableContainer implements DataChangeListener {
    /**
     * The type of the target data.
     */
    private final Class<T>  targetType;

    /**
     * Construct a new instance.
     *
     * @param cls  A {@link Class} instance that represents the target type.
     */
    protected AbstractDataChangeListener(Class<T> cls) {
        targetType = cls;
    }

    /**
     * Return the type of the target data model.
     *
     * @return  A {@link Class} instance that represents the target type.
     */
    public final Class<T> getTargetType() {
        return targetType;
    }

    /**
     * Register this instance as a data change listener.
     *
     * @param broker  A {@link DataBroker} service instance.
     * @param store   A {@link LogicalDatastoreType} instance used to determine
     *                the target datastore.
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
            addCloseable(reg);
        } catch (Exception e) {
            StringBuilder builder =
                new StringBuilder("Failed to register data change listener: ");
            String msg = builder.append(targetType.getName()).toString();
            getLogger().error(msg, e);
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Verify an instance identifier notified by a data change event.
     *
     * @param path   An instance identifier that specifies data.
     * @param label  A label only for logging.
     * @return  {@code true} only if a given instance identifier  is valid.
     */
    protected final boolean checkPath(InstanceIdentifier<?> path,
                                      Object label) {
        if (path == null) {
            getLogger().warn("{}: Null instance identifier.", label);
            return false;
        }
        if (path.isWildcarded()) {
            MiscUtils.VERBOSE_LOG.
               trace("{}: Ignore wildcard path: {}", label, path);
            return false;
        }

        return true;
    }

    /**
     * Create an {@link IdentifiedData} that represents a data object notified
     * by a data change event.
     *
     * @param type   A class which indicates the expected data type.
     * @param path   An instance identifier that specifies data.
     * @param value  An object associated with {@code path}.
     * @param label  A label only for logging.
     * @param <D>    The type of the expected target type.
     * @return  An {@link IdentifiedData} on success.
     *          Otherwise {@code null}.
     */
    protected final <D extends DataObject> IdentifiedData<D> createIdentifiedData(
        Class<D> type, InstanceIdentifier<?> path, DataObject value,
        Object label) {
        try {
            return IdentifiedData.create(type, path, value);
        } catch (DataTypeMismatchException e) {
            unexpectedData(path, e.getObject(), label);
        }

        return null;
    }

    /**
     * Create an {@link ChangedData} that represents a change of a data object
     * notified by a data change event.
     *
     * @param type   A class which indicates the expected data type.
     * @param path   An instance identifier that specifies data.
     * @param value  An object associated with {@code path}.
     * @param old    An object prior to the change.
     * @param label  A label only for logging.
     * @param <D>    The type of the expected target type.
     * @return  An {@link ChangedData} on success.
     *          Otherwise {@code null}.
     */
    protected final <D extends DataObject> ChangedData<D> createChangedData(
        Class<D> type, InstanceIdentifier<?> path, DataObject value,
        DataObject old, Object label) {
        try {
            return ChangedData.create(type, path, value, old);
        } catch (DataTypeMismatchException e) {
            unexpectedData(path, e.getObject(), label);
        }

        return null;
    }

    /**
     * Record a log that indicates a data change event has notified unwanted
     * type of instance identifier.
     *
     * @param path   An {@link InstanceIdentifier} instance.
     * @param label  A label only for logging.
     */
    protected final void unwantedIdentifier(InstanceIdentifier<?> path,
                                            Object label) {
        MiscUtils.VERBOSE_LOG.
            trace("{}: Unwanted target type in instance identifier: {}",
                  label, path);
    }

    /**
     * Record a log that indicates a data change event has notified unexpected
     * data object.
     *
     * @param path   An {@link InstanceIdentifier} instance.
     * @param value  A data object associated with {@code path}.
     * @param label  A label only for logging.
     */
    protected final void unexpectedData(InstanceIdentifier<?> path,
                                        Object value, Object label) {
        getLogger().warn("{}: Unexpected data is associated: " +
                         "path={}, value={}", label, path, value);
    }

    /**
     * Return a set of {@link VtnUpdateType} instances that specifies
     * event types to be listened.
     *
     * <p>
     *   Note that this method of this class always returns {@code null},
     *   which means all event types should be listened. Subclass can override
     *   this method to filter out events.
     * </p>
     *
     * @return  A set of {@link VtnUpdateType} instances or {@code null}.
     */
    protected Set<VtnUpdateType> getRequiredEvents() {
        return null;
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
     * Handle data creation events.
     *
     * @param ectx     An event context created by
     *                 {@link #enterEvent(AsyncDataChangeEvent)}.
     * @param created  A map that contains information about newly created
     *                 data objects.
     */
    protected abstract void onCreated(
        C ectx, Map<InstanceIdentifier<?>, DataObject> created);

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
    protected abstract void onUpdated(
        C ectx, Map<InstanceIdentifier<?>, DataObject> updated,
        Map<InstanceIdentifier<?>, DataObject> original);

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
    protected abstract void onRemoved(
        C ectx, Set<InstanceIdentifier<?>> removed,
        Map<InstanceIdentifier<?>, DataObject> original);

    /**
     * Return a wildcard instance identifier that specifies data objects
     * to be listened.
     *
     * @return  A wildcard instance identifier.
     */
    protected abstract InstanceIdentifier<T> getWildcardPath();

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

            if (isRequiredEvent(required, VtnUpdateType.REMOVED)) {
                // Process removal events.
                onRemoved(ectx, ev.getRemovedPaths(), original);
            }

            if (isRequiredEvent(required, VtnUpdateType.CHANGED)) {
                // Process change events.
                onUpdated(ectx, ev.getUpdatedData(), original);
            }
        } catch (RuntimeException e) {
            getLogger().error(
                "Unexpected exception in data change event listener.", e);
        } finally {
            exitEvent(ectx);
        }
    }
}
