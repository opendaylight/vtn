/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Collection;
import java.util.concurrent.atomic.AtomicReference;

import javax.annotation.Nonnull;

import org.slf4j.Logger;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeService;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
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
    implements AutoCloseable, DataTreeChangeListener<T> {
    /**
     * The target type.
     */
    private final Class<T>  targetType;

    /**
     * Registration of the data tree change listener.
     */
    private final AtomicReference<ListenerRegistration<?>> registration =
        new AtomicReference<>();

    /**
     * Construct a new instance.
     *
     * @param clz  A {@link Class} instance that represents the target type.
     */
    protected DataStoreListener(Class<T> clz) {
        targetType = clz;
    }

    /**
     * Register this instance as a data change listener.
     *
     * @param service  A {@link DataTreeChangeService} instance.
     * @param store    A {@link LogicalDatastoreType} instance used to
     *                 determine the target datastore.
     */
    protected final void registerListener(DataTreeChangeService service,
                                          LogicalDatastoreType store) {
        InstanceIdentifier<T> path = getWildcardPath();
        DataTreeIdentifier<T> ident = new DataTreeIdentifier<>(store, path);
        try {
            ListenerRegistration<DataTreeChangeListener<T>> reg = service.
                registerDataTreeChangeListener(ident, this);
            registration.set(reg);
        } catch (Exception e) {
            String msg = new StringBuilder(
                "Failed to register data tree change listener: type=").
                append(targetType.getName()).
                append(", path=").append(path).
                toString();
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
     * Determine whether the specified event type should be handled or not.
     *
     * <p>
     *   Note that this method of this class always returns {@code true},
     *   which means all event types should be listened. Subclass can override
     *   this method to filter out events.
     * </p>
     *
     * @param type  A {@link VtnUpdateType} instance which indicates the event
     *              type.
     * @return  {@code true} if the given event type should be handled.
     *          {@code false} otherwise.
     */
    protected boolean isRequiredEvent(@Nonnull VtnUpdateType type) {
        return true;
    }

    /**
     * Invoked when {@link #onDataTreeChanged(Collection)} has just been
     * called.
     *
     * @return  A new event context.
     */
    protected abstract C enterEvent();

    /**
     * Invoked when {@link #onDataTreeChanged(Collection)} is going to leave.
     *
     * <p>
     *   Note that this method may be called even if no modified data was
     *   notified.
     * </p>
     *
     * @param ectx  An event context created by {@link #enterEvent()}.
     */
    protected abstract void exitEvent(C ectx);

    /**
     * Invoked when a new data has been created in the datastore.
     *
     * @param ectx  An event context created by {@link #enterEvent()}.
     * @param path  An instance identifier that specifies data.
     * @param data  A created data object.
     */
    protected abstract void onCreated(
        C ectx, @Nonnull InstanceIdentifier<T> path, @Nonnull T data);

    /**
     * Invoked when a data in the datastore has been updated.
     *
     * @param ectx     An event context created by {@link #enterEvent()}.
     * @param path     An instance identifier that specifies data.
     * @param oldData  An old data object.
     * @param newData  An updated data object.
     */
    protected abstract void onUpdated(
        C ectx, @Nonnull InstanceIdentifier<T> path, @Nonnull T oldData,
        @Nonnull T newData);

    /**
     * Invoked when a data has been removed from the datastore.
     *
     * @param ectx  An event context created by {@link #enterEvent()}.
     * @param path  An instance identifier that specifies data.
     * @param data  A removed data object.
     */
    protected abstract void onRemoved(
        C ectx, @Nonnull InstanceIdentifier<T> path, @Nonnull T data);

    /**
     * Determine whether the specified data was updated or not.
     *
     * @param before  The target data object before modification.
     * @param after   The target data object after modification.
     * @return  {@code true} if the target data was updated.
     *          {@code false} otherwise.
     */
    protected abstract boolean isUpdated(@Nonnull T before, @Nonnull T after);

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
     * Verify an instance identifier notified by a data tree change event.
     *
     * @param path   An instance identifier that specifies data.
     * @param label  A label only for logging.
     * @return  {@code true} only if a given instance identifier  is valid.
     */
    private boolean checkPath(@Nonnull InstanceIdentifier<?> path,
                              Object label) {
        boolean wild = path.isWildcarded();
        if (wild) {
            getLogger().warn("{}: Ignore wildcard path: {}", label, path);
        }

        return !wild;
    }

    /**
     * Record a log that indicates a {@code null} is notified as the value of
     * the tree node.
     *
     * @param path   An {@link InstanceIdentifier} instance.
     * @param label  A label only for logging.
     */
    private void nullValue(InstanceIdentifier<?> path, Object label) {
        getLogger().warn("{}: Null value is notified: path={}", label, path);
    }

    /**
     * Notify the created tree node.
     *
     * @param ectx   An event context created by this instance.
     * @param path   The path to the target tree node.
     * @param value  The created tree node.
     */
    private void notifyCreated(C ectx, @Nonnull InstanceIdentifier<T> path,
                               @Nonnull T value) {
        VtnUpdateType utype = VtnUpdateType.CREATED;
        if (isRequiredEvent(utype)) {
            if (checkPath(path, utype)) {
                onCreated(ectx, path, value);
            }
        }
    }

    /**
     * Notify the updated tree node.
     *
     * @param ectx    An event context created by this instance.
     * @param path    The path to the target tree node.
     * @param before  The target tree node before modification.
     * @param after   The target tree node after modification.
     */
    private void notifyUpdated(C ectx, @Nonnull InstanceIdentifier<T> path,
                               @Nonnull T before, @Nonnull T after) {
        VtnUpdateType utype = VtnUpdateType.CHANGED;
        if (isRequiredEvent(utype)) {
            if (checkPath(path, utype)) {
                if (isUpdated(before, after)) {
                    onUpdated(ectx, path, before, after);
                }
            }
        }
    }

    /**
     * Notify the removed tree node.
     *
     * @param ectx   An event context created by this instance.
     * @param path   The path to the target tree node.
     * @param value  The removed tree node.
     */
    private void notifyRemoved(C ectx, @Nonnull InstanceIdentifier<T> path,
                               T value) {
        VtnUpdateType utype = VtnUpdateType.REMOVED;
        if (isRequiredEvent(utype)) {
            if (value == null) {
                nullValue(path, utype);
            } else if (checkPath(path, utype)) {
                onRemoved(ectx, path, value);
            }
        }
    }

    /**
     * Handle the given data tree modification.
     *
     * @param ectx    An event context created by {@link #enterEvent()}.
     * @param change  A {@link DataTreeModification} instance.
     */
    private void handleTree(C ectx, @Nonnull DataTreeModification<T> change) {
        InstanceIdentifier<T> path = change.getRootPath().getRootIdentifier();
        DataObjectModification<T> mod = change.getRootNode();
        ModificationType modType = mod.getModificationType();
        T before = mod.getDataBefore();
        if (modType == ModificationType.DELETE) {
            // The target data has been removed.
            notifyRemoved(ectx, path, before);
        } else {
            T after = mod.getDataAfter();
            if (after == null) {
                // This should never happen.
                nullValue(path, "handleTree");
            } else if (before == null) {
                // The target data has been created.
                notifyCreated(ectx, path, after);
            } else {
                // The target data has been updated.
                notifyUpdated(ectx, path, before, after);
            }
        }
    }

    // AutoCloseable

    /**
     * Close this listener.
     */
    @Override
    public void close() {
        ListenerRegistration<?> reg = registration.getAndSet(null);
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

    // DataTreeChangeListener

    /**
     * Invoked when the specified data tree has been modified.
     *
     * @param changes  A collection of data tree modifications.
     */
    @Override
    public final void onDataTreeChanged(
        @Nonnull Collection<DataTreeModification<T>> changes) {
        C ectx = enterEvent();
        try {
            for (DataTreeModification<T> change: changes) {
                handleTree(ectx, change);
            }
        } catch (RuntimeException e) {
            getLogger().error(
                "Unexpected exception in data tree change event listener.", e);
        } finally {
            exitEvent(ectx);
        }
    }
}
